/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <tv.h>
/**[txh]********************************************************************

  Comments:
  This module is used to interface with RHIDE.@p
  Here is the interface with the message window of RHIDE. The module is
100% untested so Robert must check it, I simply can't.@p

***************************************************************************/

extern void show_message(const char *msg, const char *file=NULL,
                         int line=0, int column=0, int msg_type=0);
/*
This function can then be declared for the standalone editor
by you and for RHIDE by me and then the editor class can
call this function if it wants to show something.

The prototype of the function is derived from my currently
used message record, which has the following structure

typedef enum {
  msgError,
  msgWarning,
  msgMessage
} msgType;

struct MsgRec
{
  char * filename;
  msgType type;
  int lineno;
  int column;
  char * msg;
  MsgRec(const char *file_name,const char *msg,
         msgType atype = msgMessage,
         int alineno = 0, int acolumn = 1);
  ~MsgRec();
};
*/

void EdShowMessage(char *msg,Boolean /*remove_old*/)
{
 show_message((const char *)msg,NULL,0,0,2);
}

void EdShowMessageFile(char *msg, FileInfo &fInfo, char *fileName,
                       unsigned /*Options*/)
{
 show_message((const char *)msg,fileName,fInfo.Line,fInfo.Column,0);
}

void EdJumpToMessage(ccIndex )
{
}

// It should reload a file from disk
int EdReloadIfOpened(char *name)
{
}

extern int RunProgram(const char *cmd,
                      Boolean redir_stderr,Boolean redir_stdout,
                      Boolean SwitchToUser);

/**[txh]********************************************************************

  Description:
  This function is needed for sLisp, it must call an external program using
system. In the editor the stderr is redirected and I parse the errors.

***************************************************************************/

void RunExternalProgram(char *Program, unsigned )
{
 if (!Program)
    return;
 RunProgram((const char *)Program,True,True,False);
}

/**[txh]********************************************************************

  Description:
  Dummy for the sLisp redirection to a variable. Robert should provide a
replacement.

***************************************************************************/

char *RunExternalProgramGetFile(int &len)
{
 len=0;
 char *s=new char[1];
 *s=0;
 return s;
}


/**[txh]********************************************************************

  Description:
  Dummy for the codepage translation.

***************************************************************************/

void RemapNStringCodePage(unsigned char *, unsigned char *, unsigned short *,
                          int )
{
}

/**[txh]********************************************************************

  Description:
  Dummy for the Special Lines Manager (spliman.cc)

***************************************************************************/

void SpLinesUpdate()
{
}
