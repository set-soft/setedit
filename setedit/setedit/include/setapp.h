/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifdef Uses_SETAppAll
#define Uses_SETAppConst
#define Uses_TSetEditorApp
#define Uses_SETAppDialogs
#define Uses_SETAppVarious
#define Uses_SETAppProject
#define Uses_SETAppFiles
#endif

#if defined(Uses_SETAppConst) && !defined(__SETAppConst__)
#define __SETAppConst__
const int cmeBase=0x2500;
const int
  cmeOpen           = cmeBase+0,
  cmeNew            = cmeBase+1,
  cmeChangeDrct     = cmeBase+2,
  cmeDosShell       = cmeBase+3,
  cmeCalculator     = cmeBase+4,
  cmeShowClip       = cmeBase+5,
  cmeInfView        = cmeBase+6,
  cmeListWin        = cmeBase+7,
  cmeLastHelp       = cmeBase+8,
  cmeUserScreen     = cmeBase+9,
  cmeAnotherInfView = cmeBase+10,
  cmeOpenPrj        = cmeBase+11,
  cmeClosePrj       = cmeBase+12,
  cmeSDG            = cmeBase+13,
  cmeSDGDialog      = cmeBase+14,
  cmeSetColors      = cmeBase+15,
  cmeQuitDelete     = cmeBase+16,
  cmeQuit           = cmeBase+17,
  cmeResize         = cmeBase+18,
  cmeZoom           = cmeBase+19,
  cmeTile           = cmeBase+20,
  cmeCascade        = cmeBase+21,
  cmeNext           = cmeBase+22,
  cmePrev           = cmeBase+23,
  cmeClose          = cmeBase+24,
  cmeEditKeyBind    = cmeBase+25,
  cmeLoadKeyScans   = cmeBase+26,
  cmeSetUpAltKeys   = cmeBase+27,
  cmeKbBackDefault  = cmeBase+28,
  cmeSeeScanCodes   = cmeBase+29,
  cmePrintEditor    = cmeBase+30,
  cmeSetUpPrinter   = cmeBase+31,
  cmeGrepDialog     = cmeBase+32,
  cmeNextMessage    = cmeBase+33,
  cmePrevMessage    = cmeBase+34,
  cmeSetScreenOps   = cmeBase+35,
  cmeEditPalette    = cmeBase+36,
  cmeEdGralOptions  = cmeBase+37,
  cmeRunCommand     = cmeBase+38,
  cmeConfRunCommand = cmeBase+39,
  cmeTipOfTheDay    = cmeBase+40,
  cmeAboutBox       = cmeBase+41,
/* These constants are defined in mp3/intermp3.h so any change here must
   be done there I did it because the editor NEVER uses the commands
   directly.
  cmeMP3Open        = cmeBase+42,
  cmeMP3Convert     = cmeBase+43,
  cmeMP3EditPlayList= cmeBase+44,
  cmeMP3Start       = cmeBase+45,
  cmeMP3Stop        = cmeBase+46,
  cmeMP3PlayList    = cmeBase+47,
  cmeMP3StopList    = cmeBase+48,
*/
  cmeKeyPadBehavior = cmeBase+49,
  cmeSyntaxHelp     = cmeBase+50,
  cmeSyntaxHelpOps  = cmeBase+51,
  cmeSyntaxHelpFiles= cmeBase+52,
  cmeHTMLAccents    = cmeBase+53,
  cmePocketCalc     = cmeBase+54,
  cmeDeleteBkps     = cmeBase+55,
  cmeOpenROCopy     = cmeBase+56,
  cmeEditUserWords  = cmeBase+57,
  cmeEditDeflOpts   = cmeBase+58,
  cmeASCIIChart     = cmeBase+59,
  cmeCalendar       = cmeBase+60,
  cmeFileOpenOptions= cmeBase+61,
  cmeScreenSaverOpts= cmeBase+62,
  cmeManPageView    = cmeBase+63,
  cmeExportAsHTML   = cmeBase+64,
  cmeRemapCodePage  = cmeBase+65,
  cmeKeyboardSetUp  = cmeBase+66, // No longer used
  cmeReDraw         = cmeBase+67,
  cmeHTMLTag2Accent = cmeBase+68,
  cmeSavePrj        = cmeBase+69,
  cmeBoardMixer     = cmeBase+70,
  cmeStopChild      = cmeBase+71,
  cmeEditNoBkp      = cmeBase+72,
  cmeIncludeList    = cmeBase+73,
  cmeColorTheme     = cmeBase+74,
  cmeSaveDesktop    = cmeBase+75,
  cmeEncodings      = cmeBase+76,
  cmeFonts          = cmeBase+77,
  cmeSaveAll        = cmeBase+78,
  cmeTagFiles       = cmeBase+79,
  cmeSearchTag      = cmeBase+80,
  cmeClassBrowser   = cmeBase+81,
  cmeWordCompletion = cmeBase+82,
  cmeExportPrj      = cmeBase+83,
  cmeImportPrj      = cmeBase+84,
  cmeTagsOps        = cmeBase+85,
  cmeHolidaysConf   = cmeBase+86,
  cmeSetModiCkOps   = cmeBase+87,
  cmeAdviceDiagConf = cmeBase+88,
  cmeSelWindow1     = cmeBase+89,
  cmeSelWindow2     = cmeBase+90,
  cmeSelWindow3     = cmeBase+91,
  cmeSelWindow4     = cmeBase+92,
  cmeSelWindow5     = cmeBase+93,
  cmeSelWindow6     = cmeBase+94,
  cmeSelWindow7     = cmeBase+95,
  cmeSelWindow8     = cmeBase+96,
  cmeSelWindow9     = cmeBase+97,
  cmeSelWindow10    = cmeBase+98,
  cmeSelWindow11    = cmeBase+99,
  cmeSelWindow12    = cmeBase+100,
  cmeSelWindow13    = cmeBase+101,
  cmeSelWindow14    = cmeBase+102,
  cmeSelWindow15    = cmeBase+103,
  cmeSelWindow16    = cmeBase+104,
  cmeSelWindow17    = cmeBase+105,
  cmeSelWindow18    = cmeBase+106,
  cmeSelWindow19    = cmeBase+107,
  cmeSelWinPrj      = cmeBase+108,
  cmeSelWinMessage  = cmeBase+109,
  // Debugger interface
  // Not a command but a mark
  cmeDbgFirstCommand= cmeBase+110,
  //------------------------------
  cmeBreakpoint     = cmeBase+110,
  cmeDebugOptions   = cmeBase+111,
  cmeDbgRunContinue = cmeBase+112,
  cmeDbgStepOver    = cmeBase+113,
  cmeDbgTraceInto   = cmeBase+114,
  cmeDbgGoToCursor  = cmeBase+115,
  cmeDbgFinishFun   = cmeBase+116,
  cmeDbgReturnNow   = cmeBase+117,
  cmeDbgStop        = cmeBase+118,
  cmeDbgKill        = cmeBase+119,
  cmeDbgCallStack   = cmeBase+120,
  cmeDbgEvalModify  = cmeBase+121,
  cmeDbgOptsMsgs    = cmeBase+122,
  cmeDbgWatchExpNorm= cmeBase+123,
  cmeDbgWatchExpScp = cmeBase+124,
  cmeDbgEditWatchPts= cmeBase+125,
  cmeDbgInspector   = cmeBase+126,
  cmeDbgEndSession  = cmeBase+127,
  cmeDbgCloseSession= cmeBase+128,
  cmeDbgGoConnected = cmeBase+129,
  cmeDbgGoReadyToRun= cmeBase+130,
  cmeDbgEditBreakPts= cmeBase+131,
  cmeDbgDataWindow  = cmeBase+132,
  cmeDbgStackWindow = cmeBase+133,
  cmeDbgCleanElem   = cmeBase+134,
  cmeDbgThreadSel   = cmeBase+135,
  cmeDbgOptionsAdv  = cmeBase+136,
  cmeDbgDisAsmWin   = cmeBase+137,
  cmeDbgDetach      = cmeBase+138,
  // That isn't a command, is to know the last+1.
  cmeDbgLastCommand = cmeBase+139,
  //-------------------------------------------------------------------------
  // That's messy, I'm reserving some commands for the "debug" group
  //-------------------------------------------------------------------------
  cmeSourceList     = cmeBase+160,
  cmeGPushCursorPos = cmeBase+161,
  cmeGPopCursorPos  = cmeBase+162,
  cmeSelDebugWin    = cmeBase+163,
  cmeSelWatchesWin  = cmeBase+164,
  cmeTagsAutoRegen  = cmeBase+165;

#endif

// TScOptsCol used to hold the screen options for each video driver.
#if defined(Uses_TScOptsCol) && !defined(Defined_TScOptsCol)
#define Defined_TScOptsCol 1
struct stScreenOptions;

class TScOptsCol : public TSortedCollection
{
public:
 TScOptsCol() : TSortedCollection(8,2) {};
 virtual void *keyOf(void *item);
 virtual int   compare(void *key1, void *key2);
 virtual void  freeItem(void* item);
 void Insert(stScreenOptions *p);
 void transfer2TV();
 SetDefStreamMembers(TScOptsCol,TSortedCollection)
};
SetDefStreamOperators(TScOptsCol)
#endif

#if (defined(Uses_TMultiMenu) || defined(Uses_TMultiMenuBar)) && \
    !defined(TMultiMenu_defined)
#define TMultiMenu_defined
class TMultiMenu : public TMenu
{
public:
 TMultiMenu() : TMenu(), next(NULL), last(NULL) {};
 ~TMultiMenu();
 void add(TMenuItem *m);

 TMultiMenu *next;
 TMenuItem *last;
 unsigned min, max;
};

class TMultiMenuBar : public TMenuBar
{
public:
 TMultiMenuBar(const TRect &bounds, TMultiMenu *aMenu) :
  TMenuBar(bounds,aMenu) { menuList=aMenu; helpCtx=(unsigned)-1; };
 TMultiMenuBar(const TRect &bounds, TMultiMenu *aMenu, TMultiMenu *cur) :
  TMenuBar(bounds,cur) { menuList=aMenu; helpCtx=(unsigned)-1; };
 ~TMultiMenuBar();
 void update();
 void findMenu();
 static TMultiMenuBar *createMultiMenuBar(const TRect &bounds,
                                          TMultiMenu *aMenu);

 unsigned helpCtx;
 TMultiMenu *menuList;
};

int LoadMenuAndStatus(char *fileName, int forceReload=0);
TMultiMenuBar *GetTVMenu(char *fileName, TRect &rect);
TStatusLine   *GetTVStatusLine(char *fileName, TRect &rect);
#endif

#if defined(Uses_TSetEditorApp) && !defined(__TSetEditorApp__)
#define __TSetEditorApp__
class TMenuBar;
class TMultiMenuBar;
class TMenu;
class TStatusLine;
class TEditWindow;
class TDialog;
class fpstream;
class TDeskTopClock;
class TDskWinHelp;
class TApplication;
class TCEditWindow;
class TVFontCollection;
struct EditorResume;
struct TScreenFont256;
struct TVBitmapFontSize;
class TEditorCollection;
#ifndef Defined_TScOptsCol
class TScOptsCol;
#endif

class XYFElement
{
public:
 XYFElement(uint32 aX, uint32 aY, const char *aFile)
  { x=aX; y=aY; file=newStr(aFile); next=prev=NULL; }
 ~XYFElement()
  { delete[] file; delete next; }

 uint32 x, y;
 char *file;
 XYFElement *next, *prev;
};

// I think that more than 32 means the user is leaking ;-)
const int maxXYFStack=32;

class XYFStack
{
public:
 XYFStack()
  { first=last=NULL; count=0; }
 ~XYFStack()
  { delete first; }

 void Push(uint32 aX, uint32 aY, const char *aFile);
 XYFElement *Pop();
 int getCount() { return count; }

protected:
 XYFElement *first, *last;
 int count;
};

const int extscrsParMxLen=80;

struct stScreenOptions
{
 char *driverName;
 // Encoding options
 // !=0 if the encoding is forced. App=Application, Scr=Screen, Snd=Second Font
 // Inp=Input.
 uchar enForceApp, enForceScr, enForceSnd, enForceInp;
 // Which encoding is used, -1 means default one.
 int   enApp, enScr, enSnd, enInp;

 // Fonts options
 // Which the user wants to loaded (Pri=Primary, Sec=Secondary)
 uchar foPriLoad, foSecLoad;
 // Names of the fonts
 char *foPriName, *foSecName;
 // Names of the font files
 char *foPriFile, *foSecFile;
 // Size of the primary (secondary must be of the same size)
 unsigned foPriW,foPriH;
 // The following are only valid at run-time
 // Which one is really loaded
 uchar foPriLoaded, foSecLoaded;
 // Font callback installed
 uchar foCallBackSet;
 // Font collections
 TVFontCollection *foPri, *foSec;

 // Screen size options
 // Behavior (set mode, use last resolution, etc.)
 uint32 scOptions;
 // Desired size
 unsigned scWidth, scHeight;
 // Desired char cell size
 unsigned scCharWidth, scCharHeight;
 // Video mode
 unsigned scModeNumber;
 // External program
 char *scCommand;

 // Is the palette different than the default one?
 char palChanged;
 // The screen colors palette, this is used during the load process.
 // The real palette is handled by the TTextPalette class.
 TScreenColor palette[16];
};

const uint32 scfDontForce=0, scfSameLast=1, scfExternal=2, scfForced=3, scfMode=4;

class TSetEditorApp : public TApplication
{
public:
    TSetEditorApp();
    ~TSetEditorApp();

    virtual void handleEvent( TEvent& event );
    static TMenuBar *initMenuBar( TRect );
    static TStatusLine *initStatusLine( TRect );
    virtual void outOfMemory();
    virtual TPalette& getPalette() const;
    TCEditWindow *openEditor(char *fileName, Boolean visible, EditorResume *res=NULL,
                             int options=0);

    void saveDesktop(const char *fName, int makeBkp);
    void storeDesktop(fpstream& s);
    Boolean loadDesktop(fpstream& s, Boolean isLocal);
    static Boolean retrieveDesktop(TSetEditorApp *app, const char *name,
                                   Boolean isLocal, int preLoad);
    static Boolean preLoadDesktop(fpstream &s);
    static void    preLoadDesktop(char *name=0, int haveFilesCL=0);
    static void    finishPreLoadDesktop();
    static void    loadOldFontInfo(fpstream& s, stScreenOptions *scrOps);
    static void    loadEditorDesktop(int LoadPrj, char *name=0,
                                     int haveFilesCL=0, int preLoad=0);
    static void    transferSetting2TV(void *p, void *arg=NULL);
    static void    displayDetectCallBack();
    static void    hotApplyScreenOptions();
    static int     resetVideoMode(Boolean redraw=False);
    static stScreenOptions *loadOldDesktopScreenInfo(fpstream &s);
    void ShowUserScreen(TEvent &event);
    void createClipBoard(void);
    virtual void idle();
    virtual void getEvent(TEvent& event);
    void screenSaver();
    static void setCmdState(uint16 command,Boolean enable);
    void GetContextHelp(void);
    void pocketCalculator(void);
    void RemapCodePageEd(void);
    // Screen options routines
    static void EncodingOptions(void);
    static void FontsOptions(void);
    static void ScreenOptions(void);
    static void SetEditorFontsEncoding(int priChanged, int enPri, int sndChanged, int enSec);
    static TScreenFont256 *FontRequestCallBack(int which, unsigned w, unsigned h);
    static void SetEditorFonts(uchar priUse, char *priName, char *priFile,
                               TVBitmapFontSize *priSize,
                               uchar secUse, char *secName, char *secFile);
    static int  ChooseConvCPs(int &From, int &To, uint32 &ops);
    static char *CreateTitle(const char *title);

    static unsigned long deskTopVersion;

    static char ShowClock;
    static char UseScreenSaver;
    static char UseExternPrgForMode;
    static char DesktopPreloaded;
    static char *WhichScrSaver;
    static char ExtScrSaverOpts[extscrsParMxLen];
    static int  screenSaverTime;
    static int  screenSaverTimeMouse;
    static char ExternalPrgMode[80];
    static struct stScreenOptions *so;
    static TScOptsCol *soCol;
    void KillClock();

    void   ShowHelpTopic(char *file, char *node);
    static TDskWinHelp *InfManager;
    static TEditorCollection *edHelper;
    static TCEditWindow *clipWindow;
    static int helpRequest;
    static ushort helpCtxRequested;
    static int maxOpenEditorsSame;
    static int DeleteFilesOnExit;
    static unsigned geFlags;
    static int widthVertWindows;
    static TVCodePageCallBack oldCPCallBack;
    static void cpCallBack(ushort *map);
    static uint32 getModifFilesOps() { return modifFilesOps; }

    void tile();
    void cascade();
    void SetTitle(const char *str1=0, const char *str2=0);

    // Debugger interface
    static void DebugToggleBreakpoint();
    static int  DebugOptionsEdit();
    static int  DebugInitVars();
    static void DebugDeInitVars();
    static int  DebugCheckStopped(Boolean showConnect=False);
    static int  DebugCheckAcceptCmd(Boolean showConnect=False);
    static int  DebugConnect();
    static int  DebugSelectTarget(Boolean showConnect=False);
    static void DebugUpdateCommands();
    static void DebugRunOrContinue();
    static void DebugPoll();
    static void DebugStepOver();
    static void DebugTraceInto();
    static void DebugGoToCursor();
    static void DebugFinishFun();
    static void DebugReturnNow();
    static void DebugStop();
    static void DebugKill();
    static void DebugCallStack();
    static void DebugEvalModify(char *startVal);
    static char *DebugEvalExpression(char *exp);
    static char *DebugModifyExpression(char *exp, char *newVal);
    static void DebugOptsMsgs();
    static void DebugWatchExp(Boolean wScope, char *val);
    static int  DebugConfirmEndSession(Boolean directRequest=False);
    static Boolean DebugCloseSession(Boolean confirm=True);
    static void DebugCommandsForDisc();
    static void DebugCommonCleanUp();
    static void DebugEditBreakPts();
    static void DebugEditWatchPts(char *startVal);
    static void DebugMoveBreakPts();
    static void DebugInspector(char *startVal);
    static void DebugDataWindow(char *startVal);
    static void DebugStackWindow();
    static void DebugCleanElem();
    static void DebugThreadSel();
    static int  DebugTimeOut(void *data);
    static int  DebugOptionsAdv();
    static void DebugDisAsmWin();
    static void DebugDetach();

protected:

    static void SetModifCheckOptions();
    virtual void fileOpen();
    void fileOpenCopy();
    void fileNew();
    void changeDir();
    virtual void dosShell();
    void showClip();

    static clock_t LastTimeUpdate;
    static TDeskTopClock *Clock;
    void CreateClock();

    char *OriginalWindowTitle;

    static CLY_StreamPosT posPreload;
    static char fontCreated;

    static uint32 modifFilesOps;

    static TMultiMenuBar *multiMenuBar;
};

// modifFilesOps:
const uint32 mfoDontCheckAfterRun=1, mfoDontCheckInIdle=2;
const int oedNoSelect=1,oedForceRO=2,oedZoom=4,oedForgetResume=8,
          oedDontOpenEmpty=16;
const unsigned geVertWindows=1,geRightSide=2,geAvoidPrjAndMsg=0x80000000,
               geZoomedIfNoPrj=0, geZoomedNPorZ=0x10, geAlwaysZoomed=0x20,
               geMask1=3, geMask2=0x30, geShift2=4;

extern TSetEditorApp *editorApp;
#endif

#ifdef Uses_SETAppDialogs
#include <stdarg.h>

ushort execDialog( TDialog *d, void *data );
unsigned doEditDialog( int dialog, va_list first );
extern void executeCalc(char *startVal=NULL);
const int maxGrepString=200;
extern void grepWindow(char *patStart);
extern void SetGeneralEditorOptions(void);
extern int  SetFileOpenDialogOptions(void);
extern void SetScreenSaversOptions(void);
extern int  AboutStartBox(void);
extern void FullAboutBox(void);
extern void ShowUserScreenDialog();
class TDskWinMan;
extern TDskWinMan *ManPageView(const char *name); // From dskman.cc
extern const char *cmeQuitDeleteMessage;
#endif

#ifdef Uses_SETAppVarious
class fpstream;
class TStringCollection;
class TDskWinHelp;
class TCEditor;
class TCEditWindow;
class TDskWin;
struct stEditorId;

const unsigned gflCPULine=1, gflDontSelect=2;

extern TView *setFocusTo;
extern Boolean focusChanged;
extern void CopyHelp2Clip(char *b, long l);
extern void ShowHelpTopic(char *file, char *node);
extern void closeView(TView *p, void *p1);
extern int ShowFileLine(int line,char *name);
extern int GotoFileLine(int line,char *name,char *msg=0,int off=-1,int len=0,
                        unsigned flags=0);
extern int GotoFileText(char *search, char *file, char *msg=0, int off=-1, int len=0);
extern TCEditWindow *GetEditorWindowForFile(char *file);
extern void GPushCursorPos();
extern void GPopCursorPos();
extern void SetScreenOps(void);
extern void EditPalette(void);
extern void SaveEnviromentFile(void);
extern void ConfigureRunCommand(void);
extern void SaveRunCommand(fpstream &s);
extern void LoadRunCommand(fpstream &s);
extern void SaveGrepData(fpstream &s);
extern void LoadGrepData(fpstream &s);
extern void ReLoadModifEditors(void);
extern int  EdReloadIfOpened(const char *name, stEditorId *id);
extern int  AskReloadEditor(TCEditWindow *edw);
extern int  ResetVideoMode(int mode,int redraw=1);
const int scsvInternal=0, scsvExternal=1;
extern TStringCollection *GetScreenSaverList(int forceReRead=0);
extern void *WichSaverIs(char *name, int &type);
extern char *GetDefaultScreenSaver(void);
extern char *GetScrSaverInfo(char *name);
extern char *GetScrSaverHelp(char *name);
extern void  ShowHelpTopic(char *file, char *node);
extern void ShowTips(char *fileName, int forceDialog=0);
extern int IsAnEditor(void *p);
extern TCEditor *GetCurrentIfEditor();
extern void UpdateEditorCommands();
extern int SelectWindowNumber(int number);
extern void HTMLAcc_Entry(Boolean compact);
TCEditWindow *IsAlreadyOnDesktop(char *fileName, int *cant=0, stEditorId *id=0);
extern void ASCIIWindow();
extern void CalendarWindow();
//extern void RemoveFromEditorsHelper(TView *p, int type);
extern void AddNonEditorToHelper(TDskWin *p);
extern unsigned doEditDialogLocal(int dialog, ...);
extern void BringListOfWindows(void);
extern void FinishFocusChange();
extern int  GetMaxWindowNumber();

extern char *TranslateCommandToMacro(unsigned command);
extern int RegisterMacroCommand(char *name);
extern int isAMacroInMenu(unsigned command);
extern void UnRegisterMacroCommands(void);

// From edcolor.cc
extern void Colors();
extern void ColorTheme();

void FullSuspendScreen();
void FullResumeScreen();
// Open a file and insert it in the desktop
extern void OpenFileFromEditor(char *fullName);
// Defined in dstfile.cc to indicate we loaded the desktop file from the current
// directory and not from the default one.
extern char DstLoadedHere;
#endif

#ifdef Uses_SETAppProject
class TRect;

extern void OpenProject(char *name=NULL, int preLoad=0);
extern void CloseProject(int openDesktop);
extern void SaveProject();
extern int  IsPrjOpened();
extern int  IsPrjVisible();
extern int IsPrjZoomed();
extern void SaveDesktopHere(void);
extern void ExportProjectItems();
extern void ImportProjectItems();
const unsigned wnopEspaceSep=0, wnopLineSep=1;
extern int WriteNamesOfProjectTo(FILE *f, unsigned mode=wnopEspaceSep);
extern int WriteNamesOfProjectToTime(FILE *f, time_t timeT, uint32 targetMask=0);
extern int ClearForceTargetBits(uint32 bits);
extern int SetForceTargetBits(uint32 bits);
extern void ProjectInsertAutoTagFiles();
extern Boolean ProjectGetSize(TRect &r);
extern char *GetRelIfFileInPrj(char *name);
extern char *GetAbsForNameInPrj(const char *name);
extern void ProjectApplyToItems(ccAppFunc action, void *arg);
extern void ProjectGetNameFromItem(void *p, char *dest, int size);

// The forceTarget is a bitmap for each target suported by the project.
// Currently the only target is the TAGs file.
// An item with forceTarget bit 1 means the entry is new and we should
// take as newer than the target.
const uint32 prjtTags=1, prjtAllTargets=1;
#endif

#ifdef Uses_SETAppFiles
extern int FileOpenDialog(char *title, char *file);
#define DeskTopFileName       "tcedit.dst"
#define DeskTopFileNameHidden ".tcedit.dst"
#define CLEFile               "errors.cle"
#define NBKPFile              "nobkp.txt"
extern const char *cDeskTopFileName;
extern const char *cDeskTopFileNameHidden;
#if defined(TVOS_UNIX) || defined(TVCompf_Cygwin)
 #define HIDDEN_DIFFERENT
#endif
#define ProjectFileExt  ".epr"
#define DeskTopFileExt  ".dst"
extern int edTestForFile(const char *name);
#endif

#ifdef Uses_SETAppHelper
class TDskWin;
int SearchInHelper(int ,void *);
TDskWin *SearchInHelperWin(int type, void *p);
void AddNonEditorToHelper(TDskWin *p);
void SaveAllEditors(void);
#endif

