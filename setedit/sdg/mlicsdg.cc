/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

 Module: Lisp Interpreter for FRT files
 Comments:
 This module defines a derived class from TMLIBase that's suitable for FRT
files. @x{TMLIBase (class)}.@p
 The derived interpreter adds support for SDG variables and adds commands
like cutCprot.

***************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "ucdefs.h"

#define Uses_TLispVariableCol
#define Uses_TMLIBase
#define Uses_TLispBaseVars
#define Uses_TLispVarDefs
#define Uses_TMLIArrayBase
#define Uses_TMLISDG
#define Uses_TLispSDGstring
#include <mli.h>

static char *emptyStr="";
extern char *TakeNum(char *s,int *num);

int TMLISDG::ParseSymbol(int &Params,int &Commands)
{
 int num;
 char *s,*e;
 int flags;

 if (*Code=='~' && ucisdigit(*(Code+1)))
   {
    Code=TakeNum(Code+1,&num);
    if (getVar(num,s,e,flags))
       array->Push(new TLispSDGstring(s,e+1,flags));
    else
      {
       s=e=emptyStr;
       flags=sdgSimpleString | sdgASCIIZ;
       array->Push(new TLispSDGstring(s,e,flags));
      }
    Params++;
    return 0;
   }
 return TMLIBase::ParseSymbol(Params,Commands);
}


int TMLISDG::MLIBooleanValOf(TLispVar *var)
{
 if (GroupTypeOf(var)==MLIGSDGvar)
   {
    TLispSDGstring *s=(TLispSDGstring *)var;
    return s->start && s->start[0];
   }
 return TMLIBase::MLIBooleanValOf(var);
}

Command TMLISDG::WhatCommand(char *s)
{
 int i;
 for (i=0; i<MLISDGCommands; i++)
     if (strcmp(s,cNames[i])==0)
        return cComms[i];
 return TMLIBase::WhatCommand(s);
}

typedef struct
{
 char *buf;
 int  len,pos;
} strCatStruct;

void StrCatInit(char *buf,int len,strCatStruct &s)
{
 s.buf=buf;
 s.len=len-1;
 s.pos=0;
}

void StrCatSpaces(int cant,strCatStruct &s)
{
 if (s.pos+cant>s.len)
    cant=s.len-s.pos;
 for (; cant; s.pos++,cant--) s.buf[s.pos]=' ';
 s.buf[s.pos]=0;
}

void StrCat(char *p,strCatStruct &s)
{
 for (; *p && s.pos<s.len; s.pos++,p++) s.buf[s.pos]=*p;
 s.buf[s.pos]=0;
}

void StrCat(char *p,int cant,strCatStruct &s)
{
 for (; cant && *p && s.pos<s.len; s.pos++,p++,cant--) s.buf[s.pos]=*p;
 s.buf[s.pos]=0;
}

void StrCat(char val,strCatStruct &s)
{
 if (s.pos<s.len)
   {
    s.buf[s.pos]=val;
    s.buf[++s.pos]=0;
   }
}

/**[txh]********************************************************************

  Description:
  That's the C++ implementation of the sLisp cutCprot command. It reformats
a C prototype to feet in a requested page width. The routine cuts the
declaration only after a parameter an tries to align all using the first
parenthesis as reference.

***************************************************************************/

void MLISDGcutCprot(TMLIBase *o,int start ,int cant)
{
 TLispVar *string=NULL;
 TLispVar *integer1=NULL;
 TLispVar *integer2=NULL;
 int error=0,len,indent,lstr;
 char *s;
 strCatStruct cat;

 if (cant!=3)
   {
    error=MLINumParam;
    goto CleanUp;
   }
 // Get the first parameter
 string=o->Solve(start);
 if (GroupTypeOf(string)!=MLIGSDGvar && GroupTypeOf(string)!=MLIGString)
   {
    error=MLITypeParam;
    goto CleanUp;
   }
 if (GroupTypeOf(string)==MLIGSDGvar)
   {
    TLispSDGstring *p=(TLispSDGstring *)string;
    s=p->start;
    if (p->flags & sdgASCIIZ)
       lstr=strlen(s);
    else
       lstr=p->end-s;
   }
 else
   {
    TLispString *p=(TLispString *)string;
    s=p->str;
    lstr=p->len;
   }

 // Get the second
 integer1=o->Solve(start+1);
 if (GroupTypeOf(integer1)!=MLIGInteger)
   {
    error=MLITypeParam;
    goto CleanUp;
   }
 len=((TLispInteger *)integer1)->val;

 // And now the third
 integer2=o->Solve(start+2);
 if (GroupTypeOf(integer2)!=MLIGInteger)
   {
    error=MLITypeParam;
    goto CleanUp;
   }
 indent=((TLispInteger *)integer2)->val;

 // After almost 1Kb of C code I have the parameters :-(
 #define maxParams 20
 char *Params[maxParams],*param,v;
 int  Lens[maxParams],numparams,i;
 numparams=1;
 v=s[lstr];
 s[lstr]=0;
 Params[0]=s;
 for (param=s; *param && *param!='('; param++);
 Lens[0]=param-s+1;
 if (*param)
   {
    int level;
    param++;
    do
      {
       for (;*param && ucisspace(*param); param++);
       Params[numparams]=param;
       for (level=0; *param && !((*param==')' || *param==',') && level==0); param++)
           if (*param=='(')
              level++;
           else
             if (*param==')')
                level--;
       if (*param)
          param++;
       Lens[numparams]=param-Params[numparams];
       numparams++;
      }
    while (*param && numparams!=(maxParams-1));
    if (*param)
      {
       Params[numparams]=++param;
       Lens[numparams]=strlen(Params[numparams]);
       numparams++;
      }

    int maxLpar=0;
    for (i=1; i<numparams; i++)
        if (Lens[i]>maxLpar)
           maxLpar=Lens[i];

    int parIndent;
    if (len-maxLpar>Lens[0]+indent)
       parIndent=Lens[0]+indent;
    else
      if (len-maxLpar<indent)
         parIndent=indent;
      else
         parIndent=len-maxLpar;

    int maximorum=parIndent+maxLpar+2;
    if (Lens[0]+indent+2>maximorum)
       maximorum=Lens[0]+indent+2;
    char *rstr=new char[numparams*maximorum];

    StrCatInit(rstr,numparams*maximorum,cat);
    StrCatSpaces(indent,cat);
    StrCat(Params[0],Lens[0],cat);
    int col=indent+Lens[0];
    for (i=1; i<numparams; i++)
       {
        if (col+Lens[i]>len)
          {
           StrCat('\n',cat);
           StrCatSpaces(parIndent,cat);
           StrCat(Params[i],Lens[i],cat);
           col=parIndent+Lens[i];
          }
        else
          {
           StrCat(Params[i],Lens[i],cat);
           col+=Lens[i];
          }
       }
    s[lstr]=v;
    MLIRetString(rstr);
   }

CleanUp:
 destroyFloatVar(string);
 destroyFloatVar(integer1);
 destroyFloatVar(integer2);
 if (error)
   {
    o->Error=error;
    MLIRetNULL();
   }
 return;
}

char *TMLISDG::cNames[MLISDGCommands]=
{
 "cutCprot"
};

Command TMLISDG::cComms[MLISDGCommands]=
{
 MLISDGcutCprot
};

