/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
int  SyntaxSearch_Search(char *word, char *&FileName, char *&NodeName,
                         char *&VisibleName);
int  SyntaxSearch_Load(fpstream &s);
int  SyntaxSearch_Save(fpstream &s);
int  SyntaxSearch_InitWithDefaults(void);
void SyntaxSearch_EditFilesList(void);
void SyntaxSearch_EditSettings(void);
void SyntaxSearch_ShutDown(void);
int  SyntaxSearch_GetJumpOption(void);
