/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
void ProgBar_Init(const char *title, int max, const char *comment1=0, const char *comment2=0);
void ProgBar_DeInit(void);
void ProgBar_UpDate(int pos);
void ProgBar_SetComments(const char *comment1, const char *comment2=0);
// Default value, to be remapped if needed and then copied to current
extern char ProgBar_DefaultChar;
// Currently used
extern char ProgBar_CurrentChar;

