/* Copyright (C) 2001-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Module: Path Lists
  Comments:
  Handles the list of directories to search for when the user press ^+Enter
over a file name. It is usually the path for include files.@*
  It is handled as some kind of string collection (TPathList), it needs
more memory and disk space but is much more easy to setup. [Note: Here I'm
comparing it with the simple string used in RHIDE]
  
***************************************************************************/

#include <ceditint.h>
#define Uses_string
#define Uses_limits
#define Uses_stdlib
#define Uses_stdio
#define Uses_unistd
// TV
#define Uses_TCollection
#define Uses_TStreamableClass
#define Uses_fpstream
#define Uses_MsgBox
#define Uses_TScreen
// EasyDiag
#define Uses_TSHzLabel
#define Uses_TSInputLine
#define Uses_TSButton
#define Uses_AllocLocal
// SetTVUti
#define Uses_TStringable
#define Uses_TDialogAID

#include <easydia1.h>
#include <settvuti.h>
#include <easydiag.h>

#define Uses_SETAppDialogs
#define Uses_SETAppConst
#define Uses_SETAppProject
#include <setapp.h>
#include <rhutils.h>
#include <pathlist.h>
#include <edspecs.h>
#include <fileopen.h>
#include <debug.h> // DBG_AddPathForSources

class TPathList : public TCollection, public TStringable
{
public:
 TPathList() : TCollection(5,2), TStringable() {};
 // TStringable:
 virtual void getText(char *dest, unsigned item, int maxLen);
 virtual unsigned GetCount(void) { return getCount(); };
 TPathList &operator = (const TPathList & pl);

 SetDefStreamMembers(TPathList,TCollection)
};

const char * const TPathList::name="TPathList";
TStreamableClass RPathList(TPathList::name,TPathList::build,__DELTA(TPathList));

SetDefStreamOperators(TPathList);

void *TPathList::readItem(ipstream& is)
{
 return (void *)is.readString();
}

void TPathList::writeItem(void *obj, opstream& os)
{
 os.writeString((const char *)obj);
}

void TPathList::getText(char *dest, unsigned item, int maxLen)
{
 const char *ori=(const char *)at(item);
 strncpy(dest,ori,maxLen);
 dest[maxLen]=EOS;
}

TPathList & TPathList::operator = (const TPathList & pl)
{
 ccIndex i;
 freeAll();
 for (i=0;i<pl.count;i++)
     insert(newStr((char *)pl.items[i]));
 return *this;
}

static TPathList *lists[paliLists]={NULL,NULL}, *lEdited;
const char Version=2;
const int MaxLineLen=PATH_MAX;
static const char *Titles[paliLists]=
{
 __("Path to look for includes"),
 __("Path to look for sources")
};
static const char *tEdited;
static int listChanged;
static unsigned hCtxEdited;

void PathListSave(fpstream& s)
{
 s << Version;
 s << (uchar)paliLists;
 for (int i=0; i<paliLists; i++)
     if (lists[i] && lists[i]->getCount())
        s << (uchar)1 << lists[i];
     else
        s << (uchar)0;
}

void PathListUnLoad()
{
 for (int i=0; i<paliLists; i++)
     destroy0(lists[i]);
}

void PathListLoad(fpstream& s)
{
 uchar aux, version, cant;
 s >> version;
 PathListUnLoad();
 if (version>=2)
    s >> cant;
 else
    cant=1;
 for (int i=0; i<cant; i++)
    {
     s >> aux;
     if (aux)
        s >> lists[i];
    }
}

static
void PathListAddFromPrj(void *p, void *data)
{
 TPathList *l=(TPathList *)data;
 char f[PATH_MAX];
 ProjectGetNameFromItem(p,f,PATH_MAX);
 // Find the "path" part
 char *end=strrchr(f,'/');
 if (end)
   {
    char aux=*end;
    *end=0;
    // Not sorted, so we must do a brut force search
    int i, c=l->getCount(), found=0;
    for (i=0; i<c; i++)
        if (strcmp(f,(char *)l->at(i))==0)
          {
           found=1;
           break;
          }
    if (!found)
      {
       //printf("Adding %s from project\n",f);
       l->insert(newStr(f));
       // It works only when "stopped"
       DBG_AddPathForSources(f);
      }
    *end=aux;
   }
}

/**[txh]********************************************************************

  Description:
  Adds a path for a project item to the @var{which} list. Note that
@var{item} must be a PrjItem pointer.
  
***************************************************************************/

void PathListAddPathFor(void *item, int which)
{
 if (lists[which])
    PathListAddFromPrj(item,lists[which]);
}

/**[txh]********************************************************************

  Description:
  Fill the list of directories with the include directories. We use the
output of cpp or a guess.
  
  Return: !=0 if we got something from cpp
  
***************************************************************************/

int PathListPopulate(int which)
{
 if (which==paliInclude)
   {
    TPathList *IncludeList=new TPathList();
    lists[paliInclude]=IncludeList;
   
    // 1) Try invoking the GNU preprocessor
    char *err=open_stderr_out();
    TScreen::System("cpp -x c++ -v /dev/null");
    close_stderr_out();
    // Check what we got
    FILE *f=fopen(err,"r");
    if (f)
      {
       char resp[PATH_MAX];
       int state=0;
       while (!feof(f) && state!=2)
         {
          fgets(resp,PATH_MAX,f);
          switch (state)
            {
             case 0:
                  if (strncmp(resp,"#include <",10)==0)
                     state=1;
                  break;
             case 1:
                  if (resp[0]!=' ')
                     state=2;
                  else
                    {// Insert the path
                     int l=1;
                     char *s=resp;
                     for (; s[l] && CLY_IsntEOL(s[l]); l++);
                     s[l]=0;
                     char *path=new char[l];
                     memcpy(path,s+1,l);
                     IncludeList->insert(path);
                     //printf("Agregando <%s>\n",path);
                    }
                  break;
            }
         }
       fclose(f);
      }
    unlink(err);
   
    if (IncludeList->getCount())
       return 1;
   
    char buffer[PATH_MAX];
    #ifdef SECompf_djgpp
    char *djdir=getenv("DJDIR");
    if (!djdir)
       djdir="c:/djgpp";
    strcpy(buffer,djdir);
    strcat(buffer,"/include");
    #else
    strcpy(buffer,"/usr/include");
    #endif
    IncludeList->insert(newStr(buffer));
   }
 else if (which==paliSource)
   {
    TPathList *SourceList=new TPathList();
    lists[paliSource]=SourceList;

    if (IsPrjOpened())
       ProjectApplyToItems(PathListAddFromPrj,SourceList);
   }

 return 0;
}

int PathListGetItem(ccIndex pos, char *buffer, int which)
{
 TPathList *p=lists[which];
 if (!p)
    PathListPopulate(which);
 if (!p || pos>=p->getCount())
    return 0;
 const char *str=(const char *)p->at(pos);
 const char *var=strstr(str,"$(");
 int avail=PATH_MAX-1;
 if (!var)
   {
    int l=min((int)strlen(str),avail);
    memcpy(buffer,str,l);
    buffer[l]=0;
   }
 else
   {// This is some rudimentary $(VARIABLE) expansion
    int offset=0;
    do
      {
       int l=var-str;
       if (offset+l>avail)
          break;
       memcpy(buffer+offset,str,l);
       offset+=l;
       str+=l+2;
       var=strchr(str,')');
       if (var)
         {
          l=var-str;
          AllocLocalStr(v,l+1);
          memcpy(v,str,l);
          v[l]=0;
          const char *vVar=GetVariable(v);
          if (vVar)
            {
             int lVar=strlen(vVar);
             if (offset+lVar>avail)
                break;
             memcpy(buffer+offset,vVar,lVar);
             offset+=lVar;
            }
          str=var+1;
          var=strstr(str,"$(");
          if (!var)
            {
             if (offset+strlen(str)>(unsigned)avail)
                break;
             strcpy(buffer+offset,str);
            }
         }
       else
          buffer[offset]=0;
      }
    while (var);
    //printf("Expandido: %s\n",buffer);
   }
 return 1;
}

static
char *EditItem(const char *s)
{
 char b[MaxLineLen];
 TSViewCol *col=new TSViewCol(tEdited);

 TSHzLabel *lineLabel=new TSHzLabel(__("Directory"),
                                    new TSInputLine(MaxLineLen,60));

 col->insert(xTSCenter,yTSUpSep,lineLabel);
 EasyInsertOKCancel(col,3);

 TDialog *d=col->doItCenter(hCtxEdited);
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
    lEdited->insert(s);
    if (lEdited==lists[paliSource])
       // It works only when "stopped"
       DBG_AddPathForSources(s);
    listChanged++;
    return 1;
   }
 return 0;
}

static
int AddDir(void)
{
 char *s=ChooseDir();
 if (s)
   {
    lEdited->insert(s);
    if (lEdited==lists[paliSource])
       // It works only when "stopped"
       DBG_AddPathForSources(s);
    listChanged++;
    return 1;
   }
 return 0;
}

static
int DeleteItem(int which)
{
 lEdited->atRemove(which);
 listChanged++;
 return 1;
}

static
int CancelConfirm(void)
{
 if (listChanged)
    return messageBox(__("Do you want to discard the changes?"),
                      mfWarning | mfYesButton | mfNoButton)==cmYes;
 return 1;
}

void PathListEdit(int which, unsigned hCtx)
{
 lEdited=lists[which];
 if (!lEdited)
   {
    PathListPopulate(which);
    lEdited=lists[which];
    if (!lEdited)
       return;
   }
 tEdited=Titles[which];
 TDialogAID *d=CreateAddInsDelDialog(-1,-1,tEdited,12,50,aidOKEnabled | aidBrowse);
 d->helpCtx=hCtxEdited=hCtx;
 d->DelAction=DeleteItem;
 d->AddAction=AddItem;
 d->CancelAction=CancelConfirm;
 d->BrowseAction=AddDir;

 TStringableListBoxRec box;
 box.items=lEdited;
 box.selection=0;
 listChanged=0;

 TPathList *backup=new TPathList();
 *backup=*lEdited;

 unsigned ret=execDialog(d,&box);
 if (listChanged && ret!=cmOK)
    *lEdited=*backup;

 CLY_destroy(backup);
}

void PathListAdd(int which, const char *path)
{
 lEdited=lists[which];
 if (!lEdited)
   {
    PathListPopulate(which);
    lEdited=lists[which];
    if (!lEdited)
       return;
   }
 lEdited->insert(newStr(path));
}

