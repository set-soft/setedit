/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TMLIEditor) && !defined(__TMLIEditor__)
#define __TMLIEditor__
const int MLIEditorCommands=33;
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

class TKeySeqCol;
struct KeyTTable;
class TLispKeyBind : public TLispVar
{
public:
 TLispKeyBind(TKeySeqCol *aSKeys, void *aData, int aBindType);
 virtual ~TLispKeyBind();
 virtual int print(FILE *);
 virtual char *toStr();

 TKeySeqCol *sKeys;
 void *data;
 int bindType;
};

#define GetKeyBind(pos,obj) \
{ obj=(TLispKeyBind *)o->Solve(start+pos); \
  if(!obj) goto CleanUp; \
  if ((obj->type>>12)!=MLIGKeyBind) \
    { o->Error=MLITypeParam; goto CleanUp; } \
}
#define MLIRetKeyBind(skeys,data,type) \
        o->array->ReplaceItem(start-1,new TLispKeyBind(skeys,data,type))
#define LocVarKeyBind(a) TLispKeyBind *a=NULL

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
 static Boolean FindOrReplaceString(char *str, char *repl, unsigned flags,
                                    char *&string, unsigned &len,
                                    Boolean again=False);
 static Boolean FindString(char *str, unsigned flags, char *&string, unsigned &len)
        { return FindOrReplaceString(str,NULL,flags,string,len); }
 static Boolean FindAgain(char *&string, unsigned &len);
 static unsigned GetFindFlags();
 static int  GetCursorX();
 static int  GetCursorY();
 static void SetCursorXY(int x, int y);
 static const char *GetSyntaxLang();
 static int  SelectWindowNumber(int num);
 static int  GetCurWindowNumber();
 static int  GetMaxWindowNumber();
 // Key binding operations
 static int  StartKeyBind();
 static void EndKeyBind();
 static void AbortKeyBind();
 static int  BindKey(TKeySeqCol *sKeys, void *data, int Type);

 static TCEditor *Editor;

private:
 static char *cNames[MLIEditorCommands];
 static Command cComms[MLIEditorCommands];
 // Variables used for the Find & Replace interface
 static char *findAgainStr;
 static char *replaceAgainStr;
 static unsigned findAgainFlags;
 // Variables used for key binding operations
 static KeyTTable *oriKeyTable;
 static int oriCanBeDeleted;
};

#endif

const int edfInfTVDriver=0, edfInfOS=1, edfInfOSFlavor=2, edfInfCPU=3,
          edfInfCompiler=4, edfInfCompilerFlavor=5;

