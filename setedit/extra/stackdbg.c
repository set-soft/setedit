/*************************************************************************
 *
 * Copyright (c) 1999 by Bjorn Reese <breese@mail1.stofanet.dk>
 * Copyright (c) 2001 by Salvador E. Tropea (SET) <set@ieee.org>
 *
 * itoa: Copyright (C) 1995 DJ Delorie.
 *
 * License: GPL
 * Original license:
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
 * CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.

  Some technicals details: (by SET)
  I modified the code to work from a signal handler with Linux+glibc
2.1.3+gdb 19990928. I hope it works for other combinations. The main
problems in the original code were:
1) Used two different programs to achive it (one to attach and another
to trace).
2) The point of the real crash was impossible to determine with the
stacktrace version. It looks like gdb needs the debugie stopped to
determine the point where the signal was triggered.

*************************************************************************/
#include "stackdbg.h"
#include <configed.h>

/*****************************************************************************
 ============================================================================
  UNIX target, Linux right now to be honest.
 ============================================================================
*****************************************************************************/
#ifdef SEOS_UNIX
#define CLY_DoNotDefineUTypes
#define Uses_itoa
#define Uses_alloca
#include <compatlayer.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h> // SET: getenv
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>

/*************************************************************************
 * Defines
 */
#define MAX_INT_PR_SIZE 32
#define DEBUGER   "gdb"
#define XTERMINAL "xterm"
#define NM        "nm"
#define SYS_ERROR -1

/*************************************************************************
 * Globals
 */
/* The name of the executable is needed by the debugger */
static const char *global_processname=NULL;
static char gdbPresent=0, xtermPresent=0, nmPresent=0, weHaveSymbols=0;
/* By default do nothing */
static char strategy=DBGST_DO_NOTHING;
/* This variable will be modified by gdb */
/* SET: This variable must be global if we want to use gcc -O2 */
/* A compiler won't respect volatile if the variable is local! */
static volatile int DebugStack_attached=0;

void StackDBG_JustDumpStack(const char *redirect);

/*****************************************************************************
  Helper functions for process handling.
*****************************************************************************/

/**[txh]********************************************************************

  Description:
  This works like system() except the parent process is forced to wait
until the debugger which is launched in the child process has been
attached to the parent process. This functions should only be used by
DebugStack().
  By breese.

  Returns: 0 on success, 1 if fork fails

***************************************************************************/

int StackDBG_SpecialSystem(const char *command)
{
 int rc=0;
 pid_t pid;
 
 pid=fork();
 switch (pid)
   {
    case -1: /* fork() failed */
         rc = 1;
         break;

    case 0: /* Child */
         /*
         * The system() call assumes that /bin/sh is
         * always available, and so will we.
         */
        execl("/bin/sh","/bin/sh","-c",command,NULL);
        _exit(1);
        break;

    default: /* Parent */
         /* Wait until the debugger is attached */
         /* It would be nicer to sleep() here, but it doesn't
          * appear to work on all platforms */
         while (!DebugStack_attached);
         break;
   } /* switch */
 return rc;
}

/**[txh]********************************************************************

  Description:
  Runs the command as a child process redirecting the output (out and error
to a us.
  Note: I changed what handle closes the child.
  By breese.
  
  Return:
  The file handle of a pipe connected to the child.
  
***************************************************************************/
static
int my_popen(const char *command, pid_t *pid)
{
 int rc;
 int pipefd[2];
 
 rc=pipe(pipefd);
 if (rc!=SYS_ERROR)
   {
    *pid=fork();
    switch (*pid)
      {
       case SYS_ERROR:
            rc = SYS_ERROR;
            close(pipefd[0]);
            close(pipefd[1]);
            break;
         
       case 0: /* Child */
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[1]);
            /*
             * The System() call assumes that /bin/sh is
             * always available, and so will we.
             */
            execl("/bin/sh", "/bin/sh", "-c", command, NULL);
            _exit(EXIT_FAILURE);
            break;
         
       default: /* Parent */
            close(pipefd[1]);
            rc = pipefd[0];
            break;
      } /* switch */
   }
 return rc;
}

/**[txh]********************************************************************

  Description:
  Gets a line of text from the child.
  By breese.
  
  Return: 
  The ammount of bytes read.
  
***************************************************************************/
static
int my_getline(int fd, char *buffer, int max)
{
 char c;
 int i=0;
 
 do
   {
    if (read(fd,&c,1)<1)
       return 0;
    if (i<max)
       buffer[i++]=c;
   }
 while (c!='\n');
 buffer[i]=(char)0;
 return i;
}

/**[txh]********************************************************************

  Description:
  Closes the pipe and ensures the child dies.
  By SET.
  
***************************************************************************/
static
void my_pclose(int fd, int pid)
{
 int wstatus,ret;
 /* Close the pipe, it should kill the child with SIGPIPE */
 close(fd);
 /* Is already dead? */
 ret=waitpid(pid,&wstatus,WNOHANG);
 if (ret==0)
   {/* Nope, wait a little bit */
    /* I choose to sleep and no hang, perhaps setting an alarm
       could be better */
    usleep(100000);
    /* Is dead? */
    ret=waitpid(pid,&wstatus,WNOHANG);
    if (ret==0)
      {/* Nope, a diehard, or just too slow. */
       /* Make sure the the child process has terminated */
       kill(pid,SIGKILL);
       /* The child can't block it so we can wait */
       waitpid(pid,&wstatus,0);
      }
   }
}

/*****************************************************************************
  Helper functions for output.
*****************************************************************************/
static
void Write(int fd, const char *s)
{
 write(fd,s,strlen(s));
}

static
void WriteNL(int fd)
{
 Write(fd,"\n");
}

#ifdef TVCPU_x86
static
void WriteSpace(int fd)
{
 Write(fd," ");
}

static
void WriteHex(int fd, unsigned long value)
{
 char bNumAux[MAX_INT_PR_SIZE+1];
 itoa(value,bNumAux,16);
 Write(fd,bNumAux);
}
#endif

/*****************************************************************************
  Core routines.
*****************************************************************************/

#if defined(TVCPU_x86) && defined(TVComp_GCC)
/**[txh]********************************************************************

  Description:
  This works for gcc+Intel, I don't know if any other platform uses the
stack like this. That's just based in the C calling convention. The address
of the first variable incremented gives the frame pointer and then the
return address follows. We can walk until the frame pointer is 0.
  It doesn't work if you compile with -fomit-frame-pointer.
  The use of __builtin_return_address(nn) isn't pratical because you don't
know how bigger can nn be and you'll most probably SIGSEGV with some
number. The code generated by __builtin_return_address(nn) becomes big
quite easily also.
  Note: I also added Intel specific code that works even with heave
optimizations enabled.
  
***************************************************************************/

void StackDBG_GCCDumpStack(int fd)
{
 #if 1
 /* The following works even for -O3 */
 /* No local variables */
 static unsigned long *a;
 static int i;
 /* a=stack frame */
 asm("movl %%ebp,%k0" : "=r" (a));
 #else
 /* Works upto -O2 for gcc 2.95.2 */
 /* One local variable */
 unsigned long *a=(unsigned long *)&a;
 static int i;
 /* skip it and get the stack frame */
 a++;
 #endif

 for (i=0; a; i++)
    {
     WriteHex(fd,a[1]);
     WriteSpace(fd);
     WriteHex(fd,(unsigned long)a);
     WriteSpace(fd);
     WriteHex(fd,a[0]);
     switch (i)
       {
        case 0:
             Write(fd," StackDBG_JustDumpStack ");
             /* The following is a trick to prevent gcc from making these
                functions inline. */
             WriteHex(fd,(unsigned long)StackDBG_JustDumpStack);
             Write(fd," ");
             WriteHex(fd,(unsigned long)StackDBG_GCCDumpStack);
             Write(fd," ");
             WriteHex(fd,(unsigned long)StackDBG_SpecialSystem);
             Write(fd," ");
             WriteNL(fd);
             break;
        case 1:
             Write(fd," DebugStack\n");
             break;
        case 2:
             Write(fd," Signal handler (?)\n");
             break;
        case 3:
             Write(fd," libc signal routines\n");
             break;
        default:
             WriteNL(fd);
       }
     a=(unsigned long *)a[0];
    }
}
#else
void StackDBG_GCCDumpStack(int fd)
{
}
#endif

/**[txh]********************************************************************

  Description:
  Wrapper for the function that dumps the stack.
  
***************************************************************************/

void StackDBG_JustDumpStack(const char *redirect)
{
 int closeIt=0,fd;

 if (redirect)
   {
    fd=open(redirect,O_WRONLY | O_CREAT | O_APPEND,0700);
    if (fd==-1)
       fd=STDERR_FILENO;
    else
       closeIt=1;
   }
 else
    fd=STDERR_FILENO;

 if (global_processname)
   {
    Write(fd,"\n\nCalling stack from ");
    Write(fd,global_processname);
   }
 WriteNL(fd);
 WriteNL(fd);
 StackDBG_GCCDumpStack(fd);
 WriteNL(fd);
 WriteNL(fd);

 if (closeIt)
    close(fd);
}


/**[txh]********************************************************************

  Description:
  Helper function to know if we can launch gdb in another xterm.

  Return:
  !=0 if a separated xterm can be used.

***************************************************************************/
static
int XtermAvailable()
{
 char *terminal=getenv("TERM");
 return terminal && strncasecmp(terminal,"xterm",5)==0;
}

/**[txh]********************************************************************

  Description:
  Helper function to know if we will launch gdb in another xterm. Exported
to help the program behave according to it.

  Return:
  !=0 if a separated xterm will be used for gdb.

***************************************************************************/

int DebugStackSeparateTerminalWillBeUsed()
{
 return strategy==DBGST_DEBUG && XtermAvailable();
}

/**[txh]********************************************************************

  Description:
  Acts according to the selected strategy.
  If the strategy is DBGST_INFORMATIVE creates the commands for gdb and
calls it using the special system routine. Then the information from gdb
is callected. If a file is indicated with redirect parameter messages
from gdb are concatenated and the stderr is suppresed. If NULL is specified
the output of gdb is redirected to stderr.
  If the strategy is DBGST_DEBUG creates the commands for gdb and calls it
using the special system routine. Then the control is passed to gdb so you
can use it for "post mortem" debuging. If the terminal is xterm* another
xterm will be used so the contents of the screen are retained.
  Note by breese: Do only use async-safe functions because DebugStack is
called from a signal handler.
  Note by SET: I rewrote it completly, of course using breese ideas.

***************************************************************************/

void DebugStack(const char *redirect)
{
 char cpid[MAX_INT_PR_SIZE+1];
 char filename[13+MAX_INT_PR_SIZE+1];
 char *buffer;
 int fd,l1,l2,h_outbak=-1;
 /* These commands assume gdb */
 char *normalCommands=
   "set height 0\n"
   "set DebugStack_attached = 1\n" /* Tell the process it can go on */
   "echo <-------- Ignore\\n\n"
   "finish\n" /* while (!attached); */
   "finish\n" /* StackDBG_SpecialSystem() */
   "finish\n" /* DebugStack() */
   "echo <-------- Crash\\n\n"
   "finish\n"
   "echo <-------- Backtrace\\n\n"
   "backtrace full\n"
   "echo \\nEnter `quit' and answer yes to exit\\n\n";
 char *quitCommands="quit\ny\n"; /* quit, yes (detach) */

 if (strategy==DBGST_DO_NOTHING)
    return;
 if (strategy==DBGST_DUMP_STACK)
   {
    StackDBG_JustDumpStack(redirect);
    return;
   }

 /* Convert the pid to a string */
 itoa((int)getpid(),cpid,10);

 /* Write the initial debugging commands to a temporary file */
 strcpy(filename,"/tmp/_attach_");
 strcat(filename,cpid);
 fd=open(filename,O_WRONLY | O_CREAT,0700);
 if (fd==-1)
    return; // :-(

 /* Compute the size of the buffer, it could be huge if global_processname
    and/or redirect are deeply nested */
 l1=64+ /* Terminal, debuger, separators, etc. I count 36 aprox. */
    strlen(filename)+strlen(global_processname)+strlen(cpid)+
    (redirect ? strlen(redirect) : 0);
 l2=strlen(normalCommands)+strlen(quitCommands)+1;
 buffer=(char *)alloca(l1>l2 ? l1 : l2);

 strcpy(buffer,normalCommands);
 /* This is if we want to end */
 if (strategy==DBGST_INFORMATIVE)
    strcat(buffer,quitCommands);
 write(fd,buffer,l2-1);
 close(fd);

 /* Arrange the command line */
 buffer[0]=0;
 /* Use xterm only if we are in another Xterminal */
 if (strategy==DBGST_DEBUG && XtermAvailable())
    strcat(buffer,XTERMINAL " -e ");
 strcat(buffer,DEBUGER " -x ");
 strcat(buffer,filename);
 strcat(buffer," ");
 strcat(buffer,global_processname);
 strcat(buffer," ");
 strcat(buffer,cpid);
 if (strategy==DBGST_INFORMATIVE)
   {
    if (redirect)
      { /* Concatenate the errors */
       strcat(buffer," >> ");
       strcat(buffer,redirect);
       /* Avoid confusing messages from gdb */
       strcat(buffer," 2> /dev/null");
      }
    else
      { /* Redirect stdout to stderr */
       h_outbak=dup(STDOUT_FILENO);
       dup2(STDERR_FILENO,STDOUT_FILENO);
      }
   }

 /* Launch the debugger */
 StackDBG_SpecialSystem(buffer);

 /* Remove the temporary file */
 unlink(filename);

 if (h_outbak!=-1)
   { /* Restore stdout */
    dup2(h_outbak,STDOUT_FILENO);
    close(h_outbak);
   }
}

/*****************************************************************************
  Initialization routines.
*****************************************************************************/

/**[txh]********************************************************************

  Description:
  Checks if the global_processname have symbols.
  
  Return: !=0 if we have symbols
  
***************************************************************************/
static
char TestForSymbols()
{
 char *buffer;
 int fd;
 pid_t pid;

 int l=8+strlen(global_processname);
 if (l<256) l=256;
 buffer=(char *)alloca(l);

 strcpy(buffer,NM);
 strcat(buffer," ");
 strcat(buffer,global_processname);

 fd=my_popen(buffer,&pid);
 if (fd!=SYS_ERROR)
    {
     my_getline(fd,buffer,l);
     my_pclose(fd,pid);
     return strstr(buffer,"no symbols")==0;
    }
 return 0;
}

/**[txh]********************************************************************

  Description:
  Routine used to initialize the system. The aStrategy parameter indicates
which strategy is desired.
  The process name should be the content of argv[0] or a binary copy with
debug information.

  Returns:
  The available strategy. If some conditions for the desired strategy
aren't met the routine choose the closest.

***************************************************************************/

char DebugStackInstall(char aStrategy, const char *processname)
{
 struct stat st;
 char *path,*pathEnv,*test,*s;
 int l,lookForProc;

 global_processname=processname;
 /* These doesn't need any special thing */
 if (aStrategy==DBGST_DO_NOTHING || aStrategy==DBGST_DUMP_STACK)
   {
    strategy=aStrategy;
    return strategy;
   }
 /* If we don't know the process name that's the best we can do */
 if (!processname)
   {
    strategy=DBGST_DUMP_STACK;
    return strategy;
   }
 /* For the rest we must be sure the programs are there or we will hang */
 gdbPresent=0; xtermPresent=0; nmPresent=0; weHaveSymbols=0;
 /* Check is the processname is full qualified */
 lookForProc=strchr(processname,'/')==0;
 /* assume we will fail */
 strategy=DBGST_DUMP_STACK;
 pathEnv=getenv("PATH");
 if (pathEnv)
   {/* Work in a copy to avoid damaging the path */
    l=strlen(pathEnv)+1;
    path=(char *)alloca(l);
    test=(char *)alloca(l+8);
    strcpy(path,pathEnv);
    s=strtok(path,":");
    while (s && (!gdbPresent || !xtermPresent || !nmPresent || lookForProc))
      {
       if (!gdbPresent)
         {
          strcpy(test,s); strcat(test,"/"); strcat(test,DEBUGER);
          if (stat(test,&st)==0) gdbPresent=1;
         }
       if (!xtermPresent)
         {
          strcpy(test,s); strcat(test,"/"); strcat(test,XTERMINAL);
          if (stat(test,&st)==0) xtermPresent=1;
         }
       if (!nmPresent)
         {
          strcpy(test,s); strcat(test,"/"); strcat(test,NM);
          if (stat(test,&st)==0) nmPresent=1;
         }
       if (lookForProc)
         {
          strcpy(test,s); strcat(test,"/"); strcat(test,processname);
          if (stat(test,&st)==0)
            {
             char *s=(char *)malloc(strlen(test)+1);
             strcpy(s,test);
             global_processname=s;
             lookForProc=0;
            }
         }
       s=strtok(NULL,":");
      }
   }
 /* We need nm to know if the file have symbols.
    If the file doesn't symbols and we call gdb we won't be
    able to set the DebugStack_attached variable.
    We can't afford it. */
 if (nmPresent && gdbPresent && !lookForProc)
   {
    weHaveSymbols=TestForSymbols();
    if (weHaveSymbols)
       strategy=aStrategy;
   }
 return strategy;
}
#endif // SEOS_UNIX

/*****************************************************************************
 ============================================================================
  DOS target, djgpp right now to be honest.
 ============================================================================
*****************************************************************************/
#ifdef SEOS_DOS
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

/* In djgpp we can't attach the debugger son only DBGST_DUMP_STACK
   is available. As that's provided by libc we don't need much code */

char DebugStackInstall(char aStrategy, const char *processname)
{
 processname=0;
 return DBGST_DUMP_STACK;
}

void DebugStack(const char *redirect)
{
 int h_err,h_errbak;
 if (redirect)
   {
    h_err=open(redirect,O_WRONLY | O_BINARY | O_CREAT | O_APPEND,
                        S_IREAD | S_IWRITE);
    if (h_err)
      {
       h_errbak=dup(STDERR_FILENO);
       dup2(h_err,STDERR_FILENO);
      }
   }
}

int DebugStackSeparateTerminalWillBeUsed()
{
 return 0;
}
#endif // SEOS_DOS

/*****************************************************************************
 ============================================================================
  Win32 target, just dummies right now to be honest.
 ============================================================================
*****************************************************************************/
#ifdef SEOS_Win32
/* Right now I know nothing here, but I'm sure we can start Dr Watson,
   Turbo Debugger, gdb win32, etc. */
static const char *global_processname=0;

char DebugStackInstall(char aStrategy, const char *processname)
{// Just use the parameters
 global_processname=processname;
 if (aStrategy!=DBGST_DO_NOTHING)
    aStrategy=DBGST_DO_NOTHING;
 return aStrategy;
}

void DebugStack(const char *redirect)
{
 if (redirect) redirect=0;
}

int DebugStackSeparateTerminalWillBeUsed()
{
 return 0;
}
#endif // SEOS_Win32




#if defined(STANDALONE)
/*****************************************************************************
Standalone test:

 It just hooks SIGSEGV and does some functions calls to finally generate a
SIGSEGV.
*****************************************************************************/

void SignalHandler(int signum)
{
 signal(signum,SIG_IGN);
 DebugStack("pp");
 signal(signum,SIG_DFL);
 raise(signum);
}

/* Create some functions to get something in the stack */
void HacerDos()
{
 char *s=0;
 int i=45; /* A local var to see */
 s[1]=0;
}

void HacerUna()
{
 HacerDos();
}

int main(int argc, char *argv[])
{
 DebugStackInstall(DBGST_DUMP_STACK,argv[0]);
 signal(SIGSEGV,SignalHandler);

 HacerUna();
 return 0;
}
#endif
