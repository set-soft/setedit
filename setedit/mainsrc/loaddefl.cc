/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#define Uses_string
#define Uses_ctype
#define Uses_AllocLocal
#define Uses_TSOSStringCollection
#define Uses_TCEditor_External
#define Uses_TCEditor_Internal
#define Uses_TCEditor
#define Uses_TPoint
#define Uses_TDialog
#define Uses_TDialogAID
#define Uses_TStringable
#define Uses_TNoCaseStringCollection
#define Uses_MsgBox

#define Uses_TSSortedListBox
#define Uses_TSButton
#define Uses_TSHzGroup
#define Uses_TSInputLine
#define Uses_TSHzLabel
#define Uses_TSLabelRadio

// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
// Second request the headers
#include <easydiag.h>

#include <pathtool.h>
#include <loadshl.h>
#define Uses_SETAppVarious // EdReloadIfOpened
#include <setapp.h>

static const char *DefaultOptsFileName="deflopts.txt";
static const char *noSHL="None";
static const int   maxDefaultOptLen=80+256;
static const int   stateLookingName=0,stateCollecting=1,stateExitLoop=2;
static char       *destFile=0;
static unsigned    localCtxHelp;
static char        warnSaveDifDir=1;
static TNoCaseStringCollection *listSettings;

static void ReplaceCRby0(char *s)
{
 for (; *s && *s!='\n' && *s!='\r'; s++);
 *s=0;
}

typedef struct
{
 const char *name;
 int len;
 uint16 type;
 uint32 mask;
} setting;

typedef struct
{
 setting st;
 unsigned value;
 char str[colMarkersStrLen+1];
} editSetting;

typedef struct
{
 TCollection *c;
 ccIndex f;
} boxCol;

const uint32 tyFlag=0,tyInt=1,tyStr=2,loTabSize=1,loWrapCol=2,loIndentSize=3,loColArM=1;

setting Settings[]=
{
{"AutoIndent",       10, tyFlag, loAutoIndent},
{"ColMarkersArray",  15, tyStr,  loColArM},
{"ColumnMarkers",    13, tyFlag, loColumnMarkers},
{"CrossCursorInCol", 16, tyFlag, loCrossCursorInCol},
{"CrossCursorInRow", 16, tyFlag, loCrossCursorInRow},
{"DontPurgeSpaces",  15, tyFlag, loDontPurgeSpaces},
{"IndentSize",       10, tyInt,  loIndentSize},
{"IntelIndent",      11, tyFlag, loIntelIndent},
{"NoInsideTabs",     12, tyFlag, loNoInsideTabs},
{"OptimalFill",      11, tyFlag, loOptimalFill},
{"Overwrite",         9, tyFlag, loOverwrite},
{"PersistentBlocks", 16, tyFlag, loPersistentBlocks},
{"SeeTabs",           7, tyFlag, loSeeTabs},
{"ShowMatchPairFly", 16, tyFlag, loShowMatchPairFly},
{"ShowMatchPair",    13, tyFlag, loShowMatchPair},
{"TabIndents",        5, tyFlag, loTabIndents},
{"TabSize",           7, tyInt,  loTabSize},
{"TransparentSel",   14, tyFlag, loTransparentSel},
{"UseIndentSize",    13, tyFlag, loUseIndentSize},
{"UseTabs",           7, tyFlag, loUseTabs},
{"WrapCol",           7, tyInt,  loWrapCol},
{"WrapLine",          8, tyFlag, loWrapLine}
};

const int cantSettings=sizeof(Settings)/sizeof(setting);

static
void ParseIt(char *s, dflOptions *shl, int set)
{
 int i;
 for (i=0; i<cantSettings; i++)
    {
     if (strncasecmp(Settings[i].name,s,Settings[i].len)==0)
        break;
    }
 if (i==cantSettings)
    return; // Not found, just ignore it
 if (Settings[i].type==tyFlag)
   {
    if (set)
       shl->setOpts|=Settings[i].mask;
    else
       shl->resetOpts&=~Settings[i].mask;
   }
 else if (Settings[i].type==tyInt)
   {
    s+=Settings[i].len;
    // Some small parsing:
    for (;*s!='=' && *s; s++);
    if (!*s) return;
    for (s++; ucisspace(*s) && *s; s++);
    if (!*s) return;
    int val=atoi(s);
    if (val<=0) return;
    switch (Settings[i].mask)
      {
       case loTabSize:
            shl->tabSize=val;
            break;
       case loIndentSize:
            shl->indentSize=val;
            break;
       case loWrapCol:
            shl->wrapCol=val;
            break;
      }
   }
 else if (Settings[i].type==tyStr)
   {
    s+=Settings[i].len;
    // Some small parsing:
    for (;*s!='=' && *s; s++);
    if (!*s) return;
    for (s++; ucisspace(*s) && *s; s++);
    if (!*s || *s!='"') return;
    s++;
    char *e;
    for (e=s; *e && *e!='"'; e++);
    if (*e!='"') return;
    *e=0;

    switch (Settings[i].mask)
      {
       case loColArM:
            shl->colMarkers=TCEditor::Str2ColMarkers(s);
            break;
      }
   }
}

/**[txh]********************************************************************

  Description:
  Loads the default options in the shl array.

***************************************************************************/

void LoadDefaultOpts(strSHL *shl, int cant)
{
 // That's the first we should do.
 for (int i=0; i<cant; i++)
    {
     shl[i].df.wrapCol=shl[i].df.tabSize=shl[i].df.setOpts=0;
     shl[i].df.resetOpts=0xFFFFFFFF;
    }

 char *fileName=ExpandHome(DefaultOptsFileName);
 FILE *f=fopen(fileName,"rt");
 if (!f)
    return;

 char line[maxDefaultOptLen];
 int  id;
 dflOptions *dflOps=0;

 while (fgets(line,maxDefaultOptLen,f))
   {
    ReplaceCRby0(line);
    switch (line[0])
      {
       case '.':
            id=SHLNumberOf(line+1);
            if (id>=0)
               dflOps=&shl[id].df;
            else
               if (strcasecmp(line+1,noSHL)==0)
                  dflOps=&TCEditor::dflOps;
               else
                  dflOps=0;
            break;
       case '+':
            if (dflOps)
               ParseIt(line+1,dflOps,1);
            break;
       case '-':
            if (dflOps)
               ParseIt(line+1,dflOps,0);
            break;
      }
   }
 fclose(f);
}

/**[txh]********************************************************************

  Description:
  Returns the name of the file (without path) for the user words file. Is
just because the constant is local.

  Return:
  A pointer to the internal constant.

***************************************************************************/

const char *GetNameOfDefaultOptsFile()
{
 return DefaultOptsFileName;
}

/**[txh]********************************************************************

  Description:
  Asks for a new user setting.

  Return:
  A newly allocated setting or 0 if the user aborted.

***************************************************************************/

static
editSetting *EnterNewSetting()
{
 TDialog *d=CreateChooseDialog(40,8,__("Available"),10,24);
 boxCol box={ listSettings, 0 };
 if (execDialog(d,&box)!=cmOK)
    return 0;

 int sel=box.f;
 // Search what index is it
 char *name=(char *)listSettings->at(sel);
 for (int i=0; i<cantSettings; i++)
     if (name==Settings[i].name)
        sel=i;

 editSetting *ret;
 if (Settings[sel].type==tyFlag)
   {// Ask if is enable or disable.
    TSViewCol *col=new TSViewCol(__("What to do"));
    TSLabel *p=TSLabelRadio(__("Action"),__("~D~isable"),__("~E~nable"),0);
    p->Flags|=wSpan;
    col->insert(2,1,p);
    EasyInsertOKCancel(col);
    TDialog *d=col->doIt(); delete col;
    d->options|=ofCentered; d->helpCtx=localCtxHelp;

    uint32 ops=1;
    if (execDialog(d,&ops)!=cmOK)
       return 0;

    ret=new editSetting;
    ret->value=ops;
   }
 else if (Settings[sel].type==tyInt)
   { // Ask the value
    TSViewCol *col=new TSViewCol(__("Associated value"));
    TSHzLabel *p=new TSHzLabel(__("Value:"),new TSInputLine(6));
    p->Flags|=wSpan;
    col->insert(2,2,p);
    EasyInsertOKCancel(col);
    TDialog *d=col->doIt(); delete col;
    d->options|=ofCentered; d->helpCtx=localCtxHelp;

    char val[6]="0";
    if (execDialog(d,&val)!=cmOK)
       return 0;
    int value=atoi(val);
    if (value<=0)
       return 0;

    ret=new editSetting;
    ret->value=value;
   }
 else
   { // Ask the value
    TSViewCol *col=new TSViewCol(__("Associated string"));
    TSHzLabel *p=new TSHzLabel(__("Value:"),new TSInputLine(256,30));
    p->Flags|=wSpan;
    col->insert(2,2,p);
    EasyInsertOKCancel(col);
    TDialog *d=col->doIt(); delete col;
    d->options|=ofCentered; d->helpCtx=localCtxHelp;

    char val[colMarkersStrLen]="";
    if (execDialog(d,&val)!=cmOK || *val==0)
       return 0;

    ret=new editSetting;
    strcpy(ret->str,val);
   }
 ret->st=Settings[sel];
 return ret;
}

// Look at this
class TSetting : public TStringCollection, public TStringable
{
public:
 TSetting(ccIndex aLimit, ccIndex aDelta) :
   TStringCollection(aLimit,aDelta),
   TStringable() { };

 virtual void getText(char *dest, unsigned item, int maxLen);
 virtual unsigned GetCount(void) { return getCount(); };
 void insert(uint32 mask, uint16 type, unsigned value, char *str=0);
 void insert(editSetting *p) { TStringCollection::insert((void *)p); };
 editSetting *at(ccIndex pos) { return (editSetting *)TStringCollection::at(pos); };
 virtual int compare(void *s1,void *s2);
 virtual void freeItem(void *item) { delete (editSetting *)item; };
 ccIndex searchByName(char *s);
};


ccIndex TSetting::searchByName(char *s)
{
 ccIndex i;
 for (i=0; i<count; i++)
    {
     if (strcasecmp(s,at(i)->st.name)==0)
        return i;
    }
 return -1;
}


void TSetting::getText(char *dest, unsigned item, int maxLen)
{
 editSetting *s=at(item);
 int l;
 if (s->st.type==tyFlag)
    l=s->st.len+3;
 else
    l=s->st.len+3+1+5;
 AllocLocalStr(buf,l);
 if (s->st.type==tyFlag)
    sprintf(buf,"%c%s",s->value ? '+' : '-',s->st.name);
 else if (s->st.type==tyInt)
    sprintf(buf,"+%s=%d",s->st.name,s->value);
 else
    sprintf(buf,"+%s=\"%s\"",s->st.name,s->str);

 strncpy(dest,buf,maxLen);
 dest[maxLen]=EOS;
}

void TSetting::insert(uint32 mask, uint16 type, unsigned value, char *str)
{
 int i;
 editSetting *s=0;

 for (i=0; i<cantSettings; i++)
    {
     if (Settings[i].type==type && Settings[i].mask==mask)
       {
        s=new editSetting;
        s->st=Settings[i];
        if (str)
           strcpy(s->str,str);
        else
           s->value=value;
        break;
       }
    }
 if (i==cantSettings)
    return; // mask not found

 TStringCollection::insert((void *)s);
}

int TSetting::compare(void *s1, void *s2)
{
 editSetting *v1=(editSetting *)s1,*v2=(editSetting *)s2;
 int ret=strcasecmp(v1->st.name,v2->st.name);
 if (!ret)
   {
    if (v1->st.type==tyStr)
       return strcmp(v1->str,v2->str);
    if (v1->value!=v2->value)
       ret=v1->value<v2->value ? -1 : 1;
   }
 return ret;
}

static
void WriteSettings(char *name, TSetting *col, FILE *f)
{
 char buf[maxDefaultOptLen];

 fprintf(f,".%s\n",name);
 int i,c=col->getCount();
 for (i=0; i<c; i++)
    {
     col->getText(buf,i,maxDefaultOptLen-1);
     fprintf(f,"%s\n",buf);
    }
 fprintf(f,"\n"); // Just to make it more readable
}

/**[txh]********************************************************************

  Description:
  Updates the user's word file deleting the old definitions and adding the
new ones. The arguments are the name of the syntax hl and the list of user
words.

***************************************************************************/

static
void UpdateFile(char *name, TSetting *col)
{
 char *origFile=ExpandHome(DefaultOptsFileName);
 FILE *dest,*ori;
 int differentFile=0;
 stEditorId idFile;

 // Be sure the idFile is cleared
 FillEditorId(&idFile);
 if (edTestForFile(origFile))
   {// We must use the values in this file
    if (CompareFileNames(origFile,destFile))
      {// We will overwrite the original
       // We must identify the file before anything
       FillEditorId(&idFile,origFile);
       // Now backup it
       char *bkpName=newStr(origFile);
       ReplaceExtension(bkpName,TCEditor::backupExt,".txt");
       rename(origFile,bkpName);
       ori=fopen(bkpName,"rt");
       delete[] bkpName;
      }
    else
      {// Is OK we are creating another
       ori=fopen(origFile,"rt");
       differentFile=1;
      }
    if (!ori)
       return;
    dest=fopen(destFile,"wt");
    if (!dest)
      {
       fclose(ori);
       return;
      }
    // Put the edited values at the start of the file
    WriteSettings(name,col,dest);
    // Copy all but this definition
    char line[maxDefaultOptLen];
    int  state=stateLookingName;
   
    while (fgets(line,maxDefaultOptLen,ori))
      {
       ReplaceCRby0(line);
       switch (state)
         {
          case stateLookingName:
               if (line[0]=='.' && strcasecmp(name,line+1)==0)
                  state=stateCollecting;
               else
                  fprintf(dest,"%s\n",line);
               break;
          case stateCollecting:
               if (line[0]=='.' && strcasecmp(name,line+1)!=0)
                 {
                  state=stateLookingName;
                  fprintf(dest,"%s\n",line);
                 }
               break;
         }
      }
    fclose(ori);
    if (differentFile && warnSaveDifDir)
      {
       ShowSavePoint(destFile);
       warnSaveDifDir=0;
      }
   }
 else
   {// Just create a new one
    dest=fopen(destFile,"wt");
    WriteSettings(name,col,dest);
    differentFile=1;
   }
 fclose(dest);
 if (!differentFile)
    EdReloadIfOpened(origFile,&idFile);
}

static TSetting *settingsCol;
static int listChanged;

static
int DeleteSetting(int which)
{
 // Make it available to choose
 editSetting *s=settingsCol->at(which);
 ccIndex pos;
 if (!listSettings->search((void *)s->st.name,pos))
    listSettings->atInsert(pos,(void *)s->st.name);
 // Remove from the list
 settingsCol->atRemove(which);
 listChanged++;
 return 1;
}

static
int AddSetting(void)
{
 editSetting *s=EnterNewSetting();
 if (s)
   {// Remove from the availables list
    ccIndex pos;
    if (listSettings->search((void *)s->st.name,pos))
       listSettings->atRemove(pos);
    // Remove from the list
    settingsCol->insert(s);
    listChanged++;
    return 1;
   }
 return 0;
}

static
int CancelConfirm(void)
{
 if (listChanged)
    return messageBox(__("Do you want to discard the changes?"),mfWarning | mfYesButton | mfNoButton)==cmYes;
 return 1;
}

static
void EditList(dflOptions *ops, char *name)
{
 TDialogAID *d=CreateAddInsDelDialog(24,3,__("Settings"),12,30,aidOKEnabled);
 TStringableListBoxRec box;
 d->helpCtx=localCtxHelp;

 // Expand the settings to a list
 settingsCol=new TSetting(4,4);
 uint32 mask=1;
 for (; mask; mask<<=1)
    {
     if (ops->setOpts & mask)
        settingsCol->insert(mask,tyFlag,1);
     if (~ops->resetOpts & mask)
        settingsCol->insert(mask,tyFlag,0);
    }
 if (ops->tabSize)
    settingsCol->insert(loTabSize,tyInt,ops->tabSize);
 if (ops->indentSize)
    settingsCol->insert(loIndentSize,tyInt,ops->indentSize);
 if (ops->wrapCol)
    settingsCol->insert(loWrapCol,tyInt,ops->wrapCol);
 if (ops->colMarkers)
   {
    char b[colMarkersStrLen];
    TCEditor::ColMarkers2Str(ops->colMarkers,b,colMarkersStrLen);
    settingsCol->insert(loColArM,tyStr,0,newStr(b));
   }


 // Create a list of settings to choose
 listSettings=new TNoCaseStringCollection(cantSettings,1);
 listSettings->setOwnerShip(False);
 for (int i=0; i<cantSettings; i++)
    {
     char *s=(char *)Settings[i].name;
     // Only if not already available
     if (settingsCol->searchByName(s)<0)
        listSettings->insert(s);
    }

 box.items=settingsCol;
 box.selection=0;
 d->DelAction=DeleteSetting;
 d->AddAction=AddSetting;
 d->CancelAction=CancelConfirm;

 if (settingsCol->getCount()==0)
   {
    //TView::disableCommand(cmOKApply); All is ok
    TView::disableCommand(cmDeleteKey);
   }
 listChanged=0;
 int ret=execDialog(d,&box);
 if (ret==cmOK && listChanged)
   {// Compact the settings
    int i,c=settingsCol->getCount();
    ops->wrapCol=ops->tabSize=ops->setOpts=ops->indentSize=0;
    ops->resetOpts=0xFFFFFFFF;
    editSetting *st;
    for (i=0; i<c; i++)
       {
        st=settingsCol->at(i);
        if (st->st.type==tyFlag)
          {
           if (st->value)
              ops->setOpts|=st->st.mask;
           else
              ops->resetOpts&=~st->st.mask;
          }
        else if (st->st.type==tyInt)
          {
           switch (st->st.mask)
             {
              case loTabSize:
                   ops->tabSize=st->value;
                   break;
              case loIndentSize:
                   ops->indentSize=st->value;
                   break;
              case loWrapCol:
                   ops->wrapCol=st->value;
                   break;
             }
          }
        else
          {
           delete[] ops->colMarkers;
           ops->colMarkers=TCEditor::Str2ColMarkers(st->str);
          }
       }
    UpdateFile(name,settingsCol);
   }
 CLY_destroy(settingsCol);
 CLY_destroy(listSettings);
}

class TDiaDO : public TDialog
{
public:
 TDiaDO(void);
 virtual void handleEvent(TEvent& event);
 TSortedListBox *list;
};

TDiaDO::TDiaDO(void) :
        TWindowInit(&TDiaDO::initFrame),
        TDialog(TRect(1,1,1,1),__("Default global options"))
{
}

void TDiaDO::handleEvent(TEvent& event)
{
 if (event.what==evCommand)
   {
    if (event.message.command==cmYes)
      {
       char *s=(char *)(TCEditor::SHLGenList->at(list->focused));
       unsigned id=SHLNumberOf(s);
       EditList(&TCEditor::SHLArray[id].df,s);
       clearEvent(event);
      }
    else
    if (event.message.command==cmNo)
      {
       EditList(&TCEditor::dflOps,(char *)noSHL);
       clearEvent(event);
      }
   }
 TDialog::handleEvent(event);
}

/**[txh]********************************************************************

  Description:
  Entry point for the dialogs stuff of the default options. Call it passing
the name of the destination file where the definitions will be stored.@p
  You should specify the help context to be used for the dialogs.

***************************************************************************/

void TCEditDefaultOpts(char *destinationFile, unsigned ctxHelp)
{
 if (!TCEditor::SHLGenList)
    return;

 destFile=newStr(destinationFile);
 localCtxHelp=ctxHelp;

 TDiaDO *ddo=new TDiaDO();
 TSViewCol *col=new TSViewCol(ddo);

 #define VeSHLW 30
 TSSortedListBox *ListaH=new TSSortedListBox(VeSHLW,12,tsslbVertical);
 ddo->list=(TSortedListBox *)ListaH->view;
 ListaH->Flags|=wSpan;

 TSHzGroup *buts=new TSHzGroup(new TSButton(__("~E~dit"),cmYes,bfDefault),
                 new TSHzGroup(new TSButton(__("O~K~"),cmOK),
                 new TSButton(__("~N~o SHL"),cmNo)));

 col->insert(2,1,ListaH);
 col->insert(xTSCenter,yTSDown,buts);
 col->doIt();
 delete col;

 ddo->helpCtx=localCtxHelp;
 boxCol box={ TCEditor::SHLGenList, 0 };
 execDialog(ddo,&box);

 delete[] destFile;
 destFile=0;
}
