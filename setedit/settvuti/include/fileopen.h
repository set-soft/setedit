/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifndef FILEOPEN_H_INCLUDED
#define FILEOPEN_H_INCLUDED
extern int GenericFileDialog(const char *title, char *file, char *mask, int histID,
                             int buttons, char *dir, unsigned flags, int ctx);
extern int FileOpenDialog(char *title, char *file, int historyID,
                          unsigned flags=0);
extern int FileSaveDialog(char *title, char *file, int historyID,
                          unsigned flags=0);
extern void SetDirForOpen(char *s);
extern void SetDirForSave(char *s);
extern char *GetLastMaskUsed();
extern void SetConfigDialogFunc(int (*func)());

const int cmFileDialogFileSelected=0x2210;
const int cmHomeDir=0x2211;
const int cmFileOpenOptions=0x2212;
const int
hcGenOpenFile =0x220F,
hcFileOpen    =0x2210,
hcFileSave    =0x2211,
hcOpenInfo    =0x2212,
hcSaveBlock   =0x2213,
hcConfigFiles =0x2214,
hcProjectFiles=0x2215,
hcOpenMP3     =0x2216,
hcSaveMP3     =0x2217,
hcGenChDir    =0x2218,
hcSelTagFile  =0x2219;

// This flag allows to select various files without living the dialog
const unsigned fdMultipleSel=0x10000,fdDialogForSave=0x80000000;
#endif
