/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#include <ctype.h>
#define Uses_TNoCaseStringCollection
#include <settvuti.h>
#define Uses_TLispVariableCol
#define Uses_TMLIEditor
#define Uses_TMLIArraySimple
#define Uses_TLispVar
#include <mli.h>
#include <edmsg.h>

int   MLIEditorError;
char *MLIEditorTypeError;
char *MLIEditorErrorName;
char *MLIEditorErrorCode;

static TMLIArraySimple  *array=0;
static TLispVariableCol *vars=0;
static TMLIEditor       *Interpreter=0;
static int okInit=0;

int InitLispEditor(void)
{
 if (!okInit)
   {
    array=new TMLIArraySimple(10,10);
    vars=new TLispVariableCol(5,5);
    if (array && vars)
      {
       Interpreter=new TMLIEditor(array,vars,stdout);
       if (Interpreter)
         {
          okInit=1;
         }
      }
   }
 return okInit;
}

void DeInitLispEditor(void)
{
 if (Interpreter)
    delete Interpreter;
 Interpreter=0;
}

char *InterpretLispEditor(char *s, Boolean print)
{
 if (!okInit)
   {
    MLIEditorError=0;
    MLIEditorTypeError=MLIEditorErrorName=MLIEditorErrorCode="";
    return NULL;
   }

 if (print)
    EdShowMessage("Running sLisp macro ...",True);
 Interpreter->Error=0;
 TLispVar *val=Interpreter->Interpret(s);

 if (!val)
   {
    MLIEditorError=Interpreter->Error;
    MLIEditorTypeError=Interpreter->GetTypeError();
    MLIEditorErrorName=Interpreter->GetError();
    MLIEditorErrorCode=Interpreter->GetCodeError();
    return NULL;
   }
 else
   {
    if (print)
      {
       char *v=val->toStr();
       EdShowMessageI(__("Return value:"));
       EdShowMessage(v);
       delete[] v;
      }
    destroyFloatVar(val);
   }

 return Interpreter->EndCode+1;
}

int InterpretLispEditorFile(char *s)
{
 while (*s)
   {
    if (*s==';')
      {
       while (*s && *s!='\n') s++;
       if (!*s)
          break;
      }
    else
    if (*s=='(')
      {
       s=InterpretLispEditor(s);
       if (!s)
          return 0;
      }
    s++;
   }
 return 1;
}

extern ccIndex SLPChoose(TNoCaseStringCollection *);
static ccIndex LastChoose=-1;

int ChooseAndRunLispEditor(void)
{
 if (!okInit)
    return SLP_NO_INIT;

 TMacrosColl *Col=Interpreter->Macros;

 int cant=Col->getCount();
 if (!cant)
    return SLP_NO_MACROS;

 ccIndex ind=SLPChoose(Col);
 if (ind<0)
    return SLP_OK;

 LastChoose=ind;
 MLIMacro *m=(MLIMacro *)(Col->at(ind));
 return InterpretLispEditor(m->start)==NULL ? SLP_ERROR : SLP_OK;
}

TNoCaseStringCollection *GetMacrosList(void)
{
 if (!okInit)
    return 0;

 return Interpreter->Macros;
}

int ReRunLastChooseLispEditor(void)
{
 if (!okInit)
    return SLP_NO_INIT;

 TNoCaseStringCollection *Col=Interpreter->Macros;
 if (LastChoose<0 || LastChoose>=Col->getCount())
    return SLP_NO_CHOOSE;

 MLIMacro *m=(MLIMacro *)(Col->at(LastChoose));
 return InterpretLispEditor(m->start)==NULL ? SLP_ERROR : SLP_OK;
}

int MLIEdSeachAndRunCom(char *name, Boolean verbose)
{
 if (!okInit)
    return SLP_NO_INIT;

 TNoCaseStringCollection *Col=Interpreter->Macros;
 ccIndex i;
 ccIndex c=Col->getCount();
 for (i=0; i<c; i++)
    {
     MLIMacro *m=(MLIMacro *)(Col->at(i));
     if (strcmp(m->Name,name)==0)
        return InterpretLispEditor(m->start,verbose)==NULL ? SLP_ERROR : SLP_OK;
    }
 return SLP_NO_CHOOSE;
}

int MLIEdIsolateCode(char *start, char *&end)
{
 int error;
 char *ret=TMLIBase::SkipCode(start,error);
 if (!ret)
    return SLP_ERROR;
 end=ret;
 return SLP_OK;
}

