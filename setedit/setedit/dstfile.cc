/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
//#define DEBUG
#include <ceditint.h>

#define Uses_fcntl
#include <sys/stat.h> /* for mode definitions */
#define Uses_dirent
#define Uses_fnmatch
#ifdef TVCompf_djgpp
#include <dir.h>
#endif

#define Uses_string
#define Uses_MsgBox
#define Uses_fpstream
#define Uses_TDeskTop
#define Uses_TRect
#define Uses_TApplication
#define Uses_TListBox
#define Uses_TPalette
#define Uses_TScreen
#define Uses_TGKey
#define Uses_TCEditor_Internal
#define Uses_TCEditWindow
#define Uses_FileOpenAid
#define Uses_TDeskTopClock
#define Uses_TStringCollectionW
#define Uses_TFileCollection
#define Uses_TSOSListBox
#define Uses_TVCodePage
#define Uses_TScOptsCol
#define Uses_TVFontCollection
#define Uses_TVConfigFile
#define Uses_TStreamableClass
// InfView requests
#include <infr.h>
#include <ceditor.h>

#define Uses_TSetEditorApp
#define Uses_SETAppProject
#define Uses_SETAppVarious
#define Uses_SETAppFiles
#include <setapp.h>

#include <dskwin.h>
#include <dskclip.h>
#include <dskhelp.h>
#include <edcollec.h>
#include <tpaltext.h>
#include <edprint.h>
#include <ssyntax.h>
#include <pathtool.h>
#include <codepage.h>
#include <mixer.h>
#include <edspecs.h>
#include <pathlist.h>
#define Uses_TSOSListBoxMsg
#include <edmsg.h>
#include <tags.h>

//#define DEBUG_CONFIG_TREE

// Used by edprj.cc to know if we loaded the desktop file from this directory
char DstLoadedHere=0;

extern char *EditorFile;

// Desktop file name
const char *cDeskTopFileName=DeskTopFileName;
#ifdef HIDDEN_DIFFERENT
const char *cDeskTopFileNameHidden=DeskTopFileNameHidden;
#endif
static char *Signature="TEditorApp desktop file\x1A";
const int    EditorsDelta=10;
const char  *defaultDrvName="_Default";

// Default Installation Options variables
typedef struct
{
 const char *option;
 int len,value;
} stOption;

static stOption Options[]=
{
 {"CentralDesktopFile",18,0},
 {"TabsForIndent",13,0},
 {"CreateBackUps",13,1}
};

const int numOptions=sizeof(Options)/sizeof(stOption);
const char *dioFile="install.log";
const int dioMaxLine=PATH_MAX;

/**[txh]********************************************************************

  Description:
  Loads settings configured during installation process. They are just a few
general and important options.
  
***************************************************************************/

static
void LoadInstallationDefaults()
{
 char *name=ExpandHome(dioFile);
 FILE *f=fopen(name,"rt");
 if (!f)
    return;
 char b[dioMaxLine],*s;
 int i;
 do
   {
    if (fgets(b,dioMaxLine,f) && *b!='#')
      {
       for (s=b; *s && ucisspace(*s); s++); // Eat spaces
       for (i=0; i<numOptions; i++)
           if (strncasecmp(s,Options[i].option,Options[i].len)==0)
              break;
       if (i!=numOptions)
         {
          s+=Options[i].len;
          // Move after =
          for (; *s && *s!='='; s++);
          if (*s) s++;
          for (; *s && ucisspace(*s); s++);
          // Get the option
          Options[i].value=*s=='1';
         }
      }
   }
 while (!feof(f));
 fclose(f);

 // CentralDesktopFile
 if (Options[0].value)
    EnvirResetBits("SET_CREATE_DST",dstCreate);
 else
    EnvirSetBits("SET_CREATE_DST",dstCreate);

 // TabsForIndent
 if (Options[1].value)
   {
    TCEditor::staticUseTabs=True;
    TCEditor::staticAutoIndent=True;
    TCEditor::staticIntelIndent=False;
    TCEditor::staticOptimalFill=True;
    TCEditor::staticNoInsideTabs=True;
    TCEditor::staticTabIndents=False;
    TCEditor::staticUseIndentSize=False;
    TCEditor::staticBackSpUnindents=False;
   }
 else
   {
    TCEditor::staticUseTabs=False;
    TCEditor::staticAutoIndent=True;
    TCEditor::staticIntelIndent=False;
    TCEditor::staticOptimalFill=False;
    TCEditor::staticNoInsideTabs=True;
    TCEditor::staticTabIndents=True;
    TCEditor::staticUseIndentSize=False;
    TCEditor::staticBackSpUnindents=True;
   }

 // CreateBackUps
 if (Options[2].value)
    TCEditor::editorFlags|=efBackupFiles;
 else
    TCEditor::editorFlags&=~efBackupFiles;
}

/**[txh]********************************************************************

  Description:
  Helper function that looks for projects in the current directory. The name
of the last found is stored in prjName.
  
  Return: The number of projects found.
  
***************************************************************************/

static
int LookForProjects(char *prjName)
{
 DIR *d;
 d=opendir(".");

 if (d)
   {
    struct dirent *de;
    int c=0;
    while ((de=readdir(d))!=0)
      {
       if (fnmatch("*" ProjectFileExt,de->d_name,0))
          continue;
       if (c==0)
         {
          strcpy(prjName,de->d_name);
          c++;
         }
       else
         {
          c++;
          break;
         }
      }
    closedir(d);
    return c;
   }
 return 0;
}

/**[txh]********************************************************************

  Description:
  Looks for a desktop file and loads it. The @var{LoadPrj} argument says if
we are also looking for a project (0 when loaded the desktop associated to
a desktop), the suggestedName is tested before trying to load. The
@var{haveFilesCL} argument indicates if we have files specified listed in
the command line. The @var{preLoad} argument indicates if we are pre-loading
a desktop file to determine default screen options.

***************************************************************************/

void TSetEditorApp::loadEditorDesktop(int LoadPrj, char *suggestedName,
                                      int haveFilesCL, int preLoad)
{
 TEditorCollection::HaveFilesCL=haveFilesCL;
 DstLoadedHere=0;

 // 0) If the user forces a project load it or if that's impossible create it
 if (suggestedName && CLY_ValidFileName(suggestedName))
   {
    OpenProject(suggestedName,preLoad);
    return;
   }
 // 1) Look for project files.
 if (LoadPrj)
   {
    char prjName[PATH_MAX];
    // If there are only one project
    if (LookForProjects(prjName)==1)
      {// Look again for the desktop
       OpenProject(prjName,preLoad);
       return;
      }
   }
 // 2) Try with the desktop file here
 if (edTestForFile(cDeskTopFileName))
   {
    if (retrieveDesktop(editorApp,cDeskTopFileName,True,preLoad))
      {
       DstLoadedHere=1;
       return;
      }
   }
 #ifdef HIDDEN_DIFFERENT
 // 2.2) Same for hidden version
 if (edTestForFile(cDeskTopFileNameHidden))
   {
    if (retrieveDesktop(editorApp,cDeskTopFileNameHidden,True,preLoad))
      {
       DstLoadedHere=1;
       return;
      }
   }
 #endif
 // 3) Try with the default desktop file
 char *s=ExpandHome(cDeskTopFileName);
 if (edTestForFile(s))
   {
    if (retrieveDesktop(editorApp,(const char *)s,False,preLoad))
       return;
   }
 #ifdef HIDDEN_DIFFERENT
 // 3.2) Same for hidden
 s=ExpandHome(cDeskTopFileNameHidden);
 if (edTestForFile(s))
   {
    if (retrieveDesktop(editorApp,(const char *)s,False,preLoad))
       return;
   }
 #endif
 retrieveDesktop(editorApp,NULL,False,preLoad);
 LoadInstallationDefaults();
}

/**[txh]********************************************************************

  Description:
  This call back is used by TV to indicate which driver was detected. The
driver isn't initialized yet, so we can set default options for the driver
here.
  
***************************************************************************/

void TSetEditorApp::displayDetectCallBack()
{
 const char *drv=TScreen::getDriverShortName();
 ccIndex pos;
 int adjustVideoMode=0;

 // Make "so" member point to a valid set of options
 if (soCol->search((void *)drv,pos))
   {
    // Ok, we had the options for this driver
    so=(stScreenOptions *)soCol->at(pos);
    // The following is needed only if the desktop file was damaged by v0.5.0 to 0.5.2
    if (!TScreen::codePageVariable())
      {// This driver have a fixed screen code page, we must use it.
       so->enForceScr=0;
      }
   }
 else
   {// We don't know about it ...
    if (soCol->search((void *)defaultDrvName,pos))
      {// but we have defaults, associate them with this driver
       so=(stScreenOptions *)soCol->at(pos);
       DeleteArray(so->driverName);
       so->driverName=newStr(drv);
       if (!TScreen::codePageVariable())
         {// This driver have a fixed screen code page, we must use it.
          so->enForceScr=0;
         }
       adjustVideoMode=1;
      }
    else
      {// do we have at least one set of options?
       if (soCol->getCount()==0)
         {// Nope, so just create a default group of settings.
          so=new stScreenOptions;
          memset(so,0,sizeof(stScreenOptions));
          so->driverName=newStr(drv);
          so->enApp=so->enScr=so->enSnd=so->enInp=-1;
          so->foPriW=8; so->foPriH=16;
          so->scCharWidth=8; so->scCharHeight=16;
          so->scModeNumber=3;
          so->scWidth=80; so->scHeight=25;
          soCol->Insert(so);
         }
       else
         {// yes, use the first available
          stScreenOptions *reference=(stScreenOptions *)soCol->at(0);
          so=new stScreenOptions;
          memcpy(so,reference,sizeof(stScreenOptions));
          adjustVideoMode=strcmp(so->driverName,"DOS")==0 && strcmp(drv,"DOS")!=0;
          so->driverName=newStr(drv);
          #define CP(a) if (reference->a) so->a=newStr(reference->a);
          CP(foPriName);
          CP(foSecName);
          CP(scCommand);
          #undef CP
          if (!TScreen::codePageVariable())
            {// This driver have a fixed screen code page, we must use it.
             so->enForceScr=0;
            }
          transferSetting2TV(so);
          soCol->Insert(so);
         }
      }
   }

 // Now fill the needed fields
 int setCallBack=0;
 TVBitmapFontDescCol *fonts=NULL;
 // Get the list of available fonts
 if ((so->foPriLoad && so->foPriName) || (so->foSecLoad && so->foSecName))
    fonts=TVFontCollection::CreateListOfFonts(GetVariable("SET_FILES"));
 if (so->foPriLoad && so->foPriName)
   {// We want a primary font
    // Search the file name
    if (fonts->search(so->foPriName,pos))
      {// Ok!
       so->foPriFile=newStr(((TVBitmapFontDesc *)fonts->at(pos))->file);
       // Create a collection of fonts using the encoding we used last run
       so->foPri=new TVFontCollection(so->foPriFile,so->enScr);
       if (so->foPri->GetError())
         {// We failed to load the file :-(
          delete so->foPri;
          so->foPri=NULL;
         }
       else
          // Got the fonts, hook the font requester
          setCallBack=1;
      }
   }
 // Same for secondary font
 if (so->foSecLoad && so->foSecName)
   {
    if (fonts->search(so->foSecName,pos))
      {
       so->foSecFile=newStr(((TVBitmapFontDesc *)fonts->at(pos))->file);
       so->foSec=new TVFontCollection(so->foSecFile,so->enSnd);
       if (so->foSec->GetError())
         {
          delete so->foSec;
          so->foSec=NULL;
         }
       else
          setCallBack=1;
      }
   }
 if (setCallBack && !so->foCallBackSet)
   {
    TScreen::setFontRequestCallBack(FontRequestCallBack);
    so->foCallBackSet=1;
   }
 CLY_destroy(fonts);

 if (adjustVideoMode && so->scOptions==scfMode)
   {// These settings come from DOS. Is much better to indicate resolutions.
    unsigned w, h;
    int fW, fH;
    if (TDisplay::searchDOSModeInfo(so->scModeNumber,w,h,fW,fH))
      {
       so->scOptions=scfForced;
       so->scWidth=w; so->scHeight=h;
       so->scCharWidth=fW; so->scCharHeight=fH;
       TVMainConfigFile::Add(drv,"ScreenWidth",w);
       TVMainConfigFile::Add(drv,"ScreenHeight",h);
       TVMainConfigFile::Add(drv,"FontWidth",fW);
       TVMainConfigFile::Add(drv,"FontHeight",fH);
      }
   }
 #ifdef DEBUG_CONFIG_TREE
 fprintf(stderr,"TV config:\n");
 TVMainConfigFile::Print(stderr);
 #endif
}

/**[txh]********************************************************************

  Description:
  Called when we loaded a desktop v0.5.0+ without preload. That's when we
close a project or open a new one.
  
***************************************************************************/

void TSetEditorApp::hotApplyScreenOptions()
{// The following ensures so points to a valid set of options.
 displayDetectCallBack();
 // Ensure a simple state:
 TScreen::restoreFonts();
 // Code pages:
 if (so->enForceApp || so->enForceScr || so->enForceInp)
    TVCodePage::SetCodePage(so->enForceApp ? so->enApp : -1,
                            so->enForceScr ? so->enScr : -1,
                            so->enForceInp ? so->enInp : -1);
 // Video mode:
 fontCreated=0;
 if (!so->foCallBackSet)
   {
    TScreen::setFontRequestCallBack(FontRequestCallBack);
    so->foCallBackSet=1;
   }
 PreparePalForRestore(so->palette);
 if (so->scOptions!=scfDontForce && TScreen::canSetVideoSize())
    resetVideoMode();
 else
    RestorePaletteSystem();
 // Fonts: fontCreated, should be loaded during the
 if (!fontCreated && (so->foPriLoad || so->foSecLoad))
   {
    TScreenFont256 primary,secondary,*prF=NULL,*seF=NULL;
    if (so->foPriLoad && so->foPri)
      {
       primary.w=so->foPriW;
       primary.h=so->foPriH;
       primary.data=so->foPri->GetFont(so->foPriW,so->foPriH);
       if (primary.data)
          prF=&primary;
      }
    if (so->foSecLoad && so->foSec)
      {
       secondary.w=so->foPriW;
       secondary.h=so->foPriH;
       secondary.data=so->foSec->GetFont(so->foPriW,so->foPriH);
       if (secondary.data)
          seF=&secondary;
      }
    TScreen::setFont(prF ? 1 : 0,prF,seF ? 1 : 0,seF,so->enScr);
    if (prF) DeleteArray(prF->data);
    if (seF) DeleteArray(seF->data);
   }
}

void TSetEditorApp::preLoadDesktop(char *name, int haveFilesCL)
{
 if (!soCol)
   {
    soCol=new TScOptsCol();
    TDisplay::setDetectCallBack(displayDetectCallBack);
   }
 so=NULL;
 loadEditorDesktop(1,name,haveFilesCL,1);
}

void TSetEditorApp::finishPreLoadDesktop()
{// We are initialized, now ask the palette system to read the actual palette
 CacheActualPalette();
}

/**[txh]********************************************************************

  Description:
  Restores the previously stored Desktop. The @var{preLoad} argument
indicates if we just want to load the information needed before starting the
application (screen options).

***************************************************************************/

Boolean TSetEditorApp::retrieveDesktop(TSetEditorApp *app, const char *name,
                                       Boolean isLocal, int preLoad)
{
 Boolean ret=False;
 if (name)
   {
    #ifdef BROKEN_CPP_OPEN_STREAM
    // In this way we avoid the destruction of the file
    int h=open(name,O_RDONLY | O_BINARY);
    fpstream *f=new fpstream(h);
    #else
    fpstream *f=new fpstream(name,CLY_IOSIn | CLY_IOSBin);
    #endif

    if (!f)
      {
       if (!preLoad)
          messageBox(__("Could not open desktop file"), mfOKButton | mfError);
      }
    else
      {
       if (preLoad)
          ret=preLoadDesktop(*f);
       else
          ret=app->loadDesktop(*f,isLocal);
       if (!f)
         {
          if (!preLoad)
             messageBox(__("Error reading desktop file"), mfOKButton | mfError);
          ret=False;
         }
       f->close();
      }
    delete f;
   }
 if (preLoad || !app)
    return ret;

 // Create all the necesary things if there is no desktop file
 if (!edHelper)
    edHelper=new TEditorCollection(EditorsDelta*2,EditorsDelta);

 if (!clipWindow)
   {
    app->createClipBoard();
    if (clipWindow)
       edHelper->addNonEditor(new TDskWinClipboard(clipWindow));
   }

 if (!InfManager)
   {
    InfManager=new TDskWinHelp(EditorFile,"",True);
    if (InfManager && InfManager->window)
      {
       deskTop->insert(InfManager->window);
       edHelper->addNonEditor(InfManager);
      }
   }
 return ret;
}

/**[txh]********************************************************************

  Description:
  Saves the DeskTop by calling storeDesktop function

***************************************************************************/

void TSetEditorApp::saveDesktop(const char *fName, int makeBkp)
{
 if (makeBkp && edTestForFile(fName))
   {// Try to keep the original file
    char backupName[PATH_MAX];
    strcpy(backupName,fName);
    ReplaceExtension(backupName,".bkp");
    rename(fName,backupName);
   }
 #ifdef BROKEN_CPP_OPEN_STREAM
 int h=open(fName,O_WRONLY | O_BINARY | O_CREAT | O_TRUNC,S_IWUSR | S_IRUSR);
 if (h<0) return;
 fpstream *f=new fpstream(h);

 dbprintf("Opened %s file, got %d handle\n",fName,h);
 #else
 fpstream *f=new fpstream(fName,CLY_IOSOut | CLY_IOSBin);
 #endif

 if (f)
   {
    TSetEditorApp::storeDesktop(*f);
    if (!f)
      {
       messageBox(mfOKButton | mfError,__("Could not create %s."),fName);
       f->close();
       ::remove(fName);
      }
    else
       f->close();
   }
 else
   {
    dbprintf("Fail! the fpstream object is unusable\n");
   }
 delete f;
}

/**[txh]********************************************************************

  Description:
  Saves the palette information to disk, you must specify which of the three
palettes must be saved.

***************************************************************************/

static
void SavePalette(int what, fpstream& s)
{
 int oldMode=TProgram::appPalette;
 // Select the mode to save
 TProgram::appPalette=what;
 // Save the palette
 TPalette &palette=TProgram::application->getPalette();
 int len=palette.data[0];
 s << len;
 s.writeBytes(&palette.data[1],len);
 // Restore the mode
 TProgram::appPalette=oldMode;
}

/**[txh]********************************************************************

  Description:
  Stores the Desktop in a resource file.

***************************************************************************/

void TSetEditorApp::storeDesktop(fpstream& s)
{
 int i,j,c;
 // Save a signature to identify the file
 s.writeString(Signature);
 // Save the version
 s << TCEDITOR_VERSION;

 // Save the video mode & font, first to avoid a lot of redraw
 if (soCol)
   {
    if (so)
      {// Refresh the information in the structure
       if (so->scOptions==scfForced && !TScreen::canSetVideoSize())
          so->scOptions=scfSameLast; // can't be forced ;-)
       if (so->scOptions!=scfForced)
         {
          so->scWidth =TScreen::getCols();
          so->scHeight=TScreen::getRows();
          unsigned w,h;
          if (TScreen::getFontGeometry(w,h))
            {
             so->scCharWidth=w;
             so->scCharHeight=h;
            }
         }
       ComputeChangedStatus();
       so->palChanged=PaletteWasChanged();
       if (so->palChanged)
          PaletteCopy(so->palette);
      }
    s << (char)1 << soCol;
   }
 else
    s << (char)0;
 // Now the 3 palettes for same reason
 SavePalette(apColor,s);
 SavePalette(apMonochrome,s);
 SavePalette(apBlackWhite,s);

 s << TCEditor::staticTabSize
   << TCEditor::staticIndentSize
   << uchar(TCEditor::staticAutoIndent)
   << uchar(TCEditor::staticIntelIndent)
   << uchar(TCEditor::staticUseTabs)
   << uchar(TCEditor::staticPersistentBlocks)
   << uchar(TCEditor::staticCrossCursorInRow)
   << uchar(TCEditor::staticCrossCursorInCol)
   << uchar(TCEditor::staticShowMatchPair)
   << uchar(TCEditor::staticShowMatchPairFly)
   << uchar(TCEditor::staticTransparentSel)
   << uchar(TCEditor::staticOptimalFill)
   << uchar(TCEditor::staticNoMoveToEndPaste)
   << uchar(TCEditor::staticSeeTabs)
   << uchar(TCEditor::staticNoInsideTabs)
   << uchar(TCEditor::staticWrapLine)
   << uchar(TCEditor::staticTabIndents)
   << uchar(TCEditor::staticBackSpUnindents)
   << uchar(TCEditor::staticShowMatchPairNow)
   << uchar(TCEditor::staticUseIndentSize)
   << uchar(TCEditor::staticDontPurgeSpaces)
   << uchar(TCEditor::staticColumnMarkers);
 TCEditor::SaveColMarkers(s,TCEditor::staticColMarkers);
 s << TCEditor::staticWrapCol
   << TCEditor::editorFlags;

 // Save the histories
 s << hID_Cant;
 for (j=hID_Start; j<hID_Start+hID_Cant; j++)
    {
     c=historyCount(j);
     s << j << c;
     for (i=c; i; --i)
         s.writeString(historyStr(j,i-1));
    }
 SaveFileIDDirs(s);

 // That saves the windowing system
 s << edHelper;

 SavePrintSetUp(&s);
 SaveRunCommand(s);
 SaveGrepData(s);
 s << TDeskTopClock::mode;
 s << ShowClock;
 s << TSetEditorApp::UseScreenSaver << TSetEditorApp::screenSaverTime
   << TSetEditorApp::screenSaverTimeMouse;
 s.writeString(TSetEditorApp::WhichScrSaver);
 s.writeString(TSetEditorApp::ExtScrSaverOpts);

 // The search & replace options
 s.writeString(TCEditor::findStr);
 s << TCEditor::editorFlags << TCEditor::SearchInSel << TCEditor::FromWhere;
 s << TCEditor::RegExStyle << TCEditor::ReplaceStyle << TCEditor::CanOptimizeRegEx;

 SyntaxSearch_Save(s);

 s << maxOpenEditorsSame << geFlags << widthVertWindows;

 // If the user wants remember the backup files we created to kill'em latter
 TStringCollectionW *fKill=GetFilesToKill();
 if (DeleteFilesOnExit || (GetDSTOptions() & dstRemmeberFK)==0)
    fKill=0;
 if (fKill)
    s << uchar(1) << fKill;
 else
    s << uchar(0);

 // Desktop options
 s << deskTop->getOptions();
 // Directory listing options
 s << TFileCollection::sortOptions;
 // Code page conversion options
 SaveConvCPOptions(s);
 s << TGKey::GetKbdMapping(0);
 BoardMixerSave(s);
 PathListSave(s);
 s << (uchar)TSOSListBoxMsg::opsEnd << (uchar)TSOSListBoxMsg::opsBeep;
 TagsSave(s);
 s << modifFilesOps << TCEditor::minDifModCheck;
 s << 0;
}

/**[txh]********************************************************************

  Description:
  Loads the palette information from disk, you must specify which of the
three palettes must be saved.

***************************************************************************/

static
void LoadPalette(int what, fpstream& s, int v)
{
 int oldMode=TProgram::appPalette;
 TProgram::appPalette=what;

 TPalette &palette=TProgram::application->getPalette();

 int lenNew=palette.data[0];
 int lenOld;
 s >> lenOld;
 // I assume the palette can only grow ;-)
 char *palOld=new char[lenNew+1];
 s.readBytes(palOld,lenOld);
 if (lenOld<lenNew)
   {
    memcpy(palOld+lenOld,&(palette.data[lenOld+1]),lenNew-lenOld);
    lenOld=lenNew;
   }

 // Patch for the palette according to the version
 // Tabs
 if (v<0x423 && palOld[0x79]==0 && palOld[0x7A]==0)
   {
    if (what==apMonochrome)
      {
       palOld[0x79]=0x70;
       palOld[0x7A]=0x01;
      }
    else
      {
       palOld[0x79]=0x20;
       palOld[0x7A]=0x40;
      }
   }
 if (v<0x446 && palOld[0x3D]==0)
   {
    switch (what)
      {
       case apMonochrome:
       case apBlackWhite:
            palOld[0x3D]=0x07;
            break;
       default:
            palOld[0x3D]=0x38;
      }
   }
 if (v<0x450 && palOld[0x7B]==0)
   {
    switch (what)
      {
       case apMonochrome:
       case apBlackWhite:
            palOld[0x7B]=0x70;
            break;
       default:
            palOld[0x7B]=0x20;
      }
   }

 TPalette paletteNew(palOld,lenOld);
 TProgram::application->getPalette()=paletteNew;
 delete[] palOld;

 // Restore the mode
 TProgram::appPalette=oldMode;
}

#if 0
 #define L(a)  s >> aux; TCEditor::##a=(aux) ? True : False
#else
 #define L(a)  s >> aux; TCEditor::a=(aux) ? True : False
#endif

inline unsigned MoveFlags(unsigned flags, unsigned mask, unsigned move)
{
 return (flags & mask) | ((flags & ~mask)<<move);
}

/**[txh]********************************************************************

  Description:
  It loads the information from the desktop file.

***************************************************************************/

Boolean TSetEditorApp::loadDesktop(fpstream &s, Boolean isLocal)
{
 char buffer[80];
 unsigned auxUN;
 int auxINT;

 s.readString(buffer,80);
 if (strcmp(buffer,Signature)!=0)
   {
    messageBox(__("Wrong desktop file."), mfOKButton | mfError);
    return False;
   }
 s >> deskTopVersion;
 if (deskTopVersion<0x300)
   {
    messageBox(__("The desktop file is too old."), mfOKButton | mfError);
    return False;
   }
 if (deskTopVersion>TCEDITOR_VERSION)
   {
    messageBox(__("You need a newer editor for this desktop file."), mfOKButton | mfError);
    return False;
   }

 if (!DesktopPreloaded)
   {// That's a desktop change, not the first load. So we must do a "hot swap" ;-)
    if (deskTopVersion>=0x500)
      {// New projects structure
       char infoSaved;
       s >> infoSaved;
       if (infoSaved)
         {// We have settings, load them replacing current settings
          CLY_destroy(soCol);
          s >> soCol;
          hotApplyScreenOptions();
         }
      }
    else
      {// Load the old information and setup it
       stScreenOptions *scrOps=loadOldDesktopScreenInfo(s);
       scrOps->driverName=newStr(TScreen::getDriverShortName());
       CLY_destroy(soCol);
       soCol=new TScOptsCol();
       soCol->Insert(scrOps);
       hotApplyScreenOptions();
      }
   }
 else
   {// We already loaded the screen settings
    if (deskTopVersion>=0x500)
      {
       char infoSaved;
       s >> infoSaved;
       if (infoSaved)
         {
          TScOptsCol *aux;
          s >> aux;
          CLY_destroy(aux);
          // TODO: this is really inefficient, but lamentably TV saves the
          // information using a crappy indexing stuff. If we skip it the
          // indexes gets wrong.
         }
      }
    else
       s.seekg(posPreload);
    DesktopPreloaded=0;
   }

 if (deskTopVersion>=0x307)
   {
    // Load the 3 palettes
    LoadPalette(apColor,s,deskTopVersion);
    LoadPalette(apMonochrome,s,deskTopVersion);
    LoadPalette(apBlackWhite,s,deskTopVersion);
    TCEditor::colorsCached=0;
    TProgram::application->Redraw();
   }

 s >> TCEditor::staticTabSize;
 if (deskTopVersion>0x450)
    s >> TCEditor::staticIndentSize;
 uchar aux;
 L(staticAutoIndent);
 L(staticIntelIndent);
 L(staticUseTabs);
 L(staticPersistentBlocks);
 if (deskTopVersion>0x308)
   {
    L(staticCrossCursorInRow);
    L(staticCrossCursorInCol);
    L(staticShowMatchPair);
    if (deskTopVersion>0x431)
      { L(staticShowMatchPairFly); }
    L(staticTransparentSel);
    L(staticOptimalFill);
    L(staticNoMoveToEndPaste);
   }
 if (deskTopVersion>0x427)
   {
    L(staticSeeTabs);
    L(staticNoInsideTabs);
    L(staticWrapLine);
   }
 if (deskTopVersion>=0x440)
   {
    L(staticTabIndents);
   }
 if (deskTopVersion>=0x445)
   {
    L(staticBackSpUnindents);
    L(staticShowMatchPairNow);
   }
 else
   { // Old desktop files with "Use Tabs" enabled globally have to disable it
    if (TCEditor::staticUseTabs)
       TCEditor::staticBackSpUnindents=False;
   }
 if (deskTopVersion>=0x448)
   {
    L(staticUseIndentSize);
    L(staticDontPurgeSpaces);
   }
 if (deskTopVersion>=0x450)
   {
    L(staticColumnMarkers);
    delete[] TCEditor::staticColMarkers;
    TCEditor::staticColMarkers=TCEditor::LoadColMarkers(s);
   }

 if (deskTopVersion>0x401)
    s >> TCEditor::staticWrapCol;
 if (deskTopVersion>=0x406)
    s >> TCEditor::editorFlags;

 // Load histories
 if (deskTopVersion<0x308)
   { // -> v0.3.7 only the number 10
    int i,j;
    char *sp;
    s >> j;
    for (i=0; i<j; i++)
       {
        s.readString(buffer,80);
        sp = new char[strlen(buffer)+1];
        strcpy(sp,buffer);
        historyAdd(hID_TextSearchEditor,sp);
       }
   }
 else
   {
    int numHists,hist,numInHis,thisHist,strHis;
    char *str;

    s >> numHists;
    for (hist=0; hist<numHists; hist++)
       {
        s >> thisHist >> numInHis;
        for (strHis=0; strHis<numInHis; strHis++)
           {
            str=s.readString();
            historyAdd(thisHist,str);
            delete[] str;
           }
       }
   }
 if (deskTopVersion>=0x415)
    LoadFileIDDirs(s,isLocal);

 if (deskTop->valid(cmClose))
    deskTop->forEach(::closeView, 0);  // Clear the desktop

 s >> edHelper;

 if (deskTopVersion>=0x310)
    LoadPrintSetUp(&s);

 if (deskTopVersion>=0x407)
    LoadRunCommand(s);

 if (deskTopVersion>=0x408)
    LoadGrepData(s);

 if (deskTopVersion>=0x409)
    s >> TDeskTopClock::mode >> ShowClock;

 if (deskTopVersion>=0x410)
   {
    s >> TSetEditorApp::UseScreenSaver >> TSetEditorApp::screenSaverTime;
    if (deskTopVersion>=0x430)
       s >> TSetEditorApp::screenSaverTimeMouse;
    delete[] TSetEditorApp::WhichScrSaver;
    TSetEditorApp::WhichScrSaver=s.readString();
    if (deskTopVersion>=0x434)
       s.readString(TSetEditorApp::ExtScrSaverOpts,extscrsParMxLen);
   }

 // The search & replace options
 if (deskTopVersion>=0x415)
   {
    s.readString(TCEditor::findStr,maxFindStrLen);
    s >> TCEditor::editorFlags >> TCEditor::SearchInSel >> TCEditor::FromWhere;
   }
 if (deskTopVersion<0x431)
    // I moved some options in 0.4.31 to left space for new ones
    TCEditor::editorFlags=MoveFlags(TCEditor::editorFlags,7,2);
 if (deskTopVersion<0x442)
    TCEditor::editorFlags=MoveFlags(TCEditor::editorFlags,0x1F,1);
 if (deskTopVersion>=0x416)
    s >> TCEditor::RegExStyle >> TCEditor::ReplaceStyle >> TCEditor::CanOptimizeRegEx;
 // Syntax Help
 if (deskTopVersion>=0x418)
    SyntaxSearch_Load(s);
 else
    SyntaxSearch_InitWithDefaults();

 if (deskTopVersion>=0x420)
    s >> maxOpenEditorsSame;
 if (deskTopVersion>=0x457)
    s >> geFlags >> widthVertWindows;

 aux=0;
 if (deskTopVersion>=0x429)
   {
    s >> aux;
    if (aux)
      {
       TStringCollectionW *FilesToKill;
       s >> FilesToKill;
       SetFilesToKill(FilesToKill);
      }
   }
 if (!aux)
    SetFilesToKill(0);

 if (deskTopVersion>=0x433)
   {
    s >> auxUN;
    deskTop->setOptions(auxUN);
    s >> TFileCollection::sortOptions;
   }
 if (deskTopVersion>=0x437)
    LoadConvCPOptions(s);
 if (deskTopVersion>=0x438)
   {// Currently useless
    s >> auxINT;
    //TGKey::SetKbdMapping(auxINT ? TGKey::dosGreek737 : TGKey::dosUS);
   }
 if (deskTopVersion>=0x444)
    BoardMixerLoad(s);
 if (deskTopVersion>=0x449)
    PathListLoad(s);
 if (deskTopVersion>=0x456)
   {
    s >> aux; TSOSListBoxMsg::opsEnd=aux;
    s >> aux; TSOSListBoxMsg::opsBeep=aux;
   }
 if (deskTopVersion>=0x501)
    TagsLoad(s);
 if (deskTopVersion>=0x502)
   {
    s >> modifFilesOps >> TCEditor::minDifModCheck;
   }

 // Even when 0.4.15 doesn't use the Config Files path we ensure it's pointing to
 // the SET_FILES path
 char *path=(GetFileIDDirBuffer(hID_ConfigFiles))->dir;

 if (path && path[0]==0)
   { // If the directory isn't configured
    char *r=(char *)GetVariable("SET_FILES");
    if (r)
       SetFileIDDirValue(hID_ConfigFiles,r);
   }
 return True;
}
#undef L

/**[txh]********************************************************************

  Description:
  Loads settings for fonts from desktops older than 0.5.0.
  
***************************************************************************/
void TSetEditorApp::loadOldFontInfo(fpstream& s, stScreenOptions *scrOps)
{
 char version,aux;
 s >> version;
 int cp1,cp2;

 if (version && version<=3)
   {// versions 1, 2 and 3
    char *namePrim=s.readString();
    if (version>=3)
      {
       s >> cp1;
       scrOps->enForceScr=1;
       scrOps->enScr=cp1;
      }
    char *nameSeco=0;

    if (version>=2)
      {
       s >> aux;
       if (aux)
         {
          nameSeco=s.readString();
          if (version>=3)
            {
             s >> cp2;
             scrOps->enForceSnd=1;
             scrOps->enSnd=cp2;
            }
         }
      }
    if (namePrim && namePrim[0])
      {
       scrOps->foPriName=namePrim;
       scrOps->foPriLoad=1;
      }
    if (nameSeco && nameSeco[0])
      {
       scrOps->foSecName=nameSeco;
       scrOps->foSecLoad=1;
      }
   }
 else
 if (version>=4)
   {// version 4
    char *namePrim=NULL;
    char *nameSeco=NULL;
    s >> cp1 >> cp2;
    s >> aux;
    if (aux)
      {
       namePrim=s.readString();
       s >> aux;
       if (aux)
          nameSeco=s.readString();
      }
    scrOps->enForceScr=1;
    scrOps->enScr=cp1;
    scrOps->enForceSnd=1;
    scrOps->enSnd=cp2;
    if (namePrim)
      {
       scrOps->foPriName=namePrim;
       scrOps->foPriLoad=1;
      }
    if (nameSeco)
      {
       scrOps->foSecName=nameSeco;
       scrOps->foSecLoad=1;
      }
   }
}

void TSetEditorApp::transferSetting2TV(void *aP, void *)
{
 stScreenOptions *p=(stScreenOptions *)aP;
 // Driver section
 const char *drv=p->driverName;
 // The _Default goes to the root
 if (*drv=='_')
    drv=NULL;
 TVMainConfigFile::Add(drv,"AppCP",p->enForceApp ? p->enApp : -1);
 TVMainConfigFile::Add(drv,"ScrCP",p->enForceScr ? p->enScr : -1);
 TVMainConfigFile::Add(drv,"InpCP",p->enForceInp ? p->enInp : -1);

 if (p->foPriLoad || p->foSecLoad)
   {
    TVMainConfigFile::Add(drv,"FontWidth",p->foPriW);
    TVMainConfigFile::Add(drv,"FontHeight",p->foPriH);
    if (p->foSecLoad)
       TVMainConfigFile::Add(drv,"LoadSecondaryFont",1);
   }

 if (p->scOptions!=scfDontForce && p->scWidth)
   {
    TVMainConfigFile::Add(drv,"ScreenWidth",p->scWidth);
    TVMainConfigFile::Add(drv,"ScreenHeight",p->scHeight);
    if (!p->foPriLoad && !p->foSecLoad)
      {
       TVMainConfigFile::Add(drv,"FontWidth",p->scCharWidth);
       TVMainConfigFile::Add(drv,"FontHeight",p->scCharHeight);
      }
   }
 if (p->scOptions==scfExternal)
   {
    TVMainConfigFile::Add(drv,"ExtProgVideoMode",p->scCommand);
   }
 else if (p->scOptions==scfMode)
   {
    TVMainConfigFile::Add(drv,"VideoMode",p->scModeNumber);
   }

 if (p->palChanged)
   {// palChanged isn't enough
    const TScreenColor *pal=TDisplay::getDefaultPalette();
    unsigned i;
    for (i=0; i<16; i++)
        if (p->palette[i].R!=pal[i].R || p->palette[i].G!=pal[i].G ||
            p->palette[i].B!=pal[i].B)
          {
           char b[192],*s=b;
           for (i=0; i<16; i++)
              {
               s+=sprintf(s,"%d,%d,%d",p->palette[i].R,p->palette[i].G,p->palette[i].B);
               if (i!=15)
                  *(s++)=',';
              }
           TVMainConfigFile::Add(drv,"ScreenPalette",b);
           break;
          }
   }
}

stScreenOptions *TSetEditorApp::loadOldDesktopScreenInfo(fpstream &s)
{
 stScreenOptions *scrOps=new stScreenOptions;
 memset(scrOps,0,sizeof(stScreenOptions));
 if (deskTopVersion>=0x404)
    loadOldFontInfo(s,scrOps);
 #ifdef TVCompf_djgpp
 if (deskTopVersion>=0x405)
    LoadPaletteSystemDontSet(s,scrOps->palette);
 #else
 /* In v0.4.15 to v0.4.17 of the Linux editor I forgot to save it so here
    I choose compatibility with these versions */
 if (deskTopVersion>=0x418)
    LoadPaletteSystemDontSet(s,scrOps->palette);
 #endif
 if (deskTopVersion>=0x403)
   {
    ushort mode;
    s >> mode;
    scrOps->scOptions=scfMode;
    scrOps->scModeNumber=mode;
    if (deskTopVersion>=0x411)
      {
       s >> UseExternPrgForMode;
       char *extPrg=s.readString();
       if (UseExternPrgForMode)
         {
          scrOps->scOptions=scfExternal;
          scrOps->scCommand=extPrg;
         }
       else
          DeleteArray(extPrg);
      }
   }
 return scrOps;
}

Boolean TSetEditorApp::preLoadDesktop(fpstream &s)
{
 char buffer[80];

 s.readString(buffer,80);
 if (strcmp(buffer,Signature)!=0)
   { // Wrong desktop file.
    return False;
   }
 s >> deskTopVersion;
 if (deskTopVersion<0x300 || deskTopVersion>TCEDITOR_VERSION)
   { // The desktop file is too old or new
    return False;
   }
 if (deskTopVersion<0x500)
   {// The preLoad was introduced in the 0.4.x to 0.5.x change.
    // We have to extract the information from the old structure.
    stScreenOptions *scrOps=loadOldDesktopScreenInfo(s);
    scrOps->driverName=newStr(defaultDrvName);
    if (!soCol)
       soCol=new TScOptsCol();
    soCol->Insert(scrOps);
    transferSetting2TV(scrOps);
   }
 else
   {
    char soColSaved;
    s >> soColSaved;
    if (soColSaved)
      {
       CLY_destroy(soCol);
       s >> soCol;
       soCol->transfer2TV();
      }
    if (!soCol)
       soCol=new TScOptsCol();
   }
 DesktopPreloaded=1;
 posPreload=s.tellg();
 return True;
}
/******************** End of save/retrieve desktop functions ****************/

void TSetEditorApp::createClipBoard(void)
{
 clipWindow=openEditor(0,False);
 if (clipWindow)
   {
    TCEditor::clipboard=clipWindow->editor;
    TCEditor::clipboard->canUndo=False;
   }
}

TScOptsCol *TSetEditorApp::soCol=NULL;

void *TScOptsCol::keyOf(void *item)
{
 return ((stScreenOptions *)item)->driverName;
}

int TScOptsCol::compare(void *key1, void *key2)
{
 return strcmp((char *)key1,(char *)key2);
}

void TScOptsCol::freeItem(void *item)
{
 stScreenOptions *p=(stScreenOptions *)item;
 DeleteArray(p->driverName);
 DeleteArray(p->foPriName);
 DeleteArray(p->foSecName);
 DeleteArray(p->foPriFile);
 DeleteArray(p->foSecFile);
 DeleteArray(p->scCommand);
 CLY_destroy(p->foPri);
 CLY_destroy(p->foSec);
 delete p;
}

const char scOptsVer=2;

void *TScOptsCol::readItem(ipstream &is)
{
 stScreenOptions *p=new stScreenOptions;
 memset(p,0,sizeof(stScreenOptions));

 char version;
 is >> version;
 p->driverName=is.readString();
 p->foPriName =is.readString();
 p->foSecName =is.readString();
 p->scCommand =is.readString();
 is >> p->enForceApp  >> p->enForceScr >> p->enForceSnd >> p->enForceInp
    >> p->enApp       >> p->enScr      >> p->enSnd      >> p->enInp
    >> p->foPriLoad   >> p->foSecLoad
    >> p->foPriW      >> p->foPriH
    >> p->scOptions   >> p->scWidth    >> p->scHeight
    >> p->scCharWidth >> p->scCharHeight
    >> p->scModeNumber;
 if (version>=2)
    is >> p->palChanged;
 else
    p->palChanged=1;
 if (p->palChanged)
   {
    unsigned i;
    for (i=0; i<16; i++)
        is >> p->palette[i].R >> p->palette[i].G >> p->palette[i].B;
   }
 else
    memcpy(p->palette,TDisplay::getDefaultPalette(),sizeof(TScreenColor)*16);
 return p;
}

void TScOptsCol::writeItem(void *obj, opstream &os)
{
 stScreenOptions *p=(stScreenOptions *)obj;
 os << scOptsVer;
 os.writeString(p->driverName);
 os.writeString(p->foPriName);
 os.writeString(p->foSecName);
 os.writeString(p->scCommand);
 os << p->enForceApp  << p->enForceScr << p->enForceSnd << p->enForceInp
    << p->enApp       << p->enScr      << p->enSnd      << p->enInp
    << p->foPriLoad   << p->foSecLoad
    << p->foPriW      << p->foPriH
    << p->scOptions   << p->scWidth    << p->scHeight
    << p->scCharWidth << p->scCharHeight
    << p->scModeNumber<< p->palChanged;
 if (p->palChanged)
   {
    unsigned i;
    for (i=0; i<16; i++)
        os << p->palette[i].R << p->palette[i].G << p->palette[i].B;
   }
}

void TScOptsCol::Insert(stScreenOptions *p)
{
 ccIndex pos=indexOf(p);
 if (pos==ccNotFound)
    insert(p);
 else
    atReplace(pos,p);
}

void TScOptsCol::transfer2TV()
{
 forEach(TSetEditorApp::transferSetting2TV,NULL);
}

const char * const TScOptsCol::name="TScOptsCol";

TStreamableClass RScOptsCol(TScOptsCol::name,TScOptsCol::build,__DELTA(TScOptsCol));

