/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// The following function must be provided by the main part of the editor.
// Your objetive is transfer the array spLines to the editor associated to fileName
extern void ApplySpLines(char *fileName, int *spLines, int *ids);


void SpLinesAdd(char *fileName, int line, int idSource, Boolean TransferNow=True);
void SpLinesUpdate(void);
int *SpLinesGetFor(char *fileName, int *&ids);
void SpLinesDeleteForId(int id, const char *file=NULL, Boolean aLine=False, int oLine=0);
int SpLineGetNewValueOf(int line, char *fileName, Boolean *found=NULL);
int SpLineGetOldValueOf(int line, char *fileName, Boolean *found=NULL);
void SpLinesCleanUp();

const int idsplError=1,
          idsplBreak=2,
          idsplRunLine=3;
