/* Copyright (C) 2003-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  This is a group of classes to handle TAG files as created by Exuberant
Ctags using format 2.
  The classes implements a sorted collection of tags from one or more tag
files.
  I use the following command line to generate the tags:

ctags -R --exclude="*.mak" --exclude=Makefile --exclude=".*.pl~" --fields=+i+l+m+z

TODO:
* make a SortedStringableListBox
* Itenationalize: void TTagFiles::getText(char *dest, unsigned item, int maxLen)
  and others

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
#define Uses_TScreen
#define Uses_AllocLocal

#define Uses_TCEditor_External // For mode constants
#define Uses_TCEditor_Commands // For the cmcJumpToFunction context

#define Uses_TSLabel
#define Uses_TSButton
#define Uses_TSHzGroup
#define Uses_TSStaticText
#define Uses_TSStringableListBox
#define Uses_TSVeGroup
#define Uses_TSSortedListBox
#define Uses_TSLabelRadio

#define Uses_TGrowDialog
#define Uses_TStringable
#define Uses_TDialogAID
#define Uses_FileOpenAid
#define Uses_TNoCaseStringCollection

#include <easydia1.h>
#include <ceditor.h>
#include <easydiag.h>

#include <tags.h>

#define Uses_SETAppConst
#define Uses_SETAppProject
#define Uses_SETAppVarious
#include <setapp.h>
#include <rhutils.h>
#include <pathtool.h>
#include <completi.h>
#include <advice.h>

static int InitTagsCollection();

// Variables for this module
static TTagCollection *tags=NULL;
static uint32   autoGenMode=0;
static char     forceRegen=0;

/* Uncomment it to get stats about the length of the id tags printed to stdout.
   I got this for SETEdit + TVision (may 5th 2003)
Max length for an Id: 48
Number of collected Ids: 28777
Accumulated length: 308819
Average length:  10.73
Distribution:
  1          457          1.59 *******
  2          367          1.28 *****
  3          580          2.02 *********
  4         1482          5.15 **********************
  5         1400          4.86 *********************
  6         1574          5.47 ************************
  7         1954          6.79 *****************************
  8         1978          6.87 ******************************
  9         1880          6.53 ****************************
 10         2610          9.07 ***************************************
 11         3214         11.17 ************************************************
 12         1937          6.73 *****************************
 13         2067          7.18 *******************************
 14         1567          5.45 ***********************
 15         1180          4.10 ******************
 16         1234          4.29 ******************
 17          824          2.86 ************
 18          600          2.08 *********
 19          401          1.39 ******
 20          293          1.02 ****
 21          351          1.22 *****
 22          235          0.82 ****
 23          161          0.56 **
 24           88          0.31 *
*/
#define TAG_STATS 0
#ifdef TAG_STATS
static unsigned maxLenIdTags, totalCantTags, lenAllIdTags;
static unsigned *idTagsLenTable;
#endif
static void JumpToTag(TListBoxRec &br, Boolean isClassCol=False);

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
char *toQuestion(char *e)
{
 for (; *e && *e!='?'; e++)
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
    CLY_snprintf(dest,maxLen,"%s [%s] %s",p->file,p->entries==-1 ? "error" : "not loaded",
                 p->flags & stfAutoGenerated ? "automatic" : "");
 else
    CLY_snprintf(dest,maxLen,"%s [%d entries] %s",p->file,p->entries,
                 p->flags & stfAutoGenerated ? "automatic" : "");
}

void TTagFiles::freeItem(void *item)
{
 stTagFile *p=(stTagFile *)item;
 DeleteArray(p->file);
 DeleteArray(p->base);
 CLY_destroy(p->info);
 delete p;
}

void *TTagFiles::keyOf(void *item)
{
 stTagFile *p=(stTagFile *)item;
 return (void *)p->file;
}

void TTagFiles::removeAutoGenerated()
{
 ccIndex i;
 for (i=0; i<count; )
    {
     stTagFile *p=atPos(i);
     if (p->flags & stfAutoGenerated)
        atFree(i);
     else
        i++;
    }
}

void TTagFiles::clearFlag(void *item, void *arg)
{
 uchar mask=*((unsigned *)arg);
 stTagFile *p=(stTagFile *)item;
 p->flags&= ~mask;
}

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
 "Cobol","Eiffel","Fortran","HTML", "Java", "Lisp",
 "Lua",  "Make", "Pascal","Perl",   "PHP",  "Python",
 "REXX", "Ruby", "Scheme","Sh",     "SLang","Tcl",
 "Vim",  "YACC",  NULL
};

const int
 ttclAsm=0,  ttclASP=1,    ttclAwk=2,    ttclBETA=3,  ttclC=4,      ttclCpp=5,
 ttclCobol=6,ttclEiffel=7, ttclFortran=8,ttclHTML=9,  ttclJava=10,  ttclLisp=11,
 ttclLua=12, ttclMake=13,  ttclPascal=14,ttclPerl=15, ttclPHP=16,   ttclPython=17,
 ttclREXX=18,ttclRuby=19,  ttclScheme=20,ttclSh=21,   ttclSLang=22, ttclTcl=23,
 ttclVim=24, ttclYACC=25,
 ttclUnknown=255;

static stTagKind Asm[]={{'d',"define"},{'l',"label"},{'m',"macro"}};
static stTagKind ASP[]={{'f',"function"},{'s',"subroutine"}};
static stTagKind Func[]={{'f',"function"}};
static stTagKind BETA[]={{'f',"fragment definition"},{'p',"all pattern"},
                         {'s',"slot"},{'v',"pattern"}};
static stTagKind Cpp[]={{'c',"class"},{'d',"macro definition"},{'e',"enumerator"},
                        {'f',"function definition"},{'g',"enumeration name"},
                        {'m',"member (c, s, or u)"},{'n',"namespaces"},
                        {'p',"function prot. and dec."},
                        {'s',"structure name"},{'t',"typedef"},
                        {'u',"union name"},{'v',"variable definition"},
                        {'x',"extern & forward var. dec."}};
static stTagKind Cobol[]={{'p',"paragraph"}};
static stTagKind Eiffel[]={{'c',"class"},{'f',"feature"},{'l',"local entity"}};
static stTagKind Fortran[]={{'b',"block data"},{'c',"common block"},{'e',"entry point"},
                            {'f',"function"},{'i',"interface"},{'k',"type component"},
                            {'l',"label"},{'L',"local and common block var"},
                            {'m',"module"},{'n',"namelist"},{'p',"program"},
                            {'s',"subroutine"},{'t',"derived type"},
                            {'v',"module variable"}};
static stTagKind HTML[]={{'a',"anchor"}};
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
 {1,Cobol},{3,Eiffel},{14,Fortran},{1,HTML},{5,Java},
 {1,Func},{1,Func},{1,Make},{2,Pascal},{2,Perl},
 {2,PHPPython},{2,PHPPython},{1,REXX},{3,Ruby},
 {2,Scheme},{1,Func},{2,SLang},{1,Tcl},{1,Func},
 {1,YACC}
};

TSpTagCollection::TSpTagCollection(unsigned size) :
  TStringCollection(size,size/2)
{
 files=new TStringCollection(size/10,size/10);
 duplicates=True;
}

TSpTagCollection::~TSpTagCollection()
{
 CLY_destroy(files);
}

TTagCollection::TTagCollection() :
  TSpTagCollection(100)
{
 tagFiles=new TTagFiles();
}

TTagCollection::~TTagCollection()
{
 CLY_destroy(tagFiles);
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
 delete p;
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
 if (((int)item)==-1)
   {
    *buf=EOS;
    return;
   }
 getText(buf,at(item),maxLen);
}

void TSpTagCollection::getTextType(char *buf, void *item, int maxLen)
{
 stTag *p=(stTag *)item;
 // Debug info uses | to show the point to replace the name
 char *pos=strchr((char *)p->type,'|');
 if (!pos)
   {
    CLY_snprintf(buf,maxLen,"%s",p->type);
    return;
   }
 int l,acum;
 // Copy all the text before |
 l=pos-p->type;
 if (l>=maxLen)
   {
    strncpy(buf,p->type,maxLen-1);
    buf[maxLen-1]=EOS;
    return;
   }
 strncpy(buf,p->type,l);
 acum=l;
 // Copy the name
 l=strlen(p->id);
 if (acum+l>=maxLen)
   {
    strncpy(buf+acum,p->id,maxLen-acum-1);
    buf[maxLen-1]=EOS;
    return;
   }
 strncpy(buf+acum,p->id,l);
 acum+=l;
 // Copy the rest
 pos++;
 l=strlen(pos);
 if (!l)
    return;
 if (acum+l>=maxLen)
   {
    strncpy(buf+acum,pos,maxLen-acum-1);
    buf[maxLen-1]=EOS;
    return;
   }
 strcpy(buf+acum,pos);
}

#define Advance maxLen-=aux; buf+=aux; if (maxLen<2) return

void TSpTagCollection::getText(char *buf, void *item, int maxLen)
{
 stTag *p=(stTag *)item;
 AllocLocalStr(preBuf,maxLen);

 int aux=p->flags & sttFgPMask;
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
    CLY_snprintf(preBuf,maxLen,"%s (%s:%c)",p->id,p->partof,idP);
   }
 else
   {
    CLY_snprintf(preBuf,maxLen,"%s",p->id);
   }
 CLY_snprintf(buf,maxLen,"%-36s [%-26s] %-6s %-40s %s",preBuf,getKind(p),getLanguage(p),
              p->source,p->type ? p->type : "");
}

#undef Advance

int TSpTagCollection::addValue(char *s, stTagFile *tf)
{
 stTag *p=new stTag;
 memset(p,0,sizeof(stTag));
 // If no language is specified assume that's C.
 // This is common for the objdump, in this case the language is unknown but
 // the kinds are reported like in C.
 p->lang=ttclC;

 char *e=toTab(s);
 if (!*e)
   {
    delete p;
    return 1;
   }
 p->id=newStrN(s,e-s);

 if (TAG_STATS)
   {
    unsigned l=e-s;
    if (l>maxLenIdTags)
      {
       idTagsLenTable=(unsigned *)realloc(idTagsLenTable,sizeof(unsigned)*(l+1));
       for (maxLenIdTags++; maxLenIdTags<l; maxLenIdTags++)
           idTagsLenTable[maxLenIdTags]=0;
       idTagsLenTable[maxLenIdTags]=1;
      }
    else
       idTagsLenTable[l]++;
    totalCantTags++;
    lenAllIdTags+=l;
   }

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
    // Exuberant Ctags doesn't use a real regex here.
    // It just puts /^*$/ where * is the content of the line.
    // So I just take this text and do a "whole word" search.
    p->regex=newStrN(s+1,e-s-2);
    e++;
    //printf("Regex: %s\n",p->regex);
   }
 else if (*s=='?')
   { // Hmm... this is wrong, how to implement it?
     // But may be isn't wrong because that's also fake.
    s++;
    e=toQuestion(s);
    p->regex=newStrN(s+1,e-s-2);
    e++;
    //printf("Backwards Regex: %s\n",p->regex);
   }
 else
   {
    printf("What it means?: %s\n",s);
    delete p;
    return 4;
   }
 p->tagFile=tf;
 if (*e!=';' || e[1]!='"')
   {// Most probably that's a format 1 (original ctags) file
    insert(p);
    return 0;
   }
 e+=2;
 if (*e=='\t') e++;
 for (s=e; *s && *s!='\r' && *s!='\n'; s++);
 *s=0;
 e=strtok(e,"\t");
 while (e)
   {
    if (!e[1]) // The kind: can be implicit
      {
       p->kind=*e;
      }
    else if (strncmp(e,"kind:",5)==0)
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
    else if (strncmp(e,"type:",5)==0)
      {
       p->type=newStr(e+5);
      }
    else if (strncmp(e,"access:",7)==0)
      {
       // Ignore it. Is language specific and not yet handled.
      }
    else if (strncmp(e,"register:",9)==0)
      {// Generated by objdump: optimized to a register
       p->flags|=sttFgRegister;
      }
    else if (strncmp(e,"value:",6)==0)
      {
       // Ignore it. It is generated by objdump and contains the value for
       // enum members.
      }
    else if (strncmp(e,"union class:",12)==0)
      {
       // Ignore it. It is generated by objdump and contains the name of
       // the "union class".
      }
    else
      {
       printf("Warning unknown attribute: %s\n",e);
      }
    e=strtok(NULL,"\t");
   }
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
    int varsFound=0;
    while (!feof(f))
      {
       if (CLY_getline(&line,&size,f)!=-1)
         {
          if (strncmp(line,"!_TAG_",6)==0)
            {
             p->info->addValue(line);
             varsFound++;
            }
          else
            {
             if (!varsFound)
               {
                messageBox(__("Wrong format for tags file!"),mfError | mfOKButton);
                break;
               }
             if (addValue(line,p)==0)
                entries++;
            }
         }
      }
    ::free(line);
   
    // Close the file and insert the info
    fclose(f);
    p->entries=entries;

    // Check if that's a FORMAT 2 Exuberant Ctags file.
    int format=0;
    ccIndex pos;
    if (p->info->search((void *)"FILE FORMAT",pos))
      {
       stTagInfo *info=p->info->atPos(pos);
       format=atoi(info->value);
      }
    if (!abortInit && format!=2 && GiveAdvice(gadvTagsOld)==cmYes)
      {
       abortInit=1;
       ShowHelpTopic("setedit","TAGS files");
      }
   }
 else
    p->entries=-1;
 return ret;
}

int TTagCollection::addFile(const char *file, int defer, uchar specialFlags)
{
 ccIndex pos;
 if (tagFiles->search((void *)file,pos))
   {
    stTagFile *p=tagFiles->atPos(pos);
    p->flags=specialFlags;
    return 0;
   }

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
 p->flags=specialFlags;

 //printf("f: %s\nb: %s\n",p->file,p->base);
 int ret=0;
 if (!defer)
    ret=loadTagsFromFile(p);
 tagFiles->insert(p);

 return ret;
}

static
int CheckForCTAGS(void)
{
 static int isCTAGSInstalled=0;

 if (!isCTAGSInstalled)
   {
    // We must rediret the error to avoid getting it in the stderr file
    char *err=open_stderr_out();
    TScreen::System("ctags --version");
    close_stderr_out();
    // Check what we got
    FILE *f=fopen(err,"r");
    int ok=0;
    if (f)
      {
       char resp[80];
       fgets(resp,80,f);
       fclose(f);
       ok=strstr(resp,"Exuberant Ctags")!=0;
      }
    unlink(err);

    if (ok)
       isCTAGSInstalled=1;
   }

 return isCTAGSInstalled;
}

int TTagCollection::refresh(Boolean advice)
{
 ccIndex c=tagFiles->getCount(),i;

 // Indicate we are busy
 Boolean oldBusy=TScreen::showBusyState(True);

 // Automatic central tags maintainance
 if (autoGenMode==stfAutoCentral)
    for (i=0; i<c; i++)
       {
        stTagFile *p=tagFiles->atPos(i);
        // Is that the central file?
        if (strcmp(p->file,"tags")==0)
          {
           if (forceRegen)
             {
              unlink(p->file);
              forceRegen=0;
             }
           struct stat st;
           int retStat=stat(p->file,&st);
           // Create a list with files newer than tags file
           char *lst=unique_name("tg");
           FILE *f=fopen(lst,"wt");
           if (f)
             {
              int ret;
              if (retStat)
                 ret=WriteNamesOfProjectTo(f,wnopLineSep);
              else
                 ret=WriteNamesOfProjectToTime(f,st.st_mtime,prjtTags);
              fclose(f);
              if (ret)
                {// Call ctags only if we have at least one newer
                 AllocLocalStr(buffer,36+strlen(lst));
                 sprintf(buffer,"ctags -a --fields=+i+l+m+z -L %s",lst);
                 TScreen::System(buffer);
                 ClearForceTargetBits(prjtTags);
                }
              unlink(lst);
             }
           string_free(lst);
           break;
          }
       }
 if (TAG_STATS)
   {
    maxLenIdTags=totalCantTags=lenAllIdTags=0;
    idTagsLenTable=(unsigned *)malloc(sizeof(unsigned));
    idTagsLenTable[0]=0;
   }
 abortInit=0;
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
        //printf("Trying to reload %s\n",p->file);
        loadTagsFromFile(p);
       }
    }

 // We finished the blocking stuff
 TScreen::showBusyState(oldBusy);

 if (advice && !abortInit && c==1) // Only one entry and ...
   {
    stTagFile *p=tagFiles->atPos(0);
    // and failed to load and is the default
    if (p->entries==-1 && strcmp(p->file,"tags")==0)
      {// Ask to RTFM
       if (GiveAdvice(gadvNoTags)==cmYes)
         {
          abortInit=1;
          ShowHelpTopic("setedit","TAGS files");
         }
       else
         {// Nope?
          if (CheckForCTAGS())
            {// Ask to generate a new one
             if (messageBox(__("I can try to generate a tag file, go ahead?"),
                 mfInformation | mfYesButton | mfNoButton)==cmYes)
               {// Try doing it
                TScreen::System("ctags -R --fields=+i+l+m+z");
                loadTagsFromFile(p);
               }
            }
          else
             // Not installed explain how to get it.
             messageBox(__("Install Exuberant Ctags, download it from http://ctags.sourceforge.net"), mfError | mfOKButton);
         }
      }
   }
 if (TAG_STATS)
   {
    printf("Max length for an Id: %d\n",maxLenIdTags);
    printf("Number of collected Ids: %d\n",totalCantTags);
    printf("Accumulated length: %d\n",lenAllIdTags);
    printf("Average length: %6.2f\n",lenAllIdTags/(double)totalCantTags);
    unsigned i;
    double maxPercent=0.0;
    for (i=0; i<=maxLenIdTags; i++)
       {
        double percent=idTagsLenTable[i]/(double)totalCantTags*100.0;
        if (percent>maxPercent)
           maxPercent=percent;
       }
    double scale=48.0/maxPercent;
    printf("Distribution:\n");
    for (i=0; i<=maxLenIdTags; i++)
       {
        double percent=idTagsLenTable[i]/(double)totalCantTags*100.0;
        printf("%3d\t%8d\t%6.2f",i,idTagsLenTable[i],percent);
        unsigned dots=unsigned(scale*percent+0.5);
        putc(' ',stdout);
        while (dots--)
           putc('*',stdout);
        putc('\n',stdout);
       }
   }
 return abortInit;
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
  TNoCaseStringCollection(20,10)
  //TStringCollection(20,10)
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
 CLY_destroy(p->parents);
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

const char tagsVersion=2;

int TTagCollection::save(fpstream& s)
{
 ccIndex count=tagFiles->getCount(),i;
 s << (char)tagsVersion;
 ccIndex saveCount=0;
 // Don't save auto-maintained files.
 for (i=0; i<count; i++)
    {
     stTagFile *p=tagFiles->atPos(i);
     if (!(p->flags & stfAutoGenerated))
        saveCount++;
    }
 s << saveCount;
 for (i=0; i<count; i++)
    {
     stTagFile *p=tagFiles->atPos(i);
     if (!(p->flags & stfAutoGenerated))
        s.writeString(p->file);
    }
 s << (uchar)autoGenMode;
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
 if (version>1)
   {
    uchar aux;
    s >> aux;
    autoGenMode=aux;
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
 CLY_destroy(tags);
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
 int level=(long)levels->at(item);
 if (level)
    CLY_snprintf(dest,maxLen,"%s (%s) %d",p->id,p->partof,level);
 else
    CLY_snprintf(dest,maxLen,"%s",p->id);
}

void TTagMembersCol::insert(stTag *tg, int level)
{
 atInsert(count,tg);
 levels->atInsert(levels->getCount(),(void *)(long)level);
}

void TTagMembersCol::insertSorted(stTag *tg, int level)
{
 ccIndex  i;
 search((void *)tg->id,i);
 atInsert(i,tg);
 levels->atInsert(i,(void *)(long)level);
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
 stTagFile *p=tags->tagFiles->atPos(which);
 if (p->flags & stfAutoGenerated)
   {
    messageBox(__("This file is automatically maintained"),mfError|mfOKButton);
    return 0;
   }
 tags->deleteTagsFor(p);
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

static
int ShowVars(int which)
{
 TDialog *d=CreateChooseDialog(0,0,__("Variables"),7,60,aidStringable|aidNoCancel);
 stTagFile *p=tags->tagFiles->atPos(which);
 TStringableListBoxRec box={p->info,0};
 execDialog(d,&box);
 return 0;
}

void EditTagFiles()
{
 if (InitTagsCollection()) return;
 // Note: avoid asking to generate the tags file here
 if (tags->refresh(False)) return;
 // Make a copy of the list to allow reverting the actions
 TStringCollection *oldList=tags->getTagFilesList();

 TStringableListBoxRec boxParam;
 TDialogAID *d=CreateAddInsDelDialog(-1,-1,__("Tag files"),12,56,aidInfo);
 d->helpCtx=cmeTagFiles;
 boxParam.items=tags->tagFiles;
 boxParam.selection=0;
 d->DelAction=DeleteItem;
 d->AddAction=AddNewItem;
 d->OkAction =OkApply;
 d->CancelAction=CancelConfirm;
 d->InfoAction=ShowVars;
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

class TTagsListBox : public TSortedListBox
{
public:
 TTagsListBox(const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar)
  : TSortedListBox(bounds,aNumCols,aScrollBar) {};
 TTagsListBox(const TRect& bounds, ushort aNumCols,
              TScrollBar *aHScrollBar, TScrollBar *aVScrollBar,
              Boolean aCenterOps=False)
  : TSortedListBox(bounds,aNumCols,aHScrollBar,aVScrollBar,aCenterOps) {};
 virtual void getText(char *dest, ccIndex item, short maxLen);
};

void TTagsListBox::getText(char *dest, ccIndex item, short maxLen)
{
 TSpTagCollection *p=(TSpTagCollection *)items;
 p->getText(dest,item,maxLen);
}

ListBoxSpecialize(TSTagsListBox);
ListBoxImplement(TagsListBox);

static
TDialog *createDialogTags(const char *title, const char *label,
                          const char *okLabel, const char *yesLabel=NULL)
{
 TGrowDialogZ *d=new TGrowDialogZ(TRect(1,1,1,1),title);
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
 return createDialogTags(__("Jump to symbol"),__("List of symbols"),__("O~K~"),
                         __("~S~earch members"));
}

static
void JumpToTag(TListBoxRec &br, Boolean isClassCol)
{
 char b[PATH_MAX],desc[120];
 stTag *p;
 TSpTagCollection *tgs;

 if (isClassCol)
   {// TTagClassCol aren't TSpTagCollection childs
    TTagClassCol *clcol=(TTagClassCol *)br.items;
    stClassTagInfo *info=clcol->atPos(br.selection);
    p=info->cl;
    tgs=info->members;
   }
 else
   {// The rest are TSpTagCollection or childs
    tgs=(TSpTagCollection *)br.items;
    p=tgs->atPos(br.selection);
   }
 if (CLY_IsValidDirSep(p->source[0]))
    CLY_snprintf(b,PATH_MAX,"%s",p->source);
 else
    CLY_snprintf(b,PATH_MAX,"%s%s",p->tagFile->base,p->source);
 if (p->type)
    tgs->getTextType(desc,p,120);
 else
    tgs->getText(desc,p,120);
 if (!p->line)
   {
    messageBox(__("Unknown line"),mfError|mfOKButton);
    p->line=1;
   }
 GPushCursorPos();
 if (p->flags & sttFgLine)
   GotoFileLine(p->line,b,desc);
 else
   GotoFileText((char *)p->regex,b,desc);
}

static
int ShowMembers(ccIndex pos)
{
 stTag *p=tags->atPos(pos);
 if (((p->kind=='e'|| p->kind=='u' || p->kind=='s') &&
      (p->lang==ttclC || p->lang==ttclCpp)) ||
     (p->kind=='c' && p->lang!=ttclFortran))
   {
    uchar mask=0;
    switch (p->kind)
      {
       case 'c':
            mask=sttFgClass;
            break;
       case 'e':
            mask=sttFgEnum;
            break;
       case 'u':
            mask=sttFgUnion;
            break;
       case 's':
            mask=sttFgStruct;
            break;
      }
    ccIndex c=tags->getCount(), i;
    TSpTagCollection *members=new TSpTagCollection(10);
    for (i=0; i<c; i++)
       {
        stTag *it=tags->atPos(i);
        if ((it->flags & sttFgPMask)==mask && strcmp(p->id,it->partof)==0)
           members->insert(it);
       }
    if (members->getCount())
      {
       TListBoxRec b={members,0};
       int ret=execDialog(createDialogTags(p->id,__("Members"),__("~J~ump")),&b);
       if (ret==cmOK)
         {
          JumpToTag(b);
          delete members;
          return 1;
         }
      }
    else
       messageBox(__("Sorry, couldn't find any member"),mfError|mfOKButton);
    delete members;
   }
 else
   {
    messageBox(__("I don't know any members for this type"),mfError|mfOKButton);
    return 0;
   }
 return 0;
}

int SearchTag(char *word)
{
 if (InitTagsCollection() ||
     tags->refresh() ||
     !tags->getCount()) return 0;

 TListBoxRec br;
 br.items=tags;
 br.selection=0;
 int perfectMatch=0;

 if (word)
   {
    if (tags->search(word,br.selection))
      {
       ccIndex pos=br.selection;
       char *id=(char *)tags->keyOf(tags->at(pos));
       if (strcmp(word,id)==0)
         {// Full match, is the only one?
          pos++;
          if (pos<tags->getCount())
            {
             id=(char *)tags->keyOf(tags->at(pos));
             //if (strncmp(word,id,strlen(word))!=0)
             if (strcmp(word,id)!=0)
                perfectMatch=1;
            }
         }
      }
    else
      {
       if (br.selection>=tags->getCount())
          br.selection=tags->getCount()-1;
      }
    DeleteArray(word);
    if (perfectMatch)
      {
       JumpToTag(br);
       return 1;
      }
   }

 int ret;
 do
   {
    ret=execDialog(createDialog(),&br);
    if (ret==cmYes && ShowMembers(br.selection))
       break;
   }
 while (ret==cmYes);
 if (ret==cmOK)
   {
    JumpToTag(br);
    return 1;
   }
 return 0;
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
            TScrollBar *aHScrollBar, TScrollBar *aVScrollBar,
            Boolean aCenterOps)
  : TSortedListBox(bounds,aNumCols,aHScrollBar,aVScrollBar,aCenterOps) {};
 virtual void getText(char *dest, ccIndex item, short maxLen);
};

void TClListBox::getText(char *dest, ccIndex item, short maxLen)
{
 if (item<0)
   {
    *dest=0;
    return;
   }
 TTagClassCol *p=(TTagClassCol *)items;
 //p->getText(dest,item,maxLen);
 CLY_snprintf(dest,maxLen,"%s",p->atPos(item)->cl->id);
}

ListBoxSpecialize(TSClListBox);
ListBoxImplement(ClListBox);

static TDialog *createDialogCl()
{
 TGrowDialogZ *d=new TGrowDialogZ(TRect(1,1,1,1),__("Class list"));
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
int buttonsCB_VCl(unsigned command, void *)
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
              new TSLabel(__("Chil~d~ren"),
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
                            new TSButton(__("This & Pa~r~ents"),cmThisAParents),
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
Boolean BrowseClass(ccIndex index, TTagClassCol *clist)
{
 int ret;
 Boolean jumped=False;
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
               jumped=ret==cmCancel ? True : False;
               break;
          case cmThisAParents:
               ret=BrowseThisAParents(cl,clist,False,index);
               jumped=ret==cmCancel ? True : False;
               break;
          case cmSorted:
               ret=BrowseThisAParents(cl,clist,True,index);
               jumped=ret==cmCancel ? True : False;
               break;
         }
      }
   }
 while (ret!=cmCancel);
 return jumped;
}

static
void BrowseClasses(TListBoxRec &br, TTagClassCol *clist)
{
 int ret;
 do
   {
    ret=execDialog(createDialogCl(),&br);
    if (ret==cmYes)
       JumpToTag(br,True);
    if (ret==cmOK && BrowseClass(br.selection,clist))
       break;
   }
 while (ret==cmOK);
}

void TagsClassBrowser(char *word)
{
 if (InitTagsCollection()) return;
 if (tags->refresh()) return;
 TTagClassCol *classList=new TTagClassCol(tags);

 if (!classList->getCount())
   {
    CLY_destroy(classList);
    messageBox(__("Sorry, but I can't find any class."),mfError | mfOKButton);
    return;
   }

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

 CLY_destroy(classList);
}

/*****************************************************************************
 Word Completion
*****************************************************************************/

static inline
stTag *SearchTagFor(char *word, int lenW, ccIndex &pos)
{
 tags->search(word,pos);
 if (pos>=tags->getCount())
    return NULL;
 stTag *p=tags->atPos(pos);
 if (strncasecmp(p->id,word,lenW)!=0)
    return NULL;

 return p;
}

char *TagsWordCompletion(int x, int y, char *word)
{
 if (!word || InitTagsCollection()) return NULL;
 if (tags->refresh()) return NULL;
 if (!tags->getCount()) return NULL;

 // Search a tag that matches
 int lenW=strlen(word),tLen;
 ccIndex pos;
 stTag *p=SearchTagFor(word,lenW,pos);
 if (!p)
    return NULL;

 TStringCollection *list=new TNoCaseStringCollection(10,4);
 int len=0;
 while (strncasecmp(p->id,word,lenW)==0)
   {
    list->insert((char *)p->id);
    tLen=strlen(p->id);
    if (tLen>len)
       len=tLen;
    pos++;
    if (pos>=tags->getCount())
       break;
    p=tags->atPos(pos);
   }

 char *ret=CompletionChooseFromList(list,list->getCount(),len,x-lenW,y,0,lenW);
 delete list;

 return ret;
}

char *TagsWordCompletionClass(int x, int y, char *word)
{
 if (!word || InitTagsCollection()) return NULL;
 if (tags->refresh()) return NULL;
 if (!tags->getCount()) return NULL;

 // Search a tag that matches
 ccIndex pos, max=tags->getCount();
 if (!tags->search(word,pos))
    return NULL;

 // Found, now search a class
 stTag *p=tags->atPos(pos);
 while (p->kind!='c')
   {
    pos++;
    if (pos>=max)
       return NULL;
    p=tags->atPos(pos);
    if (strcmp(p->id,word))
       return NULL;
   }

 // Found a class. Go and collect the members
 TStringCollection *list=new TNoCaseStringCollection(10,4);
 ccIndex c=tags->getCount(),i;
 int len=0,cant=0,tLen=0;
 for (i=0; i<c; i++)
    {
     stTag *p=tags->atPos(i);
     if ((p->flags & sttFgPMask)==sttFgClass && strcmp(word,p->partof)==0)
       {
        list->insert((char *)p->id);
        cant++;
        tLen=strlen(p->id);
        if (tLen>len)
           len=tLen;
       }
    }

 char *ret=NULL;
 if (cant)
    ret=CompletionChooseFromList(list,cant,len,x,y,0);
 delete list;

 return ret;
}

/*****************************************************************************
 Tag file generation options
*****************************************************************************/

void SetTagFilesGenerationOptions()
{
 if (!IsPrjOpened())
   {
    messageBox(__("This option is only available when using projects"),
               mfInformation | mfOKButton);
    return;
   }
 if (InitTagsCollection()) return;
 TSViewCol *col=new TSViewCol(__("Tags options"));
 col->insert(xTSCenter,yTSUp,
             TSLabelRadio(__("Automatic generation"),__("~D~isabled"),
                          __("~U~sing central file"),0));
 EasyInsertOKCancel(col);
 TDialog *d=col->doItCenter(cmeTagsOps);
 delete col;
 unsigned ops=autoGenMode;
 if (execDialog(d,&ops)==cmOK)
   {
    autoGenMode=ops;
    if (autoGenMode==stfAutoCentral)
      {
       ProjectInsertAutoTagFiles(); // Ensure the central is included
      }
    else
      {
       if (messageBox(__("Remove the tags files that I maintained from the list of files to use?"),
           mfConfirmation|mfYesButton|mfNoButton)==cmYes)
          RemoveAutoGenerated();
       else
          tags->tagFiles->unmarkAutoGenerated();
      }
   }
}

void RemoveAutoGenerated()
{
 if (tags)
    tags->tagFiles->removeAutoGenerated();
}

void InsertAutoGenerated(const char *name)
{
 if (tags)
    tags->addFile(name,1,stfAutoGenerated);
}

uint32 GetAutoGenMode()
{
 return autoGenMode;
}

void TagsAutoRegen()
{
 if (tags)
   {
    forceRegen=1;
    tags->refresh(False);
   }
}

