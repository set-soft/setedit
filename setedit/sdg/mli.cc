/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*****************************************************************************

  Micro Lisp Interpreter.

  That's a try to create a simple and very flexible Lisp interpreter. The
idea is to create a kernel for the parsing.

  Important!: The stupid files mlivar.cc and mliabase.cc are because gcc
needs a file for each of these classes. The mechanism used to detect
implementations/declarations doesn't work if gcc doesn't see it. In fact if
I make all in one cc file it doesn't create the virtual tables and generates
thousand of undefined references. I was able to avoid it with = 0 but the
code is inflated in some cases ... I moved mlivar.cc to this code.

*****************************************************************************/

#include <configed.h> // To know about PCRE support
#define Uses_ctype
#define Uses_stdlib
#define Uses_string
#define Uses_stdio
#define Uses_snprintf

#define Uses_TLispVariableCol
#define Uses_TLispBaseVars
#define Uses_TLispVarDefs
#define Uses_TMLIArraySimple
#define Uses_TMLIBase
#define Uses_TVCodePage
#include <mli.h>

#include <dyncat.h>
#include <pathtool.h>
#include <loadshl.h>

// Declaring the destructor here instead of in the header
// seems to save more than 1Kb in the EXE. Is because the
// virtual table is multiplied?!?. I didn't saw the same
// with the array.
TLispVar::~TLispVar() {}

/************************************* Lisp Objects ********************************/

TLispString::TLispString(const char *s, tlsMode mode)
{ // Support for escaped C characters, is that C Lisp? ;-) only the most common
 len=strlen(s);
 str=new char[len+1];
 if (mode==tlsParse)
   {
    int i=0;
    while (*s)
      {
       if (*s=='\\' && s[1])
         {
          s++;
          len--;
          switch (*s)
            {// OS dependant
             case 'n':
                  #if defined(TVOS_DOS) || \
                      (defined(TVOS_Win32) && !defined(TVCompf_Cygwin))
                  str[i++]='\r';
                  str[i]='\n';
                  len++;
                  #else
                  str[i]='\n';
                  #endif
                  break;
             // Line Feed alone, no matters the OS
             case 'l':
                  str[i]='\n';
                  break;
             case 't':
                  str[i]='\t';
                  break;
             case 'r':
                  str[i]='\r';
                  break;
             default:
                  str[i]=*s;
            }
         }
       else
         str[i]=*s;
       i++;
       s++;
      }
    str[i]=0;
   }
 else
   memcpy(str,s,len+1);
 type=MLITypeString;
 flags=StrOwner;
}

TLispString::TLispString(const char *s, int l, int flgs, int extraType)
{
 len=l;
 if (flgs & StrAlreadyAllocated)
    str=(char *)s;
 else
   {
    str=new char[l+1];
    memcpy(str,s,l+1);
    str[l]=0;
   }
 flags=flgs;
 type=MLITypeString | extraType;
}

int TLispString::print(FILE *f)
{
 fputs(str,f);
 return len;
}

char *TLispString::toStr()
{
 char *ret=new char[len+1];
 strcpy(ret,str);
 return ret;
}

TLispString::~TLispString()
{
 if (flags & StrOwner)
    delete[] str;
}

TLispConstString::TLispConstString(const char *s)
{
 len=strlen(s);
 str=(char *)s;
 type=MLITypeString;
 flags=StrOwner;
}

TLispConstString::TLispConstString(const char *s, int l, int flgs, int extraType)
{
 len=l;
 str=(char *)s;
 flags=flgs;
 type=MLITypeString | extraType;
}

TLispConstString::~TLispConstString()
{
}

TLispInteger::TLispInteger(int v)
{
 val=v;
 type=MLITypeInteger;
}

int TLispInteger::print(FILE *s)
{
 return fprintf(s,"%d",val);
}

char *TLispInteger::toStr()
{
 char *ret=new char[sizeof(int)*3];
 sprintf(ret,"%d",val);
 return ret;
}

TLispCommand::TLispCommand(Command c)
{
 command=c;
 type=MLITypeCommand;
}

const char ErrorPrCommand[]=" ERROR: Command printed ";

int TLispCommand::print(FILE *s)
{
 return fprintf(s,ErrorPrCommand);
}

char *TLispCommand::toStr()
{
 char *ret=new char[sizeof(ErrorPrCommand)];
 strcpy(ret,ErrorPrCommand);
 return ret;
}

TLispVariable::~TLispVariable()
{
 free(var);
 if (val)
    delete val;
}

TLispVariable::TLispVariable(char *v, TLispVar *o)
{
 var=strdup(v);
 val=o;
 type=MLITypeVar;
}

int TLispVariable::print(FILE *s)
{
 return val->print(s);
}

char *TLispVariable::toStr()
{
 return val->toStr();
}

int TLispVariableCol::compare(void *s1,void *s2)
{
 return strcmp((char *)s1,(char *)s2);
}

void *TLispVariableCol::keyOf(void *s)
{
 return ((TLispVariable *)s)->var;
}

void TLispVariableCol::freeItem(void *s)
{
 if (s)
    delete ((TLispVariable *)s);
}

TLispCode::TLispCode(char *s, char *e)
{
 start=s;
 end=e;
 type=MLITypeCode;
}

int TLispCode::print(FILE *s)
{
 char v=*end;
 *end=0;
 fputs(start,s);
 *end=v;
 return end-start;
}

char *TLispCode::toStr()
{
 int l=end-start;
 char *ret=new char[l+1];
 strncpy(ret,start,l);
 ret[l]=0;
 return ret;
}

/********************************** End Lisp Objects *********************************/

void MLIBasePrint(TMLIBase *o,int start ,int cant)
{
 int i,count=0;
 TLispVar *p;

 for (i=0; i<cant; i++)
    {
     p=o->Solve(start+i);
     if (p)
       {
        count+=p->print(o->fileOut);
        destroyFloatVar(p);
       }
     else
      {
       MLIRetNULL();
       return;
      }
    }
 MLIRetInt(count);
}

int TMLIBase::MLIBooleanValOf(TLispVar *var)
{
 switch (GroupTypeOf(var))
   {
    case MLIGString:
         {
          TLispString *s=(TLispString *)var;
          return (s->len!=0) ? 0xFFFFFFFF : 0;
         }
    case MLIGInteger:
         {
          TLispInteger *i=(TLispInteger *)var;
          return i->val;
         }
    case MLIGCommand:
    case MLIGVar:
    case MLIGCode:
         return 0xFFFFFFFF;
   }
 return 0;
}

// If returns the value of the executed code or an integer 0 if no else
void MLIBaseIf(TMLIBase *o,int start ,int cant)
{
 if (cant<2 || cant>3)
   {
    o->Error=MLINumParam;
    MLIRetNULL();
    return;
   }
 TLispVar *p=o->Solve(start);
 int boolval=o->MLIBooleanValOf(p);
 destroyFloatVar(p);
 if (boolval)
   {// Note that MLIRetObj is a macro and *have* side effects
    p=o->Solve(start+1);
    MLIRetObj(p);
   }
 else
   {
    if (cant==3)
      {
       p=o->Solve(start+2);
       MLIRetObj(p);
      }
    else
       MLIRetInt(0);
   }
}

DecFun(MLIBaseCond)
{ 
 CheckNumParams(cant<2 || (cant%2));
 
 LocVar(If); 
 LocVar(Then); 
 
 for (int i=0; i < cant; i += 2) 
    {
     GetVar(i,If);
     int b=o->MLIBooleanValOf(If);
     destroyFloatVar(If);
 
     if (b)
       {
        GetVar(i+1,Then);
        MLIRetObj(Then);
        return;
       }
    } 
 
 MLIRetInt(0); 
 
CleanUp: 
 return; 
} 


DecFun(MLIBaseLeft)
{
 int l;
 LocVarStr(string);
 LocVarInt(left);

 CheckNumParams(cant!=2);
 GetString(0,string);
 GetInteger(1,left);

 l=left->val;
 if (l>string->len)
    l=string->len;

 MLIRetStrLen(string->str,l);

CleanUp:
 destroyFloatVar(string);
 destroyFloatVar(left);
}

DecFun(MLIBaseRight)
{
 int l,lstr;
 LocVarStr(string);
 LocVarInt(len);

 CheckNumParams(cant!=2);
 GetString(0,string);
 GetInteger(1,len);

 l=len->val;
 lstr=string->len;
 if (l>lstr)
    l=lstr;

 MLIRetStrLen(string->str+lstr-l,l);

CleanUp:
 destroyFloatVar(string);
 destroyFloatVar(len);
}

DecFun(MLIBaseSubStr)
{
 int len,from,lstr;
 LocVarStr(String);
 LocVarInt(Start);
 LocVarInt(Len);

 CheckNumParams(cant!=2 && cant!=3);
 GetString(0,String);
 lstr=String->len;
 GetInteger(1,Start);
 from=Start->val;
 GetIntegerOp(2,Len,lstr-from,len);

 if (from>lstr)
   {
    len=0;
    from=0;
   }
 else
   if (len+from>lstr)
      len=lstr-start;
 MLIRetStrLen(String->str+from,len);

CleanUp:
 destroyFloatVar(String);
 destroyFloatVar(Start);
 destroyFloatVar(Len);
}

static
void MLIBaseStrCmpFunc(TMLIBase *o,int start ,int cant, int type)
{
 LocVarStr(Str1);
 LocVarStr(Str2);

 CheckNumParams(cant!=2);
 GetString(0,Str1);
 GetString(1,Str2);
 MLIRetInt(type ? strcasecmp(Str1->str,Str2->str) : strcmp(Str1->str,Str2->str));

CleanUp:
 destroyFloatVar(Str1);
 destroyFloatVar(Str2);
}

DecFun(MLIBaseStrCmp)
{
 MLIBaseStrCmpFunc(o,start,cant,0);
}

DecFun(MLIBaseStrCaseCmp)
{
 MLIBaseStrCmpFunc(o,start,cant,1);
}

DecFun(MLIBaseLength)
{
 LocVarStr(Str);

 CheckNumParams(cant!=1);
 GetString(0,Str);
 MLIRetInt(Str->len);

CleanUp:
 destroyFloatVar(Str);
}

DecFun(MLIBaseGStr)
{
 int l,lstr,lret;
 LocVarStr(string);
 LocVarInt(pos);

 CheckNumParams(cant!=2);
 GetString(0,string);
 GetInteger(1,pos);

 l=pos->val;
 lstr=string->len;
 if (l>lstr)
   {
    l=0;
    lret=0;
   }
 else
   lret=1;
 MLIRetStrLen(string->str+l,lret);

CleanUp:
 destroyFloatVar(string);
 destroyFloatVar(pos);
}

DecFun(MLIBaseSStr)
{
 int l,lstr;
 LocVarStr(string);
 LocVarInt(pos);
 LocVarStr(val);

 CheckNumParams(cant!=3);
 GetString(0,string);
 GetInteger(1,pos);
 GetString(2,val);

 if (val->len)
   {
    l=pos->val;
    lstr=string->len;
    if (l<lstr)
       string->str[l]=val->str[0];
   }
 MLIRetObj(string);

CleanUp:
 //destroyFloatVar(string); That's the return value, don't kill!
 destroyFloatVar(pos);
 destroyFloatVar(val);
}

// Create or modify a variable
DecFun(MLIBaseSetV)
{
 LocVar(Variable);
 LocVarStr(name);

 CheckNumParams(cant!=2);
 GetString(0,name);
 GetVar(1,Variable);
 o->AddVariable(name->str,Variable);
 MLIRetObj(Variable);

CleanUp:
 destroyFloatVar(name);
}

// It just evaluates the parameters.
DecFun(MLIBaseEval)
{
 int i;
 LocVar(Variable);

 for (i=0; i<cant; i++)
    {
     GetVar(i,Variable);
     if (i+1==cant)
        MLIRetObj(Variable);
     else
        destroyFloatVar(Variable);
    }
CleanUp:
 return;
}

// It just evaluates the parameters until an exitloop is executed.
DecFun(MLIBaseLoop)
{
 int i;
 LocVar(Variable);
 CheckNumParams(cant<1);

 Boolean ExitLoop=o->ExitLoop;
 o->ExitLoop=False;

 while (!o->ExitLoop)
   {
    for (i=0; i<cant; i++)
       {
        GetVar(i,Variable);
        if (o->ExitLoop)
          {
           MLIRetObj(Variable);
           break;
          }
        else
           destroyFloatVar(Variable);
       }
   }
CleanUp:
 o->ExitLoop=ExitLoop;
 return;
}

// It just evaluates the parameters until an exitloop is executed or the
// maximum number of iterations is reached.
DecFun(MLIBaseRepeat)
{
 int i, loopCount, maxLoop;
 LocVar(Variable);
 LocVarInt(oMaxLoop);
 CheckNumParams(cant<2);

 Boolean ExitLoop=o->ExitLoop;
 o->ExitLoop=False;

 GetInteger(0,oMaxLoop);
 maxLoop=oMaxLoop->val;
 destroyFloatVar(oMaxLoop);

 loopCount=0;
 while (!o->ExitLoop && loopCount<maxLoop)
   {
    for (loopCount++, i=1; i<cant; i++)
       {
        GetVar(i,Variable);
        if (o->ExitLoop)
          {
           MLIRetObj(Variable);
           break;
          }
        else
           destroyFloatVar(Variable);
       }
   }
 if (!o->ExitLoop)
   {// We must return something:
    MLIRetInt(loopCount);
   }
CleanUp:
 o->ExitLoop=ExitLoop;
 return;
}

DecFun(MLIBaseExitLoop)
{
 CheckNumParams(cant!=0);
 o->ExitLoop=True;
 return;
}

static void AddStrings(TLispString *str,TMLIBase *o,int start ,int cant, int startret)
{
 DynStrCatStruct cat;
 LocVar(Variable);
 int i,l;
 char b[32];

 DynStrCatInit(&cat,str->str,str->len);
 for (i=0; i<cant; i++)
    {
     GetVar(i,Variable);
     switch (GroupTypeOf(Variable))
       {
        case MLIGString:
             DynStrCat(&cat,((TLispString *)Variable)->str,((TLispString *)Variable)->len);
             break;
        case MLIGInteger:
             l=CLY_snprintf(b,32,"%d",((TLispInteger *)Variable)->val);
             DynStrCat(&cat,b,l);
             break;
        default:
             o->Error=MLITypeParam;
             goto CleanUp;
       }
     destroyFloatVar(Variable);
     Variable=NULL;
    }
 start=startret;
 MLIRetStrLen(cat.str,cat.len);
 free(cat.str);

CleanUp:
 destroyFloatVar(Variable);
}

static void AddIntegers(TLispInteger *Val,TMLIBase *o,int start ,int cant, int startret)
{
 LocVarInt(NewVal);
 int i,ret;

 ret=Val->val;
 for (i=0; i<cant; i++)
    {
     GetInteger(i,NewVal);
     ret+=NewVal->val;
     destroyFloatVar(NewVal);
     NewVal=NULL;
    }
 start=startret;
 MLIRetInt(ret);

CleanUp:
 destroyFloatVar(NewVal);
}

DecFun(MLIBaseAdd)
{
 LocVar(Variable);

 CheckNumParams(cant<2);
 GetVar(0,Variable);
 switch (GroupTypeOf(Variable))
   {
    case MLIGString:
         AddStrings((TLispString *)Variable,o,start+1,cant-1,start);
         break;
    case MLIGInteger:
         AddIntegers((TLispInteger *)Variable,o,start+1,cant-1,start);
         break;
    default:
         o->Error=MLIUndefOp;
         MLIRetNULL();
   }
CleanUp:
 destroyFloatVar(Variable);
}

DecFun(MLIBaseSub)
{
 LocVarInt(v1);
 LocVarInt(v2);

 CheckNumParams(cant!=2);
 GetInteger(0,v1);
 GetInteger(1,v2);
 MLIRetInt(v1->val-v2->val);

CleanUp:
 destroyFloatVar(v1);
 destroyFloatVar(v2);
}

// (strstr string search [start_pos])
DecFun(MLIBaseStrStr)
{
 int from,lstr;
 LocVarStr(String);
 LocVarStr(Search);
 LocVarInt(From);

 CheckNumParams(cant!=2 && cant!=3);
 GetString(0,String);
 lstr=String->len;
 GetString(1,Search);
 GetIntegerOp(2,From,0,from);

 if (from>=lstr)
   {
    MLIRetInt(-1);
   }
 else
   {
    char *s=strstr(String->str+from,Search->str);
    if (!s)
      {
       MLIRetInt(-1);
      }
    else
      {
       MLIRetInt(s-String->str);
      }
   }

CleanUp:
 destroyFloatVar(String);
 destroyFloatVar(Search);
 destroyFloatVar(From);
}

//(strxlt string search replace)
DecFun(MLIBaseStrXlt)
{
 int slen,sc,ss,lstr;
 char search,replace,*str;
 LocVarStr(String);
 LocVarStr(Search);
 LocVarStr(Replace);

 CheckNumParams(cant!=3);

 GetString(0,String);
 GetString(1,Search);
 GetString(2,Replace);

 slen=min(Search->len,Replace->len);
 lstr=String->len;
 str=String->str;
 for (sc=0; sc<slen; sc++)
    {
     search=Search->str[sc];
     replace=Replace->str[sc];
     for (ss=0; ss<lstr; ss++)
         if (str[ss]==search)
            str[ss]=replace;
    }
 MLIRetStrLen(str,lstr);

CleanUp:
 destroyFloatVar(String);
 destroyFloatVar(Search);
 destroyFloatVar(Replace);
}

// (and v1 v2 [v3...]) Logical and
DecFun(MLIBaseAnd)
{
 CheckNumParams(cant<2);
 int i,value=1;
 TLispVar *p;

 for (i=0; value && i<cant; i++)
    {
     p=o->Solve(start+i);
     if (p)
       {
        value&=o->MLIBooleanValOf(p) ? 1 : 0;
        destroyFloatVar(p);
       }
     else
      {
       MLIRetNULL();
       return;
      }
    }
 MLIRetInt(value);
}

// (& v1 v2 [v3...]) Bitwise and
DecFun(MLIBaseAndB)
{
 CheckNumParams(cant<2);
 int i,value=0xFFFFFFFF;
 TLispVar *p;

 for (i=0; value && i<cant; i++)
    {
     p=o->Solve(start+i);
     if (p)
       {
        value&=o->MLIBooleanValOf(p);
        destroyFloatVar(p);
       }
     else
      {
       MLIRetNULL();
       return;
      }
    }
 MLIRetInt(value);
}

// (or v1 v2 [v3...]) Logical or
DecFun(MLIBaseOr)
{
 CheckNumParams(cant<2);
 int i,value=0;
 TLispVar *p;

 for (i=0; !value && i<cant; i++)
    {
     p=o->Solve(start+i);
     if (p)
       {
        value|=o->MLIBooleanValOf(p);
        destroyFloatVar(p);
       }
     else
      {
       MLIRetNULL();
       return;
      }
    }
 MLIRetInt(value);
}

// (| v1 v2 [v3...]) Bitwise or
DecFun(MLIBaseOrB)
{
 CheckNumParams(cant<2);
 int i,value=0;
 TLispVar *p;

 for (i=0; value!=(int)0xFFFFFFFF && i<cant; i++)
    {
     p=o->Solve(start+i);
     if (p)
       {
        value|=o->MLIBooleanValOf(p);
        destroyFloatVar(p);
       }
     else
      {
       MLIRetNULL();
       return;
      }
    }
 MLIRetInt(value);
}

DecFun(MLIBaseNotB)
{
 LocVar(value);

 CheckNumParams(cant!=1);
 GetVar(0,value);

 MLIRetInt(~o->MLIBooleanValOf(value));

CleanUp:
 destroyFloatVar(value);
}

DecFun(MLIBaseNot)
{
 LocVar(value);

 CheckNumParams(cant!=1);
 GetVar(0,value);

 MLIRetInt(o->MLIBooleanValOf(value) ? 0 : 1);

CleanUp:
 destroyFloatVar(value);
}

DecFun(MLIShortFileName)
{
 LocVarStr(File);
 CheckNumParams(cant!=1);
 GetString(0,File);
 char SFN[maxSFNSize];
 MLIRetString(GetShortNameOf(File->str,SFN));
CleanUp:
 destroyFloatVar(File);
}

static
void MLIBaseEqualFunc(TMLIBase *o,int start ,int cant, int type)
{
 int t1,t2,ret;
 LocVar(v1);
 LocVar(v2);

 // Right now we don't suport (== v1 v2 v3 ...)
 CheckNumParams(cant!=2);
 GetVar(0,v1);
 GetVar(1,v2);
 t1=GroupTypeOf(v1);
 t2=GroupTypeOf(v2);
 if (t1==MLIGString && t2==MLIGString)
   {
    TLispString *s1=(TLispString *)v1;
    TLispString *s2=(TLispString *)v2;
    ret=!strcmp(s1->str,s2->str);
    MLIRetInt(type ? ret : !ret);
   }
 else if (t1==MLIGInteger && t2==MLIGInteger)
   {
    TLispInteger *i1=(TLispInteger *)v1;
    TLispInteger *i2=(TLispInteger *)v2;
    ret=(i1->val==i2->val);
    MLIRetInt(type ? ret : !ret);
   }
 else
   {// Right now we don't support mixed types
    o->Error=MLIUndefOp;
    MLIRetNULL();
   }

CleanUp:
 destroyFloatVar(v1);
 destroyFloatVar(v2);
}

DecFun(MLIBaseEqual)
{
 MLIBaseEqualFunc(o,start,cant,1);
}

DecFun(MLIBaseNotEqual)
{
 MLIBaseEqualFunc(o,start,cant,0);
}

static
void MLIBaseIncDec(TMLIBase *o, int start, int cant, int delta)
{
 LocVar(targetVar);
 LocVar(Val);
 CheckNumParams(cant!=1);

 GetVar(0,Val);
 switch (GroupTypeOf(Val))
   {
    case MLIGString:
         // Something that was reduced to a string. Look-up a variable.
         targetVar=o->SearchVar(MLIAsStrVal(Val));
         CheckForError(!targetVar,MLIUndefVar);
         CheckForError(GroupTypeOf(targetVar)!=MLIGInteger,MLIUndefOp);
         MLIAsIntVal(targetVar)+=delta;
         MLIRetInt(MLIAsIntVal(targetVar));
         break;
    case MLIGInteger:
         // Something that was reduced to an int, just increment the value and return it
         MLIRetInt(MLIAsIntVal(Val)+delta);
         break;
    default:
         CheckForError(1,MLITypeParam)
   }

CleanUp:
 destroyFloatVar(Val);
}


// (++ value) | (++ "var_name")
DecFun(MLIBaseInc)
{
 MLIBaseIncDec(o,start,cant,1);
}

// (-- value) | (-- "var_name")
DecFun(MLIBaseDec)
{
 MLIBaseIncDec(o,start,cant,-1);
}

// (tostr <object>)
// converts a Lisp object into its "printed representation"
DecFun(MLIBaseToStr)
{
 CheckNumParams(cant!=1);

 char *str;
 LocVar(val);

 GetVar(0,val);
 str=val->toStr();
 destroyFloatVar(val);

 MLIRetStrLenExists(str,strlen(str));

CleanUp:
 return;
}

// (prex <string> <str_regex>)
// performs a Perl regex search
DecFun(MLIBasePRex)
{
 LocVarStr(String);
 LocVarStr(Search);
 PCREData prexS={0,NULL};
 pcre *prexC=NULL;

 CheckForError(!SUP_PCRE,MLINoPCRE);
 CheckNumParams(cant!=2);

 GetString(0,String);
 GetString(1,Search);

 PCREInitCompiler(prexS);
 prexC=PCRECompileRegEx(Search->str,prexS);
 CheckForError(!prexC,MLIPCRE);
 PCREStopCompiler(prexS);
 if (PCREDoSearch(String->str,String->len,prexC,prexS))
   {// Get the matches
    char b[32];
    int offset, len;
    for (int i=0; i<prexS.PCREHits; i++)
       {
        PCREGetMatch(i,offset,len,prexS);
        if (offset>=0)
          {
           CLY_snprintf(b,32,"_%d",i);
           o->AddVariable(b,new TLispString(String->str+offset,len));
          }
       }
    MLIRetInt(prexS.PCREHits);
   }
 else
   {
    MLIRetInt(0);
   }

CleanUp:
 destroyFloatVar(String);
 destroyFloatVar(Search);
 PCREDataDestroy(prexS);
 free(prexC);
}

// C like for(inic;condition;increment) code; => (inic cond inc code for)
/*DecFun(MLIBaseFor)
{
}*/

char *TMLIBase::cNames[MLIBaseCommands]=
{
 "print",
 "if",
 "left",
 "right",
 "substr",
 "gstr",
 "sstr",
 "setv",
 "eval",
 "strstr",
 "strxlt",
 "and",
 "or",
 "not",
 "ShortFileName",
 "strcmp",
 "strcasecmp",
 "length",
 "progn",
 "cond",
 "tostr",
 "loop",
 "exitloop",
 "repeat",
 "prex"/*,
 "for"*/
};

Command TMLIBase::cComms[MLIBaseCommands]=
{
 MLIBasePrint,
 MLIBaseIf,
 MLIBaseLeft,
 MLIBaseRight,
 MLIBaseSubStr,
 MLIBaseGStr,
 MLIBaseSStr,
 MLIBaseSetV,
 MLIBaseEval,
 MLIBaseStrStr,
 MLIBaseStrXlt,
 MLIBaseAnd,
 MLIBaseOr,
 MLIBaseNot,
 MLIShortFileName,
 MLIBaseStrCmp,
 MLIBaseStrCaseCmp,
 MLIBaseLength,
 MLIBaseEval,
 MLIBaseCond,
 MLIBaseToStr,
 MLIBaseLoop,
 MLIBaseExitLoop,
 MLIBaseRepeat,
 MLIBasePRex/*,
 MLIBaseFor*/
};

char *TMLIBase::sNames[MLIBaseSymbols]=
{
 "+",
 "&",
 "|",
 "-",
 "~",
 "==",
 "!=",
 "++",
 "--",
 "="
};

Command TMLIBase::sComms[MLIBaseSymbols]=
{
 MLIBaseAdd,
 MLIBaseAndB,
 MLIBaseOrB,
 MLIBaseSub,
 MLIBaseNotB,
 MLIBaseEqual,
 MLIBaseNotEqual,
 MLIBaseInc,
 MLIBaseDec,
 MLIBaseSetV
};

TLispConstString CRConstant("\r\n",2,0,2);

char *TMLIBase::cNamesConst[MLIBaseConstants]=
{
 "CR"
};

TLispVar *TMLIBase::cConstants[MLIBaseConstants]=
{
 &CRConstant
};


TMLIBase::TMLIBase(TMLIArrayBase *a, TLispVariableCol *v, FILE *out)
{
 array=a;
 Error=0;
 fileOut=out;
 EndCode=0;
 Vars=v;
 ErrorReported=1;
 ExitLoop=False;
}

TMLIBase::~TMLIBase()
{
 if (array)
   {
    delete array;
    array=0;
   }
 CLY_destroy(Vars);
 Vars=0;
}

int  TMLIBase::DuplicateVar(TLispVar *&aux,TLispVar *Value)
{
 switch (GroupTypeOf(Value))
   {
    case MLIGString:
         aux=new TLispString(((TLispString *)Value)->str,((TLispString *)Value)->len);
         break;
    case MLIGInteger:
         aux=new TLispInteger(((TLispInteger*)Value)->val);
         break;
    default:
         return 1;
   }
 return 0;
}

void TMLIBase::AddVariable(char *name, TLispVar *Value)
{
 ccIndex ind;
 TLispVariable *v;

 if (Value->type & 2)
   { // The value is the one from other variable, 2 vars can't have the same
     // value or a modification in one will be reflected in the other.
    TLispVar *aux;
    if (DuplicateVar(aux,Value))
       return;
    Value=aux;
   }

 Value->type|=2;
 if (Vars->search(name,ind))
   { // If already exists replace the value
    v=(TLispVariable *)(Vars->at(ind));
    delete v->val;
    v->val=Value;
   }
 else
   { // If not create it and insert in the right place
    v=new TLispVariable(name,Value);
    v->type|=2;
    Vars->atInsert(ind,v);
   }
}

TLispVar *TMLIBase::SearchVar(char *name)
{
 ccIndex ind;
 TLispVariable *v;

 if (Vars->search(name,ind))
   {
    v=(TLispVariable *)(Vars->at(ind));
    return v->val;
   }
 return NULL;
}

TLispVar *TMLIBase::Solve(int i)
{
 TLispVar *o=array->Get(i);

 int type=GroupTypeOf(o);
 if (type==MLIGCode)
    return Interpret(((TLispCode *)o)->start);
 else
   if (type==MLIGVar)
      return ((TLispVariable *)o)->val;
 // Make it a floating var
 //o->type|=1;
 return o;
}

Command TMLIBase::WhatCommand(char *s)
{
 int i;
 for (i=0; i<MLIBaseCommands; i++)
     if (strcmp(s,cNames[i])==0)
        return cComms[i];
 return 0;
}

TLispVar *TMLIBase::WhatConstant(char *s)
{
 int i;
 for (i=0; i<MLIBaseConstants; i++)
     if (strcmp(cNamesConst[i],s)==0)
        return cConstants[i];
 return 0;
}

Command TMLIBase::WhatSymbol(char *s)
{
 int i;
 for (i=0; i<MLIBaseSymbols; i++)
     if (strcmp(s,sNames[i])==0)
        return sComms[i];
 return 0;
}

char *TMLIBase::SkipCode(char *&code, int &error)
{
 char *s=code+1,*start;
 int level;

 for (level=0; *s && (*s!=')' || level!=0); s++)
    {
     if (*s=='"')
       {
        start=s;
        for (s++; *s && *s!='"'; s++)
            if (*s=='\\')
               s++;
        if (*s!='"')
          {
           code=start;
           error=MLIEStrNoEnd;
           return 0;
          }
       }
     else
     if (*s=='\'')
       {
        start=s;
        for (s++; *s && *s!='\''; s++)
            if (*s=='\\')
               s++;
        if (*s!='\'')
          {
           code=start;
           error=MLIEStrNoEnd;
           return 0;
          }
       }
     else
     if (*s==';')
       {
        for (s++; *s && *s!='\n'; s++);
       }
     else
     if (*s=='(')
        level++;
     else
     if (*s==')')
        level--;
    }
 if (*s!=')')
   {
    error=MLIENoEnd;
    return 0;
   }
 return s;
}

char *TMLIBase::SkipCode()
{
 return SkipCode(Code,Error);
}

TLispVar *TMLIBase::Interpret(char *s)
{
 int stkPos=array->GetCount();
 TLispVar *ret=InterpretNoClean(s);
 if (ret==NULL)
   {// Now we can get the error while we are sure the code exists.
    // The code could be a variable in the stack and be freed soon.
    CopyCodeError();
    // If we parsed some parameters and found an error before finishing
    // destroy these entries in the stack.
    if (array->firstfree>stkPos)
       array->FreeItems(array->firstfree-stkPos-1);
   }
 return ret;
}

TLispVar *TMLIBase::InterpretNoClean(char *s)
{
 int Params=0;
 int Commands=0;
 int stkPos=array->GetCount();
 char *start,*end;
 // Skip whitespace, macros typed by the user could have it
 while (ucisspace(*s)) s++;
 StartCode=Code=s;
 if (*s!='(')
   {
    Error=MLINoIniPar;
    return NULL;
   }
 Code++;

 while (*Code)
   {
    for (;*Code && ucisspace(*Code); Code++);
    switch (*Code)
      {
       case '(':
            if (Commands==0)
              {
               Error=MLINoCommands;
               return NULL;
              }
            start=Code;
            end=SkipCode();
            if (!end)
               return NULL;
            Code=end+1;
            array->Push(new TLispCode(start,Code));
            Params++;
            break;
       case ')':
            {
             if (Code>EndCode)
                EndCode=Code;
             if (Commands==0)
               {
                Error=MLINoCommands;
                if (Params)
                   array->FreeItems(Params-1);
                return NULL;
               }
             TLispCommand *c=(TLispCommand *)(array->Get(stkPos));
             (c->command)(this,stkPos+1,Params);
             if (Error)
               {
                array->FreeItems(Params);
                return NULL;
               }
             return array->FreeItems(Params);
            }
       case '"':
       case '\'':
            if (Commands==0)
              {
               Error=MLINoCommands;
               return NULL;
              }
            if (ParseString(*Code))
              {
               Error=MLIEStrNoEnd;
               return NULL;
              }
            Params++;
            break;
       case ';':
            while (*Code!='\n' && *Code) Code++;
            break;
       default:
            if (TVCodePage::isAlpha(*Code) || *Code=='_')
              { // That could be a var or a command
               if (ParseVarOrCommand(Params,Commands))
                  return NULL;
               if (Commands>1)
                 {
                  Error=MLITooCommands;
                  return NULL;
                 }
              }
            else
            if ((*Code=='-' && ucisdigit(Code[1])) || ucisdigit(*Code))
              {
               if (Commands==0)
                 {
                  Error=MLINoCommands;
                  return NULL;
                 }
               if (ParseNumber())
                 {
                  Error=MLIWrongNumber;
                  return NULL;
                 }
               Params++;
              }
            else
              {
               int OldCommands=Commands;
               if (ParseSymbol(Params,Commands))
                 {
                  Error=MLIUnknownVal;
                  return NULL;
                 }
               if (OldCommands!=Commands && Commands!=1)
                 {
                  Error=MLITooCommands;
                  return NULL;
                 }
               if (Commands==0)
                 {
                  Error=MLINoCommands;
                  return NULL;
                 }
              }
      }
    if (array->GetCount()>maxNestedsLisp)
      {
       Error=MLIStakOverf;
       return NULL;
      }
   }
 Error=MLIENoEnd;
 return NULL;
}

int TMLIBase::ParseSymbol(int &,int &Commands)
{
 Command p;
 char *start,*end,v;
 start=end=Code;

 for (;*end && !ucisspace(*end) && *end!='('; end++);
 Code=end;

 v=*end;
 *end=0;
 p=WhatSymbol(start);
 *end=v;
 if (p)
   {
    array->Push(new TLispCommand(p));
    Commands++;
   }
 else
   {
    Error=MLIUndefSymbol;
    return 1;
   }
 return 0;
}

int TMLIBase::ParseVarOrCommand(int &Params,int &Commands)
{
 char *s=Code;
 char *start=s,v;
 Command p;

 for (;*s && (TVCodePage::isAlNum(*s) || *s=='_'); s++);
 Code=s;
 v=*s;
 *s=0;
 p=WhatCommand(start);
 if (p)
   {
    array->Push(new TLispCommand(p));
    Commands++;
   }
 else
   {
    ccIndex pos;
    if (Vars->search(start,pos))
      {
       array->Push((TLispVar *)(Vars->at(pos)));
       Params++;
      }
    else
      { // Now try a constant
       TLispVar *constant=WhatConstant(start);
       if (constant)
         {
          array->Push(constant);
          Params++;
         }
       else
         {
          *s=v;
          Error=MLIUndefVar;
          return 1;
         }
      }
   }
 *s=v;
 return 0;
}

int TMLIBase::ParseString(char end)
{
 char *s=Code+1;
 char *start=s;

 for (;*s && *s!=end; s++)
     if (*s=='\\')
        s++;
 if (*s!=end)
    return 1;
 *s=0;
 array->Push(new TLispString(start,tlsParse));
 *s=end;
 Code=s+1;
 return 0;
}

int TMLIBase::ParseNumber()
{
 char *end;
 array->Push(new TLispInteger(strtol(Code,&end,0)));
 Code=end;
 return !(ucisspace(*end) || *end=='(' || *end==')' || *end==';');
}

#define maxParse 6
#define maxSyntax 11

static char *UnkErr=__("unknown");

char *TMLIBase::TypeError[]=
{
 UnkErr,
 __("parser"),
 __("syntax")
};

char *TMLIBase::ParseError[]=
{
 __("string not closed"),
 __("no ) at end of code"),
 __("no starting ("),
 __("wrong number"),
 __("unknown value"),
 __("stack overflow")
};

char *TMLIBase::SyntaxError[]=
{
 __("command where a parameter was expected"),
 __("command expected"),
 __("parameter after command"),
 __("wrong number of parameters"),
 __("wrong parameter type"),
 __("undefined variable"),
 __("undefined symbol"),
 __("operation not defined"),
 __("invalid character for a name"),
 __("invalid key sequence"),
 __("PCRE not available"),
 __("PCRE compile error")
};

char *TMLIBase::GetTypeError()
{
 int type=Error>>12;
 if (type<0 || type>2)
    type=0;
 return TypeError[type];
}


char *TMLIBase::GetError()
{
 int type=Error>>12;
 int subtype=Error & 0xFFF;

 switch (type)
   {
    case 1:
         if (subtype>maxParse)
            return UnkErr;
         return ParseError[subtype];
    case 2:
         if (subtype>maxSyntax)
            return UnkErr;
         return SyntaxError[subtype];
   }
 return UnkErr;
}

static char BufError[64];

char *TMLIBase::GetCodeError()
{
 ErrorReported=1; // Now we are ready to memorize another error
 return BufError;
}

char *TMLIBase::CopyCodeError()
{// Remmember the error the first time we are called.
 if (!ErrorReported) return 0;
 ErrorReported=0;

 char *start=Code-30;
 memset(BufError,0,64);
 if (start<StartCode)
    start=StartCode;
 int cant=Code-start;
 strncpy(BufError,start,cant);
 strcat(BufError,"<*>");
 strncat(BufError,Code,30);
 return BufError;
}


#ifdef TEST

char *Code="(('varA' 'Hola que tal' setv) ((varA 4 left) CR (varA 5 substr) print) eval)";
//char *Code="(('varA' 'Hola que tal' setv) ((varA 4 left) CR (varA 5 substr) print) eval)";
//char *Code="(0 ('hola' print) ('Es 0' print) if)";
//char *Code="(\"hola\" (\"chau\" print) print)";

int main()
{
 TMLIArraySimple a(10,10);
 TLispVariableCol v(5,5);
 TMLIBase Interpreter(&a,&v,stdout);
 printf("\nOutput of program:\n");
 TLispVar *val=Interpreter.Interpret(Code);
 printf("\nReturned:\n");
 if (val)
   {
    val->print(stdout);
    printf("\nElements in stack: %d\n",Interpreter.array->GetCount());
    destroyFloatVar(val);
   }
 else
    printf("Error: %X\n",Interpreter.Error);
 return 0;
}
#endif
