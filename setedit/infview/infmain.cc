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
#include <tv.h>

#include "infalone.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <strstream.h>
#include <iomanip.h>
#include "inf.h"
//#include <ssyntax.h>

#define INFVIEW_VERSION_STR "0.2.6"

#define _cpColor \
    "\x71\x70\x78\x74\x20\x28\x24\x17\x1F\x1A\x31\x31\x1E\x71\x00" \
    "\x37\x3F\x3A\x13\x13\x3E\x21\x00\x70\x7F\x7A\x13\x13\x70\x7F\x00" \
    "\x70\x7F\x7A\x13\x13\x70\x70\x7F\x7E\x20\x2B\x2F\x78\x2E\x70\x30" \
    "\x3F\x3E\x1F\x2F\x1A\x20\x72\x31\x31\x30\x2F\x3E\x31\x13\x00\x00" \
    /*"\x1E\x71"*/ \
 cInfColor \
    "\x17\x12\x1E\x1F\x1B\x15\x16\x3F\x2B\x1D" "\x1A\x2C\x13\x00\x00\x1C" \
    "\x1E\x71" \
    "\x17\x12\x1E\x1F\x1B\x15\x16\x3F\x2B\x1D\x1A\x2C\x13\x00\x00\x1C" \
    "\x1E\x71" \
    "\x17\x12\x1E\x1F\x1B\x15\x16\x3F\x2B\x1D\x1A\x2C\x13\x00\x00\x1C" \
    /* cCrossCur, cStatusLi, cMPHighL, cRectSel */ \
    "\x70\x7C\x5F\x67\xF0\xF0\xF0\xF0\xF0\xF0" 

#define _cpBlackWhite \
    "\x70\x70\x78\x7F\x07\x07\x0F\x07\x0F\x07\x70\x70\x07\x70\x00" \
    "\x07\x0F\x07\x70\x70\x07\x70\x00\x70\x7F\x7F\x70\x07\x70\x07\x00" \
    "\x70\x7F\x7F\x70\x07\x70\x70\x7F\x7F\x07\x0F\x0F\x78\x0F\x78\x07" \
    "\x0F\x0F\x0F\x70\x0F\x07\x70\x70\x70\x07\x70\x0F\x07\x07\x00\x00" \
    "\x1E\x71" \
    "\x17\x12\x1E\x1F\x1B\x15\x16\x3F\x2B\x1D\x1A\x2C\x13\x00\x00\x1C" \
    "\x1E\x71" \
    "\x17\x12\x1E\x1F\x1B\x15\x16\x3F\x2B\x1D\x1A\x2C\x13\x00\x00\x1C" \
    "\x1E\x71" \
    "\x17\x12\x1E\x1F\x1B\x15\x16\x3F\x2B\x1D\x1A\x2C\x13\x00\x00\x1C"\
    /* cCrossCur, cStatusLi, cMPHighL, cRectSel */ \
    "\x70\x7C\x5F\x67\xF0\xF0\xF0\xF0\xF0\xF0" \
    cInfBlackWhite

#define _cpMonochrome \
    "\x70\x07\x07\x0F\x70\x70\x70\x07\x0F\x07\x70\x70\x07\x70\x00" \
    "\x07\x0F\x07\x70\x70\x07\x70\x00\x70\x70\x70\x07\x07\x70\x07\x00" \
    "\x70\x70\x70\x07\x07\x70\x70\x70\x0F\x07\x07\x0F\x70\x0F\x70\x07" \
    "\x0F\x0F\x07\x70\x07\x07\x70\x07\x07\x07\x70\x0F\x07\x07\x00\x00" \
    "\x1E\x71" \
    "\x17\x12\x1E\x1F\x1B\x15\x16\x3F\x2B\x1D\x1A\x2C\x13\x00\x00\x1C" \
    "\x1E\x71" \
    "\x17\x12\x1E\x1F\x1B\x15\x16\x3F\x2B\x1D\x1A\x2C\x13\x00\x00\x1C" \
    "\x1E\x71" \
    "\x17\x12\x1E\x1F\x1B\x15\x16\x3F\x2B\x1D\x1A\x2C\x13\x00\x00\x1C"\
    /* cCrossCur, cStatusLi, cMPHighL, cRectSel */ \
    "\x70\x7C\x5F\x67\xF0\xF0\xF0\xF0\xF0\xF0" \
    cInfMonochrome

TStatusLine *createStatusForInfView(TRect r);
TEditWindow *clipMiWindow;


TEditorMiApp::TEditorMiApp() :
    TProgInit( TEditorMiApp::initStatusLine,
               TEditorMiApp::initMenuBar,
               TEditorMiApp::initDeskTop
             ),
    TApplication()
{
}


void TEditorMiApp::dosShell()
{
 suspend();
 system("cls");
 cout << "Type EXIT to return...";
 system( getenv( "COMSPEC"));
 resume();
 redraw();
}

void TEditorMiApp::tile()
{
 deskTop->tile( deskTop->getExtent() );
}

void TEditorMiApp::cascade()
{
 deskTop->cascade( deskTop->getExtent() );
}

void TEditorMiApp::handleEvent( TEvent& event )
{
 TApplication::handleEvent( event );
 if( event.what != evCommand )
     return;
 else
     switch( event.message.command )
        {
         case cmInfOpen: // If none infview window is opened we receive the event.
                         // In this case we just open a window and pass the event.
         case cmInfView:
             {
              TInfFile *i=new TInfFile("dir");
              TInfWindow *w = new TInfWindow(i,"");
              if (validView(w))
                {
                 w->options|=ofTileable;
                 deskTop->insert(w);
                 if (event.message.command==cmInfOpen)
                    w->handleEvent(event);
                }
             }
             break;

         case cmAbout:
             {
              TInfFile *i=new TInfFile("infview");
              TInfWindow *w = new TInfWindow(i,"About the Author");
              if (validView(w))
                 deskTop->insert(w);
              w->zoom();
             }
              break;

         case cmTile:
             tile();
             break;

         case cmCascade:
             cascade();
             break;

         default:
             return ;
        }
 clearEvent( event );
}


//
// getPalette() function ( returns application's palette )
//

TPalette& TEditorMiApp::getPalette() const
{
 static TPalette newcolor ( _cpColor cInfColor, sizeof( cpColor cInfColor )-1 );
 static TPalette newblackwhite( _cpBlackWhite cInfBlackWhite, sizeof( cpBlackWhite cInfBlackWhite)-1 );
 static TPalette newmonochrome( _cpMonochrome cInfMonochrome, sizeof( cpMonochrome cInfMonochrome)-1 );
 static TPalette *palettes[] =
     {
     &newcolor,
     &newblackwhite,
     &newmonochrome
     };
 return *(palettes[appPalette]);
}

/************************* READ/WRITE desktop files ***********************************/

const char *Signature="InfView desktop file\x1A";

void TEditorMiApp::saveDesktop(const char *fName)
{
 fpstream *f=new fpstream(fName,ios::out | IOS_BIN);

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
    *s << p;
}

void TEditorMiApp::storeDesktop(fpstream& s)
{
 s << 1; // Version
 s << TInfViewer::BookMark;
 deskTop->forEach(::writeView, &s);
 s << 0;
}

void TEditorMiApp::retrieveDesktop(const char *name)
{
 if (name)
   {
    // In this way we avoid the destruction of the file
    int h=open(name, O_RDONLY | O_BINARY);
    if (h<0)
       return;
    fpstream *f=new fpstream(h);

    if (!f)
       messageBox(_("Could not open desktop file"), mfOKButton | mfError);
    else
      {
       char *s=f->readString();
       if (strcmp(s,Signature)!=0)
          messageBox(_("Wrong desktop file"), mfOKButton | mfError);
       else
         {
          loadDesktop(*f);
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

void TEditorMiApp::loadDesktop(fpstream &s)
{
 TView  *p;
 int version;

 if (deskTop->valid(cmClose))
   {
    deskTop->forEach(::closeView,0);  // Clear the desktop
    s >> version;
    s >> TInfViewer::BookMark;
    do
      {
       s >> p;
       deskTop->insertBefore(validView(p),deskTop->last);
      }
    while (p);
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
            fprintf(stderr,_("InfView v"INFVIEW_VERSION_STR". Copyright (c) 1996-2000 by Salvador E. Tropea\n\n"));
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
            fprintf(stderr,"The first argument, if present, is the name of the Info file to read.\n"
                           "Any remaining arguments are treated as the names of menu items in the initial\n"
                           #ifdef TVOSf_Linux
                           "node visited.  For example, `infview libc \"function index\" printf' moves to the\n"
                           "node `Function Index' and then to `printf' in the info file `libc'.\n\n"
                           #else
                           "node visited.  For example, `infview libc alpha printf' moves to the node\n"
                           "`Alphabetical list' and then to `printf' in the info file `libc'.\n\n"
                           #endif
                           "Email bug reports to salvador@inti.gov.ar or djgpp@delorie.com.\n");
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
   }
}

void InsertEnviromentVar(const char *,const char *)
{
}

int main(int argc, char *argv[])
{
 TEditorMiApp *editorApp;

 // Initialize the INFOPATH stuff
 InfViewGetInfoDir();
 ParseCommandLine(argc,argv);

 editorApp=new TEditorMiApp();
 #ifdef TVOSf_Linux
 if (UseRH52)
    TGKey::SetKbdMapping(KBD_REDHAT52_STYLE);
 #endif
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
 if (!startInfo) // If no command line stuff try to get a desktop file
    editorApp->retrieveDesktop(desktopIn);
 editorApp->run();
 editorApp->saveDesktop(desktopOut);
 destroy(editorApp);

 return 0;
}

