/* Copyright (C) 2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  This is a group of classes to handle TAG files as created by Exuberant
Ctags using format 2.
  The classes implements a sorted collection of tags from one or more tag
files.
  I use the following command line to generate the tags:

ctags -R --exclude="*.mak" --exclude=Makefile --exclude=".*.pl~" --fields=+i+l+m+z

TODO: make a SortedStringableListBox

***************************************************************************/

#define Uses_getcwd
#define Uses_stdio
#define Uses_stdlib
#define Uses_string
#define Uses_getline
#define Uses_ctype
#define Uses_limits
#define Uses_sys_stat
#define Uses_time
#define Uses_snprintf
#define Uses_TStringCollection
#define Uses_TEvent
#define Uses_TApplication
#define Uses_MsgBox
#define Uses_fpstream

#define Uses_TCEditor_External // For mode constants
#define Uses_TCEditor_Commands // For the cmcJumpToFunction context

#define Uses_TSLabel
#define Uses_TSButton
#define Uses_TSHzGroup
#define Uses_TSStaticText
//#define Uses_TSStringableListBox
#define Uses_TSVeGroup
#define Uses_TSSortedListBox

#define Uses_TGrowDialog
#define Uses_TStringable
#define Uses_TDialogAID
#define Uses_FileOpenAid

#include <easydia1.h>
#include <ceditor.h>
#include <easydiag.h>

#include <tags.h>

#define Uses_SETAppConst
#define Uses_SETAppVarious
#include <setapp.h>
#include <rhutils.h>
#include <pathtool.h>

static int InitTagsCollection();

// Variables for this module
static TTagCollection *tags=NULL;

// Small helpers
static
char *toTab(char *e)
{
 for (; *e && *e!='\t'; e++);
 return e;
}

static
char *toTabSp(char *e)
{
 for (; *e && *e!='\t'; e++)
     if (*e=='_')
        *e=' ';
 return e;
}

static
char *toTabSl(char *e)
{
 for (; *e && *e!='\t' && *e!='/'; e++)
     if (*e=='\\')
        e++;
 return e;
}

static
char *toSl(char *e)
{
 for (; *e && *e!='/'; e++)
     if (*e=='\\')
        e++;
 return e;
}

static
char *newStrN(const char *s, int len)
{
 char *r=new char[len+1];
 memcpy(r,s,len);
 r[len]=0;
 return r;
}

/*****************************************************************************
 TTagInfo class
*****************************************************************************/

void TTagInfo::print1(void *item, void *arg)
{
 stTagInfo *p=(stTagInfo *)item;
 printf("%s=%s",p->var,p->value);
 if (p->comment)
    printf(" # %s",p->comment);
 puts("");
}

void TTagInfo::getText(char *dest, unsigned item, int maxLen)
{
 stTagInfo *p=(stTagInfo *)at(item);

 if (p->comment)
    CLY_snprintf(dest,maxLen,"%s=%s # %s",p->var,p->value,p->comment);
 else
    CLY_snprintf(dest,maxLen,"%s=%s",p->var,p->value);
}

void *TTagInfo::keyOf(void *item)
{
 stTagInfo *p=(stTagInfo *)item;
 return (void *)p->var;
}

int TTagInfo::addValue(char *s)
{
 stTagInfo *p=new stTagInfo;
 p->var=NULL;
 p->value=NULL;
 p->comment=NULL;
 s+=6;
 char *e=toTabSp(s);
 if (*e)
   {
    p->var=newStrN(s,e-s);
    s=e+1;
    e=toTab(s);
    if (*e)
      {
       p->value=newStrN(s,e-s);
       s=e+1;
       if (*s=='/') s++;
       e=toTabSl(s);
       if (*e && e-s>1)
          p->comment=newStrN(s,e-s);
      }
   }
 insert(p);
 return 0;
}

void TTagInfo::freeItem(void *item)
{
 if (!item) return;
 stTagInfo *p=(stTagInfo *)item;
 DeleteArray(p->var);
 DeleteArray(p->value);
 DeleteArray(p->comment);
 delete p;
}

/*****************************************************************************
 TTagFiles class
*****************************************************************************/

TTagFiles::TTagFiles() :
  TStringCollection(2,2)
{
}

void TTagFiles::print1(void *item, void *)
{
 stTagFile *p=(stTagFile *)item;
 printf("%s [%s]",p->file,p->base);
 char b[40];
 struct tm *t=localtime(&p->modtime);
 strftime(b,40,"%d/%m/%Y %X",t);
 printf(" %s %d entries\n",b,p->entries);
 printf("TTagInfo:\n");
 p->info->print();
 puts("");
}

void TTagFiles::getText(char *dest, unsigned item, int maxLen)
{
 stTagFile *p=(stTagFile *)at(item);
 if (p->entries<0)
    CLY_snprintf(dest,maxLen,"%s [%s]",p->file,p->entries==-1 ? "error" : "not loaded");
 else
    CLY_snprintf(dest,maxLen,"%s [%d entries]",p->file,p->entries);
}

void TTagFiles::freeItem(void *item)
{
 stTagFile *p=(stTagFile *)item;
 DeleteArray(p->file);
 DeleteArray(p->base);
 destroy(p->info);
 delete p;
}

void *TTagFiles::keyOf(void *item)
{
 stTagFile *p=(stTagFile *)item;
 return (void *)p->file;
}

/*void *TTagFiles::readItem(ipstream& is)
void TTagFiles::writeItem(void *obj, opstream& os)
const char * const TTagFiles::name="TTagFiles";
s(TTagFiles);*/

/*****************************************************************************
 TTagCollection and TSpTagCollection classes
 The TSpTagCollection is a simple collection.
 The TTagCollection is the collection plus all the stuff needed to handle the
TAG files.
*****************************************************************************/

int TSpTagCollection::compare(void *key1, void *key2)
{
 return strcasecmp((char *)key1,(char *)key2);
}

const char *TSpTagCollection::Languages[]=
{
 "Asm",  "ASP",    "Awk",   "BETA", "C",     "C++",
 "Cobol","Eiffel","Fortran","Java", "Lisp",  "Lua",
 "Make", "Pascal","Perl",   "PHP",  "Python","REXX",
 "Ruby", "Scheme","Sh",     "SLang","Tcl",   "Vim",
 "YACC",  NULL
};

const int
 ttclAsm=0,  ttclASP=1,    ttclAwk=2,    ttclBETA=3,  ttclC=4,      ttclCpp=5,
 ttclCobol=6,ttclEiffel=7, ttclFortran=8,ttclJava=9,  ttclLisp=10,  ttclLua=11,
 ttclMake=12,ttclPascal=13,ttclPerl=14,  ttclPHP=15,  ttclPython=16,ttclREXX=17,
 ttclRuby=18,ttclScheme=19,ttclSh=20,    ttclSLang=21,ttclTcl=22,   ttclVim=23,
 ttclYACC=24,
 ttclUnknown=255;

static stTagKind Asm[]={{'d',"define"},{'l',"label"},{'m',"macro"}};
static stTagKind ASP[]={{'f',"function"},{'s',"subroutine"}};
static stTagKind Func[]={{'f',"function"}};
static stTagKind BETA[]={{'f',"fragment definition"},{'p',"all pattern"},
                         {'s',"slot"},{'v',"pattern"}};
static stTagKind Cpp[]={{'c',"class"},{'d',"macro definition"},{'e',"enumerator"},
                        {'f',"function definition"},{'g',"enumeration name"},
                        {'m',"member (c, s, or u)"},{'n',"namespaces"},
                        {'p',"function prototype and declaration"},
                        {'s',"structure name"},{'t',"typedef"},
                        {'u',"union name"},{'v',"variable definition"},
                        {'x',"extern and forward variable declaration"}};
static stTagKind Cobol[]={{'p',"paragraph"}};
static stTagKind Eiffel[]={{'c',"class"},{'f',"feature"},{'l',"local entity"}};
static stTagKind Fortran[]={{'b',"block data"},{'c',"common block"},{'e',"entry point"},
                            {'f',"function"},{'i',"interface"},{'k',"type component"},
                            {'l',"label"},{'L',"local and common block variable"},
                            {'m',"module"},{'n',"namelist"},{'p',"program"},
                            {'s',"subroutine"},{'t',"derived type"},
                            {'v',"module variable"}};
static stTagKind Java[]={{'c',"class"},{'f',"field"},{'i',"interface"},{'m',"method"},
                         {'p',"package"}};
static stTagKind Make[]={{'m',"macro"}};
static stTagKind Pascal[]={{'f',"function"},{'p',"procedure"}};
static stTagKind Perl[]={{'p',"package"},{'s',"subroutine"}};
static stTagKind PHPPython[]={{'c',"class"},{'f',"function"}};
static stTagKind REXX[]={{'s',"subroutine"}}; // REXX
static stTagKind Ruby[]={{'c',"class"},{'f',"function"},{'m',"mixin"}};
static stTagKind Scheme[]={{'f',"function"},{'s',"set"}};
static stTagKind SLang[]={{'f',"function"},{'n',"namespace"}};
static stTagKind Tcl[]={{'p',"procedure"}};
static stTagKind YACC[]={{'l',"labels"}};

stTagKinds TSpTagCollection::Kinds[]=
{
 {3,Asm},{2,ASP},{1,Func},{4,BETA},{13,Cpp},{13,Cpp},
 {1,Cobol},{3,Eiffel},{14,Fortran},{5,Java},{1,Func},
 {1,Func},{1,Make},{2,Pascal},{2,Perl},{2,PHPPython},
 {1,REXX},{3,Ruby},{2,Scheme},{1,Func},{2,SLang},
 {1,Tcl},{1,Func},{1,YACC}
};

TSpTagCollection::TSpTagCollection(unsigned size) :
  TStringCollection(size,size/2)
{
 files=new TStringCollection(size/10,size/10);
 duplicates=True;
}

TSpTagCollection::~TSpTagCollection()
{
 destroy(files);
}

TTagCollection::TTagCollection() :
  TSpTagCollection(100)
{
 tagFiles=new TTagFiles();
}

TTagCollection::~TTagCollection()
{
 destroy(tagFiles);
}

void *TSpTagCollection::keyOf(void *item)
{
 stTag *p=(stTag *)item;
 return (void *)p->id;
}

void TSpTagCollection::freeItem(void *item)
{
 stTag *p=(stTag *)item;
 DeleteArray(p->id);
 DeleteArray(p->partof);
 if (!(p->flags & sttFgLine))
    DeleteArray(p->regex);
}

void TTagCollection::print1(void *item, void *)
{
 char b[120];
 getText(b,item,120);
 puts(b);
}

const char *TSpTagCollection::getLanguage(stTag *p)
{
 return p->lang==ttclUnknown ? "unknown" : Languages[p->lang];
}

const char *TSpTagCollection::getKind(stTag *p)
{
 if (p->lang!=ttclUnknown)
   {
    int count=Kinds[p->lang].count;
    stTagKind *k=Kinds[p->lang].kinds;
    int i;
    for (i=0; i<count; i++)
        if (k[i].kind==p->kind)
           return k[i].name;
   }
 return "unknown";
}

void TSpTagCollection::getText(char *buf, unsigned item, int maxLen)
{
 getText(buf,at(item),maxLen);
}

#define Advance maxLen-=aux; buf+=aux; if (maxLen<2) return

void TSpTagCollection::getText(char *buf, void *item, int maxLen)
{
 stTag *p=(stTag *)item;

 int aux=CLY_snprintf(buf,maxLen,"%s",p->id);
 Advance;

 aux=p->flags & sttFgPMask;
 if (aux!=0 && aux!=sttFgInherits)
   {
    char idP;
    switch (aux)
      {
       case sttFgStruct:
            idP='s';
            break;
       case sttFgClass:
            idP='c';
            break;
       case sttFgEnum:
            idP='e';
            break;
       case sttFgUnion:
            idP='u';
            break;
       default:
            idP='?';
      }
    aux=CLY_snprintf(buf,maxLen," (%s:%c)",p->partof,idP);
    Advance;
   }
 CLY_snprintf(buf,maxLen," [%s] %s %s",getKind(p),getLanguage(p),p->source);
}

#undef Advance

int TSpTagCollection::addValue(char *s, stTagFile *tf)
{
 stTag *p=new stTag;
 memset(p,0,sizeof(stTag));

 char *e=toTab(s);
 if (!*e)
   {
    delete p;
    return 1;
   }
 p->id=newStrN(s,e-s);
 s=e+1;
 e=toTab(s);
 if (!*e)
   {
    delete p;
    return 2;
   }
 *e=0;
 ccIndex pos;
 if (files->search(s,pos))
   {
    //printf("%s already there (%d)\n",s,pos);
    p->source=(char *)files->at(pos);
   }
 else
   {
    char *f=newStr(s);
    //printf("First time for: %s\n",f);
    p->source=f;
    files->insert(f);
   }
 s=e+1;
 if (!*s)
   {
    delete p;
    return 3;
   }
 if (ucisdigit(*s))
   {// Line number
    p->line=strtol(s,&e,10);
    p->flags|=sttFgLine;
   }
 else if (*s=='/')
   {
    s++;
    e=toSl(s);
    p->regex=newStrN(s+1,e-s-2);
    e++;
    //printf("Regex: %s\n",p->regex);
   }
 else
   {
    printf("What it means?: %s\n",s);
    delete p;
    return 4;
   }
 if (*e!=';' || e[1]!='"')
   {
    printf("Wrong format?: %s\n",e);
    delete p;
    return 5;
   }
 e+=2;
 if (*e=='\t') e++;
 for (s=e; *s && *s!='\r' && *s!='\n'; s++);
 *s=0;
 e=strtok(e,"\t");
 while (e)
   {
    if (strncmp(e,"kind:",5)==0)
      {
       p->kind=e[5];
      }
    else if (strncmp(e,"language:",9)==0)
      {
       int i;
       e+=9;
       for (i=0; Languages[i]; i++)
           if (strcmp(e,Languages[i])==0)
             {
              p->lang=i;
              break;
             }
       if (!Languages[i])
         {
          p->lang=ttclUnknown;
          printf("Warning! unknown language: <%s>\n",e);
         }
      }
    else if (strncmp(e,"class:",6)==0)
      {
       p->partof=newStr(e+6);
       p->flags|=sttFgClass;
      }
    else if (strncmp(e,"struct:",7)==0)
      {
       p->partof=newStr(e+7);
       p->flags|=sttFgStruct;
      }
    else if (strncmp(e,"enum:",5)==0)
      {
       p->partof=newStr(e+5);
       p->flags|=sttFgEnum;
      }
    else if (strncmp(e,"union:",6)==0)
      {
       p->partof=newStr(e+6);
       p->flags|=sttFgUnion;
      }
    else if (strncmp(e,"inherits:",9)==0)
      {
       p->partof=newStr(e+9);
       p->flags|=sttFgInherits;
      }
    else if (strncmp(e,"file:",5)==0)
      {
       p->flags|=sttFgLocal;
      }
    else if (strncmp(e,"implementation:",15)==0)
      {
       e+=15;
       if (strcmp(e,"virtual")==0)
          p->flags|=sttFgVirtual;
       else if (strcmp(e,"pure virtual")==0)
          p->flags|=sttFgPureVirtual;
       else if (strcmp(e,"abstract")==0)
          p->flags|=sttFgAbstract;
      }
    else
      {
       printf("Warning unknown attribute: %s\n",e);
      }
    e=strtok(NULL,"\t");
   }
 p->tagFile=tf;
 insert(p);
 return 0;
}

int TTagCollection::loadTagsFromFile(stTagFile *p)
{
 int ret=1;
 // Open the file
 FILE *f=fopen(p->file,"rt");
 if (f)
   {
    ret=0;

    struct stat st;
    fstat(fileno(f),&st);
    p->modtime=st.st_mtime;
    unsigned entries=0;
   
    // Read the data
    char *line=(char *)malloc(200);
    size_t size=200;
    while (!feof(f))
      {
       if (CLY_getline(&line,&size,f)!=-1)
         {
          if (strncmp(line,"!_TAG_",6)==0)
             p->info->addValue(line);
          else
            {
             if (addValue(line,p)==0)
                entries++;
            }
         }
      }
    ::free(line);
   
    // Close the file and insert the info
    fclose(f);
    p->entries=entries;
   }
 else
    p->entries=-1;
 return ret;
}

int TTagCollection::addFile(const char *file, int defer)
{
 ccIndex pos;
 if (tagFiles->search((void *)file,pos))
    return 0;

 // Create a structure for info about this file
 stTagFile *p=new stTagFile;
 p->file=newStr(file);

 char b[PATH_MAX],bfile[PATH_MAX],bdir[PATH_MAX];
 strcpy(b,file);
 CLY_fexpand(b);
 CLY_ExpandPath(b,bdir,bfile);
 p->base=newStr(bdir);
 p->info=new TTagInfo();
 p->entries=defer ? -2 : -1;

 //printf("f: %s\nb: %s\n",p->file,p->base);
 int ret=0;
 if (!defer)
    ret=loadTagsFromFile(p);
 tagFiles->insert(p);

 return ret;
}

void TTagCollection::refresh()
{
 ccIndex c=tagFiles->getCount(),i;
 for (i=0; i<c; i++)
    {
     stTagFile *p=tagFiles->atPos(i);
     // Failed to load?
     int reload=p->entries<0;
     if (!reload)
       {// Changed?
        struct stat st;
        if (stat(p->file,&st)==0)
           reload=difftime(p->modtime,st.st_mtime)!=0.0;
        if (reload)
          {// Delete all current entries
           deleteTagsFor(p);
          }
       }
     if (reload)
       {
        printf("Trying to reload %s\n",p->file);
        loadTagsFromFile(p);
       }
    }
}

void TTagCollection::deleteTagsFor(stTagFile *p)
{
 ccIndex in=0;
 while (in<count)
   {
    stTag *tg=atPos(in);
    if (tg->tagFile==p)
       atFree(in);
    else
       in++;
   }
}

TStringCollection *TTagCollection::getTagFilesList()
{
 ccIndex c=tagFiles->getCount(),i;
 TStringCollection *fileC=new TStringCollection(c,1);
 for (i=0; i<c; i++)
    {
     stTagFile *p=tagFiles->atPos(i);
     fileC->insert((void *)p->file);
    }
 return fileC;
}

/*****************************************************************************
 TTagClassCol
*****************************************************************************/

TTagClassCol::TTagClassCol(TSpTagCollection *from) :
  TStringCollection(20,10)
{
 ccIndex c=from->getCount(),i;
 for (i=0; i<c; i++)
    {
     stTag *p=from->atPos(i);
     if (p->kind=='c') // Class for most languages except Fortran
       {
        addClass(p);
       }
     /*else if (p->kind=='m' &&                            // member | method
              (p->lang==ttclCpp || p->lang==ttclJava) && // from C++ | Java
              (p->flags & sttFgPMask)==sttFgClass)       // from a class*/
     else if ((p->flags & sttFgPMask)==sttFgClass)
       {
        addMember(p);
       }
    }
}

void TTagClassCol::addClass(stTag *p)
{
 ccIndex pos;
 stClassTagInfo *cl;
 Boolean addParents=False;
 if (searchId(p->id,pos))
   {// Already there
    cl=atPos(pos);
    if (!cl->cl->source)
      {// That's a fake entry, replace using the real data
       deleteFake(cl->cl);
       cl->cl=p;
       addParents=True;
      }
   }
 else
   {// New class
    cl=newClass();
    cl->cl=p;
    atInsert(pos,cl);
    addParents=True;
   }
 if (addParents && p->partof)
   {
    char *s=strdup(p->partof);
    char *tok=strtok(s,",");
    while (tok)
      {
       cl->parents->insert(newStr(tok));
       addChildTo(tok,p->id);
       tok=strtok(NULL,",");
      }
    ::free(s);
   }
}

void TTagClassCol::addChildTo(const char *parent, const char *child)
{
 stClassTagInfo *cl=getClassOrFake(parent);
 cl->childs->insert((void *)child);
}

stTag *TTagClassCol::newFake(const char *id)
{
 stTag *p=new stTag;
 p->source=NULL;
 p->id=newStr(id);
 return p;
}

void TTagClassCol::deleteFake(stTag *p)
{
 DeleteArray(p->id);
 delete p;
}

stClassTagInfo *TTagClassCol::newClass()
{
 stClassTagInfo *ret=new stClassTagInfo;
 ret->parents=new TStringCollection(2,2);
 ret->childs =new TStringCollection(2,2);
 ret->members=new TSpTagCollection(10);
 return ret;
}

stClassTagInfo *TTagClassCol::getClassOrFake(const char *id)
{
 ccIndex pos;
 stClassTagInfo *cl;
 if (searchId(id,pos))
   {// Already there
    cl=atPos(pos);
   }
 else
   {// Add a fake
    cl=newClass();
    cl->cl=newFake(id);
    atInsert(pos,cl);
   }
 return cl;
}

void TTagClassCol::addMember(stTag *p)
{
 const char *to=p->partof;
 if (!to)
    return;
 stClassTagInfo *cl=getClassOrFake(to);
 cl->members->insert(p);
}

void TTagClassCol::freeItem(void *item)
{
 stClassTagInfo *p=(stClassTagInfo *)item;
 destroy(p->parents);
 delete p->childs;
 delete p->members;
}

void *TTagClassCol::keyOf(void *item)
{
 stClassTagInfo *p=(stClassTagInfo *)item;
 return (void *)p->cl->id;
}

/*****************************************************************************
 Save and restore
*****************************************************************************/

const char tagsVersion=1;

int TTagCollection::save(fpstream& s)
{
 ccIndex count=tagFiles->getCount(),i;
 s << (char)tagsVersion << count;
 for (i=0; i<count; i++)
    {
     stTagFile *p=tagFiles->atPos(i);
     s.writeString(p->file);
    }
 return 0;
}

int TTagCollection::load(fpstream& s)
{
 char version;
 ccIndex count,i;
 s >> version >> count;
 for (i=0; i<count; i++)
    {
     char *file=s.readString();
     addFile(file,1);
     DeleteArray(file);
    }
 return 0;
}

int TagsSave(fpstream& s)
{
 if (InitTagsCollection()) return 1;
 return tags->save(s);
}

int TagsLoad(fpstream& s)
{
 destroy(tags);
 tags=new TTagCollection();
 return tags->load(s);
}

/*****************************************************************************
 TTagMembersCol
 Used to browse class members from this class and the parents.
*****************************************************************************/

TTagMembersCol::TTagMembersCol() :
  TSpTagCollection(10)
{
 levels=new TNSCollection(10,5);
}

TTagMembersCol::~TTagMembersCol()
{
 delete levels;
}

void TTagMembersCol::getText(char *dest, unsigned item, int maxLen)
{
 stTag *p=(stTag *)at(item);
 int level=(int)levels->at(item);
 if (level)
    CLY_snprintf(dest,maxLen,"%s (%s) %d",p->id,p->partof,level);
 else
    CLY_snprintf(dest,maxLen,"%s",p->id);
}

void TTagMembersCol::insert(stTag *tg, int level)
{
 atInsert(count,tg);
 levels->atInsert(levels->getCount(),(void *)level);
}

void TTagMembersCol::insertSorted(stTag *tg, int level)
{
 ccIndex  i;
 search((void *)tg->id,i);
 atInsert(i,tg);
 levels->atInsert(i,(void *)level);
}

void TTagMembersCol::collectFromOne(TSpTagCollection *cl, int level, Boolean sort)
{
 ccIndex c=cl->getCount(),i;
 if (sort)
    for (i=0; i<c; i++)
        insertSorted(cl->atPos(i),level);
 else
    for (i=0; i<c; i++)
        insert(cl->atPos(i),level);
}

void TTagMembersCol::collect(stClassTagInfo *p, TTagClassCol *clist,
                             int level, Boolean sort)
{
 collectFromOne(p->members,level,sort);
 ccIndex cParents, iParent;
 cParents=p->parents->getCount();
 for (iParent=0; iParent<cParents; iParent++)
    {
     ccIndex pos;
     if (clist->search((char *)p->parents->at(iParent),pos))
        collect(clist->atPos(pos),clist,level+1,sort);
    }
}

/*****************************************************************************
 User interface
*****************************************************************************/

static
int InitTagsCollection()
{
 if (tags) return 0;
 tags=new TTagCollection();
 if (!tags) return 1;
 tags->addFile("tags",1);
 return 0;
}

void TagsFreeMemory()
{
 destroy0(tags);
}

/*****************************************************************************
 List of tag files
*****************************************************************************/

static int Modified;

static
int DeleteItem(int which)
{
 tags->deleteTagsFor(tags->tagFiles->atPos(which));
 tags->tagFiles->atFree(which);
 Modified=1;
 return 1;
}

static
int OkApply(void)
{
 return 1;
}

static
int CancelConfirm(void)
{
 if (Modified)
    return messageBox(__("Do you want to discard the changes?"),mfWarning | mfYesButton | mfNoButton)==cmYes;
 return 1;
}

static
int AddNewItem(void)
{
 char buffer[PATH_MAX];
 strcpy(buffer,"tags*");

 if (GenericFileDialog(__("Select tags file"),buffer,0,hID_SelectTagFile)==cmCancel)
    return 0;

 // Check if we can add it as relative path
 int options=0;
 char *entered =strdup(buffer);
 char *relative=strdup(buffer);
 getcwd(buffer,PATH_MAX);
 if (CheckIfPathAbsolute(entered))
   {
    options=AbsToRelPath(buffer,relative,0);
    //printf("a: %s\nr: %s\n",entered,relative);
   }
 char *toUse;
 if (options)
   {
    int ret=messageBox(__("Use relative path for the file?"),
           mfYesButton | mfNoButton | mfConfirmation);
    if (ret==cmCancel)
      {
       free(entered);
       free(relative);
       return 0;
      }
    toUse=ret==cmYes ? relative : entered;
   }
 else
    toUse=entered;
 tags->addFile(toUse);
 free(entered);
 free(relative);
 return 1;
}

void EditTagFiles()
{
 if (InitTagsCollection()) return;
 tags->refresh();
 // Make a copy of the list to allow reverting the actions
 TStringCollection *oldList=tags->getTagFilesList();

 TStringableListBoxRec boxParam;
 TDialogAID *d=CreateAddInsDelDialog(-1,-1,__("Tag files"),12,56,0);
 d->helpCtx=cmeTagFiles;
 boxParam.items=tags->tagFiles;
 boxParam.selection=0;
 d->DelAction=DeleteItem;
 d->AddAction=AddNewItem;
 d->OkAction =OkApply;
 d->CancelAction=CancelConfirm;
 Modified=0;

 if (execDialog(d,&boxParam)==cmCancel && Modified)
   {// Revert
    // 1) Delete all entries not found in the old list
    ccIndex i=0,pos;
    TTagFiles *tagFiles=tags->tagFiles;
    while (i<tagFiles->getCount())
      {
       stTagFile *p=tagFiles->atPos(i);
       if (!oldList->search((void *)p->file,pos))
         {// Not in the old list, remove it
          tags->deleteTagsFor(p);
          tagFiles->atFree(i);
         }
       else
          i++;
      }
    // 2) Insert all the files, the repeated will be rejected
    for (i=0; oldList->getCount(); i++)
        tags->addFile((char *)oldList->at(i),1);
   }

 delete oldList; // Not owner
}

/*****************************************************************************
 Jump to tag
*****************************************************************************/

class TLThings : public TGrowDialog
{
public:
 TLThings( TRect r, const char *name, int extraOptions=0 ) :
      TGrowDialog(r,name,extraOptions),
      TWindowInit( &TLThings::initFrame ) {};
 void handleEvent(TEvent& event);
};

void TLThings::handleEvent(TEvent& event)
{
 TDialog::handleEvent(event);
 if ( event.what == evCommand || event.what == evBroadcast)
   {
    switch ( event.message.command )
      {
       // a button
       /*case cmListItemSelected:
            endModal(event.message.command);
            clearEvent(event);
            break;*/
       case cmeZoom:
            event.message.command=cmZoom;
            TDialog::handleEvent(event);
            break;
       default:
           break;
      }
   }
}

class TTagsListBox : public TSortedListBox
{
public:
 TTagsListBox(const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar)
  : TSortedListBox(bounds,aNumCols,aScrollBar) {};
 TTagsListBox(const TRect& bounds, ushort aNumCols,
              TScrollBar *aHScrollBar, TScrollBar *aVScrollBar)
  : TSortedListBox(bounds,aNumCols,aHScrollBar,aVScrollBar) {};
 virtual void getText(char *dest, ccIndex item, short maxLen);
};

void TTagsListBox::getText(char *dest, ccIndex item, short maxLen)
{
 TSpTagCollection *p=(TSpTagCollection *)items;
 p->getText(dest,item,maxLen);
}

ListBoxSpecialize(TSTagsListBox);
ListBoxImplement(TagsListBox)

static
TDialog *createDialogTags(const char *title, const char *label,
                          const char *okLabel, const char *yesLabel=NULL)
{
 TLThings *d=new TLThings(TRect(1,1,1,1),title);
 TSViewCol *col=new TSViewCol(d);

 TRect r=TApplication::deskTop->getExtent();
 int h=r.b.y-r.a.y-10;
 int w=r.b.x-r.a.x-15;

 //TSStringableListBox *ListaH=new TSStringableListBox(w,h,tsslbVertical|tsslbHorizontal,1,256);
 TSTagsListBox *ListaH=new TSTagsListBox(w,h,tsslbVertical|tsslbHorizontal,1,256);

 ListaH->view->growMode=gfMoveBottomCorner;
 TSLabel *lista=new TSLabel(label,ListaH);

 col->insert(xTSLeft,yTSUp,lista);

 TSButton *ok=new TSButton(okLabel,cmOK,bfDefault);
 TSButton *cancel=new TSButton(__("Cancel"),cmCancel);
 ok->view->growMode=cancel->view->growMode=gfGrowAll;
 TSHzGroup *but123;

 if (yesLabel)
   {
    TSButton *yes=new TSButton(yesLabel,cmYes);
    yes->view->growMode=gfGrowAll;
    but123=MakeHzGroup(ok,yes,cancel,0);
   }
 else
    but123=MakeHzGroup(ok,cancel,0);

 col->insert(xTSCenter,yTSDown,but123);
 col->doItCenter(cmeSearchTag);
 delete col;
 return d;
}

static
TDialog *createDialog()
{
 return createDialogTags(__("Jump to symbol"),__("List of symbols"),__("O~K~"));
}

static
void JumpToTag(TListBoxRec &br)
{
 char b[PATH_MAX],desc[120];
 TSpTagCollection *tgs=(TSpTagCollection *)br.items;
 stTag *p=tgs->atPos(br.selection);
 CLY_snprintf(b,PATH_MAX,"%s%s",p->tagFile->base,p->source);
 tgs->getText(desc,p,120);
 if (p->flags & sttFgLine)
   GotoFileLine(p->line,b,desc);
 else
   GotoFileText((char *)p->regex,b,desc);
}

void SearchTag(char *word)
{
 if (InitTagsCollection()) return;
 tags->refresh();
 TListBoxRec br;
 br.items=tags;
 br.selection=0;

 if (word)
   {
    tags->search(word,br.selection);
    if (br.selection>=tags->getCount())
       br.selection=tags->getCount()-1;
    DeleteArray(word);
   }
 int ret=execDialog(createDialog(),&br);
 if (ret==cmOK)
    JumpToTag(br);
}

/*****************************************************************************
 Class Browser
*****************************************************************************/

class TClListBox : public TSortedListBox
{
public:
 TClListBox(const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar)
  : TSortedListBox(bounds,aNumCols,aScrollBar) {};
 TClListBox(const TRect& bounds, ushort aNumCols,
              TScrollBar *aHScrollBar, TScrollBar *aVScrollBar)
  : TSortedListBox(bounds,aNumCols,aHScrollBar,aVScrollBar) {};
 virtual void getText(char *dest, ccIndex item, short maxLen);
};

void TClListBox::getText(char *dest, ccIndex item, short maxLen)
{
 TTagClassCol *p=(TTagClassCol *)items;
 //p->getText(dest,item,maxLen);
 CLY_snprintf(dest,maxLen,"%s",p->atPos(item)->cl->id);
}

ListBoxSpecialize(TSClListBox);
ListBoxImplement(ClListBox)

static TDialog *createDialogCl()
{
 TLThings *d=new TLThings(TRect(1,1,1,1),__("Class list"));
 TSViewCol *col=new TSViewCol(d);

 TRect r=TApplication::deskTop->getExtent();
 int h=r.b.y-r.a.y-10;
 int w=r.b.x-r.a.x-25;

 //TSStringableListBox *ListaH=new TSStringableListBox(w,h,tsslbVertical|tsslbHorizontal,1,256);
 TSClListBox *ListaH=new TSClListBox(w,h,tsslbVertical|tsslbHorizontal,1,256);

 ListaH->view->growMode=gfMoveBottomCorner;
 TSLabel *lista=new TSLabel(__("List of classes"),ListaH);

 col->insert(xTSLeft,yTSUp,lista);

 TSButton *ok=new TSButton(__("~V~iew"),cmOK,bfDefault);
 TSButton *yes=new TSButton(__("~J~ump"),cmYes);
 TSButton *cancel=new TSButton(__("Cancel"),cmCancel);
 ok->view->growMode=cancel->view->growMode=yes->view->growMode=gfGrowAll;
 TSHzGroup *but123=MakeHzGroup(ok,yes,cancel,0);
 col->insert(xTSCenter,yTSDown,but123);
 col->doItCenter(cmeClassBrowser);
 delete col;
 return d;
}

const int cmThisClass=cmOK,
          cmThisAParents=cmYes,
          cmSorted=cmNo,
          cmExit=cmCancel;
const unsigned 
          cmParent=0x2000,
          cmChild =0x2001,
          cmNone  =0;

// This small trick saves me from creating an specialized TDialog
static unsigned commandCB;
static
int buttonsCB_VCl(unsigned command)
{
 commandCB=command;
 return btcbEndModal;
}

static
TDialog *createDialogVCl(stClassTagInfo *cl)
{
 TSViewCol *col=new TSViewCol(cl->cl->id);
 commandCB=cmNone;

 TSVeGroup *parents=NULL, *childs=NULL;
 if (cl->parents->getCount())
   {
    parents=MakeVeGroup(1 | tsveMakeSameW,
              new TSLabel(__("P~a~rents"),
                          new TSSortedListBox(20,6,tsslbVertical|tsslbHorizontal,1,80)),
              new TSButton(__("Browse ~p~arent"),cmParent,bfNormal,buttonsCB_VCl),0);
   }
 if (cl->childs->getCount())
   {
    childs=MakeVeGroup(1 | tsveMakeSameW,
              new TSLabel(__("Chil~d~s"),
                          new TSSortedListBox(20,6,tsslbVertical|tsslbHorizontal,1,80)),
              new TSButton(__("Browse ~c~hild"),cmChild,bfNormal,buttonsCB_VCl),0);
   }
 TSView *relations=NULL;
 if (parents && childs)
    relations=MakeHzGroup(parents,childs,0);
 else
    relations=parents ? parents : childs;

 TSVeGroup *grp=MakeVeGroup(tsveMakeSameW,
                            new TSStaticText(__("View")),
                            new TSButton(__("~T~his class"),cmThisClass,bfDefault),
                            new TSButton(__("This & ~P~arents"),cmThisAParents),
                            new TSButton(__("~S~orted"),cmSorted),
                            new TSButton(__("E~x~it"),cmExit),
                            relations,
                            0);
 col->insert(xTSCenter,yTSUp,grp);

 TDialog *d=col->doItCenter(cmeClassBrowser);
 delete col;
 return d;
}

static
int BrowseClassMembers(stClassTagInfo *cl)
{
 TListBoxRec b={cl->members,0};
 int ret=execDialog(createDialogTags(cl->cl->id,__("Members"),__("~J~ump")),&b);
 if (ret==cmOK)
   {
    JumpToTag(b);
    return cmCancel;
   }
 return cmOK;
}

static
int BrowseThisAParents(stClassTagInfo *cl, TTagClassCol *clist, Boolean sorted,
                       ccIndex &toView)
{
 TTagMembersCol *col=new TTagMembersCol();
 col->collect(cl,clist,0,sorted);
 TListBoxRec b={col,0};
 int ret=execDialog(createDialogTags(cl->cl->id,__("Members"),__("~J~ump"),
                    __("View ~C~lass")),&b);
 if (ret==cmOK)
   {
    JumpToTag(b);
    ret=cmCancel;
   }
 else if (ret==cmYes)
   {
    clist->searchId(col->atPos(b.selection)->partof,toView);
   }
 else
    ret=cmOK;
 delete col;
 return ret;
}

static
void BrowseClass(ccIndex index, TTagClassCol *clist)
{
 int ret;
 do
   {
    stClassTagInfo *cl=clist->atPos(index);
    struct
    {
     TListBoxRec b1;
     TListBoxRec b2;
    } box;
    TListBoxRec *next=&box.b1;
    TListBoxRec *parents=NULL, *childs=NULL;
    if (cl->parents->getCount())
      {
       box.b1.items=cl->parents;
       box.b1.selection=0;
       parents=&box.b1;
       next=&box.b2;
      }
    if (cl->childs->getCount())
      {
       next->items=cl->childs;
       next->selection=0;
       childs=next;
      }   
    ret=execDialog(createDialogVCl(cl),&box);
    printf("commandCB: %d\n",commandCB);
    printf("ret: %d\n",ret);
    if (commandCB==cmParent)
      {
       clist->search(cl->parents->at(parents->selection),index);
      }
    else if (commandCB==cmChild)
      {
       clist->search(cl->childs->at(childs->selection),index);
      }
    else
      {
       switch (ret)
         {
          case cmThisClass:
               ret=BrowseClassMembers(cl);
               break;
          case cmThisAParents:
               ret=BrowseThisAParents(cl,clist,False,index);
               break;
          case cmSorted:
               ret=BrowseThisAParents(cl,clist,True,index);
               break;
         }
      }
   }
 while (ret!=cmCancel);
}

static
void BrowseClasses(TListBoxRec &br, TTagClassCol *clist)
{
 int ret;
 do
   {
    ret=execDialog(createDialogCl(),&br);
    if (ret==cmYes)
       JumpToTag(br);
    if (ret==cmOK)
       BrowseClass(br.selection,clist);
   }
 while (ret==cmOK);
}

void TagsClassBrowser(char *word)
{
 if (InitTagsCollection()) return;
 tags->refresh();
 TTagClassCol *classList=new TTagClassCol(tags);

 TListBoxRec br;
 br.items=classList;
 br.selection=0;

 Boolean perfectMatch=False;
 if (word)
   {
    perfectMatch=classList->search(word,br.selection);
    if (br.selection>=classList->getCount())
       br.selection=classList->getCount()-1;
    DeleteArray(word);
   }
 if (perfectMatch)
    BrowseClass(br.selection,classList);
 else
    BrowseClasses(br,classList);

 destroy(classList);
}

