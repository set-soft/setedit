/* Copyright (C) 2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Module: Debugger interface.
  Comments:
  It implements the interface with the debugger.

  TODO:
  The most important unimplemented features and unsolved things are:


-----------------------------------------------
Low priority:

General:
* Some stop mechanism to avoid waiting for response time out.
* Some way to revert the "don't show next time" options. Like with the
advices. It means we must concentrate them into one place.
* Mechanism to select a PID from the running processes.

Breakpoints:
* Detect recompilation of the target from "outside", that's without using Run
program. A good moment could be when we are restarting. In this case move the
breakpoints.
* Right click on a line where we have a bkpt should offer to edit it.
* When adding a new breakpoint from the dialog use the current file and
function to field the empty fields.

Inspectors:
* Highlight changed values.
* Add "graphic lines" to the tree.
* Mouse click to un/fold sections.

Watches:
* Highlight changed values.

Data Window:
* Small dialog with all the modes.
* Mouse movement.
* Configure the max length (same warning).
* Things ML had:
 - search(-next,-prev) ^S ^N ^P
 - bookmarks?
 - try to fix 32-bit dependencies
 - change address types to correct types using libgdb.h

Debug Window:
* Save content to disk.

Disassembler Window:
* Keys or similar to change the size used by the registers and the code.
* Some, slow, way to choose the format for the registers. It will imply
fetching its values by batchs with the same format.
* Support syntax hl. for other CPUs.
* The "code" created is pure UNIX \n text, but what about DOS files with
\r\n? (I mean source C/C++ files mixed with asm code).
* Option to avoid the "auto open" (current behavior)

by Function:
void TSetEditorApp::DebugFinishFun()
void TSetEditorApp::DebugReturnNow() (same?)
 Should we check if we are at main? Not sure, the check is expensive:
1) Get the list of frames, 2) Determine if we are at the top, 3) Release the
frames. First time I got the gdb error I got confused, but is quite obvious.

void TSetEditorApp::DebugCallStack()
should I modify mi lib to return something better ...
Move the code to the "parser.c" in migdb

int TDisAsmEdWin::jumpToFrame(mi_frames *f)
We should lock the editor until the end, that's annoying because of the
unlocks on fails.

void TInspector::updateVars(mi_gvar_chg *changed)
What if a variable changes its type? I don't know how can it be but gdb code
seems to support it.

int isValidAddress(const char *taddr, unsigned long &addr)
It determines if something is a pointer using a very weak methode.

void TDataViewer::handleEvent(TEvent & event)
It does a complete redraw when 1 char is inserted, it looks like an overkill.

-----------------------------------------------
Whish gdb could:

HAVE LESS BUGS!!!!

Eval/Modify:
* Set the format for results, currently you must inspect the variable to do it.

Disassembler Window:
* Choose the format for registers individualy, not for all.
* Avoid reporting wrong functions. Like when I stop during a getchar and it says
it was executing read but the read symbol is 800 KB away from the real address.


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

3) When gdb stops in a function inside a library (no debug info) the
reported function can be bogus. In my tests with getchar it says we stopped
in read (when sending a sigint or attaching), but read is 800 KB away in
another library. So in the middle we have a gap with unaccessable memory. So
when we ask gdb to disassemble read we cross the gap and gdb dies. Here I see
two bugs: 1) Wrong function name and 2) Crashing just because reading
unaccessable memory.

***************************************************************************/

#ifndef _GNU_SOURCE
 // SuSE SLES 8 defines it at the command line!
 #define _GNU_SOURCE
#endif
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
#define Uses_TPalette
#define Uses_TKeys
#define Uses_TGKey

// Messages
#define Uses_TDialog
#define Uses_TScroller
#define Uses_TScrollBar
#define Uses_TListBox
#define Uses_TDeskTop
#define Uses_TNoStaticText
#define Uses_AllocLocal
#define Uses_TFileDialog

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
#define Uses_TSNoStaticText

#define Uses_TSSortedListBox
#define Uses_TDialogAID
#define Uses_FileOpenAid
#define Uses_TCEditWindow

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
 #define Uses_TDisAsmWin
#endif
#include <debug.h>
#include <dyncat.h>
#include <edhists.h>
#include <dskwin.h>
#include <pathlist.h>
#include <diaghelp.h>
#include <pathtool.h> // FindFile
#include <edcollec.h>
#include <edspecs.h>
#include <loadshl.h> // SHLNumberOf
#include <advice.h>

const uint32 msgConsole=1, msgTarget=2, msgLog=4, msgTo=8, msgFrom=16;
static uint32 msgOps=msgConsole | msgTarget | msgLog;
const char svPresent=1, svAbsent=0, svYes=1, svNo=0;
const char bkptsVersion=2;
const char wptsVersion=1;
const char inspVersion=2;
const char dwVersion=2;
const char debugDataVersion=1;
const char msgWinVersion=2;
const char watchWinVersion=2;

#ifdef HAVE_GDB_MI

#include <sys/time.h> // Profile

#define DEBUG_ME 0
#if DEBUG_ME
 #define dbgPr(format, args...) printf(format , ## args)
#else
 #define dbgPr(format, args...)
#endif
#define DEBUG_BREAKPOINTS_UPDATE 0
#define PROFILE 0

// Variables for the configuration
const int maxWBox=70, widthFiles=256, widthShort=80, widthExpRes=1024;
const int widthWtExp=widthFiles, maxWtBox=maxWBox;
const int widthGDBCom=widthFiles, maxGDBComBox=maxWBox;
const int widthPID=32;
const unsigned dmLocal=0, dmPID=1, dmRemote=2;
const int wTimeOut=8, wLinesMsg=8;
const int wVisible=50, wInt=32, wLabels=9,
          wFilename=PATH_MAX,
          wFunction=256,
          wLine=wInt,
          wAddress=wInt,
          wCondition=256,
          wCount=wInt,
          wThreadB=wInt;

struct DebugOptionsStruct
{
 char program[wFilename];
 uint32 mode;
 char args[widthFiles];
 char tty[widthShort];
 char rtype[widthShort];
 char rparam[widthShort];
};
static int debugOpsNotIndicated=1;
static DebugOptionsStruct dOps;
// End of Variables for the configuration

const int defaultTimeOut=MI_DEFAULT_TIME_OUT, defaultLinesMsg=1000;
const int maxWDbgSt=20, maxWStatus=256;
const unsigned miscNoBanner=1, miscMIv2=2, miscSymWA=4, miscNoSourceInAsm=8;

const int modeX11=0, modeLinux=1;
static int localMode;
static MIDebugger *dbg=NULL;
// Last async response
static mi_stop *stoppedInfo=NULL;
static char pendingStoppedInfo=0;
static char killAfterStop=0;
// CPU line
static char *cpuLFile=NULL;
static int   cpuLLine;
// Path for the binary
static char *binReference=NULL;
static int   binReferenceLen=0;
// Modification time
static time_t binMTime;
// Main function (main)
static char *mainFunction=NULL;
// Max. lines in debug window
static int msgsInDebugWindow;

// Some common messages
static stTVIntl *icUnknown=NULL,
                *icNotStarted=NULL,
                *icDisconnected=NULL,
                *icConnected=NULL,
                *icReadyToRun=NULL,
                *icRunning=NULL,
                *icStopped=NULL,
                *icDefault=NULL,
                *icInspOk=NULL,
                *icInspWait=NULL,
                *icInspNoScope=NULL;
static const char *cInspOk=__("Ok");
static const char *cInspWait=__("Can't update");
static const char *cInspNoScope=__("Out of scope");

static void OpenInspector(const char *var);
static TDialog *createEditExp(char *tit);
static void ShowErrorInMsgBox();
static void SaveExpandedRect(opstream &os, TRect &r, unsigned wS, unsigned hS);
static void ReadExpandedRect(ipstream &is, TRect &r, unsigned wS, unsigned hS);
static int  IsDisAsmWinCurrent();

static
int IsEmpty(const char *s)
{
 for (;*s && ucisspace(*s); s++);
 return *s==0;
}

static inline
unsigned GetGDBTimeOut()
{
 return EnvirGetIntVar("SET_GDB_TIME_OUT",defaultTimeOut);
}

static inline
void SetGDBTimeOut(unsigned val)
{
 EnvirSetIntVar("SET_GDB_TIME_OUT",val);
}

static inline
unsigned GetMsgLines()
{
 return EnvirGetIntVar("SET_GDB_MSG_LINES",defaultLinesMsg);
}

static inline
void SetMsgLines(unsigned val)
{
 EnvirSetIntVar("SET_GDB_MSG_LINES",val);
 msgsInDebugWindow=val;
}

static inline
unsigned GetGDBMisc()
{
 return EnvirGetIntVar("SET_GDB_MISC",0);
}

static inline
void SetGDBMisc(unsigned val)
{
 EnvirSetIntVar("SET_GDB_MISC",val);
 // It shouldn't be needed because the dialog is enabled only when dbg is NULL
 // but in this way we are ready for future changes.
 if (dbg)
   {
    if (val & miscMIv2)
       dbg->ForceMIVersion(2,0,0);
    else
       dbg->ForceMIVersion(1,0,0);
   }
 mi_set_workaround(MI_PSYM_SEARCH,val & miscSymWA ? 0 : 1);
}

static inline
const char *GetGDBExe()
{
 return GetVariable("SET_GDB_EXE",MIDebugger::GetGDBExe());
}

static inline
const char *GetGDBExeNoD()
{
 return GetVariable("SET_GDB_EXE",NULL);
}

static inline
void SetGDBExe(const char *name)
{
 InsertEnvironmentVar("SET_GDB_EXE",IsEmpty(name) ? NULL: name);
}

static inline
const char *GetXTermExe()
{
 return GetVariable("SET_XTERM_EXE",MIDebugger::GetXTermExe());
}

static inline
const char *GetXTermExeNoD()
{
 return GetVariable("SET_XTERM_EXE",NULL);
}

static inline
void SetXTermExe(const char *name)
{
 InsertEnvironmentVar("SET_XTERM_EXE",IsEmpty(name) ? NULL: name);
}

static inline
const char *GetMainFunc()
{
 if (mainFunction)
    return mainFunction;
 return MIDebugger::GetMainFunc();
}

static inline
void SetMainFunc(const char *mf, Boolean copy)
{
 delete[] mainFunction;
 mainFunction=copy ? newStr(mf) : (char *)mf;
 return MIDebugger::SetMainFunc(mf);
}

static inline
int IsStopped()
{
 return dbg && dbg->GetState()==MIDebugger::stopped;
}

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
 if (!s)
    return NULL;
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
 // Nope, so we must do a deep search
 char *fullName, *retName;
 if (FindFile(s,fullName,binReference))
   {
    dbgPr("Adding %s to cache\n",fullName);
    sourcesCache->insert(s,fullName,retName);
    delete[] fullName;
    return retName;
   }
 // Ask the user what to do
 if (messageBox(mfWarning|mfYesButton|mfNoButton,
                __("Do you want to provide the path for: '%s'."),s)==cmYes)
   {
    char buf[PATH_MAX];

    // The file with * appended
    const char *start=strrchr(s,'/');
    if (start)
       start++;
    else
       start=s;
    CLY_snprintf(buf,PATH_MAX,"%s*",start);

    // Bring a dialog to select it
    if (GenericFileDialog(__("Source file"),buf,NULL,hID_DbgSourceLoc,
        fdNoMask)!=cmCancel)
      {
       if (!edTestForFile(buf))
          // Failed to stat or not regular file
          messageBox(mfError|mfOKButton,__("Can't use: '%s'."),s);
       else
         {// Add to cache
          sourcesCache->insert(s,buf,retName);
          // May be also to the list of paths
          char *last=strrchr(buf,'/');
          if (last && last!=buf)
            {
             *last=0;
             if (messageBox(mfConfirmation|mfYesButton|mfNoButton,
                 __("Add path to list: '%s'?"),buf)==cmYes)
               {
                PathListAdd(paliSource,buf);
                if (dbg)
                   dbg->PathSources(buf);
               }
            }
          return retName;
         }
      }
   }
 else
    // The user is lazy or doesn't have the source code, don't bother again
    sourcesCache->insert(s,NULL,retName);
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
 TVIntl::freeSt(icInspOk);
 TVIntl::freeSt(icInspWait);
 TVIntl::freeSt(icInspNoScope);
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
 // TInspectors needs it, maybe others in the future.
 message(TProgram::deskTop,evBroadcast,cmDbgChgState,NULL);
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
  That's a callback called when a gdb response exceeds the time out. The
dialog asks the user if s/he wants to wait more time or just take it as an
error.
  
  Return: !=0 continue waiting.
  
***************************************************************************/

int TSetEditorApp::DebugTimeOut(void *)
{
 return messageBox(__("Time out in gdb response. Continue waiting?"),
                   mfError|mfYesButton|mfNoButton)==cmYes;
}

/**[txh]********************************************************************

  Description:
  Tries to connect to gdb. It doesn't check if we are initialized or in
which state is the debugger. Call it only after checking.
  
  Return: !=0 OK
  
***************************************************************************/

int TSetEditorApp::DebugConnect()
{
 unsigned misc=GetGDBMisc();
 mi_set_workaround(MI_PSYM_SEARCH,misc & miscSymWA ? 0 : 1);
 const char *aux=GetGDBExeNoD();
 if (aux)
    dbg->SetGDBExe(aux);
 aux=GetXTermExeNoD();
 if (aux)
    dbg->SetXTermExe(aux);
 int res=dbg->Connect();
 if (res)
   {
    DebugMsgClear();
    DBG_SetCallBacks();
    dbg->SetTimeOut(GetGDBTimeOut());
    dbg->SetTimeOutCB(DebugTimeOut,NULL);
    DebugMsgSetState();
    if (!(misc & miscNoBanner) && !dbg->Version())
       return 0;
    if (misc & miscMIv2)
       dbg->ForceMIVersion(2,0,0);
    // Set the path for sources
    char n[PATH_MAX];
    if (dbg->PathSources(NULL))
      {
       for (int i=0; PathListGetItem(i,n,paliSource); i++)
           if (!dbg->PathSources(n))
              break;
       // At the end so it becomes the fist in the list:
       // Yet another gdb bug?? some gdbs need it
       if (getcwd(n,PATH_MAX))
          dbg->PathSources(n);
      }
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

static
const char *getCurTTY()
{
 const char *tty=NULL;
 if (dOps.tty[0])
    tty=dOps.tty;
 if (!tty)
    tty=dbg->GetAuxTTY();
 if (!tty)
    tty=TVIntl::getText(__("default"),icDefault);
 return tty;
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
            if (res)
               GiveAdvice(gadvDbgLinuxTTY);
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
         TSetEditorApp::setCmdState(cmeDbgOptionsAdv,False);
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
         TSetEditorApp::setCmdState(cmeDbgInspector,True);
         TSetEditorApp::setCmdState(cmeDbgDataWindow,True);
         TSetEditorApp::setCmdState(cmeDbgStackWindow,True);
         TSetEditorApp::setCmdState(cmeDbgDisAsmWin,True);
         TSetEditorApp::setCmdState(cmeDbgThreadSel,True);
         TSetEditorApp::setCmdState(cmeDbgDetach,Boolean(dOps.mode!=dmLocal));
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
         TSetEditorApp::setCmdState(cmeDbgCleanElem,True);
         if (st!=MIDebugger::stopped)
           {
            TSetEditorApp::setCmdState(cmeDbgKill,False);
            TSetEditorApp::setCmdState(cmeDbgCallStack,False);
            TSetEditorApp::setCmdState(cmeDbgInspector,False);
            TSetEditorApp::setCmdState(cmeDbgDataWindow,False);
            TSetEditorApp::setCmdState(cmeDbgStackWindow,False);
            TSetEditorApp::setCmdState(cmeDbgDetach,False);
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
         TSetEditorApp::setCmdState(cmeDbgCleanElem,False);
         TSetEditorApp::setCmdState(cmeDbgDisAsmWin,False);
         TSetEditorApp::setCmdState(cmeDbgThreadSel,False);
         TSetEditorApp::setCmdState(cmeDbgDetach,False);
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
 TView::curCommandSet-=cmeDbgCleanElem;
 TView::curCommandSet-=cmeDbgDisAsmWin;
 TView::curCommandSet-=cmeDbgThreadSel;
 TView::curCommandSet-=cmeDbgDetach;
}

/**[txh]********************************************************************

  Description:
  Starts running the program or resumes execution. It tries to be smart.  
  
***************************************************************************/

void TSetEditorApp::DebugRunOrContinue()
{
 int wasStopped=IsStopped();
 if (!DebugCheckStopped())
    return;
 DBG_RefreshIgnoreBkpts();
 if (!wasStopped && dOps.mode==dmPID)
   {// Just attach if that's the first run
    DebugMsgSetState();
    return;
   }
 if (dbg->RunOrContinue())
    DebugMsgSetState();
}

/**[txh]********************************************************************

  Description:
  Used to detach from a remote or pid target.
  
***************************************************************************/

void TSetEditorApp::DebugDetach()
{
 if (dOps.mode==dmLocal || !IsStopped())
    return;
 if (dbg->TargetUnselect())
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

 TDisAsmWin::updateCodeInfo();

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
 if (dbg->StepOver(IsDisAsmWinCurrent()))
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
 if (dbg->TraceInto(IsDisAsmWinCurrent()))
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
 if (!DebugCheckStopped())
    return;

 int res;
 if (IsDisAsmWinCurrent())
   {
    res=TDisAsmWin::runToCursor();
   }   
 else
   {
    TCEditor *e=GetCurrentIfEditor();
    if (!e)
       return;
   
    // We use the full name
    char s[PATH_MAX];
    strcpy(s,e->fileName);
    CLY_fexpand(s);

    res=dbg->GoTo(s,e->curPos.y+1);
   }

 if (res)
    DebugMsgSetState();
 else
    DebugMsgSetError();
}

/**[txh]********************************************************************

  Description:
  Executes until the end of the current function. Not smart, only works for
stopped state.
  
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
 int cnt;
 DynStrCatStruct msg;
 fI.Column=1;
 fI.offset=-1;
 fI.len=0;

 for (cnt=0, r=f; r; r=r->next, cnt++)
    {
     options=0;
     if (r==f) // First
        options|=edsmRemoveOld;
     if (r->next)
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
     EdShowMessageFile(msg.str,fI,SolveFileName(r->file),options);
     free(msg.str);
    }
 mi_free_frames(f);
 if (cnt)
   {
    // Important note: we can't play with the edsmUpdateSpLines flag because
    // not all splines will belong to the same file.
    SpLinesUpdate();
    EdJumpToFirstError();
   }
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

struct evalBox
{
 char exp[widthFiles];
 char res[widthExpRes];
 char val[widthShort];
};


class TDbgEvalModify : public TDialog
{
public:
 TDbgEvalModify(evalBox *aBox) :
   TWindowInit(&TDbgEvalModify::initFrame),
   TDialog(TRect(1,1,1,1),__("Evaluate and Modify")) { box=aBox; };
 virtual void handleEvent(TEvent &);

 TInputLinePiped *exp;
 TInputLinePiped *res;
 TInputLine *val;

 evalBox *box;
};

/**[txh]********************************************************************

  Description:
  Command loop for the Debug Evaluate & Modify dialog. It calls
DebugEvalExpression and DebugModifyExpression and redirects copy/paste
actions to the correct input line.
  
***************************************************************************/

void TDbgEvalModify::handleEvent(TEvent &event)
{
 char *ret;

 TDialog::handleEvent(event);
 if (event.what==evCommand)
    switch (event.message.command)
      {
       case cmEval:
             exp->getData(box->exp);
             ret=TSetEditorApp::DebugEvalExpression(box->exp);
             if (ret)
               {
                strncpyZ(box->res,ret,widthExpRes);
                res->setData(box->res);
                free(ret);
                exp->selectAll(True);
               }
            clearEvent(event);
            break;
       case cmChange:
             exp->getData(box->exp);
             val->getData(box->val);
             ret=TSetEditorApp::DebugModifyExpression(box->exp,box->val);
             if (ret)
               {
                strncpyZ(box->res,ret,widthExpRes);
                res->setData(box->res);
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
       case cmDbgInspect:
            endModal(cmDbgInspect);
            clearEvent(event);
            break;
      }
}

/**[txh]********************************************************************

  Description:
  Creates the debugger eval/modify dialog.
  
  Return: TDialog pointer to the new dialog.
  
***************************************************************************/

TDbgEvalModify *createEvalModifyDialog(evalBox *box)
{
 TDbgEvalModify *d=new TDbgEvalModify(box);
 TSViewCol *col=new TSViewCol(d);

 TSInputLinePiped *sExp=
   new TSInputLinePiped(widthFiles,1,hID_DbgEvalModifyExp,maxWBox);
 TSInputLinePiped *sRes=
   new TSInputLinePiped(widthExpRes,0,0,maxWBox,tilpNoPipe | tilpNoPaste);
 TSInputLine *sVal=new TSInputLine(widthShort,1,hID_DbgEvalModifyNewV,maxWBox);
 d->exp=(TInputLinePiped *)sExp->view;
 d->res=(TInputLinePiped *)sRes->view;
 d->val=(TInputLine *)sVal->view;
 TSetEditorApp::setCmdState(cmDbgInspect,dbg->GetState()==MIDebugger::stopped ?
                            True : False);

 // EN: CEHIPRNV
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
             new TSButton(__("~I~nspect"),cmDbgInspect),
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
 evalBox box;
 memset(&box,0,sizeof(box));
 if (startVal)
   {
    strncpyZ(box.exp,startVal,widthFiles);
    delete[] startVal;
   }

 TDbgEvalModify *d=createEvalModifyDialog(&box);
 if (execDialog(d,&box)==cmDbgInspect)
   {
    if (IsEmpty(box.exp))
       messageBox(__("Nothing to inspect, please provide an expression"),
                  mfError | mfOKButton);
    else
       OpenInspector(box.exp);
   }
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
 MIDebugger::eState st=dbg->GetState();
 if (st!=MIDebugger::stopped && st!=MIDebugger::running)
    return 1; // We are just connected
 return GiveAdvice(directRequest ? gadvDbgDestSes : gadvDbgSesActive)==cmYes;
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
       if (confirm && GiveAdvice(gadvDbgKillPrg)!=cmYes)
          return False;
      }
    delete dbg; // It will close the debug session
    dbg=NULL;
   }
 TProgram::deskTop->lock();
 DebugMsgSetState();
 DebugMsgClose();
 WatchesClose();
 DebugCommonCleanUp();
 TProgram::deskTop->unlock();

 return True;
}

// Small class to display frames
class TFramesList : public TStringable
{
public:
 TFramesList(mi_frames *theFrames);
 ~TFramesList();

 virtual void getText(char *dest, unsigned item, int maxLen);
 mi_frames *getItem(int index);

protected:
 mi_frames *frames;
};

TFramesList::~TFramesList()
{
 mi_free_frames(frames);
}

TFramesList::TFramesList(mi_frames *theFrames)
{
 frames=theFrames;
 Count=0;
 while (theFrames)
   {
    Count++;
    theFrames=theFrames->next;
   }
}

mi_frames *TFramesList::getItem(int num)
{
 mi_frames *p=frames;
 while (num && p)
   {
    num--;
    p=p->next;
   }
 return num ? NULL : p;
}

void TFramesList::getText(char *dest, unsigned item, int maxLen)
{
 mi_frames *r=getItem(item);
 if (!r)
   {
    *dest=0;
    return;
   }
 const char *unknown=TVIntl::getText(__("unknown"),icUnknown);
 if (r->addr)
    TVIntl::snprintf(dest,maxLen,__("%d: %s:%s:%d addr %p"),r->thread_id,
                     r->func ? r->func : unknown,
                     r->file ? r->file : unknown,
                     r->line,r->addr);
 else
    TVIntl::snprintf(dest,maxLen,"%d: %s:%s:%d",r->thread_id,
                     r->func ? r->func : unknown,
                     r->file ? r->file : unknown,
                     r->line);
}

void TSetEditorApp::DebugThreadSel()
{
 if (!IsStopped())
    return;
 // Find the possible ids
 int *list=NULL;
 int cant=dbg->ThreadListIDs(list);
 if (cant<=1)
   {
    free(list);
    messageBox(__("No threads to select"),mfError|mfOKButton);
    return;
   }
 // Get info about them
 mi_frames *f=dbg->ThreadList();
 TFramesList *fl=new TFramesList(f);
 if (fl->GetCount()!=(unsigned)cant)
   {
    delete fl;
    messageBox(__("Inconsistent info from gdb"),mfError|mfOKButton);
    return;
   }
 // Transfer the ids
 mi_frames *c=f;
 for (int i=0; i<cant; i++)
    {
     c->thread_id=list[i];
     c=c->next;
    }
 // Create a dialog to select them
 TSViewCol *col=new TSViewCol(__("Select Thread"));

 int h=cant, hmax=GetDeskTopRows()-6, ops=0;
 if (h>hmax)
   {
    h=hmax;
    ops=tsslbVertical;
   }
 col->insert(xTSCenter,yTSUp,new TSLabel(__("ID"),
             new TSStringableListBox(GetDeskTopCols()-6,h,ops)));
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(cmeDbgThreadSel);
 delete col;
 TStringableListBoxRec box;
 box.items=fl;
 box.selection=0;
 // Get the current id from the stopped info
 if (stoppedInfo && stoppedInfo->have_thread_id)
    for (int i=0; i<cant; i++)
        if (list[i]==stoppedInfo->thread_id)
          {
           box.selection=i;
           break;
          }
 // Execute the dialog
 if (execDialog(d,&box)==cmOK)
   {
    mi_frames *frame=fl->getItem(box.selection);
    frame=dbg->ThreadSelect(frame->thread_id);
    if (frame)
      {
       DebugMsgJumpToFrame(frame,NULL,0);
       mi_free_frames(frame);
      }
    else
       ShowErrorInMsgBox();
   }
 delete fl;
 free(list);
}

//---------------------------------------------------------------------------

const int disAsmAfterPC=500;
const ulong disAsmMaxDist=20000;

// Collection to convert an address into a line number inside the assembler text
Boolean TAdd2Line::searchL(int line, ccIndex &pos)
{
 byLine=1;
 Boolean ret=search((void *)(long)line,pos);
 byLine=0;
 return ret;
}

void *TAdd2Line::keyOf(void *item)
{
 stAdd2Line *p=(stAdd2Line *)item;
 return byLine ? (void *)(long)p->line : p->addr;
}

int TAdd2Line::compare(void *s1, void *s2)
{
 return (char *)s1-(char *)s2;
}

void TAdd2Line::freeItem(void *s)
{
 delete (stAdd2Line *)s;
}

void TAdd2Line::insert(void *addr, int line, mi_asm_insns *sl, mi_asm_insn *al)
{
 stAdd2Line *p=new stAdd2Line;
 p->addr=addr;
 p->line=line;
 p->sourceL=sl;
 p->asmL=al;
 TNSSortedCollection::insert(p);
}

TDisAsmEdWin::TDisAsmEdWin(const TRect &aR) :
    TWindowInit(TDisAsmEdWin::initFrame),
    TCEditWindow(aR,NULL,wnNoNumber,False)
{// Can't be: moved, zoomed, resized, etc.
 flags=0;
 // Doesn't have a shadow
 state&=~sfShadow;
 // Usualy windows climbs to the top when selected. But here we are in a dialog
 // and doing this changes the objects order generating a real mess.
 // The net result is that after a couple of selections no TView can be
 // selected.
 options&=~ofTopSelect;
 growMode=gfGrowHiX | gfGrowHiY;
 // Enable some special hacks for this case
 editor->isDisassemblerEditor=1;
 // SPARC code have tabs
 editor->tabSize=8;
 editor->SeeTabs=False;
 // We start without code
 lines=NULL;
 a2l=NULL;
 from=to=NULL;
 spLine=NULL;
 curLine=NULL;

 MIDebugger::archType tp=dbg ? dbg->GetTargetArchitecture() : MIDebugger::arUnknown;
 int shlNum;
 switch (tp)
   {
    case MIDebugger::arSPARC:
         shlNum=SHLNumberOf("SPARC asm");
         break;
    // Use IA32 as default
    case MIDebugger::arIA32:
    default:
         shlNum=SHLNumberOf("80x86 asm (AT&T syntax)");
   }
 editor->SetHighlightTo(shlGenericSyntax,shlNum);    
}

const char *TDisAsmEdWin::getTitle(short)
{
 return __("Code");
}

TPalette &TDisAsmEdWin::getPalette() const
{
 static TPalette
 pal(cpDisAsmEd,sizeof(cpDisAsmEd)-1);

 return pal;
}

TDisAsmEdWin::~TDisAsmEdWin()
{
 mi_free_asm_insns(lines);
 destroy0(a2l);
 destroy0(spLine);
}

char *TDisAsmEdWin::getCodeInfo(char *b, int l)
{// If we failed to get the code just abort
 if (!a2l)
    return NULL;
 b[0]=0;
 int line=editor->curPos.y+1;
 ccIndex pos;
 if (a2l->searchL(line,pos))
   {
    mi_asm_insn *p=a2l->At(pos)->asmL;
    if (p->func)
       CLY_snprintf(b,l,"%p <%s+%d>",p->addr,p->func,p->offset);
    else
       CLY_snprintf(b,l,"%p",p->addr);
   }
 return b;
}

int TDisAsmEdWin::runToCursor()
{
 if (!a2l)
    // Pretend we succeed and avoid farther messages.
    // The user should avoid going nowhere.
    return 1;

 int line=editor->curPos.y+1;
 ccIndex pos;
 void *addr=NULL;
 if (a2l->searchL(line,pos))
   {
    mi_asm_insn *p=a2l->At(pos)->asmL;
    addr=p->addr;
   }
 int res=1;
 if (addr)
    res=dbg->GoTo(addr);
 else
    messageBox(__("No address associated with this line"),mfError|mfOKButton);

 return res;
}

void TDisAsmEdWin::setCode(mi_asm_insns *aLines)
{
 mi_free_asm_insns(lines);
 lines=aLines;
 destroy0(a2l);

 // For the comments
 editor->CacheSyntaxHLData(editor->GenericSHL);

 // Create the new content
 char *curFunc=NULL;
 DynStrCatStruct tx;
 DynStrCatInit(&tx);
 mi_asm_insns *c=lines;
 mi_asm_insn *a;
 // First pass: create a list of jump targets
 TStringCollection *col=new TStringCollection(10,10);
 while (c)
   {
    a=c->ins;
    while (a)
      {
       char *s=a->inst;
       if (s)
         {
          int l=strlen(s);
          if (l && s[l-1]=='>')
            {
             int ls=l-2;
             for (; ls>=0 && s[ls]!='<'; ls--);
             if (s[ls]=='<')
               {
                char *label=newStrL(s+ls+1,l-ls-2);
                col->insert(label);
                dbgPr("Label: <%s>\n",label);
               }
            }
         }
       a=a->next;
      }
    c=c->next;
   }
 // Second pass: generate the text
 c=lines;
 a2l=new TAdd2Line();
 from=to=NULL;
 int line=1;
 // Variables used to obtain source lines
 char *curFile=NULL;
 LineHandler lh;
 unsigned misc=GetGDBMisc();
 while (c)
   {// If source file info is available add it
    if (c->file)
      {// We insert the info commented
       // The comment is architecture dependent
       DynStrCat(&tx,TCEditor::strC.EOLCom1,TCEditor::strC.lEOLCom1);
       DynStrCat(&tx," ",1);
       char added=0;
       if (!(misc & miscNoSourceInAsm))
         {// Try adding the source code
          if (!curFile || strcmp(curFile,c->file))
            {
             curFile=c->file;
             TCEditWindow *edw=NULL;
             char *curSolvedFile=SolveFileName(c->file);
             if (curSolvedFile)
                edw=GetEditorWindowForFile(curSolvedFile);
             lh.setEditor(edw ? edw->editor : NULL);
            }
          unsigned len;
          char *s=lh.getLine(c->line-1,len);
          if (s && len)
            {// Avoid mixing with a DOS EOL
             if (len>1 && s[len-2]=='\r')
               {
                s[len-2]='\n';
                len--;
               }
             DynStrCat(&tx,s,len);
             // Ensure we have an EOL
             if (s[len-1]!='\n')
                DynStrCat(&tx,"\n",1);
             added=1;
            }
         }
       if (!added)
         {// Failed or the user doesn't want source.
          // So put the file and line number.
          DynStrCat(&tx,c->file);
          DynStrCat(&tx,":",1);
          char b[32];
          int l=CLY_snprintf(b,32,"%d\n",c->line);
          DynStrCat(&tx,b,l);
         }
       line++;
      }
    a=c->ins;
    while (a)
      {
       int lf=0;
       if (a->func)
          lf=strlen(a->func);
       // Put a label if we changed function
       if (a->func && (!curFunc || strcmp(curFunc,a->func)))
         {
          DynStrCat(&tx,a->func,lf);
          DynStrCat(&tx,":\n",2);
          curFunc=a->func;
          line++;
         }
       // Put a jump label we detected a jump to this location
       if (a->offset && a->func)
         {
          AllocLocalStr(b,lf+32);
          int l=CLY_snprintf(b,lf+32,"%s+%d",a->func,a->offset);
          ccIndex pos;
          if (col->search(b,pos))
            {
             DynStrCat(&tx,b,l);
             DynStrCat(&tx,":\n",2);
             line++;
            }
         }
       // Add the assembler code
       DynStrCat(&tx,"    ",4);
       if (a->inst)
         {
          DynStrCat(&tx,a->inst);
          a2l->insert(a->addr,line,c,a);
         }
       DynStrCat(&tx,"\n",1);
       line++;
       if (!from)
          to=from=a->addr;
       else
          to=a->addr;
       a=a->next;
      }
    c=c->next;
   }
 destroy(col);
 editor->lock();
 // Clear the current content
 if (editor->bufLen)
    editor->setBufLen(0);
 // Insert the new content
 editor->isReadOnly=False;
 editor->insertText(tx.str,tx.len,False);
 editor->isReadOnly=True;
 // Avoid the question about saving it ;-)
 editor->modified=False;
 editor->unlock();
 free(tx.str);
}

int TDisAsmEdWin::dissasembleFrame(mi_frames *f)
{
 mi_asm_insns *r;
 // Disassemble according to the info we have
 if (f->file)
   {// We have file & line. We can ask for the whole function.
    r=dbg->Disassemble(f->file,f->line,-1,MI_DIS_SRC_ASM);
   }
 else
   {// We don't know the source file so it doesn't have debug info.
    if (!f->addr)
       // Hope never happend
       return 0;

    char endB[32];
    char startB[32];
    ulong start=0;
    // Disassemble upto addr+disAsmAfterPC
    ulong end=(ulong)((char *)f->addr+disAsmAfterPC);

    // Solve the starting point
    if (f->func)
      {// We know the function name and will try to get from the beggining
       // Find if the function is valid
       unsigned len=strlen(f->func)+20;
       AllocLocalStr(b,len);
       CLY_snprintf(b,len,"(unsigned long)%s",f->func);
       char *res=TSetEditorApp::DebugEvalExpression(b);
       if (res && MIDebugger::GetErrorNumber()==MI_OK)
         {
          start=atoi(res);
          // Put some limits, some times gdb reports a wrong function
          if (start>end || end-start>disAsmMaxDist)
             start=0;
         }
       free(res);
      }
    if (!start)
       start=(ulong)f->addr;

    CLY_snprintf(startB,32,"0x%lx",start);
    CLY_snprintf(endB,32,"0x%lx",end);
    r=dbg->Disassemble(startB,endB,MI_DIS_ASM);
   }
 if (!r)
   {
    ShowErrorInMsgBox();
    return 0;
   }

 if (0)
   {// Debug code
    mi_asm_insns *c=r;
    mi_asm_insn *a;
    while (c)
      {
       if (c->file)
          printf("Source: %s:%d\n",c->file,c->line);
       a=c->ins;
       while (a)
         {
          printf("%p: %s [%s+%d]\n",a->addr,a->inst,a->func,a->offset);
          a=a->next;
         }
       c=c->next;
      }
    mi_free_asm_insns(r);
   }

 setCode(r);
 return 1;
}

int TDisAsmEdWin::jumpToFrame(mi_frames *f)
{
 Boolean newCode=False;
 curLine=NULL;
 // Make sure we have the address in our range
 if (f->addr<from || f->addr>to)
   {
    if (!dissasembleFrame(f))
       return 0;
    newCode=True;
   }
 // Get the line for this address
 ccIndex pos;
 Boolean res=a2l->search(f->addr,pos);
 if (!res)
    printf("Oops! no line\n");
 if (pos<0 || pos>=a2l->getCount())
    return 0;
 stAdd2Line *a=a2l->At(pos);
 curLine=a->sourceL;
 editor->lock();
 editor->GoAndSelectLine(a->line,False);
 editor->trackCursor(newCode);
 // The CPU line
 TSpCollection *nSpLine=new TSpCollection(1);
 nSpLine->insert(a->line-1,idsplRunLine);
 editor->SetSpecialLines(nSpLine);
 destroy(spLine);
 spLine=nSpLine;
 // Unlock
 editor->unlock();
 return 1;
}

TRegisters::TRegisters(mi_chg_reg *aRegs, int cRegs)
{
 regs=aRegs;
 Count=cRegs;
}

TRegisters::~TRegisters()
{
 mi_free_chg_reg(regs);
}

void TRegisters::getText(char *dest, unsigned item, int maxLen)
{
 mi_chg_reg *p=getItem(item);
 CLY_snprintf(dest,maxLen,"%c %s=%s",p->updated ? '*' : ' ',p->name,p->val);
}

mi_chg_reg *TRegisters::getItem(int num)
{
 mi_chg_reg *p=regs;
 while (num && p)
   {
    num--;
    p=p->next;
   }
 return num ? NULL : p;
}

int TRegisters::update()
{
 return dbg->UpdateRegisters(regs);
}

TDisAsmWin::TDisAsmWin(const TRect &aR) :
    TWindowInit(TDisAsmWin::initFrame),
    TDialog(aR,__("Disassembler Window"))
{
 flags=wfMove | wfClose | wfGrow | wfZoom;
 //growMode=gfGrowLoY | gfGrowHiX | gfGrowHiY;

 // Code
 TRect r=getExtent();
 int rbx=r.b.x;
 r.a.x++; r.a.y++; r.b.y-=2; r.b.x/=2;
 edw=new TDisAsmEdWin(r);
 // An editor inside a dialog!!
 // That's the meaning of code reuse ;-)
 insert(edw);

 // Code Info
 char b[81];
 memset(b,' ',80);
 b[80]=0;
 int ray=r.a.y;
 r.a.y=r.b.y; r.b.y=r.a.y+1;
 codeInfo=new TNoStaticText(r,b);
 codeInfo->growMode=gfGrowHiX | gfGrowHiY | gfGrowLoY;
 insert(codeInfo);
 codeInfoLine=-1;

 int cRegs;
 mi_chg_reg *rg=dbg->GetRegisterValues(&cRegs);
 if (rg && !dbg->GetRegisterNames(rg))
   {
    mi_free_chg_reg(rg);
    rg=NULL;
   }
 if (rg)
   {
    regs=new TRegisters(rg,cRegs);
    // Registers
    r.a.x=r.b.x; r.b.x=rbx-2; r.a.y=ray; r.b.y--;
    TScrollBar *vScrollBar=new TScrollBar(TRect(r.b.x,1,r.b.x+1,r.b.y));
    TScrollBar *hScrollBar=new TScrollBar(TRect(r.a.x,r.b.y,r.b.x,r.b.y+1));
    hScrollBar->growMode=gfGrowAll;
    hScrollBar->setRange(0,100);
    bRegs=new TStringableListBox(r,1,hScrollBar,vScrollBar);
    bRegs->newList(regs);
    bRegs->growMode=gfGrowLoX | gfGrowHiX | gfGrowHiY;
    insert(vScrollBar);
    insert(hScrollBar);
    insert(bRegs);
   }
 else
   {
    regs=NULL;
    bRegs=NULL;
    ShowErrorInMsgBox();
   }

 // Select the code window
 selectNext(True);
 theDisAsmWin=this;
 helpCtx=hcDisassembler;
}

TDisAsmWin::~TDisAsmWin()
{
 delete regs;
}

static
int DebugEvalExpressionNoRet(char *exp)
{
 free(TSetEditorApp::DebugEvalExpression(exp));
 if (MIDebugger::GetErrorNumber()!=MI_OK)
   {
    ShowErrorInMsgBox();
    return 0;
   }
 return 1;
}

void TDisAsmWin::handleEvent(TEvent &event)
{
 TDialog::handleEvent(event);
 if (bRegs &&
     ((event.what==evBroadcast && event.message.command==cmListItemSelected) ||
     (event.what==evCommand && event.message.command==cmModifyReg)))
   {
    char b[widthWtExp];
    mi_chg_reg *r=regs->getItem(bRegs->focused);
    if (r->val)
       strncpyZ(b,r->val,widthWtExp);
    else
       b[0]=0;
    if (r->name && execDialog(createEditExp(__("Modify register")),b)==cmOK)
      {
       char e[widthWtExp];
       CLY_snprintf(e,widthWtExp,"$%s=%s",r->name,b);
       if (DebugEvalExpressionNoRet(e))
         {
          regs->update();
          bRegs->drawView();
         }
      }
    clearEvent(event);
   }
}

int IsDisAsmWinCurrent()
{
 return TDisAsmWin::isDisAsmWinCurrent();
}

int IsDisAsmWinAvailable()
{
 return TDisAsmWin::isDisAsmWinAvailable();
}

void TDisAsmWin::close(void)
{
 theDisAsmWin=NULL;
 TDialog::close();
}

TPalette &TDisAsmWin::getPalette() const
{
 static TPalette
 pal(cpDisAsmWin,sizeof(cpDisAsmWin)-1);

 return pal;
}

void TDisAsmWin::upCodeInfo()
{
 int curL=edw->editor->curPos.y;
 if (codeInfoLine!=curL)
   {
    codeInfoLine=curL;
    char b[80];
    char *res=edw->getCodeInfo(b,80);
    if (res)
       codeInfo->setText(res);
   }
}

class TDskDbgDisasm : public TDskWin
{
public:
 TDskDbgDisasm(TView *w);

 char *GetText(char *dest, short maxLen);
};

TDskDbgDisasm::TDskDbgDisasm(TView *w)
{
 view=w;
 type=dktDbgDisasm;
 CanBeDeletedFromDisk=0;
 CanBeSaved=0;
 ZOrder=-1;
}

char *TDskDbgDisasm::GetText(char *dest, short maxLen)
{
 TVIntl::snprintf(dest,maxLen,__("   Disassembler [%p-%p]"),
                  TDisAsmWin::getFrom(),
                  TDisAsmWin::getTo());
 return dest;
}

static
void DisAsmWin()
{
 if (TDisAsmWin::windowCreated())
   {
    TDisAsmWin::beSelected();
    return;
   }
 TRect re=GetDeskTopSize();
 re.grow(-4,-2);
 AddAndInsertDskWin(new TDskDbgDisasm(new TDisAsmWin(re)));
}

void TSetEditorApp::DebugDisAsmWin()
{
 if (!IsStopped() || !stoppedInfo || !stoppedInfo->frame)
    return;
 DisAsmWin();
 TDisAsmWin::jumpToFrame(stoppedInfo->frame);
}

/*****************************************************************************
  End of Editor debug commands
*****************************************************************************/

/*****************************************************************************
  TInspector class and functionality
*****************************************************************************/

class TPVarTree : public TStringable
{
public:
 TPVarTree(const char *var, Boolean fake=False);
 ~TPVarTree();

 virtual void getText(char *dest, unsigned item, int maxLen);

 mi_gvar *getItem(int index);
 char *getExpression(int index);

 void collapse(mi_gvar *p);
 void expand(mi_gvar *p);
 int countVisChildren(mi_gvar *p);
 const char *mainVarName() { return var->name; }
 const char *mainVarExp() { return var->exp; }
 int markChanged(const char *name, int depth);
 int recycle();
 char *createTreeState(mi_gvar *p);
 char *createTreeState() { return createTreeState(var); }
 int applyTreeState(mi_gvar *p, const char *st);
 int applyTreeState(const char *st) { return applyTreeState(var,st); };

 int isOK() { return ok; }
 void disable() { outOfScope=1; }
 void undefine();
 Boolean canBeDestroyed();

protected:
 mi_gvar *var;
 int ok;
 int outOfScope;
 char *exp;

 mi_gvar *init(const char *anExp);
};

Boolean TPVarTree::canBeDestroyed()
{
 if (!dbg || outOfScope)
    return True;
 // We can't delete the variable while running
 return dbg->GetState()==MIDebugger::running ? False : True;
}

TPVarTree::~TPVarTree()
{
 if (var)
   {
    if (dbg && !outOfScope)
       dbg->DelgVar(var);
    mi_free_gvar(var);
   }
 delete[] exp;
}

TPVarTree::TPVarTree(const char *anExp, Boolean fake) :
   TStringable()
{
 exp=NULL;
 if (fake)
   {
    var=mi_alloc_gvar();
    var->name=strdup(anExp);
    outOfScope=1;
    Count=1;
    ok=1;
   }
 else
   {
    ok=0;
    Count=0;
    outOfScope=0;
    var=NULL;
    var=init(anExp);
   }
}

const int tstOpened=1, tstHaveChild=2, tstLast=4,
          tstBase=64; // Just to make them ASCII

char *TPVarTree::createTreeState(mi_gvar *p)
{
 int l=1+p->vischild+1, i=0;
 char *r=new char[l];
 r[l-1]=0;

 while (i<l && p)
   {
    r[i]=tstBase | (((unsigned)p->format)<<3);
    if (p->numchild)
       r[i]|=tstHaveChild;
    if (!p->next)
       r[i]|=tstLast;
    if (p->child && p->opened)
      {
       r[i]|=tstOpened;
       p=p->child;
      }
    else if (p->next)
      {
       p=p->next;
      }
    else
      {
       while (p && !p->next)
          p=p->parent;
       if (p)
          p=p->next;
      }
    i++;
   }
 return r;
}

int TPVarTree::applyTreeState(mi_gvar *p, const char *st)
{
 enum mi_gvar_fmt fmt;
 while (*st && p)
   {
    if ((*st & tstHaveChild) && !p->numchild)
       return 0;
    if ((*st & tstOpened) && !p->opened)
       expand(p);
    fmt=(enum mi_gvar_fmt)((*st>>3) & 7);
    if (fmt!=p->format)
      {
       dbg->SetFormatgVar(p,fmt);
       if (p->value)
          p->changed=1;
      }
    if (p->child)
      {
       p=p->child;
      }
    else if (p->next)
      {
       p=p->next;
      }
    else
      {
       while (p && !p->next)
          p=p->parent;
       if (p)
          p=p->next;
      }
    st++;
   }
 return !*st && !p;
}

void TPVarTree::undefine()
{
 if (dbg && !outOfScope)
    dbg->DelgVar(var);
 disable();
}

mi_gvar *TPVarTree::init(const char *anExp)
{
 mi_gvar *v=NULL;
 if (dbg && dbg->GetState()==MIDebugger::stopped)
   {
    v=dbg->AddgVar(anExp);
    if (v && dbg->EvalgVar(v))
      {
       if (v->numchild)
          ok=dbg->GetChildgVar(v) && dbg->FillTypeVal(v->child);
       else
          ok=1;
       if (!exp)
          exp=newStr(anExp);
       Count=1+v->numchild;
      }
   }
 return v;
}

int TPVarTree::recycle()
{
 if (!outOfScope  || !dbg || dbg->GetState()!=MIDebugger::stopped)
    return 0;
 mi_gvar *v=init(exp);
 if (v)
   {// Find which ones are expanded
    char *st=createTreeState(var);
    mi_free_gvar(var);
    var=v;
    outOfScope=0;
    // Try to expand the same vars
    applyTreeState(var,st);
    delete[] st;
    return 1;
   }
 return 0;
}

int TPVarTree::markChanged(const char *name, int depth)
{
 mi_gvar *p=var;
 int curDepth=0;

 while (p)
   {
    if (curDepth==depth && strcmp(p->name,name)==0)
      {
       p->changed=1;
       dbgPr("Marked %s as changed\n",p->exp);
       return 1;
      }
    if (p->child)
      {
       p=p->child;
       curDepth++;
      }
    else if (p->next)
      {
       p=p->next;
      }
    else
      {
       while (p && !p->next)
         {
          p=p->parent;
          curDepth--;
         }
       if (p)
          p=p->next;
      }
   }
 return 0;
}

mi_gvar *TPVarTree::getItem(int index)
{
 mi_gvar *p=var;
 for (int i=0; p && i<index; )
    {
     int have=1+p->vischild;
     if (i+have>index)
       {// Go deeper
        p=p->child;
        i++;
       }
     else
       {// Next in the list
        p=p->next;
        i+=have;
       }
    }
 return p;
}

char *TPVarTree::getExpression(int index)
{
 DynStrCatStruct st;

 DynStrCatInit(&st,"*(",2);
 mi_gvar *p=var;
 int first=1;
 for (int i=0; p && i<index; )
    {
     int have=1+p->vischild;
     if (i+have>index)
       {// Go deeper
        // Classes uses <class> to separate members
        if (p->exp && p->type && p->type[0])
          {
           if (first)
              DynStrCat(&st,"(",1);
           DynStrCat(&st,p->exp);
           if (first)
             {
              DynStrCat(&st,")",1);
              first=0;
             }
           DynStrCat(&st,".",1);
          }
        p=p->child;
        i++;
       }
     else
       {// Next in the list
        p=p->next;
        i+=have;
       }
    }
 if (!p)
   {
    free(st.str);
    return NULL;
   }
 DynStrCat(&st,p->exp);
 DynStrCat(&st,")",1);
 return st.str;
}

void TPVarTree::getText(char *dest, unsigned item, int maxLen)
{
 mi_gvar *p=getItem(item);

 // Indent
 int spaces=p->depth*2;
 if (spaces>=maxLen)
    spaces=maxLen-1;
 if (spaces)
   {
    memset(dest,' ',spaces);
    dest+=spaces;
    maxLen-=spaces;
    if (maxLen==1)
      {
       *dest=0;
       return;
      }
   }

 if (p->numchild)
   {
    int size=CLY_snprintf(dest,maxLen,"%c ",p->opened ? '-' : '+');
    dest+=size;
    maxLen-=size;
   }

 // The following operations needs gdb in stopped state.
 if (!outOfScope && dbg && dbg->GetState()==MIDebugger::stopped)
   {
    if (!p->numchild && p->attr==MI_ATTR_DONT_KNOW)
       dbg->FillAttr(p);
    if (p->changed)
      {
       p->changed=0;
       ::free(p->value);
       p->value=NULL;
       dbgPr("Forcing update of %s\n",p->name);
      }
    if (!p->type || !p->value)
       dbg->FillOneTypeVal(p);
   }

 if ((!p->type || *p->type==0) && (!p->value || *p->value==0))
    CLY_snprintf(dest,maxLen,"%s",p->name);
 else
    CLY_snprintf(dest,maxLen,"%s [%s]=%s",p->exp,p->type,p->value);
}

void TPVarTree::collapse(mi_gvar *p)
{
 p->opened=0;
 int reduce=p->vischild;
 mi_gvar *r=p->parent;
 p->vischild=0;
 while (r)
   {
    r->vischild-=reduce;
    r=r->parent;
   }
 Count=1+var->vischild;
}

int TPVarTree::countVisChildren(mi_gvar *p)
{
 mi_gvar *n=p->child;
 int res=0;
 while (n)
   {
    res+=1+n->vischild;
    n=n->next;
   }
 return res;
}

void TPVarTree::expand(mi_gvar *p)
{
 if (p->numchild && !p->child)
   {// First time we expand it
    if (outOfScope || !dbg || dbg->GetState()!=MIDebugger::stopped)
       // We will hang if running and get nothing for other states.
       return;
    dbg->GetChildgVar(p);
    // That's too slow :-(. We just update what's needed.
    //dbg->FillTypeVal(p->child);
   }
 p->opened=1;
 int increase=countVisChildren(p);
 p->vischild=increase;
 mi_gvar *r=p->parent;
 while (r)
   {
    r->vischild+=increase;
    r=r->parent;
   }
 Count=1+var->vischild;
}

class TInspector : public TDialog
{
public:
 TInspector(TPVarTree *p);
 TInspector(const TRect &aR, char *anExp, char *aTState);
 ~TInspector();

 virtual void handleEvent(TEvent &event);
 virtual void setState(uint16 aState, Boolean enable);
 virtual Boolean valid(ushort command);

 void inspect();
 void expand();
 void collapse();
 void recycle();
 int  modify();
 int  format();
 void updateCommands(Boolean all=True);
 void updateVars(mi_gvar_chg *changed);
 const char *getVar() { return tree ? tree->mainVarExp() : exp; }
 char *getTreeState() { return tree ? tree->createTreeState() : newStr(tstate); }

 static int getCountInspectors() { return cInspectors; }

protected:
 TPVarTree *tree, *fake;
 mi_gvar *focused;
 int nFocused;
 TStringableListBox *theLBox;
 TNoStaticText      *status;
 int outOfScope;
 enum { stNone, stOk, stWaitGDB, stOutOfScope };
 int iStatus;
 TSNoStaticText *CreateStatus();

 char *exp, *tstate;

 static int cInspectors;
};

int TInspector::cInspectors=0;

TInspector::~TInspector()
{
 delete tree;
 delete fake;
 delete[] exp;
 delete[] tstate;
 cInspectors--;
}

Boolean TInspector::valid(ushort command)
{
 if (command==cmClose && tree)
    return tree->canBeDestroyed();
 return TDialog::valid(command);
}

int TInspector::modify()
{
 if (!focused || !dbg)
    return 0;
 char exp[widthWtExp];
 if (focused->value)
    strncpyZ(exp,focused->value,widthWtExp);
 else
    exp[0]=0;
 if (execDialog(createEditExp(__("Modify variable")),exp)==cmOK)
   {
    if (dbg->AssigngVar(focused,exp))
      {
       theLBox->drawView();
       return 1;
      }
    ShowErrorInMsgBox();
   }
 return 0;
}

void TInspector::setState(uint16 aState, Boolean enable)
{
 TDialog::setState(aState,enable);
 // Update the commands when we become active.
 if (aState==sfActive && enable)
    updateCommands();
}

void TInspector::recycle()
{
 if (!outOfScope)
    return;
 if (!tree && exp)
   {// It was loaded from disk and we are recreating it
    tree=new TPVarTree(exp);
    if (tree->isOK())
      {
       delete[] exp; exp=NULL;
       tree->applyTreeState(tstate);
       delete[] tstate; tstate=NULL;
       outOfScope=0;
       theLBox->newList(tree);
       // It was released by newList, don't try to release again
       fake=NULL;
       updateCommands();
      }
    else
       delete tree;
    return;
   }
 if (tree->recycle())
   {
    outOfScope=0;
    theLBox->setRange(tree->GetCount());
    theLBox->focusItem(0);
    updateCommands();
   }
}

void TInspector::expand()
{
 if (outOfScope || !focused || !focused->numchild || focused->opened)
    return;
 tree->expand(focused);
 theLBox->setRange(tree->GetCount());
 theLBox->drawView();
 updateCommands();
}

void TInspector::collapse()
{
 if (outOfScope || !focused || !focused->numchild || !focused->opened)
    return;
 tree->collapse(focused);
 theLBox->setRange(tree->GetCount());
 theLBox->drawView();
 updateCommands();
}

void TInspector::updateCommands(Boolean all)
{
 // Compute the new status:
 int cond1=dbg && dbg->GetState()==MIDebugger::stopped;
 int cond2=cond1 && focused;
 int cond=cond2 && focused->numchild;
 int niStatus;
 const char *cStatus;
 if (outOfScope)
   {
    niStatus=stOutOfScope;
    cStatus=TVIntl::getText(cInspNoScope,icInspNoScope);
   }
 else if (!cond1)
   {
    niStatus=stWaitGDB;
    cStatus=TVIntl::getText(cInspWait,icInspWait);
   }
 else
   {
    niStatus=stOk;
    cStatus=TVIntl::getText(cInspOk,icInspOk);
   }
 // Update according it
 if (all)
   {
    TSetEditorApp::setCmdState(cmDbgInspect,cond2 && focused->ispointer ?
                               True : False);
    TSetEditorApp::setCmdState(cmExpand,cond && !focused->opened ? True : False);
    TSetEditorApp::setCmdState(cmCollapse,cond && focused->opened ? True : False);
    TSetEditorApp::setCmdState(cmRecycle,cond1 && outOfScope ? True : False);
    TSetEditorApp::setCmdState(cmModifyIns,cond2 && !focused->numchild &&
                               focused->attr==MI_ATTR_EDITABLE ? True : False);
   }
 if (niStatus!=iStatus)
   {
    iStatus=niStatus;
    status->setText(cStatus);
    if (all)
       TSetEditorApp::setCmdState(cmFormatIns,cond2 ? True : False);
   }
}

void TInspector::updateVars(mi_gvar_chg *changed)
{
 if (outOfScope)
    return;

 mi_gvar_chg *ch=changed;
 const char *n=tree->mainVarName();
 int l=strlen(n);
 int nchanged=0;

 while (ch)
   {
    if (ch->name && strncmp(n,ch->name,l)==0 &&
        (!ch->name[l] || ch->name[l]=='.'))
      {
       int depth=0;
       char *s;
       if (ch->name[l])
          for (depth=1, s=ch->name+l+1; *s; s++)
              if (*s=='.') depth++;
       dbgPr("Changed var: %s (%d)\n",ch->name,depth);
       if (depth==0 && (!ch->in_scope || ch->new_type))
         {
          if (ch->new_type)
             printf("It happened!!! type changed for gdb var\n");
          dbgPr("Disabling var\n");
          tree->undefine();
          outOfScope=1;
          updateCommands();
          return;
         }
       nchanged+=tree->markChanged(ch->name,depth);
      }
    ch=ch->next;
   }
 if (nchanged)
    theLBox->drawView();
}

static
TDialog *createFormatInst()
{
 TSViewCol *col=new TSViewCol(__("Format"));

 // EN: BDHNO
 col->insert(xTSLeft,yTSUp,
             TSLabelRadio(__("Format"),
                          __("~N~atural"),
                          __("~B~inary"),
                          __("~D~ecimal"),
                          __("~H~exadecimal"),
                          __("~O~ctal"),0));
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(cmFormatIns);
 delete col;
 return d;
}

int TInspector::format()
{
 if (!dbg || !focused || iStatus!=stOk)
    return 0;
 uint32 box=(uint32)focused->format;
 if (execDialog(createFormatInst(),&box)==cmOK)
   {
    if (dbg->SetFormatgVar(focused,(enum mi_gvar_fmt)box))
      {
       focused->changed=1;
       theLBox->drawView();
       return 1;
      }
    ShowErrorInMsgBox();
   }
 return 0;
}

void TInspector::handleEvent(TEvent &event)
{
 TDialog::handleEvent(event);
 if (event.what==evCommand)
   {
    switch (event.message.command)
      {
       case cmDbgInspect:
            inspect();
            break;
       case cmExpand:
            expand();
            break;
       case cmCollapse:
            collapse();
            break;
       case cmRecycle:
            recycle();
            break;
       case cmClose:
            close();
            break;
       case cmModifyIns:
            modify();
            break;
       case cmFormatIns:
            format();
            break;
       default:
            return;
      }
    clearEvent(event);
   }
 else if (event.what==evBroadcast)
   {
    switch (event.message.command)
      {
       case cmListItemFocused:
            nFocused=((TListViewer *)event.message.infoPtr)->focused;
            focused=tree->getItem(nFocused);
            updateCommands();
            break;
       case cmVarChanged:
            updateVars((mi_gvar_chg *)event.message.infoPtr);
            break;
       case cmDbgChgState:
            if (!dbg || (dbg->GetState()==MIDebugger::disconnected ||
                dbg->GetState()==MIDebugger::connected))
              {
               if (tree)
                  tree->disable();
               outOfScope=1;
              }
            updateCommands(owner->current==this ? True : False);
            return;
       default:
            return;
      }
    clearEvent(event);
   }
}

void TInspector::inspect()
{
 char *e=tree->getExpression(nFocused);
 dbgPr("Expression: %s\n",e);
 OpenInspector(e);
 free(e);
}

TInspector::TInspector(TPVarTree *p) :
    TWindowInit(TInspector::initFrame),
    TDialog(TRect(-1,-1,-1,-1),__("Inspector"))
{
 flags=wfMove | wfClose | wfGrow | wfZoom;
 growMode=gfGrowLoY | gfGrowHiX | gfGrowHiY;
 outOfScope=0;

 focused=NULL;
 exp=tstate=NULL;
 fake=NULL;
 TSetEditorApp::setCmdState(cmDbgInspect,False);
 TSetEditorApp::setCmdState(cmExpand,False);
 TSetEditorApp::setCmdState(cmCollapse,False);

 TSViewCol *col=new TSViewCol(this);
 TRect r=GetDeskTopSize();

 tree=p;
 int hMax=r.b.y-r.a.y-3-2-2;
 int wMax=(r.b.x-r.a.x)*8/10, w;
 int h=hMax;
 w=wMax;

 TSStringableListBox *lbox=new TSStringableListBox(w,h,tsslbVertical | tsslbHorizontal);
 theLBox=(TStringableListBox *)(lbox->view);
 theLBox->hScrollBar->setParams(0,0,5000,w,1);
 theLBox->growMode=gfGrowHiX | gfGrowHiY;

 col->insert(xTSLeft,yTSUp,MakeVeGroup(0,lbox,CreateStatus(),0));

 col->doItCenter(hcInspector);
 delete col;

 cInspectors++;
}

TInspector::TInspector(const TRect &aR, char *anExp, char *aTState) :
    TWindowInit(TInspector::initFrame),
    TDialog(aR,__("Inspector"))
{
 flags=wfMove | wfClose | wfGrow | wfZoom;
 growMode=gfGrowLoY | gfGrowHiX | gfGrowHiY;

 outOfScope=1;
 focused=NULL;
 tree=NULL;
 exp=anExp;
 tstate=aTState;

 TSViewCol *col=new TSViewCol(this);

 int w=aR.b.x-aR.a.x-4;
 int h=aR.b.y-aR.a.y-3;

 TSStringableListBox *lbox=new TSStringableListBox(w,h,tsslbVertical | tsslbHorizontal);
 theLBox=(TStringableListBox *)(lbox->view);
 theLBox->hScrollBar->setParams(0,0,5000,w,1);
 theLBox->growMode=gfGrowHiX | gfGrowHiY;
 fake=new TPVarTree(anExp,True);
 theLBox->newList(fake);

 col->insert(xTSLeft,yTSUp,MakeVeGroup(0,lbox,CreateStatus(),0));

 col->doIt();
 helpCtx=hcInspector;
 delete col;

 updateCommands();

 cInspectors++;
}

TSNoStaticText *TInspector::CreateStatus()
{
 const char *m1=TVIntl::getText(cInspOk,icInspOk);
 const char *m2=TVIntl::getText(cInspWait,icInspWait);
 const char *m3=TVIntl::getText(cInspNoScope,icInspNoScope);
 int l1=strlen(m1), l2=strlen(m2), l=l1;
 if (l2>l) l=l2;
 l2=strlen(m3);
 if (l2>l) l=l2;

 char b[l+1];
 memset(b,' ',l);
 b[l]=0;
 TSNoStaticText *st=new TSNoStaticText(b);
 status=(TNoStaticText *)st->view;
 status->growMode=gfGrowHiY | gfGrowLoY;
 iStatus=stNone;

 return st;
}

class TDskInspector : public TDskWin
{
public:
 TDskInspector(TInspector *w);
 ~TDskInspector() {};

 char *GetText(char *dest, short maxLen);
 void saveData(opstream &os);
 static void readData(ipstream &is, char version);

protected:
 TInspector *inspector;
};

TDskInspector::TDskInspector(TInspector *w)
{
 view=inspector=w;
 type=dktDbgIns;
 CanBeDeletedFromDisk=0;
 CanBeSaved=0;
 ZOrder=-1;
}

char *TDskInspector::GetText(char *dest, short maxLen)
{
 TVIntl::snprintf(dest,maxLen,__("   Inspector %s"),inspector->getVar());
 return dest;
}

void TDskInspector::saveData(opstream &os)
{
 unsigned wS=TScreen::getCols();
 unsigned hS=TScreen::getRows();
 TRect size=inspector->getBounds();
 // size
 SaveExpandedRect(os,size,wS,hS);
 // z-order
 os << (int)(TProgram::deskTop->indexOf(view));
 // expression
 os.writeString(inspector->getVar());
 // tree state
 char *state=inspector->getTreeState();
 os.writeString(state);
 delete[] state;
 // zoomRect
 SaveExpandedRect(os,inspector->zoomRect,wS,hS);
}

void TDskInspector::readData(ipstream &is, char version)
{
 unsigned wS=TScreen::getCols();
 unsigned hS=TScreen::getRows();
 TRect size;

 // size
 ReadExpandedRect(is,size,wS,hS);
 // z-order
 int ZOrder;
 is >> ZOrder;
 // expression
 char *exp=is.readString();
 // tree state
 char *tstate=is.readString();

 // Recreate it
 TInspector *d=new TInspector(size,exp,tstate);
 // zoomRect
 if (version>1)
    ReadExpandedRect(is,d->zoomRect,wS,hS);
 TDskInspector *win=new TDskInspector(d);
 win->ZOrder=ZOrder;
 AddNonEditorToHelper(win);
 InsertInOrder(TProgram::deskTop,win);
}

static
void OpenInspector(const char *var)
{
 TPVarTree *p=new TPVarTree(var);

 if (p && p->isOK())
   {
    TInspector *d=new TInspector(p);
   
    TStringableListBoxRec box;
    box.items=p;
    box.selection=0;
    d->setData(&box);

    TDskInspector *win=new TDskInspector(d);
    AddAndInsertDskWin(win);
   }
 else
    ShowErrorInMsgBox();
}

void TSetEditorApp::DebugInspector(char *startVal)
{
 char exp[widthWtExp];
 if (startVal)
   {
    strncpyZ(exp,startVal,widthWtExp);
    delete[] startVal;
   }
 else
    exp[0]=0;
 if (execDialog(createEditExp(__("Inspect variable")),exp)==cmOK)
    OpenInspector(exp);
}

/*****************************************************************************
  End of TInspector class and functionality
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
    dbgPr("Breakpoint without file name\n");
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
   {
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

void TBreakpoints::refreshIgnore()
{// Only before running
 if (!dbg || dbg->GetState()!=MIDebugger::target_specified)
    return;
 if (DEBUG_BREAKPOINTS_UPDATE)
    dbgPr("TBreakpoints::refreshIgnore: Refreshing ignore field\n");
 mi_bkpt *b=first;
 while (b)
   {
    if (b->enabled && b->ignore>0)
       dbg->BreakAfter(b);
    b=b->next;
   }
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
 static int  Delete(int which);

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
 int iErr=DebugGetErrorSt();
 if (iErr==MI_OK)
   {
    messageBox(__("Sorry, you can't perform this operation right now"),
               mfOKButton | mfError);
    return;
   }
 char *cErr=TVIntl::getTextNew(MIDebugger::GetErrorStr());

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

int TDiagBrk::Delete(int which)
{
 mi_bkpt *p=bkpts.getItem(which);
 // Delete the current breakpoint
 if (!DeleteFromGDBandSpLines(p))
    return 0;
 bkpts.remove(p);
 return 1;
}

int TDiagBrk::Delete()
{
 return Delete(focusedB);
}

int TDiagBrk::Enable()
{
 mi_bkpt *p=bkpts.getItem(focusedB);
 if (p->enabled)
    return 0;
 p->enabled=1;

 return ApplyBkt(p,p,1);
}

int TDiagBrk::Disable()
{
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
      {// Just in case
       case cmBkModify:
       case cmBkDel:
       case cmBkEnable:
       case cmBkDisable:
       case cmBkGo:
            if (!bkpts.GetCount())
              {
               clearEvent(event);
               return;
              }
      }
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
   {
    dbgPr("Don't know how to move %s:%d\n",file,spline->oline+1);
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
   {
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
 TDiagWp(const TRect &aR, const char *aStartVal);

 virtual void handleEvent(TEvent &event);

 int Modify();
 int Add();
 int Delete();
 int Enable();
 int Disable();
 static void UpdateCommadsForCount(int c);
 static int Delete(int which);

protected:
 int focusedB;
 TStringableListBox *theLBox;
 const char *startVal;

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
 box.mode=0;
 box.enabled=1;
 if (startVal)
    strncpyZ(box.exp,startVal,wExp);
 else
    box.exp[0]=0;

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

int TDiagWp::Delete(int which)
{
 mi_wp *p=wpts.getItem(which);
 // Delete the current breakpoint
 if (!DeleteFromGDB(p))
    return 0;
 wpts.remove(p);
 return 1;
}

int TDiagWp::Delete()
{
 return Delete(focusedB);
}

int TDiagWp::Enable()
{
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
      {// Just in case
       case cmBkModify:
       case cmBkDel:
       case cmBkEnable:
       case cmBkDisable:
            if (!wpts.GetCount())
              {
               clearEvent(event);
               return;
              }
      }
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

TDiagWp::TDiagWp(const TRect &r, const char *aStartVal) :
    TWindowInit(TDiagWp::initFrame),
    TDialog(r,__("Watchpoints"))
{
 flags=wfMove | wfClose;
 growMode=gfGrowLoY | gfGrowHiX | gfGrowHiY;
 focusedB=0;
 startVal=aStartVal;

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

void TSetEditorApp::DebugEditWatchPts(char *startVal)
{
 // Currently we allow editing breakpoints only if we can get feadback from
 // gdb. I think its possible to avoid it, but I don't see the point and
 // the consequences can be quite confusing.
 if (DebugCheckStopped())
   {
    TRect r=GetDeskTopSize();
    TDiagWp *d=new TDiagWp(TRect(0,0,r.b.x,r.b.y-3),startVal);
    TDiagWp::UpdateCommadsForCount(wpts.GetCount());
   
    TStringableListBoxRec box;
    box.items=&wpts;
    box.selection=0;
   
    execDialog(d,&box);
   }
 delete[] startVal;
}

/*****************************************************************************
  End of TWatchpoints class
*****************************************************************************/

/*****************************************************************************
  Data Window
  From "DataWindow v0.10 Copyright (C) 1998 Laszlo Molnar" that was
contributed to RHIDE.
*****************************************************************************/

/*
 Palette:
 01 Normal text (active)
 02 Normal text (not active)
 03 Focused text
 04 Selected text
 05 Changed test
 06 reserved
*/

const uchar stNonAccess=1, stEdited=2, stChanged=4;
const int addrLen=10; // "12345678: "

// A little indicator: show endian mode, radix etc.
class TDIndicator: public TIndicator
{
public:
 TDIndicator(const TRect & bounds);
 virtual TPalette &getPalette() const;

 enum IndiType { iChanged=1, iEndian, iRadix, iAutofollow, iBaseAddress };
 virtual void draw();
 virtual void changeState(IndiType, int);
 // Some wrappers to make things more clear
 void setEndian(uchar endian)
 {
  changeState(iEndian,"eE"[endian & 1]);
 }
 void setRadix(uchar radix)
 {
  changeState(iRadix,"XD"[radix & 1]);
 }
 void setAutofollow(uchar autoFollow)
 {
  changeState(iAutofollow," A"[autoFollow & 1]);
 }
 void setBaseAddress(ulong baseAddress)
 {
  changeState(iBaseAddress,baseAddress ? 'B' : ' ');
 }

protected:
  char thestate[10];
};

const int taddNone=-1, taddNewValue=0, taddFrom=1, taddTo=2, taddExp=3,
          taddLength=4, taddValue=5, taddChangeOrig=0x100;
const ulong maxBlockLen=0x100000, warnBlockLen=0x20000;

class TDataViewer: public TView
{
public:
 TDataViewer(const TRect &bounds, TScrollBar *aVScrollBar,
             const char *addr_txt, TDIndicator *aIndi);
 ~TDataViewer();
 virtual TPalette &getPalette() const;
 virtual void changeBounds(const TRect &bounds);
 virtual void setState(ushort aState, Boolean enable);
 virtual void draw();
 virtual void handleEvent(TEvent & event);
 void update(unsigned long address, Boolean external=False);
 int  getLine(char *buf, char *cols, int row);
 void cursorHoriz(int);
 void adjustWindow();
 unsigned char *curs2memo();
 void printCursorAddress(char *buf, Boolean deref=False);
 void saveData(opstream &os);
 static TDataViewer *readData(ipstream &is, uchar version,
                              const TRect &bounds, TScrollBar *aVScrollBar,
                              TDIndicator *aIndi);
 void recycle();

 // Wrappers to make things more clear
 void setAutoFollow(uchar nVal)
 {
  autoFollow=nVal;
  indi->setAutofollow(autoFollow);
 }
 void setEndian(uchar nVal)
 {
  endian=nVal;
  indi->setEndian(endian);
 }
 void setRadix(uchar nVal)
 {
  radix=nVal;
  indi->setRadix(radix);
 }
 void setBaseAddress(ulong nVal)
 {
  baseAddress=nVal;
  indi->setBaseAddress(baseAddress);
 }

 TDIndicator *indi;
 unsigned long memStart;
 unsigned memLen;
 uchar *memo;

 unsigned long origAddr;
 char *origAddrTxt;
 unsigned bytesPerLine;

 enum { dmBytes=0, dm2Bytes=1, dm4Bytes=2, dmChars=3, dmMAX=4 };
 uchar dispMode;

 enum { rxHex=0, rxDec=1, rxMAX=2 };
 uchar radix;
 uchar endian;

 uchar autoFollow;
 uchar outOfScope;
 Boolean addressChanged;
 unsigned long baseAddress;

 TScrollBar *vs;
 unsigned long memMin;
 unsigned long memMax;
 void UpdateScroll();

 static int getCountDataViewers() { return cDataViewers; }

protected:
 static int cDataViewers;

 static void setCommands(Boolean enable);
 int EnterAddresses(const char *tit, int t1, ulong *v1,
                    const char *startVal=NULL, int t2=taddNone,
                    ulong *v2=NULL, int t3=taddNone, ulong *v3=NULL);
 void updateIfOverlaps(ulong from, ulong len);
};

int TDataViewer::cDataViewers=0;

static
int isValidAddress(const char *taddr, unsigned long &addr)
{
 if (!dbg)
    return 0;
 int na, res;
 uchar test;
 int convAddr=!(isdigit(*taddr) || *taddr=='&' || *taddr=='$');

 res=dbg->ReadMemory(taddr,1,&test,na,convAddr,&addr);
 if (!res)
    ShowErrorInMsgBox();
 else if (na)
    messageBox(__("Unaccessable memory address specified"),mfError|mfOKButton);

 return res && !na;
}

static
int isValidAddressNA(const char *taddr, unsigned long &addr)
{
 if (!dbg)
    return 0;
 int na, res;
 uchar test;
 int convAddr=!(isdigit(*taddr) || *taddr=='&' || *taddr=='$');

 res=dbg->ReadMemory(taddr,1,&test,na,convAddr,&addr);
 if (!res)
    ShowErrorInMsgBox();

 return res;
}

static
int isValidUL(const char *exp, unsigned long &val)
{
 if (!dbg)
    return 0;
 char *res=dbg->EvalExpression(exp);
 if (!res || MIDebugger::GetErrorNumber())
   {
    ShowErrorInMsgBox();
    return 0;
   }
 char *end;
 val=strtoul(res,&end,0);

 int ret=1;
 if (*end && !isspace(*end))
   {
    ret=0;
    messageBox(__("Error in expression, an integer is needed"),mfError | mfOKButton);
   }
 free(res);

 return ret;
}

//\\//\\//\\//\\//\\//\\ Memory handling

/**[txh]********************************************************************

  Description:
  Reads @var{len} bytes from the specified @var{addr} to the @var{dest}
buffer. It uses one gdb transfer, it means that transfering more than 8 KB
can result in a gdb response of over than 64 KB. That's a data window of
90 lines with 90 cols (270 physical cols). So I think that's ok for the data
window, but not for other things.
  
  Return: !=0 if the read succeed. Note that if just one byte can't be
accessed gdb will refuse to read the rest.
  
***************************************************************************/

static
int targetReadMemory(unsigned long addr, uchar *dest, unsigned len)
{
 if (!dbg)
    return 0;
 char b[64];
 CLY_snprintf(b,64,"0x%lx",addr);
 int na;
 int res=dbg->ReadMemory(b,len,dest,na,0,NULL);

 return res && !na;
}

const unsigned lenAssign=1024, lenBufAssign=48+lenAssign*4,
               lenMultiAssign=8*lenAssign;

/**[txh]********************************************************************

  Description:
  Writes @var{len} bytes to memory address @var{addr} from @var{memo}. It
does the transfers in blocks of 1 KB exploiting the "type[repeat]" cast
implemented by gdb.
  
  Return: !=0 OK, else at least one transfer failed and we aborted.
  
***************************************************************************/

int targetWriteMemory(ulong addr, uchar *memo, int len)
{
 char b[lenBufAssign];
 while (len)
   {
    unsigned transfer=len;
    if (transfer>lenAssign)
       transfer=lenAssign;
    int offset=CLY_snprintf(b,lenBufAssign,"(unsigned char[%d])*0x%lx={",
                            transfer,addr);
    for (unsigned i=0; i<transfer; i++)
       {
        unsigned val=memo[i];
        // Convert the number fast, avoid calling snprintf thousands of times
        if (val>=200)
          {
           b[offset++]='2';
           val-=200;
           if (val<10)
              b[offset++]='0';
          }
        else if (val>=100)
          {
           b[offset++]='1';
           val-=100;
           if (val<10)
              b[offset++]='0';
          }
        if (val>=10)
          {
           unsigned d=val/10;
           b[offset++]=d+'0';
           val-=d*10;
          }
        b[offset++]=val+'0';
        if ((i<transfer-1) || (len-transfer))
           b[offset++]=',';
       }
    b[offset++]='}';
    b[offset]=0;

    if (!DebugEvalExpressionNoRet(b))
       return 0;

    len-=transfer;
    addr+=transfer;
   }
 return 1;
}

/**[txh]********************************************************************

  Description:
  That's a version of readBytes that just assumes that all the block can't
be accessed if the read fails. Is called for small blocks that we assume
are atomic. @x{readBytes}.
  
***************************************************************************/

static
void readBytesFail(ulong addr, uchar *memo, unsigned len, unsigned clen)
{
 int ok=targetReadMemory(addr,memo,len);
 memset(memo+clen,ok ? 0 : stNonAccess,len);
}

/**[txh]********************************************************************

  Description:
  Read @var{len} bytes from @var{addr} to @var{memo}. The @var{memo} vector
must hold at least twice the @var{len}. The bottom half is for the data and
the rest for the flags indicating if the memory can be accessed or was
edited. The @var{clen} is the offset of the flags.@*
  This function will try to read the whole block in one transfer. If it
fails it tries to read 8 bytes elements (aligned).
  
***************************************************************************/

static
void readBytes(ulong addr, uchar *memo, unsigned len, unsigned clen)
{
 if (targetReadMemory(addr,memo,len))
   {
    memset(memo+clen,0,len);
    return;
   }
 // Ok, the ML methode is too slow here. We will assume nothing is smaller
 // than 8 bytes.
 unsigned r=addr & 7;
 if (r)
   {
    if (r>len) r=len;
    readBytesFail(addr,memo,r,clen);
    len-=r;
   }
 while (len>=8)
   {
    readBytesFail(addr+r,memo+r,8,clen);
    len-=8;
    r+=8;
   }
 if (len)
    readBytesFail(addr+r,memo+r,len,clen);
}

/**[txh]********************************************************************

  Description:
  Write @var{len} bytes from @var{memo} to @var{addr}. The @var{memo} vector
must hold at least twice the @var{len}. The bottom half is for the data and
the rest for the flags indicating if the memory can be accessed or was
edited. The @var{clen} is the offset of the flags.@*
  
***************************************************************************/

static
void writeBytes(ulong addr, uchar *memo, unsigned len, unsigned clen)
{
 if (!len)
    return;

 unsigned ic, jc;

 // Skip memory we can't access at the beggining also unmodified
 for (ic=0; ic<len && ((memo[ic+clen] & stNonAccess)
      || !(memo[ic+clen] & stEdited)); ic++);
 // Same at the end
 for (jc=len-1; jc>ic && ((memo[jc+clen] & stNonAccess)
      || !(memo[jc+clen] & stEdited)); jc--);
 jc++;

 len=jc-ic;
 if (len)
   {
    if (targetWriteMemory(addr+ic,memo+ic,len))
      {// Mark as not edited, but changed.
       memo+=clen+ic;
       for (ic=0; ic<len; ic++)
           if (memo[ic] & stEdited)
             {// This way we can add other flags
              memo[ic]&=~stEdited;
              memo[ic]|=stChanged;
             }
      }
   }
}

static
void targetFillMem(ulong to, unsigned len, unsigned value)
{
 uchar *mem=new uchar[min(lenMultiAssign,len)];
 memset(mem,value,min(lenMultiAssign,len));

 while (len)
   {
    unsigned size=min(lenMultiAssign,len);
    if (!targetWriteMemory(to,mem,size))
       break;
    len-=size;
    to+=size;
   }
 delete[] mem;
}

static
void targetMoveMem(ulong from, ulong to, unsigned len)
{
 uchar *mem=new uchar[min(lenMultiAssign,len)];

 while (len)
   {
    unsigned size=min(lenMultiAssign,len);
    if (!targetReadMemory(from,mem,size))
       break;
    if (!targetWriteMemory(to,mem,size))
       break;
    len-=size;
    to+=size;
    from+=size;
   }
 delete[] mem;
}

static
unsigned readFile(FILE *f, ulong to, unsigned len)
{
 unsigned bRead=0;
 uchar *mem=new uchar[min(lenMultiAssign,len)];

 while (len)
   {
    unsigned size=min(lenMultiAssign,len);
    unsigned got=fread(mem,1,size,f);
    bRead+=got;
    if (!targetWriteMemory(to,mem,got))
       break;
    if (got!=size)
       break;
    len-=size;
   }
 delete[] mem;

 return bRead;
}

static
unsigned writeFile(FILE *f, ulong from, unsigned len)
{
 unsigned bWrote=0;
 uchar *mem=new uchar[min(lenMultiAssign,len)];

 while (len)
   {
    unsigned size=min(lenMultiAssign,len);
    if (!targetReadMemory(from,mem,size))
       break;
    unsigned got=fwrite(mem,1,size,f);
    bWrote+=got;
    if (got!=size)
       break;
    len-=size;
   }
 delete[] mem;

 return bWrote;
}

//\\//\\//\\//\\//\\//\\ TDataViewer

const uchar enLittle=0, enBig=1;

TDataViewer::TDataViewer(const TRect & bounds, TScrollBar * aVScrollBar,
                         const char *taddr, TDIndicator *aIndi):
 TView(bounds),
 memStart(0),
 memo(0),
 bytesPerLine(16),
 dispMode(dmBytes),
 radix(rxHex),
 autoFollow(0),
 addressChanged(False),
 baseAddress(0)
{
 helpCtx=hcDataViewer;
 growMode=gfGrowHiX | gfGrowHiY;
 eventMask=evMouseDown | evKeyDown | evCommand | evBroadcast;
 indi=aIndi; // They are created indicating little endian
 endian=enLittle;
 if (dbg && dbg->GetTargetEndian()==MIDebugger::enBig)
   {
    endian=enBig;
    indi->setEndian(endian);
   }
 origAddrTxt=newStr(taddr);
 isValidAddress(origAddrTxt,origAddr);
 setCursor(addrLen,0);
 showCursor();
 vs=aVScrollBar;
 memMin=0xFFFFFFFF;
 memMax=0;
 cDataViewers++;
 outOfScope=0;
}

TDataViewer *TDataViewer::readData(ipstream &is, uchar , const TRect &bounds,
                                   TScrollBar *aVScrollBar, TDIndicator *aIndi)
{
 char *taddr=is.readString();
 TDataViewer *dv=new TDataViewer(bounds,aVScrollBar,taddr,aIndi);
 // state
 // iChanged and iBaseAddress not saved
 // iEndian, iRadix, iAutofollow
 uchar aux;
 is >> aux;
 dv->setEndian(aux);
 is >> aux;
 dv->setRadix(aux);
 is >> aux;
 dv->setAutoFollow(aux);
 is >> dv->dispMode >> dv->bytesPerLine;
 dv->outOfScope=1;
 setCommands(False);

 return dv;
}

void TDataViewer::recycle()
{
 if (outOfScope)
   {
    if (isValidAddress(origAddrTxt,origAddr))
      {
       setCommands(True);
       update(origAddr);
       TSetEditorApp::setCmdState(cmRecycle,False);
       outOfScope=0;
      }
   }
}

TDataViewer::~TDataViewer()
{
 delete[] origAddrTxt;
 free(memo);
 cDataViewers--;
}

void TDataViewer::saveData(opstream &os)
{
 os.writeString(origAddrTxt);
 // state
 // iChanged and iBaseAddress not saved
 // iEndian, iRadix, iAutofollow
 os << endian << radix << autoFollow
    << dispMode << bytesPerLine;
}

void TDataViewer::update(ulong addr, Boolean external)
{
 int memoIsNew=0;
 if (!memo)
   {
    memLen=bytesPerLine*size.y;
    memo=(uchar *)malloc(memLen*2);
    memoIsNew=1;
   }

 indi->changeState(TDIndicator::iChanged,' ');
 if (external && autoFollow)
   {
    addr=0;
    addressChanged=False;
    if (isValidAddress(origAddrTxt,addr))
      {
       addressChanged=origAddr!=addr ? True : False;
       origAddr=addr;
      }
   }
 Boolean otherAddress=addressChanged;
 if (memStart!=addr)
    otherAddress=True;
 memStart=addr;

 uchar *oldMemo=NULL;
 if (!memoIsNew && !otherAddress)
   {
    oldMemo=new uchar[memLen*2];
    memcpy(oldMemo,memo,memLen*2);
   }

 clock_t t1,t2;
 struct timeval T1,T2;
 double secs,secs2;

 if (PROFILE)
   {
    t1=clock();
    gettimeofday(&T1,0);
   }

 readBytes(addr,memo,size.y*bytesPerLine,memLen);

 if (PROFILE)
   {
    t2=clock();
    gettimeofday(&T2,0);
    // Substract the reference
    T2.tv_sec-=T1.tv_sec;
    if (T2.tv_usec<T1.tv_usec)
      {
       T2.tv_sec--;
       T2.tv_usec=T1.tv_usec-T2.tv_usec;
      }
    else
       T2.tv_usec-=T1.tv_usec;
    secs=T2.tv_sec+T2.tv_usec/1e6;
    secs2=(t2-t1)/(double)CLOCKS_PER_SEC;
    int bts=size.y*bytesPerLine;
    printf("Time: %f seconds\nSpeed: %f bytes/second\n%5.2f%% Editor\n",
           secs,bts/secs,secs2/secs*100);
   }

 if (oldMemo)
   {
    for (unsigned i=0; i<memLen; i++)
        if (memo[i]!=oldMemo[i])
           memo[i+memLen]|=stChanged;
    delete[] oldMemo;
   }

 drawView();
 UpdateScroll();
}

void TDataViewer::UpdateScroll()
{
 unsigned long mmax;

 if (memStart<memMin)
    memMin=memStart;
 mmax=memStart+bytesPerLine*(size.y-1);
 if (mmax>memMax)
    memMax=mmax;
 vs->setParams(memStart,memMin,memMax,(size.y-1)*bytesPerLine,bytesPerLine);
 if (0)
    messageBox(mfError | mfOKButton,"%lX %lX %lX %X %X",memStart,memMin,memMax,
               (size.y-1)*bytesPerLine,bytesPerLine);
}

static const char *const notAccess="----------- ";
static const uchar fieldLen[4][3]=
{
 {2, 3, 4}, {4, 5, 6}, {8, 10, 11}, {0, 0, 0},
};

static const char *fieldStr[4][2]=
{
 {"%02X ", "%3u "}, {"%04X ", "%5u "}, {"%08X ", "%10u "}, {NULL, NULL},
};

static const uchar fieldBytes[4]={1,2,4,1};

static inline
uchar toPrintable(uchar uc)
{
 if (0)
    // I think that all current TV drivers supports all chars.
    return (uc)<0x7e && (uc)>=32 ? (uc) : '.';
 else
    return uc ? uc : ' ';
}

int TDataViewer::getLine(char *buf, char *cols, int row)
{
 if (!memo)
   {
    *buf=0;
    return 0;
   }

 unsigned ic;
 const unsigned bpl=bytesPerLine;
 uchar *mem=memo+row*bpl, *cmem=mem+memLen, uc;
 ushort us;
 const unsigned fl=fieldLen[dispMode][radix]+1;
 const char *fs=fieldStr[dispMode][radix];
 const char *notAcc=notAccess+12-fl;
 const char *bufOri=buf;

 switch (dispMode)
   {
    case dmBytes:              // 1-byte-length unsigned integers
         for (ic=0; ic<bpl; ic++, buf+=fl, cols+=fl)
            {
             if (cmem[ic] & stEdited)
                memset(cols,1,fl-1);
             if (cmem[ic] & stChanged)
                memset(cols,2,fl-1);
             sprintf(buf,cmem[ic] & stNonAccess ? notAcc : fs,mem[ic]);
            }
         if (radix==rxHex)
           {
            *buf++=' ';
            for (ic=0; ic<bpl; ic++)
               {
                cols++;
                if (cmem[ic] & stEdited)
                   *cols=1;
                if (cmem[ic] & stChanged)
                   *cols=2;
                *buf++=cmem[ic] & stNonAccess ? ' ' : toPrintable(mem[ic]);
               }
           }
         break;
    case dm2Bytes:             // 2-byte-length unsigned integers
         for (ic=0; ic<bpl; ic+=2, buf+=fl, cols+=fl)
            {
             uc=cmem[ic] | cmem[ic+1];
             if (uc & stEdited)
                memset(cols,1,fl-1);
             if (uc & stChanged)
                memset(cols,2,fl-1);
             if (uc & stNonAccess)
               {
                memcpy(buf,notAcc,fl);
                continue;
               }
             if (!endian)
                us=mem[ic]+mem[ic+1]*0x100;
             else
                us=mem[ic+1]+mem[ic]*0x100;
             sprintf(buf, fs, us);
            }
         break;
    case dm4Bytes:             // 4-byte-length unsigned integers
         unsigned uw;

         for (ic=0; ic<bpl; ic+=4, buf+=fl, cols+=fl)
            {
             uc=cmem[ic] | cmem[ic+1] | cmem[ic+2] | cmem[ic+3];
             if (uc & stEdited)
                memset(cols,1,fl-1);
             if (uc & stChanged)
                memset(cols,2,fl-1);
             if (uc & stNonAccess)
               {
                memcpy(buf,notAcc,fl);
                continue;
               }
             if (!endian)
                uw=mem[ic]+mem[ic+1]*0x100+mem[ic+2]*0x10000+
                   mem[ic+3]*0x1000000;
             else
                uw=mem[ic+3]+mem[ic+2]*0x100+mem[ic+1]*0x10000+
                   mem[ic]*0x1000000;
             sprintf(buf, fs, uw);
            }
         break;
    case dmChars:              // characters only
         for (ic=0; ic<bpl; ic++, cols++)
            {
             if (cmem[ic] & stEdited)
                *cols=1;
             if (cmem[ic] & stChanged)
                *cols=2;
             *buf++=(cmem[ic] & stNonAccess) ? ' ' : toPrintable(mem[ic]);
            }
         break;
    default:
         break;
   }
 *buf=0;
 return buf-bufOri;
}

void TDataViewer::draw()
{
 TDrawBuffer b;
 uchar normalColor=getColor(1);
 uchar modifiedColor=getColor(2);
 uchar focusedColor=getColor(3);
 uchar changedColor=getColor(5);
 unsigned bpl=bytesPerLine;

 int ic, jc;
 AllocLocalStr(buf,addrLen+bpl*4+1);
 AllocLocalStr(cols,addrLen+bpl*4+1); // should be enough

 for (ic=0; ic<size.y; ic++)
    {
     uchar color;

     memset(cols,0,size.x);
     sprintf(buf,"%08lX: ",ic*bpl+memStart-baseAddress);
     int len=addrLen+getLine(buf+addrLen,cols,ic);

     color=normalColor;
     b.moveBuf(0,buf,color,len);
     if (len<size.x)
        b.moveChar(len,' ',color,size.x-len);

     if (addressChanged && origAddr==memStart+ic*bpl)
        for (jc=0; jc<addrLen-2; jc++)
            b.putAttribute(jc,modifiedColor);

     for (jc=addrLen; jc<size.x; jc++)
         if (cols[jc-addrLen]==1)
            b.putAttribute(jc,modifiedColor);
         else if (cols[jc-addrLen]==2)
            b.putAttribute(jc,changedColor);

     // colorize the "home" address
     if (memStart+ic*bpl<=origAddr && memStart+ic*bpl+bpl>origAddr)
       {
        int fl, pos;

        fl=fieldLen[dispMode][radix];
        pos=addrLen+(origAddr-memStart-ic*bpl)/fieldBytes[dispMode]*(fl+1);
        if (dispMode==dmChars)
           fl=1;
        for (jc=0; jc<fl; jc++)
            b.putAttribute(pos+jc,focusedColor);
       }

     writeLine(0,ic,size.x,1,b);
    }
}

void TDataViewer::setState(ushort aState, Boolean enable)
{
 TView::setState(aState,enable);
 if (aState==sfActive)
   {
    vs->setState(sfVisible,enable);
    indi->setState(sfActive,enable);
    indi->drawView();
    if (enable)
      {// Check if we have to dis/enable the commands
       if (outOfScope)
          TSetEditorApp::setCmdState(cmRecycle,dbg &&
            dbg->GetState()==MIDebugger::stopped ? True : False);
       else
          setCommands(dbg && dbg->GetState()==MIDebugger::stopped ? True : False);
      }
   }
}

#define cpDataViewer "\x06\x07\x08\x09\x0A\x0B"

TPalette &TDataViewer::getPalette() const
{
 static TPalette
 pal(cpDataViewer,sizeof(cpDataViewer)-1);

 return pal;
}

void TDataViewer::cursorHoriz(int delta)
{
 int cx=cursor.x-addrLen;
 unsigned fl=fieldLen[dispMode][radix];

 if (fl)
    cx=cx/(fl+1)*fl+(cx%(fl+1));
 if (delta>0)
    cx=min(cx+delta,
           fl ? fl*bytesPerLine/fieldBytes[dispMode]-1 : bytesPerLine-1);
 else
    cx=max(cx+delta,0);
 if (fl)
    cx=cx/fl*(fl+1)+(cx%fl);
 setCursor(addrLen+cx,cursor.y);
}

void TDataViewer::adjustWindow()
{
 unsigned xnew;

 xnew=bytesPerLine/fieldBytes[dispMode]*(1+fieldLen[dispMode][radix]);
 if (dispMode==dmBytes && radix==rxHex)
    xnew+=bytesPerLine+2;
 if (dispMode==dmChars)
    xnew=bytesPerLine+1;
 owner->growTo(xnew+3+addrLen,owner->size.y);
}

uchar *TDataViewer::curs2memo()
{
 return memo+bytesPerLine*cursor.y+
        (cursor.x-addrLen)/(fieldLen[dispMode][radix]+1)*fieldBytes[dispMode];
}

const char *taddNames[]=
{
 __("~N~ew Value"),
 __("~F~rom"),
 __("~T~o"),
 __("~E~xpression"),
 __("~L~ength"),
 __("~V~alue")
};

static
struct
{
 char v1[widthWtExp], v2[widthWtExp], v3[widthWtExp];
} boxEA;

int TDataViewer::EnterAddresses(const char *tit, int t1, ulong *v1,
                                const char *startVal, int t2, ulong *v2,
                                int t3, ulong *v3)
{
 if (startVal)
    strncpyZ(boxEA.v1,startVal,widthWtExp);
 else
    boxEA.v1[0]=0;
 boxEA.v2[0]=boxEA.v3[0]=0;

 Boolean changeOrigTxt=False;
 if (t1 & taddChangeOrig)
   {
    t1&=~taddChangeOrig;
    changeOrigTxt=True;
   }

 Boolean edit;
 do
   {
    TSViewCol *col=new TSViewCol(tit);
   
    // EN: EFLNT
    TSLabel *o1=new TSLabel(taddNames[t1],
              new TSInputLine(widthWtExp,1,hID_DbgEvalModifyExp,maxWtBox));
    col->insert(xTSLeft,yTSUp,o1);
    if (v2)
      {
       TSLabel *o2=new TSLabel(taddNames[t2],
                 new TSInputLine(widthWtExp,1,hID_DbgEvalModifyExp,maxWtBox));
       o2->ySep=0;
       col->insert(xTSLeft,yTSUnder,o2,NULL,o1);
       if (v3)
         {
          TSLabel *o3=new TSLabel(taddNames[t3],
                    new TSInputLine(widthWtExp,1,hID_DbgEvalModifyExp,maxWtBox));
          o3->ySep=0;
          col->insert(xTSLeft,yTSUnder,o3,NULL,o2);
         }
      }
    EasyInsertOKCancel(col);
   
    TDialog *d=col->doItCenter(hcDataViewer);
    delete col;

    edit=True;
    if (execDialog(d,&boxEA)!=cmOK)
       edit=False;
    else
      {// Ensure all the requested values are filled.
       if (IsEmpty(boxEA.v1) || (v2 && IsEmpty(boxEA.v2)) || (v3 && IsEmpty(boxEA.v3)))
         {
          messageBox(__("Please fill all the requested values"),mfError|mfOKButton);
          continue;
         }
       if ((t1==taddNewValue ? isValidAddressNA(boxEA.v1,*v1) :
                               isValidAddress(boxEA.v1,*v1))
           && (!v2 || (t2>=taddLength ? isValidUL(boxEA.v2,*v2) :
                                        isValidAddress(boxEA.v2,*v2)))
           && (!v3 || (t3>=taddLength ? isValidUL(boxEA.v3,*v3) :
                                        isValidAddress(boxEA.v3,*v3))))
         {// Avoid huge lengths, warn for large ones.
          ulong *vLen=NULL;
          if (t1==taddLength)
             vLen=v1;
          else if (t2==taddLength)
             vLen=v2;
          else if (t3==taddLength)
             vLen=v3;
          if (vLen)
            {
             if (*vLen>maxBlockLen)
               {
                messageBox(mfError|mfOKButton,
                  __("For security reasons the maximum length is limited to %lu bytes"),
                  maxBlockLen);
                continue;
               }
             if (*vLen>warnBlockLen)
               {
                if (messageBox(__("This could be slow. Do you want to continue?"),
                    mfWarning|mfYesButton|mfNoButton)!=cmYes)
                   continue;
               }
            }

          if (changeOrigTxt)
            {
             delete[] origAddrTxt;
             origAddrTxt=newStr(boxEA.v1);
            }
          return 1;
         }
      }
   }
 while (edit);
 return 0;
}

static
int getFilename(char *buf, int typ)
{
 strcpy(buf,"*");
 return GenericFileDialog(
     typ ? __("Write block to file") : __("Read block from file"),buf,"*",
     typ ? hID_FileSave : hID_FileOpen,fdDialogForSave)!=cmCancel;
}

void TDataViewer::printCursorAddress(char *buf, Boolean deref)
{
 sprintf(buf,deref ? "**0x%lx" : "0x%lx",curs2memo()-memo+memStart);
}

void TDataViewer::setCommands(Boolean enable)
{
 if (enable)
   {
    if (!TView::curCommandSet.has(cmDWFirstCommand))
      {// Enable all
       TView::commandSetChanged=True;
       TView::curCommandSet.enableCmd(cmDWFirstCommand,cmDWLastCommand);
      }
   }
 else
   {
    if (TView::curCommandSet.has(cmDWFirstCommand))
      {// Disable all
       TView::commandSetChanged=True;
       TView::curCommandSet.disableCmd(cmDWFirstCommand,cmDWLastCommand);
      }
   }
}

void TDataViewer::updateIfOverlaps(ulong from, ulong len)
{
 ulong memEnd=memStart+memLen;
 ulong to=from+len;
 if (from<memEnd && to>=memStart)
    update(memStart);
}

void TDataViewer::handleEvent(TEvent & event)
{
 char buf[PATH_MAX];
 TView::handleEvent(event);

 unsigned long newAddr=memStart, from, to, len, value;

 if (event.what==evMouseDown)
   {
    clearEvent(event);
   }
 else if (event.what==evKeyDown)
   {
    switch (event.keyDown.keyCode)
      {
       case kbEsc:
            message(owner,evCommand,cmClose,NULL);
            clearEvent(event);
            return;
       default:
            unsigned kc=event.keyDown.charScan.charCode;

            //fprintf(stderr,"%c",kc);
            if (dispMode==dmChars) // characters only
              {
               memo[cursor.y*bytesPerLine+cursor.x-addrLen]=kc;
               // mark changed
               memo[cursor.y*bytesPerLine+cursor.x-addrLen+memLen]|=stEdited;
               indi->changeState(TDIndicator::iChanged,'*');
               cursorHoriz(1);
               drawView();
              }
            else if ((kc>='0' && kc<='9') || (radix==rxHex && (kc | 0x20)>='a'
                     && (kc | 0x20)<='f'))
              {
               getLine(buf,buf,cursor.y);
               buf[cursor.x-addrLen]=kc;
               for (kc=cursor.x-addrLen; kc && buf[kc]!=' '; kc--);
               if (!sscanf(buf+kc,fieldStr[dispMode][radix],&kc))
                  break;
               unsigned char *mem=curs2memo();

               switch (dispMode)
                 {
                  case dmBytes:
                       *mem=kc;
                       break;
                  case dm2Bytes:
                       mem[endian]=kc;
                       mem[1-endian]=kc>>8;
                       break;
                  case dm4Bytes:
                       mem[3*endian]=kc;
                       mem[1+endian]=kc>>8;
                       mem[2-endian]=kc>>16;
                       mem[3-3*endian]=kc>>24;
                       break;
                  default:
                       break;
                 }
               for (kc=0; kc<fieldBytes[dispMode]; kc++)
                   mem[memLen+kc]|=stEdited;
               indi->changeState(TDIndicator::iChanged,'*');
               cursorHoriz(1);
               drawView();
              }
            break;
      }
    if (newAddr!=memStart)
       update(newAddr);
    clearEvent(event);
   }
 else if (event.what==evBroadcast)
   {
    switch (event.message.command)
      {
       case cmScrollBarChanged:
            if (vs==event.message.infoPtr && (unsigned)vs->value!=memStart)
              {
               update(vs->value);
               clearEvent(event);
              }
            break;
       case cmDbgChgState:
            if (dbg && dbg->GetState()==MIDebugger::stopped)
              {
               if (outOfScope)
                 {
                  if (owner && owner->current==this)
                     TSetEditorApp::setCmdState(cmRecycle,True);
                 }
               else
                 {
                  setCommands(True);
                  update(memStart,True);
                 }
              }
            else
              {// Any other state is useless
               if (outOfScope)
                 {
                  if (owner && owner->current==this)
                     TSetEditorApp::setCmdState(cmRecycle,False);
                 }
               else
                  setCommands(False);
              }
            break;
      }
   }
 else if (event.what==evCommand)
   {
    switch (event.message.command)
      {
       // Cursor movement
       case cmDWUp:
            if (cursor.y==0)
               newAddr=memStart-bytesPerLine;
            else
               setCursor(cursor.x,cursor.y-1);
            break;
       case cmDWDown:
            if (cursor.y==size.y-1)
               newAddr=memStart+bytesPerLine;
            else
               setCursor(cursor.x,cursor.y+1);
            break;
       case cmDWRight:
            cursorHoriz(1);
            break;
       case cmDWBaseIncrement:
            newAddr=memStart+1;
            break;
       case cmDWLeft:
            cursorHoriz(-1);
            break;
       case cmDWBaseDecrement:
            newAddr=memStart-1;
            break;
       case cmDWPgDn:
            newAddr=memStart+size.y*bytesPerLine;
            break;
       case cmDWPgUp:
            newAddr=memStart-size.y*bytesPerLine;
            break;
       case cmDWFirstColumn:
            setCursor(addrLen,cursor.y);
            break;
       case cmDWFirstRow:
            setCursor(cursor.x,0);
            break;
       case cmDWLastRow:
            setCursor(cursor.x,size.y-1);
            break;
       case cmDWLastColumn:
            cursorHoriz(size.x);
            break;
       // End of cursor movement
       case cmDWLessLines:        // decrease bytes/line
            if (bytesPerLine>fieldBytes[dispMode])
              {
               bytesPerLine-=fieldBytes[dispMode];
               update(memStart);
               adjustWindow();
               setCursor(addrLen,cursor.y);
              }
            break;
       case cmDWMoreLines:         // increase bytes/line
            bytesPerLine+=fieldBytes[dispMode];
            free(memo);
            memo=NULL;
            update(memStart);
            adjustWindow();
            break;
       case cmDWUpdateMemory:     // update changes
            writeBytes(memStart,memo,bytesPerLine*size.y,memLen);
            drawView();
            break;
       case cmDWTogAutoF:         // toggle auto follow mode
            setAutoFollow(autoFollow^1);
            break;
       case cmDWBaseAddress:      // set new base address
            printCursorAddress(buf);
            if (EnterAddresses(__("Base Address"),taddNewValue,&baseAddress,buf))
              {
               setBaseAddress(baseAddress);
               drawView();
              }
            break;
       case cmDWDispMode:         // change display mode
            dispMode=(dispMode+1) % dmMAX;
            bytesPerLine&=~(fieldBytes[dispMode]-1);
            update(memStart);
            adjustWindow();
            setCursor(addrLen,cursor.y);
            break;
       case cmDWTogEndian:       // change endianness
            setEndian(endian^1);
            if (fieldBytes[dispMode]>1)
               update(memStart);
            break;
       case cmDWFollowPointer:   // follow pointer
            printCursorAddress(buf,True);
            if (isValidAddress(buf,newAddr))
              {
               origAddr=newAddr;
               setCursor(addrLen,0);
              }
            break;
       case cmDWGotoAddress:     // goto to a new address
            if (EnterAddresses(__("Data window"),taddExp | taddChangeOrig,
                &newAddr,origAddrTxt))
              {
               setCursor(addrLen,0);
               addressChanged=True;
              }
            break;
       case cmDWRecompute:       // reevalute the original address then go to there
            if (isValidAddress(origAddrTxt,newAddr))
              {
               origAddr=newAddr;
               setCursor(addrLen,0);
              }
            break;
       case cmDWFill:           // fill block
            printCursorAddress(buf);
            if (EnterAddresses(__("Fill Block"),taddFrom,&from,buf,
                taddLength,&len,taddValue,&value))
              {
               targetFillMem(from,len,value);
               updateIfOverlaps(from,len);
              }
            break;
       case cmDWClear:          // clear block
            printCursorAddress(buf);
            if (EnterAddresses(__("Clear Block"),taddFrom,&from,buf,
                taddLength,&len) && len<maxBlockLen)
              {
               targetFillMem(from,len,0);
               updateIfOverlaps(from,len);
              }
            break;
       case cmDWMove:           // move block
            printCursorAddress(buf);
            if (EnterAddresses(__("Move Block"),taddFrom,&from,buf,
                taddTo,&to,taddLength,&len) && len<maxBlockLen)
              {
               targetMoveMem(from,to,len);
               updateIfOverlaps(to,len);
              }
            break;
       case cmDWFollowPtnNew:   // follow pointer & open new window
            printCursorAddress(buf,True);
            TSetEditorApp::DebugDataWindow(newStr(buf));
            break;
       case cmDWRead:           // read block
            {
             FILE *f1=NULL;

             if (getFilename(buf,0) && (f1=fopen(buf,"rb"))!=NULL)
               {
                printCursorAddress(buf);
                if (EnterAddresses(__("Read Block"),taddTo,&to,buf,taddLength,
                    &len) && len<maxBlockLen)
                  {
                   unsigned bRead=readFile(f1,to,len);
                   messageBox(mfOKButton | mfInformation,__("%u bytes read."),
                              bRead);
                   updateIfOverlaps(to,bRead);
                  }
                fclose(f1);
               }
             break;
            }
       case cmDWWrite:           // write block
            printCursorAddress(buf);
            if (EnterAddresses(__("Write Block"),taddFrom,&from,buf,taddLength,
                &len) && len<maxBlockLen)
              {// Now we have what to write ask for the file, not in the
               // reverse order to avoid creating an empty file.
               FILE *f;
               if (getFilename(buf,1) && (f=fopen(buf,"wb"))!=NULL)
                 {
                  messageBox(mfOKButton | mfInformation,
                             __("%u bytes written."),writeFile(f,from,len));
                  fclose(f);
                 }
              }
            break;
       case cmDWRadix:          // change radix
            setRadix((radix+1) % rxMAX);
            update(memStart);
            adjustWindow();
            setCursor(addrLen,cursor.y);
            break;
       case cmRecycle:
            recycle();
            // Avoid an extra (and wrong) update
            newAddr=memStart;
            break;
       default:
            return;
      }
    if (newAddr!=memStart)
       update(newAddr);
    clearEvent(event);
   }
}

void TDataViewer::changeBounds(const TRect &bounds)
{
 if (size.y<bounds.b.y-bounds.a.y)
   {
    free(memo);
    memo=NULL;
   }
 setBounds(bounds);
 update(memStart);
 drawView();
}

//\\//\\//\\//\\//\\//\\ TDataWindow

class TDataWindow: public TDialog
{
public:
 TDataWindow(const TRect &, const char *aTitle);
 virtual TPalette &getPalette() const;

 static TDataWindow *createNew(const char *naddr=NULL, Boolean edit=False);
 static TDataWindow *stackWindow();

 void saveData(opstream &os) { viewer->saveData(os); };
 static TDataWindow *readData(ipstream &is, char version, const TRect &bounds);

protected:
 TDataViewer *viewer;
 TScrollBar *vs;
 TDIndicator *indi;

 void loadFromDisk(ipstream &is, char version);
};

#define cpDataWindow "\x97\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F\xA0\xA1"

TPalette &TDataWindow::getPalette() const
{
 static TPalette pal(cpDataWindow,sizeof(cpDataWindow)-1);

 return pal;
}

TDataWindow::TDataWindow(const TRect &bounds, const char *aTitle) :
  TWindowInit(TDataWindow::initFrame),
  TDialog(bounds,aTitle)
{
 growMode=gfGrowLoY | gfGrowHiX | gfGrowHiY;
 flags|=wfGrow | wfZoom;
 helpCtx=hcDataViewer;

 TRect r=getExtent();
 r.grow(-1,-1);

 vs=new TScrollBar(TRect(r.b.x-1,r.a.y,r.b.x,r.b.y));
 insert(vs);

 indi=new TDIndicator(TRect(2,size.y-1,12,size.y));

 if (aTitle)
   {
    r.b.x--;
    viewer=new TDataViewer(r,vs,aTitle,indi);
    insert(viewer);
    insert(indi);
    viewer->select();
   }
}

TDataWindow *TDataWindow::readData(ipstream &is, char version,
                                   const TRect &bounds)
{// Two steps
 TDataWindow *w=new TDataWindow(bounds,NULL);
 w->loadFromDisk(is,version);

 return w;
}

void TDataWindow::loadFromDisk(ipstream &is, char version)
{
 TRect r=getExtent();
 r.grow(-1,-1);
 r.b.x--;
 viewer=TDataViewer::readData(is,version,r,vs,indi);
 insert(viewer);
 insert(indi);
 viewer->select();
 title=newStr(viewer->origAddrTxt);
}

TDataWindow *TDataWindow::createNew(const char *naddr, Boolean edit)
{
 static int winpos=0;
 unsigned long addr;
 char exp[widthWtExp];

 if (!naddr || edit)
   {
    if (naddr)
       strncpyZ(exp,naddr,widthWtExp);
    else
       exp[0]=0;
    if (execDialog(createEditExp(__("Data window")),exp)!=cmOK)
       return NULL;
    naddr=exp;
   }
 if (!isValidAddress(naddr,addr))
    return NULL;

 TRect r(winpos,5+winpos,70,TScreen::getRows()-10+winpos);
 
 winpos=(winpos+1) % 10;
 TDataWindow *dw=new TDataWindow(r,naddr ? naddr : boxEA.v1);
 
 //dw->viewer->update(addr); Will be updated by adjustWindow side effect
 dw->viewer->memStart=addr; // But we need it.
 dw->viewer->adjustWindow();
 return dw;
}

TDataWindow *TDataWindow::stackWindow()
{
 if (!dbg)
    return NULL;

 const char *spReg;
 switch (dbg->GetTargetArchitecture())
   {
    case MIDebugger::arIA32:
         spReg="$esp"; // IA32: Extended Stack Pointer
         break;
    case MIDebugger::arSPARC:
         spReg="$sp";  // SPARC: Stack Pointer
         break;
    default:
         messageBox(__("Not implemented for your platform, please help to implement it"),
                    mfError|mfOKButton);
         return NULL;
   }

 TDataWindow *dw=createNew(spReg,False);

 if (dw)
   {
    dw->viewer->bytesPerLine=4;
    dw->viewer->dispMode=TDataViewer::dm4Bytes;
    dw->viewer->setAutoFollow(1);
    dw->viewer->adjustWindow();
   }
 return dw;
}

//\\//\\//\\//\\//\\//\\ TIndicator

TDIndicator::TDIndicator(const TRect &bounds):
  TIndicator(bounds)
{
 strcpy(thestate,"  eX  ");
}

void TDIndicator::draw()
{
 uchar color, frame;
 TDrawBuffer b;

 if (state & sfDragging)
   {
    color=getColor(2);
    frame=normalFrame;
   }
 else if (state & sfActive)
   {
    color=getColor(1);
    frame=dragFrame;
   }
 else
   {
    color=getColor(3);
    frame=normalFrame;
   }

 b.moveChar(0,frame,color,size.x);
 b.moveCStr(0,thestate,color);
 writeBuf(0,0,size.x,1,b);
}

void TDIndicator::changeState(IndiType snum, int value)
{
 thestate[snum]=value;
 drawView();
}

#define cpDIndicator "\x02\x03\x01"

TPalette &TDIndicator::getPalette() const
{
 static TPalette
 pal(cpDIndicator,sizeof(cpDIndicator)-1);

 return pal;
}

/*****************************************************************************
  Dsk wrapper for the data window
*****************************************************************************/

class TDskDataWin : public TDskWin
{
public:
 TDskDataWin(TDataWindow *w);
 ~TDskDataWin() {};

 char *GetText(char *dest, short maxLen);
 static void Insert(TDataWindow *w);

 void saveData(opstream &os);
 static void readData(ipstream &is, char version);

protected:
 TDataWindow *window;
};

TDskDataWin::TDskDataWin(TDataWindow *w)
{
 view=window=w;
 type=dktDbgDataWin;
 CanBeDeletedFromDisk=0;
 CanBeSaved=0;
 ZOrder=-1;
}

char *TDskDataWin::GetText(char *dest, short maxLen)
{
 TVIntl::snprintf(dest,maxLen,__("   Data Window %s"),window->getTitle(maxLen));
 return dest;
}

void TDskDataWin::Insert(TDataWindow *w)
{
 AddAndInsertDskWin(new TDskDataWin(w));
}

void TDskDataWin::saveData(opstream &os)
{
 unsigned wS=TScreen::getCols();
 unsigned hS=TScreen::getRows();
 TRect size=window->getBounds();
 // size
 SaveExpandedRect(os,size,wS,hS);
 // z-order
 os << (int)(TProgram::deskTop->indexOf(view));
 // expression and state
 window->saveData(os);
 // zoomRect
 SaveExpandedRect(os,window->zoomRect,wS,hS);
}

void TDskDataWin::readData(ipstream &is, char version)
{
 unsigned wS=TScreen::getCols();
 unsigned hS=TScreen::getRows();
 TRect size;

 // size
 ReadExpandedRect(is,size,wS,hS);
 // z-order
 int ZOrder;
 is >> ZOrder;
 // expression and state
 TDataWindow *d=TDataWindow::readData(is,version,size);
 // zoomRect
 if (version>1)
    ReadExpandedRect(is,d->zoomRect,wS,hS);
 // Recreate it
 TDskDataWin *win=new TDskDataWin(d);
 win->ZOrder=ZOrder;
 AddAndInsertDskWin(win);
}

/*****************************************************************************
  TSetEditorApp members for the data window
*****************************************************************************/

void TSetEditorApp::DebugDataWindow(char *startVal)
{
 TDataWindow *dw=TDataWindow::createNew(startVal,True);
 if (dw)
    TDskDataWin::Insert(dw);
 delete[] startVal;
}

void TSetEditorApp::DebugStackWindow()
{
 TDataWindow *dw=TDataWindow::stackWindow();
 if (dw)
    TDskDataWin::Insert(dw);
}

/*
 done:
 -----
 + block(clear ^L,fill ^I,read ^R,write ^W,move ^M)
 + goto   ^G
 + follow ^F
 + follow and open new window ^O
 + display(byte->half word->word->chars->float?->double?) ^D
 + radix 16-10?  ^X
 + endian? ^E
 + edit
 + reevaluate & goto  ^H
 + bytesPerLine increase Gray+, decrease Gray-
 + auto adjust window size
 + base address mode ^B
 + autoFollow ^A
 + add mouse & scrollbar handling
 + remove magic constants

 Here are things that ML marked as thing to do and we did:
 + data breakpoints, we have the watchpoints dialog.
 + fix the FIXMEs
 + popup menus, we have a hole menu for it.
 + disassembler, we have a separated window for it.
*/

/*****************************************************************************
  End of Data Window
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

void DBG_RefreshIgnoreBkpts()
{
 bkpts.refreshIgnore();
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

void DBG_AddPathForSources(const char *path)
{
 if (IsStopped())
    dbg->PathSources(path);
}

static
char prefixList[]="CTL><";

void DBG_GenericCB(const char *m, void *p)
{
 static DynStrCatStruct st={0,NULL};
 int l=strlen(m);
 if (!l)
    return;

 if (*((char *)p)!='>')
   {
    char *b=new char[l+4];
    b[0]=*((char *)p);
    b[1]=':';
    b[2]=' ';
    memcpy(b+3,m,l+1);
    // Remove LF
    if (b[l+2]=='\n')
       b[l+2]=0;
    DebugMsgAdd(b);
    return;
   }

 // Concatenate commands sent to gdb until we get \n
 if (st.str)
   {// Concatenating
    DynStrCat(&st,m,l);
    if (m[l-1]=='\n')
      {// EOL, send it
       st.str[st.len-1]=0;
       DebugMsgAdd(newStr(st.str));
       free(st.str);
       st.str=NULL;
      }
   }
 else
   {// First message
    if (m[l-1]=='\n')
      {// Whole line, send it
       char *b=new char[l+4];
       b[0]=*((char *)p);
       b[1]=':';
       b[2]=' ';
       memcpy(b+3,m,l+1);
       // Remove LF
       b[l+2]=0;
       DebugMsgAdd(b);
      }
    else
      {// Partial line
       char b[4];
       b[0]=*((char *)p);
       b[1]=':';
       b[2]=' ';
       b[3]=0;
       DynStrCatInit(&st,b,3);
       DynStrCat(&st,m,l);
      }
   }
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
};

TDskDbgMsg::TDskDbgMsg(TView *w)
{
 view=w;
 type=dktDbgMsg;
 CanBeDeletedFromDisk=0;
 CanBeSaved=0;
 ZOrder=-1;
}

TDskDbgMsg::~TDskDbgMsg()
{
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

/*****************************************************************************
  End of Dsk Wrapper for Debug Status and Messages Window
*****************************************************************************/

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
 msgsInDebugWindow=GetMsgLines();
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
 while (MsgCol->getCount()>msgsInDebugWindow)
    MsgCol->atFree(0);
 DebugMsgUpdate(edsmDontSelect);
}

void DebugMsgClear()
{
 if (!MsgCol || !MsgList)
    return;
 MsgCol->removeAll();
 MsgList->setRange(0);
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
 if (dbg)
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
 if (TInspector::getCountInspectors() || TDataViewer::getCountDataViewers())
    message(TProgram::deskTop,evBroadcast,cmDbgChgState,NULL);

 MsgWindow->SetStatusGDB(msg);
 if (cleanStop)
    MsgWindow->SetStatusStop("");
 DebugMsgUpdate(edsmDontSelect);
}

int DebugGetErrorSt()
{
 if (!dbg)
    return MIDebugger::GetErrorNumber();

 MIDebugger::eState st=dbg->GetState();
 int e=dbg->GetErrorNumberSt();
 // This is a trick to detect gdb died
 if (st!=dbg->GetState())
    DebugMsgSetState();
 return e;
}

void DebugMsgSetError()
{
 DebugMsgInit();

 char b[maxWStatus];

 char *cErr=TVIntl::getTextNew(MIDebugger::GetErrorStr());
 int iErr=DebugGetErrorSt();

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
         tty=getCurTTY();
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
   {
    if (IsDisAsmWinCurrent())
      {// The disassembler window is the active one, jump there
       jumped=TDisAsmWin::jumpToFrame(f);
       int line;
       char *file=SolveFileName(TDisAsmWin::getFileLine(line));
       if (file)
          GotoFileLine(f->line,file,msg,-1,l,gflCPULine|gflDontSelect);
      }
    else
      {// Try to jump to the source line.
       if (f->file)
         {
          char *file=SolveFileName(f->file);
          if (file && GotoFileLine(f->line,file,msg,-1,l,gflCPULine))
             jumped=1;
          if (IsDisAsmWinAvailable())
             TDisAsmWin::jumpToFrame(f);
         }
       else
         {// No file name => use disassembler window
          DisAsmWin();
          TDisAsmWin::jumpToFrame(f);
         }
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
    if (!f->file)
       l+=CLY_snprintf(b+l,maxWStatus-l," [%s:%p]",
                       f->func ? f->func : unknown,f->addr);
    else
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
    dbgPr("Disabling watchpoint %d, out of scope\n",stoppedInfo->wpno);
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

 int  DeleteAction(ccIndex i, Boolean fromDiskToo=False);
 char *GetText(char *dest, short maxLen);
};

TDskDbgWt::TDskDbgWt(TView *w)
{
 view=w;
 type=dktDbgWt;
 CanBeDeletedFromDisk=0;
 CanBeSaved=0;
 ZOrder=-1;
}

TDskDbgWt::~TDskDbgWt()
{
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

/*****************************************************************************
  End of Dsk Wrapper for Watches Window
*****************************************************************************/

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
 if (!WtWindow || !WtCol || !WtList || !WtCol->getCount())
   {
    if (TInspector::getCountInspectors())
      {
       mi_gvar_chg *changed;
       if (!dbg || !dbg->ListChangedgVar(changed))
          return;
       message(TProgram::deskTop,evBroadcast,cmVarChanged,changed);
       mi_free_gvar_chg(changed);
      }
    return;
   }
 ccIndex c=WtCol->getCount(), i;
 int changedVars=0, deletedVars=0, withScope=0;
 // First pass: try to define pending vars and evaluate normal ones.
 for (i=0; i<c; i++)
    {
     gVar *gv=WtCol->At(i);
     if (gv->type==gvtNormal || !gv->v)
       {
        WtCol->Refresh(i);
        changedVars++;
       }
     else if (gv->type==gvtWScope)
        withScope++;
    }
 if (!withScope && !TInspector::getCountInspectors())
   {// Don't bother if we aren't using them
    if (changedVars)
       WtList->drawView();
    return;
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

 if (withScope)
   {
    ch=changed;
    while (ch)
      {
       for (i=0; i<c; i++)
          {
           gVar *gv=WtCol->At(i);
           if (gv->type==gvtWScope && gv->v && strcmp(gv->v->name,ch->name)==0)
             {
              // Mark used so TInspectors doesn't need to check it
              free(ch->name);
              ch->name=NULL;
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
   }
 if (changed)
    message(TProgram::deskTop,evBroadcast,cmVarChanged,changed);
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

// Special TSView to select a file
class TSChooseFile : public TSView
{
public:
 TSChooseFile(const char *aText, unsigned anId, int wForce, const char *aMask);
 virtual ~TSChooseFile() { delete linked; delete[] mask; };
 virtual void insert(TDialog *);
 virtual void setWidth(int aW);
 virtual void setGrowMode(unsigned val);
 unsigned getId() { return id; }
 void setFileName(char *s) { linked->view->setData(s); }
 char *mask;

protected:
 TSInputLine *linked;
 TSButton *bt;
 unsigned id;
};

static
int ButtonCallBack(unsigned , void *data)
{
 TSChooseFile *cf=(TSChooseFile *)data;
 char buf[wFilename];
 strcpy(buf,cf->mask);
 if (GenericFileDialog(__("Choose file"),buf,NULL,cf->getId(),fdNoMask)
     !=cmCancel)
   {
    cf->setFileName(buf);
   }
 return btcbGoOn;
}

TSChooseFile::TSChooseFile(const char *aText, unsigned anId, int wForce,
                           const char *aMask) :
   TSView()
{
 bt=new TSButton(__("Browse"),cmYes,bfNormal,ButtonCallBack,this);
 linked=new TSInputLine(wFilename,1,anId,wForce-bt->w);
 id=anId;
 mask=newStr(aMask);

 stTVIntl *cache=NULL;
 const char *str=TVIntl::getText(aText,cache);
 w=max(cstrlen(str)+1+bt->w,wForce);
 h=2;
 view=new TLabel(TRect(0,0,w,1),aText,linked->view,cache);
}

void TSChooseFile::insert(TDialog *d)
{
 TRect r(x-1,y,x-1+w-bt->w,y+1);
 view->locate(r);
 d->insert(view);
 linked->x=x;
 linked->y=y+1;
 linked->insert(d);
 bt->x=x+linked->w;
 bt->y=y;
 bt->insert(d);
}

void TSChooseFile::setWidth(int aW)
{
 w=aW-bt->w;
 linked->setWidth(w);
}

void TSChooseFile::setGrowMode(unsigned val)
{
 view->growMode=val;
 linked->setGrowMode(val);
 bt->setGrowMode(val);
}


TDialog *createDebugOpsDialog(TSViewCol *&cl)
{
 TSViewCol *col=new TSViewCol(__("Debug Options"));

 // EN: ADELMPRTU
 // ES: ADEIMPRTU
 TSVeGroup *o1=
 MakeVeGroup(0, // All together
   new TSChooseFile(__("~P~rogram to debug, with debug info"),
                    hID_DbgBinary,maxWBox,"*"),
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

 TDialog *d=col->doItCenter(hcDebugOps);
 cl=col;
 return d;
}

int TSetEditorApp::DebugOptionsEdit()
{
 if (!IsPrjOpened())
    GiveAdvice(gadvDbgNoPrj);
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

 TSViewCol *cl;
 int ret=0;
 if (execDialog(createDebugOpsDialog(cl),&edit)==cmOK)
   {
    memcpy(&dOps,&edit,sizeof(DebugOptionsStruct));
    ret=1;
   }
 delete cl;
 return ret;
}

TDialog *createDebugOpsAdvDialog(TSViewCol *&cl)
{
 TSViewCol *col=new TSViewCol(__("Advanced Debug Options"));

 // EN: BCFGILMSTX
 // ES:
 TSVeGroup *o1=
 MakeVeGroup(0, // All together
   new TSChooseFile(__("~G~DB executable"),hID_DbgGDB,wVisible,"gdb*"),
   new TSChooseFile(__("~X~ terminal executable"),hID_DbgXTerm,wVisible,"xterm*"),
   new TSLabel(__("~M~ain function"),
       new TSInputLine(wFunction,1,hID_DbgMainFunc,wVisible)),
   new TSHzGroup(
       new TSHzLabel(__("GDB ~T~ime out [s]"),new TSInputLine(wTimeOut)),
       new TSHzLabel(__("Max. ~l~ines in Debug window"),new TSInputLine(wLinesMsg))),
   TSLabelCheck(__("M~i~scellaneous"),
                __("No gdb ~b~anner after connecting"),
                __("Enable MI v2 ~f~eatures"),
                __("No ~s~ymbols bug workaround"),
                __("No source ~c~ode in disasm. window"),0),0);
 o1->makeSameW();

 col->insert(xTSLeft,yTSUp,o1);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(hcDebugAdvOps);
 // Hack: the TSChooseFile object have important data needed during the
 // execution of the dialog. We can't release this data right now and we
 // have to wait until the dialog is destroyed.
 //delete col;
 cl=col;
 return d;
}

struct DebugOptionsAdvStruct
{
 char gdb[wFilename];
 char xterm[wFilename];
 char main[wFunction];
 char to[wTimeOut];
 char lines[wLinesMsg];
 uint32 misc;
};

int TSetEditorApp::DebugOptionsAdv()
{
 // Get current values
 const char *gdb=GetGDBExe();
 const char *xterm=GetXTermExe();
 const char *mainf=GetMainFunc();
 unsigned to=GetGDBTimeOut();
 unsigned lines=GetMsgLines();
 unsigned misc=GetGDBMisc();

 // Transfer to the edit box
 DebugOptionsAdvStruct box;
 strncpyZ(box.gdb,gdb,wFilename);
 strncpyZ(box.xterm,xterm,wFilename);
 strncpyZ(box.main,mainf,wFunction);
 CLY_snprintf(box.to,wTimeOut,"%d",to);
 CLY_snprintf(box.lines,wLinesMsg,"%d",lines);
 box.misc=misc;

 TSViewCol *cl;
 // Edit
 if (execDialog(createDebugOpsAdvDialog(cl),&box)==cmOK)
   {// Set the changed values
    if (strcmp(box.gdb,gdb))
       SetGDBExe(box.gdb);
    if (strcmp(box.xterm,xterm))
       SetXTermExe(box.xterm);
    if (strcmp(box.main,mainf))
       SetMainFunc(box.main,True);
    unsigned bto=atoi(box.to);
    if (bto<2) bto=2;
    if (bto!=to)
       SetGDBTimeOut(bto);
    unsigned blines=atoi(box.lines);
    if (blines<100) blines=100;
    if (blines!=lines)
       SetMsgLines(blines);
    if (box.misc!=misc)
       SetGDBMisc(box.misc);
    return 1;
   }
 delete cl;
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

const unsigned ceBreakpoints=1, ceWatchpoints=2, ceWatches=4, ceInspectors=8,
               ceDataWindows=16, ceAll=32-1;

static
void CloseNonEd(TDskWin *win, void *)
{
 TDskDataWin *w=(TDskDataWin *)win;
 message(w->view,evCommand,cmClose,NULL);
}

void TSetEditorApp::DebugCleanElem()
{
 if (!DebugCheckStopped())
    return;

 TSViewCol *col=new TSViewCol(__("Clean elements"));

 // EN: ABDIW
 col->insert(xTSCenter,yTSUp,
             TSLabelCheck(__("Elements"),
                          __("~B~reakpoints"),
                          __("~W~atchpoints"),
                          __("W~a~tches"),
                          __("~I~nspectors"),
                          __("~D~ata windows"),0));
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(cmeDbgCleanElem);
 delete col;

 uint32 what=ceAll;
 if (execDialog(d,&what)==cmOK)
   {
    int c, i;

    if (what & ceBreakpoints)
      {
       c=bkpts.GetCount();
       for (i=0; i<c; i++)
           TDiagBrk::Delete(i);
      }
    if (what & ceWatchpoints)
      {
       c=wpts.GetCount();
       for (i=0; i<c; i++)
           TDiagWp::Delete(i);
      }
    if (what & ceWatches)
       WatchesDeInit();
    if (what & ceInspectors)
       TSetEditorApp::edHelper->forEachNonEditor(dktDbgIns,CloseNonEd,NULL);
    if (what & ceDataWindows)
       TSetEditorApp::edHelper->forEachNonEditor(dktDbgDataWin,CloseNonEd,NULL);
   }
}

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

static
void SaveInspector(TDskWin *win, void *data)
{
 TDskInspector *w=(TDskInspector *)win;
 opstream *os=(opstream *)data;
 w->saveData(*os);
}

static
void SaveDW(TDskWin *win, void *data)
{
 TDskDataWin *w=(TDskDataWin *)win;
 opstream *os=(opstream *)data;
 w->saveData(*os);
}

static
void CountNonEditors(TDskWin *, void *data)
{
 int *i=(int *)data;
 (*i)++;
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
       os << msgWinVersion << (int)(TProgram::deskTop->indexOf(MsgWindow))
          << (char)((MsgWindow->state & sfVisible) ? svYes : svNo);
       SaveExpandedRect(os,MsgWindow->zoomRect,wS,hS);
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
       os << watchWinVersion << (int)(TProgram::deskTop->indexOf(WtWindow))
          << (char)((WtWindow->state & sfVisible) ? svYes : svNo);
       SaveExpandedRect(os,WtWindow->zoomRect,wS,hS);
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
 // Inspectors
 os << inspVersion;
 int inspectors=0;
 TSetEditorApp::edHelper->forEachNonEditor(dktDbgIns,CountNonEditors,&inspectors);
 os << inspectors;
 if (inspectors)
    TSetEditorApp::edHelper->forEachNonEditor(dktDbgIns,SaveInspector,&os);
 // Data windows
 os << dwVersion;
 int dataWindows=0;
 TSetEditorApp::edHelper->forEachNonEditor(dktDbgDataWin,CountNonEditors,&dataWindows);
 os << dataWindows;
 if (dataWindows)
    TSetEditorApp::edHelper->forEachNonEditor(dktDbgDataWin,SaveDW,&os);
 // Main Function
 os << svPresent;
 os.writeString(mainFunction);
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
       if (aux>1)
          ReadExpandedRect(is,MsgWindow->zoomRect,wS,hS);
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
       if (aux>1)
          ReadExpandedRect(is,WtWindow->zoomRect,wS,hS);
      }
   }
 // Breakpoints
 DBG_ReadBreakpoints(is);
 // Watchpoints
 is >> aux;
 if (!aux)
    return;
 DBG_ReadWatchpoints(is,aux);
 // Inspectors
 is >> aux;
 if (!aux)
    return;
 int inspectors;
 is >> inspectors;
 for (int i=0; i<inspectors; i++)
     TDskInspector::readData(is,aux);
 // Data Windows
 is >> aux;
 if (!aux)
    return;
 int dataWindows;
 is >> dataWindows;
 for (int i=0; i<dataWindows; i++)
     TDskDataWin::readData(is,aux);
 // Main Function
 is >> aux;
 if (!aux)
    return;
 char *mf=is.readString();
 SetMainFunc(mf,False);
 // No more data
 is >> aux;
}

/*****************************************************************************
  End of Persistence.
*****************************************************************************/

/*****************************************************************************
  End of Data Window
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
void TSetEditorApp::DebugWatchExp(Boolean , char *) {}
void TSetEditorApp::DebugDeInitVars() {}
Boolean TSetEditorApp::DebugCloseSession(Boolean ) { return True; }
int  TSetEditorApp::DebugCheckAcceptCmd(Boolean ) { return 0; }
int  TSetEditorApp::DebugCheckStopped(Boolean ) { return 1; }
void TSetEditorApp::DebugEditBreakPts() {}
void TSetEditorApp::DebugEditWatchPts(char *startVal) { delete[] startVal; }
void TSetEditorApp::DebugInspector(char *startVal) { delete[] startVal; }
void TSetEditorApp::DebugDataWindow(char *startVal) { delete[] startVal; }
void TSetEditorApp::DebugCleanElem() {}
void TSetEditorApp::DebugThreadSel() {}
int  TSetEditorApp::DebugTimeOut(void *) { return 0; }
int  TSetEditorApp::DebugOptionsAdv() { return 0; }
void TSetEditorApp::DebugDisAsmWin() {}
void TSetEditorApp::DebugStackWindow() {}
void TSetEditorApp::DebugMoveBreakPts() {}
void TSetEditorApp::DebugDetach() {}
void DebugSetCPULine(int , char *) {}
void TSetEditorApp::DebugPoll() {}
void DebugReadData(ipstream &) {}

void DBG_AddPathForSources(const char *) {}

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
__("Time out in gdb response")
__("GDB suddenly died")
__("Can't execute X terminal")
__("Failed to create temporal")
__("Can't execute the debugger")
#endif

