/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// Include for runprog.cc
const unsigned repDontShowDialog=1,repDontShowAsMessage=2,repRestoreScreen=4,
               repDontFork=8,repRedirIn=16;

char   *RunExternalProgramGetFile(int &len);
void    RunExternalProgram(char *Program=0, unsigned flags=0, char *compiler=0);
void    RunExternalProgramKillFile(void);
pid_t   RunExternalProgramGetChildPid();
Boolean RunExternalProgramRunning();
void    RunExternalProgramIncParse();
Boolean RunExternalProgramNotRunning();
void    RunExternalProgramStopChild();
void    RunExternalProgramSetInRedir(const char *buffer, unsigned len);
void    RunExternalProgramRemoveInRedir();

