/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
void PrintSetup(void);
int  PrintSource(char *b, char *FileName, unsigned tabSize);
void PrintSetDefaults(void);
void SavePrintSetUp(opstream* s);
void LoadPrintSetUp(ipstream* s);

