/* Copyright (C) 2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Module: Debugger interface.
  Comments:
  It implements the interface with the debugger.

  TODO:
  The most important unimplemented features and unsolved things are:

  * When debugging remote programs gdb exposes a nasty bug: When I do next
at the end of main it replies "running", but then it sends a couple of
errors to the log (because gdb can't set a temporal breakpoint) and then
generates an error result record. But it doesn't generate an stopped info.
So we thing the program is running (gdb told it) but it isn't true. What's
worst is what happends when we send SIGINT ... finally we hang at exit waiting
for replies.

  * When a new item is added to the project try to add its path to the
  sources path.

  * Fix: We are saving the "TRect" for msg and watches, but not the "zoom
  state" and the "unzoomed" size. That's annoying.

  * Add some mechanism to enable MI v2. That's much better.

  * Advices (only project, ...).
  * Watchpoints.
  * Dialog to edit breakpoints
  * Some mechanism to verify that a location for a breakpoint is OK. GDB
says ok to everything. -symbol-list-lines could help, but that's only for
GDB 6.x.
  * Inspect variables.
  * Data window.
  * Disassembler window.
  * Put a limit to the ammount of messages in the debug message window.
  * Allow to configure gdb and XTerm program.
  * Setup the "main" function, currently we have a "run to main", but this
is C/C++ specific.
  * Not related: beep when we finish compilation.
  * Add the menubind.smn options to the redmond.smn
  * Don't show again in the confirmation for exit while debugging.
  * Detect recompilations (target time stamp), then ask to the user if we
should "move" the breakpoints to their new locations. That's tricky.
  * Time out in the gdb responses to avoid hanging. Also a stop mechanism.

IMPORTANT NOTES!!!

1) Breakpoints are very tied to splines. Splines associate an *absolute*
file name with some lines. But breakpoints usually are specified as
*relative* file names. It means that when we search for a breakpoint using
its name we must expand it. Is gdb the one in charge to keep its name
relative because we always send to gdb absolute file names.

2) Path for sources and similar details.
GDB is smart if we compile using absolute paths. But is quite idiot if we
don't. Specially the one from Debian GNU/Linux Woody (2002-04-01-cvs).
Is quite bizarre that:
 -break-insert /full-path/file.cc:line
 -> file="file.cc"
But latter says that /full-path/file.cc isn't in the symtab.
The CLI dir command or MI -environment-directory seems to solve the
problems. Currently after connecting I'm sending to gdb a list of
directories to look for sources.

***************************************************************************/

#include <ceditint.h>
#define Uses_stdio
#define Uses_unistd
#define Uses_snprintf
#define Uses_TApplication
#define Uses_TScreen
#define Uses_TVCodePage
#define Uses_TCEditor_Class
#define Uses_TCEditor
#define Uses_MsgBox
#define Uses_TStreamableClass
#define Uses_TCommandSet
#define Uses_ctype

// Messages
#define Uses_TDialog
#define Uses_TScroller
#define Uses_TScrollBar
#define Uses_TListBox
#define Uses_TDeskTop
#define Uses_TNoStaticText
#define Uses_AllocLocal

// Watches
#define Uses_TStringableListBox

// Config
// EasyDiag requests
#define Uses_TSButton
#define Uses_TSStaticText
#define Uses_TSInputLine
#define Uses_TSInputLinePiped
#define Uses_TSLabel
#define Uses_TSVeGroup
#define Uses_TSHzGroup
#define Uses_TSLabelRadio
#define Uses_TSLabelCheck

// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
// Second request the headers
#include <easydiag.h>

#define Uses_SETAppAll
#define Uses_SETAppHelper
#include <setapp.h>

#include <splinman.h>
#include <editcoma.h>
#include <edmsg.h>
#include <debug.h>
#include <dyncat.h>
#include <edhists.h>
#include <dskwin.h>
#include <pathlist.h>

const uint32 msgConsole=1, msgTarget=2, msgLog=4, msgTo=8, msgFrom=16;
static uint32 msgOps=msgConsole | msgTarget | msgLog;
const char svPresent=1, svAbsent=0, svYes=1, svNo=0;
const char bkptsVersion=1;
const char debugDataVersion=1;

#ifdef HAVE_GDB_MI

#include <mi_gdb.h>

#define DEBUG_ME 0
#if DEBUG_ME
 #define dbgPr(format, args...) printf(format , ## args)
#else
 #define dbgPr(format, args...)
#endif
#define DEBUG_BREAKPOINTS_UPDATE 0

// Variables for the configuration
const int maxWBox=70, widthFiles=256, widthShort=80;
const int widthWtExp=widthFiles, maxWtBox=maxWBox;
const int widthGDBCom=widthFiles, maxGDBComBox=maxWBox;
const int widthPID=32;
const int dmLocal=0, dmPID=1, dmRemote=2;

struct DebugOptionsStruct
{
 char program[widthFiles];
 uint32 mode;
 char args[widthFiles];
 char tty[widthShort];
 char rtype[widthShort];
 char rparam[widthShort];
};
static int debugOpsNotIndicated=1;
static DebugOptionsStruct dOps;
// End of Variables for the configuration

const int maxWDbgSt=20, maxWStatus=256;

const int modeX11=0, modeLinux=1;
static int localMode;
static MIDebugger *dbg=NULL;
// Last async response
static mi_stop *stoppedInfo=NULL;
static char pendingStoppedInfo=0;
static char killAfterStop=0;
// CPU line
// TODO: Move to app class
static char *cpuLFile=NULL;
static int   cpuLLine;
// Path for the binary
static char *binReference=NULL;
static int   binReferenceLen=0;

/*****************************************************************************
  Editor debug commands
*****************************************************************************/

void TSetEditorApp::DebugToggleBreakpoint()
{
 TCEditor *e=GetCurrentIfEditor();
 if (e)
   {
    if (!DebugCheckStopped())
       return;
    int l=e->curPos.y+1, ol;
    Boolean found;

    // We use the expanded name for the SpLines
    char s[PATH_MAX];
    strcpy(s,e->fileName);
    CLY_fexpand(s);

    ol=SpLineGetOldValueOf(l,s,&found);
    if (found)
      {
       if (DBG_RemoveBreakpoint(s,ol))
          SpLinesDeleteForId(idsplBreak,s,True,ol);
      }
    else
      {
       if (DBG_SetBreakpoint(s,l))
          SpLinesAdd(s,l,idsplBreak,True);
      }
   }
}

/**[txh]********************************************************************

  Description:
  Initializes the debug object and related variables.
  
  Return: !=0 OK
  
***************************************************************************/

int TSetEditorApp::DebugInitVars()
{
 if (!dbg)
    dbg=new MIDebugger();
 return dbg!=NULL;
}

/**[txh]********************************************************************

  Description:
  Releases the memory used. If the debug session still active it tries to
stop the program and close the gdb connection.
  
***************************************************************************/

void TSetEditorApp::DebugDeInitVars()
{
 if (dbg)
   {
    delete dbg; // It will close the debug session
    dbg=NULL;
   }
 TProgram::deskTop->lock();
 if (DEBUG_BREAKPOINTS_UPDATE)
    dbgPr("DebugDeInitVars: Deleting all splines\n");
 SpLinesDeleteForId(idsplBreak);
 DBG_ReleaseMemory();
 DebugMsgDeInit();
 WatchesDeInit();
 DebugCommonCleanUp();
 TProgram::deskTop->unlock();
}

/**[txh]********************************************************************

  Description:
  Actions shared between @x{::DebugDeInitVars} and @x{::DebugCloseSession}.
  
***************************************************************************/

void TSetEditorApp::DebugCommonCleanUp()
{
 DebugClearCPULine();
 mi_free_stop(stoppedInfo);
 stoppedInfo=NULL;
 pendingStoppedInfo=killAfterStop=0;
 DebugCommandsForDisc();
 delete[] binReference;
 binReference=NULL;
 binReferenceLen=0;
}

/**[txh]********************************************************************

  Description:
  It verifies if the debugger is in a state that can accept things like
breakpoints, watchpoints, etc. The possible states are target_specified and
stopped. If the state is running we won't stop. For other states we will try
to reach target_specified.@p
  If @var{showConnect} is specified the message dialog will be selected to
show success. On error it is always selected.
  
  Return: !=0 OK
  
***************************************************************************/

int TSetEditorApp::DebugCheckStopped(Boolean showConnect)
{
 if (!DebugInitVars())
    return 0;
 MIDebugger::eState st=dbg->GetState();
 if (st==MIDebugger::running)
   {
    messageBox(__("The program is running and GDB/MI doesn't support async commands"),
               mfOKButton | mfError);
    return 0;
   }
 if (st==MIDebugger::disconnected)
   {
    if (!DebugConnect())
       return 0;
    st=dbg->GetState(); // Should be connected
   }
 if (st==MIDebugger::connected)
   {
    if (!DebugSelectTarget(showConnect))
       return 0;
    st=dbg->GetState(); // Should be target_specified
   }
 return (st==MIDebugger::stopped || st==MIDebugger::target_specified);
}

/**[txh]********************************************************************

  Description:
  It verifies if the debugger is in a state that can accept commands. The
possible states are target_specified, connected and stopped. If the state
is running we won't stop. For other states we will try to reach
connected.@p
  If @var{showConnect} is specified the message dialog will be selected to
show success. On error it is always selected.
  
  Return: !=0 OK
  
***************************************************************************/

int TSetEditorApp::DebugCheckAcceptCmd(Boolean showConnect)
{
 if (!DebugInitVars())
    return 0;
 MIDebugger::eState st=dbg->GetState();
 if (st==MIDebugger::running)
   {
    messageBox(__("The program is running and GDB/MI doesn't support async commands"),
               mfOKButton | mfError);
    return 0;
   }
 if (st==MIDebugger::disconnected)
   {
    if (!DebugConnect())
       return 0;
    st=dbg->GetState(); // Should be connected
   }
 return (st==MIDebugger::stopped || st==MIDebugger::target_specified ||
         st==MIDebugger::connected);
}

/**[txh]********************************************************************

  Description:
  Tries to connect to gdb. It doesn't check if we are initialized or in
which state is the debugger. Call it only after checking.
  
  Return: !=0 OK
  
***************************************************************************/

int TSetEditorApp::DebugConnect()
{
 // TODO: REMOVE IT!!!
 //dbg->SetGDBExe("/mnt/hda3/var/tmp/Compartido/set/src/gdb-6.1.1/gdb/gdb");
 int res=dbg->Connect();
 if (res)
   {
    DBG_SetCallBacks();
    DebugMsgSetState();
    // TODO: optional?
    dbg->Version();
    // Set the path for sources
    char n[PATH_MAX];
    dbg->PathSources(NULL);
    for (int i=0; PathListGetItem(i,n,paliSource); i++)
        dbg->PathSources(n);
    // At the end so it becomes the fist in the list:
    // Yet another gdb bug?? some gdbs need it
    if (getcwd(n,PATH_MAX))
       dbg->PathSources(n);
   }
 else
    DebugMsgSetError();

 return res;
}


static
TDialog *createPidDlg()
{
 TSViewCol *col=new TSViewCol(__("PID (Process ID) of the running process"));

 // EN: P
 TSHzLabel *o1=new TSHzLabel(__("~P~ID:"),new TSInputLine(widthPID));

 col->insert(xTSLeft,yTSUpSep,o1);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(cmGDBCommand);
 delete col;
 return d;
}

static
int GetPID()
{
 char pid[widthPID];
 pid[0]=0;
 if (execDialog(createPidDlg(),pid)==cmOK)
    return atoi(pid);
 return 0;
}

static
int IsEmpty(const char *s)
{
 for (;*s && ucisspace(*s); s++);
 return *s==0;
}

/**[txh]********************************************************************

  Description: 
  The sources reported in "frames" are relative to the binary or just
absolute. If the user specifies "path/binary" we have to use the "path" to
look for sources.
  
***************************************************************************/

static
void ExtractBinReference()
{
 char b[PATH_MAX], path[PATH_MAX];
 strcpy(b,dOps.program);
 CLY_fexpand(b);
 CLY_ExpandPath(b,path,NULL);
 delete[] binReference;
 binReferenceLen=strlen(path);
 binReference=new char[binReferenceLen+1];
 memcpy(binReference,path,binReferenceLen+1);
 if (dbg)
    dbg->PathSources(binReference);
}

/**[txh]********************************************************************

  Description:
  Tries to select the target to debug. It doesn't check if we are
initialized or in which state is the debugger. Call it only after checking.
  If @var{showConnect} is specified the message dialog will be selected to
show success. On error it is always selected.
  
  Return: !=0 OK
  
***************************************************************************/

int TSetEditorApp::DebugSelectTarget(Boolean showConnect)
{
 if (debugOpsNotIndicated)
   {// We don't even know what to debug ;-)
    if (messageBox(__("You must set the debug options first. Do you want to do it now?"),
        mfYesButton | mfNoButton)!=cmYes)
       return 0;
    // Give a chance to say what to debug and how
    if (!DebugOptionsEdit())
       return 0;
   }

 int res=0, pid;
 char *args, *tty, *aux;
 mi_frames *fr;

 if (!edTestForFile(dOps.program))
   {// Don't even try if the program doesn't exist
    messageBox(mfError | mfOKButton,__("The '%s' file doesn't exist"),dOps.program);
    return 0;
   }
 switch (dOps.mode)
   {
    case dmLocal:
         // Now try
         args=dOps.args[0] ? dOps.args : NULL;
         tty=dOps.tty[0] ? dOps.tty : NULL;
         if (strcmp(TScreen::getDriverShortName(),"X11")==0)
           {
            localMode=modeX11;
            res=dbg->SelectTargetX11(dOps.program,args,tty);
           }
         else
           {
            localMode=modeLinux;
            res=dbg->SelectTargetLinux(dOps.program,args,tty);
           }
         break;

    case dmPID:
         pid=GetPID();
         if (!pid)
            return 0;
         if (pid==getpid())
           {
            messageBox(__("Hey! That's my PID!"),mfError | mfOKButton);
            return 0;
           }
         fr=dbg->SelectTargetPID(dOps.program,pid);
         if (fr)
           {
            res=1;
            // TODO: i18n
            char b[maxWStatus];
            int l=CLY_snprintf(b,maxWStatus,__("Attached to PID %d"),pid);
            DebugMsgJumpToFrame(fr,b,l);
            mi_free_frames(fr);
           }
         break;

    case dmRemote:
         if (IsEmpty(dOps.rtype) || IsEmpty(dOps.rparam))
           {// Don't even try if nothing to try ;-)
            messageBox(__("Please fill both 'Remote target options'. Try extended-remote for the type and IP:PORT for location."),
                       mfError | mfOKButton);
            return 0;
           }
         // Check if we have something that looks like TCP/IP location
         aux=strchr(dOps.rparam,':');
         if (aux)
           {
            *aux=0;
            messageBox(mfInformation | mfOKButton,
                       __("At the '%s' machine run: 'gdbserver :%s program args...'"),
                       dOps.rparam,aux+1);
            *aux=':';
           }
         else
           {
            messageBox(__("Run gdbserver at the remote machine"),
                       mfInformation | mfOKButton);
           }
         res=dbg->SelectTargetRemote(dOps.program,dOps.rparam,dOps.rtype);
         break;
   }

 if (!res)
    DebugMsgSetError();
 else
   {
    DebugMsgSetState();
    DebugMsgSetMode(showConnect);
    // Apply breakpoints.
    DBG_ApplyBkpts();
    ExtractBinReference();
   }

 return res;
}

/**[txh]********************************************************************

  Description:
  This function dis/enables the commands related to debug operations. It
should be called each time we change the debug state.
  
***************************************************************************/

void TSetEditorApp::DebugUpdateCommands()
{
 if (!dbg)
    return;
 MIDebugger::eState st=dbg->GetState();
 switch (st)
   {
    case MIDebugger::disconnected:
         DebugCommandsForDisc();
         break;
    case MIDebugger::connected:
         TSetEditorApp::setCmdState(cmeDebugOptions,False);
         TSetEditorApp::setCmdState(cmeDbgEndSession,True);
         TSetEditorApp::setCmdState(cmeDbgCloseSession,True);
         TSetEditorApp::setCmdState(cmGDBCommand,True);
         TSetEditorApp::setCmdState(cmeDbgGoConnected,False);
         TSetEditorApp::setCmdState(cmeDbgGoReadyToRun,True);
         // When a PID exits we go directly to connected:
         TSetEditorApp::setCmdState(cmeBreakpoint,True);
         TSetEditorApp::setCmdState(cmeDbgRunContinue,True);
         TSetEditorApp::setCmdState(cmeDbgStepOver,True);
         TSetEditorApp::setCmdState(cmeDbgTraceInto,True);
         TSetEditorApp::setCmdState(cmeDbgGoToCursor,True);
         TSetEditorApp::setCmdState(cmeDbgStop,False);
         TSetEditorApp::setCmdState(cmeDbgKill,False);
         TSetEditorApp::setCmdState(cmeDbgCallStack,False);
         break;
    case MIDebugger::stopped:
         TSetEditorApp::setCmdState(cmeDbgFinishFun,True);
         TSetEditorApp::setCmdState(cmeDbgReturnNow,True);
         TSetEditorApp::setCmdState(cmeDbgCallStack,True);
    case MIDebugger::target_specified:
         TSetEditorApp::setCmdState(cmeBreakpoint,True);
         TSetEditorApp::setCmdState(cmeDbgRunContinue,True);
         TSetEditorApp::setCmdState(cmeDbgStepOver,True);
         TSetEditorApp::setCmdState(cmeDbgTraceInto,True);
         TSetEditorApp::setCmdState(cmeDbgGoToCursor,True);
         TSetEditorApp::setCmdState(cmGDBCommand,True);
         TSetEditorApp::setCmdState(cmeDbgStop,False);
         TSetEditorApp::setCmdState(cmeDbgGoReadyToRun,False);
         if (st!=MIDebugger::stopped)
           {
            TSetEditorApp::setCmdState(cmeDbgKill,False);
            TSetEditorApp::setCmdState(cmeDbgCallStack,False);
           }
         break;
    case MIDebugger::running:
         TSetEditorApp::setCmdState(cmeBreakpoint,False);
         TSetEditorApp::setCmdState(cmeDbgRunContinue,False);
         TSetEditorApp::setCmdState(cmeDbgStepOver,False);
         TSetEditorApp::setCmdState(cmeDbgTraceInto,False);
         TSetEditorApp::setCmdState(cmeDbgGoToCursor,False);
         TSetEditorApp::setCmdState(cmeDbgFinishFun,False);
         TSetEditorApp::setCmdState(cmeDbgReturnNow,False);
         TSetEditorApp::setCmdState(cmeDbgStop,True);
         TSetEditorApp::setCmdState(cmeDbgKill,True);
         TSetEditorApp::setCmdState(cmeDbgCallStack,False);
         TSetEditorApp::setCmdState(cmGDBCommand,False);
         break;
   }
}

/**[txh]********************************************************************

  Description:
  Sets the commands for the disconnected state. Must be called when we
asynchronously abort the debug session or at the beggining of the program.
  
***************************************************************************/

void TSetEditorApp::DebugCommandsForDisc()
{
 TView::commandSetChanged=True;
 TView::curCommandSet.enableCmd(cmeDbgFirstCommand,cmeDbgLastCommand);
 TView::curCommandSet-=cmeDbgFinishFun;
 TView::curCommandSet-=cmeDbgReturnNow;
 TView::curCommandSet-=cmeDbgStop;
 TView::curCommandSet-=cmeDbgKill;
 TView::curCommandSet-=cmeSelDebugWin;
 TView::curCommandSet-=cmeSelWatchesWin;
 TView::curCommandSet-=cmeDbgEndSession;
 TView::curCommandSet-=cmeDbgCloseSession;
 TView::curCommandSet-=cmGDBCommand;
}

/**[txh]********************************************************************

  Description:
  Starts running the program or resumes execution. It tries to be smart.  
  
***************************************************************************/

void TSetEditorApp::DebugRunOrContinue()
{
 if (!DebugCheckStopped())
    return;
 if (dbg->RunOrContinue())
    DebugMsgSetState();
}


static
Boolean DeskTopIsModal()
{
 // Check if the desktop is executing a modal dialog
 TView *p=TApplication::deskTop->current;
 return (p && (p->state & sfModal)) ? True : False;
}

/**[txh]********************************************************************

  Description:
  Poll gdb looking for async responses. Must be called from idle. If an
error is received it is displayed in the message window. If the program
stops the line where we stopped is selected.
  
***************************************************************************/

void TSetEditorApp::DebugPoll()
{
 if (!dbg)
    return;

 // We want to communicate a change
 if (pendingStoppedInfo)
   {
    if (DeskTopIsModal())
       return;
    pendingStoppedInfo=0;
    if (stoppedInfo)
       DebugMsgSetStopped();
    else
      {
       DebugMsgSetError();
       DebugMsgSetState();
      }
    return;
   }

 mi_stop *rs;
 if (dbg->Poll(rs))
   {
    // Discard the previous info
    mi_free_stop(stoppedInfo);
    stoppedInfo=rs;
    // Process the new one
    if (DeskTopIsModal())
      {// Wait until the desktop isn't modal
       pendingStoppedInfo=1;
       return;
      }
    if (stoppedInfo)
       DebugMsgSetStopped();
    else
      {
       DebugMsgSetError();
       DebugMsgSetState();
      }
   }
}


/**[txh]********************************************************************

  Description:
  Steps in the code. It tries to be smart.
  
***************************************************************************/

void TSetEditorApp::DebugStepOver()
{
 if (!DebugCheckStopped())
    return;
 if (dbg->StepOver())
    DebugMsgSetState();
 else
    DebugMsgSetError();
}

/**[txh]********************************************************************

  Description:
  Steps in the code, following functio calls. It tries to be smart.
  
***************************************************************************/

void TSetEditorApp::DebugTraceInto()
{
 if (!DebugCheckStopped())
    return;
 if (dbg->TraceInto())
    DebugMsgSetState();
 else
    DebugMsgSetError();
}

/**[txh]********************************************************************

  Description:
  Executes until the cursor position. It tries to be smart.
  
***************************************************************************/

void TSetEditorApp::DebugGoToCursor()
{
 TCEditor *e=GetCurrentIfEditor();
 if (!e)
    return;
 if (!DebugCheckStopped())
    return;
 if (dbg->GoTo(e->fileName,e->curPos.y+1))
    DebugMsgSetState();
 else
    DebugMsgSetError();
}

/**[txh]********************************************************************

  Description:
  Executes until the end of the current function. Not smart, only works for
stopped state.
TODO: Should we check if we are at main? Not sure, the check is expensive:
1) Get the list of frames, 2) Determine if we are at the top, 3) Release the
frames. First time I got the gdb error I got confused, but is quite obvious.
  
***************************************************************************/

void TSetEditorApp::DebugFinishFun()
{
 if (!dbg || dbg->GetState()!=MIDebugger::stopped)
    return;
 if (dbg->FinishFun())
    DebugMsgSetState();
 else
    DebugMsgSetError();
}

/**[txh]********************************************************************

  Description:
  Finishes current function and goes to the previous frame. Not smart, only
works for stopped state.
TODO: Should we check if we are at main? Not sure, the check is expensive:
1) Get the list of frames, 2) Determine if we are at the top, 3) Release the
frames. First time I got the gdb error I got confused, but is quite obvious.
  
***************************************************************************/

void TSetEditorApp::DebugReturnNow()
{
 if (!dbg || dbg->GetState()!=MIDebugger::stopped)
    return;
 mi_frames *f=dbg->ReturnNow();
 if (f)
   {// TODO: i18n
    char b[maxWStatus];
    int l=DebugMsgFillReason(f,b,False);
    DebugMsgJumpToFrame(f,b,l);
    mi_free_frames(f);
   }
 else
    DebugMsgSetError();
}

/**[txh]********************************************************************

  Description:
  Sends a SIGINT to the debugger so it stops the debug session. Lamentably
GDB/MI isn't fully asynchronous and hence -exec-interrupt can't be used.
Isn't smart and should be used only when running.
  
***************************************************************************/

void TSetEditorApp::DebugStop()
{
 if (!dbg || dbg->GetState()!=MIDebugger::running)
    return;
 if (dbg->Stop())
    DebugMsgSetState();
 else
    DebugMsgSetError();
}

/**[txh]********************************************************************

  Description:
  Tries to kill the program. If we are running we must do it in two steps:
1) We stop the program and recover gdb prompt and 2) We actually kill it.
  
***************************************************************************/

void TSetEditorApp::DebugKill()
{
 if (!dbg)
    return;
 if (dbg->GetState()==MIDebugger::running)
   {// We must recover gdb control first
    DebugStop();
    // We do it in 2 steps
    killAfterStop=1;
    return;
   }
 if (dbg->GetState()==MIDebugger::stopped)
   {
    if (dbg->Kill())
       DebugMsgSetState();
    else
       DebugMsgSetError();
   }
}

/**[txh]********************************************************************

  Description:
  Sends the stack frames to the message window so you can browse the calling
sequence. Isn't smart and must be used only during "stopped".
  
***************************************************************************/

void TSetEditorApp::DebugCallStack()
{
 if (!dbg || dbg->GetState()!=MIDebugger::stopped)
    return;
 mi_frames *f=dbg->CallStack(true), *r;
 if (!f)
   {
    DebugMsgSetError();
    return;
   }

 unsigned options;
 FileInfo fI;
 char b[maxWStatus];
 DynStrCatStruct msg;
 fI.Column=1;
 fI.offset=-1;
 fI.len=0;

 for (r=f; r; r=r->next)
    {
     options=0;
     if (r==f) // First
        options|=edsmRemoveOld;
     if (!r->next) // Last
        options|=edsmUpdateSpLines;
     else
        options|=edsmDontUpdate;
     fI.Line=r->line;
     // Convert the frame into something "human readable"
     // TODO i18n
     int l=CLY_snprintf(b,maxWStatus,"%d: %s:%s:%d addr %p",r->level,
                        r->func ? r->func : __("unknown"),
                        r->file ? r->file : __("unknown"),
                        r->line,r->addr);
     DynStrCatInit(&msg,b,l);
     // Add the function arguments.
     // TODO: should I modify mi lib to return something better ...
     mi_results *args=r->args;
     if (args)
       {
        DynStrCat(&msg," args: (");
        // messy ;-P
        while (args)
          {
           if (args->type==t_tuple && args->v.rs)
             {
              mi_results *name=args->v.rs;
              mi_results *val=name->next;
              if (val && name->type==t_const && val->type==t_const)
                {
                 l=CLY_snprintf(b,maxWStatus,"%s=%s",name->v.cstr,val->v.cstr);
                 DynStrCat(&msg,b,l);
                }
             }
           args=args->next;
           if (args)
              DynStrCat(&msg,", ",2);
          }
        DynStrCat(&msg,")",1);
       }
     EdShowMessageFile(msg.str,fI,r->file,options);
     free(msg.str);
    }
 mi_free_frames(f);
}

/**[txh]********************************************************************

  Description:
  Evaluates the provided expression. Tries to be smart.
  
  Return: The result, the error description or NULL for incorrect states.
  
***************************************************************************/

char *TSetEditorApp::DebugEvalExpression(char *exp)
{
 if (!DebugCheckAcceptCmd())
    return NULL;
 return dbg->EvalExpression(exp);
}

/**[txh]********************************************************************

  Description:
  Modifies the provided expression. Tries to be smart.
  
  Return: The result, the error description or NULL for incorrect states.
  
***************************************************************************/

char *TSetEditorApp::DebugModifyExpression(char *exp, char *newVal)
{
 if (!DebugCheckAcceptCmd())
    return NULL;
 return dbg->ModifyExpression(exp,newVal);
}

class TDbgEvalModify : public TDialog
{
public:
 TDbgEvalModify() :
   TWindowInit(&TDbgEvalModify::initFrame),
   TDialog(TRect(1,1,1,1),__("Evaluate and Modify")) {};
 virtual void handleEvent(TEvent &);
 void SetStartVal(char *val);

 TInputLinePiped *exp;
 TInputLinePiped *res;
 TInputLine *val;
};

/**[txh]********************************************************************

  Description:
  Command loop for the Debug Evaluate & Modify dialog. It calls
DebugEvalExpression and DebugModifyExpression and redirects copy/paste
actions to the correct input line.
  
***************************************************************************/

void TDbgEvalModify::handleEvent(TEvent &event)
{
 char inputBuffer[widthFiles];
 char valBuffer[widthFiles];
 char *ret;

 TDialog::handleEvent(event);
 if (event.what==evCommand)
    switch (event.message.command)
      {
       case cmEval:
             exp->getData(inputBuffer);
             ret=TSetEditorApp::DebugEvalExpression(inputBuffer);
             if (ret)
               {
                res->setData(ret);
                free(ret);
                exp->selectAll(True);
               }
            clearEvent(event);
            break;
       case cmChange:
             exp->getData(inputBuffer);
             val->getData(valBuffer);
             ret=TSetEditorApp::DebugModifyExpression(inputBuffer,valBuffer);
             if (ret)
               {
                res->setData(ret);
                free(ret);
                exp->selectAll(True);
               }
             clearEvent(event);
            break;
       case cmCaCopy:
            event.message.command=cmtilCopy;
            res->handleEvent(event);
            break;
       case cmCaPaste:
            event.message.command=cmtilPaste;
            exp->handleEvent(event);
            break;
      }
}

/**[txh]********************************************************************

  Description:
  Sets the starting value for the "Expression".
  
***************************************************************************/

void TDbgEvalModify::SetStartVal(char *val)
{
 if (val)
   {
    char aux[widthFiles];
    strncpy(aux,val,widthFiles);
    exp->setData(aux);
   }
}

/**[txh]********************************************************************

  Description:
  Creates the debugger eval/modify dialog.
  
  Return: TDialog pointer to the new dialog.
  
***************************************************************************/

TDbgEvalModify *createEvalModifyDialog()
{
 TDbgEvalModify *d=new TDbgEvalModify();
 TSViewCol *col=new TSViewCol(d);

 TSInputLinePiped *sExp=
   new TSInputLinePiped(widthFiles,1,hID_DbgEvalModifyExp,maxWBox);
 TSInputLinePiped *sRes=
   new TSInputLinePiped(widthFiles,0,0,maxWBox,tilpNoPipe | tilpNoPaste);
 TSInputLine *sVal=new TSInputLine(widthShort,1,hID_DbgEvalModifyNewV,maxWBox);
 d->exp=(TInputLinePiped *)sExp->view;
 d->res=(TInputLinePiped *)sRes->view;
 d->val=(TInputLine *)sVal->view;

 // EN: CEHPRNV
 TSVeGroup *o1=
 MakeVeGroup(0, // All together
             new TSLabel(__("~E~xpression (escape \" characters: \\\")"),sExp),
             new TSLabel(__("~R~esult"),sRes),
             new TSLabel(__("~N~ew value"),sVal),
             0);
 //o1->makeSameW();
 TSHzGroup *o2=
 MakeHzGroup(new TSButton(__("E~v~al"),cmEval,bfDefault),
             new TSButton(__("C~h~ange"),cmChange),
             new TSButton(__("Cancel"),cmCancel),
             new TSButton(__("~C~opy"),cmCaCopy),
             new TSButton(__("~P~aste"),cmCaPaste),
             0);

 col->insert(xTSLeft,yTSUp,o1);
 col->insert(xTSCenter,yTSDown,o2);

 col->doItCenter(cmeDbgEvalModify);
 delete col;
 return d;
}


/**[txh]********************************************************************

  Description:
  That's the high level function to bring the Debug Evaluate & Modify
dialog. It ensures the current state is correct and executes the dialog.
  
***************************************************************************/

void TSetEditorApp::DebugEvalModify(char *startVal)
{
 if (!DebugCheckAcceptCmd())
    return;
 TDbgEvalModify *d=createEvalModifyDialog();
 d->SetStartVal(startVal);
 TProgram::deskTop->execView(d);
 // Dialogs should be destroyed or your members won't de deleted.
 CLY_destroy(d);
 delete[] startVal;
}

/**[txh]********************************************************************

  Description:
  Removes the "CPU Line". Currently it doesn't force a redraw. This have a
side effect: CPU lines remains painted when the next CPU line isn't in the
same file. I like it, but could change in the future, needs more testing.
  
***************************************************************************/

void DebugClearCPULine()
{
 if (!cpuLFile)
    return;
 SpLinesDeleteForId(idsplRunLine,cpuLFile,True,cpuLLine);
 delete[] cpuLFile;
 cpuLFile=NULL;
}

/**[txh]********************************************************************

  Description:
  Sets the "CPU Line" and remmembers its position to allow removing it using
@x{DebugClearCPULine}. Is called from @x{GotoFileLine}.
  
***************************************************************************/

void DebugSetCPULine(int line, char *file)
{
 DebugClearCPULine();
 SpLinesAdd(file,line,idsplRunLine,True);
 cpuLFile=newStr(file);
 cpuLLine=line;
}

/**[txh]********************************************************************

  Description:
  Verifies if we have an active debug session. If that's the case asks the
user if it's ok to end it. Should be called when closing the project,
exiting the application, etc. Note it doesn't close the session. Use
@x{::DebugDeInitVars} for this.
  
  Return: !=0, ok, go ahead.
  
***************************************************************************/

int TSetEditorApp::DebugConfirmEndSession(Boolean directRequest)
{
 if (!dbg)
    return 1; // Ok, we are not debugging.
 char *msg;
 if (directRequest)
    msg=__("Please confirm you really want to finish the debug session. Breakpoints and other things will be lost.");
 else
    msg=__("A debug session is active. Do you want to stop it?");
 if (messageBox(msg,mfConfirmation | mfYesButton | mfNoButton)==cmYes)
    return 1;
 return 0;
}

/**[txh]********************************************************************

  Description:
  That's a light alternative to @x{::DebugDeInitVars}. It doesn't kill the
list of breakpoints, watchpoints, etc. It just closes gdb connection and
hides the windows. Asks for confirmation if we are running or stopped.
  
***************************************************************************/

void TSetEditorApp::DebugCloseSession()
{
 if (dbg)
   {
    MIDebugger::eState st=dbg->GetState();
    if (st==MIDebugger::running || st==MIDebugger::stopped &&
        messageBox(__("It will kill the program you are debugging. Go ahead?"),
                   mfConfirmation | mfYesButton | mfNoButton)!=cmYes)
       return;
   }
 if (dbg)
   {
    delete dbg; // It will close the debug session
    dbg=NULL;
   }
 TProgram::deskTop->lock();
 DebugMsgClose();
 WatchesClose();
 DebugCommonCleanUp();
 TProgram::deskTop->unlock();
}

/*****************************************************************************
  End of Editor debug commands
*****************************************************************************/

/*****************************************************************************
  GDB/MI interface
*****************************************************************************/

static mi_bkpt *firstB=NULL, *lastB=NULL;

static
void DBG_AddBkpt(mi_bkpt *b)
{
 if (firstB)
    lastB->next=b;
 else
    firstB=b;
 lastB=b;
}

static
mi_bkpt *DBG_SearchBkpt(const char *source, int line)
{
 mi_bkpt *b=firstB;
 char n[2*PATH_MAX];

 while (b)
   {
    if (binReference)
       memcpy(n,binReference,binReferenceLen);
    strcpy(n+binReferenceLen,b->file);
    CLY_fexpand(n);
    if (strcmp(n,source)==0 && b->line==line)
       return b;
    b=b->next;
   }
 return b;
}

static
void DBG_RemoveBkpt(mi_bkpt *b)
{
 mi_bkpt *e=firstB, *ant=NULL;

 while (e)
   {
    if (e==b)
      {
       if (ant)
          ant->next=b->next;
       else
          firstB=b->next;
       if (!b->next)
          lastB=ant;
       b->next=NULL;
       mi_free_bkpt(b);
       return;
      }
    ant=e;
    e=e->next;
   }
 // TODO: Remove
 dbgPr("Oops! can't find bkp DBG_RemoveBkpt()\n");
}

int DBG_SetBreakpoint(const char *source, int line)
{
 mi_bkpt *b=dbg->Breakpoint(source,line);
 if (b)
   {
    DBG_AddBkpt(b);
    return 1;
   }
 DebugMsgSetError();
 return 0;
}

int DBG_RemoveBreakpoint(const char *source, int line)
{
 mi_bkpt *b=DBG_SearchBkpt(source,line);
 if (!b)
   {// TODO: Remove
    dbgPr("Oops! where is bkpt %s:%d\n",source,line);
    return 0;
   }
 if (!dbg->BreakDelete(b))
   {
    DebugMsgSetError();
    return 0;
   }
 DBG_RemoveBkpt(b);
 return 1;
}

static
void AddSpLineBinRef(const char *file, int line)
{
 char n[2*PATH_MAX];

 if (binReference)
    memcpy(n,binReference,binReferenceLen);
 strcpy(n+binReferenceLen,file);
 CLY_fexpand(n);
 
 if (DEBUG_BREAKPOINTS_UPDATE)
    dbgPr("Adding SpLine for %s:%d\n",n,line);
 SpLinesAdd(n,line,idsplBreak,False);
}

/**[txh]********************************************************************

  Description:
  Transfer our list to gdb and set the lines as special ones.
  
***************************************************************************/

void DBG_ApplyBkpts()
{
 if (!dbg)
    return;
 if (DEBUG_BREAKPOINTS_UPDATE)
    dbgPr("DBG_ApplyBkpts: Deleting all splines\n");
 SpLinesDeleteForId(idsplBreak); // Ensure no previous bkpt survived ;-)
 mi_bkpt *b=firstB, *aux;
 // Disconnect current list, we will be creating a new one.
 firstB=lastB=NULL;
 int disabledBkpts=0, killIt, applied=0;

 while (b)
   {
    killIt=0;
    if (b->enabled) // Avoid disabled ones.
      {
       // TODO: b->type currently ignored, is that HW assisted?
       // TODO: I'm quite sure we will lose something in the transfer, make
       // sure it doesn't.
       mi_bkpt *nb=dbg->BreakpointFull(b->file,b->line,b->disp==d_del,b->cond,
                                       b->times,b->thread,false);
       if (!nb)
         {
          b->enabled=0;
          DBG_AddBkpt(b);
          disabledBkpts++;
         }
       else
         {
          DBG_AddBkpt(nb);
          killIt=1;
          AddSpLineBinRef(nb->file,nb->line);
          applied++;
         }
      }
    else
       // Pass unchanged
       DBG_AddBkpt(b);
    aux=b->next;
    if (killIt)
      {
       b->next=NULL;
       mi_free_bkpt(b);
      }
    b=aux;
   }
 if (applied)
    SpLinesUpdate();
 if (disabledBkpts)
    messageBox(mfWarning | mfOKButton,
               __("%d breakpoints failed to apply, they are disabled now."),
               disabledBkpts);
}

void DBG_SaveBreakpoints(opstream &os)
{
 int32 count=0;
 // Count them
 mi_bkpt *p=firstB;
 while (p)
   {
    count++;
    p=p->next;
   }
 // CAUTION!!! update the dummy
 // Save them
 os << bkptsVersion << count;
 dbgPr("%d Breakpoints\n",count);
 p=firstB;
 while (p)
   {
    os.writeString(p->file);
    os.writeString(p->cond);
    os << (int32)p->line << p->enabled << (int32)p->times
       << (char)p->type << (char)p->disp;
    p=p->next;
   }
}

static
char *ReadStringC(ipstream &is)
{
 char *aux=is.readString(), *str;
 if (!aux)
    return NULL;
 str=strdup(aux);
 delete[] aux;
 return str;
}

inline static
int ReadInt32(ipstream &is)
{
 int32 aux;
 is >> aux;
 return aux;
}

inline static
char ReadChar(ipstream &is)
{
 char aux;
 is >> aux;
 return aux;
}

void DBG_ReadBreakpoints(ipstream &is)
{
 int32 count;
 char version;

 is >> version >> count;
 // TODO:
 if (firstB)
    dbgPr("Oops! loading breakpoints when we already have!!!\n");
 for (int32 i=0; i<count; i++)
    {
     mi_bkpt *b=mi_alloc_bkpt();
     if (b)
       {
        b->file=ReadStringC(is);
        b->cond=ReadStringC(is);
        b->line=ReadInt32(is);
        is >> b->enabled;
        b->times=ReadInt32(is);
        b->type=(enum mi_bkp_type)ReadChar(is);
        b->disp=(enum mi_bkp_disp)ReadChar(is);
        DBG_AddBkpt(b);
        AddSpLineBinRef(b->file,b->line);
       }
    }
 if (count)
    SpLinesUpdate();
}

static
char prefixList[]="CTL><";

void DBG_GenericCB(const char *m, void *p)
{
 int l=strlen(m);
 char *b=new char[l+4];
 b[0]=*((char *)p);
 b[1]=':';
 b[2]=' ';
 memcpy(b+3,m,l+1);
 // Remove LF if present
 if (b[l+2]=='\n')
    b[l+2]=0;
 DebugMsgAdd(b);
}

void DBG_SetCallBacks()
{
 if (!dbg)
    return;
 dbg->SetConsoleCB(msgOps & msgConsole ? DBG_GenericCB : NULL,prefixList+0);
 dbg->SetTargetCB(msgOps & msgTarget ? DBG_GenericCB : NULL,prefixList+1);
 dbg->SetLogCB(msgOps & msgLog ? DBG_GenericCB : NULL,prefixList+2);
 dbg->SetToGDBCB(msgOps & msgTo ? DBG_GenericCB : NULL,prefixList+3);
 dbg->SetFromGDBCB(msgOps & msgFrom ? DBG_GenericCB : NULL,prefixList+4);
}

void DBG_ReleaseMemory()
{
 mi_free_bkpt(firstB);
 firstB=lastB=NULL;
}

/*****************************************************************************
  End of GDB/MI interface
*****************************************************************************/

/*****************************************************************************

  Debug Status and Messages Window

/------------------------------------------------------------------------------\
|C blabla                                                                     ^|
|> Blelelel                                                                   ||
|< Ppepepepep                                                                 ||
|                                                                             V|
|<---------------------------------------------------------------------------> |
|------------------------------------------------------------------------------|
|Mode: Remote (192.168.1.2:5000) [target.exe]                                  |
|Reason stopped: Hit a breakpoint                                              |
\-Running----------------------------------------------------------------------/

|------------------------------------------------------------------------------|
|Mode: X11 (/dev/pts/5) [target.exe]                                           |
|Error: From GDB (Not such a thing)                                            |
\-Stopped----------------------------------------------------------------------/

*****************************************************************************/

class TDebugMsgDialog : public TDialog
{
public:
 TDebugMsgDialog(const TRect &aR, const char *tit);
 void SetStatusGDB(char *s) { statusF->setText(s); };
 void SetStatusStop(char *s) { status2->setText(s); };
 void SetStatusError(char *s) { status2->setText(s); };
 void SetStatusMode(char *s) { status1->setText(s); };
 virtual void changeBounds(const TRect &r);
 virtual void close();
 virtual void handleEvent(TEvent &event);
 void realClose() { TDialog::close(); };
 void SendGDBCommand();

protected:
 TNoStaticText *status1, *status2, *statusF;
};

// We need a TCollection for the TListBox.
class TMsgCollection : public TCollection
{
public:
 TMsgCollection(ccIndex lim, ccIndex delta) : TCollection(lim, delta) {}

private:
 virtual void *readItem( ipstream& ) { return 0; }
 virtual void writeItem( void *, opstream& ) {}
};

/*****************************************************************************
  Dsk Wrapper for Debug Status and Messages Window
*****************************************************************************/

class TDskDbgMsg : public TDskWin
{
public:
 TDskDbgMsg(TView *w);
 ~TDskDbgMsg();

 //int  GoAction(ccIndex i);
 int  DeleteAction(ccIndex i, Boolean fromDiskToo=False);
 char *GetText(char *dest, short maxLen);
 int  Compare(void *p,int t) { return (t==dktDbgMsg); };

 void write(opstream& os);
 void *read(ipstream& is);

 const char *streamableName() const
     { return name; }

protected:
 TDskDbgMsg(StreamableInit) { type=dktDbgMsg; CanBeSaved=0; CanBeDeletedFromDisk=0; };

public:
 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TDskDbgMsg& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskDbgMsg*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskDbgMsg& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskDbgMsg* cl )
    { return os << (TStreamable *)cl; }

TDskDbgMsg::TDskDbgMsg(TView *w)
{
 view=w;
 type=dktDbgMsg;
 CanBeDeletedFromDisk=0;
 CanBeSaved=0;
 ZOrder=-1;
 //wS=TScreen::getCols();
 //hS=TScreen::getRows();
}

TDskDbgMsg::~TDskDbgMsg()
{
 dbgPr("~TDskDbgMsg\n");
}

int TDskDbgMsg::DeleteAction(ccIndex, Boolean)
{
 view->hide();
 return 0;
}

char *TDskDbgMsg::GetText(char *dest, short maxLen)
{
 char *msg=DebugMsgStateName();
 if (!msg)
    msg=__("Not started");
 // TODO: i18n
 TVIntl::snprintf(dest,maxLen,__("   Debugger Window [%s]"),msg);
 return dest;
}

const char * const TDskDbgMsg::name="TDskDbgMsg";

TStreamable *TDskDbgMsg::build()
{
 return new TDskDbgMsg(streamableInit);
}

void TDskDbgMsg::write(opstream &)
{
}

void *TDskDbgMsg::read(ipstream &)
{
 return this;
}

/*****************************************************************************
  End of Dsk Wrapper for Debug Status and Messages Window
*****************************************************************************/

// TODO: enclose in a class
static TDebugMsgDialog *MsgWindow=NULL;
static TMsgCollection  *MsgCol   =NULL;
static TListBox        *MsgList  =NULL;
static TRect MsgWindowRect(-1,-1,-1,-1);

TDebugMsgDialog::TDebugMsgDialog(const TRect &aR, const char *tit) :
    TWindowInit(TDebugMsgDialog::initFrame),
    TDialog(aR,tit)
{
 flags = wfMove | wfGrow | wfZoom | wfClose;
 growMode = gfGrowLoY | gfGrowHiX | gfGrowHiY;

 TRect r=getExtent();
 r.grow(-1,-1);
 r.b.y-=3;
 r.b.x-=1;
 unsigned width=r.b.x-r.a.x;

 // Vertical scrollbar
 TRect rs=r;
 rs=TRect(r.b.x,r.a.y,r.b.x+1,r.b.y);
 TScrollBar *scrBVe=new TScrollBar(rs);
 insert(scrBVe);
 scrBVe->options|=ofPostProcess;

 // Horizontal scrollbar
 rs=r;
 rs=TRect(r.a.x,r.b.y,r.b.x,r.b.y+1);
 TScrollBar *scrBHz=new TScrollBar(rs);
 insert(scrBHz);
 scrBHz->options|=ofPostProcess;
 scrBHz->setParams(0,0,1000-width,40,1);

 // List of messages
 MsgList=new TListBox(r,1,scrBHz,scrBVe);
 MsgList->growMode=gfGrowHiX | gfGrowHiY;
 insert(MsgList);

 // Two status lines
 AllocLocalStr(empty,width+1);
 memset(empty,' ',width);
 empty[width]=0;
 r.b.x++;
 rs.move(0,1);
 status1=new TNoStaticText(rs,empty);
 status1->growMode=gfGrowLoY | gfGrowHiX | gfGrowHiY;
 insert(status1);
 rs.move(0,1);
 status2=new TNoStaticText(rs,empty);
 status2->growMode=gfGrowLoY | gfGrowHiX | gfGrowHiY;
 insert(status2);

 // The debug status
 rs.move(0,1);
 rs.a.x+=2;
 rs.b.x=rs.a.x+maxWDbgSt;
 empty[maxWDbgSt]=0;
 statusF=new TNoStaticText(rs,empty);
 statusF->growMode=gfGrowLoY | gfGrowHiY;
 insert(statusF);

 helpCtx=hcDebugMsgWin;
}

void TDebugMsgDialog::changeBounds(const TRect &r)
{
 TDialog::changeBounds(r);
 MsgWindowRect=r;
}

void TDebugMsgDialog::close(void)
{
 hide();
}

void TDebugMsgDialog::handleEvent(TEvent &event)
{
 TDialog::handleEvent(event);
 if (event.what==evCommand)
    switch (event.message.command)
      {
       case cmGDBCommand:
            SendGDBCommand();
            clearEvent(event);
            break;
      }
}

static
TDialog *createCmdDlg()
{
 TSViewCol *col=new TSViewCol(__("Send command to gdb"));

 // EN: C
 TSLabel *o1=new TSLabel(__("~C~ommand, be careful!"),
                         new TSInputLine(widthGDBCom,maxGDBComBox));

 col->insert(xTSLeft,yTSUp,o1);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(cmGDBCommand);
 delete col;
 return d;
}

void TDebugMsgDialog::SendGDBCommand()
{
 if (!dbg)
    return;
 char command[widthGDBCom];
 command[0]=0;
 if (execDialog(createCmdDlg(),command)==cmOK)
   {
    DBG_GenericCB(command,(void *)"U");
    dbg->Send(command);
   }
}

TDebugMsgDialog *DebugMsgInit(Boolean hide, int ZOrder)
{
 if (MsgWindow)
    return MsgWindow;
 // Create the list:
 if (!MsgCol)
    MsgCol=new TMsgCollection(8,6);
 // Create the dialog:
 if (MsgWindowRect.a.x==-1)
   {
    MsgWindowRect=TProgram::deskTop->getExtent();
    MsgWindowRect.a.y=MsgWindowRect.b.y-10;
   }
 MsgWindow=new TDebugMsgDialog(MsgWindowRect,__("Debugger Window"));
 MsgList->newList(MsgCol);

 // Insert in the helper
 TDskWin *dskWin=SearchInHelperWin(dktDbgMsg,MsgWindow);
 if (!dskWin)
   {
    TDskDbgMsg *p=new TDskDbgMsg(MsgWindow);
    AddNonEditorToHelper(p);
    TSetEditorApp::setCmdState(cmeSelDebugWin,True);
    dskWin=p;
   }

 dskWin->ZOrder=ZOrder;
 TProgram::deskTop->lock();
 InsertInOrder(TProgram::deskTop,dskWin);
 if (hide)
    MsgWindow->hide();
 TProgram::deskTop->unlock();

 return MsgWindow;
}

void DebugMsgDeInit()
{
 if (MsgWindow)
   {
    MsgWindow->realClose();
    MsgWindow=NULL;
   }
 destroy0(MsgCol);
 MsgList=NULL;
}

void DebugMsgClose()
{
 if (MsgWindow)
    MsgWindow->close();
}

void DebugMsgAdd(char *msg)
{
 DebugMsgInit();
 MsgCol->insert(msg);
 DebugMsgUpdate(edsmDontSelect);
}

static
void ResetHz()
{
 if (MsgList->hScrollBar)
    MsgList->hScrollBar->setValue(0);
}

void DebugMsgUpdate(unsigned options)
{
 TProgram::deskTop->lock();
 if (!(options & edsmNoHzReset))
    ResetHz();
 MsgWindow->show();
 if (!(options & edsmDontSelect))
    MsgWindow->select();
 MsgList->setRange(MsgCol->getCount());
 // Scroll it
 uint32 opsScr=options & edsmScrollMask;
 if (opsScr==edsmEverScroll ||
     (opsScr==edsmScrollIfNoFocus && (MsgWindow->state & sfActive)==0))
   {
    if (MsgCol->getCount())
       MsgList->focusItem(MsgCol->getCount()-1);
   }
 MsgList->drawView();
 TProgram::deskTop->unlock();
}

char *DebugMsgStateName()
{
 char *msg=NULL;
 if (dbg)
   {
    switch (dbg->GetState())
      {
       case MIDebugger::disconnected:
            msg=__("Disconnected");
            break;
       case MIDebugger::connected:
            msg=__("Connected");
            break;
       case MIDebugger::target_specified:
            msg=__("Ready to Run");
            break;
       case MIDebugger::running:
            msg=__("Running");
            break;
       case MIDebugger::stopped:
            msg=__("Stopped");
            break;
      }
   }
 return msg;
}

/**[txh]********************************************************************

  Description:
  Updates the gdb status in the message window.
  
***************************************************************************/

void DebugMsgSetState()
{
 // If we changed the state then the commands must be updated.
 TSetEditorApp::DebugUpdateCommands();
 DebugMsgInit();
 char *msg=DebugMsgStateName();
 if (!msg)
   msg=__("Unknown");
 // Execute associated actions
 int cleanStop=1;
 switch (dbg->GetState())
   {
    case MIDebugger::disconnected:
         break;
    case MIDebugger::connected:
         break;
    case MIDebugger::target_specified:
         DebugClearCPULine();
         cleanStop=0;
         break;
    case MIDebugger::running:
         DebugClearCPULine();
         break;
    case MIDebugger::stopped:
         DebugUpdateWatches();
         cleanStop=0;
         break;
   }
 MsgWindow->SetStatusGDB(msg);
 if (cleanStop)
    MsgWindow->SetStatusStop("");
 DebugMsgUpdate(edsmDontSelect);
}

void DebugMsgSetError()
{
 DebugMsgInit();

 char b[maxWStatus];

 // TODO: i18n
 const char *cErr=MIDebugger::GetErrorStr();
 int iErr=MIDebugger::GetErrorNumber();

 int l=CLY_snprintf(b,maxWStatus,__("Error: %s (%d)"),cErr,iErr);

 if (iErr==MI_FROM_GDB)
    CLY_snprintf(b+l,maxWStatus-l," [%s]",MIDebugger::GetGDBError());

 MsgWindow->SetStatusError(b);
 DebugMsgUpdate();

 if (killAfterStop)
    // Something went wrong, forget about it.
    killAfterStop=0;
}

void DebugMsgSetMode(Boolean select)
{
 DebugMsgInit();

 // TODO: i18n
 char b[maxWStatus];
 const char *tty;
 b[0]=0;

 switch (dOps.mode)
   {
    case dmLocal:
         tty=NULL;
         if (dOps.tty[0])
            tty=dOps.tty;
         if (!tty)
            tty=dbg->GetAuxTTY();
         if (!tty)
            tty=__("default");
         CLY_snprintf(b,maxWStatus,__("Mode: %s (%s) [%s]"),
                      localMode==modeX11 ? "X11" : "Console",tty,dOps.program);
         break;
    case dmPID:
         break;
    case dmRemote:
         CLY_snprintf(b,maxWStatus,__("Mode: Remote (%s) [%s]"),
                      dOps.rparam,dOps.program);
         break;
   }
 MsgWindow->SetStatusMode(b);
 DebugMsgUpdate(select ? 0 : edsmDontSelect);
}

int DebugMsgJumpToFrame(mi_frames *f, char *msg, int l)
{
 // Jump to the line where we stopped
 int jumped=0;
 if (f)
   {// Try to jump to the source line.
    if (f->file)
      {
       char file[PATH_MAX];
       if (binReference)
          strcpy(file,binReference);
       else
          file[0]=0;
       strcat(file,f->file);
       dbgPr("Source relative to binary: %s\n",file);
       if (GotoFileLine(f->line,file,msg,-1,l,False))
          jumped=1;
      }
   }
 return jumped;
}

int DebugMsgFillReason(mi_frames *f, char *b, Boolean stop)
{
 int l;

 // TODO: i18n
 if (stop)
    l=CLY_snprintf(b,maxWStatus,__("Reason stopped: %s"),
                   mi_reason_enum_to_str(stoppedInfo->reason));
 else
    l=CLY_snprintf(b,maxWStatus,__("Returned immediatly"));

 if (f && l+10<maxWStatus)
   {
    l+=CLY_snprintf(b+l,maxWStatus-l," [%s:%s:%d]",
                    f->func ? f->func : __("unknown"),
                    f->file ? f->file : __("unknown"),
                    f->line);
   }
 MsgWindow->SetStatusStop(b);

 return l;
}

void DebugMsgSetStopped()
{
 if (!stoppedInfo)
    return;

 char b[maxWStatus];
 int l=DebugMsgFillReason(stoppedInfo->frame,b,True);

 // If we stopped we changed the state
 DebugMsgSetState();
 // Jump to the line where we stopped
 int jumped=DebugMsgJumpToFrame(stoppedInfo->frame,b,l);
 // Update the window
 DebugMsgUpdate(jumped ? edsmDontSelect : 0);

 if (killAfterStop)
   {
    killAfterStop=0;
    TSetEditorApp::DebugKill();
   }
}

/*****************************************************************************
  End of Debug Status and Messages Window
*****************************************************************************/

/*****************************************************************************

  Watches Window

  "~W~atch an expression"

  I first implemented the "variable objects" mechanism provided by gdb
(according to docs that's what Cygnus' Insight uses). But the way it works is
confusing, specially for RHIDE users. So I added watches that works like the
ones found in RHIDE and they are the "normal".
  As the "v.o." mechanism takes care of the scope I named them "with scope".
  For long lists the "with scope" watches should be faster because we know
which ones changed and we don't evaluate all.

/------------------------------------------------------------------------------\
|00: exp value                                                                 ^
|01: exp value                                                                 |
|                                                                              |
|                                                                              V
\<---------------------------------------------------------------------------->/

*****************************************************************************/

struct gVar
{
 char *exp;
 char type;
 mi_gvar *v;
 char *value;
};

const char gvtWScope=1, gvtNormal=0;

class TGVarCollection : public TCollection, public TStringable
{
public:
 TGVarCollection() :
   TCollection(8,4),
   TStringable() {}
 virtual unsigned GetCount() { return getCount(); };
 virtual void getText(char *dest, unsigned item, int maxLen);
 virtual void freeItem(void *item);
 int Insert(char *s, Boolean wScope);
 int DeleteVar(ccIndex pos, Boolean removeIt=True);
 void Refresh(ccIndex pos);
 gVar *At(ccIndex pos) { return (gVar *)at(pos); };

 static void MsgNotWhileRunning();

 SetDefStreamMembers(TGVarCollection,TCollection);
};

SetDefStreamOperators(TGVarCollection);
TStreamableClass RGVarCollection(TGVarCollection::name,TGVarCollection::build,
                                 __DELTA(TGVarCollection));
const char * const TGVarCollection::name="TGVarCollection";

static TGVarCollection *WtCol=NULL;


void *TGVarCollection::readItem(ipstream &is)
{
 gVar *gv=new gVar;
 memset(gv,0,sizeof(gVar));
 gv->exp=is.readString();
 is >> gv->type;

 return gv;
}

void TGVarCollection::writeItem(void *obj, opstream &os)
{
 gVar *gv=(gVar *)obj;
 os.writeString(gv->exp);
 os << gv->type;
}

void TGVarCollection::freeItem(void *item)
{
 gVar *gv=(gVar *)item;
 delete[] gv->exp;
 mi_free_gvar(gv->v);
 ::free(gv->value);
 delete gv;
}

void TGVarCollection::getText(char *dest, unsigned item, int maxLen)
{
 gVar *gv=At(item);
 if (gv->v)
    CLY_snprintf(dest,maxLen,"%s: [%s] %s",gv->exp,gv->v->type,gv->value);
 else
   {
    if (gv->value)
       CLY_snprintf(dest,maxLen,"%s: %s",gv->exp,gv->value);
    else
       // TODO: i18n
       CLY_snprintf(dest,maxLen,__("%s: Not yet in debugger"),gv->exp);
   }
}

int TGVarCollection::Insert(char *s, Boolean wScope)
{
 gVar *gv=new gVar;
 gv->type=wScope ? gvtWScope : gvtNormal;
 gv->exp=newStr(s);
 gv->v=NULL;
 gv->value=NULL;
 ccIndex pos=insert((void *)gv);
 Refresh(pos);

 return 1;
}

void TGVarCollection::MsgNotWhileRunning()
{
 messageBox(__("Wait until the process is stopped to delete this watch"),
            mfError | mfOKButton);
}

int TGVarCollection::DeleteVar(ccIndex pos, Boolean removeIt)
{
 gVar *gv=At(pos);

 if (gv->v)
   {// GDB knows about it
    if (dbg)
      {
       MIDebugger::eState st=dbg->GetState();
       if (st==MIDebugger::running)
         {// Not async op :-(
          MsgNotWhileRunning();
          return 0;
         }
       if (st==MIDebugger::stopped || st==MIDebugger::target_specified)
         {// Must delete from gdb
          if (!dbg->DelgVar(gv->v))
            {
             messageBox(__("Error deleting variable"),mfError | mfOKButton);
             return 0;
            }
         }
      }
    // Just to be sure it won't be reused. freeItem releases it.
    mi_free_gvar(gv->v);
    gv->v=NULL;
   }
 if (removeIt)
    atFree(pos);
 else
   {
    delete[] gv->exp;
    ::free(gv->value);
    gv->exp=gv->value=NULL;
   }

 return 1;
}

void TGVarCollection::Refresh(ccIndex pos)
{
 if (!dbg || dbg->GetState()!=MIDebugger::stopped)
    return;
 // Ok, we can update it
 gVar *gv=At(pos);
 if (gv->type==gvtNormal)
   {// Normal ones, they are just evaluated
    ::free(gv->value);
    gv->value=dbg->EvalExpression(gv->exp);
    return;
   }
 // With scope, they use the "variable objects" mechanism
 if (!gv->v)
   {// Not yet defined for gdb
    gv->v=dbg->AddgVar(gv->exp);
    if (!gv->v) // Most probably a wrong expression or just out of scope
      {
       ::free(gv->value);
       gv->value=strdup(dbg->GetGDBError());
      }
   }
 if (gv->v)
   {
    ::free(gv->value);
    gv->value=dbg->EvalExpression(gv->exp);
   }
}

class TWatchesDialog : public TDialog
{
public:
 TWatchesDialog(const TRect &aR, const char *tit);
 virtual void changeBounds(const TRect &r);
 virtual void close(void);
 virtual void handleEvent(TEvent &event);

 int AddWatch(Boolean wScope);
 int DeleteWatch();
 int EditWatch();
 int EditExpression(char *exp, char *tit);
 int InsertExp(char *exp, Boolean wScope);
 void realClose() { TDialog::close(); };
};

/*****************************************************************************
  Dsk Wrapper for Watches Window
*****************************************************************************/

class TDskDbgWt : public TDskWin
{
public:
 TDskDbgWt(TView *w);
 ~TDskDbgWt();

 //int  GoAction(ccIndex i);
 int  DeleteAction(ccIndex i, Boolean fromDiskToo=False);
 char *GetText(char *dest, short maxLen);
 int  Compare(void *p,int t) { return (t==dktDbgMsg); };

 void write(opstream& os);
 void *read(ipstream& is);

 const char *streamableName() const
     { return name; }

protected:
 TDskDbgWt(StreamableInit) { type=dktDbgMsg; CanBeSaved=0; CanBeDeletedFromDisk=0; };

public:
 static const char * const name;
 static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TDskDbgWt& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TDskDbgWt*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TDskDbgWt& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TDskDbgWt* cl )
    { return os << (TStreamable *)cl; }

TDskDbgWt::TDskDbgWt(TView *w)
{
 view=w;
 type=dktDbgWt;
 CanBeDeletedFromDisk=0;
 CanBeSaved=0;
 ZOrder=-1;
 //wS=TScreen::getCols();
 //hS=TScreen::getRows();
}

TDskDbgWt::~TDskDbgWt()
{
 dbgPr("~TDskDbgWt\n");
}

int TDskDbgWt::DeleteAction(ccIndex, Boolean)
{
 view->hide();
 return 0;
}

char *TDskDbgWt::GetText(char *dest, short maxLen)
{
 int watches=0;
 if (WtCol)
    watches=WtCol->getCount();
 TVIntl::snprintf(dest,maxLen,__("   Watches Window (%d watches)"),watches);
 return dest;
}

const char * const TDskDbgWt::name="TDskDbgWt";

TStreamable *TDskDbgWt::build()
{
 return new TDskDbgWt(streamableInit);
}

void TDskDbgWt::write(opstream &)
{
}

void *TDskDbgWt::read(ipstream &)
{
 return this;
}

/*****************************************************************************
  End of Dsk Wrapper for Watches Window
*****************************************************************************/

// TODO: enclose in a class
static TWatchesDialog     *WtWindow=NULL;
static TStringableListBox *WtList  =NULL;
static TRect WtWindowRect(-1,-1,-1,-1);

TWatchesDialog::TWatchesDialog(const TRect &aR, const char *tit) :
    TWindowInit(TWatchesDialog::initFrame),
    TDialog(aR,tit)
{
 TScrollBar *scrollbar;

 flags = wfMove | wfGrow | wfZoom | wfClose;
 growMode = gfGrowLoY | gfGrowHiX | gfGrowHiY;

 TRect r=getExtent();
 r.grow(-1,-1);
 scrollbar=standardScrollBar(sbVertical | sbHandleKeyboard);

 WtList=new TStringableListBox(r,1,scrollbar);
 WtList->growMode=gfGrowHiX | gfGrowHiY;

 scrollbar=standardScrollBar(sbHorizontal | sbHandleKeyboard);
 scrollbar->setParams(0,0,1000-(r.b.x-r.a.x),40,1);

 WtList->hScrollBar=scrollbar;
 insert(WtList);

 helpCtx=hcWatchesWin;
}

void TWatchesDialog::changeBounds(const TRect &r)
{
 TDialog::changeBounds(r);
 WtWindowRect=r;
}

void TWatchesDialog::close(void)
{
 hide();
}

int TWatchesDialog::InsertExp(char *exp, Boolean wScope)
{
 int res=WtCol->Insert(exp,wScope);
 if (res)
   {
    WtList->setRange(WtCol->getCount());
    WtList->drawView();
   }
 return res;
}

TDialog *createEditExp(char *tit)
{
 TSViewCol *col=new TSViewCol(tit);

 // EN: E
 TSLabel *o1=new TSLabel(__("~E~xpression"),
                         new TSInputLine(widthWtExp,maxWtBox));

 col->insert(xTSLeft,yTSUp,o1);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(hcWatchesWinEdit);
 delete col;
 return d;
}

int TWatchesDialog::EditExpression(char *exp, char *tit)
{
 if (execDialog(createEditExp(tit),exp)==cmOK)
    return 1;
 return 0;
}

int TWatchesDialog::AddWatch(Boolean wScope)
{
 char exp[widthWtExp];
 exp[0]=0;
 if (!EditExpression(exp,__("Add to watch list")))
    return 0;
 InsertExp(exp,wScope);
 return 1;
}

int TWatchesDialog::DeleteWatch()
{
 if (!WtCol || WtCol->getCount()==0)
    return 0;
 if (WtCol->DeleteVar(WtList->focused))
   {
    WtList->setRange(WtCol->getCount());
    WtList->drawView();
    return 1;
   }
 return 0;
}

int TWatchesDialog::EditWatch()
{
 if (!WtCol || WtCol->getCount()==0)
    return 0;
 if (dbg && dbg->GetState()==MIDebugger::running)
   {
    TGVarCollection::MsgNotWhileRunning();
    return 0;
   }

 ccIndex pos=WtList->focused;
 gVar *gv=WtCol->At(pos);

 char exp[widthWtExp];
 strcpy(exp,gv->exp);
 if (!EditExpression(exp,__("Change the expression to watch")))
    return 0;

 if (!WtCol->DeleteVar(pos,False))
    return 0;

 gv->exp=newStr(exp);
 WtCol->Refresh(pos);
 WtList->drawView();
 return 1;
}

void TWatchesDialog::handleEvent(TEvent &event)
{
 TDialog::handleEvent(event);
 if (event.what==evCommand)
    switch (event.message.command)
      {
       case cmInsert:
            AddWatch(False);
            clearEvent(event);
            break;
       case cmInsertWScope:
            AddWatch(True);
            clearEvent(event);
            break;
       case cmDelete:
            DeleteWatch();
            clearEvent(event);
            break;
       case cmEditWatch:
            EditWatch();
            clearEvent(event);
            break;
      }
}

TWatchesDialog *WatchesInit(Boolean hide, int ZOrder)
{
 if (WtWindow)
    return WtWindow;
 // Create the list:
 if (!WtCol)
    WtCol=new TGVarCollection();
 // Create the dialog:
 if (WtWindowRect.a.x==-1)
   {
    WtWindowRect=TProgram::deskTop->getExtent();
    WtWindowRect.a.y=WtWindowRect.b.y-7;
   }
 WtWindow=new TWatchesDialog(WtWindowRect,__("Watches"));
 WtList->newList(WtCol);

 // Insert in the helper
 TDskWin *dskWin=SearchInHelperWin(dktDbgWt,WtWindow);
 if (!dskWin)
   {
    TDskDbgWt *p=new TDskDbgWt(WtWindow);
    AddNonEditorToHelper(p);
    TSetEditorApp::setCmdState(cmeSelWatchesWin,True);
    dskWin=p;
   }

 dskWin->ZOrder=ZOrder;
 TProgram::deskTop->lock();
 InsertInOrder(TProgram::deskTop,dskWin);
 if (hide)
    WtWindow->hide();
 TProgram::deskTop->unlock();

 return WtWindow;
}

void WatchesDeInit()
{
 if (WtWindow)
   {
    WtWindow->realClose();
    WtWindow=NULL;
   }
 destroy0(WtCol);
 WtList=NULL;
}

void WatchesClose()
{
 if (WtWindow)
    WtWindow->close();
}

void TSetEditorApp::DebugWatchExp(Boolean wScope)
{
 WatchesInit();
 WtWindow->AddWatch(wScope);
}


/**[txh]********************************************************************

  Description:
  Updates the watches. That's called when we switch to stopped.
  
***************************************************************************/

void DebugUpdateWatches()
{
 if (!WtWindow || !WtCol || !WtList)
    return;
 ccIndex c=WtCol->getCount(), i;
 if (!c)
    return;
 int changedVars=0, deletedVars=0;
 // First pass: try to define pending vars and evaluate normal ones.
 for (i=0; i<c; i++)
    {
     gVar *gv=WtCol->At(i);
     if (gv->type==gvtNormal || !gv->v)
       {
        WtCol->Refresh(i);
        changedVars++;
       }
    }
 // Now find which variables changed
 mi_gvar_chg *changed;
 if (!dbg->ListChangedgVar(changed))
    return;

 mi_gvar_chg *ch;

 if (0)
   {// Debug code
    ch=changed;
    static int conta=0;
    dbgPr("%d) list of changed variables:\n",conta);
    conta++;
    while (ch)
      {
       dbgPr("name: %s, in scope: %c ",ch->name,ch->in_scope ? 'y' : 'n');
       if (ch->in_scope && ch->new_type)
         {
          dbgPr("new type: %s new num children: %d ",ch->new_type,ch->new_num_children);
         }
       ch=ch->next;
       dbgPr("\n");
      }
   }

 ch=changed;
 while (ch)
   {
    for (i=0; i<c; i++)
       {
        gVar *gv=WtCol->At(i);
        if (gv->v && strcmp(gv->v->name,ch->name)==0)
          {
           if (!ch->in_scope)
             {// Ok, no longer exists
              if (WtCol->DeleteVar(i))
                {
                 c=WtCol->getCount();
                 deletedVars++;
                }
             }
           else
             {
              if (ch->new_type)
                {// I know it from gdb sources but I don't know how can it happend.
                 ::free(gv->v->type);
                 gv->v->type=ch->new_type;
                 ch->new_type=NULL;
                }
              WtCol->Refresh(i);
              changedVars++;
             }
           break;
          }
       }
    ch=ch->next;
   }
 mi_free_gvar_chg(changed);

 if (changedVars || deletedVars)
   {
    if (deletedVars)
       WtList->setRange(c);
    WtList->drawView();
   }
}

/*****************************************************************************
  End of Watches Window
*****************************************************************************/

/*****************************************************************************
  Configuration dialog
*****************************************************************************/

TDialog *createDebugOpsDialog()
{
 TSViewCol *col=new TSViewCol(__("Debug Options"));

 // EN: ADELMPRTU
 // ES: ADEIMPRTU
 TSVeGroup *o1=
 MakeVeGroup(0, // All together
             new TSLabel(__("~P~rogram to debug, with debug info"),
                 new TSInputLine(widthFiles,maxWBox)),
             TSLabelRadio(__("~M~ode"),
                          __("Local with ~e~xecutable"),
                          __("Local with r~u~nning process"),
                          __("Remote (g~d~bserver/stub)"),0),
             new TSStaticText(__("Local target options")),
             new TSLabel(__("Program ~a~rguments, not for remote mode"),
                 new TSInputLine(widthFiles,maxWBox)),
             new TSLabel(__("Forced ~t~erminal, leave blank for auto"),
                 new TSInputLine(widthShort,maxWBox)),
             new TSStaticText(__("Remote target options")),
             new TSLabel(__("Remote p~r~otocol type"),
                 new TSInputLine(widthShort,maxWBox)),
             new TSLabel(__("Remote ~l~ocation"),
                 new TSInputLine(widthShort,maxWBox)),
             0);
 o1->makeSameW();

 col->insert(xTSLeft,yTSUp,o1);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(cmeDebugOptions);
 delete col;
 return d;
}

int TSetEditorApp::DebugOptionsEdit()
{
 if (debugOpsNotIndicated)
   {// Fill the structure with examples and recommendations
    debugOpsNotIndicated=0;
    // TODO: i18n
    strcpy(dOps.program,__("Your program compiled with -g"));
    dOps.mode=dmLocal;
    dOps.args[0]=0;
    dOps.tty[0]=0;
    strcpy(dOps.rtype,"extended-remote");
    strcpy(dOps.rparam,"192.168.1.2:5000");
   }

 DebugOptionsStruct edit;
 memcpy(&edit,&dOps,sizeof(DebugOptionsStruct));

 if (execDialog(createDebugOpsDialog(),&edit)==cmOK)
   {
    memcpy(&dOps,&edit,sizeof(DebugOptionsStruct));
    return 1;
   }
 return 0;
}

TDialog *createDebugOptsMsgsDialog()
{
 TSViewCol *col=new TSViewCol(__("Messages"));

 // EN: ACDFLT
 // ES: CDELRT
 TSLabel *o1=TSLabelCheck(__("~D~isplay messages related to"),
                          __("GDB ~c~onsole (CLI)"),
                          __("T~a~rget (not really implemented in gdb)"),
                          __("~L~og (gdb internal messages)"),
                          __("GDB/MI commands sent ~t~o gdb"),
                          __("GDB/MI responses ~f~rom gdb"),0);

 col->insert(xTSLeft,yTSUp,o1);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(cmeDebugOptions);
 delete col;
 return d;
}

void TSetEditorApp::DebugOptsMsgs()
{
 uint32 edit=msgOps;

 if (execDialog(createDebugOptsMsgsDialog(),&edit)==cmOK)
   {
    msgOps=edit;
    DBG_SetCallBacks();
   }
}

/*****************************************************************************
  End of Configuration dialog
*****************************************************************************/

/*****************************************************************************
  Persistence.
  This information is stored only in project files, to do "persistent"
debugging the user needs a project, for "casual" use the project isn't needed.
*****************************************************************************/

// Allow -2*size to 2*size
const unsigned maxRef=1000000000;

static
int32 ExpandCoor(int coord, unsigned  ref)
{// Relative
 double v=(double)coord/ref;
 v*=maxRef;
 // Round
 if (v>=0.0)
    v+=0.5;
 else
    v-=0.5;
 // Clamp
 if (v>INT_MAX)
    v=INT_MAX;
 else if (v<INT_MIN)
    v=INT_MIN;
 return (int32)v;
}

static
void SaveExpandedRect(opstream &os, TRect &r, unsigned wS, unsigned hS)
{
 os << ExpandCoor(r.a.x,wS) << ExpandCoor(r.b.x,wS)
    << ExpandCoor(r.a.y,hS) << ExpandCoor(r.b.y,hS);
}

static
int CompactCoord(int32 coord, unsigned  ref)
{
 double v=(double)coord/maxRef;
 v*=ref;
 // Round
 if (v>=0.0)
    v+=0.5;
 else
    v-=0.5;
 return (int)v;
}

static
void ReadExpandedRect(ipstream &is, TRect &r, unsigned wS, unsigned hS)
{
 int32 a, b;
 is >> a >> b;
 r.a.x=CompactCoord(a,wS);
 r.b.x=CompactCoord(b,wS);
 is >> a >> b;
 r.a.y=CompactCoord(a,hS);
 r.b.y=CompactCoord(b,hS);
}

void DebugSaveData(opstream &os)
{
 unsigned wS=TScreen::getCols();
 unsigned hS=TScreen::getRows();

 // CAUTION!!! update the dummy
 os << debugDataVersion;

 // Target options
 os << (uchar)debugOpsNotIndicated;
 if (!debugOpsNotIndicated)
   {
    dbgPr("debugOpsNotIndicated\n");
    os.writeString(dOps.program);
    os.writeString(dOps.args);
    os.writeString(dOps.tty);
    os.writeString(dOps.rtype);
    os.writeString(dOps.rparam);
    os << (uchar)dOps.mode;
   }
 // Type of Messages
 os << (uchar)msgOps;
 // Debug window information
 if (MsgWindowRect.a.x!=-1)
   {
    dbgPr("MsgWindowRect\n");
    os << svPresent;
    SaveExpandedRect(os,MsgWindowRect,wS,hS);
    if (MsgWindow)
      {
       dbgPr("MsgWindow\n");
       os << svPresent << (int)(TProgram::deskTop->indexOf(MsgWindow))
          << (char)((MsgWindow->state & sfVisible) ? svYes : svNo);
      }
    else
       os << svAbsent;
   }
 else
    os << svAbsent;
 // Watches
 if (WtCol && WtCol->getCount())
   {
    dbgPr("WtCol\n");
    os << svPresent << WtCol;
   }
 else
    os << svAbsent;
 // Watches window information
 if (WtWindowRect.a.x!=-1)
   {
    dbgPr("WtWindowRect\n");
    os << svPresent;
    SaveExpandedRect(os,WtWindowRect,wS,hS);
    if (WtWindow)
      {
       dbgPr("WtWindow\n");
       os << svPresent << (int)(TProgram::deskTop->indexOf(WtWindow))
          << (char)((WtWindow->state & sfVisible) ? svYes : svNo);
      }
    else
       os << svAbsent;
   }
 else
    os << svAbsent;
 // Breakpoints
 DBG_SaveBreakpoints(os);
 // No more data
 os << svAbsent;
}

void DebugReadData(ipstream &is)
{
 unsigned char version, aux;
 unsigned wS=TScreen::getCols();
 unsigned hS=TScreen::getRows();

 is >> version;
 // Target options
 is >> aux;
 debugOpsNotIndicated=aux;
 if (!debugOpsNotIndicated)
   {
    dbgPr("debugOpsNotIndicated\n");
    is.readString(dOps.program,sizeof(dOps.program));
    is.readString(dOps.args,sizeof(dOps.args));
    is.readString(dOps.tty,sizeof(dOps.tty));
    is.readString(dOps.rtype,sizeof(dOps.rtype));
    is.readString(dOps.rparam,sizeof(dOps.rparam));
    is >> aux;
    dOps.mode=aux;
    ExtractBinReference();
   }
 // Type of Messages
 is >> aux;
 msgOps=aux;
 // Debug window information
 is >> aux;
 if (aux)
   {
    dbgPr("MsgWindowRect\n");
    ReadExpandedRect(is,MsgWindowRect,wS,hS);
    is >> aux;
    if (aux)
      {
       dbgPr("MsgWindow\n");
       int ZOrder;
       char visible;
       is >> ZOrder >> visible;
       DebugMsgInit(visible ? False : True,ZOrder);
      }
   }
 // Watches
 is >> aux;
 if (aux)
   {
    dbgPr("WtCol\n");
    is >> WtCol;
   }
 // Watches window information
 is >> aux;
 if (aux)
   {
    dbgPr("WtWindowRect\n");
    ReadExpandedRect(is,WtWindowRect,wS,hS);
    is >> aux;
    if (aux)
      {
       dbgPr("WtWindow\n");
       int ZOrder;
       char visible;
       is >> ZOrder >> visible;
       WatchesInit(visible ? False : True,ZOrder);
      }
   }
 // Breakpoints
 DBG_ReadBreakpoints(is);
 // No more data
 is >> aux;
}

/*****************************************************************************
  End of Persistence.
*****************************************************************************/
#else // !HAVE_GDB_MI

void TSetEditorApp::DebugCommandsForDisc() {}
int  TSetEditorApp::DebugConfirmEndSession(Boolean ) { return 1; }
void TSetEditorApp::DebugToggleBreakpoint() {}
int  TSetEditorApp::DebugOptionsEdit() { return 0; }
void TSetEditorApp::DebugRunOrContinue() {}
void TSetEditorApp::DebugStepOver() {}
void TSetEditorApp::DebugTraceInto() {}
void TSetEditorApp::DebugGoToCursor() {}
void TSetEditorApp::DebugFinishFun() {}
void TSetEditorApp::DebugReturnNow() {}
void TSetEditorApp::DebugStop() {}
void TSetEditorApp::DebugKill() {}
void TSetEditorApp::DebugCallStack() {}
void TSetEditorApp::DebugEvalModify(char *startVal) { delete[] startVal; }
void TSetEditorApp::DebugOptsMsgs() {}
void TSetEditorApp::DebugWatchExp(Boolean ) {}
void TSetEditorApp::DebugDeInitVars() {}
void TSetEditorApp::DebugCloseSession() {}
int  TSetEditorApp::DebugCheckAcceptCmd(Boolean ) { return 0; }
int  TSetEditorApp::DebugCheckStopped(Boolean ) { return 1; }
void DebugSetCPULine(int , char *) {}
void TSetEditorApp::DebugPoll() {}
void DebugReadData(ipstream &) {}

void DBG_SaveBreakpoints(opstream &os)
{
 int32 count=0;
 // Save them
 os << bkptsVersion << count;
}

void DebugSaveData(opstream &os)
{
 os << debugDataVersion;
 // Target options
 os << (uchar)1;
 // Type of Messages
 os << (uchar)msgOps;
 // Debug window information
 os << svAbsent;
 // Watches
 os << svAbsent;
 // Watches window information
 os << svAbsent;
 // Breakpoints
 DBG_SaveBreakpoints(os);
 // No more data
 os << svAbsent;
}

#endif // HAVE_GDB_MI

