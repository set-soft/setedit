/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TMLIEditor) && !defined(__TMLIEditor__)
#define __TMLIEditor__
const int MLIEditorCommands=24;
class TNoCaseStringCollection;
class TCEditor;

typedef struct
{
 char Name[32];
 char *start;
} MLIMacro;

class TMacrosColl : public TNoCaseStringCollection
{
public:
 TMacrosColl() : TNoCaseStringCollection(10,10) {};
 virtual void freeItem(void *item);
};

class TMLIEditor : public TMLIBase
{
public:
 TMLIEditor(TMLIArrayBase *a, TLispVariableCol *v, FILE *f);
 ~TMLIEditor();
 virtual Command   WhatCommand(char *s);
 virtual TLispVar *WhatConstant(char *s);

 TMacrosColl *Macros;

 /* These functions are here just for scope reasons, like a namespace */
 static int SendCommand(int command);
 static int InsertText(char *str,int len,int select,int move);
 static char *GetEditorName(void);
 static char *GetSelection(int &len);
 static char *GetWordUnderCursor(int lenMax, int &len, unsigned options);
 static void ShowInStatusLine(char *s, int l);
 static char *CompletionChoose(char *options, char *delimiter, unsigned flags);
 static int  GetSyntaxAtCursor();
 static int  ForceUpdate();
 static char *AskString(const char *title, const char *message);
 static int  OpenFile(const char *fileName);
 static Boolean SelectionExists();
 static Boolean FindString(char *str, unsigned flags);
 static int  GetCursorX();
 static int  GetCursorY();
 static void SetCursorXY(int x, int y);
 static const char *GetSyntaxLang();

 static TCEditor *Editor;

private:
 static char *cNames[MLIEditorCommands];
 static Command cComms[MLIEditorCommands];
};

#endif
