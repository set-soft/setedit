/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TMLIBase) && !defined(__TMLIBase__)
#define __TMLIBase__
const int MLIBaseCommands=25;
const int MLIBaseConstants=1;
const int MLIBaseSymbols=10;

class TMLIBase
{
public:
 TMLIBase(TMLIArrayBase *a, TLispVariableCol *v, FILE *out);
 virtual ~TMLIBase();
 TLispVar *Interpret(char *);
 int ParseString(char s);
 int ParseNumber();
 int ParseVarOrCommand(int &Params,int &Commands);
 virtual int ParseSymbol(int &Params,int &Commands);
 virtual int MLIBooleanValOf(TLispVar *var);
 virtual Command   WhatCommand(char *s);
 virtual Command   WhatSymbol(char *s);
 virtual TLispVar *WhatConstant(char *s);
 char *SkipCode();
 static char *SkipCode(char *&code, int &error);
 TLispVar *Solve(int i);
 virtual char *GetTypeError();
 virtual char *GetError();
 char *GetCodeError();
 void AddVariable(char *name, TLispVar *Value);
 int  DuplicateVar(TLispVar *&aux,TLispVar *Value);
 TLispVar *SearchVar(char *name);

 int Error;
 TMLIArrayBase *array;
 TLispVariableCol *Vars;
 char *EndCode;
 char *Code;
 Boolean ExitLoop;  // Used to indicate we found an exitloop sentence

private:
 char *CopyCodeError();
 TLispVar *InterpretNoClean(char *s);
 
 FILE *fileOut;
 static char *cNames[MLIBaseCommands];
 static Command cComms[MLIBaseCommands];
 static char *sNames[MLIBaseSymbols];
 static Command sComms[MLIBaseSymbols];
 static char *TypeError[];
 static char *ParseError[];
 static char *SyntaxError[];
 static char *cNamesConst[MLIBaseConstants];
 static TLispVar *cConstants[MLIBaseConstants];
 char  ErrorReported;
 char *StartCode;

 friend void MLIBasePrint(TMLIBase *o,int start ,int cant);
};

void MLIRetNULL(TMLIBase *o,int stkPos);
void MLIRetObj(TMLIBase *o,int stkPos,TLispVar *v);
void MLIRetString(TMLIBase *o,int stkPos,char *str);
#define MLIRetInt(val) \
        o->array->ReplaceItem(start-1,new TLispInteger(val));
#define MLIRetStrLen(str,len) \
        o->array->ReplaceItem(start-1,new TLispString(str,len))
#define MLIRetStrLenExists(str,len) \
        o->array->ReplaceItem(start-1,new TLispString(str,len,StrAlreadyAllocated))
#define MLIRetObj(v) \
        do { \
        if (v) v->type|=1; \
        o->array->ReplaceItem(start-1,v); \
        } while(0)
#define MLIRetNULL() \
        o->array->ReplaceItem(start-1,NULL)
#define MLIRetString(str) \
        o->array->ReplaceItem(start-1,new TLispString(str));

#define GetString(pos,obj) \
{ obj=(TLispString *)o->Solve(start+pos); \
  if(!obj) goto CleanUp; \
  if ((obj->type>>12)!=MLIGString) \
    { o->Error=MLITypeParam; goto CleanUp; } \
}
#define GetInteger(pos,obj) \
{ obj=(TLispInteger *)o->Solve(start+pos); \
  if(!obj) goto CleanUp; \
  if ((obj->type>>12)!=MLIGInteger) \
    { o->Error=MLITypeParam; goto CleanUp; } \
}
#define GetVar(pos,obj) \
{ obj=o->Solve(start+pos); if(!obj) goto CleanUp;}
#define GetIntegerOp(pos,obj,ops,dest) \
{ if (pos>=cant)  \
    { dest=ops; } \
 else \
    { obj=(TLispInteger *)o->Solve(start+pos); \
      if(!obj) goto CleanUp; \
      if ((obj->type>>12)!=MLIGInteger) \
        { o->Error=MLITypeParam; goto CleanUp; } \
      dest=obj->val; } }
#define GetCode(pos,obj) \
{ obj=(TLispCode *)(o->array->Get(start+pos)); \
  if(!obj) goto CleanUp; \
  if ((obj->type>>12)!=MLIGCode) \
    { o->Error=MLITypeParam; goto CleanUp; } \
}
#define CheckNumParams(cond) \
{ if (cond) \
    { o->Error=MLINumParam; \
      MLIRetNULL(); \
      return; } }
#define CheckForError(cond,error) \
{ if (cond) \
    { o->Error=error; \
      MLIRetNULL(); \
      goto CleanUp; } }

#define MLIAsInt(var) ((TLispInteger *)var)
#define MLIAsStr(var) ((TLispString *)var)
#define MLIAsIntVal(var) (((TLispInteger *)var)->val)
#define MLIAsStrVal(var) (((TLispString *)var)->str)

#define DecFun(a) void a(TMLIBase *o,int start ,int cant)
#define LocVarStr(a) TLispString *a=NULL
#define LocVarInt(a) TLispInteger *a=NULL
#define LocVarCode(a) TLispCode *a=NULL
#define LocVar(a)    TLispVar *a=NULL

#endif
