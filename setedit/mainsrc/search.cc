/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  This module have all the low level functions for search and replace in
the editor. I put it in a separated file because the support of normal
search, RegEx and PCRE is a little complex and messy.

***************************************************************************/
#include <ceditint.h>

#include <stddef.h>
#define Uses_regex
#define Uses_string
#define Uses_TVCodePage
#include <stdio.h>

#include <ced_pcre.h>
#define Uses_AllocLocal
#define Uses_MsgBox
#define Uses_TCEditor
#define Uses_TCEditor_Class
#define Uses_TCEditor_Internal
#define Uses_TCEditor_External
#include <ceditor.h>

#define Block ((const char *)(block))

/*****************************************************************************

  General interface, it calls to the specific functions.
  They use the static flags of the class for the mode.

*****************************************************************************/

static char *GetPCREHit(int hit, int &len);
static char *GetRegExHit(int hit, int &len);
static char *GetNormalHit(int hit, int &len);

/**[txh]********************************************************************

  Description:
  It compiles the search. That's just a wrapper. For RegEx the functions
really compile the expressions, for common searchs it just makes some simple
tasks.

***************************************************************************/

int TCEditor::CompileSearch(char *searchStr, char *replaceStr)
{
 if (replaceStr)
    if (CompileReplace(replaceStr))
       return -1;

 if (editorFlags & efRegularEx)
   {
    if (SUP_PCRE && RegExStyle==efPerlRegEx)
       return CompilePCRE(searchStr);
    else
       return CompileRegEx(searchStr);
   }
 return CompileNormal(searchStr);
}


uint32 TCEditor::MakeASearch(char *text, uint32 len, int &matchLen)
{
 if ((editorFlags & efRegularEx) && !(editorFlags & efOptimizedRegex))
   {
    if (SUP_PCRE && RegExStyle==efPerlRegEx)
       return MakeAPCRESearch(text,len,matchLen);
    else
       return MakeARegExSearch(text,len,matchLen);
   }
 return MakeANormalSearch(text,len,matchLen);
}

char *TCEditor::GetTheReplace(int &mustDelete, uint32 &len)
{
 if ((ReplaceStyle==efNormalText) || !(editorFlags & efRegularEx))
   {
    return GetNormalReplace(mustDelete,len);
   }
 if ((editorFlags & efRegularEx) && !(editorFlags & efOptimizedRegex))
   {
    if (SUP_PCRE && RegExStyle==efPerlRegEx)
       return GetSpecialReplace(mustDelete,len,GetPCREHit);
    else
       return GetSpecialReplace(mustDelete,len,GetRegExHit);
   }
 return GetSpecialReplace(mustDelete,len,GetNormalHit);
}

static
int TryToOptimize(const char *expr)
{
 const char *s=expr;
 for (; *s; s++)
     if (!isWordChar(*s))
        break;
 return (*s==0);
}

/*****************************************************************************

 Replace routines

*****************************************************************************/

static char *CompiledReplace=0;
static char  ReplaceIsCompiled=0;
static int   numReplaceTags;
static int   lenReplaceStr;
static int   MaxReplaceNeeded;

int TCEditor::CompileReplace(char *replaceStr)
{
 if (ReplaceIsCompiled)
   {
    ReplaceIsCompiled=0;
    delete CompiledReplace;
   }

 if (ReplaceStyle==efNormalText)
   {
    CompiledReplace=replaceStr;
    lenReplaceStr=strlen(replaceStr);
    return 0;
   }

 // Make a copy
 int l=strlen(replaceStr);
 char *aux=new char[l+1];
 if (!aux)
    return 1;
 memcpy(aux,replaceStr,l);
 aux[l]=0;
 CompiledReplace=aux;

 numReplaceTags=0;
 lenReplaceStr=0;
 MaxReplaceNeeded=0;
 char *s=CompiledReplace, *end;
 int val,dist;
 while (*s)
   {
    if (*s=='$')
      {
       s++;
       if (ucisdigit(*s))
         {
          val=strtol(s,&end,10);
          *(s-1)=0;
          dist=end-s-1;
          if (dist)
             memcpy(s+1,end,l-(end-CompiledReplace)+1);
          *s=char(val);
          if (int(*s)>MaxReplaceNeeded)
             MaxReplaceNeeded=int(*s);
          numReplaceTags++;
          s++;
         }
       else
         {
          lenReplaceStr++;
          if (*s)
            {
             lenReplaceStr++;
             s++;
            }
         }
      }
    else
      {
       lenReplaceStr++;
       s++;
      }
   }
 #if 0
 fprintf(stderr,"Targets: %d, largo: %d\n",numReplaceTags,lenReplaceStr);
 #endif
 ReplaceIsCompiled=1;
 MaxReplaceNeeded++;

 return 0;
}

char *TCEditor::GetNormalReplace(int &mustDelete, uint32 &len)
{
 mustDelete=0;
 len=lenReplaceStr;
 return CompiledReplace;
}

char *TCEditor::GetSpecialReplace(int &mustDelete, uint32 &len,
                                  char *(*GetHit)(int numHit, int &lenHit))
{
 if (!ReplaceIsCompiled)
    return 0;
 // first meassure the size
 int lenMatchs=lenReplaceStr;
 int i,wich,lenHit;
 char *s=CompiledReplace;
 char *ret,*dest,*hit;

 mustDelete=1;
 i=0;
 while (i<numReplaceTags)
   {
    while (*s) s++;
    s++;
    wich=*s;
    hit=GetHit(wich,lenHit);
    if (hit)
       lenMatchs+=lenHit;
    s++;
    i++;
   }
 ret=new char[lenMatchs+1];
 len=lenMatchs;
 dest=ret;
 s=CompiledReplace;
 i=0;
 while (i<numReplaceTags)
   {
    while (*s) *(dest++)=*(s++);
    s++;
    wich=*s;
    hit=GetHit(wich,lenHit);
    if (hit)
      {
       while (lenHit--)
         *(dest++)=*(hit++);
      }
    s++;
    i++;
   }
 while (*s) *(dest++)=*(s++);
 *dest=0;

 return ret;
}

/*****************************************************************************

 POSIX RegEx interface

*****************************************************************************/

// Perhaps that's wrong, but I didn't put this variable in the class just because
// if I do it I'll force to include the RegEx header everywhere.
static regex_t CompiledRegEx;
static regmatch_t *RegExMatchs=0;
static int  RegExMaxMatchs=0;
static char *RegExText;
static char RegExIsCompiled=0;
static char RegExHitAvailable=0;

static
void ShowRegexError(int retval)
{
 char b[200];

 regerror(retval,&CompiledRegEx,b,200);
 messageBox(mfError | mfOKButton,__("Regular expression error: %s"),b);
 RegExIsCompiled=0;
}


static
void FreeRegExLastSearch(void)
{
 // Free the memory of the last search
 if (RegExIsCompiled)
   {
    RegExIsCompiled=0;
    RegExHitAvailable=0;
    delete RegExMatchs;
    regfree(&CompiledRegEx);
   }
}

int TCEditor::CompileRegEx(char *searchStr)
{
 FreeRegExLastSearch();

 // Test if REGEX are needed.
 if (CanOptimizeRegEx==efOptimizeRegEx)
   {
    if (TryToOptimize(searchStr))
      {
       editorFlags|=efOptimizedRegex;
       return CompileNormal(searchStr);
      }
   }

 RegExMaxMatchs=max(MaxReplaceNeeded,1);
 RegExMatchs=new regmatch_t[RegExMaxMatchs];
 if (!RegExMatchs)
    return -1;

 int flags=REG_NEWLINE;
 if (RegExStyle==efExtendedRegEx)
    flags|=REG_EXTENDED;
 if (!(editorFlags & efCaseSensitive))
    flags|=REG_ICASE;

 int retcomp=regcomp(&CompiledRegEx,searchStr,flags);

 if (retcomp)
   {
    ShowRegexError(retcomp);
    return -1;
   }
 RegExIsCompiled=1;
 return 0;
}

uint32 TCEditor::MakeARegExSearch(char *block, uint32 size, int &matchLen)
{
 if (!size || !RegExIsCompiled)
    return sfSearchFailed;

 RegExText=block;
 char v=block[size];
 block[size]=0;
 int retval=regexec(&CompiledRegEx,Block,RegExMaxMatchs,RegExMatchs,0);
 block[size]=v;

 if (retval && retval!=REG_NOMATCH)
    ShowRegexError(retval);
 else
   {
    if (retval!=REG_NOMATCH)
      {
       RegExHitAvailable=1;
       matchLen=RegExMatchs[0].rm_eo-RegExMatchs[0].rm_so;
       return RegExMatchs[0].rm_so;
      }
   }
 return sfSearchFailed;
}

static
char *GetRegExHit(int hit, int &len)
{
 if (!RegExHitAvailable || hit>=RegExMaxMatchs)
    return 0;
 len=RegExMatchs[hit].rm_eo-RegExMatchs[hit].rm_so;
 return RegExText+RegExMatchs[hit].rm_so;
}


/*****************************************************************************

 Perl-Compatible Regular Expressions (PCRE) interface
 Thanks to the University of Cambridge

*****************************************************************************/

// Perhaps that's wrong, but I didn't put this variable in the class just because
// if I do it I'll force to include the RegEx header everywhere.
static pcre *CompiledPCRE;
static pcre_extra *CompiledPCREExtra;
static int *PCREMatchs=0;
static int  PCREMaxMatchs=0;
static char *PCREText;
static char PCREIsCompiled=0;
static char PCREHitAvailable=0;

static
void ShowPCREError(const char *error)
{
 messageBox(mfError | mfOKButton,__("Regular expression error: %s"),error);
 PCREIsCompiled=0;
}

static
void FreePCRELastSearch()
{
 if (!SUP_PCRE)
    return;
 if (PCREIsCompiled)
   {
    PCREIsCompiled=0;
    PCREHitAvailable=0;
    delete PCREMatchs;
    // No regfree?
    free(CompiledPCRE);
    free(CompiledPCREExtra);
   }
}

int TCEditor::CompilePCRE(char *searchStr)
{
 if (!SUP_PCRE)
    return -1;
 // Free the memory of the last search
 FreePCRELastSearch();

 // Test if PCRE are needed.
 if (CanOptimizeRegEx==efOptimizeRegEx)
   {
    if (TryToOptimize(searchStr))
      {
       editorFlags|=efOptimizedRegex;
       return CompileNormal(searchStr);
      }
   }

 int flags=PCRE_MULTILINE;
 if (!(editorFlags & efCaseSensitive))
    flags|=PCRE_CASELESS;

 const char *error;
 int   errorOffset;
 CompiledPCRE=pcre_compile(searchStr,flags,&error,&errorOffset,0);
 if (!CompiledPCRE)
   {
    ShowPCREError(error);
    return -1;
   }

 CompiledPCREExtra=pcre_study(CompiledPCRE,0,&error);
 if (error)
   {
    free(CompiledPCRE);
    ShowPCREError(error);
    return -1;
   }

 // For PCRE 2.x this value should be enlarged to left space needed by
 // pcre_exec 
 PCREMaxMatchs=(pcre_info(CompiledPCRE,0,0)+1)*3;
 PCREMatchs=new int[PCREMaxMatchs];

 if (!PCREMatchs)
   {
    free(CompiledPCRE);
    free(CompiledPCREExtra);
    return -1;
   }

 PCREIsCompiled=1;
 return 0;
}

uint32 TCEditor::MakeAPCRESearch(char *block, uint32 size, int &matchLen)
{
 if (!SUP_PCRE || !size || !PCREIsCompiled)
    return sfSearchFailed;

 PCREText=block;
 int hits=pcre_exec(CompiledPCRE,CompiledPCREExtra,block,size, PCRE206 0,PCREMatchs,
                    PCREMaxMatchs);

 PCREHitAvailable=0;
 if (hits<0 && hits!=PCRE_ERROR_NOMATCH)
   {
    char *s=TVIntl::getTextNew(__("PCRE Exec error"));
    ShowPCREError(s);
    DeleteArray(s);
   }
 else
   {
    if (hits!=PCRE_ERROR_NOMATCH)
      {
       PCREHitAvailable=hits;
       matchLen=PCREMatchs[1]-PCREMatchs[0];
       return PCREMatchs[0];
      }
   }
 return sfSearchFailed;
}

static
char *GetPCREHit(int hit, int &len)
{
 if (!SUP_PCRE || hit>=PCREHitAvailable || PCREMatchs[2*hit]<0)
    return NULL;
 len=PCREMatchs[2*hit+1]-PCREMatchs[2*hit];
 return PCREText+PCREMatchs[2*hit];
}

void TCEditor::FreeRegExMemory(void)
{
 FreeRegExLastSearch();
 FreePCRELastSearch();
}

/*****************************************************************************

 Normal search interface

*****************************************************************************/

static char *NormalText=0;
static int lenNormalText;
static uint32 lastHit=0;

int TCEditor::CompileNormal(char *searchStr)
{
 NormalText=searchStr;
 lenNormalText=strlen(searchStr);
 return 0;
}

/**[txh]********************************************************************

  Description:
  Search a string inside the buffer. Case sensitive. block: The buffer.
size:  Size of the buffer. str: The string to search.

***************************************************************************/

unsigned TCEditor_scan(const void *block, unsigned size, const char *str)
{
 if (!size)
    return sfSearchFailed;
 uint32 ret=0;
 while (size--)
   {
    if (Block[ret]==str[0])
      {
       uint32 i=0;
       do
         {
          i++;
          if (!str[i])
             return (ret);
          if (size<i)
             return sfSearchFailed;
         }
       while (Block[ret+i]==str[i]);
      }
    ret++;
   }
 return sfSearchFailed;
}

/**[txh]********************************************************************

  Description:
  Search a string inside the buffer. No case sensitive. block: The buffer.
size: Size of the buffer. str: The string to search.

***************************************************************************/

unsigned TCEditor_iScan(const void *block, unsigned size, const char *str)
{
 if (!size)
    return sfSearchFailed;
 uint32 ret=0;
 while (size--)
  {
   if (TVCodePage::toUpper(Block[ret])==TVCodePage::toUpper(str[0]))
     {
      uint32 i=0;
      do
        {
         i++;
         if (!str[i])
            return (ret);
         if (size<i)
            return sfSearchFailed;
        }
      while (TVCodePage::toUpper(Block[ret+i])==TVCodePage::toUpper(str[i]));
     }
   ret++;
  }
 return sfSearchFailed;
}


uint32 TCEditor::MakeANormalSearch(char *block, uint32 size, int &matchLen)
{
 if (!NormalText)
    return sfSearchFailed;

 uint32 pos;
 if (editorFlags & efCaseSensitive)
    pos=TCEditor_scan(block,size,NormalText);
 else
    pos=TCEditor_iScan(block,size,NormalText);

 matchLen=lenNormalText;
 lastHit=pos;

 return pos;
}

static
char *GetNormalHit(int hit, int &len)
{
 if (hit>0)
    return 0;
 len=lenNormalText;
 return NormalText+lastHit;
}

