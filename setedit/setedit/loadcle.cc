/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#define Uses_string
#define Uses_ctype
#define Uses_stdio
#define Uses_stdlib

#define Uses_TStringCollection
#include <tv.h>

#include <ced_pcre.h>
#include <dyncat.h>
#include <loadcle.h>

strCLE *CLEValues=0;
static int     CLECant=0;
static char    CLELoaded=0;
static char   *CLEFile=0;
static int     CLEMaxSubEx=0;
static int    *PCREMatchs=0;
static TStringCollection *CLEList=0;

static void ReplaceCRby0(char *s)
{
 for (; *s && *s!='\n' && *s!='\r'; s++);
 *s=0;
}

static char *MoveAfterEqual(char *s)
{
 for (; *s && *s!='='; s++);
 if (*s) s++;
 for (; *s && ucisspace(*s); s++);
 return s;
}

static
void LoadCLENames()
{
 FILE *f;
 int i;
 char *pos;

 if (CLELoaded || !CLEFile)
    return;

 // We must have at least an empty list
 destroy0(CLEList);
 CLEList=new TStringCollection(6,4);

 if ((f=fopen(CLEFile,"rt"))==NULL)
    return;

 char b[maxCLEFileWidth];
 // Meassure the number of definitions
 for (CLECant=0; !feof(f); )
    {
     fgets(b,maxCLEFileWidth,f);
     if (!feof(f) && strncasecmp(b,"End",3)==0)
        CLECant++;
    }
 CLELoaded=1;
 if (!CLECant)
   {
    fclose(f);
    return;
   }

 // Allocate enough memory
 CLEValues=new strCLE[CLECant];
 memset(CLEValues,0,sizeof(strCLE)*CLECant);

 rewind(f);
 for (i=0; i<CLECant && !feof(f); i++)
    {
     do
       {
        do
          {
           fgets(b,maxCLEFileWidth,f);
          }
        while (*b=='#' || ucisspace(*b));

        if (strncasecmp(b,"Name",4)==0)
          {
           pos=MoveAfterEqual(b);
           ReplaceCRby0(pos);
           char *s=newStr(pos);
           CLEValues[i].Name=s;
           CLEList->insert(s);
          }
        else
        // Avoid name pointing to NULL
        if (strncasecmp(b,"End",3)==0 && !CLEValues[i].Name)
          {
           char *s=TVIntl::getTextNew(__("Unknown"));
           CLEValues[i].Name=s;
           CLEList->insert(s);
          }
       }
     while (!feof(f) && strncasecmp(b,"End",3)!=0);
    }
 fclose(f);
}

/**[txh]********************************************************************

  Description:
  Returns the index of the CLE in the array.

  Return:
  Index or -1 if not found or load error.

***************************************************************************/

int CLEGetIndexOf(const char *name)
{
 if (!CLELoaded) LoadCLENames();
 if (!CLELoaded) return -1;

 int eureka,i;
 // Look for name in the array
 for (eureka=0, i=0; i<CLECant; i++)
    {
     eureka=strcmp(CLEValues[i].Name,name)==0;
     if (eureka) break;
    }
 if (!eureka)
    return -1;
 return i;
}

#if 0
 #define SubExVal(a) \
     if (strncasecmp(b,#a,sizeof(#a)-1)==0) \
       { \
        pos=MoveAfterEqual(b); \
        ReplaceCRby0(pos); \
        CLEValues[i].##a=atoi(pos); \
       }
 
 #define ExpComp(a) \
     if (strncasecmp(b,#a,sizeof(#a)-1)==0) \
       { \
        pos=MoveAfterEqual(b); \
        ReplaceCRby0(pos); \
        CLEValues[i].##a=CLECompileRegEx(pos,subExp); \
        if (subExp>subExpMax) \
           subExpMax=subExp; \
       }
#else
 #define SubExVal(a) \
     if (strncasecmp(b,#a,sizeof(#a)-1)==0) \
       { \
        pos=MoveAfterEqual(b); \
        ReplaceCRby0(pos); \
        CLEValues[i].a=atoi(pos); \
       }
 
 #define ExpComp(a) \
     if (strncasecmp(b,#a,sizeof(#a)-1)==0) \
       { \
        pos=MoveAfterEqual(b); \
        ReplaceCRby0(pos); \
        CLEValues[i].a=CLECompileRegEx(pos,subExp); \
        if (subExp>subExpMax) \
           subExpMax=subExp; \
       }
#endif

static
void LoadOneCLE(const char *name)
{
 if (!SUP_PCRE)
    return;
 FILE *f;
 int i,eureka,subExp,subExpMax=0,index;
 char *pos;

 index=CLEGetIndexOf(name);
 if (index<0 || CLEValues[index].Loaded) return;
 if ((f=fopen(CLEFile,"rt"))==NULL)
    return;

 char b[maxCLEFileWidth];
 // Look for name in the file
 for (eureka=0, i=0; i<CLECant && !eureka && !feof(f); i++)
    {
     do
       {
        do
          {
           fgets(b,maxCLEFileWidth,f);
          }
        while (*b=='#' || ucisspace(*b));

        if (strncasecmp(b,"Name",4)==0)
          {
           pos=MoveAfterEqual(b);
           ReplaceCRby0(pos);
           if (strcmp(name,pos)==0)
              eureka=1;
          }
       }
     while (!eureka && !feof(f) && strncasecmp(b,"End",3)!=0);
    }
 if (!eureka)
   {
    fclose(f);
    return;
   }

 i=index;
 #define C(a) CLEValues[i].a=0xFF
 C(File);
 C(Line);
 C(Severity);
 C(Description);
 C(EnterDirDir);
 #undef C
 // Load it.
 do
   {
    do
      {
       fgets(b,maxCLEFileWidth,f);
      }
    while (*b=='#' || ucisspace(*b));

    if (strncasecmp(b,"UseInternal",11)==0)
       CLEValues[i].UseInternal=1;
    else
      ExpComp(Pattern)
    else
      SubExVal(File)
    else
      SubExVal(Line)
    else
      SubExVal(Severity)
    else
      SubExVal(Description)
    else
      ExpComp(EnterDirPat)
    else
      ExpComp(LeaveDir)
    else
      SubExVal(EnterDirDir)
   }
 while (!feof(f) && strncasecmp(b,"End",3)!=0);
 CLEValues[i].Loaded=1;

 if (subExpMax>CLEMaxSubEx)
   {
    delete PCREMatchs;
    CLEMaxSubEx=subExpMax;
    PCREMatchs=new int[CLEMaxSubEx];
   }

 fclose(f);
}

int CLEGetIndexOfLoad(const char *name)
{
 int i=CLEGetIndexOf(name);
 if (!CLELoaded) return -1;
 if (!CLEValues[i].Loaded) LoadOneCLE(name);
 if (!CLEValues[i].Loaded || CLEValues[i].UseInternal) return -1;
 return i;
}

void UnloadCLEFile()
{
 int i;
 destroy0(CLEList);
 if (CLECant && CLEValues)
   {
    for (i=0; i<CLECant; i++)
       {
        free(CLEValues[i].Pattern);
        free(CLEValues[i].EnterDirPat);
        free(CLEValues[i].LeaveDir);
        // Part of the string collection!
        //delete CLEValues[i].Name;
       }
   }
 delete[] CLEValues;
 CLEValues=0;
 CLECant=0;
 CLELoaded=0;
 CLEMaxSubEx=0;
 delete[] PCREMatchs;
 PCREMatchs=0;
 delete[] CLEFile;
 CLEFile=0;
}

void CLESetFileName(const char *name)
{
 delete[] CLEFile;
 CLEFile=newStr(name);
}

TStringCollection *CLEGetList()
{
 if (!CLELoaded) LoadCLENames();
 return CLEList;
}

/**[txh]********************************************************************

  Description:
  Compiles a RegEx.

  Return: A pointer to the compiled RegEx or 0 if error.

***************************************************************************/

pcre *CLECompileRegEx(char *text, int &subX)
{
 if (!SUP_PCRE)
    return NULL;
 const char *error;
 int   errorOffset;
 pcre *ret=pcre_compile(text,0,&error,&errorOffset,0);
 if (!ret)
   {
    subX=0;
    return 0;
   }

 subX=(pcre_info(ret,0,0)+1)*3;
 return ret;
}

static int LastHits=0;
static char *BufSearch;

int CLEDoSearch(char *search, int len, pcre *CompiledPCRE)
{
 if (!SUP_PCRE)
    return 0;
 LastHits=pcre_exec(CompiledPCRE,0,search,len,PCRE206 0,PCREMatchs,CLEMaxSubEx);
 BufSearch=search;

 return LastHits<0 ? 0 : LastHits;
}

void CLEGetMatch(int match, char *buf, int maxLen)
{
 if (!SUP_PCRE)
    return;
 if (match<0 || match>LastHits)
   {
    *buf=0;
    return;
   }
 int start=PCREMatchs[match*2];
 int end=PCREMatchs[match*2+1];
 int len=end-start;
 if (len>maxLen-1)
    len=maxLen-1;
 memcpy(buf,BufSearch+start,len);
 buf[len]=0;
}

void CLEGetMatch(int match, int &offset, int &len)
{
 if (!SUP_PCRE || match<0 || match>LastHits)
   {
    offset=-1; len=0;
    return;
   }
 offset=PCREMatchs[match*2];
 int end=PCREMatchs[match*2+1];
 len=end-offset;
}

