/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#define Uses_string
#define Uses_stdio
#define Uses_stdlib
#define Uses_TApplication
#define Uses_TEditWindow
#define Uses_TDeskTop
#define Uses_TRect
#define Uses_TEditor
#define Uses_TFileEditor
#define Uses_TFileDialog
#define Uses_TChDirDialog
#define Uses_TStatusLine
#define Uses_TCommandSet
#define Uses_TPalette
#define Uses_fpstream
#define Uses_MsgBox
#define Uses_TScreen
#define Uses_TGKey
#define Uses_IOS_BIN
#define Uses_fcntl
#define Uses_sys_stat
#define Uses_getopt
#define Uses_TCEditor_Commands
#define Uses_TNSSortedCollection

#define Uses_TSSortedListBox
#define Uses_TSLabel
#define Uses_TSButton
// InfView requests
#include <infr.h>

// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
// Second request the headers
#include <easydiag.h>

#include "infalone.h"

#include <stdarg.h>
#include <locale.h>
#include "inf.h"
#define Uses_TManWindow
#include <manview.h>
#include <codepage.h>

#if TV_MAJOR_VERSION>=2
#define TV_System TScreen::System
#define Uses_TScreen
#include <tv/screen.h>
#endif
#if TV_MAJOR_VERSION<2
 #define getWindowTitle GetWindowTitle
 #define setWindowTitle SetWindowTitle
#endif

#define INFVIEW_VERSION_STR "0.2.7"
const char *EditorFile="setedit";

TStatusLine *createStatusForInfView(TRect r);
TEditWindow *clipMiWindow;

static int views=0,infViews=0;

class TInfViewList: public TNSSortedCollection
{
public:
 TInfViewList() :
   TNSSortedCollection(3,3) {}

private:
 virtual int compare(void *key1, void *key2);
};

int TInfViewList::compare(void *key1, void *key2)
{
 return (ulong)key1-(ulong)key2;
}

TInfViewList *List=0;

static
void SetPluralCommands(Boolean enable)
{
 if (enable)
   {
    TView::enableCommand(cmTile);
    TView::enableCommand(cmCascade);
    TView::enableCommand(cmNext);
    TView::enableCommand(cmPrev);
   }
 else
   {
    TView::disableCommand(cmTile);
    TView::disableCommand(cmCascade);
    TView::disableCommand(cmNext);
    TView::disableCommand(cmPrev);
   }
}

static
void IncrementViews()
{
 if (views)
    SetPluralCommands(True);
 views++;
}

static
void DecrementViews()
{
 views--;
 if (views==1)
    SetPluralCommands(False);
}

static
void IncrementInfViews(TView *p)
{
 infViews++;
 IncrementViews();
 List->insert(p);
}

static
void DecrementGenericView(TView *p)
{
 ccIndex pos;
 if (List->search(p,pos))
   {
    infViews--;
    List->remove(p);
   }
 DecrementViews();
}

TEditorMiApp::TEditorMiApp() :
    TProgInit( TEditorMiApp::initStatusLine,
               TEditorMiApp::initMenuBar,
               TEditorMiApp::initDeskTop
             ),
    TApplication()
{// Disable all the commands that needs at least one view opened
 TInfViewer::DisableAllCommands();
 SetPluralCommands(False);
 curCodePage=GetCurrentOSCodePage();
}

void TEditorMiApp::dosShell()
{
 suspend();
 TV_System(CLY_GetShellName());
 resume();
 Redraw();
}

void TEditorMiApp::tile()
{
 deskTop->tile(deskTop->getExtent());
}

void TEditorMiApp::cascade()
{
 deskTop->cascade(deskTop->getExtent());
}


void TEditorMiApp::ManPageView()
{
 ManPageOptions *op;
 TDialog *d=ManPageViewSelect(0,&op);
 if (execDialog(d,op)==cmOK)
   {
    TView *view=CreateManWindow(op->program,op->section,op->options);
    if (validView(view))
      {
       view->options|=ofTileable;
       deskTop->insert(view);
       IncrementViews();
      }
   }
}


void TEditorMiApp::handleEvent( TEvent& event )
{
 TInfWindow *w;

 TApplication::handleEvent( event );
 if (event.what==evBroadcast)
   {
    if (event.message.command==cmClosingWindow)
       DecrementGenericView((TView *)event.message.infoPtr);
    else
       return;
   }
 else
 if (event.what!=evCommand)
    return;
 else
    {
     switch( event.message.command )
        {
         case cmInfMainOpen:
              if (!infViews)
                {
                 TInfFile *i=new TInfFile("dir");
                 w=new TInfWindow(i,"");
                 if (validView(w))
                   {
                    w->options|=ofTileable;
                    deskTop->insert(w);
                    IncrementInfViews(w);
                   }
                 else
                   return;
                }
              else
                {
                 w=(TInfWindow *)List->at(0);
                 if (deskTop->current!=w)
                    w->select();
                }
              event.message.command=cmInfOpen;
              w->handleEvent(event);
              break;

         case cmInfView:
             {
              TInfFile *i=new TInfFile("dir");
              TInfWindow *w = new TInfWindow(i,"");
              if (validView(w))
                {
                 w->options|=ofTileable;
                 deskTop->insert(w);
                 IncrementInfViews(w);
                }
             }
             break;

         case cmAbout:
             {
              TInfFile *i=new TInfFile("infview");
              TInfWindow *w = new TInfWindow(i,"About the Author");
              if (validView(w))
                {
                 w->options|=ofTileable;
                 IncrementInfViews(w);
                 deskTop->insert(w);
                }
              w->zoom();
             }
              break;

         case cmManPage:
              ManPageView();
              break;

         case cmTile:
             tile();
             break;

         case cmCascade:
             cascade();
             break;

         case cmScreenConf:
              SetScreenOps();
              break;

         case cmDosShell:
              dosShell();
              break;

         default:
             return;
        }
    }
 clearEvent(event);
}


// Palette from setedit, the only way to maintain it in sync
#include <pal.h>

char SEcpColor[]     ={ SE_cpColor 0 };
char SEcpBlackWhite[]={ SE_cpBlackWhite 0 };
char SEcpMonochrome[]={ SE_cpMonochrome 0 };

TPalette& TEditorMiApp::getPalette() const
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

/************************* Code page options ***********************************/

TDialog *CreateScreenOpsDialog()
{
 TSViewCol *col=new TSViewCol(__("Screen Options"));

 TSLabel *lbe1=new TSLabel(_("Primary ~e~ncoding"),
                           new TSSortedListBox(24,10,tsslbVertical));
 col->insert(2,2,lbe1);
 EasyInsertOKCancel(col);

 TDialog *d=col->doIt();
 delete col;

 d->options|=ofCentered;
 return d;
}

#pragma pack(1)
typedef struct
{
 TCollection *font_enco  CLY_Packed;
 ccIndex font_encs       CLY_Packed;
} ScreenBox;
#pragma pack()

void TEditorMiApp::SetCodePage(int cp)
{
 curCodePage=cp;
 RemapCharactersFor(curCodePage);
 deskTop->setState(sfVisible,True);
 deskTop->Redraw();
 //redraw();
}

void TEditorMiApp::SetScreenOps(void)
{
 if (!TScreen::codePageVariable())
   {
    messageBox(_("This terminal have a fixed code page"),mfError | mfOKButton);
    return;
   }
 ccIndex oldE1;

 ScreenBox box;
 box.font_enco=GetCodePagesList();
 oldE1=box.font_encs=CodePageIDToIndex(curCodePage);

 TDialog *d=CreateScreenOpsDialog();

 if (execDialog(d,&box)==cmOK && oldE1!=box.font_encs)
    SetCodePage(IndexToCodePageID(box.font_encs));
}



/************************* READ/WRITE desktop files ***********************************/

const char *Signature="InfView desktop file\x1A";

void TEditorMiApp::saveDesktop(const char *fName)
{
 fpstream *f=new fpstream(fName,CLY_std(ios::out) | CLY_IOSBin);

 if (f)
   {
    f->writeString(Signature);
    storeDesktop(*f);
    if (!f)
      {
       messageBox(mfOKButton | mfError,_("Could not create %s."),fName);
       f->close();
       ::remove(fName);
      }
    else
       f->close();
   }
 delete f;
}

static
void writeView(TView *p, void *strm)
{
 fpstream *s=(fpstream *)strm;
 if (p!=TProgram::deskTop->last)
   {
    ccIndex pos;
    uchar IsInfView=List->search(p,pos) ? 1 : 0;
    *s << IsInfView << p;
   }
}

void TEditorMiApp::storeDesktop(fpstream& s)
{
 s << 2; // Version
 s << curCodePage << TInfViewer::BookMark;
 deskTop->forEach(::writeView,&s);
 s << 0;
}

void TEditorMiApp::retrieveDesktop(const char *name, int loadWindows)
{
 if (name)
   {
    #ifdef BROKEN_CPP_OPEN_STREAM
    // In this way we avoid the destruction of the file
    int h=open(name, O_RDONLY | O_BINARY);
    if (h<0)
       return;
    fpstream *f=new fpstream(h);
    #else
    fpstream *f=new fpstream(name,CLY_std(ios::in) | CLY_IOSBin);
    #endif

    if (!f)
       messageBox(_("Could not open desktop file"), mfOKButton | mfError);
    else
      {
       char *s=f->readString();
       if (strcmp(s,Signature)!=0)
          messageBox(_("Wrong desktop file"), mfOKButton | mfError);
       else
         {
          loadDesktop(*f,loadWindows);
          if (!f)
             messageBox(_("Error reading desktop file"), mfOKButton | mfError);
         }
       delete[] s;
       f->close();
      }
    delete f;
   }
}

static
void closeView(TView *p, void *p1)
{
 message(p, evCommand, cmClose, p1);
}

void TEditorMiApp::loadDesktop(fpstream &s, int loadWindows)
{
 TView  *p;
 int version;

 if (deskTop->valid(cmClose))
   {
    if (loadWindows)
       deskTop->forEach(::closeView,0);  // Clear the desktop
    s >> version;
    if (version>=2)
      {
       s >> curCodePage;
       SetCodePage(curCodePage);
      }
    s >> TInfViewer::BookMark;
    if (!loadWindows)
       return;
    if (version<2)
      {
       do
         {
          s >> p;
          if (validView(p))
            {
             deskTop->insertBefore(p,deskTop->last);
             IncrementInfViews(p);
            }
         }
       while (p);
      }
    else
      {
       uchar IsInfView;
       do
         {
          s >> IsInfView >> p;
          if (validView(p))
            {
             deskTop->insertBefore(p,deskTop->last);
             if (IsInfView)
                IncrementInfViews(p);
             else
                IncrementViews();
            }
         }
       while (p);
      }
   }
}
/******************** End of READ/WRITE desktop files *******************************/

/******* Command line parsing *******/
static char  CommandLineParsed=0;
// Red Hat 5.2 keyboard layout
static char  UseRH52=0;
static char *FileToLoad=0,*JumpTo=0,*desktopIn=0,*desktopOut=0;

static
struct option longopts[] =
{
  { "directory", 1, 0, 'd' },
  { "file", 1, 0, 'f' },
  { "force-no-lfn", 0, 0, 'l' },
  { "force-lfn", 0, 0, 'L' },
  { "node", 1, 0, 'n' },
  { "read-dkt", 1, 0, 'r' },
  { "save-dkt", 1, 0, 's' },
  { "use-rh-52-keys", 0, 0, 'k' },
  { "help", 0, 0, 'h' },
  { 0, 0, 0, 0 }
};

static
void ParseCommandLine(int argc, char *argv[])
{
 if (CommandLineParsed)
    return;
    
 int optc;

 while ((optc=getopt_long(argc,argv,"d:f:lLn:kr:s:h",longopts,0))!=EOF)
   {
    switch (optc)
      {
       case 'l':
            putenv("LFN=N");
            break;
       case 'L':
            putenv("LFN=Y");
            break;
       case 'k':
            UseRH52=1;
            break;
       case 'd':
            InfViewAddInfoDir(optarg);
            break;
       case 'f':
            FileToLoad=optarg;
            break;
       case 'n':
            JumpTo=optarg;
            break;
       case 'r':
            desktopIn=optarg;
            break;
       case 's':
            desktopOut=optarg;
            break;
       case 'h':
       default:
            TScreen::suspend();
            fprintf(stderr,_("InfView v"INFVIEW_VERSION_STR". Copyright (c) 1996-2001 by Salvador E. Tropea\n\n"));
            fprintf(stderr,_("infview [option]... [info_file [menu_item...]]\n\n"));
            fprintf(stderr,_("Valid options are:\n"));
            fprintf(stderr,_("-d, --directory DIR      adds a directory to the list of directories to search\n"
                             "                         (DIR to INFOPATH)\n"));
            fprintf(stderr,_("-f, --file FILENAME      load FILENAME info file.\n"));
            #ifdef TVCompf_djgpp // Don't name it under Linux
            fprintf(stderr,_("-l, --force-no-lfn       avoids the use of long file names under W9x.\n"));
            fprintf(stderr,_("-L, --force-lfn          forces the use of long file names under W9x.\n"));
            #else
            fprintf(stderr,_("-k, --use-rh-52-keys     enables the Red Hat 5.2 style keyboard mapping.\n"));
            #endif
            fprintf(stderr,_("-n, --node NODE          jumps to this node.\n"));
            fprintf(stderr,_("-r, --read-dkt FILENAME  if no file is opened use this desktop file.\n"));
            fprintf(stderr,_("-s, --save-dkt FILENAME  save the desktop to this file at exit.\n"));
            fprintf(stderr,_("-h, --help               displays this text ;-).\n\n"));
            fprintf(stderr,_("The first argument, if present, is the name of the Info file to read.\n"
                             "Any remaining arguments are treated as the names of menu items in the initial\n"));
            #ifdef TVOSf_Linux
            fprintf(stderr,_("node visited.  For example, `infview libc \"function index\" printf' moves to the\n"
                             "node `Function Index' and then to `printf' in the info file `libc'.\n\n"));
            #else
            fprintf(stderr,_("node visited.  For example, `infview libc alpha printf' moves to the node\n"
                             "`Alphabetical list' and then to `printf' in the info file `libc'.\n\n"));
            #endif
            fprintf(stderr,_("Also note that info files are searched in the INFOPATH directories. To load a\n"
                             "file stored in the current directory add ./ at the beginning of the name.\n"
                             "Email bug reports to salvador@inti.gov.ar or djgpp@delorie.com.\n"));
            fflush(stderr);
            exit(1);
            break;
      }
   }
 CommandLineParsed=1;
}
/******* End of Command line parsing *******/

const char *cDktNameOld="desktop.dkt";
const char *cDktName   ="infview.dkt";
TInfWindow *startInfo=0;

char *TryFileName(const char *path, const char *file)
{
 char aux[PATH_MAX];
 struct stat st;

 strcpy(aux,path);
 #ifdef TVOS_UNIX
 strcat(aux,".");
 #endif
 strcat(aux,file);

 if (stat(aux,&st)==0 && S_ISREG(st.st_mode))
    return strdup(aux);
 return 0;
}

char *StringCat(const char *s1, ...)
{
 va_list va;
 char *s,*ret;
 int len=strlen(s1)+1;

 va_start(va,s1);
 while ((s=va_arg(va,char *))!=0)
   len+=strlen(s);
 va_end(va);

 ret=new char[len];
 va_start(va,s1);
 strcpy(ret,s1);
 while ((s=va_arg(va,char *))!=0)
   strcat(ret,s);
 va_end(va);

 return ret;
}

void CreateDesktopNames(char *file)
{
 if (desktopIn  && desktopOut)
    return; // Nothing to do the user specified both

 char Share[PATH_MAX];
 char *pos=strrchr(file,'/');
 if (pos && pos-file>4 && strncmp(pos-3,"bin",3)==0)
   {
    strncpy(Share,file,pos-file-3);
    Share[pos-file-3]=0;
   }
 #ifdef TVCompf_djgpp
 else
 if ((pos=getenv("DJDIR"))!=0)
   {
    strcpy(Share,pos);
    strcat(Share,"/");
   }
 #endif
 else
    strcpy(Share,"./");
 strcat(Share,"share/infview/");

 #if defined(TVOS_UNIX) || defined(TVCompf_Cygwin)
 char Home[PATH_MAX];
 pos=getenv("HOME");
 if (!pos)
   {
    pos=getenv("HOMEDIR");
    if (!pos)
       pos=".";
   }
 strcpy(Home,pos);
 strcat(Home,"/");
 #endif

 // Solve the read point
 // Try with the new name
 if (!desktopIn)
   {// Here
    desktopIn=TryFileName("./",cDktName);
    #if defined(TVOS_UNIX) || defined(TVCompf_Cygwin)
    // User's home
    if (!desktopIn)
       desktopIn=TryFileName(Home,cDktName);
    #endif
    // Shared dir
    if (!desktopIn)
       desktopIn=TryFileName(Share,cDktName);
   }
 // Try with the old name
 if (!desktopIn)
   {// Here
    desktopIn=TryFileName("./",cDktNameOld);
    #if defined(TVOS_UNIX) || defined(TVCompf_Cygwin)
    // User's home
    if (!desktopIn)
       desktopIn=TryFileName(Home,cDktNameOld);
    #endif
    // Shared dir
    if (!desktopIn)
       desktopIn=TryFileName(Share,cDktNameOld);
    if (!desktopIn)
       desktopIn=(char *)cDktNameOld;
   }

 // Solve the write point
 if (!desktopOut)
   {
    #if defined(TVOS_UNIX) || defined(TVCompf_Cygwin)
    // Home
    desktopOut=StringCat(Home,".",cDktName,0);
    #else
    // Share
    desktopOut=StringCat(Share,cDktName,0);
    #endif
   }
}

void OpenInfView(TEditorMiApp *editorApp, char *name)
{
 TInfFile *i=new TInfFile(name);
 startInfo=new TInfWindow(i,"");
 if (editorApp->validView(startInfo))
   {
    editorApp->deskTop->insert(startInfo);
    startInfo->options|=ofTileable;
    IncrementInfViews(startInfo);
   }
}

void InsertEnviromentVar(const char *,const char *)
{
}

static
void InitInternacSup()
{
 //------ International support
 #ifndef NO_INTL_SUP
 char *locale_dir,localedir[PATH_MAX];
 setlocale(LC_ALL,"");
 // Use the LOCALEDIR var for the directory
 locale_dir=getenv("LOCALEDIR");

 if (!locale_dir)
   {
    #ifdef NoHomeOrientedOS
    // if LOCALEDIR doesn't exists use %DJDIR%/share/locale
    locale_dir=getenv("DJDIR");
    if (locale_dir)
      {
       strcpy(localedir,locale_dir);
       strcat(localedir,"/share/locale");
      }
    else
      {
       // if DJDIR doesn't exists use SET_FILES
       locale_dir=(char *)getenv("SET_FILES");
       if (locale_dir)
          strcpy(localedir,locale_dir);
       else
         {
          // if SET_FILES doesn't exists (imposible) use .
          // That's only to avoid a GPF in the strcpy and add a little facility
          localedir[0]='.';
          localedir[1]=0;
         }
      }
    #else
    strcpy(localedir,"/usr/share/locale");
    #endif
   }
 else
    strcpy(localedir,locale_dir);

 BINDTEXTDOMAIN(EditorFile,localedir);
 TEXTDOMAIN(EditorFile);
 #endif
 //------ end of int. support
}

int main(int argc, char *argv[])
{
 TEditorMiApp *editorApp;
 List=new TInfViewList();

 // Initialize the INFOPATH stuff
 InfViewGetInfoDir();
 InitInternacSup();
 ParseCommandLine(argc,argv);

 editorApp=new TEditorMiApp();
 #ifdef TVOSf_Linux
  #if TV_MAJOR_VERSION<2
 if (UseRH52)
    TGKey::SetKbdMapping(KBD_REDHAT52_STYLE);
  #endif
 #endif
 TScreen::setWindowTitle("InfView");
 if (FileToLoad)
   {
    OpenInfView(editorApp,FileToLoad);
    if (JumpTo)
       startInfo->viewer->switchToTopic(JumpTo);
   }
 while (optind<argc)
   {
    if (!FileToLoad)
      {// First parameter is the info file if no --file option was provided
       FileToLoad=argv[optind];
       OpenInfView(editorApp,FileToLoad);
       if (JumpTo)
          startInfo->viewer->switchToTopic(JumpTo);
      }
    else
      {// The rest are cross references
       if (startInfo && startInfo->viewer)
         {
          if (!startInfo->viewer->jumpXRefPartial(argv[optind],jmpXRSubStr | bestMVisibleName) &&
              // Try with the node name instead of the visible name:
              !startInfo->viewer->jumpXRefPartial(argv[optind],jmpXRSubStr))
            {
             messageBox(mfOKButton | mfError,_("Could not find '%s'."),argv[optind]);
             break;
            }
         }
      }
    optind++;
   } 
 if (JumpTo && !FileToLoad && *JumpTo=='(')
   {
    OpenInfView(editorApp,"dir");
    startInfo->viewer->switchToTopic(JumpTo);
   }

 CreateDesktopNames(argv[0]);
 editorApp->retrieveDesktop(desktopIn,startInfo==0);
 editorApp->run();
 editorApp->saveDesktop(desktopOut);
 destroy(editorApp);

 return 0;
}

