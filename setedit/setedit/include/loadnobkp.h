/* Copyright (C) 2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifndef LOADNOBKP_H_INCLUDED
#define LOADNOBKP_H_INCLUDED

const int NBKPMaxLineLen=1024;

void NBKPSetFileName(const char *name, unsigned ctxHelp);
void NBKPSetSaveFileName(const char *save);
int  NBKPMakeIt(const char *name);
void UnloadNBKP();
void NBKPEdit(void);

#endif // LOADNOBKP_H_INCLUDED
