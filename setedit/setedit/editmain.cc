/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_BestWrite
#include <ceditint.h>

#ifdef TVCompf_djgpp
#include <conio.h>
#include <io.h>
#include <dpmi.h>
#endif

#define Uses_TCEditWindow
#define Uses_TCEditor_Commands
#define Uses_TCEditor_Internal
#define Uses_TApplication
#define Uses_TWindow
#define Uses_TPalette
#define Uses_TDeskTop
#define Uses_TRect
#define Uses_TFileDialog
#define Uses_TChDirDialog
#define Uses_TStringCollection
#define Uses_MsgBox
#define Uses_TCollection
#define Uses_TCommandSet
#define Uses_TScreen
#define Uses_TDeskTopClock
#define Uses_TMenuBar
#define Uses_TCalculator
#define Uses_TFileCollection // To set the default sorting
#define Uses_TNoSortedStringCollection
#define Uses_TStreamableClass
#define Uses_TGKey
#define Uses_TSubMenu
#define Uses_TMenu
#define Uses_TVCodePage
#define Uses_string
#define Uses_alloca
#define Uses_stdlib
#define Uses_access
#define Uses_getopt
#define Uses_ctype
#define Uses_unistd
#define Uses_ProgBar  // Needed for recoding stuff only!
#define Uses_TVConfigFile
#define Uses_TScOptsCol
#ifndef SECompf_djgpp
 #define Uses_TGKey
#endif
#define Uses_FileOpenAid
// InfView requests
#include <infr.h>
#include <ceditor.h>
#define Uses_SETAppAll
#define Uses_SETAppHelper
#define Uses_PrjFunctions
#define Uses_TMultiMenu
#include <setapp.h>
#include <dskwin.h>
#include <dskedito.h>
#include <dskclose.h>
#include <dskhelp.h>
#include <dskman.h>
#include <edcollec.h>
#include <sdginter.h>
#include <edprint.h>
#include <edmsg.h>
#include <tpaltext.h>
#include <diaghelp.h>
#include <splinman.h>
#include <pathtool.h>
#include <intermp3.h>
#include <ssyntax.h>

#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <loadkbin.h>
#include <runprog.h>
#include <loadcle.h>
#include <mixer.h>
#include <bufun.h>
#include <edspecs.h>
#include <stackdbg.h>
#include <loadshl.h>
#include <loadnobkp.h>
#include <pathlist.h>
#include <datetools.h>
#define Uses_TagsOnlyFuncs
#include <tags.h>
#include <rhutils.h>
#include <advice.h>
#include <debug.h>

void AddToEditorsHelper(TCEditWindow *p, int SelectHL=0);
static void PrintEditor(void);
static void ExportAsHTML(void);
int  AskForClosedResume(EditorResume *r,char *fileName);
extern int  AskForProjectResume(EditorResume *r,char *fileName);
void InitEditorSignals(char aStrategy, const char *prgName, const char *errFile);
void ShowMenuLoadError(void);
void UnLoadTVMenu(void);
void SLPInterfaceInit(char *);
void SLPInterfaceDeInit(void);
// RHTV setting
extern void setIntenseState(void);

// That's the manager for InfView, we don't worry about the viewer all is handled by it
TDskWinHelp       *TSetEditorApp::InfManager=NULL;
TEditorCollection *TSetEditorApp::edHelper=NULL;
TCEditWindow      *TSetEditorApp::clipWindow=NULL;
TMultiMenuBar     *TSetEditorApp::multiMenuBar=NULL;
// For the tricky context latch
int      TSetEditorApp::helpRequest=0;
ushort   TSetEditorApp::helpCtxRequested=0;
int      TSetEditorApp::maxOpenEditorsSame=1;
int      TSetEditorApp::DeleteFilesOnExit=0;
char     TSetEditorApp::ExtScrSaverOpts[extscrsParMxLen]="";
unsigned TSetEditorApp::geFlags=0;
int      TSetEditorApp::widthVertWindows=24;
unsigned long
         TSetEditorApp::deskTopVersion;
uint32   TSetEditorApp::modifFilesOps=0;

XYFStack stkPos;
const char *KeyBindFName="keybind.dat";
// Name specified by the user
static char *KeyBindFNameUser=0;

const char *cmeQuitDeleteMessage=__("Do you want to delete all the .BKP, desktop and project files?");

const char *TipsFName="editor.tip";
#ifdef NoHomeOrientedOS
// Used to guess the info directory in DOS
static char *EditorFileExt="setedit.inf";
#endif
char *EditorFile="setedit";
DeclarePalette;

TSetEditorApp *editorApp;
#ifdef TEST_SPLINES
int spLines[]={10-1,11-1,20-1,splEndOfList};
#endif
#ifdef USE_TSTRCOL
TStrCol *ReservedWords;
TStrCol *UserWords;
TStrCol *PascalRWords;
TStrCol *ClipperRWords;
#else
TStringCollection *ReservedWords;
TStringCollection *UserWords;
TStringCollection *PascalRWords;
TStringCollection *ClipperRWords;
#endif
static int StdErrOri=STDERR_FILENO,StdErrNew=-1;
static char *TemporalStdErr=0;

extern void InsertProjectWindow(void);
void StopStdErrRedirection();

//
// closeView() function
//
void closeView(TView *p, void *p1)
{
 if (p)
    message(p, evCommand, cmClose, p1);
}

#include <pal.h>

char SEcpColor[]     ={ SE_cpColor 0 };
char SEcpBlackWhite[]={ SE_cpBlackWhite 0 };
char SEcpMonochrome[]={ SE_cpMonochrome 0 };

TPalette& TSetEditorApp::getPalette() const
{
 static TPalette color ( SEcpColor, sizeof( SEcpColor )-1 );
 static TPalette blackwhite( SEcpBlackWhite, sizeof( SEcpBlackWhite )-1 );
 static TPalette monochrome( SEcpMonochrome, sizeof( SEcpMonochrome )-1 );
 static TPalette *palettes[] =
     {
     &color,
     &blackwhite,
     &monochrome
     };
 return *(palettes[appPalette]);
}

static
void FixUpName(char *fileName)
{
 if (!fileName) return;
 // Correct the name
 int l=strlen(fileName)-1;
 if (fileName[l]=='.')
    fileName[l]=0;
}

static
unsigned Surface(TRect &r)
{
 TPoint dif=r.b-r.a;
 if (dif.x<=0 || dif.y<=0)
    return 0;
 return dif.x*dif.y;
}

static
void FindBestRect(TRect &avail, TRect &avoid, TRect &ret)
{
 TRect left;
 left.a=avail.a;
 left.b.x=avoid.a.x;
 left.b.y=avail.b.y;

 TRect right;
 right.a.x=avoid.b.x;
 right.a.y=avail.a.y;
 right.b=avail.b;

 TRect upper;
 upper.a=avail.a;
 upper.b.x=avail.b.x;
 upper.b.y=avoid.a.y;

 TRect lower;
 lower.a.x=avail.a.x;
 lower.a.y=avoid.b.y;
 lower.b=avail.b;

 unsigned sl=Surface(left);
 unsigned sr=Surface(right);
 unsigned su=Surface(upper);
 unsigned so=Surface(lower);

 if (sl>sr)
   {
    if (sl>su)
       ret=sl>so ? left : lower;
    else
       ret=su>so ? upper : lower;
   }
 else
   {
    if (sr>su)
       ret=sr>so ? right : lower;
    else
       ret=su>so ? upper : lower;
   }
}

TCEditWindow *TSetEditorApp::openEditor(char *fileName, Boolean visible,
                                        EditorResume *res, int options)
{
 Boolean openAsReadOnly=(options & oedForceRO) ? True : False;
 int numEditors=0;
 TCEditWindow *ain=NULL;

 if (visible && fileName)
   {
    edHelper->reIdEditors();
    ain=IsAlreadyOnDesktop(fileName,&numEditors);
   }
 TRect r=deskTop->getExtent();
 TView *p;

 // Let some space
 Boolean sizeDetermined=False;
 if (geFlags & geAvoidPrjAndMsg)
   {// Avoid message and project windows
    TRect prj, msg;
    Boolean prjF, msgF;

    prjF=ProjectGetSize(prj);
    msgF=EdMessageGetSize(msg);
    if (msgF || prjF)
      {
       TRect desk=r;
       sizeDetermined=True;
       if (prjF)
          FindBestRect(r,prj,r);
       if (msgF)
          FindBestRect(r,msg,r);
       // Ensure the result is usable
       TPoint sz=r.b-r.a;
       if (sz.x<24)
         {
          r.b.x=r.a.x+24;
          if (r.b.x>desk.b.x)
            {
             int dif=r.b.x-desk.b.x;
             r.b.x-=dif;
             r.a.x-=dif;
            }
         }
       if (sz.y<6)
         {
          r.b.y=r.a.y+6;
          if (r.b.y>desk.b.y)
            {
             int dif=r.b.y-desk.b.y;
             r.b.y-=dif;
             r.a.y-=dif;
            }
         }
      }
   }
 if (!sizeDetermined)
   {
    if (geFlags & geVertWindows)
      {
       if (geFlags & geRightSide)
          r.b.x-=widthVertWindows;
       else
          r.a.x=widthVertWindows;
      }
    else
       r.b.y-=7;
   }
 /* First check if the user wants more copies, in this case foget the one found */
 if (ain && numEditors<maxOpenEditorsSame)
   {
    /* If we have only RO editors open it as !RO */
    if (!openAsReadOnly && !ain->editor->isReadOnly)
       openAsReadOnly=True;
    ain=NULL;
   }
 if (ain)
   {
    if (options & oedForgetResume)
       ain->editor->handleCommand(cmcTextStart);
    if (options & oedNoSelect)
      {
       if (deskTop->current->prev()!=ain)
         {
          deskTop->lock();
          TView *c=deskTop->current;
          ain->options&= ~ofSelectable;
          ain->makeFirst();
          ain->options|=ofSelectable;
          c->select();
          deskTop->unlock();
         }
      }
    else
       ain->select();
    p=(TView *)ain;
    if (openAsReadOnly) // Could be forced
       ain->editor->isReadOnly=True;
   }
 else
   {
    FixUpName(fileName);
    // If the file is in the project open it using the relative path as if
    // the file came from the project
    char *relName=GetRelIfFileInPrj(fileName);
    ain=new TCEditWindow(r,relName ? relName : fileName,wnNoNumber,openAsReadOnly);
    string_free(relName);
    if ((options & oedDontOpenEmpty) && ain->editor->FailedToLoad)
      {
       CLY_destroy(ain);
       return 0;
      }
    p=validView(ain);
    // If fail during load
    if (!p)
       return (TCEditWindow *)p;

    ain=(TCEditWindow *)p;

    // Transfer the special lines
    TSpCollection *spL=SpLinesGetFor(fileName);
    if (spL)
       ain->editor->SetSpecialLines(spL);
    #ifdef TEST_SPLINES
    ain->editor->SetSpecialLines(spLines);
    #endif

    if (visible)
      { // A local copy, we can use a pointer
       int validResume=0;
       EditorResume r;
       if (res)
         {
          CopyEditorResume(&r,res);
          validResume=1;
         }
       validResume|=AskForClosedResume(&r,fileName);
       validResume|=AskForProjectResume(&r,fileName);
       AddToEditorsHelper((TCEditWindow *)p,1);
       unsigned dstOps=GetDSTOptions();
       if (validResume && // If the user doesn't like memories forget it.
           !(dstOps & (dstNoCursorPos | dstEdNever)) &&
           !(options & oedForgetResume))
          ain->ApplyResume(r);
       if (options & oedNoSelect)
         {
          deskTop->lock();
          TView *c=deskTop->current;
          deskTop->insert(p);
          c->select();
          deskTop->unlock();
         }
       else
          deskTop->insert(p);

       if (!validResume)
         {
          int prjWinThere=IsPrjOpened() && IsPrjVisible();
          switch (TSetEditorApp::geFlags & geMask2)
            {
             case geZoomedIfNoPrj:
                  // When the project is OFF zoom it
                  if (!prjWinThere)
                     ain->zoom();
                  break;
             case geZoomedNPorZ:
                  if (!prjWinThere || IsPrjZoomed())
                     ain->zoom();
                  break;
             case geAlwaysZoomed:
                  ain->zoom();
                  break;
            }
         }
      }
    else
      {
       p->hide();
       deskTop->insert(p);
      }
   }
 if (options & oedZoom)
   {
    TPoint minSize, maxSize;
    ain->sizeLimits(minSize,maxSize);
    if (ain->size!=maxSize)
       ain->zoom();
   }
 return (TCEditWindow *)p;
}


/**[txh]********************************************************************

  Description:
  Sets the window title. Of course it only works when windowed ;-)

***************************************************************************/

void TSetEditorApp::SetTitle(const char *str1, const char *str2)
{
 char *str0="SETEdit ";
 int len=strlen(str0)+sizeof(TCEDITOR_VERSION_STR)+3;

 if (!str1) str1=__("No project loaded");
 char *istr1=TVIntl::getTextNew(str1);
 len+=strlen(istr1);
 if (str2)
    len+=strlen(str2);
 len++;

 char *s=(char *)alloca(len);
 strcpy(s,str0);
 strcat(s,TCEDITOR_VERSION_STR);
 strcat(s," - ");
 strcat(s,istr1);
 if (str2) strcat(s,str2);

 if (!OriginalWindowTitle)
    OriginalWindowTitle=(char *)TScreen::getWindowTitle();
 TScreen::setWindowTitle(s);
 DeleteArray(istr1);
}

TSetEditorApp::~TSetEditorApp()
{
 if (OriginalWindowTitle)
   {
    // That's just in case, but isn't really needed. Not at least in W9x
    TScreen::setWindowTitle((const char *)OriginalWindowTitle);
    delete[] OriginalWindowTitle;
   }
 CLY_destroy(soCol);
}

/**[txh]********************************************************************

  Description:
  That's a hook for the editor. The editor doesn't interact with the user
directly instead most of the dialogs are created through calls
TCEditor::editorDialog in this way we can change the behavior easilly. But
this routine takes a variable number of arguments and hence is hard to
"hook the hook". For this reason now I provide a default function in
doedidia.cc that must be called indirectly by the editor. Synopsis: That's
a hook for the hook function.

***************************************************************************/

unsigned doEditDialogLocal(int dialog, ...)
{
 typedef char *charPtr;
 va_list arg;
 char *str;
 int   flags;

 va_start(arg,dialog);
 switch(dialog)
   {
    case edFileExists:
         str=va_arg(arg,charPtr);
         flags=va_arg(arg,int);
         if (!flags && IsAlreadyOnDesktop(str))
           {
            messageBox(__("This file is already open, close it first."),mfError | mfOKButton);
            return cmNo;
           }
         break;
   }
 va_end(arg);

 va_start(arg,dialog);
 unsigned ret=doEditDialog(dialog,arg);
 va_end(arg);
 return ret;
}

TSetEditorApp::TSetEditorApp() :
    TProgInit( TSetEditorApp::initStatusLine,
               TSetEditorApp::initMenuBar,
               TSetEditorApp::initDeskTop
             ),
    TApplication()
{
 TCommandSet ts;
 ts.enableCmd(cmcSave);
 ts.enableCmd(cmcSaveAs);
 ts.enableCmd(cmcCut);
 ts.enableCmd(cmcCopy);
 ts.enableCmd(cmcPaste);
 ts.enableCmd(cmcClear);
 ts.enableCmd(cmcUndo);
 ts.enableCmd(cmcRedo);
 ts.enableCmd(cmcFind);
 ts.enableCmd(cmcReplace);
 ts.enableCmd(cmcSearchAgain);
 ts.enableCmd(cmeClosePrj);
 ts.enableCmd(cmeSavePrj);
 ts.enableCmd(cmeExportPrj);
 ts.enableCmd(cmeImportPrj);
 if (TScreen::noUserScreen())
   {
    ts.enableCmd(cmeDosShell);
    ts.enableCmd(cmeUserScreen);
   }
 disableCommands(ts);
 DebugCommandsForDisc();

 TCEditor::editorDialog=doEditDialogLocal;
 doNotReleaseCPU=1;

 // The clipboard is created during the load of the DeskTop
 clipWindow=0;
 OriginalWindowTitle=0;
}


/**[txh]********************************************************************

  Description:
  Returns the name of the keybinding file. Passing 1 for save the name
returned should be valid for writing. If the user specified an special name
in the command line that's the returned value.

  Return:
  keybind.dat file name.

***************************************************************************/

static
char *GetKeyBindFName(int save)
{
 if (KeyBindFNameUser)
    return KeyBindFNameUser;
 return save ? ExpandHomeSave(KeyBindFName) : ExpandHome(KeyBindFName);
}

/* No longer used
int FileOpenDialog(char *title, char *file)
{
 return
 execDialog( new TFileDialog( file, title, __("~N~ame"), fdOpenButton, hID_FileOpen ),
             file) != cmCancel;
}*/


void TSetEditorApp::fileOpen()
{
 char fileName[PATH_MAX];
 strcpy(fileName,"*");

 if (GenericFileDialog(__("Open File"),fileName,0,hID_FileOpen)!=cmCancel)
     openEditor(fileName,True);
}

void TSetEditorApp::fileOpenCopy()
{
 TCEditor *e;

 if ((e=GetCurrentIfEditor())!=0)
   { // Open a RO copy
    maxOpenEditorsSame++;
    openEditor(e->fileName,True);
    maxOpenEditorsSame--;
   }
 else
   messageBox(__("You must select an editor window for this operation"),mfError | mfOKButton);
}

void TSetEditorApp::fileNew()
{
 openEditor( 0, True );
}

void TSetEditorApp::changeDir()
{
 TChDirDialog *d=new TChDirDialog(cdNormal,0);
 d->helpCtx=cmeChangeDrct;
 execDialog(d,0);
}

void FullSuspendScreen()
{
 //TProgram::deskTop->setState(sfVisible,False);
 TProgram::application->suspend();
}

int ResetVideoMode(int mode, int redraw)
{
 int wasExt=0;
 // Kill the clock because it's never resized correctly
 editorApp->KillClock();
 // Set the video mode
 if (TSetEditorApp::UseExternPrgForMode)
   {
    TProgram::application->setScreenMode(0xFFFF,TSetEditorApp::ExternalPrgMode);
    wasExt=1;
   }
 else
    TProgram::application->setScreenMode(mode);
 // We ever use intense mode I don't need blinks
 //setIntenseState();
 // Restore the user palette, we can lose it during the setScreenMode
 RestorePaletteSystem();
 // Redraw ALL
 if (redraw)
   { // Force a full redraw
    TProgram::deskTop->setState(sfVisible,True);
    TProgram::deskTop->redraw();
    TProgram::application->redraw();
   }
 if (wasExt)
    return 0;
 // Check for succesfull change
 if (mode==7 || mode==3)
    return TScreen::screenMode!=mode;
 return TScreen::screenMode==3 || TScreen::screenMode==7;
}

int TSetEditorApp::resetVideoMode(Boolean redraw)
{
 // Kill the clock because it's never resized correctly
 editorApp->KillClock();
 // Set the video mode
 if (so->scOptions==scfSameLast || so->scOptions==scfForced)
    TProgram::application->setScreenMode(so->scWidth,so->scHeight,so->scCharWidth,so->scCharHeight);
 else if (so->scOptions==scfExternal && so->scCommand)
    TProgram::application->setScreenMode(0xFFFF,so->scCommand);
 else if (so->scOptions==scfMode)
    TProgram::application->setScreenMode(so->scModeNumber);
 // We ever use intense mode I don't need blinks
 //setIntenseState();
 // Restore the user palette, we can lose it during the setScreenMode
 RestorePaletteSystem();
 // Redraw ALL
 if (redraw)
   { // Force a full redraw
    TProgram::deskTop->setState(sfVisible,True);
    TProgram::deskTop->redraw();
    TProgram::application->redraw();
   }
 if (so->scOptions==scfExternal && so->scCommand)
    return 0;
 // Check for succesfull change
 if (so->scOptions==scfMode)
   {
    if (so->scOptions==7 || so->scOptions==3)
       return (unsigned)TScreen::screenMode!=so->scOptions;
    return TScreen::screenMode==3 || TScreen::screenMode==7;
   }
 return 0;// ?
}

void FullResumeScreen()
{
 TProgram::application->resume();
 ResetVideoMode(TScreen::screenMode); // It resumes the palette too
}

void TSetEditorApp::showClip()
{
 clipWindow->select();
 clipWindow->show();
}

void TSetEditorApp::tile()
{
 deskTop->tile(deskTop->getExtent());
}

void TSetEditorApp::cascade()
{
 deskTop->cascade(deskTop->getExtent());
}

void CopyHelp2Clip(char *b, long l)
{
 if (TSetEditorApp::clipWindow)
    TSetEditorApp::clipWindow->editor->insertBuffer( b,0,(unsigned)l,False,True);
}

static
void ExportAsHTML(void)
{
 TCEditor *e;

 if ((e=GetCurrentIfEditor())!=0)
   {
    struct
    {
     uint32 flags;
     uint32 color;
    } flags;
    flags.flags=xhtmlTitle | xhtmlBackground | xhtmlMonoFont | xhtmlBoldFont |
                xhtmlUseCSS;
    flags.color=0;
    if (!TCEditor::editorDialog(edExportHTMLOps,&flags)) return;
   
    char fileName[PATH_MAX];
    strcpy(fileName,"*.html");
   
    if (GenericFileDialog(__("Export file as"),fileName,"*",hID_FileSave,
        fdDialogForSave)!=cmCancel)
      {
       if (access(fileName,F_OK)==0 &&
           TCEditor::editorDialog(edFileExists,fileName,0)==cmNo)
          return;
       FILE *f=fopen(fileName,"wb");
       if (f)
         {/* Create a table with the palette */
          unsigned pal[16];
          GetRGBArrayPaletteSystem(pal);
          unsigned flgs=flags.flags;
          if (flags.color==0)
             flgs|=xhtmlUseColors;
          e->SourceToHTML(f,pal,flgs);
          if (ferror(f))
             TCEditor::editorDialog(edWriteError,fileName);
          fclose(f);
         }
       else
         TCEditor::editorDialog(edCreateError,fileName);
      }
   }
}

static
char *GetWordUnderCursor(unsigned size, unsigned options=0)
{
 TCEditor *e;

 if ((e=GetCurrentIfEditor())!=0)
    return e->WordUnderCursor(size,options);
 return 0;
}

static
void MakeTagsWordCompletion()
{
 TCEditor *e;

 if ((e=GetCurrentIfEditor())!=0)
   {
    char *word=e->WordUnderCursor(80,wucTakeOneLeft | wucIncludeColon | wucCanStartColon);
    if (word)
      {
       unsigned l=strlen(word);
       char *exp;
       Boolean fullInsert=False;

       if (l>2 && word[l-1]==':' && word[l-2]==':')
         {// That's for C++ classes
          word[l-2]=0;
          exp=TagsWordCompletionClass(e->cursor.x+e->owner->origin.x,
                                      e->cursor.y+e->owner->origin.y+1,word);
          fullInsert=True;
         }
       else
          exp=TagsWordCompletion(e->cursor.x+e->owner->origin.x,
                                 e->cursor.y+e->owner->origin.y+1,word);
       if (exp)
         {
          char *cursor=e->ColToPointerPost();
          if (!fullInsert)
             e->deleteRange(cursor-strlen(word),cursor,True);
          e->insertText(exp,strlen(exp),False);
          DeleteArray(exp);
         }
       DeleteArray(word);
      }
   }
}

static
void PrintEditor(void)
{
 TCEditor *e;

 if ((e=GetCurrentIfEditor())!=0)
   {
    if (e->IslineInEdition)
       e->MakeEfectiveLineInEdition();
    e->buffer[e->bufLen]=0;
    char *s=strrchr(e->fileName,'/');
    if (!s)
       s=e->fileName;
    else
       s++;
    PrintSource(e->buffer,s,e->tabSize);
   }
 else
   messageBox(__("This window can't be printed. Select an editor window instead"),mfError | mfOKButton);
}

#define T(a) \
case cme##a:\
     event.message.command=cm##a;\
     TApplication::handleEvent(event);\
     return;
#define TCheck(a) \
case cme##a:\
     if (TProgram::deskTop->current)\
       {\
        event.message.command=cm##a;\
        TApplication::handleEvent(event);\
       }\
     return;

void TSetEditorApp::getEvent(TEvent& event)
{
 // Why in the hell that's so tricky? If I don't get the context here but in the
 // handleEvent the originating routine already lost the focus so I get nothing
 TApplication::getEvent(event);
 if (helpRequest && event.what==evNothing)
   { // If we already got a request now pop-up the help
    event.what=evCommand;
    event.message.command=cmHelp;
   }
 switch (event.what)
   {
    case evCommand:
         switch (event.message.command)
           {
            case cmHelp:
            case cmeInfView:
                 if (!helpRequest)
                   { // It acts like a latch to hold the context until the help is displayed
                    helpRequest=1;
                    helpCtxRequested=getHelpCtx();
                    event.message.command=evNothing;
                   }
                 else
                   {
                    GetContextHelp();
                    clearEvent(event);
                   }
                 break;
           }
         break;
   }
}

void TSetEditorApp::handleEvent( TEvent& event )
{
 TApplication::handleEvent( event );
 if (event.what==evBroadcast)
   {
    if (event.message.command==cmClosingWindow && edHelper)
       edHelper->removeWindow(event.message.infoPtr);
       // We clear the event before returning
    else
       return;
   }
 else
 if (event.what!=evCommand)
    return;
 else
    {
     switch (event.message.command)
        {
         case cmeOpen:
              fileOpen();
              break;
 
         case cmeNew:
              fileNew();
              break;

         case cmeOpenROCopy:
              fileOpenCopy();
              break;
 
         case cmeChangeDrct:
              changeDir();
              break;
 
         case cmeDosShell:
              dosShell();
              break;
 
         case cmeShowClip:
              showClip();
              break;
 
         case cmeTile:
              tile();
              break;
 
         case cmeCascade:
              cascade();
              break;

         case cmeCalculator:
              executeCalc(GetWordUnderCursor(250));
              // In case the user copied to the clipboard.
              UpdateEditorCommands();
              break;

         case cmePocketCalc:
              pocketCalculator();
              break;

         case cmeGrepDialog:
              if (RunExternalProgramNotRunning())
                 grepWindow(GetWordUnderCursor(maxGrepString));
              break;

         case cmeInfView:
              GetContextHelp();
              break;

         case cmeLastHelp:
              InfManager->MakeVisible();
              break;

         case cmeAnotherInfView:
             {
              TDskWinHelp *p;

              p=new TDskWinHelp(EditorFile,"");
              deskTop->insert(p->window);
              edHelper->addNonEditor(p);
              p->window->select();
              p->window->show();
             }
              break;

         case cmeManPageView:
             {
              TDskWinMan *p;
              char *word=GetWordUnderCursor(80);

              p=ManPageView(word);
              if (p)
                {
                 deskTop->insert(p->view);
                 edHelper->addNonEditor(p);
                 p->view->select();
                 p->view->show();
                }
              delete[] word;
             }
              break;

         case cmeSyntaxHelp:
             {
              char *FileName,*NodeName,*VisibleName;
              char *word=GetWordUnderCursor(MAX_NODE_NAME);
              switch (SyntaxSearch_Search(word,FileName,NodeName,VisibleName))
                {
                 case 0:
                      messageBox(__("Couldn't find any matching help"),mfOKButton);
                      break;
                 case 1:
                      InfManager->Goto(FileName,NodeName,
                                       SyntaxSearch_GetJumpOption() ? VisibleName : 0);
                      InfManager->MakeVisible();
                      break;
                }
              delete[] word;
             }
              break;

         case cmeSyntaxHelpOps:
              SyntaxSearch_EditSettings();
              break;

         case cmeSyntaxHelpFiles:
              SyntaxSearch_EditFilesList();
              break;

         case cmeTipOfTheDay:
              ShowTips(ExpandHome(TipsFName),1);
              break;

         case cmeAboutBox:
              FullAboutBox();
              break;

         case cmeListWin:
              BringListOfWindows();
              break;

         case cmeUserScreen:
              ShowUserScreen(event);
              break;

         case cmeEditKeyBind:
              if (KeyBindEdit())
                 SaveKeyBind(GetKeyBindFName(1));
              break;

         case cmeSetUpAltKeys:
              if (AltKeysSetUp())
                 SaveKeyBind(GetKeyBindFName(1));
              break;

         case cmeKbBackDefault:
              if (KeyBackToDefault())
                 SaveKeyBind(GetKeyBindFName(1));
              break;

         case cmeKeyPadBehavior:
              if (KeyPadSetUp())
                 SaveKeyBind(GetKeyBindFName(1));
              break;

         case cmeSeeScanCodes:
              SeeScanCodes();
              break;

         case cmeOpenPrj:
              if (DebugConfirmEndSession())
                 OpenProject();
              break;

         case cmeClosePrj:
              if (DebugConfirmEndSession())
                 CloseProject(1);
              break;

         case cmeSavePrj:
              SaveProject();
              break;

         case cmeSaveDesktop:
              SaveDesktopHere();
              break;

         case cmeExportPrj:
              ExportProjectItems();
              break;

         case cmeImportPrj:
              ImportProjectItems();
              break;

         case cmeSDG:
              if (RunExternalProgramNotRunning())
                 SDGInterfaceRun();
              break;

         case cmeSDGDialog:
              SDGInterfaceDialog();
              break;

         case cmeSetColors:
              Colors();
              break;

         case cmeColorTheme:
              ColorTheme();
              break;

         case cmePrintEditor:
              PrintEditor();
              break;

         case cmeSetUpPrinter:
              PrintSetup();
              break;

         case cmeNextMessage:
              EdMessageSelectNext();
              break;

         case cmePrevMessage:
              EdMessageSelectPrev();
              break;

         case cmeSetScreenOps:
              //SetScreenOps();
              ScreenOptions();
              break;

         case cmeEditPalette:
              EditPalette();
              break;

         case cmeEdGralOptions:
              {
               SetGeneralEditorOptions();
               // The save-as-is option affects some commands.
               UpdateEditorCommands();
              }
              break;

         case cmeScreenSaverOpts:
              SetScreenSaversOptions();
              break;

         case cmcSetGlobalOptions:
              TCEditor::SetGlobalOptions();
              break;

         case cmeEditUserWords:
              TCEditUserWords(ExpandHomeSave(GetNameOfUserWordsFile()),cmeEditUserWords);
              edHelper->redrawEditors();
              break;

         case cmeEditDeflOpts:
              TCEditDefaultOpts(ExpandHomeSave(GetNameOfDefaultOptsFile()),cmeEditDeflOpts);
              break;

         case cmeRunCommand:
              if (RunExternalProgramNotRunning())
                 RunExternalProgram();
              break;

         case cmeConfRunCommand:
              ConfigureRunCommand();
              break;

         case cmeStopChild:
              RunExternalProgramStopChild();
              break;

         case cmeHTMLAccents:
              HTMLAcc_Entry(False);
              break;

         case cmeHTMLTag2Accent:
              HTMLAcc_Entry(True);
              break;

         case cmeExportAsHTML:
              ExportAsHTML();
              break;

         case cmeQuitDelete:
              DeleteFilesOnExit=1;
         case cmeQuit:
              if (RunExternalProgramRunning())
                {
                 if (messageBox(__("A background task is still running, exit anyway?"),
                     mfYesButton | mfNoButton | mfConfirmation)==cmNo)
                   {
                    DeleteFilesOnExit=0;
                    break;
                   }
                 RunExternalProgramStopChild();
                }
              if (!DebugConfirmEndSession())
                {
                 DeleteFilesOnExit=0;
                 break;
                }
              if (DeleteFilesOnExit)
                {
                 int ret=messageBoxDSA(cmeQuitDeleteMessage,
                     mfYesButton | mfNoButton | mfWarning,"SET_CONFQUIT",cmYes);
                 if (ret!=cmYes)
                   {
                    DeleteFilesOnExit=0;
                    if (ret==cmCancel)
                       break;
                   }
                }
              event.message.command=cmQuit;
              TApplication::handleEvent(event);
              break;

         case cmeDeleteBkps:
              KillFilesToKill();
              ReleaseFilesToKill();
              break;

         case cmeASCIIChart:
              ASCIIWindow();
              break;

         case cmeCalendar:
              CalendarWindow();
              break;

         case cmeFileOpenOptions:
              SetFileOpenDialogOptions();
              break;

         case cmeRemapCodePage:
              RemapCodePageEd();
              break;

         case cmeReDraw:
              redraw();
              break;

         case cmeBoardMixer:
              BoardMixerDialog();
              break;

         case cmeEditNoBkp:
              NBKPEdit();
              break;

         case cmeIncludeList:
              PathListEdit(paliInclude,cmeIncludeList);
              break;

         case cmeSourceList:
              PathListEdit(paliSource,cmeSourceList);
              break;

         case cmeEncodings:
              EncodingOptions();
              break;

         case cmeFonts:
              FontsOptions();
              break;

         case cmeSaveAll:
              SaveAllEditors();
              break;

         case cmeTagFiles:
              EditTagFiles();
              break;

         case cmeSearchTag:
              SearchTag(GetWordUnderCursor(80));
              break;

         case cmeClassBrowser:
              TagsClassBrowser(GetWordUnderCursor(80));
              break;

         case cmeWordCompletion:
              MakeTagsWordCompletion();
              break;

         case cmeTagsOps:
              SetTagFilesGenerationOptions();
              break;

         case cmeTagsAutoRegen:
              TagsAutoRegen();
              break;

         case cmeHolidaysConf:
              ConfigureHolidays();
              break;

         case cmeSetModiCkOps:
              SetModifCheckOptions();
              break;

         case cmeAdviceDiagConf:
              AdviceManager();
              break;

         case cmeSelWinPrj:
              SelectWindowNumber(-dktPrj);
              break;

         case cmeSelWinMessage:
              SelectWindowNumber(-dktMessage);
              break;

         case cmeSelDebugWin:
              SelectWindowNumber(-dktDbgMsg);
              break;

         case cmeSelWatchesWin:
              SelectWindowNumber(-dktDbgWt);
              break;

         case cmeSelWindow1:
         case cmeSelWindow2:
         case cmeSelWindow3:
         case cmeSelWindow4:
         case cmeSelWindow5:
         case cmeSelWindow6:
         case cmeSelWindow7:
         case cmeSelWindow8:
         case cmeSelWindow9:
         case cmeSelWindow10:
         case cmeSelWindow11:
         case cmeSelWindow12:
         case cmeSelWindow13:
         case cmeSelWindow14:
         case cmeSelWindow15:
         case cmeSelWindow16:
         case cmeSelWindow17:
         case cmeSelWindow18:
         case cmeSelWindow19:
              SelectWindowNumber(event.message.command-cmeSelWindow1+1);
              break;

         case cmeGPushCursorPos:
              GPushCursorPos();
              break;

         case cmeGPopCursorPos:
              GPopCursorPos();
              break;

         //********** DEBUG commands
         case cmeBreakpoint:
              DebugToggleBreakpoint();
              break;

         case cmeDebugOptions:
              DebugOptionsEdit();
              break;

         case cmeDbgRunContinue:
              DebugRunOrContinue();
              break;

         case cmeDbgStepOver:
              DebugStepOver();
              break;

         case cmeDbgTraceInto:
              DebugTraceInto();
              break;

         case cmeDbgGoToCursor:
              DebugGoToCursor();
              break;

         case cmeDbgFinishFun:
              DebugFinishFun();
              break;

         case cmeDbgReturnNow:
              DebugReturnNow();
              break;

         case cmeDbgStop:
              DebugStop();
              break;

         case cmeDbgKill:
              DebugKill();
              break;

         case cmeDbgCallStack:
              DebugCallStack();
              break;

         case cmeDbgEvalModify:
              DebugEvalModify(GetWordUnderCursor(250));
              break;

         case cmeDbgOptsMsgs:
              DebugOptsMsgs();
              break;

         case cmeDbgWatchExpNorm:
              DebugWatchExp(False,GetWordUnderCursor(250));
              break;

         case cmeDbgWatchExpScp:
              DebugWatchExp(True,GetWordUnderCursor(250));
              break;

         case cmeDbgEndSession:
              if (DebugConfirmEndSession(True)) // Indicate that's user explicit request
                 DebugDeInitVars();
              break;

         case cmeDbgCloseSession:
              DebugCloseSession(); // It asks for confirmation.
              break;

         case cmeDbgGoConnected:
              DebugCheckAcceptCmd(True);
              break;

         case cmeDbgGoReadyToRun:
              DebugCheckStopped(True);
              break;

         case cmeDbgEditBreakPts:
              DebugEditBreakPts();
              break;

         case cmeDbgEditWatchPts:
              DebugEditWatchPts(GetWordUnderCursor(250));
              break;

         case cmeDbgInspector:
              DebugInspector(GetWordUnderCursor(250));
              break;

         case cmeDbgDataWindow:
              DebugDataWindow(GetWordUnderCursor(250));
              break;

         case cmeDbgStackWindow:
              DebugStackWindow();
              break;

         case cmeDbgCleanElem:
              DebugCleanElem();
              break;

         case cmeDbgThreadSel:
              DebugThreadSel();
              break;

         case cmeDbgOptionsAdv:
              DebugOptionsAdv();
              break;

         case cmeDbgDisAsmWin:
              DebugDisAsmWin();
              break;

         case cmeDbgDetach:
              DebugDetach();
              break;

         // These commands are traslated to the original values
         TCheck(Resize)
         TCheck(Zoom)
         TCheck(Next)
         TCheck(Prev)
         TCheck(Close)

         default:
             ProcessMP3Commands;
             if (isAMacroInMenu(event.message.command)) break;
             return;
         }
    }
 clearEvent( event );
}
#undef T

void TSetEditorApp::RemapCodePageEd(void)
{
 TCEditor *e;
 int from,to;
 unsigned ops;

 if ((e=GetCurrentIfEditor())!=0 && ChooseConvCPs(from,to,ops))
    e->RemapCodePageBuffer(from,to,ops);
}

void TSetEditorApp::pocketCalculator(void)
{
 #if HAVE_CALCULATOR
 TCalculator *calc=(TCalculator *)validView(new TCalculator);

 if (calc)
   {
    //calc->helpCtx = hcCalculator; very obvious to use
    deskTop->insert(calc);
   }
 #else
 messageBox(__("This functionality was disabled at compile time"),
            mfError | mfOKButton);
 #endif
}

char DumpStartName[]="\n\n>>>>>>>>>>>>>>\n";

void DumpEditors(void)
{
 if (TSetEditorApp::edHelper)
   {
    int c=TSetEditorApp::edHelper->Editors;
    if (c>0 && c<100)
      {
       int i;
       TDskWinEditor *st;
       for (i=0; i<c; i++)
          {
           st=(TDskWinEditor *)(TSetEditorApp::edHelper->at(i));
           if (st)
             {
              TCEditWindow *edw=st->edw;
              if (edw)
                {
                 TCEditor *ed=edw->editor;
                 if (ed->modified==True)
                   {
                    unsigned l=ed->bufLen;
                    char *b=ed->buffer;
                    ed->fileName[PATH_MAX-1]=0;
                    BestWrite(DumpStartName,sizeof(DumpStartName)-1);
                    BestWrite(ed->fileName,strlen(ed->fileName));
                    BestWrite(DumpStartName,sizeof(DumpStartName)-1);
                    if (b && l<1000000)
                       BestWrite(b,l);
                   }
                }
             }
          }
      }
   }
}


static
void WaitForKeyDown(TEvent &event)
{
 #ifdef SECompf_djgpp
 do
   {
    editorApp->idle();
    editorApp->clearEvent(event);
    event.getKeyEvent();
   }
 while ((event.what & (evKeyDown)) == 0);
 #endif
 #ifdef SEOS_UNIX
 // The Linux port is totally broken and needs this workaround
 getchar();
 editorApp->clearEvent(event);
 #endif
 #ifdef SEOS_Win32
 //$todo: implement (SAA)
 #endif
}

void TSetEditorApp::ShowUserScreen(TEvent &event)
{
 #ifdef SEOS_UNIX
 ShowUserScreenDialog();
 #endif

 TProgram::deskTop->setState(sfVisible,False);
 TProgram::application->suspend();
 WaitForKeyDown(event);
 clearEvent(event);
 TProgram::application->resume();
 ResetVideoMode(TScreen::screenMode);
}

//
// Force to link some stream modules
//
__link(RBackground)
__link(RCEditWindow)
__link(RInfViewer)
#ifdef SEOS_Win32
// setedit/streams
__link(RDskWinASCII)
__link(RDskWinCalendar)
__link(RDskWinClipboard)
__link(RDskWinClosed)
__link(RDskWinEditor)
__link(RDskWinHelp)
__link(RDskWinMan)
__link(RDskWinPrj)
__link(RDskWinMP3)
__link(RDskWinMessage)
__link(REditorCollection)
__link(RManWindow)
#endif

//
//   Some of the following functions are just wrappers to hide
// the implementation of the editor's collection. In this way
// other files doesn't need to know about edHelper and your
// class.
//

void AddToEditorsHelper(TCEditWindow *p, int SelectHL)
{
 TSetEditorApp::edHelper->addEditor(p,SelectHL);
}

int IsAnEditor(void *p)
{
 if (!TSetEditorApp::edHelper) return 0;
 return TSetEditorApp::edHelper->search(p,dktEditor)>=0;
}

TCEditor *GetCurrentIfEditor()
{
 TView *p=editorApp->deskTop->current;
 if (!p) return 0; // Avoid a search
 if (IsAnEditor(p))
    return ((TCEditWindow *)p)->editor;
 return 0;
}

void UpdateEditorCommands()
{
 TCEditor *e=GetCurrentIfEditor();
 if (e)
    e->updateCommands(1);
}

TCEditWindow *GetCurrentIfEditorWindow()
{
 TView *p=editorApp->deskTop->current;
 if (!p) return 0; // Avoid a search
 if (IsAnEditor(p))
    return (TCEditWindow *)p;
 return 0;
}

TCEditWindow *IsAlreadyOnDesktop(char *fileName, int *cant, stEditorId *id)
{
 // First search by inode, the only way
 ccIndex pos=TSetEditorApp::edHelper->searchEditorbyINode(id,fileName,cant);
 if (pos<0)
    return NULL;
 TDskWinEditor *st=(TDskWinEditor *)TSetEditorApp::edHelper->at(pos);
 return st->edw;
}

int AskForClosedResume(EditorResume *r,char *fileName)
{
 if (!fileName)
    return 0;
 ccIndex pos=TSetEditorApp::edHelper->search(fileName,dktClosed);
 if (pos<0)
    return 0;
 CopyEditorResume(r,&(((TDskWinClosed *)TSetEditorApp::edHelper->at(pos))->resume));
 return 2;
}

int SearchInHelper(int type, void *p)
{
 return TSetEditorApp::edHelper->search(p,type)>=0;
}

TDskWin *SearchInHelperWin(int type, void *p)
{
 ccIndex pos=TSetEditorApp::edHelper->search(p,type);
 if (pos<0)
    return NULL;
 return (TDskWin *)TSetEditorApp::edHelper->at(pos);
}

/*Boolean View2WindowNumber(void *view, int &number)
{
 ccIndex pos=TSetEditorApp::edHelper->searchByView(view);
 if (pos==-1)
    return False;
 number=((TDskWinClosed *)TSetEditorApp::edHelper->at(pos))->GetNumber();
 return True;
}*/

/**[txh]********************************************************************

  Description:
  Used to know the biggest number of editor window
  
  Return: The biggest number or 0 if no editors.
  
***************************************************************************/

int GetMaxWindowNumber()
{
 return TSetEditorApp::edHelper->GetMaxWindowNumber();
}

void AddNonEditorToHelper(TDskWin *p)
{
 if (p)
    TSetEditorApp::edHelper->addNonEditor(p);
}


static int NamesPrinted;
static
void EdPrintName(void *p, void *f)
{
 TDskWin *dsk=(TDskWin *)p;
 if (dsk->type==dktEditor)
   {
    fprintf((FILE *)f," \"%s\" ",((TDskWinEditor *)p)->edw->editor->fileName);
    NamesPrinted++;
   }
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
 TDskWin *p=TSetEditorApp::edHelper->searchByNumber(number);
 if (p)
   {
    p->GoAction(0);
    FinishFocusChange();
   }
 return p!=0;
}

/**[txh]********************************************************************

  Description:
  Writes all the names of the opened windows to the stream f. The names are
separated by spaces.@p
  Used by the Grep Interface.@p

  Returns:
  The number of names sent to the stream.

***************************************************************************/

int WriteNamesOfOpenedTo(FILE *f)
{
 NamesPrinted=0;
 if (TSetEditorApp::edHelper)
    TSetEditorApp::edHelper->forEach(EdPrintName,f);
 return NamesPrinted;
}

/**[txh]********************************************************************

  Description:
  Shows the file and line specified. If the file isn't opened the routine
opens it.

***************************************************************************/

int ShowFileLine(int line, char *file)
{
 if (!line)
    return 0;
 TCEditWindow *edw=editorApp->openEditor(file,True,0,oedNoSelect | oedDontOpenEmpty);
 if (edw)
   {
    TCEditor *ed=edw->editor;
    ed->lock();
    ed->GoAndSelectLine(line);
    ed->trackCursor(True);
    ed->update(ufView);
    ed->unlock();
    return 1;
   }
 return 0;
}

void XYFStack::Push(uint32 aX, uint32 aY, const char *aFile)
{
 XYFElement *p=new XYFElement(aX,aY,aFile);

 if (last)
   {
    last->next=p;
    p->prev=last;
    last=p;
   }
 else
    first=last=p;
 // Put a limit
 count++;
 if (count>maxXYFStack)
   {
    count--;
    p=first->next;
    first->next=NULL;
    delete first;
    first=p;
   }
}

XYFElement *XYFStack::Pop()
{
 if (!last)
    return NULL;
 XYFElement *ret=last;
 if (last->prev)
   {
    last=last->prev;
    last->next=NULL;
   }
 else
    first=last=NULL;
 count--;

 return ret;
}

void GPushCursorPos()
{
 TCEditor *e=GetCurrentIfEditor();
 if (!e)
    return;
 stkPos.Push(e->curPos.x,e->curPos.y,e->fileName);
}

void GPopCursorPos()
{
 XYFElement *p=stkPos.Pop();
 if (p)
   {
    TCEditWindow *edw=editorApp->openEditor(p->file,True,NULL,oedDontOpenEmpty);
    if (edw)
      {
       TCEditor *ed=edw->editor;
       ed->MoveCursorTo(p->x,p->y,True);
       ed->trackCursor(True);
      }
    delete p;
   }
}

/**[txh]********************************************************************

  Description:
  Goes to the file and line specified. If the file isn't opened the routine
opens it.@p
  The @var{normalLine} hack is currently used when debugging. In this case
the line isn't "selected", instead we set the "CPU Line" for this line.

***************************************************************************/

int GotoFileLine(int line, char *file, char *msg, int off, int len,
                 unsigned flags)
{
 if (!line)
   {
    messageBox(__("This line no longer exists"),mfOKButton);
    return 0;
   }
 TView *oldCur=editorApp->deskTop->current;
 int options=oedDontOpenEmpty;
 if (flags & gflDontSelect)
    options|=oedNoSelect;
 TCEditWindow *edw=editorApp->openEditor(file,True,NULL,options);
 Boolean notCPULine=flags & gflCPULine ? False : True;
 if (edw)
   {
    TCEditor *ed=edw->editor;
    ed->lock();
    ed->GoAndSelectLine(line,notCPULine);
    if (!notCPULine)
      {
       DebugSetCPULine(line,file);
       ed->trackCursor(oldCur==editorApp->deskTop->current ? False : True);
      }
    else
       ed->trackCursor(True);
    if (notCPULine)
       ed->update(ufView); // Be sure we cleared the last hit
    if (msg)
      {// Show only a portion if they asked for it
       if (off>=0)
         {
          char oldV=msg[off+len];
          msg[off+len]=0;
          ed->setStatusLine(msg+off);
          msg[off+len]=oldV;
         }
       else
         ed->setStatusLine(msg);
      }
    ed->unlock();
    return 1;
   }
 return 0;
}

TCEditWindow *GetEditorWindowForFile(char *file)
{
 return editorApp->openEditor(file,True,NULL,oedDontOpenEmpty | oedNoSelect);
}

int GotoFileText(char *search, char *file, char *msg, int off, int len)
{
 if (!search)
    return 0;

 TCEditWindow *edw=editorApp->openEditor(file,True,NULL,oedDontOpenEmpty);
 if (edw)
   {
    TCEditor *ed=edw->editor;
    ed->lock();
    ed->SearchAndJump(search,efCaseSensitive | efWholeWordsOnly);
    ed->trackCursor(True);
    ed->update(ufView); // Be sure we cleared the last hit
    if (msg)
      {// Show only a portion if they asked for it
       if (off>=0)
         {
          char oldV=msg[off+len];
          msg[off+len]=0;
          ed->setStatusLine(msg+off);
          msg[off+len]=oldV;
         }
       else
         ed->setStatusLine(msg);
      }
    ed->unlock();
    return 1;
   }
 return 0;
}

/**[txh]********************************************************************

  Description:
  Called by the special lines manager to notify that some special lines list
changed.

***************************************************************************/

void ApplySpLines(char *fileName, TSpCollection *spLines)
{
 TCEditWindow *edw=IsAlreadyOnDesktop(fileName);
 if (edw)
    edw->editor->SetSpecialLines(spLines);
}

void SaveAllEditors(void)
{
 if (TSetEditorApp::edHelper)
    TSetEditorApp::edHelper->saveEditors();
}

/**[txh]********************************************************************

  Description:
  If a file with the provided name exists it is reloaded from disk.
  
  Return: 1 if we reloaded.
  
***************************************************************************/

int EdReloadIfOpened(const char *name, stEditorId *id)
{
 TCEditWindow *edw=IsAlreadyOnDesktop((char *)name,0,id);
 if (edw && !edw->editor->isReadOnly)
   {
    if (edw->editor->reLoadFile()==False)
       closeView(edw,0);
    return 1;
   }
 return 0;
}

static
Boolean CheckForDiff()
{
 static Boolean isDiffInstalled=False;

 if (!isDiffInstalled)
   {
    // We must rediret the error to avoid getting it in the stderr file
    char *err=open_stderr_out();
    TScreen::System("diff --version");
    close_stderr_out();
    // Check what we got
    FILE *f=fopen(err,"r");
    int ok=0;
    if (f)
      {
       char resp[80];
       fgets(resp,80,f);
       fclose(f);
       ok=strstr(resp,"GNU diff")!=0;
      }
    unlink(err);

    if (ok)
       isDiffInstalled=True;
   }

 return isDiffInstalled;
}

static
Boolean TestDiffInstalled()
{
 Boolean ret=CheckForDiff();
 if (!ret)
    GiveAdvice(gadvDiffModOpts);

 return ret;
}

static
void ReOpen(TCEditWindow *edw)
{
 char *name=strdup(edw->editor->fileName);
 closeView(edw,0);
 editorApp->openEditor(name,True);
 free(name);
}

const char *DiffError=__("Error creating the difference, aborting");

// Handles the case where an editor contains a modified file that was
// modified on disk. That's most probably a problem.
static
int SolveModifiedCollision(TCEditWindow *edw)
{
 Boolean haveDiff=TestDiffInstalled();
 TDialog *d=createSolveModifCollision(haveDiff);
 int ret=execDialog(d,NULL);
 if (ret==cmCancel)
    return 0;
 TCEditor *e=edw->editor;
 if (ret==cmOK)
   {
    e->modified=False; // The user wants to discard it, don't ask again
    ReOpen(edw);
    return 1;
   }
 // The user wants to see a diff
 // Save the current buffer to a temporal
 char *tmpFile=e->saveToTemp();
 char *tmpDifFile=unique_name("di",NULL);
 char *command=NULL;
 int   retDiff=1;
 // Run diff
 if (tmpFile && tmpDifFile)
   {
    string_cat(command,"diff -u ",tmpFile," ",e->fileName," > ",tmpDifFile,0);
    if (command)
      {
       retDiff=TScreen::System(command);
       if (retDiff>0xFF)
          retDiff>>=8;
       if (retDiff==1)
          retDiff=0;
       free(command);
      }
   }
 // The temporal file must be deleted in any case
 if (tmpFile)
   {
    unlink(tmpFile);
    free(tmpFile);
   }
 // Check for errors
 if (retDiff)
   {
    if (tmpDifFile)
      {
       unlink(tmpDifFile);
       free(tmpDifFile);
      }
    messageBox(DiffError,mfError|mfOKButton);
    return 0;
   }
 // Finally we have the diff so now we can discard the buffer (or not)
 int retF=0;
 if (ret==cmYes)
   {
    e->modified=False; // The user wants to discard it, don't ask again
    ReOpen(edw);
    retF=1;
   }
 // Open it read-only
 editorApp->openEditor(tmpDifFile,True,NULL,oedForceRO | oedForgetResume | oedZoom);
 // Don't let it on disk
 unlink(tmpDifFile);
 free(tmpDifFile);
 return retF;
}

int AskReloadEditor(TCEditWindow *edw)
{
 char *fileName=edw->editor->fileName;
 if (LimitedFileNameDialog(mfYesButton | mfNoButton | mfConfirmation,
     __("The disk copy of %s is newer, reload it?"),fileName)==cmYes)
   {// OK, but ... is this buffer modified?
    if (edw->editor->modified)
       // This is complex, handled in another function
       return SolveModifiedCollision(edw);
    // Just close it and open again
    ReOpen(edw);
    return 1;
   }
 return 0;
}

static
int EdReLoad(void *p)
{
 TDskWin *dsk=(TDskWin *)p;
 if (dsk->type==dktEditor)
   {
    TCEditWindow *edw=((TDskWinEditor *)p)->edw;
    TCEditor *e=edw->editor;
    if (e->checkDiskCopyChanged(True))
       return AskReloadEditor(edw);
   }
 return 0;
}

/**[txh]********************************************************************

  Description:
  Writes all the names of the opened windows to the stream f. The names are
separated by spaces.@p
  Used by the Grep Interface.@p

  Returns:
  The number of names sent to the stream.

***************************************************************************/

void ReLoadModifEditors(void)
{
 if (TSetEditorApp::edHelper &&
     (TSetEditorApp::getModifFilesOps() & mfoDontCheckAfterRun)==0)
   {
    int again=1;
    int c,i;
    do
      {
       again=0;
       c=TSetEditorApp::edHelper->getCount();
       i=0;
       while (i<c && !again)
          again=EdReLoad(TSetEditorApp::edHelper->at(i++));
      }
    while (again);
   }
}

clock_t TSetEditorApp::LastTimeUpdate=0;
char TSetEditorApp::UseScreenSaver=1;
char TSetEditorApp::ShowClock=1;
char TSetEditorApp::UseExternPrgForMode=0;
char TSetEditorApp::DesktopPreloaded=0;
char TSetEditorApp::fontCreated=0;
char *TSetEditorApp::WhichScrSaver=0;
int  TSetEditorApp::screenSaverTime=180;
int  TSetEditorApp::screenSaverTimeMouse=3;
CLY_StreamPosT TSetEditorApp::posPreload;
TDeskTopClock *TSetEditorApp::Clock=0;
char TSetEditorApp::ExternalPrgMode[80]="c:/etc/stm -t c:/etc/TextConfig 108x30";

const int ClockWidth=7;

void TSetEditorApp::CreateClock()
{
 // Take the last ClockWidth characters for the clock
 TRect r=menuBar->getExtent();
 int start=r.a.x;
 r.a.x=r.b.x-ClockWidth;
 Clock=new TDeskTopClock(r);
 insert(Clock);
 // Reduce the menubar
 r.b.x=r.a.x;
 r.a.x=start;
 menuBar->changeBounds(r);
}

void TSetEditorApp::KillClock()
{
 if (Clock)
   {
    remove(Clock);
    CLY_destroy(Clock);
    Clock=0;
    // Enlarge the menuBar again
    TRect r=menuBar->getExtent();
    r.b.x+=ClockWidth;
    menuBar->changeBounds(r);
   }
}

void TSetEditorApp::setCmdState( uint16 command, Boolean enable )
{
 if (enable)
    enableCommand(command);
 else
    disableCommand(command);
}

#ifdef SECompf_djgpp
const int clockResolution=CLOCKS_PER_SEC;
#elif defined(SEOS_UNIX)
const int clockResolution=100;
#elif defined(SEOS_Win32)
const int clockResolution=(int)CLK_TCK;
#endif

void TSetEditorApp::idle()
{
 ProcessMP3Idle;
 // Look for async responses from gdb.
 DebugPoll();
 if (multiMenuBar)
    multiMenuBar->update();
 TApplication::idle();
 clock_t DifLastTime=lastIdleClock-LastTimeUpdate;
 // Update 2 times per second
 if (DifLastTime<clockResolution/2)
   {
    RunExternalProgramIncParse();
    CLY_YieldProcessor(-1);
    return;
   }
 LastTimeUpdate=lastIdleClock;
 /***** Update cme Commands here *****/
 // Now update commands
 // cme commands must follow cm commands
 setCmdState(cmeResize,commandEnabled(cmResize));
 setCmdState(cmeZoom,commandEnabled(cmZoom));
 setCmdState(cmeClose,commandEnabled(cmClose));
 Boolean genState=commandEnabled(cmNext);
 setCmdState(cmeNext,genState);
 setCmdState(cmePrev,genState);
 setCmdState(cmeTile,genState);
 setCmdState(cmeCascade,genState);
 setCmdState(cmeSaveDesktop,IsPrjOpened() ? False : True);
 setCmdState(cmeGPopCursorPos,stkPos.getCount() ? True : False);
 setCmdState(cmeTagsAutoRegen,GetAutoGenMode()==stfAutoCentral ? True : False);
 // Disable "Local Options" if no editors are available
 TCEditor *e;
 if ((e=GetCurrentIfEditor())!=0)
   {
    setCmdState(cmePrintEditor,True);
    setCmdState(cmeOpenROCopy,True);
    setCmdState(cmeRemapCodePage,True);
    setCmdState(cmeHTMLAccents,True);
    setCmdState(cmeHTMLTag2Accent,True);
    setCmdState(cmeGPushCursorPos,True);
    if (e->ShowMatchPairFly)
       e->handleCommand(cmcForceMatchPairHL);
   }
 else
   {
    setCmdState(cmePrintEditor,False);
    setCmdState(cmeOpenROCopy,False);
    setCmdState(cmeRemapCodePage,False);
    setCmdState(cmeHTMLAccents,False);
    setCmdState(cmeHTMLTag2Accent,False);
    setCmdState(cmeGPushCursorPos,False);
   }

 if (!(modifFilesOps & mfoDontCheckInIdle) && e && e->checkDiskCopyChanged())
    AskReloadEditor(GetCurrentIfEditorWindow());

 if (ShowClock)
   {
    if (!Clock)
       CreateClock();
    Clock->update();
   }
 else
    if (Clock)
       KillClock();

 if (UseScreenSaver && TScreen::useScreenSaver())
   {
    int seconds=inIdleTime/clockResolution;
    if (seconds==screenSaverTimeMouse)
      {
       MouseEventType me;
       TMouse::getEvent(me);
       TRect r=deskTop->getExtent();
       if (me.where.x==r.b.x-1 && me.where.y==0)
          screenSaver();
      }
    else
    if (seconds>=screenSaverTime)
       screenSaver();
   }
}


void OpenFileFromEditor(char *fullName)
{
 editorApp->openEditor(fullName,True);
}

#define README_1ST "readme.1st"

static
void ShowAboutStartBox(void)
{
 if (AboutStartBox())
   {// Check to avoid showing it more than ones
    if (EnvirGetIntVar("SET_README_SHOWN")>=TCEDITOR_VERSION)
       return;
    EnvirSetIntVar("SET_README_SHOWN",TCEDITOR_VERSION);

    // Load the readme.1st
    #ifdef NoHomeOrientedOS
    char Name[PATH_MAX];
    GetPathRelativeToRunPoint(Name,"contrib/setedit.bin/",README_1ST);
    if (!edTestForFile(Name)) // The simplified distribution have a more simple name
       GetPathRelativeToRunPoint(Name,"texts/",README_1ST);
    #else
    char *Name=ExpandFileNameToThePointWhereTheProgramWasLoaded(README_1ST);
    if (!edTestForFile(Name))
       Name=ExpandFileNameToThePointWhereTheProgramWasLoaded(README_1ST ".gz");
    #endif

    if (!edTestForFile(Name))
      {
       messageBox(__("I can't find the readme.1st file, please look for it in the .ZIP and read the file."),mfOKButton);
       //messageBox(Name,mfOKButton);
       return;
      }
    editorApp->openEditor(Name,True,NULL,oedForceRO | oedForgetResume | oedZoom);
   }
}

static
int GuessOneSET_FILES(const char *OSShareDir, char isAbsolute)
{
 // The fool doesn't put SET_FILES in the autoexec.bat, will try to guess here
 char Name[PATH_MAX];
 char *end;

 if (isAbsolute)
   {
    strcpy(Name,OSShareDir);
    strcat(Name,"/share/setedit");
    end=Name+strlen(Name);
    strcat(Name,"/" SHLFile);
   }
 else
   end=GetPathRelativeToRunPoint(Name,OSShareDir,SHLFile);

 if (!edTestForFile(Name))
    // Bad luck guessing
    return 1;

 *end=0;
 char *b=new char[12+strlen(Name)];
 sprintf(b,"SET_FILES=%s",Name);
 putenv(b);
 return 0;
}

const char *OSShareDir[]=
{// First from the configuration script
 CONFIG_PREFIX,
 #ifdef NoHomeOrientedOS
 "share/setedit/",
 #else
 "/usr",
 "/usr/local",
 #endif
 0
};

char OSShareDirT[]=
{
 1,
 #ifdef NoHomeOrientedOS
 0
 #else
 1,1
 #endif
};

static
int GuessSET_FILES()
{
 int ret=1,i=0;
 do
   {
    if (OSShareDir[i][0])
       ret=GuessOneSET_FILES(OSShareDir[i],OSShareDirT[i]);
   }
 while (ret && OSShareDir[++i]);
 return ret;
}

static
int GuessOneSET_LIBS(const char *OSShareDir, char isAbsolute)
{
 char Name[PATH_MAX];
 char *end;

 if (isAbsolute)
   {
    strcpy(Name,OSShareDir);
    strcat(Name,"/lib/setedit");
    end=Name+strlen(Name);
    strcat(Name,"/datetools.so");
   }
 else
   end=GetPathRelativeToRunPoint(Name,OSShareDir,"datetools.so");

 if (!edTestForFile(Name))
    // Bad luck guessing
    return 1;

 *end=0;
 char *b=new char[12+strlen(Name)];
 sprintf(b,"SET_LIBS=%s",Name);
 putenv(b);
 return 0;
}

static
int GuessSET_LIBS()
{
 int ret=1,i=0;
 do
   {
    if (OSShareDir[i][0])
       ret=GuessOneSET_LIBS(OSShareDir[i],OSShareDirT[i]);
   }
 while (ret && OSShareDir[++i]);
 return ret;
}

#ifdef NoHomeOrientedOS
static
int GuessINFOPATH(void)
{
 char Name[PATH_MAX];
 char *end=GetPathRelativeToRunPoint(Name,"info/",EditorFileExt);

 if (!edTestForFile(Name))
    return 1;

 *end=0;
 /**** No longer needed
 sprintf(Val,"INFOPATH=%s",Name);
 putenv(Val);
 *****/
 InfViewAddInfoDir(Name);
 return 0;
}
#endif

static
void ShowInstallError(char *var, const char *suggest, int end)
{
 TScreen::suspend();
 fprintf(stderr,_("\nWrong installation! You must define the %s environment variable.\n"),var);
 fprintf(stderr,_("Read the readme.1st file included in the .zip distribution file.\n\n"));
 #ifdef NoHomeOrientedOS
 char *s=getenv("DJDIR");
 if (s)
   fprintf(stderr,_("I suggest that: SET %s=%s/%s\n\n"),var,s,suggest);
 #else
 fprintf(stderr,_("I suggest that: SET %s=%s\n\n"),var,suggest);
 #endif
 fflush(stderr);
 if (end)
    exit(1);
 fprintf(stderr,_("press ENTER to continue\n"));
 getchar();
 TScreen::resume();
}

static
void ShowErrorSET_FILES()
{
 TScreen::suspend();
 fputs(_("\nYou defined SET_FILES wrongly, it doesn't point to a directory.\n\n"),stderr);
 fflush(stderr);
 exit(1);
}

extern void RestoreScreen();

/*************************************************************************************
   Memory full protection, I hope it will be usefull with djgpp v2.02
*************************************************************************************/

#if (defined(SEComp_GCC) && __GNUC__>=3) || !defined(SEComp_GCC)
// Not present in gcc 3.0.1
#define InitSafetyPool()
#define DeInitSafetyPool()
#else
#include <tv/no_mss.h>
#include NEW_HEADER
#include <tv/yes_mss.h>

static char *safetypool;

static void mynewhandler(void)
{
 if (safetypool==NULL)
    _exit(1); // We can't call exit because it can need malloc
 delete [] safetypool;
 safetypool=NULL;
 set_new_handler(NULL);
 messageBox(__("Memory is nearly full. Please exit, and restart."), mfOKButton | mfError);
}

static void InitSafetyPool()
{
 safetypool=new char [64000];
 set_new_handler(mynewhandler);
}

static void DeInitSafetyPool()
{
 delete[] safetypool;
}
#endif
/*************************************************************************************/

static TNoSortedStringCollection *FilesToLoad=0;

static
void AddToListOfFilesToLoad(const char *fileName)
{
 if (!fileName) return;
 FILE *f=fopen(fileName,"rt");
 if (!f) return;
 if (!FilesToLoad) FilesToLoad=new TNoSortedStringCollection(10,10);

 char Name[PATH_MAX],*s,*b;
 while (!feof(f))
   {
    b=fgets(Name,PATH_MAX,f);
    if (b)
      {
       for (s=b; *s; s++)
           if (*s=='\n') *s=0;
       if (*b)
          FilesToLoad->insert(newStr(b));
      }
   }
 fclose(f);
}

/******* Command line parsing *******/
static char *ProjectAskedByUser=0;
static char  RedirectStderr=1,CommandLineParsed=0,DisableBoardMixer=0;
// SAA: CascadeWindows conflicts with the CascadeWindows Win32 API
static char  TileVertical=0,TileHorizontal=0,SE_CascadeWindows=0;
// Red Hat 5.2 keyboard layout
static char  UseRH52=0;
extern int   use_mouse_handler;
// By default dump the stack
static char StackDbgStrategy=DBGST_DUMP_STACK;
#ifdef SECompf_djgpp
extern char useBIOS_VGA_State;
#endif

static
int CountExtraCMDLine(char *s)
{
 int inStr=0;
 int count=0;
 while (*s)
   {
    for (;*s && ucisspace(*s); s++);
    if (*s) count++;
    for (;*s && (!ucisspace(*s) || inStr); s++)
       {
        if (*s=='\\' && s[1])
           s++;
        else
           if (*s=='"')
              inStr=1-inStr;
       }
   }
 return count;
}

static
void GetExtraCMDLine(char *s, char *a[])
{
 int inStr=0,i=0;
 char *e,*n,*n2;
 while (*s)
   {
    for (;*s && ucisspace(*s); s++);
    for (e=s; *e && (!ucisspace(*e) || inStr); e++)
       {
        if (*e=='\\' && e[1])
           e++;
        else
           if (*e=='"')
              inStr=1-inStr;
       }
    n2=n=new char[e-s+1];
    for (; s<e; s++)
       {
        if (*s=='"') continue;
        if (*s=='\\' && s[1]=='"') { *(n2++)='"'; s++; continue; }
        *(n2++)=*s;
       }
    *n2=0;
    a[++i]=n;
   }
}

int Argc,CMDLineDelete=0;
char **Argv;

static
void DestroyCMDLine()
{
 if (CMDLineDelete)
   {
    for (int i=0; i<Argc; i++)
        delete[] Argv[i];
    delete[] Argv;
   }
}

static
struct CLY_option longopts[] =
{
  { "bios-keyb",      0, 0, 'b' }, // obsolete
  { "no-bios-keyb",   0, 0, 'B' }, // obsolete
  { "cascade",        0, 0, 'c' },
  { "stack-dbg",      1, 0, 'd' },
  { "file-list",      1, 0, 'f' },
  { "help",           0, 0, 'h' },
  { "use-rh-52-keys", 0, 0, 'k' },
  { "keybind",        1, 0, 'K' },
  { "force-no-lfn",   0, 0, 'l' },
  { "force-lfn",      0, 0, 'L' },
  { "no-mouse-hook",  0, 0, 'm' },
  { "no-mixer",       1, 0, 'M' },
  { "project",        1, 0, 'p' },
  { "no-redirect",    0, 0, 'r' },
  { "low-vga-save",   0, 0, 'S' },
  { "tile-vert",      0, 0, 't' },
  { "tile-horiz",     0, 0, 'T' },
  { 0, 0, 0, 0 }
};

static
void ParseCommandLine(int argc, char *argv[])
{
 if (CommandLineParsed)
    return;
 TProgInit::config=new TVMainConfigFile();

 int optc;
 char *ExtraCMDLine=getenv("SET_CMDLINE");
 int  c=0;
 Argv=argv;
 Argc=argc;

 #ifdef SEOS_Win32
 char* argv0 = Argv[0];
 while (*argv0)
   {
    if (*argv0 == '\\') *argv0 = '/';
    argv0++;
   }
 #endif

 if (ExtraCMDLine)
   {
    c=CountExtraCMDLine(ExtraCMDLine);
    if (c)
      {
       CMDLineDelete=1;
       Argc+=c;
       Argv=new char *[Argc];
       Argv[0]=newStr(argv[0]);
       GetExtraCMDLine(ExtraCMDLine,Argv);
       for (int i=1; i<argc; i++)
           Argv[i+c]=newStr(argv[i]); // Copy it because getopt_long can move entries
      }
   }

 ExtraCMDLine=getenv("SET_STACKDBG");
 if (ExtraCMDLine)
    StackDbgStrategy=atoi(ExtraCMDLine);

 while ((optc=CLY_getopt_long(Argc,Argv,"b:Bcd:fhkK:lLmMp:rStT:",longopts,0))!=EOF)
   {
    switch (optc)
      {
       case 'c':
            SE_CascadeWindows=1;
            break;
       case 'd':
            StackDbgStrategy=atoi(CLY_optarg);
            break;
       case 'f':
            AddToListOfFilesToLoad(CLY_optarg);
            break;
       case 'k':
            UseRH52=1;
            break;
       case 'K':
            KeyBindFNameUser=newStr(CLY_optarg);
            break;
       case 'l':
            putenv("LFN=N");
            break;
       case 'L':
            putenv("LFN=Y");
            break;
       case 'm':
            TVMainConfigFile::Add("DOS","PollMouse",1);
            break;
       case 'M':
            DisableBoardMixer=1;
            break;
       case 'p':
            ProjectAskedByUser=CLY_optarg;
            break;
       case 'r':
            RedirectStderr=0;
            break;
       case 'S':
            #ifdef SECompf_djgpp
            useBIOS_VGA_State=0;
            #endif
            break;
       case 't':
            TileVertical=1;
            break;
       case 'T':
            TileHorizontal=1;
            break;

       case 'b':
            TVMainConfigFile::Add("DOS","BIOSKey",1);
            break;

       case 'B':
            TVMainConfigFile::Add("DOS","BIOSKey",0L);
            break;

       case 'h':
       default:
            #define PrintHelp(a) printf(a)
            #define FlushHelp()  fflush(stdout)
            TScreen::suspend();

            PrintHelp(_("Setedit "));
            PrintHelp(TCEDITOR_VERSION_STR);
            PrintHelp(_(". Copyright (c) "));
            PrintHelp(TCEDITOR_C_YEAR);
            PrintHelp(_(" by Salvador E. Tropea\n\n"));

            PrintHelp(_("setedit [options] [file_name ...]\n\n"));
            PrintHelp(_("Valid options are:\n"));
            PrintHelp(_("+[line number]           jumps to the specified line. It only affects the next\n"
                        "                         file in the list and should be specified after the\n"
                        "                         options. If the line number is omitted you'll jump to\n"
                        "                         the end of the text. Example: +6 file\n"));
            PrintHelp(_("-c, --cascade:           arranges the windows using cascade style.\n"));
            PrintHelp(_("-d, --stack-dbg=n:       indicates which method will be used in the event of a\n"
                        "                         crash. The default method is 0.\n"
                        "                         0: dump unsaved buffers and stack calls.\n"
                        "                         1: do nothing (conservative).\n"));
            #ifdef SEOS_UNIX
            PrintHelp(_("                         2: dump unsaved buffers and call debugger to get\n"
                        "                            information. Then die.\n"
                        "                         3: like 2 but stay in the debugger.\n"));
            #endif
            PrintHelp(_("-f, --file-list file_n:  loads the files listed in file_n, each line in this\n"
                        "                         file must contain only one file name.\n"));
            PrintHelp(_("-h, --help:              displays this text ;-).\n\n"));
            #ifdef SEOSf_Linux
            PrintHelp(_("-k, --use-rh-52-keys:    enables the Red Hat 5.2 style keyboard mapping.\n"));
            #endif
            PrintHelp(_("-K, --keybind file_name: uses the specified name as keybinding file.\n"));
            #ifdef SECompf_djgpp // Don't name it under Linux
            PrintHelp(_("-l, --force-no-lfn:      avoids the use of long file names under W9x.\n"));
            PrintHelp(_("-L, --force-lfn:         forces the use of long file names under W9x.\n"));
            PrintHelp(_("-m, --no-mouse-hook:     don't hook the mouse interrupt, poll it.\n"));
            #endif
            #ifdef HAVE_MIXER
            PrintHelp(_("-M, --no-mixer:          disable board level mixer.\n"));
            #endif
            PrintHelp(_("-p, --project file_name: loads the indicated project, if the file doesn't exist\n"
                        "                         the editor creates a new one\n"));
            PrintHelp(_("-r, --no-redirect:       disables the stderr redirection. Only used during\n"
                        "                         debugging.\n"));
            #ifdef SECompf_djgpp // Don't name it under Linux
            PrintHelp(_("-S, --low-vga-save:      use low level functions to save/restore VGA state.\n"));
            #endif
            PrintHelp(_("-t, --tile-vert:         the windows are arranged vertically.\n"));
            PrintHelp(_("-T, --tile-horiz:        the windows are arranged horizontally.\n"));
            FlushHelp();
            exit(1);
            break;
      }
   }
 CommandLineParsed=1;
}
/******* End of Command line parsing *******/

void TSetEditorApp::dosShell()
{
 if (TScreen::noUserScreen())
    return;
 SaveAllEditors(); // To avoid crashes and inconsistences
 FullSuspendScreen();
 if (RedirectStderr && StdErrNew!=-1)
   dup2(StdErrOri,STDERR_FILENO); // Restore stderr
 // Stop the playing engine
 MP3Suspend;
 TScreen::System(CLY_GetShellName());
 MP3Resume;
 if (RedirectStderr && StdErrNew!=-1)
    dup2(StdErrNew,STDERR_FILENO); // Redirected again
 FullResumeScreen();
 ReLoadModifEditors();
}


#ifdef SECompf_djgpp
// Command line options from crt0 module
extern int   __crt0_argc;
extern char **__crt0_argv;
// DJGPP share options
extern int __djgpp_share_flags;

static //__attribute__ ((constructor)) Fails for gcc 3.2.2
void initProgram(void)
{
 // MUST BE ON or some frt files will fail
 __system_flags|=__system_allow_multiple_cmds;
 // Fine tune the share flags
 __djgpp_share_flags=SH_DENYWR;
 ParseCommandLine(__crt0_argc,__crt0_argv);
}

// This trick is needed to call the initProgram() function
class WrapperClassToCallConstructor
{
public:
 WrapperClassToCallConstructor() {initProgram();};
};
static WrapperClassToCallConstructor theInitCaller;
#endif

/**[txh]********************************************************************

  Description:
  Opens the specified file or sets the jump line. That's used only to files
specified in the command line.

***************************************************************************/

static
void LoadSpecifiedFile(char *name)
{
 static int LineToJump=-2;
 char tmpPath[PATH_MAX];

 if (name[0]=='+' && (name[1]==0 || ucisdigit(name[1])))
   { // That's +line_number
    if (!name[1]) // Nothing => jump to end
       LineToJump=-1;
    else
       LineToJump=atoi(name+1);
   }
 else
   {
    strcpy(tmpPath,name);
    CLY_fexpand(tmpPath);
    TCEditWindow *ed=editorApp->openEditor(tmpPath,True);
    if (ed && LineToJump!=-2)
      {
       if (LineToJump<0)
          LineToJump=ed->editor->limit.y;
       ed->editor->GoAndSelectLine(LineToJump);
       ed->editor->trackCursor(True);
       LineToJump=-2;
      }
   }
}

/**[txh]********************************************************************

  Description:
  This function should be as safe as possible and just stop redirecting.
  Is called during a catastrophic crash.
  
***************************************************************************/

void StopStdErrRedirection()
{
 if (RedirectStderr && StdErrNew!=-1)
   {
    dup2(StdErrOri,STDERR_FILENO);
    close(StdErrNew);
    close(StdErrOri);
   }
}

// This helps to use memory debuggers like MSS. In this way we don't need to
// compile libpcre with the memory debugger nor see spureous reports about
// memory allocated by libpcre and freed by the editor.
static
void *My_pcre_malloc(size_t a)
{
 return malloc(a);
}

static
void My_pcre_free(void *p)
{
 free(p);
}

static
void InitPCRELibrary()
{
 if (SUP_PCRE)
   {
    pcre_malloc=My_pcre_malloc;
    pcre_free=My_pcre_free;
   }
}

int main(int argc, char *argv[])
{
 TSetEditorApp::oldCPCallBack=TVCodePage::SetCallBack(TSetEditorApp::cpCallBack);
 // Avoid handling Alt+N for window selection in TV core.
 TProgram::doNotHandleAltNumber=1;
 
 ParseCommandLine(argc,argv);
 CheckIfCurDirValid();

 //  The point where the exe was loaded is a reference. Before SET_FILES because I use
 //  it to guess.
 SetReferencePath(Argv[0]);
 //  That's a new policy, after releasing v0.4.1 and saw that nobody bothers about
 //  reading the readme.1st (mainly because of the distribution structure) now the
 //  editor assumes that:
 //  [In 0.4.14 I added a guess to help fools]
 char *set_files=getenv("SET_FILES");
 if (set_files && !IsADirectory(set_files) && GuessSET_FILES())
    ShowErrorSET_FILES();
 if (!set_files && GuessSET_FILES())
    ShowInstallError("SET_FILES",OSShareDir[1],1);

 char *set_libs=getenv("SET_LIBS");
 if (!set_libs)
    GuessSET_LIBS();

 // Redirect stderr to a unique file to catch any kind of errors.
 // We want it as soon as possible so any errors can be dumped there.
 // Note: We need SET_FILES to redirect under DOS, so it comes after SET_FILES stuff
 if (RedirectStderr)
    TemporalStdErr=RedirectStdErrToATemp(StdErrOri,StdErrNew);
 // After redirecting the error we can enable the routines that
 // traps signals and dumps important information.
 InitEditorSignals(StackDbgStrategy,argv[0],TemporalStdErr);

 InitPCRELibrary();

 // That's better for me, easier for incremental searchs
 TFileCollection::sortOptions=fcolAlphabetical | fcolCaseInsensitive;
 // Be sure we can store temporals, or at least do our best
 CheckForValidTMPDIR();

 //  After SET_FILES because it needs SET_FILES.
 InitEnvirVariables();

 // Check if the user have the INFOPATH defined. On UNIX InfView already makes a guess
 #ifdef NoHomeOrientedOS
 InfViewGetInfoDir();
 GuessINFOPATH();
 /****** Old code: I no longer test it, the installer avoids this situation.
         I also need to add 2 places: djgpp one and editor's one, a djgpp user
 installed the editor outside the djgpp tree and reported obvious problems.
 LoadInfoEnviroment();
 if (!getenv("INFOPATH") && GuessINFOPATH())
    ShowInstallError("INFOPATH","info",0);
 ********/
 #endif

 SetConfigDialogFunc(SetFileOpenDialogOptions);
 InitPaletteSystem();
 //#if TV_MAJOR_VERSION<2
 //setIntenseState();
 //#endif
 if (DisableBoardMixer)
    BoardMixerDisable();

 TSetEditorApp::WhichScrSaver=GetDefaultScreenSaver();

 InitSafetyPool();

 #ifdef USE_TSTRCOL
 ReservedWords = new TStrCol(20,5);
 UserWords     = new TStrCol(20,5);
 PascalRWords  = new TStrCol(20,5);
 ClipperRWords = new TStrCol(20,5);
 #else
 ReservedWords = new TStringCollection(20,5);
 UserWords     = new TStringCollection(20,5);
 PascalRWords  = new TStringCollection(20,5);
 ClipperRWords = new TStringCollection(20,5);
 #endif

 #define I(a) ReservedWords->insert((void *)a)
 I("int");        I("char");      I("unsigned");
 I("signed");     I("float");     I("double");
 I("short");      I("long");      I("void");
 I("bool");

 I("for");        I("while");     I("do");
 I("if");         I("else");      I("return");
 I("continue");   I("goto");      I("true");
 I("false");

 I("static");     I("const");      I("enum");
 I("struct");     I("extern");     I("sizeof");
 I("union");      I("typedef");    I("inline");
 I("register");   I("volatile");

 I("switch");     I("case");       I("default");
 I("break");

 I("new");        I("delete");

 I("class");      I("private");     I("protected");
 I("public");     I("this");        I("template");
 I("throw");      I("typename");    I("catch");
 I("virtual");    I("friend");      I("operator");
 I("try");
 // namespace stuff
 I("namespace");  I("using");
 ReservedWords->setOwnerShip(False);
 #undef I

 #define I(a) UserWords->insert((void *)a)
 I("uchar");      I("ushort");      I("uint16");
 I("uint32");
 UserWords->setOwnerShip(False);
 #undef I

 // List of reserved words for Turbo Pascal 5.0, if some of them isn't
 // reserved in GPC please comment this line.
 #define I(a) PascalRWords->insert((void *)a)
 I("absolute");   I("and");         I("array");
 I("begin");      I("case");        I("const");
 I("div");        I("do");          I("downto");
 I("else");       I("end");         I("external");
 I("file");       I("for");         I("forward");
 I("function");   I("goto");        I("if");
 I("implementation");               I("in");
 I("inline");     I("interface");   I("interrupt");
 I("label");      I("mod");         I("nil");
 I("not");        I("of");          I("or");
 I("packed");     I("procedure");   I("program");
 I("record");     I("repeat");      I("set");
 I("shl");        I("shr");         I("string");
 I("then");       I("to");          I("type");
 I("unit");       I("until");       I("uses");
 I("var");        I("while");       I("with");
 I("xor");
 PascalRWords->setOwnerShip(False);
 #undef I

 // Taked from the Clipper.EXE 5.2e
 #define I(a) ClipperRWords->insert((void *)a)
 I("announce");   I("begin");       I("break");
 I("call");       I("case");        I("declare");
 I("do");         I("else");        I("elseif");
 I("endcase");    I("enddo");       I("endif");
 I("exit");       I("external");    I("field");
 I("for");        I("function");    I("if");
 I("iif");        I("in");          I("init");
 I("local");      I("loop");        I("memvar");
 I("next");       I("otherwise");   I("parameters");
 I("private");    I("procedure");   I("public");
 I("recover");    I("return");      I("sequence");
 I("static");     I("step");        I("text");
 I("to");         I("then");        I("using");
 I("with");       I("while");
 ClipperRWords->setOwnerShip(False);
 #undef I

 PrintSetDefaults();

 LoadKeysForTCEditor(GetKeyBindFName(0));
 TCEditor::SHLGenList=new TNoCaseStringCollection(5,5);
 LoadSyntaxHighLightFile(ExpandHome(SHLFile),TCEditor::SHLArray,TCEditor::SHLGenList,
                         TCEditor::SHLCant);
 CLESetFileName(ExpandHome(CLEFile));
 // Setup the list of files we don't want to backup
 TCEditor::MakeBkpForIt=NBKPMakeIt;
 NBKPSetFileName(ExpandHome(NBKPFile),cmeEditNoBkp);
 NBKPSetSaveFileName(ExpandHomeSave(NBKPFile));

 //------ International support
 #ifndef NO_INTL_SUP
 #ifdef NoHomeOrientedOS
 if (!TVIntl::autoInit(EditorFile,getenv("SET_LOCALEDIR")))
   {
    char localedir[PATH_MAX];
    GetPathRelativeToRunPoint(localedir,"share/locale","");
    TVIntl::autoInit(EditorFile,localedir);
   }
 #else
 TVIntl::autoInit(EditorFile,getenv("SET_LOCALEDIR"));
 #endif
 #endif // NO_INTL_SUP
 //------ end of int. support

 // Initialize the MP3 Stuff, it must be done before loading the desktop.
 // It just disable some commands
 MP3Initialize;

 // This is a pre-load pass. It just retrieves information that's useful before
 // starting the application. A good example is the window size, is much better
 // to create a window of the desired size than creating an 80x25 window and
 // the resize.
 TSetEditorApp::preLoadDesktop(ProjectAskedByUser,CLY_optind<Argc);
 // Now create the application, it will init TV
 editorApp=new TSetEditorApp();
 // We finished the preload stuff
 TSetEditorApp::finishPreLoadDesktop();
 if (TScreen::avoidMoire)
    TCEditor::TabChar=TCEditor::oTabChar='.';

 // Set's the window title for our application (W9x,X,etc.)
 editorApp->SetTitle();

 TSetEditorApp::loadEditorDesktop(1,ProjectAskedByUser,CLY_optind<Argc);

 // Open all the files indicated in the command line
 while (CLY_optind<Argc)
   {
    LoadSpecifiedFile(Argv[CLY_optind]);
    CLY_optind++;
   }
 // Open files specified as lists
 if (FilesToLoad)
   {
    int i,c=FilesToLoad->getCount();
    for (i=0; i<c; i++)
        LoadSpecifiedFile((char *)FilesToLoad->at(i));
   }
 destroy0(FilesToLoad);
 if (TileVertical)
   {
    unsigned dsktOps=TApplication::deskTop->getOptions();
    TApplication::deskTop->setOptions(dsktOps | dsktTileVertical);
    editorApp->tile();
    TApplication::deskTop->setOptions(dsktOps);
   }
 if (TileHorizontal)
   {
    unsigned dsktOps=TApplication::deskTop->getOptions();
    TApplication::deskTop->setOptions(dsktOps & (~dsktTileVertical));
    editorApp->tile();
    TApplication::deskTop->setOptions(dsktOps);
   }
 if (SE_CascadeWindows)
    editorApp->cascade();

 ShowMenuLoadError();
 ShowKeyBindError();
 SLPInterfaceInit(ExpandHome("macros.slp"));

 ShowAboutStartBox();
 ShowTips(ExpandHome(TipsFName));

 editorApp->run();

 // That saves the desktop too, even if there isn't a project
 SaveProject();
 TSetEditorApp::DebugDeInitVars();
 CLY_destroy(TSetEditorApp::edHelper);

 if (TSetEditorApp::DeleteFilesOnExit)
   {
    KillFilesToKill();
    DeleteWildcard("*.bkp");
    DeleteWildcard("*.BKP");
    DeleteWildcard("*.dst");
    DeleteWildcard("*.epr");
   }

 ReleaseFilesToKill();
 ShutDownPaletteSystem();
 SaveEnviromentFile();

 /*
   Restore stderr
 */
 if (TemporalStdErr)
   {
    StopStdErrRedirection();

    struct stat s;
    if (stat(TemporalStdErr,&s)==0)
       if (s.st_size==0)
          unlink(TemporalStdErr);
    free(TemporalStdErr);
   }

 MP3DeInitialize;
 SyntaxSearch_ShutDown();
 UnLoadSyntaxHighLightFile(TCEditor::SHLArray,TCEditor::SHLGenList,TCEditor::SHLCant);
 UnloadCLEFile();
 UnloadNBKP();
 UnLoadTVMenu();
 if (TCEditor::RightClickMenu)
    delete TCEditor::RightClickMenu;
 CLY_destroy(editorApp);
 delete[] TSetEditorApp::WhichScrSaver; // static member
 SLPInterfaceDeInit();
 CLY_destroy(ReservedWords);
 CLY_destroy(UserWords);
 CLY_destroy(PascalRWords);
 CLY_destroy(ClipperRWords);
 DeInitEnvirVariables();
 DeInitSafetyPool();
 delete[] KeyBindFNameUser;
 DestroyCMDLine();
 DestroyFunctionList();
 RunExternalProgramFreeMemory();
 LoadKeysForTCEditorFreeMemory();
 TagsFreeMemory();
 PathListUnLoad();
 SpLinesCleanUp();
 op_cl_std_clean_up();

 return 0;
}
END_OF_MAIN()

