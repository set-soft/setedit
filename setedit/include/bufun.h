/* Copyright (C) 2000-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  bufun.cc definitions.
  They are needed for the parsers for languages other than C and the
  editor class.

***************************************************************************/

#ifndef BUFUN_H_INCLUDED
#define BUFUN_H_INCLUDED

typedef void (*tAddFunc)(char *name, int len, int lineStart, int lineEnd);
const int MaxLenWith0=256,MaxLen=255,MaxLen_1=254;

// Generic use buffers of MaxLenWith0 bytes, can be used by all parsers
extern char bfBuffer[];
extern char bfNomFun[];
extern char bfTempNomFun[];

// This structure holds the parsers we have.
// Each parser must export a `func' function.
// Prefferred names are SearchLanguageFuncs
typedef struct
{
 const char *shl;
 int (*func)(char *buffer, unsigned len, int mode, tAddFunc AddFunc);
} stFuncsSHL;

class SOStack;
class TNoCaseSOSStringCollection;

const int modeBFPrototypes=0, modeBFFunctions=1, modeBFClassSep=2;
extern int SelectFunctionToJump(char *b, unsigned l, char *word, int mode,
                                char *fileName, char *shl);
extern void DestroyFunctionList();
extern int CreateFunctionList(char *b, unsigned l, const char *fileName,
                              unsigned ID, char *shl);
extern int CreateFunctionList(char *b, unsigned l, SOStack &stk,
                              TNoCaseSOSStringCollection *FunList, unsigned ops,
                              char *shl);
extern int SearchFunctionByLine(int line, int &start, int &end, char *&name);

// Other parsers:
extern int SearchClipperFuncs(char *b, unsigned l, int mode, tAddFunc AddFunc);
extern int SearchPerlFuncs(char *b, unsigned l, int mode, tAddFunc AddFunc);
extern int SearchSHLDefs(char *b, unsigned l, int mode, tAddFunc AddFunc);
extern int SearchTxiSecs(char *b, unsigned l, int mode, tAddFunc AddFunc);
extern int SearchAsmLabels(char *b, unsigned l, int mode, tAddFunc AddFunc);
extern int SearchPHPFuncs(char *srcBuffer, unsigned len, int mode, tAddFunc addFunc);

#endif // BUFUN_H_INCLUDED
