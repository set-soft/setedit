/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Module: Syntax Help
  Description:
  This module is used to bring help about the word under cursor. I taked
some ideas from the Robert implementation but that's totally diferent. I
was hopping to make it a little bit more friendly.@p
  The interface with the program is done through:@p

int  SyntaxSearch_Search(char *word, char *&FileName, char *&NodeName);@*
int  SyntaxSearch_Load(fpstream &s);@*
int  SyntaxSearch_Save(fpstream &s);@*
int  SyntaxSearch_InitWithDefaults(void);@*
void SyntaxSearch_EditFilesList(void);@*
void SyntaxSearch_EditSettings(void);@*
void SyntaxSearch_ShutDown(void);@*

***************************************************************************/

#include <ceditint.h>
#define Uses_TSortedCollection
#define Uses_TStringable
#define Uses_TDialogAID
#define Uses_fpstream
#define Uses_MsgBox
#define Uses_TVCodePage
// EasyDiag requests
#define Uses_TSInputLine
#define Uses_TSButton
#define Uses_TSStringableListBox
#define Uses_TStreamableClass
#define Uses_TSLabelRadio
#define Uses_TSLabelCheck
#define Uses_TSVeGroup
#define Uses_string
#define Uses_ctype
#define Uses_snprintf
// InfView requests
#include <infr.h>

// First include creates the dependencies
#include <easydia1.h>
#include <settvuti.h>
// Second request the headers
#include <easydiag.h>

#include <inf.h>
#include <ssyntax.h>

// Help context:
#define Uses_SETAppConst
#include <setapp.h>

extern ushort execDialog( TDialog *d, void *data );

const char Version=2;
const int maxFuzzy=1000;

/********************** Settings structure ************************
  This structure holds the search preferences.
  global static.
******************************************************************/

struct SearchSettings
{
 int type;          // scht*
 int fuzzyThresold; // 0..1000
 int caseSensitive; // 0 => no, 1 => yes
 int sortByScore;   // 1 => scores instead of alphabetical
 int otherOps;     // 1 => jumps to the word in the node
};

const int schtExact=0,schtSubStr=1,schtFuzzy=2;

/* Hardcoded defaults: Fuzzy with 600 as trigger, case sensitive and
   sorted by score */
static SearchSettings seSet={schtFuzzy,600,1,1,1};


/**[txh]********************************************************************

  Class: TFileEntryCollection
  Description:
  This class contains the list of entries provided by the user for syntax
search, each entry is a file/node pair.@p
  That's the main class, but it's relative independent of InfView.

***************************************************************************/

// This structure holds the information for one entry in the list of
// files/nodes to search provided by the user
typedef struct
{
 char *file;
 char *node;
 char isOK;
 TStringCollection *items;
} entryInfo;

const  char fileOK=1,fileNotFound=0,fileDontKnow=2,fileCheckIt=3;
static char fileInfo[]={'*',' ','?'};

class TFileEntryCollection : public TStringCollection, public TStringable
{
public:
 TFileEntryCollection(ccIndex aLimit, ccIndex aDelta) :
   TStringCollection(aLimit,aDelta) { };
 void insert(entryInfo *entry);
 void insertCopy(char *file, char *node, int ok=fileDontKnow, TStringCollection *it=0);
 char Solve(entryInfo *entry);
 virtual int compare(void *key1, void *key2);
 virtual void getText(char *dest, unsigned item, int maxLen);
 virtual unsigned GetCount(void);
 virtual void freeItem(void *s);

 SetDefStreamMembers(TFileEntryCollection,TStringCollection)
};

SetDefStreamOperators(TFileEntryCollection)
const char * const TFileEntryCollection::name="TFileEntryCollection";
s(FileEntryCollection);

void TFileEntryCollection::insert(entryInfo *entry)
{
 TSortedCollection::insert(entry);
}

void TFileEntryCollection::insertCopy(char *file, char *node, int ok,
                                      TStringCollection *it)
{
 entryInfo *entry;

 entry=new entryInfo;
 if (entry)
   {
    entry->file=strdup(file);
    entry->node=strdup(node);
    switch (ok)
      {
       case fileCheckIt:
            entry->isOK=Solve(entry);
            break;
       case fileOK:
            entry->isOK=ok;
            entry->items=it;
            break;
       case fileNotFound:
       case fileDontKnow:
            entry->isOK=ok;
            entry->items=0;
            break;
      }
    insert(entry);
   }
}

int TFileEntryCollection::compare(void *key1, void *key2)
{
 entryInfo *s1=(entryInfo *)key1;
 entryInfo *s2=(entryInfo *)key2;
 int ret;
 ret=strcmp(s1->file,s2->file);
 if (ret==0)
   {
    if (s1->node)
       return s2->file ? strcmp(s1->node,s2->node) : -1;
    return s2->file ? 1 : 0;
   }
 return ret;
}

void TFileEntryCollection::getText(char *dest, unsigned item, int maxLen)
{
 entryInfo *p=(entryInfo *)(at(item));
 int d=0;
 char *s=p->file;
 maxLen--;

 dest[d++]=fileInfo[(int) p->isOK];
 dest[d++]=' ';
 if (s)
   {
    if (d<maxLen)
       dest[d++]='(';
    while (d<maxLen && *s)
      {
       dest[d++]=*(s++);
      }
    if (d<maxLen)
       dest[d++]=')';
   }
 s=p->node;
 while (d<maxLen && *s)
   {
    dest[d++]=*(s++);
   }
 if (d<maxLen && p->isOK==fileOK && p->items)
   {
    char buf[64],*aux;
    aux=TVIntl::getTextNew(__(" [%d nodes]"));
    CLY_snprintf(buf,64,aux,p->items->getCount());
    DeleteArray(aux);
    s=buf;
    while (d<maxLen && *s)
       dest[d++]=*(s++);
   }
 dest[d++]=0;
}

unsigned TFileEntryCollection::GetCount(void)
{
 return getCount();
}

void TFileEntryCollection::writeItem(void *obj, opstream& os)
{
 entryInfo *p=(entryInfo *)obj;
 os.writeString(p->file);
 os.writeString(p->node);
}

void *TFileEntryCollection::readItem(ipstream& is)
{
 entryInfo *p=new entryInfo;
 p->file=is.readString();
 p->node=is.readString();
 p->isOK=fileDontKnow;
 p->items=0;
 return p;
}

void TFileEntryCollection::freeItem(void *s)
{
 entryInfo *p=(entryInfo *)s;
 delete[] p->file;
 delete[] p->node;
 if (p->isOK==fileOK)
    CLY_destroy(p->items);
 delete p;
}

static
char *StrDupDual(char *s1, char *s2)
{
 int l2=0;
 int l1=strlen(s1)+1;
 int l=l1+1;
 int use_s2=s2 && strcmp(s1,s2)!=0;
 if (use_s2)
   {
    l2=strlen(s2);
    l+=l2;
   }

 char *s=new char[l];
 memcpy(s,s1,l1);
 if (use_s2)
    memcpy(s+l1,s2,l2+1);
 else
    s[l1]=0;

 return s;
}

char TFileEntryCollection::Solve(entryInfo *entry)
{
 TInfFile *i=new TInfFile(entry->file);

 if (!i || i->Status)
   {
    delete i; // Sure, delete checks for 0
    return fileNotFound;
   }

 if (entry->node[0]==0)
   {
    TInfIndexCollection *p=i->index->coll;
    int j,c;

    c=p->getCount();
    entry->items=new TStringCollection(c,2);
    for (j=0; j<c; j++)
        entry->items->insert( StrDupDual(p->GetString(p->at(j)),0) );
   }
 else
   {
    int suggY;
    TInfTopic *topic=i->getTopic(entry->node,0,moinCutNodeWord | moinHideNodeLink,suggY);
    if (!topic || topic->Status || topic->numRefs==0 ||
        (entry->items=new TStringCollection(topic->numRefs,2))==0)
      {
       delete topic;
       delete i;
       return fileNotFound;
      }
    int j;
    for (j=0; j<topic->numRefs; j++)
        entry->items->insert( StrDupDual(topic->crossRefs[j].Name2,
                                         topic->crossRefs[j].Name) );
    delete topic;
   }
 delete i;
 return fileOK;
}
/************************************************************************************/

static TFileEntryCollection *entries=0;
static TFileEntryCollection *entriesCopy=0;
static int Modified;

int SyntaxSearch_GetJumpOption(void)
{
 return seSet.otherOps & 1;
}

/**[txh]********************************************************************

  Description:
  Initializes the files/nodes list with hardcoded defaults. Only if the list
isn't initialized yet.

  Return:
  0 on success.

***************************************************************************/

int SyntaxSearch_InitWithDefaults(void)
{
 if (entries)
    return 0;
 entries=new TFileEntryCollection(4,2);
 if (!entries)
    return 1;
 #ifdef TVCompf_djgpp
 entries->insertCopy("libc","Alphabetical List");
 entries->insertCopy("allegro","Index");
 #else
 entries->insertCopy("libc","Function Index");
 #endif
 return 0;
}

/**************************** List of files User Interface *************************/
// Action for delete button
static
int DeleteItem(int wich)
{
 entriesCopy->atFree(wich);
 Modified=1;
 return 1;
}

static
int OkApply(void)
{
 return 1;
}


static
int AddNewItem(void)
{
 struct
 {
  char file[60];
  char node[60];
 } box;

 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),__("New node for search")));

 TSInputLine *file=new TSInputLine(60);
 TSLabel *File=new TSLabel(__("Name of the info file"),file);
 TSInputLine *node=new TSInputLine(60);
 TSLabel *Node=new TSLabel(__("Name of the node (leave blank for all)"),node);

 col->insert(xTSCenter,2,File);
 col->insert(xTSCenter,yTSUnder,Node,0,File);
 EasyInsertOKCancel(col,3);

 TDialog *d=col->doIt();
 delete col;

 d->options|=ofCentered;
 d->helpCtx=cmeSyntaxHelpFiles;
 box.file[0]=0;
 strcpy(box.node,"Index");
 if (execDialog(d,&box)!=cmCancel)
   {
    entriesCopy->insertCopy(box.file,box.node,fileCheckIt);
    Modified=1;
    return 1;
   }
 return 0;
}

char *DuplicateDual(char *s)
{
 int l=strlen(s)+1;
 if (s[l]!=0)
    l+=strlen(s+l);
 l++;

 char *ret=new char[l];
 memcpy(ret,s,l);
 return ret;
}

static
void forEachAction(void *item, void *)
{
 entryInfo *p=(entryInfo *)item;
 TStringCollection *it=0;

 if (p->isOK==fileOK && p->items)
   {
    int i,c=p->items->getCount();
    it=new TStringCollection(c,2);
    for (i=0; i<c; i++)
        it->insert( DuplicateDual((char *)p->items->at(i)) );
   }
 entriesCopy->insertCopy(p->file,p->node,p->isOK,it);
}

void EditSearchList(void)
{
 TStringableListBoxRec boxParam;
 // Make a copy
 entriesCopy=new TFileEntryCollection(entries->getCount(),2);
 if (!entriesCopy)
    return;
 entries->forEach(forEachAction,0);

 TDialogAID *d=CreateAddInsDelDialog(-1,-1,__("Nodes for syntax help"),12,56,0);
 d->helpCtx=cmeSyntaxHelpFiles;
 boxParam.items=entriesCopy;
 boxParam.selection=0;
 d->DelAction=DeleteItem;
 d->AddAction=AddNewItem;
 d->OkAction =OkApply;
 d->CancelAction=0;
 Modified=0;

 if (execDialog(d,&boxParam)!=cmCancel && Modified)
   {
    CLY_destroy(entries);
    entries=entriesCopy;
   }
 else
    CLY_destroy(entriesCopy);
 entriesCopy=0;
}

/**[txh]********************************************************************

  Description:
  External routine for the call to the dialog that allows the user
configure the list of files/nodes to uses for syntax help.

***************************************************************************/

void SyntaxSearch_EditFilesList(void)
{
 if (SyntaxSearch_InitWithDefaults())
    return;
 EditSearchList();
}
/******************* End of List of files User Interface *************************/

void SyntaxSearch_EditSettings(void)
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),__("Options for syntax help")));

 /* I think that's easiest way! The following 5 lines are equivalent to 20 lines
    of cryptic stuff if you don't use EasyDiag! (I can write it in 1 too ;-) */
 TSLabel *Method=TSLabelRadio(__("Search ~m~ethod"),__("E~x~act"),__("~S~ubstring"),
                              __("~F~uzzy"),0);
 TSLabel *Options=TSLabelCheck(__("Search O~p~tions"),__("~C~ase sensitive"),
                               __("So~r~t by score"),0);
 TSVeGroup *o1=new TSVeGroup(Method,Options);

 o1->Flags=wSpan;
 TSLabel *FuzzyV=new TSLabel(__("Fu~z~zy value [1..1000]"),new TSInputLine(5));
 
 TSLabel *oOptions=TSLabelCheck(__("Other options"),__("Search word in the ~t~opic"),0);

 col->insert(2,1,o1);
 col->insert(2,yTSUnder,oOptions,0,o1);
 col->insert(2,yTSUnder,FuzzyV,0,oOptions);
 EasyInsertOKCancelSp(col,2);

 TDialog *d=col->doIt();
 d->helpCtx=cmeSyntaxHelpOps;
 delete col;

 d->options|=ofCentered;

 struct
 {
  uint32 type;
  uint32 options;
  uint32 oOps;
  char b[5];
 } box;
 box.type=seSet.type;
 box.options=seSet.caseSensitive ? 1 : 0;
 if (seSet.sortByScore)
    box.options|=2;
 /* Avoid b overflow */
 if (seSet.fuzzyThresold<0)
    seSet.fuzzyThresold=0;
 if (seSet.fuzzyThresold>maxFuzzy)
    seSet.fuzzyThresold=maxFuzzy;
 sprintf(box.b,"%d",seSet.fuzzyThresold);
 box.oOps=seSet.otherOps;

 if (execDialog(d,&box)!=cmCancel)
   {
    seSet.type=box.type;
    seSet.caseSensitive=(box.options & 1) ? 1 : 0;
    seSet.sortByScore=(box.options & 2) ? 1 : 0;
    seSet.fuzzyThresold=atoi(box.b);
    if (seSet.fuzzyThresold<0)
       seSet.fuzzyThresold=0;
    if (seSet.fuzzyThresold>maxFuzzy)
       seSet.fuzzyThresold=maxFuzzy;
    seSet.otherOps=box.oOps;
   }
 return;
}

struct NodeInfo
{
 char *file; // This pointer points to one instance of the file name never delete it!
 char *node; // the same.
 int   points; // Wow, that's a very good idea. Like the Web search engines ;-)
};

class TNodeCollection : public TSortedCollection, public TStringable
{
public:
  TNodeCollection(int aMode) :
    TSortedCollection(15,15), mode(aMode) { duplicates=True; };
  virtual void *keyOf(void *item);
  virtual void getText(char *dest, unsigned item, int maxLen);
  virtual unsigned GetCount(void);
private:
  virtual void *readItem( ipstream& ) { return this; };
  virtual void writeItem( void *, opstream& ) {};
  virtual int compare(void *,void *);
  virtual void freeItem(void *p) { delete (NodeInfo *)p; }
  int mode; // 1 => alphabetical, 0 => score
};

unsigned TNodeCollection::GetCount(void)
{
 return getCount();
}

void *TNodeCollection::keyOf(void *item)
{
 if (mode)
    return (void *)(long)(((NodeInfo *)item)->points);
 return ((NodeInfo *)item)->node;
}

int TNodeCollection::compare(void *n1,void *n2)
{
 if (mode)
    return (long)n1==(long)n2 ? 0 : ((long)n1>(long)n2 ? -1 : 1);
 return strcmp((char *)n1,(char *)n2);
}

const int lenFile=12;

void TNodeCollection::getText(char *dest, unsigned item, int maxLen)
{
 NodeInfo *node=(NodeInfo *)(at(item));
 int i,fpos=maxLen-lenFile-5-2; // 17 file name 5 number 2 because the nature of it
 char *s=node->node;
 char b[12];

 for (i=0; i<fpos && s[i]; i++)
     dest[i]=s[i];
 for (; i<fpos; i++)
     dest[i]=' ';
 s=node->file;
 for (fpos+=lenFile; i<fpos && *s; i++,s++)
     dest[i]=*s;
 for (; i<fpos; i++)
     dest[i]=' ';
 sprintf(b," %4d",node->points);
 s=b;
 for (fpos+=5; i<fpos; i++,s++)
     dest[i]=*s;
 dest[i]=0;
}

/**[txh]********************************************************************

  Description:
  Adds a node to the list of "found nodes".

***************************************************************************/

static
NodeInfo *AddNewNode(entryInfo *file,TNodeCollection *nodes, ccIndex pos, int points)
{
 NodeInfo *i=new NodeInfo;
 i->file=file->file;
 i->node=(char *)file->items->at(pos);
 i->points=points;
 nodes->insert(i);
 return i;
}

/**[txh]********************************************************************

  Description:
  Generic version of the routine that searchs in the list of nodes and adds
matchs to the node collection. SearchFun points to the specialized match
routine.

***************************************************************************/

static
void *GenericSearch(char *word,entryInfo *file,TNodeCollection *nodes,
                    int (*SearchFun)(char *s, char *find))
{
 TStringCollection *items=file->items;
 int cant=items->getCount(),i,ret;
 void *exact=0;

 for (i=0; i<cant; i++)
    {
     ret=SearchFun((char *)items->at(i),word);
     if (ret)
        AddNewNode(file,nodes,i,ret);
    }
 return exact;
}


/*********************** EXACT SEARCH STUFF *******************************/
static
void SearchExact(char *word,entryInfo *file,TNodeCollection *nodes)
{
 ccIndex pos;
 TStringCollection *items=file->items;

 if (seSet.caseSensitive)
   {
    if (items->search(word,pos))
       AddNewNode(file,nodes,pos,1000);
    return;
   }

 // no case sensitive
 char aux[2],*candidate;
 aux[0]=TVCodePage::toLower(word[0]);
 aux[1]=0;
 int cant=items->getCount();
 items->search(aux,pos);
 // Search words starting with the same letter only
 while (pos<cant)
   {
    candidate=(char *)items->at(pos);
    if (*candidate!=*aux)
       break;
    if (strcasecmp(word,candidate)==0)
       AddNewNode(file,nodes,pos,strcmp(word,candidate)==0 ? 1000 : 500);
    pos++;
   }
 aux[0]=TVCodePage::toUpper(aux[0]);
 items->search(aux,pos);
 while (pos<cant)
   {
    candidate=(char *)items->at(pos);
    if (*candidate!=*aux)
       break;
    if (strcasecmp(word,candidate)==0)
       AddNewNode(file,nodes,pos,strcmp(word,candidate)==0 ? 1000 : 500);
    pos++;
   }
}
/******************** End of EXACT SEARCH STUFF ****************************/

/********************** SubString SEARCH STUFF******************************/
/**[txh]********************************************************************

  Description:
  Modified version of strstr to return 1 if the string is inside or 2 if we
have a perfect match.

***************************************************************************/

static
int StrStr(char *s, char *find)
{
 char c, sc;
 size_t len=0,lenS;

 lenS=strlen(s);
 if ((c=*find++)!=0)
   {
    len=strlen(find);
    do
      {
       do
         {
          if ((sc=*s++)==0)
             return 0;
         }
       while (sc!=c);
      }
    while (strncmp(s,find,len)!=0);
    s--;
   }
 len++;
 return len*1000/lenS;
}

/**[txh]********************************************************************

  Description:
  Modified version of strstr to return 1 if the string is inside or 2 if we
have a semi-perfect match. That's the non-case sensitive version.
Attention: assumes the s parameter is uppercase.

***************************************************************************/

static
int StrStrUCase(char *s, char *find)
{
 char c, sc;
 size_t len=0,lenS;

 lenS=strlen(s);
 if ((c=*find++)!=0)
   {
    len=strlen(find);
    do
      {
       do
         {
          if ((sc=*s++)==0)
             return 0;
         }
       while (TVCodePage::toUpper(sc)!=c);
      }
    while (strncasecmp(s,find,len)!=0);
    s--;
   }
 len++;
 return len*1000/lenS;
}

static
void SearchSubString(char *word,entryInfo *file,TNodeCollection *nodes)
{
 if (seSet.caseSensitive)
    GenericSearch(word,file,nodes,StrStr);
 else
    GenericSearch(word,file,nodes,StrStrUCase);
}
/****************** End of SubString SEARCH STUFF ****************************/

/************************ Fuzzy SEARCH STUFF *********************************/

// declared in fstrcmp.h from gettext
extern "C" double fstrcmp(const char *,const char *);
// added by me:
extern "C" void fstrcmp_free_memory(void);

#ifndef TVCompf_djgpp
extern "C" void *xrealloc(void *ptr,int new_size)
{
 return realloc(ptr,new_size);
}
#endif

static
int FuzzyCase(char *s, char *find)
{
 int result=(int)(fstrcmp(s,find)*maxFuzzy);
 return result>=seSet.fuzzyThresold ? result : 0;
}

static
int FuzzyNoCase(char *s, char *find)
{
 char upper[MAX_NODE_NAME];
 /* string copy limiting the size and converting to uppercase */
 int i;
 for (i=0; i<MAX_NODE_NAME-1 && s[i]; i++)
     upper[i]=TVCodePage::toUpper(s[i]);
 upper[i]=0;

 int result=(int)(fstrcmp(upper,find)*maxFuzzy);
 return result>=seSet.fuzzyThresold ? result : 0;
}


static
void SearchFuzzy(char *word,entryInfo *file,TNodeCollection *nodes)
{
 if (seSet.caseSensitive)
    GenericSearch(word,file,nodes,FuzzyCase);
 else
    GenericSearch(word,file,nodes,FuzzyNoCase);
 // We finished owr search, we can release it. Is just to avoid confusing messages in
 // the leaks report.
 fstrcmp_free_memory();
}
/********************* End of Fuzzy SEARCH STUFF ***************************/

class TSelNodeDiag : public TDialog
{
public:
 TSelNodeDiag(TRect r, const char *name) :
      TWindowInit( &TSelNodeDiag::initFrame ),
      TDialog(r,name) {};
 void handleEvent(TEvent& event);
};

void TSelNodeDiag::handleEvent(TEvent& event)
{
 TDialog::handleEvent(event);
 if ( event.what == evCommand || event.what == evBroadcast)
   {
    switch ( event.message.command )
      {
       case cmListItemSelected:
            endModal(event.message.command);
            clearEvent(event);
            break;
       default:
           break;
      }
   }
}

/**[txh]********************************************************************

  Description:
  User interface function to offer to the user a list of possible matchs.
You must pass the list of choices in nodes.

  Return:
  A NodeInfo pointer to the selected node or 0 if none.

***************************************************************************/

static
NodeInfo *ChooseOne(TNodeCollection *nodes)
{
 /* Search the best match */
 int max=0,i,c,mp=0;

 if (!seSet.sortByScore)
   {
    c=nodes->getCount();
    for (i=0; i<c; i++)
       {
        NodeInfo *p=(NodeInfo *)nodes->at(i);
        if (p->points>max)
          {
           max=p->points;
           mp=i;
          }
       }
   }

 TSViewCol *col=new TSViewCol(new TSelNodeDiag(TRect(1,1,1,1),__("Choose a node")));

 TSStringableListBox *Nodes=new TSStringableListBox(73,16,tsslbVertical);
 col->insert(2,1,Nodes);
 EasyInsertOKCancel(col,15);

 TDialog *d=col->doIt();
 delete col;

 d->options|=ofCentered;

 TStringableListBoxRec box;
 box.items=nodes;
 box.selection=mp;

 if (execDialog(d,&box)!=cmCancel)
   {
    return (NodeInfo *)nodes->at(box.selection);
   }
 return 0;
}

/**[txh]********************************************************************

  Description:
  Searchs word, offers a window with the possible matchs and returns the
name of the file and node in the reference parameters. The passed string
belongs to the internal collections so you don't have to delete it.

  Return:
  1 => FileName and NodeName holds the node to jump.@*
  0 => no match.@*
  2 => User aborter.

***************************************************************************/

int SyntaxSearch_Search(char *word, char *&FileName, char *&NodeName,
                        char *&VisibleName)
{
 /* Reject 0 len strings */
 if (!word || *word==0)
    return 0;

 /* Be sure we have a list of files */
 SyntaxSearch_InitWithDefaults();
 if (!entries)
    return 0;

 /* Check the list isn't empty */
 int numFiles=entries->getCount(),numFile;
 if (numFiles==0)
   {
    messageBox(__("No INFO files for syntax help defined"),mfError|mfOKButton);
    return 0;
   }
 entryInfo *file;
 TNodeCollection *nodes=new TNodeCollection(seSet.sortByScore);
 if (!seSet.caseSensitive)
    strupr(word);
 /* Search in each file */
 for (numFile=0; numFile<numFiles; numFile++)
    {
     file=(entryInfo *)(entries->at(numFile));
     if (file->isOK==fileDontKnow || file->isOK==fileCheckIt)
        file->isOK=entries->Solve(file);
     /* Only if the file is there */
     if (file->isOK==fileOK)
       {
        switch (seSet.type)
          {
           case schtExact:
                SearchExact(word,file,nodes);
                break;
           case schtSubStr:
                SearchSubString(word,file,nodes);
                break;
           case schtFuzzy:
                SearchFuzzy(word,file,nodes);
                break;
          }
       }
    }
 int ret=0;
 if (nodes->getCount()>0)
   {
    NodeInfo *choice;
    /* If only one just use it */
    if (nodes->getCount()==1)
      {
       choice=(NodeInfo *)nodes->at(0);
       ret=1;
       FileName=choice->file;
       NodeName=choice->node;
      }
    else
      { /* Ask the user which one */
       choice=ChooseOne(nodes);
       if (choice)
         {
          ret=1;
          FileName=choice->file;
          NodeName=choice->node;
         }
       else
          ret=2;
      }
   }
 CLY_destroy(nodes);
 if (ret==1)
   {
    VisibleName=NodeName;
    int l=strlen(NodeName)+1;
    if (NodeName[l]!=0)
       NodeName+=l;
   }
 return ret;
}

/**[txh]********************************************************************

  Description:
  Just a call to destroy all the allocated objects.

***************************************************************************/

void SyntaxSearch_ShutDown(void)
{
 CLY_destroy(entries);
 entries=0;
}

/**[txh]********************************************************************

  Description:
  Loads the list of files/nodes to search from the s stream.

  Return:
  0 on success.

***************************************************************************/

int SyntaxSearch_Load(fpstream &s)
{
 char cVersion,aux;
 s >> cVersion;
 if (cVersion==1)
   {
    s.readBytes(&seSet,sizeof(SearchSettings)-sizeof(int));
    seSet.otherOps=1;
   }
 else
    s.readBytes(&seSet,sizeof(SearchSettings));
 s >> aux;
 if (aux)
   {
    s >> entriesCopy;
    // Now don't waste any efforts we did in entries, just copy all the index
    // we can find
    if (entries && entriesCopy)
      {
       ccIndex pos,c,i;
       entryInfo *p,*pNew;

       c=entriesCopy->getCount();
       for (i=0; i<c; i++)
          {
           pNew=(entryInfo *)entriesCopy->at(i);
           if (entries->search(pNew,pos))
             {
              p=(entryInfo *)entries->at(pos);
              if (p->isOK==fileOK)
                {
                 pNew->items=p->items;
                 pNew->isOK=fileOK;
                 p->items=0;
                 p->isOK=fileNotFound;
                }
             }
          }
      }
    CLY_destroy(entries);
    entries=entriesCopy;
    entriesCopy=0;
   }
 return 0;
}

/**[txh]********************************************************************

  Description:
  Stores the list of files/nodes to search to the s stream.

  Return:
  0 on success.

***************************************************************************/

int SyntaxSearch_Save(fpstream &s)
{
 s << Version;
 s.writeBytes(&seSet,sizeof(SearchSettings));
 if (entries)
    s << (char)1 << entries;
 else
    s << (char)0;
 return 0;
}


