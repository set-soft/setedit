/* Copyright (C) 2001-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  Handles the list of directories to search for when the user press ^+Enter
over a file name. It is usually the path for include files.@*
  It is handled as some kind of string collection (TPathList), it needs
more memory and disk space but is much more easy to setup.
  
***************************************************************************/

#include <ceditint.h>
#define Uses_string
#define Uses_limits
#define Uses_stdlib
// TV
#define Uses_TCollection
#define Uses_TStreamableClass
#define Uses_fpstream
#define Uses_MsgBox
// EasyDiag
#define Uses_TSHzLabel
#define Uses_TSInputLine
#define Uses_TSButton
// SetTVUti
#define Uses_TStringable
#define Uses_TDialogAID

#include <easydia1.h>
#include <settvuti.h>
#include <easydiag.h>

#define Uses_SETAppDialogs
#define Uses_SETAppConst
#include <setapp.h>

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

SetDefStreamOperators(TPathList)

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

static TPathList *IncludeList=0;
const char Version=1;
const int MaxLineLen=PATH_MAX;
static const char *Title=__("Path to look for files under cursor");
static int listChanged;

void PathListSave(fpstream& s)
{
 s << Version;
 if (IncludeList && IncludeList->getCount())
    s << (uchar)1 << IncludeList;
 else
    s << (uchar)0;
}

void PathListLoad(fpstream& s)
{
 uchar aux;
 s >> aux; // Version
 destroy0(IncludeList);
 s >> aux;
 if (aux)
    s >> IncludeList;
}

int PathListGetItem(ccIndex pos, char *buffer)
{
 if (!IncludeList)
   {
    if (!pos)
      {
       // If none defined try to guess
       #ifdef SECompf_djgpp
       char *djdir=getenv("DJDIR");
       if (!djdir)
          djdir="c:/djgpp";
       strcpy(buffer,djdir);
       strcat(buffer,"/include");
       #else
       strcpy(buffer,"/usr/include");
       #endif
       return 1;
      }
    return 0;
   }
 if (pos>=IncludeList->getCount())
    return 0;
 strcpy(buffer,(const char *)IncludeList->at(pos));
 return 1;
}

static
char *EditItem(const char *s)
{
 char b[MaxLineLen];
 TSViewCol *col=new TSViewCol(Title);

 TSHzLabel *lineLabel=new TSHzLabel(__("Directory"),
                                    new TSInputLine(MaxLineLen,60));

 col->insert(xTSCenter,yTSUpSep,lineLabel);
 EasyInsertOKCancel(col,3);

 TDialog *d=col->doItCenter(cmeIncludeList);
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
    IncludeList->insert(s);
    listChanged++;
    return 1;
   }
 return 0;
}

static
int DeleteItem(int which)
{
 IncludeList->atRemove(which);
 listChanged++;
 return 1;
}

static
int CancelConfirm(void)
{
 if (listChanged)
    return messageBox(_("Do you want to discard the changes?"),
                      mfWarning | mfYesButton | mfNoButton)==cmYes;
 return 1;
}

void PathListEdit(void)
{
 if (!IncludeList)
   {
    IncludeList=new TPathList();
    if (!IncludeList)
       return;
   }
 TDialogAID *d=CreateAddInsDelDialog(-1,-1,Title,12,50,aidOKEnabled);
 d->helpCtx=cmeIncludeList;
 d->DelAction=DeleteItem;
 d->AddAction=AddItem;
 d->CancelAction=CancelConfirm;

 TStringableListBoxRec box;
 box.items=IncludeList;
 box.selection=0;
 listChanged=0;

 TPathList *backup=new TPathList();
 *backup=*IncludeList;

 unsigned ret=execDialog(d,&box);
 if (listChanged && ret!=cmOK)
    *IncludeList=*backup;

 destroy(backup);
}

