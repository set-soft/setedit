/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_stdio
#define Uses_unistd
#define Uses_stdlib
#include <tv.h>
#define Uses_EditorId
#include <ced_exte.h>
#include <edmsg.h>
#include "rhutils.h"

class TCEditor;
extern int FindFile(const char *name, char *&fullName);

/**[txh]********************************************************************

  Comments:
  This module is used to interface with RHIDE.@p
  Here is the interface with the message window of RHIDE. The module is
100% untested so Robert must check it, I simply can't.@p
  Note that from time to time I add new functions here that are just dummies
to compile RHIDE but they need to be implemented propperly in RHIDE.

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

void EdShowMessage(const char *msg, Boolean /*remove_old*/)
{
 show_message(msg,NULL,0,0,2);
}

void EdShowMessageI(const char *msg, unsigned /*Options*/)
{
 char *aux=TVIntl::getTextNew(msg);
 show_message(aux,NULL,0,0,2);
 DeleteArray(aux);
}

void EdShowMessageI(const char *msg, Boolean , Boolean )
{
 char *aux=TVIntl::getTextNew(msg);
 show_message(aux,NULL,0,0,2);
 DeleteArray(aux);
}

void EdShowMessageFile(const char *msg, FileInfo &fInfo, char *fileName,
                       unsigned /*Options*/)
{
 show_message(msg,fileName,fInfo.Line,fInfo.Column,0);
}

void EdJumpToMessage(ccIndex )
{
}

// It should reload a file from disk
int EdReloadIfOpened(const char *name, stEditorId *id)
{
 return 0;
}

extern int RunProgram(const char *cmd,
                      Boolean redir_stderr,Boolean redir_stdout,
                      Boolean SwitchToUser);

/**[txh]********************************************************************

  Description:
  This function is needed for sLisp, it must call an external program using
system. In the editor the stderr is redirected and I parse the errors.
  IMPORTANT! one flag passed to it can indicate we want to redirect the
input stream of the child process:

 if ((flags & repRedirIn) && RedirInputFile && strlen(RedirInputFile)+
     strlen(s)+4<PATH_MAX)

 This should be implemented in RHIDE. I already copied the support functions
that stores/remove the data into a temporal file whose name is pointed by
RedirInputFile.

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

static char    *RedirInputFile=0;

/**[txh]********************************************************************

  Description:
  Saves the indicated data to a temporal file. It can be used to redirect
the input of a program when calling RunExternalProgram with the repRedirIn
option. Use RunExternalProgramRemoveInRedir to release the used resources.
@x{RunExternalProgramRemoveInRedir}.
  
***************************************************************************/

void RunExternalProgramSetInRedir(const char *buffer, unsigned len)
{
 free(RedirInputFile);
 RedirInputFile=unique_name("in");
 FILE *f=fopen(RedirInputFile,"wb");
 if (f)
   {
    fwrite(buffer,len,1,f);
    fclose(f);
   }
}

/**[txh]********************************************************************

  Description:
  Releases all the resourses allocated by RunExternalProgramSetInRedir.
@x{RunExternalProgramSetInRedir}.
  
***************************************************************************/

void RunExternalProgramRemoveInRedir()
{
 if (RedirInputFile)
   {
    unlink(RedirInputFile);
    free(RedirInputFile);
    RedirInputFile=0;
   }
}

/**[txh]********************************************************************

  Description:
  Dummy for the Special Lines Manager (spliman.cc)

***************************************************************************/

void SpLinesUpdate()
{
}


/**[txh]********************************************************************

  Description:
  Dummy for the EdShowMessage(const char*,bool,bool)  (edmsg.cc)

***************************************************************************/

void EdShowMessage (const char *, bool, bool)
{
}

/**[txh]********************************************************************

  Description:
  Dummy for the GiveAdvice(int)  (advice.cc)

***************************************************************************/

int GiveAdvice (int)
{
  return 0;
}

/**[txh]********************************************************************

  Description:
  Selects the window that matchs the provided number. The number can be
negative indicating a type of DskWin object, in this case the first match
is returned.
  
  Return: !=0 if the window was selected.
  
***************************************************************************/

int SelectWindowNumber(int number)
{
 return 0;
}

// The following doesn't have to be implemented by RHIDE.
TCEditor *GetCurrentIfEditor()
{
 return NULL;
}

/**[txh]********************************************************************

  Description:
  Used to know the biggest number of editor window
  
  Return: The biggest number or 0 if no editors.
  
***************************************************************************/

int GetMaxWindowNumber()
{
 return 0;
}

/**[txh]********************************************************************

  Description:
  Look for a file name in the project. If the file is there return the full
name of the file (with path).
  This is also used by the function that looks for a file under the cursor.
  
  Return: A newly allocated file name or NULL.
  
***************************************************************************/

char *GetAbsForNameInPrj(const char * /*name*/)
{
 return NULL;
}

/**[txh]********************************************************************

  Description:
  My function have an extra argument, this wrapper adapts it to RHIDE's
version.
  
***************************************************************************/

int FindFile(const char *name, char *&fullName, const char *)
{
 return FindFile(name,fullName);
}

