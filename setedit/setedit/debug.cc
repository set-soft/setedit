/* Copyright (C) 2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Module: Debugger interface.
  Comments:
  It implements the interface with the debugger.

  TODO:
  The most important unimplemented features and unsolved things are:

  * Join the watchpoint dialog with the word under cursor.

  * When debugging remote programs gdb exposes a nasty bug: When I do next
at the end of main it replies "running", but then it sends a couple of
errors to the log (because gdb can't set a temporal breakpoint) and then
generates an error result record. But it doesn't generate an stopped info.
So we thing the program is running (gdb told it) but it isn't true. What's
worst is what happends when we send SIGINT ... finally we hang at exit waiting
for replies.
  A workaround for a similar was implemented, must be tested on this scenario.

  * GDB meaning of the ignore field is quite different to what I thinked. That's
a one shot option, you say "ignore 2" and then the next 2 passes are ignored.
After it the breakpoint becomes "normal". It means that after a program reset it
won't ignore is the "times" reached the ignore count in the previous pass. So
may be we should "refresh" the ignores before running.

  * When a new item is added to the project try to add its path to the
  sources path.

  * Fix: We are saving the "TRect" for msg and watches, but not the "zoom
  state" and the "unzoomed" size. That's annoying.

  * Add some mechanism to enable MI v2. That's much better.

  * Advices (only project, ...).
  * Watchpoints.
  * Some mechanism to verify that a location for a breakpoint is OK. GDB
says ok to everything. -symbol-list-lines could help, but that's only for
GDB 6.x. Note: GDB accepts breakpoints everywhere and puts the breakpoint
somewhere close to the indicated point, may be we just need to find where
is the real breakpoint.
  * Inspect variables.
  * Data window.
  * Disassembler window.
  * Put a limit to the ammount of messages in the debug message window.
Reset it when reseting the session.
  * Allow to configure gdb and XTerm program.
  * Setup the "main" function, currently we have a "run to main", but this
is C/C++ specific.
  * Add the menubind.smn options to the redmond.smn
  * Don't show again in the confirmation for exit while debugging.
  * Detect recompilations (target time stamp), then ask to the user if we
should "move" the breakpoints to their new locations. That's tricky.
Note: currently we do it if the user used "run program".
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

// Breakpoints
#define Uses_TSStringableListBox
#define Uses_TSCheckBoxes
#define Uses_TSRadioButtons

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

#define Uses_TDialogAID

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
#ifdef HAVE_GDB_MI
 #include <mi_gdb.h>
 #define Uses_TBreakpoints
 #define Uses_TWatchpoints
#endif
#include <debug.h>
#include <dyncat.h>
#include <edhists.h>
#include <dskwin.h>
#include <pathlist.h>
#include <diaghelp.h>
#include <pathtool.h> // FindFile

const uint32 msgConsole=1, msgTarget=2, msgLog=4, msgTo=8, msgFrom=16;
static uint32 msgOps=msgConsole | msgTarget | msgLog;
const char svPresent=1, svAbsent=0, svYes=1, svNo=0;
const char bkptsVersion=2;
const char wptsVersion=1;
const char debugDataVersion=1;

#ifdef HAVE_GDB_MI

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
// Modification time
static time_t binMTime;

// Some common messages
static stTVIntl *icUnknown=NULL,
                *icNotStarted=NULL,
                *icDisconnected=NULL,
                *icConnected=NULL,
                *icReadyToRun=NULL,
                *icRunning=NULL,
                *icStopped=NULL,
                *icDefault=NULL;

/*****************************************************************************
  Sources files cache:
  This cache holds the list of files we know where are located.
*****************************************************************************/

struct stFileCache
{
 char *gdb;
 char *real;
};

class TSourcesCache : public TNSSortedCollection
{
public:
 TSourcesCache() : TNSSortedCollection(10,10) {};

 void insert(const char *gdb, char *real, char *&newReal);
 stFileCache *At(ccIndex pos) { return (stFileCache *)at(pos); };

 virtual void *keyOf(void *item);
 virtual int compare(void *s1, void *s2);
 virtual void freeItem(void *s);
};

void *TSourcesCache::keyOf(void *item)
{
 return ((stFileCache *)item)->gdb;
}

int TSourcesCache::compare(void *s1, void *s2)
{
 return strcmp((char *)s1,(char *)s2);
}

void TSourcesCache::freeItem(void *s)
{
 stFileCache *p=(stFileCache *)s;
 delete[] p->gdb;
 delete[] p->real;
 delete p;
}

void TSourcesCache::insert(const char *gdb, char *real, char *&newReal)
{
 stFileCache *p=new stFileCache;
 p->gdb=newStr(gdb);
 newReal=p->real=newStr(real);
 TNSSortedCollection::insert(p);
}

TSourcesCache *sourcesCache=NULL;

static inline
void InitSourcesCache()
{
 if (!sourcesCache)
    sourcesCache=new TSourcesCache();
}

static inline
void DeInitSourcesCache()
{
 destroy0(sourcesCache);
}

static
char *SolveFileName(const char *s)
{
 if (CheckIfPathAbsolute(s))
    return edTestForFile(s) ? newStr(s) : NULL;
 // Look if we already know about it
 InitSourcesCache();
 ccIndex pos;
 if (sourcesCache->search((char *)s,pos))
   {
    stFileCache *p=sourcesCache->At(pos);
    dbgPr("Found source in cache: %s\n",p->real);
    return p->real;
   }
 // Nope, so we must do a search
 char *fullName, *retName;
 if (FindFile(s,fullName,binReference))
   {
    dbgPr("Adding %s to cache\n",fullName);
    sourcesCache->insert(s,fullName,retName);
    delete[] fullName;
    return retName;
   }
 // TODO: Bring a dialog like in RHIDE?
 dbgPr("Can't find %s file\n",s);
 return NULL;
}

/*****************************************************************************
  End of Sources files cache
*****************************************************************************/

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

    ol=SpLineGetOldValueOf(l,s,idsplBreak,&found);
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
 TVIntl::freeSt(icUnknown);
 TVIntl::freeSt(icNotStarted);
 TVIntl::freeSt(icDisconnected);
 TVIntl::freeSt(icConnected);
 TVIntl::freeSt(icReadyToRun);
 TVIntl::freeSt(icRunning);
 TVIntl::freeSt(icStopped);
 TVIntl::freeSt(icDefault);
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
 DeInitSourcesCache();
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

 struct stat st;
 if (!edTestForFile(dOps.program,st))
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
            char b[maxWStatus];
            int l=TVIntl::snprintf(b,maxWStatus,__("Attached to PID %d"),pid);
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
    ExtractBinReference();
    binMTime=st.st_mtime;
    // Apply breakpoints.
    DBG_ApplyBkpts();
    DBG_ApplyWpts();
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
         TSetEditorApp::setCmdState(cmeDbgEditBreakPts,True);
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
         TSetEditorApp::setCmdState(cmeDbgEditBreakPts,False);
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

 // We use the full name
 char s[PATH_MAX];
 strcpy(s,e->fileName);
 CLY_fexpand(s);

 if (dbg->GoTo(s,e->curPos.y+1))
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
   {
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
     const char *unknown=TVIntl::getText(__("unknown"),icUnknown);
     int l=TVIntl::snprintf(b,maxWStatus,__("%d: %s:%s:%d addr %p"),r->level,
                            r->func ? r->func : unknown,
                            r->file ? r->file : unknown,
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

Boolean TSetEditorApp::DebugCloseSession(Boolean confirm)
{
 if (dbg)
   {
    MIDebugger::eState st=dbg->GetState();
    if (st==MIDebugger::running || st==MIDebugger::stopped)
      {
       if (confirm &&
           messageBox(__("It will kill the program you are debugging. Go ahead?"),
                      mfConfirmation | mfYesButton | mfNoButton)!=cmYes)
          return False;
      }
    delete dbg; // It will close the debug session
    dbg=NULL;
   }
 TProgram::deskTop->lock();
 DebugMsgClose();
 WatchesClose();
 DebugCommonCleanUp();
 TProgram::deskTop->unlock();

 return True;
}

/*****************************************************************************
  End of Editor debug commands
*****************************************************************************/

/*****************************************************************************
  TBreakpoints class

 RHIDE dialog to be compatible:
 E/D file | line/function | Condition | Count
 Buttons: ~M~odify ~N~ew" ~D~elete ~E~nable D~i~sable ~S~how

*****************************************************************************/

mi_bkpt *TBreakpoints::first=NULL;
mi_bkpt *TBreakpoints::last=NULL;
int TBreakpoints::count=0;
stTVIntl *TBreakpoints::icNone=NULL;
static TBreakpoints bkpts;


mi_bkpt *TBreakpoints::getItem(int num)
{
 mi_bkpt *p=first;
 while (num && p)
   {
    num--;
    p=p->next;
   }
 return num ? NULL : p;
}

const int wThread=5, wTimes=5, wFormat=28;
const char *eThread="  *  ";
const char *eTimes ="  -  ";

static inline
void ComputeBreakW(int &wWhere, int &wCond, int maxLen)
{
 int rest;

 rest=maxLen-(7+wThread+wTimes);
 wWhere=rest*60/100;
 wCond=rest-wWhere;
 //printf("wWhere: %d wCond: %d\n",wWhere,wCond);
 // Add EOS
 wWhere++; wCond++;
}

void TBreakpoints::getText(char *dest, unsigned item, int maxLen)
{
 mi_bkpt *p=getItem(item);
 if (!p)
   {
    *dest=0;
    dbgPr("Oops! item %d gives NULL\n",item);
    return;
   }
 int wWhere, wCond;
 ComputeBreakW(wWhere,wCond,maxLen);

 AllocLocalStr(where,wWhere);
 switch (p->mode)
   {
    case m_file_line:
         CLY_snprintf(where,wWhere,"%s:%d",p->file,p->line);
         break;
    case m_function:
         CLY_snprintf(where,wWhere,"%s",p->func);
         break;
    case m_file_function:
         CLY_snprintf(where,wWhere,"%s:%s",p->file,p->func);
         break;
    case m_address:
         CLY_snprintf(where,wWhere,"%p",p->addr);
         break;
   }

 char format[wFormat];
 CLY_snprintf(format,wFormat,"%%%dd",wThread);
 char thread[wThread+1];
 if (p->thread>=0)
    CLY_snprintf(thread,wThread+1,format,p->thread);
 else
    strcpy(thread,eThread);

 CLY_snprintf(format,wFormat,"%%%dd",wTimes);
 char times[wTimes+1];
 if (p->ignore>0)
    CLY_snprintf(times,wTimes+1,format,p->ignore);
 else
    strcpy(times,eTimes);

 CLY_snprintf(format,wFormat,"%%-%ds",wCond-1);
 AllocLocalStr(cond,wCond);
 CLY_snprintf(cond,wCond,format,p->cond ? p->cond :
              TVIntl::getText(__("None"),icNone));

 CLY_snprintf(format,wFormat,"%%c|%%-%ds|%%s|%%s|%%s",wWhere-1);
 CLY_snprintf(dest,maxLen,format,p->enabled ? '*' : ' ',where,cond,times,
              thread);
}

void TBreakpoints::add(mi_bkpt *b)
{
 if (first)
    last->next=b;
 else
    first=b;
 last=b;
 // Solve the absolute name and cache it.
 updateAbs(b);
 count++;
}

void TBreakpoints::updateAbs(mi_bkpt *b)
{
 if (!b->file)
   {
    printf("Bogus breakpoint\n");
    return;
   }
 free(b->file_abs);
 char *file=SolveFileName(b->file);
 b->file_abs=strdup(file ? file : b->file);
}

void TBreakpoints::refreshBinRef()
{
 mi_bkpt *p=first;
 while (p)
   {
    updateAbs(p);
    p=p->next;
   }
}

mi_bkpt *TBreakpoints::search(const char *source, int line)
{
 mi_bkpt *b=first;

 while (b)
   {
    if (strcmp(b->file_abs,source)==0 && b->line==line)
       return b;
    b=b->next;
   }
 return b;
}

void TBreakpoints::remove(mi_bkpt *b)
{
 mi_bkpt *e=first, *ant=NULL;

 while (e)
   {
    if (e==b)
      {
       if (ant)
          ant->next=b->next;
       else
          first=b->next;
       if (!b->next)
          last=ant;
       b->next=NULL;
       mi_free_bkpt(b);
       count--;
       return;
      }
    ant=e;
    e=e->next;
   }
 // TODO: Remove
 dbgPr("Oops! can't find bkp TBreakpoints::remove\n");
}

void TBreakpoints::replace(mi_bkpt *old, mi_bkpt *b)
{
 mi_bkpt *e=first, *ant=NULL;

 while (e)
   {
    if (e==old)
      {
       if (ant)
          ant->next=b;
       else
          first=b;
       if (!old->next)
          last=b;
       b->next=old->next;
       old->next=NULL;
       mi_free_bkpt(old);
       return;
      }
    ant=e;
    e=e->next;
   }
 // TODO: Remove
 dbgPr("Oops! can't find bkp TBreakpoints::replace\n");
}

int TBreakpoints::set(const char *source, int line)
{
 mi_bkpt *b=dbg->Breakpoint(source,line);
 if (b)
   {
    add(b);
    return 1;
   }
 DebugMsgSetError();
 return 0;
}

int TBreakpoints::unset(const char *source, int line)
{
 mi_bkpt *b=search(source,line);
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
 remove(b);
 return 1;
}

void TBreakpoints::apply()
{
 if (!dbg)
    return;
 if (DEBUG_BREAKPOINTS_UPDATE)
    dbgPr("TBreakpoints::apply: Deleting all splines\n");
 SpLinesDeleteForId(idsplBreak); // Ensure no previous bkpt survived ;-)
 mi_bkpt *b=first, *aux;
 // Disconnect current list, we will be creating a new one.
 first=last=NULL;
 count=0;
 int disabledBkpts=0, killIt, applied=0;

 while (b)
   {
    killIt=0;
    if (b->enabled) // Avoid disabled ones.
      {
       // TODO: I'm quite sure we will lose something in the transfer, make
       // sure it doesn't.
       mi_bkpt *nb=dbg->Breakpoint(b);
       if (!nb)
         {
          b->enabled=0;
          add(b);
          disabledBkpts++;
         }
       else
         {
          add(nb);
          killIt=1;
          SpLinesAdd(nb->file_abs,nb->line,idsplBreak,False);
          applied++;
         }
      }
    else
       // Pass unchanged
       add(b);
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

void TBreakpoints::save(opstream &os)
{// Save them
 os << bkptsVersion << (int32)count;
 dbgPr("%d Breakpoints\n",count);
 mi_bkpt *p=first;
 while (p)
   {
    os.writeString(p->file);
    os.writeString(p->cond);
    os << (int32)p->line << p->enabled << (int32)p->ignore
       << (char)p->type << (char)p->disp;
    // v2
    os.writeString(p->func);
    os.write64((uint64)p->addr);
    os << (char)p->mode << (int32)p->thread;
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

void TBreakpoints::load(ipstream &is)
{
 char version;
 int32 cnt;

 is >> version >> cnt;
 // TODO:
 if (first)
    dbgPr("Oops! loading breakpoints when we already have!!!\n");
 dbgPr("%d breakpoints\n",cnt);
 for (int32 i=0; i<cnt; i++)
    {
     mi_bkpt *b=mi_alloc_bkpt();
     if (b)
       {
        b->file=ReadStringC(is);
        b->cond=ReadStringC(is);
        b->line=ReadInt32(is);
        is >> b->enabled;
        b->ignore=ReadInt32(is);
        b->type=(enum mi_bkp_type)ReadChar(is);
        b->disp=(enum mi_bkp_disp)ReadChar(is);
        if (version>=2)
          {// v2
           b->func=ReadStringC(is);
           b->addr=(void *)is.read64();
           b->mode=(enum mi_bkp_mode)ReadChar(is);
           b->thread=ReadInt32(is);
          }
        if (b->file)
          {
           add(b);
           SpLinesAdd(b->file_abs,b->line,idsplBreak,False);
          }
       }
    }
 if (count)
    SpLinesUpdate();
}

void TBreakpoints::releaseAll()
{
 mi_free_bkpt(first);
 first=last=NULL;
 count=0;
}

TBreakpoints::~TBreakpoints()
{
 releaseAll();
 TVIntl::freeSt(icNone);
}

const int wVisible=50, wInt=32, wLabels=9,
          wFilename=PATH_MAX,
          wFunction=256,
          wLine=wInt,
          wAddress=wInt,
          wCondition=256,
          wCount=wInt,
          wThreadB=wInt;

struct stBrkEdit
{
 uint32 type;
 char filename[wFilename],
      function[wFunction],
      line[wLine],
      address[wAddress],
      condition[wCondition],
      count[wCount],
      thread[wThreadB];
 uint32 enabled;
 uint32 hw;
};


class TDiagBrk : public TDialog
{
public:
 TDiagBrk(const TRect &aR);

 virtual void handleEvent(TEvent &event);

 int Modify();
 int Add();
 int Delete();
 int Enable();
 int Disable();
 int Go();
 static void UpdateCommadsForCount(int c);

 static const char *file;
 static int line;

protected:
 int focusedB;
 TStringableListBox *theLBox;
 static TSView *sviews[4];
 static uint32  vmasks[4];

 TDialog *createEdit(const char *title);
 static mi_bkpt *CreateBktFromBox(stBrkEdit &box);
 static int DeleteFromGDBandSpLines(mi_bkpt *b);
 static int ApplyBkt(mi_bkpt *nb, mi_bkpt *old, int enabled);
 void UpdateCommandsFocused();
};

const char *TDiagBrk::file=NULL;
// file,func,line,addr:
TSView *TDiagBrk::sviews[4];
uint32  TDiagBrk::vmasks[4]={0xA,0x4,0xC,0x1};
int TDiagBrk::line=0;

void TDiagBrk::UpdateCommadsForCount(int c)
{
 TSetEditorApp::setCmdState(cmBkModify,c ? True : False);
 TSetEditorApp::setCmdState(cmBkDel,c ? True : False);
 TSetEditorApp::setCmdState(cmBkGo,c ? True : False);
}

TDialog *TDiagBrk::createEdit(const char *title)
{
 TSViewCol *col=new TSViewCol(title);

 // EN: CDEFHILNOSTUW
 // ES: ABCDFHILORSUV
 TSLabel *type=TSLabelRadio(__("Type"),
                            __("File/L~i~ne"),
                            __("Fu~n~ction"),
                            __("File/Functi~o~n"),
                            __("~A~ddress"),0);
 // Same width
 TSHzLabel *file=new TSHzLabel(__(" ~F~ilename"),
                  new TSInputLine(wFilename,1,hID_DbgBkFilename,wVisible));
 // Same width
 TSHzLabel *func=new TSHzLabel(__(" F~u~nction"),
                  new TSInputLine(wFunction,1,hID_DbgBkFunction,wVisible));
 // Same width
 TSHzLabel *line=new TSHzLabel(__("     ~L~ine"),
                  new TSInputLine(wLine,1,hID_DbgBkLine,wVisible));
 // Same width
 TSHzLabel *addr=new TSHzLabel(__("  Addre~s~s"),
                  new TSInputLine(wAddress,1,hID_DbgBkAddress,wVisible));
 // Same width
 TSHzLabel *cond=new TSHzLabel(__("Con~d~ition"),
                  new TSInputLine(wCondition,1,hID_DbgBkCondition,wVisible));
 // Same width
 TSHzLabel *coun=new TSHzLabel(__("    Coun~t~"),
                  new TSInputLine(wCount,1,hID_DbgBkCount,(wVisible-wLabels)/2));
 TSHzLabel *thre=new TSHzLabel(__("   T~h~read"),
                  new TSInputLine(wThreadB,1,hID_DbgBkThread,(wVisible-wLabels)/2));
 TSHzGroup *c_t=MakeHzGroup(coun,thre,0);
 TSCheckBoxes *enable=new TSCheckBoxes(new TSItem(__("~E~nabled"),0));
 TSCheckBoxes *hw=new TSCheckBoxes(new TSItem(__("Hard~w~are assisted"),0));

 TSVeGroup *all=MakeVeGroup(0,type,file,func,line,addr,cond,c_t,enable,hw,0);
 all->makeSameW();

 // The dependencies:
 sviews[3]=file;
 sviews[2]=func;
 sviews[1]=line;
 sviews[0]=addr;
 ((TSRadioButtons *)type->linked)->setEnableMask(vmasks,sviews,4);

 col->insert(xTSLeft,yTSUp,all);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(hcEditBkpt);
 delete col;
 return d;
}

static
void ShowErrorInMsgBox()
{
 char *cErr=TVIntl::getTextNew(MIDebugger::GetErrorStr());
 int iErr=MIDebugger::GetErrorNumber();

 if (iErr==MI_FROM_GDB)
    messageBox(mfOKButton | mfError,
               __("Error: %s (%d) [%s]"),cErr,iErr,MIDebugger::GetGDBError());
 else
    messageBox(mfOKButton | mfError,__("Error: %s (%d)"),cErr,iErr);
 delete[] cErr;
}

mi_bkpt *TDiagBrk::CreateBktFromBox(stBrkEdit &box)
{
 // Create an structure to specify the new one
 mi_bkpt *nb=mi_alloc_bkpt();
 nb->mode=(enum mi_bkp_mode)box.type;
 char *end;
 switch (box.type)
   {
    case m_file_line:
         nb->file=strdup(box.filename);
         nb->line=atoi(box.line);
         break;
    case m_function:
         nb->func=strdup(box.function);
         break;
    case m_file_function:
         nb->file=strdup(box.filename);
         nb->func=strdup(box.function);
         break;
    case m_address:
         nb->addr=(void *)strtol(box.address,&end,0);
         break;
   }
 if (!IsEmpty(box.count))
    nb->ignore=atoi(box.count);
 if (!IsEmpty(box.thread))
    nb->thread=atoi(box.thread);
 if (box.hw)
    nb->type=t_hw;
 if (!IsEmpty(box.condition))
    nb->cond=strdup(box.condition);
 TBreakpoints::updateAbs(nb);
 nb->enabled=1;

 return nb;
}

int TDiagBrk::DeleteFromGDBandSpLines(mi_bkpt *p)
{
 if (p->enabled)
   {
    if (!dbg->BreakDelete(p))
      {
       ShowErrorInMsgBox();
       return 0;
      }
    // Update splines
    if (p->file_abs)
       SpLinesDeleteForId(idsplBreak,p->file_abs,True,p->line);
   }
 return 1;
}

int TDiagBrk::ApplyBkt(mi_bkpt *nb, mi_bkpt *old, int enabled)
{
 mi_bkpt *nb_gdb=dbg->Breakpoint(nb);
 if (nb!=old)
    mi_free_bkpt(nb);
 if (!nb_gdb)
   {
    ShowErrorInMsgBox();
    // Recover the old breakpoint
    if (old->enabled)
      {
       nb_gdb=dbg->Breakpoint(old);
       if (nb_gdb)
         {
          bkpts.updateAbs(nb_gdb);
          bkpts.replace(old,nb_gdb);
          if (nb_gdb->file_abs)
             SpLinesAdd(nb_gdb->file_abs,nb_gdb->line,idsplBreak,True);
         }
      }
    return 0;
   }
 // Ok, we succeed to apply the breakpoint.
 // Update our list:
 nb_gdb->enabled=enabled;
 bkpts.updateAbs(nb_gdb);
 bkpts.replace(old,nb_gdb);
 // If this breakpoint is disabled remove it.
 if (!nb_gdb->enabled)
    dbg->BreakDelete(nb_gdb);
 else
    if (nb_gdb->file_abs)
       SpLinesAdd(nb_gdb->file_abs,nb_gdb->line,idsplBreak,True);

 return 1;
}

int TDiagBrk::Modify()
{
 if (!bkpts.GetCount())
    return 0; // Just in case
 TDialog *d=createEdit(__("Modify breakpoint"));

 stBrkEdit box;
 mi_bkpt *p=bkpts.getItem(focusedB);

 box.type=p->mode;
 #define C(a,b,c) if (a) strncpyZ(b,a,c); else b[0]=0;
 C(p->file,box.filename,wFilename);
 C(p->func,box.function,wFunction);
 C(p->cond,box.condition,wCondition);
 #undef C
 CLY_snprintf(box.line,wLine,"%d",p->line);
 CLY_snprintf(box.address,wAddress,"%p",p->addr);
 if (p->ignore>0)
    CLY_snprintf(box.count,wCount,"%d",p->ignore);
 else
    box.count[0]=0;
 if (p->thread>=0)
    CLY_snprintf(box.thread,wThreadB,"%d",p->thread);
 else
    box.thread[0]=0;
 box.enabled=p->enabled ? 1 : 0;
 box.hw=p->type==t_breakpoint ? 0 : 1;

 if (execDialog(d,&box)==cmOK)
   {// Note: We know gdb is there. But this is complex.
    // Delete the current breakpoint
    if (!DeleteFromGDBandSpLines(p))
       return 0;
    // Create a structure to specify the new one
    mi_bkpt *nb=CreateBktFromBox(box);
    // Try to apply it
    return ApplyBkt(nb,p,box.enabled);
   }
 return 0;
}

int TDiagBrk::Add()
{
 TDialog *d=createEdit(__("Add breakpoint"));

 stBrkEdit box;
 // TODO: Could we use current file and function?
 memset(&box,0,sizeof(box));
 box.enabled=1;

 if (execDialog(d,&box)==cmOK)
   {// Note: We know gdb is there.
    // Create a structure to specify the new one
    mi_bkpt *nb=CreateBktFromBox(box);
    // Try to apply it
    mi_bkpt *nb_gdb=dbg->Breakpoint(nb);
    mi_free_bkpt(nb);
    if (!nb_gdb)
      {
       ShowErrorInMsgBox();
       return 0;
      }
    // Ok, we succeed to apply the breakpoint.
    // Update our list:
    nb_gdb->enabled=box.enabled;
    bkpts.add(nb_gdb);
    // If this breakpoint is disabled remove it.
    if (!nb_gdb->enabled)
       dbg->BreakDelete(nb_gdb);
    else
       if (nb_gdb->file_abs)
          SpLinesAdd(nb_gdb->file_abs,nb_gdb->line,idsplBreak,True);
    return 1;
   }
 return 0;
}

int TDiagBrk::Delete()
{
 if (!bkpts.GetCount())
    return 0; // Just in case
 mi_bkpt *p=bkpts.getItem(focusedB);
 // Delete the current breakpoint
 if (!DeleteFromGDBandSpLines(p))
    return 0;
 bkpts.remove(p);
 return 1;
}

int TDiagBrk::Enable()
{
 if (!bkpts.GetCount())
    return 0; // Just in case

 mi_bkpt *p=bkpts.getItem(focusedB);
 if (p->enabled)
    return 0;
 p->enabled=1;

 return ApplyBkt(p,p,1);
}

int TDiagBrk::Disable()
{
 if (!bkpts.GetCount())
    return 0; // Just in case

 mi_bkpt *p=bkpts.getItem(focusedB);
 if (!p->enabled)
    return 0;

 if (DeleteFromGDBandSpLines(p))
   {
    p->enabled=0;
    return 1;
   }
 return 0;
}

void TDiagBrk::UpdateCommandsFocused()
{
 mi_bkpt *p=bkpts.getItem(focusedB);
 TSetEditorApp::setCmdState(cmBkEnable,!p || p->enabled ? False : True);
 TSetEditorApp::setCmdState(cmBkDisable,!p || !p->enabled ? False : True);
}
int TDiagBrk::Go()
{
 // TODO: looks like all actions checks it, move to the caller.
 if (!bkpts.GetCount())
    return 0; // Just in case

 mi_bkpt *p=bkpts.getItem(focusedB);
 if (p->file_abs)
   {
    file=p->file_abs;
    line=p->line;
    return 1;
   }

 return 0;
}

void TDiagBrk::handleEvent(TEvent &event)
{
 int range=0, drawV=0;

 TDialog::handleEvent(event);
 if (event.what==evCommand)
   {
    switch (event.message.command)
      {
       case cmBkModify:
            if (Modify())
               drawV=1;
            break;
       case cmBkAdd:
            if (Add())
               drawV=range=1;
            break;
       case cmBkDel:
            if (Delete())
               drawV=range=1;
            break;
       case cmBkEnable:
            if (Enable())
              drawV=1;
            break;
       case cmBkDisable:
            if (Disable())
              drawV=1;
            break;
       case cmBkGo:
            if (Go())
               endModal(cmBkGo);
            break;
       default:
            return;
      }
    if (range)
      {
       int c=bkpts.GetCount();
       UpdateCommadsForCount(c);
       theLBox->setRange(c);
      }
    if (drawV)
      {
       theLBox->drawView();
       UpdateCommandsFocused();
      }
    clearEvent(event);
   }
 else if (event.what==evBroadcast)
   {
    switch (event.message.command)
      {
       case cmListItemFocused:
            focusedB=((TListViewer *)event.message.infoPtr)->focused;
            UpdateCommandsFocused();
            break;
       default:
            return;
      }
    clearEvent(event);
   }
}

TDiagBrk::TDiagBrk(const TRect &r) :
    TWindowInit(TDiagBrk::initFrame),
    TDialog(r,__("Breakpoints"))
{
 flags=wfMove | wfClose;
 growMode=gfGrowLoY | gfGrowHiX | gfGrowHiY;
 focusedB=0;

 TSetEditorApp::setCmdState(cmBkEnable,False);
 TSetEditorApp::setCmdState(cmBkDisable,False);

 TSViewCol *col=new TSViewCol(this);

 int wWhere, wCond;
 int width=r.b.x-r.a.x-4;
 ComputeBreakW(wWhere,wCond,width);
 char format[wFormat];
 CLY_snprintf(format,wFormat," %%s %%-%ds %%-%ds %%-%ds %%-%ds",wWhere-1,
              wCond-1,wTimes,wThread);
 AllocLocalStr(cols,width+1);

 // The following is quite annoying because we construct the format from its
 // parts.
 // Keep 1 char
 char *cE=TVIntl::getTextNew(__("E"));
 char *cWhere=TVIntl::getTextNew(__("Where"));
 char *cCond=TVIntl::getTextNew(__("Condition"));
 // Keep 5 chars
 char *cCount=TVIntl::getTextNew(__("Count"));
 // Keep 5 chars
 char *cThread=TVIntl::getTextNew(__("Thre."));
 CLY_snprintf(cols,width+1,format,cE,cWhere,cCond,cCount,cThread);
 delete[] cE;
 delete[] cWhere;
 delete[] cCond;
 delete[] cCount;
 delete[] cThread;

 int height=r.b.y-r.a.y-3;
 TSStringableListBox *lbox=new TSStringableListBox(width,height,tsslbVertical);
 theLBox=(TStringableListBox *)(lbox->view);
 TSLabel *lb=new TSLabel(cols,lbox);

 col->insert(xTSLeft,yTSUp,lb);
 // EN: DEIMNSX
 // ES: BDHIMNS
 col->insert(xTSCenter,yTSDown,MakeHzGroup(
             new TSButton(__("E~x~it")   ,cmOK,bfDefault),
             new TSButton(__("~M~odify") ,cmBkModify),
             new TSButton(__("~N~ew")    ,cmBkAdd),
             new TSButton(__("~D~elete") ,cmBkDel),
             new TSButton(__("~E~nable") ,cmBkEnable),
             new TSButton(__("D~i~sable"),cmBkDisable),
             new TSButton(__("~S~how")   ,cmBkGo),0));

 col->doItCenter(hcBkptDialog);
 delete col;
}

void TSetEditorApp::DebugEditBreakPts()
{
 // Currently we allow editing breakpoints only if we can get feadback from
 // gdb. I think its possible to avoid it, but I don't see the point and
 // the consequences can be quite confusing.
 if (!DebugCheckStopped())
    return;

 TRect r=GetDeskTopSize();
 TDiagBrk *d=new TDiagBrk(TRect(0,0,r.b.x,r.b.y-3));
 TDiagBrk::UpdateCommadsForCount(bkpts.GetCount());

 TStringableListBoxRec box;
 box.items=&bkpts;
 box.selection=0;

 if (execDialog(d,&box)==cmBkGo)
    GotoFileLine(TDiagBrk::line,(char *)TDiagBrk::file);
}

static
void MoveBreakpoint(const char *file, stSpLine *spline, void *data)
{
 mi_bkpt *b=bkpts.search(file,spline->oline+1);
 if (b)
   {
    dbgPr("Moved %s:%d to line %d\n",file,spline->oline+1,spline->nline+1);
    spline->oline=spline->nline;
    b->line=spline->nline+1;
   }
 else
   {// TODO: Change to dbgPr
    printf("Don't know how to move %s:%d\n",file,spline->oline+1);
   }
}

/**[txh]********************************************************************

  Description:
  Transfers the SpLines movements to the list of breakpoints.
  
***************************************************************************/

void TSetEditorApp::DebugMoveBreakPts()
{
 struct stat st;

 if (!edTestForFile(dOps.program,st))
    return;
 if (binMTime!=st.st_mtime)
   {
    binMTime=st.st_mtime;
    SpLinesForEach(idsplBreak,MoveBreakpoint);
   }
}

/*****************************************************************************
  End of TBreakpoints class
*****************************************************************************/

/*****************************************************************************
  TWatchpoints class
*****************************************************************************/

mi_wp *TWatchpoints::first=NULL;
mi_wp *TWatchpoints::last=NULL;
int TWatchpoints::count=0;
static TWatchpoints wpts;


mi_wp *TWatchpoints::getItem(int num)
{
 mi_wp *p=first;
 while (num && p)
   {
    num--;
    p=p->next;
   }
 return num ? NULL : p;
}

void TWatchpoints::getText(char *dest, unsigned item, int maxLen)
{
 mi_wp *p=getItem(item);
 if (!p)
   {
    *dest=0;
    dbgPr("Oops! item %d gives NULL\n",item);
    return;
   }
 CLY_snprintf(dest,maxLen,"%c %c %c %s",p->enabled ? '*' : ' ',
              (p->mode==wm_read || p->mode==wm_rw) ? 'R' : ' ',
              (p->mode==wm_write|| p->mode==wm_rw) ? 'W' : ' ',
              p->exp);
}

void TWatchpoints::add(mi_wp *b)
{
 if (first)
    last->next=b;
 else
    first=b;
 last=b;
 count++;
}

void TWatchpoints::remove(mi_wp *b)
{
 mi_wp *e=first, *ant=NULL;

 while (e)
   {
    if (e==b)
      {
       if (ant)
          ant->next=b->next;
       else
          first=b->next;
       if (!b->next)
          last=ant;
       b->next=NULL;
       mi_free_wp(b);
       count--;
       return;
      }
    ant=e;
    e=e->next;
   }
 // TODO: Remove
 dbgPr("Oops! can't find wp TWatchpoints::remove\n");
}

void TWatchpoints::replace(mi_wp *old, mi_wp *b)
{
 mi_wp *e=first, *ant=NULL;

 while (e)
   {
    if (e==old)
      {
       if (ant)
          ant->next=b;
       else
          first=b;
       if (!old->next)
          last=b;
       b->next=old->next;
       old->next=NULL;
       mi_free_wp(old);
       return;
      }
    ant=e;
    e=e->next;
   }
 // TODO: Remove
 dbgPr("Oops! can't find wp TWatchpoints::replace\n");
}

void TWatchpoints::apply()
{
 if (!dbg)
    return;
 mi_wp *b=first, *aux;
 // Disconnect current list, we will be creating a new one.
 first=last=NULL;
 count=0;
 int disabledWpts=0, killIt, applied=0;

 while (b)
   {
    killIt=0;
    if (b->enabled) // Avoid disabled ones.
      {
       mi_wp *nb=dbg->Watchpoint(b->mode,b->exp);
       if (!nb)
         {
          b->enabled=0;
          add(b);
          disabledWpts++;
         }
       else
         {
          add(nb);
          killIt=1;
          applied++;
         }
      }
    else
       // Pass unchanged
       add(b);
    aux=b->next;
    if (killIt)
      {
       b->next=NULL;
       mi_free_wp(b);
      }
    b=aux;
   }
 if (disabledWpts)
    messageBox(mfWarning | mfOKButton,
               __("%d watchpoints failed to apply, they are disabled now."),
               disabledWpts);
}

void TWatchpoints::save(opstream &os)
{// Save them
 os << wptsVersion << (int32)count;
 dbgPr("%d Watchpoints\n",count);
 mi_wp *p=first;
 while (p)
   {
    os << p->enabled << (char)p->mode;
    os.writeString(p->exp);
    p=p->next;
   }
}

void TWatchpoints::load(ipstream &is, char version)
{
 int32 cnt;

 is >> cnt;
 // TODO:
 if (first)
    dbgPr("Oops! loading watchpoints when we already have!!!\n");
 dbgPr("%d watchpoints\n",cnt);
 for (int32 i=0; i<cnt; i++)
    {
     mi_wp *b=mi_alloc_wp();
     if (b)
       {
        is >> b->enabled;
        b->mode=(enum mi_wp_mode)ReadChar(is);
        b->exp=ReadStringC(is);
        add(b);
       }
    }
}

void TWatchpoints::releaseAll()
{
 mi_free_wp(first);
 first=last=NULL;
 count=0;
}

TWatchpoints::~TWatchpoints()
{
 releaseAll();
}

mi_wp *TWatchpoints::search(int num)
{
 mi_wp *b=first;

 while (b)
   {
    if (b->number==num && b->enabled)
       return b;
    b=b->next;
   }
 return b;
}

int TWatchpoints::unset(int num)
{
 mi_wp *b=search(num);
 if (!b)
   {// TODO: Remove
    dbgPr("Oops! where is wp %s\n",num);
    return 0;
   }
 if (!dbg->WatchDelete(b))
   {
    DebugMsgSetError();
    return 0;
   }
 b->enabled=0;
 return 1;
}

const int wExp=256;

struct stWpEdit
{
 char exp[wExp];
 uint32 mode;
 uint32 enabled;
};


class TDiagWp : public TDialog
{
public:
 TDiagWp(const TRect &aR);

 virtual void handleEvent(TEvent &event);

 int Modify();
 int Add();
 int Delete();
 int Enable();
 int Disable();
 static void UpdateCommadsForCount(int c);

protected:
 int focusedB;
 TStringableListBox *theLBox;

 TDialog *createEdit(const char *title);
 static mi_wp *CreateWpFromBox(stWpEdit &box);
 static int DeleteFromGDB(mi_wp *b);
 static int ApplyWp(mi_wp *nb, mi_wp *old, int enabled);
 void UpdateCommandsFocused();
};

void TDiagWp::UpdateCommadsForCount(int c)
{
 TSetEditorApp::setCmdState(cmBkModify,c ? True : False);
 TSetEditorApp::setCmdState(cmBkDel,c ? True : False);
}

TDialog *TDiagWp::createEdit(const char *title)
{
 TSViewCol *col=new TSViewCol(title);

 // EN: AERTW
 // ES: AELST
 TSLabel *exp=new TSLabel(__("~E~xpression (escape \" characters: \\\")"),
                          new TSInputLinePiped(wExp,1,hID_DbgEvalModifyExp,wVisible));
 TSLabel *type=TSLabelRadio(__("~T~ype"),
                            __("~W~rite"),
                            __("~R~ead"),
                            __("~A~ccess (read or write)"),0);
 TSCheckBoxes *enable=new TSCheckBoxes(new TSItem(__("~E~nabled"),0));

 TSVeGroup *all=MakeVeGroup(0,exp,type,enable,0);
 all->makeSameW();

 col->insert(xTSLeft,yTSUp,all);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(hcEditWp);
 delete col;
 return d;
}

#define BoxMode2WpMode(a) ((enum mi_wp_mode)(a+1))

mi_wp *TDiagWp::CreateWpFromBox(stWpEdit &box)
{
 // Create an structure to specify the new one
 mi_wp *nb=mi_alloc_wp();
 nb->mode=BoxMode2WpMode(box.mode);
 nb->exp=strdup(box.exp);
 nb->enabled=1;

 return nb;
}

int TDiagWp::DeleteFromGDB(mi_wp *p)
{
 if (p->enabled)
   {
    if (!dbg->WatchDelete(p))
      {
       ShowErrorInMsgBox();
       return 0;
      }
   }
 return 1;
}

int TDiagWp::Modify()
{
 if (!wpts.GetCount())
    return 0; // Just in case
 TDialog *d=createEdit(__("Modify watchpoint"));

 stWpEdit box;
 mi_wp *p=wpts.getItem(focusedB);

 box.mode=p->mode-1;
 #define C(a,b,c) if (a) strncpyZ(b,a,c); else b[0]=0;
 C(p->exp,box.exp,wExp);
 #undef C
 box.enabled=p->enabled ? 1 : 0;

 if (execDialog(d,&box)==cmOK)
   {// Note: We know gdb is there.
    // Delete the current watchpoint
    if (!DeleteFromGDB(p))
       return 0;
    if (box.enabled)
      {// Apply it
       mi_wp *nw_gdb=dbg->Watchpoint(BoxMode2WpMode(box.mode),box.exp);
       if (!nw_gdb)
         {
          ShowErrorInMsgBox();
          p->enabled=0;
          return 0;
         }
       wpts.replace(p,nw_gdb);
      }
    else
      {// Just modify it
       free(p->exp);
       p->exp=strdup(box.exp);
       p->mode=BoxMode2WpMode(box.mode);
       p->enabled=0;
      }
    return 1;
   }
 return 0;
}

int TDiagWp::Add()
{
 TDialog *d=createEdit(__("Add watchpoint"));

 stWpEdit box;
 memset(&box,0,sizeof(box));
 box.enabled=1;

 if (execDialog(d,&box)==cmOK)
   {// Note: We know gdb is there.
    if (box.enabled)
      {// Apply it
       mi_wp *nw_gdb=dbg->Watchpoint(BoxMode2WpMode(box.mode),box.exp);
       if (!nw_gdb)
         {
          ShowErrorInMsgBox();
          return 0;
         }
       wpts.add(nw_gdb);
      }
    else
      {// Just add to the list
       wpts.add(CreateWpFromBox(box));
      }
    return 1;
   }
 return 0;
}

int TDiagWp::Delete()
{
 if (!wpts.GetCount())
    return 0; // Just in case
 mi_wp *p=wpts.getItem(focusedB);
 // Delete the current breakpoint
 if (!DeleteFromGDB(p))
    return 0;
 wpts.remove(p);
 return 1;
}

int TDiagWp::Enable()
{
 if (!wpts.GetCount())
    return 0; // Just in case

 mi_wp *p=wpts.getItem(focusedB);
 if (p->enabled)
    return 0;

 mi_wp *nw_gdb=dbg->Watchpoint(p->mode,p->exp);
 if (!nw_gdb)
   {
    ShowErrorInMsgBox();
    return 0;
   }
 wpts.replace(p,nw_gdb);

 return 1;
}

int TDiagWp::Disable()
{
 if (!wpts.GetCount())
    return 0; // Just in case

 mi_wp *p=wpts.getItem(focusedB);
 if (!p->enabled)
    return 0;

 if (DeleteFromGDB(p))
   {
    p->enabled=0;
    return 1;
   }
 return 0;
}

void TDiagWp::UpdateCommandsFocused()
{
 mi_wp *p=wpts.getItem(focusedB);
 TSetEditorApp::setCmdState(cmBkEnable,!p || p->enabled ? False : True);
 TSetEditorApp::setCmdState(cmBkDisable,!p || !p->enabled ? False : True);
}

void TDiagWp::handleEvent(TEvent &event)
{
 int range=0, drawV=0;

 TDialog::handleEvent(event);
 if (event.what==evCommand)
   {
    switch (event.message.command)
      {
       case cmBkModify:
            if (Modify())
               drawV=1;
            break;
       case cmBkAdd:
            if (Add())
               drawV=range=1;
            break;
       case cmBkDel:
            if (Delete())
               drawV=range=1;
            break;
       case cmBkEnable:
            if (Enable())
              drawV=1;
            break;
       case cmBkDisable:
            if (Disable())
              drawV=1;
            break;
       default:
            return;
      }
    if (range)
      {
       int c=wpts.GetCount();
       UpdateCommadsForCount(c);
       theLBox->setRange(c);
      }
    if (drawV)
      {
       theLBox->drawView();
       UpdateCommandsFocused();
      }
    clearEvent(event);
   }
 else if (event.what==evBroadcast)
   {
    switch (event.message.command)
      {
       case cmListItemFocused:
            focusedB=((TListViewer *)event.message.infoPtr)->focused;
            UpdateCommandsFocused();
            break;
       default:
            return;
      }
    clearEvent(event);
   }
}

TDiagWp::TDiagWp(const TRect &r) :
    TWindowInit(TDiagWp::initFrame),
    TDialog(r,__("Watchpoints"))
{
 flags=wfMove | wfClose;
 growMode=gfGrowLoY | gfGrowHiX | gfGrowHiY;
 focusedB=0;

 TSetEditorApp::setCmdState(cmBkEnable,False);
 TSetEditorApp::setCmdState(cmBkDisable,False);

 TSViewCol *col=new TSViewCol(this);

 int width=r.b.x-r.a.x-4;
 int height=r.b.y-r.a.y-3;
 TSStringableListBox *lbox=new TSStringableListBox(width,height,tsslbVertical);
 theLBox=(TStringableListBox *)(lbox->view);
 TSLabel *lb=new TSLabel(__(" E R/W Expression"),lbox);

 col->insert(xTSLeft,yTSUp,lb);
 // EN: DEIMNX
 // ES: BDHMNS
 col->insert(xTSCenter,yTSDown,MakeHzGroup(
             new TSButton(__("E~x~it")   ,cmOK,bfDefault),
             new TSButton(__("~M~odify") ,cmBkModify),
             new TSButton(__("~N~ew")    ,cmBkAdd),
             new TSButton(__("~D~elete") ,cmBkDel),
             new TSButton(__("~E~nable") ,cmBkEnable),
             new TSButton(__("D~i~sable"),cmBkDisable),0));

 col->doItCenter(hcWpDialog);
 delete col;
}

void TSetEditorApp::DebugEditWatchPts()
{
 // Currently we allow editing breakpoints only if we can get feadback from
 // gdb. I think its possible to avoid it, but I don't see the point and
 // the consequences can be quite confusing.
 if (!DebugCheckStopped())
    return;

 TRect r=GetDeskTopSize();
 TDiagWp *d=new TDiagWp(TRect(0,0,r.b.x,r.b.y-3));
 TDiagWp::UpdateCommadsForCount(wpts.GetCount());

 TStringableListBoxRec box;
 box.items=&wpts;
 box.selection=0;

 execDialog(d,&box);
}

/*****************************************************************************
  End of TWatchpoints class
*****************************************************************************/

/*****************************************************************************
  GDB/MI interface
*****************************************************************************/

int DBG_SetBreakpoint(const char *source, int line)
{
 return bkpts.set(source,line);
}

int DBG_RemoveBreakpoint(const char *source, int line)
{
 return bkpts.unset(source,line);
}

/**[txh]********************************************************************

  Description:
  Transfer our list to gdb and set the lines as special ones.
  
***************************************************************************/

void DBG_ApplyBkpts()
{
 bkpts.apply();
}

void DBG_ApplyWpts()
{
 wpts.apply();
}

void DBG_SaveBreakpoints(opstream &os)
{
 bkpts.save(os);
}

void DBG_SaveWatchpoints(opstream &os)
{
 wpts.save(os);
}

void DBG_ReadBreakpoints(ipstream &is)
{
 bkpts.load(is);
}

void DBG_ReadWatchpoints(ipstream &is, char version)
{
 wpts.load(is,version);
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
 bkpts.releaseAll();
 wpts.releaseAll();
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
 void SetStatusGDB(const char *s) { statusF->setText(s); };
 void SetStatusStop(const char *s) { status2->setText(s); };
 void SetStatusError(const char *s) { status2->setText(s); };
 void SetStatusMode(const char *s) { status1->setText(s); };
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
 const char *msg=DebugMsgStateName();
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
            new TSInputLinePiped(widthGDBCom,1,hID_DbgCommand,maxGDBComBox));

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

const char *DebugMsgStateName()
{
 const char *msg=NULL;
 if (dbg)
   {
    switch (dbg->GetState())
      {
       case MIDebugger::disconnected:
            msg=TVIntl::getText(__("Disconnected"),icDisconnected);
            break;
       case MIDebugger::connected:
            msg=TVIntl::getText(__("Connected"),icConnected);
            break;
       case MIDebugger::target_specified:
            msg=TVIntl::getText(__("Ready to Run"),icReadyToRun);
            break;
       case MIDebugger::running:
            msg=TVIntl::getText(__("Running"),icRunning);
            break;
       case MIDebugger::stopped:
            msg=TVIntl::getText(__("Stopped"),icStopped);
            break;
      }
   }
 else
    msg=TVIntl::getText(__("Not started"),icNotStarted);

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
 const char *msg=DebugMsgStateName();
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

 char *cErr=TVIntl::getTextNew(MIDebugger::GetErrorStr());
 int iErr=MIDebugger::GetErrorNumber();

 int l=TVIntl::snprintf(b,maxWStatus,__("Error: %s (%d)"),cErr,iErr);
 delete[] cErr;

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
            tty=TVIntl::getText(__("default"),icDefault);
         TVIntl::snprintf(b,maxWStatus,__("Mode: %s (%s) [%s]"),
                          localMode==modeX11 ? "X11" : "Console",tty,dOps.program);
         break;
    case dmPID:
         break;
    case dmRemote:
         TVIntl::snprintf(b,maxWStatus,__("Mode: Remote (%s) [%s]"),
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
       char *file=SolveFileName(f->file);
       if (file && GotoFileLine(f->line,file,msg,-1,l,False))
          jumped=1;
      }
   }
 return jumped;
}

int DebugMsgFillReason(mi_frames *f, char *b, Boolean stop)
{
 int l;

 if (stop)
   {
    char *reason=TVIntl::getTextNew(mi_reason_enum_to_str(stoppedInfo->reason));
    l=TVIntl::snprintf(b,maxWStatus,__("Reason stopped: %s"),reason);
    delete[] reason;
   }
 else
    l=TVIntl::snprintf(b,maxWStatus,__("Returned immediatly"));

 if (f && l+10<maxWStatus)
   {
    const char *unknown=TVIntl::getText(__("unknown"),icUnknown);
    l+=CLY_snprintf(b+l,maxWStatus-l," [%s:%s:%d]",
                    f->func ? f->func : unknown,
                    f->file ? f->file : unknown,
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
 if (stoppedInfo->reason==sr_wp_scope && stoppedInfo->have_wpno)
   {// Must be disabled
    wpts.unset(stoppedInfo->wpno);
    printf("Disabling watchpoint %d, out of scope\n",stoppedInfo->wpno);
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
       TVIntl::snprintf(dest,maxLen,__("%s: Not yet in debugger"),gv->exp);
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

 int AddWatch(Boolean wScope, char *val=NULL);
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
    show();
    if (owner->current!=this)
       select();
   }
 return res;
}

TDialog *createEditExp(char *tit)
{
 TSViewCol *col=new TSViewCol(tit);

 // EN: E
 TSLabel *o1=new TSLabel(__("~E~xpression"),
           new TSInputLinePiped(widthWtExp,1,hID_DbgEvalModifyExp,maxWtBox));

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

int TWatchesDialog::AddWatch(Boolean wScope, char *val)
{
 char exp[widthWtExp];
 if (val)
    strncpyZ(exp,val,widthWtExp);
 else
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

void TSetEditorApp::DebugWatchExp(Boolean wScope, char *val)
{
 WatchesInit();
 WtWindow->AddWatch(wScope,val);
 delete[] val;
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
    char *fakeName=TVIntl::getTextNew(__("Your program compiled with -g"));
    strcpy(dOps.program,fakeName);
    delete[] fakeName;
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
 // Watchpoints
 DBG_SaveWatchpoints(os);
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
 // Watchpoints
 is >> aux;
 if (aux)
    DBG_ReadWatchpoints(is,aux);
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
void TSetEditorApp::DebugCloseSession(Boolean ) { return True; }
int  TSetEditorApp::DebugCheckAcceptCmd(Boolean ) { return 0; }
int  TSetEditorApp::DebugCheckStopped(Boolean ) { return 1; }
void TSetEditorApp::DebugEditBreakPts() {}
void TSetEditorApp::DebugEditWatchPts() {}
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

#if 0
__("Hit a breakpoint")
__("Write watchpoint")
__("Read watchpoint")
__("Access watchpoint")
__("Watchpoint out of scope")
__("Function finished")
__("Location reached")
__("End of stepping")
__("Exited signalled")
__("Exited with error")
__("Exited normally")
__("Signal received")
__("Unknown (temp bkp?)")
__("natural")
__("binary")
__("decimal")
__("hexadecimal")
__("octal")
__("unknown")
__("Ok")
__("Out of memory")
__("Pipe creation")
__("Fork failed")
__("GDB not running")
__("Parser failed")
__("Unknown asyn response")
__("Unknown result response")
__("Error from gdb")
__("Unknown")
#endif

