/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_BestWrite
#define Uses_itoa
#include <ceditint.h>
#include <signal.h>
#define Uses_unistd
#define Uses_string
#define Uses_TScreen
#define Uses_TEventQueue
#include <ced_inte.h>
#include <tv.h>
#include <stackdbg.h>

extern void StopStdErrRedirection();
extern void DumpEditors(void);
static const char *ErrFile=0;
static char Strategy;

#ifdef SEOS_UNIX
static
void WriteOut(const char *s)
{
 write(STDOUT_FILENO,s,strlen(s));
}
#endif

static
void EditorSignal(int num)
{
 static char msg[] = "Panic! got a signal, quiting\r\n";
 static char msg1[] ="You asked for it, so quiting\r\n";
 static char msg2[] ="That's from SET's editor " TCEDITOR_VERSION_STR "\r\n";
 static char msg3[] ="Signal number: ";

 // No signals now
 signal(SIGABRT, SIG_IGN);
 signal(SIGFPE,  SIG_IGN);
 signal(SIGILL,  SIG_IGN);
 signal(SIGSEGV, SIG_IGN);
 signal(SIGTERM, SIG_IGN);
 signal(SIGINT,  SIG_IGN);
 /* It doesn't have much sense, SIGKILL never reachs the application ;-)
 #ifdef SIGKILL
 signal(SIGKILL, SIG_IGN);
 #endif */
 #ifdef SIGQUIT
 signal(SIGQUIT, SIG_IGN);
 #endif

 BestWrite(num==SIGINT ? msg1 : msg,sizeof(msg)-1);
 BestWrite(msg2,sizeof(msg2)-1);
 // Print which signal we received
 BestWrite(msg3,sizeof(msg3)-1);
 char cnum[33];
 itoa(num,cnum,10);
 BestWrite(cnum,strlen(cnum));
 BestWrite("\r\n",2);
 DumpEditors();

 #ifdef SEOS_UNIX
 // A dangerous thing but if we don't do it the keyboard will be messed
 TEventQueue::suspend();
 if (Strategy!=DBGST_DEBUG)
   {
    WriteOut("\rI'm sorry but I did something wrong :-(\r\n");
    if (ErrFile)
      {
       WriteOut("Look in the file ");
       WriteOut(ErrFile);
       WriteOut(" for unsaved buffers and information.\r\n");
       WriteOut("Please send this file to the author (set@ieee.org).\r\n");
      }
   }
 #endif
 if (Strategy==DBGST_DEBUG && !DebugStackSeparateTerminalWillBeUsed())
   {// That's really dangerous, but if we don't do it the debugger will
    // be useless
    TScreen::suspend();
    StopStdErrRedirection();
   }
 DebugStack(NULL);
 // Generate the signal
 signal(num, SIG_DFL);
 raise(num);
}

#ifdef SECompf_djgpp
#include <sys/exceptn.h>
#include <go32.h>
void CtrlCOff(void)
{
 // Interrupt key: RightShift+Ctrl+Del
 __djgpp_set_sigint_key(0x553);
 __djgpp_set_sigquit_key(0x553);
 _go32_want_ctrl_break(1);
}
#endif

#ifdef SEOS_UNIX
void CtrlCOff(void)
{
 // Just ignore ^C and ^\ so people doesn't break the editor unintentionally
 signal(SIGINT,SIG_IGN);
 signal(SIGQUIT,SIG_IGN);
}
#endif

#ifdef SEOS_Win32
void CtrlCOff(void)
{
 // Ctrl-C and Ctrl-Break keys are disabled in TScreen so we
 // do nothing here.
}
#endif

void InitEditorSignals(char aStrategy, const char *prgName, const char *errFile)
{
 Strategy=DebugStackInstall(aStrategy,prgName);
 if (Strategy==DBGST_DO_NOTHING)
   {
    CtrlCOff();
    return;
   }
 ErrFile=errFile;

 signal(SIGABRT, EditorSignal);
 signal(SIGFPE,  EditorSignal);
 signal(SIGILL,  EditorSignal);
 signal(SIGSEGV, EditorSignal);
 signal(SIGTERM, EditorSignal);
 signal(SIGINT,  EditorSignal);
 /* It doesn't have much sense, SIGKILL never reachs the application ;-)
 #ifdef SIGKILL
 signal(SIGKILL, EditorSignal);
 #endif */
 #ifdef SIGQUIT
 signal(SIGQUIT, EditorSignal);
 #endif
 CtrlCOff();
}

