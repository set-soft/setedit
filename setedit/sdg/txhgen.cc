/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*****************************************************************************

11/7/97
Wau! es impresionante!.

Ojo no libera la memoria, no puedo usarlo como subrutina, hay que adaptarlo
26/6/97: Estoy en los sig:
Ya hace listas asociativas, el problema es como hacerlas configurables, tengo
que agregar algo en el archivo de configuraci¢n.
---------
Hasta ahora genera bastante bien, habria que implementar la busqueda automatica
de funcion y prototipo. La idea es llamar a bufun y en cada comentario fijarse
cual es la 1er funcion debajo de el y tomar ese valor.
25/6/97:
El archivo .txh generado parece OK, compilado con: --no-validate anda ok.
El problema es que falta un Top y ademas que necesita un menu. La idea seria
poner las funciones en una coleccion (ya hecho) y luego volcarlo a un nodo
Top. Despues habria que unir los dos archivos en uno, con esto quedaria
bastante bueno.

*****************************************************************************/

#define Uses_stdio
#define Uses_string
#define Uses_ctype
#define Uses_stdlib
#define Uses_alloca
#define Uses_unistd
#define Uses_limits

#define Uses_fcntl
#define Uses_TNoCaseSOSStringCollection
#define Uses_TNCSAssociative
#define Uses_TNSSOSCol
#define Uses_TVCodePage
#include <settvuti.h>

#define Uses_TMLISDGDefs
#include <mli.h>
#include <txhgen.h>
#include <bufun.h>
#include <rhutils.h>

#define MAX_DEFS 16
#define MAX_LEN  12
#define MAX_ASSO 8

static char *Defs[MAX_DEFS];
static int   lDefs[MAX_DEFS];
static int   tDefs[MAX_DEFS];
static char *Used[MAX_DEFS];
static char *Ends[MAX_DEFS];
static char *Defaults[MAX_DEFS];
static char *DefaultsEnds[MAX_DEFS];
static char  SkipDefault[MAX_DEFS];
static char  AddedToAsoc[MAX_DEFS];

static char *AssoMenu[MAX_ASSO];
static char *AssoNode[MAX_ASSO];
static char *AssoExtra[MAX_ASSO];
static char  AssoVar[MAX_ASSO];
static TNCSAssociative *AssoObj[MAX_ASSO];

static char  Start[MAX_LEN];
static char  End[MAX_LEN];
static int   Definitions;
static int   Associations;
static char *Mask;
static stkHandler HandlerKey;
static int   Distinguish=1;
static char *StartMenu;
// The next 2 variables have Val at the end to avoid collisions with the
// Win32 API.
static char *EntryMenuVal;
static char *EndMenuVal;
static char *mAssoMain;
static char *mAssoRest;
static char *mMain;
static char *FileUnderProcess;
static FILE *TXHNodesFile;
static char *EndOfPar;
static char *CrossRef;
static char *DoubleA;
static char *BreakLine;
static stkHandler *ASCIIConv;
static int   Replaces;

int  TXHLine;

char *TXHErrors[] =
{
"Unknown error",                //0
"Can't open the file",          //1
"Can't find SectionStart",      //2
"Can't find SectionEnd",        //3
"Can't find any definition",    //4
"Can't find the code to emit",  //5
"Not enough memory",            //6
"Missing variables",            //7
"Can't find any section",       //8
"Syntax error",                 //9
"Too much associations",        //10
"Too much variables",           //11
"Too much replaces",            //12
"Incomplet GenMenu section",    //13
"Missing GenAssoMain section",  //14
"Missing GenAssoRest section",  //15
"Missing GenMain section",      //16
"Missing Commands section",     //17
"CommandLine without name",     //18
"Name without CommandLine, use NULL if none", // 19
"Can't find the end of the string", // 20
"Error reading the file",       // 21
"Can't create output file"      // 22
};
int TXHError;

char *TXHErrorSection[]=
{
"unknown part",
"loading the format file",
"generating the output file",
"interpreting a sLisp code"
};

int TXHGenSection;

// FunList is usually the list of functions. But that's configurable.
// In fact FunList is just the list of names that distinguish a function
// from another. That's variable 0 and all the associations are related
// to it.
static TNoCaseSOSStringCollection *FunList;
static TNoCaseSOSStringCollection *fList;
static TNoCaseSOSStringCollection *DefinedCommands;
static TNSSOSCol *ListOfConstants;
static TNSSOSCol *CommandLines;
static SOStack *stkAliasGlobal;

// The following vars controls the program from outside
int TXHKeepTemporal=1;              // 1=> don't delete the file with the functions code
TXHGetNextFileType TXHGetNextFile;  // Function to get the files
char *TXHFormatFile;                // Configuration file
char *TXHTempGenFile;               // Name of the temporal for nodes
char *TXHOutBaseName;               // No extentions here!
char *TXHFilesDir=NULL;             // Base for format files
void (*TXHPrintMessage)(char *s);

static void GenerateAssoc(TNCSAssociative *a, char *name, char *extra, FILE *f);
void OutPutString(char *s, char *e,FILE *f);

char *TXHSections[] =
{
 "Delimiters",
 "Variables",
 "Associations",
 "Replace",
 "GenNode",
 "GenMenu",
 "GenAssoMain",
 "GenAssoRest",
 "GenMain",
 "Commands",
 "DefinedCommands",
 "ASCIIConvert",
 "Configuration"
};
const int maxSections=sizeof(TXHSections)/sizeof(char *);

static  int GenerateCodeFor(char *mask,int (*checkUsedVar)(int),
                            void (*putVarText)(int,FILE*),
                            getVarFunction getVar,FILE *f);

static void CopyFile(char *origin, char *destination)
{
 char *buf;
 buf = new char[16*1024]; // Just the transfer buffer
 int count;
 FILE *f = fopen(origin,"rb");
 FILE *stream = fopen(destination,"w+b");
 while ((count = fread(buf,1,16*1024,f)) > 0)
 {
   fwrite(buf,1,count,stream);
 }
 fclose(stream);
 fclose(f);
 delete[] buf;
}

static char *MoveAfterEqual(char *s)
{
 for (; *s && *s!='=' && *s!='\n'; s++);
 if (*s && *s!='\n') s++;
 for (; *s && ucisspace(*s) && *s!='\n'; s++);
 return s;
}

static void GetUpTo(int len, char *s, char *d)
{
 int l;
 for (l=0; *s && *s!='\n' && *s!='\r' && l<len; s++,l++) d[l]=*s;
 d[l]=0;
}

static void ReplaceCRby0(char *s)
{
 for (; *s && *s!='\n' && *s!='\r'; s++);
 *s=0;
}

static int LookForSection(char *s)
{
 int i;

 for (i=0; i<maxSections; i++)
     if (strcmp(s,TXHSections[i])==0)
        return i;
 return -1;
}

/*static void CutEOL(char *b)
{
 int l=strlen(b);
 if (b[l-1]=='\n')
    l--;
 if (b[l-1]=='\r')
    l--;
 if (b[l-1]=='\"')
    l--;
 b[l]=0;
}*/

static char *NextLineOf(char *s)
{
 for (;*s && *s!='\n'; s++);
 if (*s)
    return s+1;
 return s;
}

static char *LoadGenerate(char *&fPos)
{
 long l,l1,l2;
 char *code,*Code,*s;

 code=fPos;
 do
   {
    code=NextLineOf(code);
    TXHLine++;
   }
 while (*code!='[' && *code);
 TXHLine--; // The last is counted by wasMoved

 l=code-fPos;
 Code=new char[l+1];
 if (!Code)
   {
    TXHError=6;
    return NULL;
   }
 for (l1=0,l2=0,s=fPos; l1<l; l1++)
     if (s[l1]=='#')
       {
        if (l1 && s[l1-1]=='\n')
           for (l1++; s[l1]!='\n' && l1<l; l1++);
        else
           Code[l2++]=s[l1];
       }
     else
       Code[l2++]=s[l1];
 Code[l2]=0;
 fPos=code;
 return Code;
}

static stkHandler CopyToStack(char *pos,char *&fPos)
{
 fPos=NextLineOf(pos);
 ReplaceCRby0(pos);
 return stkAliasGlobal->addStr(pos);
}

static char *CopyStrQuoted(char *pos, char *s)
{
 for (; *s && *s!='\"'; s++,pos++)
    {
     if (*s=='\\')
       {
        s++;
        switch (*s)
          {
           case 't':
                *pos='\t';
                break;
           case 'r':
                *pos='\r';
                break;
           case 'n':
                *pos='\n';
                break;
           case '"':
                *pos='"';
                break;
           case '\\':
                *pos='\\';
                break;
           // The rest are skipped
           default:
                pos--;
          }
       }
     else
       *pos=*s;
     if (*s=='\n')
        TXHLine++;
    }
 *pos=0;
 return s;
}

static stkHandler GetValueStk(char *&fPos,char &wasMoved)
{
 stkHandler stAux;
 int Size;
 char *pos;

 pos=MoveAfterEqual(fPos);
 if (*pos!='"')
   {
    wasMoved=1;
    return CopyToStack(pos,fPos);
   }
 for (fPos=++pos; *pos && *pos!='\"'; pos++)
    {
     if (*pos=='\\' && *(pos+1)) pos++;
    }
 if (!*pos)
   {
    TXHError=20;
    return stkNULL;
   }
 Size=pos-fPos+1;
 stAux=stkAliasGlobal->alloc(Size);
 if (stAux==stkNULL)
   {
    TXHError=6;
    return stAux;
   }
 fPos=CopyStrQuoted(stkAliasGlobal->GetStrOf(stAux),fPos);
 return stAux;
}

static stkHandler ExtractPair(char *&fPos, char &wasMoved)
{
 char *s=fPos,p,*start_name=fPos;
 stkHandler ret;

 if (GetValueStk(fPos,wasMoved)==stkNULL)
    return stkNULL;
 // Now take the name
 for(;*s && !ucisspace(*s) && *s!='='; s++);
 // Now s points to the end of the name
 p=*s; *s=0;
 ret=stkAliasGlobal->addStr(start_name);
 *s=p;
 return ret;
}

static char *DuplicateStr(char *&fPos, char &wasMoved)
{
 int Size;
 char *pos,*aux;

 pos=MoveAfterEqual(fPos);
 if (*pos!='"')
   {
    wasMoved=1;
    fPos=NextLineOf(pos);
    ReplaceCRby0(pos);
    return strdup(pos);
   }
 for (fPos=++pos; *pos && *pos!='\"'; pos++)
    {
     if (*pos=='\\' && *(pos+1)) pos++;
    }
 if (!*pos)
   {
    TXHError=20;
    return NULL;
   }
 Size=pos-fPos+1;
 aux=new char[Size];
 if (aux==NULL)
   {
    TXHError=6;
    return aux;
   }
 fPos=CopyStrQuoted(aux,fPos);
 return aux;
}

// free checks for NULL that's safe
#define RetError(a) { TXHError=a; delete file; return 1; }

int TXHReadSpects(char *name, SOStack *stk)
{
 FILE *f;
 char *pos;
 int StartFound=0,EndFound=0,GenerateFound=0;
 int Section,i,SpectCommand=1;
 stkHandler stAux;
 char *file=NULL,*fPos,auxV,wasMoved=0,*auxP;
 long fileLen;

 Replaces=0;
 TXHLine=1;
 Associations=Definitions=0;

 // Open the file and read it to memory
 f=fopen(name,"rt");
 if (!f) RetError(1)
 fseek(f,0,SEEK_END);
 fileLen=ftell(f);
 file=new char[fileLen+1];
 if (!file)
   {
    fclose(f);
    RetError(6)
   }
 fseek(f,0,SEEK_SET);
 fileLen=fread(file,1,fileLen,f);
 fclose(f);
 if (fileLen<=0)
    RetError(21)
 fPos=file;
 file[fileLen]=0;

 // Init Vars
 for (i=0; i<MAX_ASSO; i++)
    {
     AssoMenu[i]=AssoNode[i]=AssoExtra[i]=NULL;
     AssoObj[i]=NULL;
     AssoVar[i]=0;
    }
 Mask=NULL;
 StartMenu=EntryMenuVal=EndMenuVal=NULL;
 mAssoMain=mAssoRest=mMain=NULL;
 EndOfPar=CrossRef=BreakLine=NULL;
 ASCIIConv=NULL;

 // Search a section
 while (*fPos!='[' && *fPos)
   {
    fPos=NextLineOf(fPos);
    TXHLine++;
   }
 if (*fPos!='[') RetError(8)
 do
   {
    // Here we have the section name in b
    pos=strchr(fPos,']');
    if (!pos)
       RetError(9)
    auxV=*pos; *pos=0;
    if ((Section=LookForSection(fPos+1))<0)
       RetError(9)
    *pos=auxV;

    do
      {
       // Search a line with something
       do
         {
          if (wasMoved)
             wasMoved=0;
          else
             fPos=NextLineOf(fPos);
          TXHLine++;
         }
       while ((*fPos=='#' || ucisspace(*fPos)) && *fPos);
       if (*fPos=='[' || *fPos==0)
          break;

       switch (Section)
         {
          // Delimiters
          case 0:
               if (strncmp(fPos,"SectionStart",12)==0)
                 {
                  pos=MoveAfterEqual(fPos);
                  GetUpTo(11,pos,Start);
                  StartFound=strlen(Start);
                 }
               else
               if (strncasecmp(fPos,"SectionEnd",10)==0)
                 {
                  pos=MoveAfterEqual(fPos);
                  GetUpTo(11,pos,End);
                  EndFound=strlen(End);
                 }
               else
                 RetError(9)
               break;

          // Variables
          case 1:
               if (strncmp(fPos,"AddDefinition",13)==0)
                 {
                  if (Definitions==MAX_DEFS)
                     RetError(11)
                  pos=MoveAfterEqual(fPos);
                  fPos=NextLineOf(pos); wasMoved=1;
                  ReplaceCRby0(pos);
                  strtok(pos,",");
                  Defs[Definitions]=strdup(pos);
                  lDefs[Definitions]=strlen(pos);
                  pos=strtok(NULL,",");
                  if (pos)
                     tDefs[Definitions]=atoi(pos);
                  else
                     tDefs[Definitions]=1;
                  Definitions++;
                 }
               else
               if (strncmp(fPos,"Distinguish",11)==0)
                 {
                  pos=MoveAfterEqual(fPos);
                  Distinguish=atoi(pos);
                 }
               else
                 RetError(9)
               break;

          // Associations
          case 2:
               if (strncmp(fPos,"AddAssoc",8)==0)
                 {
                  if (Associations==MAX_ASSO)
                     RetError(10)
                  pos=MoveAfterEqual(fPos);
                  fPos=NextLineOf(pos); wasMoved=1;
                  ReplaceCRby0(pos);
                  strtok(pos,",");
                  AssoMenu[Associations]=strdup(pos);
                  pos=strtok(NULL,",");
                  if (!pos)
                     RetError(9)
                  AssoNode[Associations]=strdup(pos);
                  pos=strtok(NULL,",");
                  if (!pos)
                     RetError(9)
                  AssoVar[Associations]=atoi(pos);
                  pos=strtok(NULL,",");
                  if (pos)
                     AssoExtra[Associations]=strdup(pos);
                  AssoObj[Associations]=new TNCSAssociative(5,5,stk);
                  Associations++;
                 }
               else
                 RetError(9)
               break;

          // Replace
          case 3:
               if (strncmp(fPos,"Constant",8)!=0)
                  RetError(9)
               stAux=GetValueStk(fPos,wasMoved);
               ListOfConstants->insert(stAux);
               Replaces++;
               break;

          // GenNode
          case 4:
               Mask=LoadGenerate(fPos);
               if (!Mask)
                  RetError(6)
               wasMoved=1;
               GenerateFound=1;
               break;

          // GenMenu
          case 5:
               if (strncmp(fPos,"Start",5)==0)
                  StartMenu=DuplicateStr(fPos,wasMoved);
               else
               if (strncasecmp(fPos,"Entry",5)==0)
                  EntryMenuVal=DuplicateStr(fPos,wasMoved);
               else
               if (strncasecmp(fPos,"End",3)==0)
                  EndMenuVal=DuplicateStr(fPos,wasMoved);
               else
                  RetError(9)
               break;

          // GenAssoMain
          case 6:
               mAssoMain=LoadGenerate(fPos);
               if (!mAssoMain)
                  RetError(6)
               wasMoved=1;
               break;

          // GenAssoRest
          case 7:
               mAssoRest=LoadGenerate(fPos);
               if (!mAssoRest)
                  RetError(6)
               wasMoved=1;
               break;

          // GenMain
          case 8:
               mMain=LoadGenerate(fPos);
               if (!mMain)
                  RetError(6)
               wasMoved=1;
               break;

          // Commands
          case 9:
               if (strncmp(fPos,"EndOfPar",8)==0)
                  EndOfPar=DuplicateStr(fPos,wasMoved);
               else
               if (strncmp(fPos,"CrossRef",8)==0)
                  CrossRef=DuplicateStr(fPos,wasMoved);
               else
               if (strncmp(fPos,"Double@",7)==0)
                  DoubleA=DuplicateStr(fPos,wasMoved);
               else
               if (strncmp(fPos,"BreakLine",9)==0)
                  BreakLine=DuplicateStr(fPos,wasMoved);
               else
                  RetError(9)
               break;

          // DefinedCommands
          case 10:
               if ((stAux=ExtractPair(fPos,wasMoved))!=stkNULL)
                  DefinedCommands->insert(stAux);
               else
                  RetError(9)
               break;

          // ASCIIConvert
          case 11:
               if (!ASCIIConv)
                 {
                  ASCIIConv=new stkHandler[256];
                  for (i=0; i<256; i++)
                      ASCIIConv[i]=stkNULL;
                 }
/*               pos=MoveAfterEqual(fPos);
               auxP=fPos;
               fPos=NextLineOf(pos); wasMoved=1;
               ReplaceCRby0(pos);
               ASCIIConv[(unsigned char)*auxP]=stk->addStr(pos);*/
               auxP=fPos;
               ASCIIConv[(unsigned char)*auxP]=GetValueStk(fPos,wasMoved);
               break;

          // Configuration
          case 12:
               if (strncmp(fPos,"CommandLine",11)==0)
                 {
                  if (!SpectCommand)
                     RetError(18)
                  GetValueStk(fPos,wasMoved);
                  SpectCommand=0;
                 }
               else
               if (strncmp(fPos,"Name",4)==0)
                 {
                  if (SpectCommand)
                     RetError(19)
                  CommandLines->insert(GetValueStk(fPos,wasMoved));
                  SpectCommand=1;
                 }
               else
                  RetError(9)
               break;
         }
       if (TXHError)
         {
          delete file;
          return 1;
         }
      }
    while (*fPos);
   }
 while (*fPos);

 delete file;
 if (!StartFound)
   {
    TXHError=2;
    return 1;
   }
 if (!EndFound)
   {
    TXHError=3;
    return 1;
   }
 if (!Definitions)
   {
    TXHError=4;
    return 1;
   }
 if (!GenerateFound)
   {
    TXHError=5;
    return 1;
   }
 if (!StartMenu || !EntryMenuVal || !EndMenuVal)
   {
    TXHError=13;
    return 1;
   }
 if (!mAssoMain)
   {
    TXHError=14;
    return 1;
   }
 if (!mAssoRest)
   {
    TXHError=15;
    return 1;
   }
 if (!mMain)
   {
    TXHError=16;
    return 1;
   }
 if (!EndOfPar || !CrossRef)
   {
    TXHError=17;
    return 1;
   }
/* for (i=0; i<256; i++)
    {
     if (ASCIIConv[i]!=stkNULL)
        printf("\"%s\", // %d\n",stk->GetStrOf(ASCIIConv[i]),i);
     else
        printf("NULL, // %d\n",i);
   }*/
 return 0;
}
#undef RetError

static char *SkipThisLine(char *s)
{
 for (; *s && *s!='\n' && *s!='\r'; s++);
 if (*s)
    for (; *s && ucisspace(*s); s++);
 return s;
}

char *TakeNum(char *s,int *num)
{
 int val=0;

 if (!*s)
    return s;
 if (ucisdigit(*s))
   {
    val=*s-'0';
    s++;
    if (ucisdigit(*s))
      {
       val=val*10+*s-'0';
       s++;
      }
   }
 *num=val;
 return s;
}

static int IsUsed(int n)
{
 if (n==100)
    return 1;
 if (n>=MAX_DEFS)
    return 0;
 return Used[n]!=NULL || (tDefs[n]==2 && Defaults[n]!=NULL && !SkipDefault[n]);
}


static int FixUpPointers(int num, char *&start, char *&end)
{
 char *s,*e;

 if (!IsUsed(num))
    return 0;

 if (Used[num])
   {
    s=Used[num];
    e=Ends[num];
   }
 else
   {
    s=Defaults[num];
    e=DefaultsEnds[num];
   }

 // Skip blanks
 if (!(SkipDefault[num] && tDefs[num]!=2))
   {
    for (s++;*s && s<e && ucisspace(*s); s++);
    for (--e;e>s && *e!='\n'; e--);
    for (;e>s && ucisspace(*e); e--);
    if (e==s)
       return 0;
   }

 start=s;
 end=e;
 return 1;
}

static char *XrefPar[2];

static int CheckUsedXref(int num)
{
 if (num>=0 && num<2)
    return XrefPar[num]!=NULL;
 return 0;
}

static void PutXrefCode(int num,FILE *f)
{
 if (num>=0 && num<2)
    fputs(XrefPar[num],f);
}

static int GetVarXref(int num,char *&start,char *&end,int &flags)
{
 if (CheckUsedXref(num))
   {
    start=XrefPar[num];
    end=0;
    flags=sdgSimpleString | sdgASCIIZ;
    return 1;
   }
 return 0;
}

static int ParseXref(char *s, char *e,FILE *f)
{
 int l=e-s;
 int r=0;
 char *st,*pos,*s2,*e2,v,*saux;

 if (l<4)
    return 0;

 if (*(s+2)=='{')
   {
    for (st=s+3 ;l && *s!='}'; s++,r++,l--);
    if (*s=='}')
      {
       *s=0;
       pos=strstr(st,"::");
       if (!pos)
         {
          pos=strchr(st,',');
          if (!pos)
             XrefPar[1]=XrefPar[0]=st;
          else
            {
             l=pos-st;
             saux=(char *)alloca(l+1);
             strncpy(saux,pos,l);
             saux[l]=0;
             XrefPar[0]=saux;
             XrefPar[1]=pos+1;
            }
         }
       else
       if (pos==st)
         {
          l=strlen(st+2);
          if (Distinguish && FixUpPointers(Distinguish,s2,e2))
            {
             l+=5+e2-s2;
             saux=(char *)alloca(l);
             v=*(e2+1);
             *(e2+1)=0;
             sprintf(saux,"%s (%s)",st+2,s2);
             *(e2+1)=v;
             XrefPar[1]=saux;
            }
          else
            XrefPar[1]=st+2;
          XrefPar[0]=st+2;
         }
       else
         {
          *pos=0;
          saux=(char *)alloca(strlen(pos+2)+strlen(st)+5);
          sprintf(saux,"%s (%s)",pos+2,st);
          XrefPar[1]=saux;
          XrefPar[0]=pos+2;
          *pos=':';
         }
       GenerateCodeFor(CrossRef,CheckUsedXref,PutXrefCode,GetVarXref,f);
       *s='}';
      }
   }

 return r;
}

const int maxParameters=10;
typedef struct
{
 char *command;
 int num_param;
 char *parameters[maxParameters];
 int   lens[maxParameters];
} stParseCom;

stParseCom *GlobalParseCom;

static int CheckUsedPCom(int num)
{
 return (num<GlobalParseCom->num_param);
}

static void PutPComVal(int num, FILE *f)
{
 if (num<GlobalParseCom->num_param)
    OutPutString(GlobalParseCom->parameters[num],
                 GlobalParseCom->parameters[num]+GlobalParseCom->lens[num],
                 f);
//    fwrite(GlobalParseCom->parameters[num],GlobalParseCom->lens[num],1,f);
}

static int GetVarPCom(int num,char *&start,char *&end,int &flags)
{
 if (CheckUsedPCom(num))
   {
    start=GlobalParseCom->parameters[num];
    end=GlobalParseCom->parameters[num]+GlobalParseCom->lens[num];
    flags=0;
    return 1;
   }
 return 0;
}

static int ParseCommand(char *s, char *e, FILE *f)
{
 int l=e-s,Level;
 int r=0;
 char *p,*st;
 stParseCom pc,*old;
 ccIndex index;

 old=GlobalParseCom;
 GlobalParseCom=&pc;
 if (l<3)
    goto GoBack;
 // Start of name
 st=s+2;
 for (p=st; p<e && *p!='>'; p++)
     if (*p=='@' && *(p+1)=='>')
        p++;
 if (*p!='>')
    goto GoBack;
 l=p-st;
 pc.command=(char *)alloca(l+1);
 if (!pc.command)
    goto GoBack;
 for (p=pc.command; st<e && *st!='>'; st++,p++)
    {
     if (*st=='@' && *(st+1)=='>')
        st++;
     *p=*st;
    }
 *p=0;
 pc.num_param=0;
 p=st; p++;
 if (p<e && *p=='{')
   {
    do
      {
       p++;
       st=pc.parameters[pc.num_param]=p;
       Level=0;
       for (l=0; p<e; p++,l++)
          {
           if (*p=='@' && p+1<e)
             {
              p++; l++;
             }
           else
             if (*p=='{')
                Level++;
             else
                if (*p=='}')
                  {
                   if (Level==0)
                      break;
                   Level--;
                  }
                else
                  if (*p==',' && Level==0)
                     break;
          }
       pc.lens[pc.num_param++]=p-st-1;
      }
    while (*p!='}' && pc.num_param<maxParameters);
    p++;
   }
 r=p-s-1;

 // Now: is that a valid command?
 if (DefinedCommands->Search(pc.command,index))
   {
    stkHandler temp=(stkHandler)(DefinedCommands->at(index));
    temp=stkAliasGlobal->GetPreviousOf(temp);
    GenerateCodeFor(stkAliasGlobal->GetStrOf(temp),CheckUsedPCom,PutPComVal,GetVarPCom,f);
   }
 else
   // If isn't valid just let as-is
   fputs(pc.command,f);

GoBack:
 GlobalParseCom=old;
 return r;
}

void OutPutString(char *s, char *e,FILE *f)
{
 for (;s<=e && *s; s++)
    {
     switch (*s)
       {
        case '@':
             if (s==e)
                fputc(*s,f);
             else
             if (*(s+1)=='@')
               {
                if (DoubleA)
                   fputs(DoubleA,f);
                s++;
               }
             else
             if (*(s+1)=='{' || *(s+1)=='}')
               {
                s++;
                if (ASCIIConv[(unsigned char)*s]!=stkNULL)
                   fputs(stkAliasGlobal->GetStrOf(ASCIIConv[(unsigned char)*s]),f);
                else
                   fputc(*s,f);
               }
             else
             if (*(s+1)=='p' && !TVCodePage::isAlpha(*(s+2)))
               {
                fprintf(f,"\n%s\n",EndOfPar);
                s++;
               }
             else
             if (*(s+1)=='*')
               {
                if (BreakLine)
                   fputs(BreakLine,f);
                s++;
               }
             else
             if (*(s+1)=='x' && *(s+2)=='{')
                s+=ParseXref(s,e,f);
             else
             if (*(s+1)=='<')
                s+=ParseCommand(s,e,f);
             else
                fputc(*s,f);
             break;

        default:
             if (ASCIIConv[(unsigned char)*s]!=stkNULL)
                fputs(stkAliasGlobal->GetStrOf(ASCIIConv[(unsigned char)*s]),f);
             else
                fputc(*s,f);
       }
    }
}

static void TakeValueOf(int num,FILE *f)
{
 char *s,*e;

 if (!FixUpPointers(num,s,e))
    return;

 OutPutString(s,e,f);
}

static void FillAssociations(int num, char *var)
{
 char *s,*e,v;
 int i;

 if (num>=90)
   {
    for (i=0; i<Associations; i++)
        if (AssoVar[i]==num)
           AssoObj[i]->insert(var,HandlerKey);
   }
 else
   {
    if (!FixUpPointers(num,s,e))
       return;
   
    for (i=0; i<Associations; i++)
        if (AssoVar[i]==num)
          {
           v=*(e+1);
           *(e+1)=0;
           AssoObj[i]->insert(s,HandlerKey);
           *(e+1)=v;
          }
   }
}

inline
void AlignLen(int &len)
{
 if (len & 3)
    len+=4-(len & 3);
}

static char *SearchInfList(int line)
{
 ccIndex i,c;
 int l,min=1000000,dif;
 char *best=NULL;

 c=fList->getCount();
 for (i=0; i<c; i++)
    {
     char *s=fList->atStr(i);
     int length=strlen(s)+1;
     AlignLen(length);
     l=*(int *)(s+length);
     dif=abs(l-line);
     if (dif<min)
       {
        min=dif;
        best=s;
       }
    }

 return best;
}

static void AutoFill(int line)
{
 char *s,*p;
 // GCC says Class and Function can be used unitialized?!
 char *Class=0,*Function=0,*Prototype;
 int  lClass=0,lFunction=0,lPrototype=0;
 int l;

 if ((s=SearchInfList(line))!=NULL)
   {
    // Get the prototype (no ret value sorry), Function and Class names
    Prototype=s;
    lPrototype=strlen(s);
    s=strstr(Prototype,"::");
    if (s)
      {
       lClass=s-Prototype;
       Class=Prototype;
       s+=2;
      }
    else
      s=Prototype;
    p=strchr(s,'(');
    if (p)
      {
       lFunction=p-s;
       Function=s;
      }
    // Fill the variables
    for (l=0; l<Definitions; l++)
       {
        if (Used[l]==NULL)
          {
           switch (tDefs[l])
             {
              case 3:
                   Used[l]=Prototype;
                   Ends[l]=Prototype+lPrototype;
                   SkipDefault[l]=1;
                   break;
              case 4:
                   if (lClass)
                     {
                      Used[l]=Class;
                      Ends[l]=Class+lClass-1;
                      SkipDefault[l]=1;
                     }
                   break;
              case 5:
                   if (lFunction)
                     {
                      Used[l]=Function;
                      Ends[l]=Function+lFunction-1;
                      SkipDefault[l]=1;
                     }
                   break;
             }
          }
       }
   }
}


const int DistExtraLen=8;

/**[txh]********************************************************************

  Description:
  This function adds a new string to FunList but making sure it is a
different one.

***************************************************************************/

static
stkHandler AddDifferentStr(SOStack &stk, char *s, int l)
{
 ccIndex pos;
 char *aux=(char *)alloca(l+DistExtraLen);

 strcpy(aux,s);
 if (FunList->Search(s,pos))
   { // Change it until we find a non-used name
    int i=1;
    do
      {
       sprintf(aux,"%s <%d>",s,i);
      }
    while (++i<1000 && FunList->Search(aux,pos));
   }
 return stk.addStr(aux);
}

static void AddMain(SOStack &stk)
{
 char *s,*e;
 char *s2,*e2,*aux;

 if (FixUpPointers(0,s,e))
   {
    if (Distinguish && FixUpPointers(Distinguish,s2,e2))
      {
       int l1=e-s+1;
       int l2=e2-s2+1;
       int l=l1+l2+3+1;

       aux=(char *)alloca(l);
       strncpy(aux,s,l1);
       aux[l1]=0;
       strcat(aux," (");
       strncat(aux,s2,l2);
       aux[l-2]=0;
       strcat(aux,")");
       HandlerKey=AddDifferentStr(stk,aux,l);
      }
    else
      {
       int l=e-s+1+1;

       aux=(char *)alloca(l);
       strncpy(aux,s,l);
       aux[l-1]=0;
       HandlerKey=AddDifferentStr(stk,aux,l);
      }
    FunList->insert(HandlerKey);
   }
}

static void AddContent(void)
{
 int i,j,var;
 char *s1,*e1,*s2,*e2,v1,v2;

 for (i=0; i<Definitions; i++)
    {
     if (Used[i]!=NULL && tDefs[i]==6)
       {
        for (j=0; j<Associations; j++)
           {
            var=AssoVar[j];
            if (Used[var])
              {
               if (!FixUpPointers(i,s1,e1))
                  return;
               v1=*(e1+1);
               *(e1+1)=0;
               if (!FixUpPointers(var,s2,e2))
                  return;
               v2=*(e2+1);
               *(e2+1)=0;
               AssoObj[j]->SetContent(s2,s1);
               *(e1+1)=v1;
               *(e2+1)=v2;
               return;
              }
           }
       }
    }
}

static int CheckUsedVar(int num)
{
 return num<MAX_DEFS && IsUsed(num);
}

static int CheckUsedMain(int num)
{
 return num>0 && (num<6 || (num>=50 && num<50+Replaces));
}

#define CheckUsedAssoMain CheckUsedMenu
#define CheckUsedAssoRest CheckUsedMenu
static int CheckUsedMenu(int num)
{
 return num>0 && num<3;
}

static void PutVarText(int num,FILE *f)
{
 if (num==90)
    fputs(FileUnderProcess,f);
 else
 if (num==91)
    fprintf(f,"%d",TXHLine);
 else
 if (num==92)
   {
    #if 1
    fputs(stkAliasGlobal->GetStrOf(HandlerKey),f);
    #else
    TakeValueOf(0,f);
    if (Used[Distinguish])
      {
       fputs(" (",f);
       TakeValueOf(Distinguish,f);
       fputs(")",f);
      }
    #endif
   }
 else
    TakeValueOf(num,f);
}

static int GetVarText(int num,char *&start,char *&end,int &flags)
{
 if (num==90)
   {
    start=end=FileUnderProcess;
    flags=sdgSimpleString | sdgASCIIZ;
    return 1;
   }
 else
 if (num==91)
   {
    end=start=new char[26];
    sprintf(start,"%d",TXHLine);
    flags=sdgSimpleString | sdgASCIIZ;
    return 1;
   }
 else
 if (num==92)
   {
    char *s1,*e1;
    if (Used[Distinguish] && FixUpPointers(Distinguish,s1,e1))
      {
       char *s2,*e2;
       if (!FixUpPointers(0,s2,e2))
          return 0;
       int l=e1-s1+e2-s2+4;
       end=start=new char[l];
       strncpy(start,s2,e2-s2);
       strcat(start," (");
       strncat(start,s1,e1-s1);
       strcat(start,")");
       flags=sdgFreeIt | sdgSimpleString;
      }
    else
      {
       if (!FixUpPointers(0,start,end))
          return 0;
       flags=sdgSimpleString;
      }
    return 1;
   }
 else
   {
    if (!FixUpPointers(num,start,end))
       return 0;
    flags=sdgSimpleString;
   }
 return 1;
}

static char *Vs,*No,*Ex;

static void PutMenuVal(int num,FILE *f)
{
 switch (num)
   {
    case 1:
         fputs(Vs,f);
         break;
    case 2:
         fputs(No,f);
         if (Ex)
            fprintf(f," %s",Ex);
         break;
   }
}

static int GetVarMenu(int num,char *&start,char *&end,int &flags)
{
 flags=sdgSimpleString | sdgASCIIZ;
 switch (num)
   {
    case 1:
         end=start=Vs;
         return 1;
    case 2:
         if (Ex)
           {
            int l;
            char *s;
            l=strlen(No)+strlen(Ex)+2;
            s=new char[l];
            sprintf(s,"%s %s",No,Ex);
            flags|=sdgFreeIt;
           }
         else
            end=start=No;
         return 1;
   }
 return 0;
}

void GenMenuEntry(FILE *f, char *Visible, char *Node, char *extra=NULL)
{
 char *s1=Vs,*s2=No,*s3=Ex; // Funny thing, a menu entry could support a menu entry inside ;-P
 Vs=Visible; No=Node; Ex=extra;
 GenerateCodeFor(EntryMenuVal,CheckUsedMenu,PutMenuVal,GetVarMenu,f);
 fputc('\n',f);
 Vs=s1; No=s2; Ex=s3;
}

static void PutMainCode(int num,FILE *f)
{
 int i,l;
 char LineB[1000];

 switch (num)
   {
    // Main menu
    case 1:
         fprintf(f,"%s\n",StartMenu);
         GenMenuEntry(f,"Alphabetical list of functions","Alphabetical List");
         for (i=0; i<Associations; i++)
             GenMenuEntry(f,AssoMenu[i],AssoNode[i]);
         fprintf(f,"%s\n\n",EndMenuVal);
         break;

    // Name of the function list node
    case 2:
         fputs("Alphabetical List",f);
         break;

    // Menu for all the functions
    case 3:
         fprintf(f,"%s\n",StartMenu);
         i=FunList->getCount();
         for (l=0; l<i; l++)
             GenMenuEntry(f,FunList->atStr(l),FunList->atStr(l));
         fprintf(f,"%s\n\n",EndMenuVal);
         break;

    // All the associations code
    case 4:
         for (i=0; i<Associations; i++)
             GenerateAssoc(AssoObj[i],AssoNode[i],AssoExtra[i],f);
         break;

    // All the function nodes from the other file
    case 5:
         while (!feof(TXHNodesFile))
           { // That's strange to me, looks like feof is not enough here
            if (!fgets(LineB,999,TXHNodesFile))
               break;
            fputs(LineB,f);
           }
         break;

    default:
         if (num>=50)
           { // Values from section Replace
            num-=50;
            if (num<Replaces)
              {
               char *s=ListOfConstants->atStr(num);
               OutPutString(s,s+strlen(s),f);
              }
           }
   }
}

static int GetVarMain(int num,char *&start,char *&end,int &flags)
{
 switch (num)
   {
    // Name of the function list node
    case 2:
         start=end="Alphabetical List";
         flags=sdgSimpleString | sdgASCIIZ;
         return 1;

    default:
         if (num>=50)
           { // Values from section Replace
            num-=50;
            if (num<Replaces)
              {
               start=end=ListOfConstants->atStr(num);
               flags=sdgASCIIZ;
               return 1;
              }
           }
   }
 return 0;
}

static  int GenerateCodeFor(char *mask,int (*checkUsedVar)(int),
                            void (*putVarText)(int,FILE*),
                            getVarFunction getVar,FILE *f)
{
 int Level=0,OkLevel=0,num;
 char *s;
 
 for (s=mask; *s;)
    {
     if (*s=='~') // Is an special value?
       {
        s++;
        if (*s=='~') // Is a conditional?
          {
           s=TakeNum(++s,&num);
           for (; *s && *s!='{'; s++);
           if (*s) s++;
           if (Level==OkLevel && checkUsedVar(num))
              OkLevel++;
           Level++;
          }
        else
          if (*s=='}') // End of conditional
            {
             Level--;
             if (OkLevel>Level)
                OkLevel--;
             s++;
            }
          else
            if (*s=='(') // sLisp code
              {
               s=InterpretLispCode(s,getVar,f);
               if (!s)
                  return 1; // Ret with error
              }
            else
              { // A simple var
               s=TakeNum(s,&num);
               if (Level==OkLevel)
                  putVarText(num,f);
              }
       }
     else
       {
        if (Level==OkLevel)
           fputc(*s,f); // Not special put it
        s++;
       }
    }
 return 0;
}

int TXHGenerateFor(char *b,FILE *f,SOStack &stk)
{
 char *pos2=NULL,*st,*aux,*Next=b;
 int i,cual;
 int GenNode;
 TXHLine=1;

 for (i=0; i<Definitions; i++)
     Defaults[i]=NULL;
 do
   {
    // Search the start of the comment
    b=Next;
    st=strstr(b,Start);
    if (st)
      {
       // Search the end
       pos2=strstr(st+1,End);
       if (pos2)
         {
          for (aux=b;aux<=pos2; aux++)
              if (*aux=='\n')
                 TXHLine++;
          Next=aux;
          b=st;
          // Scan the comment to know what markers are used
          cual=-1;
          for (i=0; i<Definitions; i++)
             {
              Used[i]=NULL;
              SkipDefault[i]=0;
              AddedToAsoc[i]=0;
             }
          GenNode=1;
          do
            {
             b=SkipThisLine(b);
             for (i=0; i<Definitions; i++)
                 if (strncmp(Defs[i],b,lDefs[i])==0)
                   {
                    if (tDefs[i]==6)
                       GenNode=0;
                    if (cual>=0)
                      {
                       Ends[cual]=b;
                       if (!SkipDefault[cual])
                          DefaultsEnds[cual]=b;
                      }
                    b+=lDefs[i];
                    cual=i;
                    if (tDefs[i]==2)
                      {
                       if (strncmp(b+1,"~no",2)==0)
                          SkipDefault[i]=1;
                       else
                          if (strncmp(b+1,"~clear",5)==0)
                             Defaults[i]=NULL;
                          else
                             Defaults[i]=Used[i]=b;
                      }
                    else
                       Used[i]=b;
                    break;
                   }
            }
          while (b<pos2);
          if (cual>=0)
            {
             Ends[cual]=pos2;
             if (!SkipDefault[cual])
                DefaultsEnds[cual]=pos2;
            }
          if (GenNode)
            {
             // Process automatic fields
             AutoFill(TXHLine);
             // Add to main list
             AddMain(stk);
             for (i=0; i<Definitions; i++)
                 FillAssociations(i,NULL);
             FillAssociations(90,FileUnderProcess);
             // Scan the "Generate" code
             if (GenerateCodeFor(Mask,CheckUsedVar,PutVarText,GetVarText,f))
                return 1;
            }
          else // GenNode
            // For vars type 6 => Set Content of association
            AddContent();
         }
      }
   }
 while (st && pos2);
 return 0;
}


static char *AssoNameT;
static TNCSAssociative *AssoT;
static char *extraT;

static void PutAssoMainCode(int num, FILE *f)
{
 int i,l;
 char *n;

 switch (num)
   {
    // the name of the association
    case 1:
         fputs(AssoNameT,f);
         break;
    // the menu for it
    case 2:
         i=AssoT->getCount();
         fprintf(f,"%s\n",StartMenu);
         for (l=0; l<i; l++)
            {
             n=AssoT->atStr(l);
             GenMenuEntry(f,n,n,extraT);
            }
         fprintf(f,"%s\n\n",EndMenuVal);
         break;
   }
}

static int GetVarAssoMain(int num,char *&start,char *&end,int &flags)
{
 if (num==1)
   {
    // the name of the association
    end=start=AssoNameT;
    flags=sdgSimpleString | sdgASCIIZ;
    return 1;
   }
 return 0;
}

static int AssoNum;

static void PutAssoRestCode(int num, FILE *f)
{
 char *Comment;
 TNoCaseSOSStringCollection *col;
 col=AssoT->atCol(AssoNum);

 switch (num)
   {
    case 1:
         fputs(AssoT->atStr(AssoNum),f);
         if (extraT)
            fprintf(f," %s",extraT);
         break;

    case 2:
         fputs(AssoT->atStr(AssoNum),f);
         break;

    case 3:
         Comment=AssoT->GetContent(AssoNum);
         if (Comment)
            OutPutString(Comment,Comment+strlen(Comment)-1,f);
         break;

    case 4:
         {
          int c,j;
          c=col->getCount();
          fprintf(f,"%s\n",StartMenu);
          for (j=0; j<c; j++)
              GenMenuEntry(f,col->atStr(j),col->atStr(j));
          fprintf(f,"%s\n\n",EndMenuVal);
         }
         break;
   }
}

static int GetVarAssoRest(int num,char *&start,char *&end,int &flags)
{
 char *s;

 switch (num)
   {
    case 1:
         s=AssoT->atStr(AssoNum);
         if (extraT)
           {
            int l=strlen(s)+strlen(extraT)+2;
            end=start=new char[l];
            sprintf(start,"%s %s",s,extraT);
            flags=sdgSimpleString | sdgASCIIZ | sdgFreeIt;
           }
         else
           {
            end=start=s;
            flags=sdgSimpleString | sdgASCIIZ;
           }
         return 1;

    case 2:
         start=end=AssoT->atStr(AssoNum);
         flags=sdgSimpleString | sdgASCIIZ;
         return 1;

    case 3:
         s=AssoT->GetContent(AssoNum);
         if (s)
           {
            start=end=s;
            flags=sdgASCIIZ;
            return 1;
           }
         break;
   }
 return 0;
}

static void GenerateAssoc(TNCSAssociative *a, char *name, char *extra, FILE *f)
{
 int i,l;

 AssoNameT=name; AssoT=a; extraT=extra;
 GenerateCodeFor(mAssoMain,CheckUsedAssoMain,PutAssoMainCode,GetVarAssoMain,f);
 i=a->getCount();
 for (l=0; l<i; l++)
    {
     AssoNum=l;
     GenerateCodeFor(mAssoRest,CheckUsedAssoRest,PutAssoRestCode,GetVarAssoRest,f);
    }
}

static void DumpFile(char *file,char *from,int kill=1)
{
 FILE *f;
 char buf[256];
 int l;

 f=fopen(file,"rt");
 if (f)
   {
    fgets(buf,256,f);
    if (!feof(f))
      {
       TXHPrintMessage(from);
       do
         {
          l=strlen(buf);
          if (buf[l-1]=='\n')
             buf[l-1]=0;
          TXHPrintMessage(buf);
          fgets(buf,256,f);
         }
       while (!feof(f));
      }
    fclose(f);
    if (kill)
       unlink(file);
   }
}

static void CallCommandLine(char *s,char *pattern,char *s0,char *s1)
{ // picky tricky string merge ;-)
 if (strcmp(pattern,"NULL")==0)
    return;
 if (*pattern=='@')
   {
    char buf[PATH_MAX];
    strcpy(buf,s1);
    strcat(buf,pattern+1);
    TXHPrintMessage("Copying files ...");
    CopyFile(s0,buf);
    return;
   }
 char *ori=s;
 char *aux;
 for (;*pattern; pattern++)
    {
     if (*pattern=='~' && *(pattern+1)=='0')
        for (pattern++,aux=s0; *aux; aux++,s++) *s=*aux;
     else
     if (*pattern=='~' && *(pattern+1)=='1')
        for (pattern++,aux=s1; *aux; aux++,s++) *s=*aux;
     else
     if (*pattern=='~' && *(pattern+1)=='9' && *(pattern+2)=='0')
       {
        pattern+=2;
        if (TXHFilesDir)
           for (aux=TXHFilesDir; *aux; aux++,s++) *s=*aux;
        else
           *(s++)='.';
       }
     else
       {
        *s=*pattern;
        s++;
       }
    }
 *s=0;
 char *out=open_stdout();
 char *err=open_stderr();
 system(ori);
 close_stdout();
 close_stderr();
 DumpFile(out,"From stdout:");
 DumpFile(err,"From stderr:");
}


int TXHGenerateAll(void)
{
 FILE *f=NULL,*d=NULL;
 int l,c,i,MustBeDeleted;
 char *buffer;
 SOStack stk,stkL;
 char FileName[PATH_MAX];
 char LineB[PATH_MAX];
 char *tmp1=0,*tmp2=0;
 int ret=0;

 TXHError=0;
 DefinedCommands=new TNoCaseSOSStringCollection(5,5,&stk);
 ListOfConstants=new TNSSOSCol(7,5,&stk);
 CommandLines   =new TNSSOSCol(3,2,&stk);
 stkAliasGlobal=&stk;

 if (!DefinedCommands || !ListOfConstants || !CommandLines)
    goto CleanUp;
 // Read the spects
 if (TXHFilesDir)
   {
    strcpy(FileName,TXHFilesDir);
    l=strlen(FileName)-1;
    if (FileName[l-1]!='/' && FileName[l-1]!='\\')
      {
       FileName[l+1]='/';
       FileName[l+2]=0;
      }
    strcat(FileName,TXHFormatFile);
   }
 else
   strcpy(FileName,TXHFormatFile);
 if (TXHReadSpects(FileName,&stk))
   {
    ret=1;
    goto CleanUp;
   }
      
 // Generate the file
 ret=2;
 tmp1=unique_name("tx");
 if (TXHKeepTemporal)
    tmp2=string_dup(TXHTempGenFile);
 else
    tmp2=unique_name("tg");
 TXHNodesFile=f=fopen(tmp1,"wt+");
 if (!f)
   {
    TXHError=22;
    goto CleanUp;
   }
 // List of all functions
 FunList = new TNoCaseSOSStringCollection(20,5,&stk);

 ret=3;
 TXHGenSection=4;
 do
   {
    buffer=TXHGetNextFile(l,MustBeDeleted,FileName);
    if (buffer)
      {
       fList=new TNoCaseSOSStringCollection(20,5,&stkL);
       if (fList)
         {
          CreateFunctionList(buffer,l,stkL,fList,0,"C/C++");
          FileUnderProcess=FileName;
          if (TXHGenerateFor(buffer,f,stk))
             goto CleanUp;
          delete fList;
          fList=NULL; // Mark as deleted
          stkL.Clean();
         }
       if (MustBeDeleted)
          delete buffer;
      }
   }
 while (buffer);
 ret=2;

 rewind(f);
 d=fopen(tmp2,"wt");
 if (!d)
   {
    TXHError=22;
    goto CleanUp;
   }
 GenerateCodeFor(mMain,CheckUsedMain,PutMainCode,GetVarMain,d);
 fclose(d);
 fclose(f);
 d=f=NULL;

 c=CommandLines->getCount();
 for (i=0; i<c; i++)
    {
     stkHandler st=(stkHandler)(CommandLines->at(i));
     stkHandler pr=stk.GetPreviousOf(st);

     TXHPrintMessage("");
     char tmp[200];
     strcpy(tmp,"Generating: ");
     strncat(tmp,stk.GetStrOf(st),187);
     tmp[199]=0;
     TXHPrintMessage(tmp);
     CallCommandLine(LineB,stk.GetStrOf(pr),tmp2,TXHOutBaseName);
    }
 ret=0;

CleanUp:
 if (f)
    fclose(f);
 if (d)
    fclose(d);

 unlink(tmp1);
 string_free(tmp1);
 if (!TXHKeepTemporal)
    unlink(tmp2);
 string_free(tmp2);

 // TVision objects have destroy to check for NULL
 CLY_destroy(FunList);
 CLY_destroy(fList);
 CLY_destroy(DefinedCommands);
 CLY_destroy(ListOfConstants);
 CLY_destroy(CommandLines);
 // Init Vars
 for (i=0; i<Associations; i++)
    {
     // delete double checks for NULL pointers, so it's safe
     delete AssoMenu[i];
     delete AssoNode[i];
     delete AssoExtra[i];
     delete AssoObj[i];
    }
 delete Mask;
 delete StartMenu;
 delete EntryMenuVal;
 delete EndMenuVal;
 delete EndOfPar;
 delete CrossRef;
 delete DoubleA;
 delete ASCIIConv;
 delete BreakLine;
 return ret;
}


