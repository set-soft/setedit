/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#define Uses_TLispVariableCol
#define Uses_TMLISDG
#define Uses_TLispSDGstring
#define Uses_TMLIArraySimple
#include <mli.h>

extern void OutPutString(char *s, char *e,FILE *f);
extern char *TakeNum(char *s,int *num);

TLispSDGstring::~TLispSDGstring()
{
 if (flags & sdgFreeIt)
    delete start;
}

int TLispSDGstring::print(FILE *s)
{
 char v=0; // can be used unitialized!?
 int ret;

 if (flags & sdgSimpleString)
   {
    if (!(flags & sdgASCIIZ))
      {
       v=*end;
       *end=0;
      }
    fputs(start,s);
    if (flags & sdgASCIIZ)
       ret=strlen(start);
    else
      {
       ret=end-start;
       *end=v;
      }
   }
 else
   {
    if (flags & sdgASCIIZ)
      {
       ret=strlen(start);
       OutPutString(start,start+ret,s);
      }
    else
      {
       ret=end-start;
       OutPutString(start,end,s);
      }
   }
 return ret;
}

char *TLispSDGstring::toStr()
{// Not implemented
 return newStr("");
}

int MLISDGError;
char *MLISDGTypeError;
char *MLISDGErrorName;
char *MLISDGErrorCode;

char *InterpretLispCode(char *s,getVarFunction getVar,FILE *f)
{
 TMLIArraySimple a(10,10);
 TLispVariableCol v(5,5);
 TMLISDG Interpreter(&a,&v,f);
 Interpreter.SetGetVar(getVar);
 TLispVar *val=Interpreter.Interpret(s);
/* if (val)
   {
    val->print(stdout);
    printf("\nElements in stack: %d\n",Interpreter.array->GetCount());
    destroyFloatVar(val);
   }
 else*/
 if (!val)
   {
    MLISDGError=Interpreter.Error;
    MLISDGTypeError=Interpreter.GetTypeError();
    MLISDGErrorName=Interpreter.GetError();
    MLISDGErrorCode=Interpreter.GetCodeError();
    return NULL;
   }
 else
    destroyFloatVar(val);
 return Interpreter.EndCode+1;
}
