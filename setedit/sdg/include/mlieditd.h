/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TMLIEditorDefs) && !defined(__TMLIEditorDefs__)
#define __TMLIEditorDefs__
class TNoCaseStringCollection;

extern int   InitLispEditor(void);
extern void  DeInitLispEditor(void);
extern char *InterpretLispEditor(char *s, Boolean print=False);
extern int   ChooseAndRunLispEditor(void);
extern int   InterpretLispEditorFile(char *s);
extern int   ReRunLastChooseLispEditor(void);
extern int   MLIEdSeachAndRunCom(char *name, Boolean verbose);
extern TNoCaseStringCollection *GetMacrosList(void);
extern int   MLIEdIsolateCode(char *start, char *&end);

// Definitions for move
#define SLP_MOVE_CURSOR      -1
#define SLP_DONT_MOVE_CURSOR 0
#define SLP_DEFAULT_MOVE     1

// Definitions for choose & run
#define SLP_ERROR     0
#define SLP_NO_MACROS 1
#define SLP_OK        2
#define SLP_NO_CHOOSE 3
#define SLP_NO_INIT   4

extern int MLIEditorError;
extern char *MLIEditorTypeError;
extern char *MLIEditorErrorName;
extern char *MLIEditorErrorCode;
#endif
