/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TMLISDGDefs) && !defined(__TMLISDGDefs__)
#define __TMLISDGDefs__
typedef int (*getVarFunction)(int,char *&,char *&,int &);
extern char *InterpretLispCode(char *s,getVarFunction getVar,FILE *f);

// Default: Don't free it, belongs to the generator
#define sdgFreeIt       1
// Default: Out it expanding
#define sdgSimpleString 2
// Default: end marks the end
#define sdgASCIIZ       4

extern int MLISDGError;
extern char *MLISDGTypeError;
extern char *MLISDGErrorName;
extern char *MLISDGErrorCode;
#endif
