/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifndef PATHTOOL_H_INCLUDED
#define PATHTOOL_H_INCLUDED
class TStringCollectionW;

struct stat;
int   edTestForFile(const char *name);
int   edTestForFile(const char *name, struct stat &st);
char *ExpandFileNameToThePointWhereTheProgramWasLoaded(const char *s);
int   FindFile(const char * name,char * & fullName, const char *reference);
char *ReplaceExtension(char *name, const char *ext, const char *old);
char *ReplaceExtension(char *name, const char *ext);
char *AddToNameOfFile(char *fname, char *add);
int   DeleteWildcard(char *mask);
char *GetPathRelativeToRunPoint(char *dest, const char *binReplace, char *file);
void  SetReferencePath(char *orig);
char *RedirectStdErrToATemp(int &StdErrOri,int &StdErrNew);
int  FileCopy(const char *orig, const char *dest);
void CheckForValidTMPDIR();
int  IsADirectory(const char *name);
int  IsASoftLink(const char *name);
int  MakeFileHidden(char *file);
int  RemoveFileHidden(char *file);
void AddToFilesToKill(char *name);
void KillFilesToKill();
void ReleaseFilesToKill();
TStringCollectionW *GetFilesToKill();
void SetFilesToKill(TStringCollectionW *files);
int CompareFileNames(char *origFile, char *destFile);
void CheckIfCurDirValid(void);
char *GetShortNameOf(char *longName, char *shortName);
int CheckIfPathAbsolute(const char *s);
char *MakeItHiddenName(char *);

const int maxSFNSize=68;

inline
char *ExpandHome(const char *s)
{
 return ExpandFileNameToThePointWhereTheProgramWasLoaded(s);
}
char *ExpandHomeSave(const char *s);
#endif
