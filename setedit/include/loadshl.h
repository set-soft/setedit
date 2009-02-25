/* Copyright (C) 2001-2005 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifndef LOADSHL_H_INCLUDED
#define LOADSHL_H_INCLUDED

#include <ced_pcre.h>

typedef struct
{
 int PCREMaxMatchs;
 int *PCREMatchs;
 int PCREHits;
} PCREData;

void  PCREInitCompiler(PCREData &p);
void  PCREStopCompiler(PCREData &p);
pcre *PCRECompileRegEx(char *text, PCREData &p);
int   PCREDoSearch(char *search, int len, pcre *CompiledPCRE, PCREData &p);
#define PCREDataDestroy(p) PCREInitCompiler(p)
void  PCREGetMatch(int match, int &offset, int &len, PCREData &p);

#define MaxExtension 80

struct strSHL;
class  TStringCollection;
extern int   LoadSyntaxHighLightFile(char *name, strSHL *&hl, TStringCollection *list,
                                     int &cant);
extern int   LoadSyntaxHighLightKeywords(strSHL &hl);
extern void  UnLoadSyntaxHighLightFile(strSHL *&hl, TStringCollection *list,int &Cant);
extern const char *SHLNameOf(unsigned number);
extern int   SHLNumberOf(char *name);
class TCEditor;
extern char *SHLConstructEmacsModeComment(TCEditor &e, int &sizeSt, int &sizeEnd);
extern int   SHLSelect(TCEditor &e, char *buffer, int lenBuf);
extern void  SHLTransferDefaultsNewFile(TCEditor &e);
extern int   TakeCommentEmacs(char *buffer, int lenBuf, char *ext, int *tab_width,
                              int *startCom=NULL, int *endCom=NULL);
extern void  ShowSHLLoadErrors();

#if defined(Uses_TNLIndentCol)
class NLIndent
{
public:
 NLIndent() { cArgStr[0]=cArgStr[1]=NULL; };
 ~NLIndent() { delete[] cArgStr[0]; delete[] cArgStr[1]; };
 uchar cond[2], action;
 unsigned cArgInt[2], acArgInt;
 char *cArgStr[2];
};

class TNLIndentCol : public TNSCollection
{
public:
 TNLIndentCol() : TNSCollection(18,8) {};
 virtual void freeItem(void *item);
 NLIndent *At(ccIndex i) { return (NLIndent *)at(i); };
};

// Conditions
const uchar nliAlways=0,          // Default: "True"
            nliParBalancePos=1,   // More ( than )
            nliParBalanceNeg=2,   // More ) than (
            nliFirstWord=3,       // A word to match, ArgInt is the length
            nliNoLastChar=4;      // Last char in line must be different than ArgInt
// Actions
const uchar nliAutoIndent=0,      // Default, ArgInt is an offset to add
            nliUnindent=1,        // Unindent
            nliMoveAfterPar=2;    // Move to the col of the arg after a (
#endif // Uses_TNLIndentCol

#if defined(Uses_TSHLErros)
class SHLError
{
public:
 SHLError(const char *s, int l) { str=s; line=l; };

 const char *str;
 int line;
};

class TSHLErrors : public TNSCollection
{
public:
 TSHLErrors() : TNSCollection(2,2) {};
 virtual void freeItem(void *item);
};

void SHLAddLoadError(const char *error, int line);
#endif // Uses_TSHLErros

#endif // LOADSHL_H_INCLUDED
