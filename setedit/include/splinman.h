/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// The following function must be provided by the main part of the editor.
// Your objetive is transfer the array spLines to the editor associated to fileName
extern void ApplySpLines(char *fileName,int *spLines);

void SpLinesAdd(char *fileName, int line, int idSource, Boolean TransferNow=True);
void SpLinesUpdate(void);
int *SpLinesGetFor(char *fileName);
void SpLinesDeleteForId(int id);
int SpLineGetNewValueOf(int line, char *fileName);
void SpLinesCleanUp();

const int idsplError=1,
          idsplBreak=2;
