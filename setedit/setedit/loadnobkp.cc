/* Copyright (C) 2001-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#define Uses_string
#define Uses_stdio
#define Uses_stdlib

// TV
#define Uses_TNSCollection
#define Uses_MsgBox
// EasyDiag
#define Uses_TSHzLabel
#define Uses_TSInputLine
#define Uses_TSButton
// SetTVUti
#define Uses_TStringable
#define Uses_TDialogAID
// TCEditor
#define Uses_TCEditor_External // execDialog
#define Uses_EditorId
#include <easydia1.h>
#include <ceditor.h>
#include <easydiag.h>

#include <ced_pcre.h>
#include <loadshl.h>
#include <loadnobkp.h>
#define Uses_SETAppVarious // EdReloadIfOpened
#include <setapp.h>

class TNBKPColl;
typedef struct
{
 const char *texto;
 pcre *regex;
} strNBKP;

static char      *NBKPFile=0,*NBKPSaveFile=0;
static TNBKPColl *NBKPList=0;
static PCREData   NBKPpcre={0,0};
static int        listChanged;
static unsigned   localCtxHelp;
static char       warnSaveDifDir=1;

const char *Title=__("Don't create backups for");

static
void ReplaceCRby0(char *s)
{
 for (; *s && *s!='\n' && *s!='\r'; s++);
 *s=0;
}

class TNBKPColl : public TNSCollection, public TStringable
{
public:
 TNBKPColl() : TNSCollection(6,6), TStringable() {};
 // TStringable:
 virtual void getText(char *dest, unsigned item, int maxLen);
 virtual unsigned GetCount(void) { return getCount(); };
 // TNSCollection:
 virtual void freeItem(void *item);
 // TNBKPColl:
 strNBKP *At(ccIndex item) { return (strNBKP *)(at(item)); };
 int  check(const char *name);
 void Load(const char *file);
 void Insert(const char *text, char NoCpText=0);
 int  Save(const char *file);
};

void TNBKPColl::getText(char *dest, unsigned item, int maxLen)
{
 const char *ori=At(item)->texto;
 strncpy(dest,ori,maxLen);
 dest[maxLen]=EOS;
}

void TNBKPColl::freeItem(void *item)
{
 strNBKP *p=(strNBKP *)item;
 DeleteArray(p->texto);
 ::free(p->regex);
 delete p;
}

int TNBKPColl::check(const char *name)
{
 int i;
 int l=strlen(name);
 for (i=0; i<count; i++)
    {
     strNBKP *p=(strNBKP *)items[i];
     if (p->regex && PCREDoSearch((char *)name,l,p->regex,NBKPpcre))
        return 0;
    }
 return 1;
}

void TNBKPColl::Load(const char *file)
{
 FILE *f=fopen(file,"rt");
 if (!f) return;
 PCREInitCompiler(NBKPpcre);

 char b[NBKPMaxLineLen];
 while (!feof(f))
   {
    if (fgets(b,NBKPMaxLineLen-1,f))
      {
       ReplaceCRby0(b);
       Insert(b);
      }
   }

 PCREStopCompiler(NBKPpcre);
 fclose(f);
}

int TNBKPColl::Save(const char *file)
{
 FILE *f=fopen(file,"wt");
 if (!f) return 0;
 int i;
 for (i=0; i<count; i++)
    {
     strNBKP *p=(strNBKP *)items[i];
     fputs(p->texto,f);
     fputc('\n',f);
    }
 int ret=ferror(f)==0;
 fclose(f);
 return ret;
}

void TNBKPColl::Insert(const char *text, char NoCpText)
{
 strNBKP *p=new strNBKP;
 p->texto=NoCpText ? text : newStr(text);
 p->regex=PCRECompileRegEx((char *)text,NBKPpcre);
 insert(p);
}

static
void NBKPLoad()
{
 if (!NBKPFile) return;
 NBKPList=new TNBKPColl();
 NBKPList->Load(NBKPFile);
}

int NBKPMakeIt(const char *name)
{
 if (!NBKPList)
   {
    NBKPLoad();
    if (!NBKPList)
       return 1;
   }
 return NBKPList->check(name);
}

static
int DeleteItem(int wich)
{
 NBKPList->atRemove(wich);
 listChanged++;
 return 1;
}

static
char *EditItem(const char *s)
{
 char b[NBKPMaxLineLen];
 TSViewCol *col=new TSViewCol(Title);

 TSHzLabel *lineLabel=new TSHzLabel(__("Perl regular expression"),
                                    new TSInputLine(NBKPMaxLineLen,50));

 col->insert(xTSCenter,yTSUpSep,lineLabel);
 EasyInsertOKCancel(col,3);

 TDialog *d=col->doItCenter(localCtxHelp);
 delete col;

 strcpy(b,s);
 unsigned ret=execDialog(d,b);
 if (ret==cmOK)
    return newStr(b);
 return 0;
}

static
int AddItem(void)
{
 char *s=EditItem("");
 if (s)
   {
    NBKPList->Insert(s,1);
    listChanged++;
    return 1;
   }
 return 0;
}

static
int CancelConfirm(void)
{
 if (listChanged)
    return messageBox(__("Do you want to discard the changes?"),
                      mfWarning | mfYesButton | mfNoButton)==cmYes;
 return 1;
}

void NBKPEdit(void)
{
 if (!NBKPList)
   {
    NBKPLoad();
    if (!NBKPList || !NBKPSaveFile)
       return;
   }
 TDialogAID *d=CreateAddInsDelDialog(-1,-1,Title,12,50,aidOKEnabled);
 d->helpCtx=localCtxHelp;
 d->DelAction=DeleteItem;
 d->AddAction=AddItem;
 d->CancelAction=CancelConfirm;

 TStringableListBoxRec box;
 box.items=NBKPList;
 box.selection=0;
 listChanged=0;

 unsigned ret=execDialog(d,&box);
 if (listChanged)
   {
    if (ret==cmOK)
      {
       stEditorId idFile;
       int reLoad=FillEditorId(&idFile,NBKPSaveFile);
       NBKPList->Save(NBKPSaveFile);
       if (reLoad)
          EdReloadIfOpened(NBKPSaveFile,&idFile);
       if (warnSaveDifDir && strcmp(NBKPSaveFile,NBKPFile)!=0)
         {
          ShowSavePoint(NBKPSaveFile);
          warnSaveDifDir=0;
         }
      }
    else
      {
       NBKPList->freeAll();
       NBKPList->Load(NBKPFile);
      }
   }
}

void NBKPSetFileName(const char *name, unsigned ctxHelp)
{
 DeleteArray(NBKPFile);
 NBKPFile=newStr(name);
 localCtxHelp=ctxHelp;
}

void NBKPSetSaveFileName(const char *save)
{
 DeleteArray(NBKPSaveFile);
 NBKPSaveFile=newStr(save);
}

void UnloadNBKP()
{
 DeleteArray(NBKPFile);
 NBKPFile=0;
 DeleteArray(NBKPSaveFile);
 NBKPSaveFile=0;
 destroy0(NBKPList);
 PCREDataDestroy(NBKPpcre);
}
