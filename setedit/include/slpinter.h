/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
extern void SLPInterfaceRun(TCEditor *ed);
extern void SLPInterfaceInit(char *file);
extern void SLPInterfaceReRun(TCEditor *ed);
extern int  SLPSearchMacro(TCEditor *ed, char *name, Boolean verbose);
extern void SLPInterfaceRunSelection(TCEditor *ed);
extern void SLPInterfaceRunAsk(TCEditor *ed, char *code=0);
extern int  SLPInterfaceRunString(TCEditor *ed, char *code, Boolean verbose);

const int maxRunAskCode=1024;

