/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifndef FIOPEAID_H__
#define FIOPEAID_H__
class fpstream;

typedef struct
{
 int id;
 char *dir;
 char *mask;
 int helpCtx;
 int ax,ay,bx,by;
} AsoID;

AsoID *GetFileIDDirBuffer(int id);
void  SetFileIDDirValue(int id, char *dir, char *mask=0);
int   GenericFileDialog(const char *title, char *file, char *mask, int id, unsigned flags=0);
void  LoadFileIDDirs(fpstream &s, Boolean isLocal);
void  SaveFileIDDirs(fpstream &s);

#endif
