/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
class TMLIArrayBase;
class TMLIBase;

#if defined(Uses_TLispVarDefs) && !defined(__TLispVarDefs__)
#define __TLispVarDefs__

typedef void (*Command)(TMLIBase *a,int start ,int cant);

const int
      MLITypeString =0x1000,
      MLITypeInteger=0x2000,
      MLITypeCommand=0x3000,
      MLITypeVar    =0x4000,
      MLITypeCode   =0x5000,
      MLITypeSDGvar =0x6000,
      MLITypeKeyBind=0x7000; // Used by the editor

const int
      MLIGString =0x1,
      MLIGInteger=0x2,
      MLIGCommand=0x3,
      MLIGVar    =0x4,
      MLIGCode   =0x5,
      MLIGSDGvar =0x6,
      MLIGKeyBind=0x7;

const int
      MLIEStrNoEnd  =0x1000, // Parse errors
      MLIENoEnd     =0x1001,
      MLINoIniPar   =0x1002,
      MLIWrongNumber=0x1003,
      MLIUnknownVal =0x1004,
      MLIStakOverf  =0x1005,
      MLITooCommands=0x2000, // Syntax errors
      MLINoCommands =0x2001,
      MLIComOrder   =0x2002,
      MLINumParam   =0x2003,
      MLITypeParam  =0x2004,
      MLIUndefVar   =0x2005,
      MLIUndefSymbol=0x2006,
      MLIUndefOp    =0x2007,
      MLIInvaForName=0x2008,
      MLIInvaKeySeq =0x2009,
      MLINoPCRE     =0x200a,
      MLIPCRE       =0x200b;

// Just an arbitrary limit to stop infinite recursion that will end in a crash
const int maxNestedsLisp=1024;
#endif

#if defined(Uses_TLispVar) && !defined(__TLispVar__)
#define __TLispVar__
class TLispVar
{
public:
 TLispVar() {};
 virtual ~TLispVar();
 virtual int print(FILE *) = 0;
 virtual char *toStr() = 0;

 int type;
};

// Destroys a variables from the stack (array)
inline void destroyVar(TLispVar *v)
{
 if (v && !(v->type & 3))
   {
    //fprintf(stderr,"Liberando %X del array\n",(unsigned)v);
    delete v;
   }
}

// Destroys a floating variable, that's a returned value
inline void destroyFloatVar(TLispVar *v)
{
 if (v && (v->type & 3)==1)
   {
    //fprintf(stderr,"Liberando %X flotante\n",(unsigned)v);
    delete v;
   }
}

#define GroupTypeOf(a) ((a->type)>>12)
#define GroupValOf(a) ((a)>>12)
#endif


#if defined(Uses_TLispBaseVars) && !defined(__TLispBaseVars__)
#define __TLispBaseVars__
const int StrOwner=1,StrAlreadyAllocated=2;
typedef enum { tlsRaw, tlsParse } tlsMode;

class TLispString : public TLispVar
{
public:
 TLispString(const char *s, tlsMode mode=tlsRaw);
 TLispString(const char *s, int len, int flags=StrOwner, int extraType=0);
 TLispString() {}; // Default constructor does nothing, needed by TLispConstString
 virtual ~TLispString();
 virtual int print(FILE *s);
 virtual char *toStr();

 char *str;
 int len;
 int flags;
};

class TLispConstString : public TLispString
{
public:
 TLispConstString(const char *s);
 TLispConstString(const char *s, int len, int flags=StrOwner, int extraType=0);
 virtual ~TLispConstString();
};

class TLispInteger : public TLispVar
{
public:
 TLispInteger(int v);
 virtual ~TLispInteger() {};
 virtual int print(FILE *s);
 virtual char *toStr();

 int val;
};

class TLispCommand : public TLispVar
{
public:
 TLispCommand(Command c);
 virtual ~TLispCommand() {};
 virtual int print(FILE *s);
 virtual char *toStr();

 Command command;
};

class TLispVariable : public TLispVar
{
public:
 TLispVariable(char *v, TLispVar *o);
 virtual ~TLispVariable();
 virtual int print(FILE *s);
 virtual char *toStr();

 TLispVar *val;
 char *var;
};

class TLispCode : public TLispVar
{
public:
 TLispCode(char *s, char *e);
 virtual ~TLispCode() {};
 virtual int print(FILE *s);
 virtual char *toStr();

 char *start;
 char *end;
};
#endif


#if defined(Uses_TLispVariableCol) && !defined(__TLispVariableCol__)
#define __TLispVariableCol__

class TLispVariableCol : public TNSSortedCollection
{
public:
 TLispVariableCol(ccIndex aLimit, ccIndex aDelta) :
      TNSSortedCollection(aLimit,aDelta) {};
 int compare(void *s1,void *s2);
 void freeItem(void *s);
 void *keyOf(void *s);
};
#endif
