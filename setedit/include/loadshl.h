/* Copyright (C) 2001-2004 by Salvador E. Tropea (SET),
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
extern char *SHLNameOf(unsigned number);
extern int   SHLNumberOf(char *name);
class TCEditor;
extern char *SHLConstructEmacsModeComment(TCEditor &e, int &sizeSt, int &sizeEnd);
extern int   SHLSelect(TCEditor &e, char *buffer, int lenBuf);
extern void  SHLTransferDefaultsNewFile(TCEditor &e);
extern int   TakeCommentEmacs(char *buffer, int lenBuf, char *ext, int *tab_width,
                              int *startCom=NULL, int *endCom=NULL);

#endif // LOADSHL_H_INCLUDED
