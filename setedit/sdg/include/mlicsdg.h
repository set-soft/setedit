/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TMLISDG) && !defined(__TMLISDG__)
#define __TMLISDG__
const int MLISDGCommands=1;

class TMLISDG : public TMLIBase
{
public:
 TMLISDG(TMLIArrayBase *a, TLispVariableCol *v, FILE *f) : TMLIBase(a,v,f) {};
 void SetGetVar(getVarFunction gv) { getVar=gv; };
 virtual int ParseSymbol(int &Params,int &Commands);
 virtual int MLIBooleanValOf(TLispVar *var);
 virtual Command WhatCommand(char *s);

private:
 static char *cNames[MLISDGCommands];
 static Command cComms[MLISDGCommands];
 getVarFunction getVar;
};
#endif
