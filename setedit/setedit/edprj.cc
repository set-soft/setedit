/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#define Uses_stdio
#define Uses_getcwd
#define Uses_string
#define Uses_unistd

#define Uses_TCEditWindow
#define Uses_TStreamable
#define Uses_TStreamableClass
#define Uses_TScrollBar
#define Uses_TRect
#define Uses_TDialog
#define Uses_TWindow
#define Uses_TStringCollection
#define Uses_TSortedListBox
#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_MsgBox
#define Uses_TKeys
#define Uses_TFileDialog
#define Uses_fpstream
#define Uses_TFileList
#define Uses_FileOpenAid
#define Uses_TCEditor_Commands // For the code page update
#define Uses_IOS_BIN
#define Uses_fcntl
#define Uses_filelength
#define Uses_TVCodePage
#define Uses_TScreen
#include <ceditor.h>
#include <editcoma.h>

#define Uses_PrjFunctions
#define Uses_SETAppAll
#include <setapp.h>
#include <dskwin.h>
#include <dskprj.h>
#include <edcollec.h>
#include <sdginter.h>
#include <codepage.h>
#include <pathtool.h>
#include <advice.h>
#include <rhutils.h>
#include <edspecs.h>
#define Uses_TagsOnlyFuncs
#include <tags.h>
#include <debug.h>
#include <pathlist.h>

extern char *ExpandFileNameToThePointWhereTheProgramWasLoaded(const char *s);
static TDskWinPrj *prjWin=NULL;
#define PrjExists() (prjWin!=NULL)
extern void closeView(TView *p, void *p1);

static int LoadingPrjVersion;

class TEditorProjectListBox : public TSortedListBox
{
public:
  TEditorProjectListBox(const TRect& bounds, ushort aNumCols,
                        TScrollBar *aScrollBar);
  virtual void handleEvent(TEvent &);
  virtual void selectItem(ccIndex item);
  virtual void getText(char *dest, ccIndex item, short maxLen);
  int addFile(char *name, Boolean interactive=True);
  void delFile(void);
  void changeSorting(int mode);
  void toggleSorting();
};

class TEditorProjectWindow : public TDialog
{
public:
  TEditorProjectListBox *list;
  TEditorProjectWindow(const TRect &,const char *);
  ~TEditorProjectWindow();
  virtual void close();
  virtual void handleEvent(TEvent& event);
  virtual const char *getTitle(short maxSize);
  static const int Version;
  TScrollBar *scrollbar;
  char *FileName;

protected:
  int sizeBufTitle;
  char *bufTitle;
  stTVIntl *titleCache;
};

typedef struct
{
 char *name;
 char *shortName;
 EditorResume resume;
 uint32 forceTarget;  // Read the header for more info
} PrjItem;

const int TEditorProjectWindow::Version=8;

const int crtInteractive=1, crtUseFullName=2;
const int prjShortName=0, prjName=1;

class TPrjItemColl : public TStringCollection
{
public:
 TPrjItemColl(ccIndex aLimit, ccIndex aDelta);
 ~TPrjItemColl();
 void atInsert(ccIndex pos, char *s, int flags=crtInteractive);
 void freeItem(void *);
 void *keyOf(void *item)
   { return sortMode==prjShortName ? (void *)((PrjItem *)item)->shortName :
            (void *)((PrjItem *)item)->name; };
 const char *keyOf(ccIndex item) { return (const char *)keyOf(at(item)); }
 char *referencePath;
 char *referenceCurDelta;
 Boolean Search(char *file, ccIndex &pos);
 int addFile(char *name, ccIndex &pos, int flags=0, char **test=NULL);
 void analizeReference(const char *filename);
 char *applyPrjPath(const char *name);
 void  changeSorting(int mode);
 void  changeSorting(int mode, ccIndex &pos);
 void  toggleSorting(ccIndex &pos)
   { changeSorting(sortMode==prjShortName ? prjName : prjShortName,pos); };
 int   getSortMode() { return sortMode; };
  PrjItem *At(ccIndex pos) { return (PrjItem *)at(pos); };

private:
 PrjItem *createNewElement(char *name, int flags=0);

 const char *streamableName() const
     { return name; }
 void *readItem( ipstream& is );
 void writeItem( void *p, opstream &os );

protected:
 TPrjItemColl(StreamableInit);
 int  sortMode; // prjShortName or prjName
 void initClass();

public:
 static const char *name;
 static TStreamable *build() {return new TPrjItemColl( streamableInit );};
};

const char *TPrjItemColl::name="TPrjItemColl";

SetDefStreamOperators(TPrjItemColl)

static TPrjItemColl *ProjectList=NULL;

TPrjItemColl::TPrjItemColl(ccIndex aLimit, ccIndex aDelta) :
     TStringCollection(aLimit,aDelta)
{
 initClass();
}

TPrjItemColl::TPrjItemColl(StreamableInit) :
     TStringCollection(streamableInit)
{
 initClass();
}

void TPrjItemColl::initClass()
{
 referencePath=getcwd(0,PATH_MAX);
 referenceCurDelta=NULL;
 if (!referencePath)
    string_dup(referencePath,"");
 sortMode=prjShortName;
}

TPrjItemColl::~TPrjItemColl()
{
 ::free(referencePath);
 ::free(referenceCurDelta);
}

void TPrjItemColl::changeSorting(int mode)
{
 sortMode=mode;
 reSort();
}

void TPrjItemColl::changeSorting(int mode, ccIndex &pos)
{
 if (pos==-1)
   {
    changeSorting(mode);
    return;
   }
 void *p=at(pos);
 changeSorting(mode);
 search(keyOf(p),pos);
}

void TPrjItemColl::analizeReference(const char *filename)
{
 char b1[PATH_MAX], b2[PATH_MAX];
 // Make this path absolute
 strcpy(b1,filename);
 CLY_fexpand(b1);
 // Extract the dir part
 CLY_ExpandPath(b1,b2,NULL);
 char *endB2=b2+strlen(b2)-1;
 if (CLY_IsValidDirSep(*endB2))
    *endB2=0;
 // Compare with the reference
 if (strcmp(b2,referencePath)!=0)
   { // This isn't the same reference
    ::free(referencePath);
    ::free(referenceCurDelta);
    getcwd(b1,PATH_MAX);
    if (strcmp(b1,b2)==0)
      { // The reference is the current directory
        // I think it never happends
       referencePath=strdup(b1);
       referenceCurDelta=NULL;
      }
    else
      { // Different reference
       char *s=NULL;
       referencePath=strdup(b2);
       string_dup(s,b2);
       AbsToRelPath(b1,s,0);
       referenceCurDelta=s;
      }
   }
}

void *TPrjItemColl::readItem( ipstream& is )
{
 char Buffer[PATH_MAX+1];

 is.readString((char *)Buffer,PATH_MAX);
 PrjItem *st;
 if (LoadingPrjVersion<4)
    st=createNewElement(Buffer);
 else
   {
    char isSame;
    is >> isSame;
    st=createNewElement(Buffer,isSame ? crtUseFullName : 0);
   }
 if (LoadingPrjVersion>2)
    TCEditWindow::ReadResume(st->resume,is);
 if (LoadingPrjVersion>5)
    is >> st->forceTarget;
 return st;
}

void TPrjItemColl::writeItem( void *p, opstream &os )
{
 PrjItem *pi=(PrjItem *)p;
 os.writeString(pi->name);
 os << (char)(pi->shortName==pi->name);
 TCEditWindow::SaveResume(pi->resume,os);
 os << pi->forceTarget;
}

TStreamableClass RPrjItemColl( TPrjItemColl::name,
                               TPrjItemColl::build,
                               __DELTA(TPrjItemColl)
                              );

void TPrjItemColl::freeItem(void *p)
{
 PrjItem *s=(PrjItem *)p;
 if (s)
   {
    string_free(s->name);
    delete s;
   }
}

static char *GetShortName(char *name)
{
 char *slash=strrchr(name,'/');
 if (slash)
    return slash+1;
 return name;
}

PrjItem *TPrjItemColl::createNewElement(char *name, int flags)
{
 PrjItem *st=new PrjItem;
 if (st)
   {// Initialize it to avoid saving garbage that could contain anything important
    memset(st,0,sizeof(PrjItem));
    char *s;
    string_dup(s,name);
    // Is name an absolute path? (relative values comes from disk, absolute from user)
    if ((flags & crtInteractive) && CheckIfPathAbsolute(s))
      {
       if (!AbsToRelPath(referencePath,s,0))
         { // Warning, it will generate problems if the project is moved
          GiveAdvice(gadvAbsolutePath);
         }
      }
    st->name=s;
    if (flags & crtUseFullName)
       st->shortName=st->name;
    else
       st->shortName=GetShortName(st->name);
    // Indicate is empty
    // - Side effect of the memset -
    //st->resume.prj_flags=0;
   }
 return st;
}

Boolean TPrjItemColl::Search(char *name, ccIndex &pos)
{
 // Get the short name and relative name of this file
 char *sName=GetShortName(name);
 char *relName;
 Boolean ret=False;
 string_dup(relName,name);
 AbsToRelPath(referencePath,relName,0);
 int oldSortMode=sortMode;

 if (sortMode!=prjShortName)
    // We must be in short mode to make the next search
    changeSorting(prjShortName);
 // Search the short name
 if (search(sName,pos))
   {// We found it, now make sure that's the same file
    PrjItem *st=(PrjItem *)ProjectList->at(pos);
    if (strcmp(relName,st->name)==0 || // Is it?
        search(relName,pos)) // Is the relative there?
       ret=True;
   }
 else
   {// The short name isn't there, but perhaps the relative is
    ret=search(relName,pos);
   }
 string_free(relName);
 if (oldSortMode!=sortMode)
    // Revert the sorting adjusting the position
    changeSorting(oldSortMode,pos);
 return ret;
}

void TPrjItemColl::atInsert(ccIndex pos, char *s, int flags)
{
 PrjItem *st=createNewElement(s,flags);
 if (st)
   {
    st->forceTarget=prjtAllTargets;
    TStringCollection::atInsert(pos,st);
   }
}


TEditorProjectListBox::TEditorProjectListBox(const TRect& bounds, ushort aNumCols,
                                             TScrollBar *aScrollBar) :
    TSortedListBox(bounds,aNumCols,aScrollBar)
{
}

void TEditorProjectListBox::changeSorting(int mode)
{
 TPrjItemColl *p=(TPrjItemColl *)list();
 ccIndex newFocused=focused;
 p->changeSorting(mode,newFocused);
 if (newFocused!=-1)
    focusItem(newFocused);
}

void TEditorProjectListBox::toggleSorting()
{
 TPrjItemColl *p=(TPrjItemColl *)list();
 ccIndex newFocused=focused;
 p->toggleSorting(newFocused);
 if (newFocused!=-1)
    focusItem(newFocused);
}

void TEditorProjectListBox::getText(char *dest,ccIndex item,short maxlen)
{
 TPrjItemColl *p=(TPrjItemColl *)list();
 strncpy(dest,p->keyOf(item),maxlen);
 dest[maxlen]=EOS;
}


char *TPrjItemColl::applyPrjPath(const char *name)
{
 char *dest=NULL;
 string_cat(dest,referenceCurDelta,DIRSEPARATOR_,name,0);
 return dest;
}

extern void OpenFileFromEditor(char *fullName);

void TEditorProjectListBox::selectItem(ccIndex item)
{
 PrjItem *st=(PrjItem *)(list()->at(item));

 message( owner, evBroadcast, cmListItemSelected, list() );
 if (ProjectList->referenceCurDelta)
   {// The project was loaded from another directory, not curdir
    char *dest=ProjectList->applyPrjPath(st->name);
    OpenFileFromEditor(dest);
    string_free(dest);
   }
 else
    OpenFileFromEditor(st->name);
}


int TEditorProjectListBox::addFile(char *name, Boolean interactive)
{
 ccIndex pos;
 int flags=interactive ? crtInteractive : 0;

 if (!ProjectList->addFile(name,pos,flags))
   {
    if (interactive)
       messageBox(__("File already in project"),mfOKButton | mfError);
    return 0;
   }
 setRange(ProjectList->getCount());
 focusItem(pos);
 drawView();
 return 1;
}

/**[txh]********************************************************************

  Description:
  Adds a file to the project file collection, but can be used to check if a
file is already part of the collection.
  
***************************************************************************/

int TPrjItemColl::addFile(char *name, ccIndex &pos, int flags, char **test)
{
 char *sName=GetShortName(name);
 char *relName=NULL;
 int oldSortMode=sortMode, ret=1;

 if (sortMode!=prjShortName)
    // We must be in short mode to make the next search
    changeSorting(prjShortName);
 if (search(sName,pos))
   {
    PrjItem *st=At(pos);
    string_dup(relName,name);
    AbsToRelPath(referencePath,relName,0);
    if (strcmp(relName,st->name)==0 || search(relName,pos))
      {
       ret=0;
       pos=-1; // Avoid tracking it
      }
    flags|=crtUseFullName;
   }
 if (ret)
   {// Can be inserted
    if (test)
      {
       pos=-1;
       *test=NULL;
      }
    else
      {
       atInsert(pos,name,flags);
       PathListAddPathFor(at(pos),paliSource);
      }
    // Not returned, free it
    string_free(relName);
   }
 else
   {// Already there
    if (test)
       *test=relName;
    else
       string_free(relName);
   }
 if (oldSortMode!=sortMode)
    // Revert the sorting adjusting the position
    changeSorting(oldSortMode,pos);
 return ret;
}

void TEditorProjectListBox::delFile(void)
{
 int c=ProjectList->getCount();

 if (c>0)
   {
    ProjectList->atFree(focused);
    setRange(c-1);
    drawView();
   }
}

void TEditorProjectListBox::handleEvent(TEvent &event)
{
 char name[PATH_MAX];

 TSortedListBox::handleEvent(event);
 switch (event.what)
   {
    case evKeyDown:
         switch (event.keyDown.keyCode)
           {
            case kbEnter:
                 if (!list() || list()->getCount()==0)
                    return;
                 selectItem(focused);
                 break;
            default:
                 return;
           }
         break;
    case evCommand:
         switch (event.message.command)
           {
            case cmDelete:
                 delFile();
                 break;
            case cmInsert:
                 *name=0;
                 GenericFileDialog(__("Add File"),name,"*",hID_FileOpen,
                                   fdMultipleSel | fdAddButton);
                 break;
            case cmChangeSort:
                 toggleSorting();
                 draw();
                 break;                 
            default:
                 return;
           }
         break;
    case evBroadcast:
         switch (event.message.command)
           {
            case cmFileDialogFileSelected:
                 addFile((char *)event.message.infoPtr);
                 break;
            default:
                 return;
           }
         break;
   }
 clearEvent(event);
}

//class TEditorProjectListBox;

TEditorProjectWindow::TEditorProjectWindow(const TRect & rect,
                                           const char *tit) :
        TWindowInit(TEditorProjectWindow::initFrame),
        TDialog(rect,tit)
{
 if (!ProjectList)
    ProjectList=new TPrjItemColl(5,5);
 TRect r=getExtent();
 r.grow(-1,-1);
 scrollbar=standardScrollBar(sbVertical | sbHandleKeyboard);
 list=new TEditorProjectListBox(r,(TSetEditorApp::geFlags & geVertWindows) ? 1 : 3,scrollbar);
 growMode=gfGrowHiX | gfGrowHiY | gfGrowLoY;
 list->growMode=gfGrowHiX | gfGrowHiY;
 list->newList(ProjectList);
 insert(list);
 flags|=wfGrow | wfZoom;
 options|=ofFirstClick;
 helpCtx=hcEditorProjectWindow;
 number=1;
 sizeBufTitle=0;
 bufTitle=0;
 FileName=0;
 titleCache=NULL;
}

TEditorProjectWindow::~TEditorProjectWindow()
{
 CLY_destroy(ProjectList);
 ProjectList=NULL;
 delete[] bufTitle;
 delete[] FileName;
 TVIntl::freeSt(titleCache);
}

void TEditorProjectWindow::handleEvent(TEvent& event)
{
 if (event.what==evKeyboard && event.keyDown.keyCode==kbEsc)
   {
    close();
    clearEvent(event);
    return;
   }
 TDialog::handleEvent(event);
}

void TEditorProjectWindow::close()
{
 hide();
}

const char *TEditorProjectWindow::getTitle(short maxSize)
{
 const char *intlTitle=TVIntl::getText(title,titleCache);
 int len=strlen(intlTitle)+strlen(FileName)+4;
 if (len>sizeBufTitle)
   {
    delete[] bufTitle;
    bufTitle=new char[len];
   }
 sprintf(bufTitle,"%s - %s",intlTitle,FileName);

 return bufTitle;
}

TStreamable *TDskWinPrj::build()
{
 return new TDskWinPrj( streamableInit );
}

static char *Signature="Editor project file\x1A";

void TDskWinPrj::write( opstream& os )
{
 os << window->origin << window->size
    << ProjectList  << (int)(TProgram::deskTop->indexOf(window));
}

void *TDskWinPrj::read( ipstream& is )
{
 TRect pos;

 is >> pos.a >> pos.b >> ProjectList >> ZOrder;
 pos.b+=pos.a;
 window=new TEditorProjectWindow(pos,__("Project Window"));
 view=window;

 return this;
}

char *TDskWinPrj::GetText(char *dest, short maxLen)
{
 TVIntl::snprintf(dest,maxLen,__(" 1 Project Window"));
 return dest;
}

char *TDskWinPrj::getFileName()
{
 if (!window)
    return 0;
 return window->FileName;
}

void TDskWinPrj::setFileName(char *file)
{
 if (window)
   {
    delete[] window->FileName;
    window->FileName=newStr(file);
    ProjectList->analizeReference(file);
   }
}

TDskWinPrj::TDskWinPrj(char *fName)
{
 TRect r=TProgram::deskTop->getExtent();
 if (TSetEditorApp::geFlags & geVertWindows)
   {
    if (TSetEditorApp::geFlags & geRightSide)
       r.a.x=r.b.x-TSetEditorApp::widthVertWindows;
    else
       r.b.x=TSetEditorApp::widthVertWindows;
   }
 else
    r.a.y=r.b.y-7;
 view=window=new TEditorProjectWindow(r,__("Project Window"));
 setFileName(fName);
 type=dktPrj;
 CanBeSaved=0;
 ZOrder=-1;
 wS=TScreen::getCols();
 hS=TScreen::getRows();
}

TDskWinPrj::~TDskWinPrj()
{
 CLY_destroy(window);
 editorApp->SetTitle();
}

int TDskWinPrj::GoAction(ccIndex )
{
 TProgram::deskTop->lock();
 setFocusTo=window;
 focusChanged=True;

 return 0;
}


int TDskWinPrj::DeleteAction(ccIndex, Boolean)
{
 //CloseProject(1); That's imposible because destroy the current object.
 return 0;
}

void LoadProject(char *name)
{
 #ifdef BROKEN_CPP_OPEN_STREAM
 int h=open(name, O_RDONLY | O_BINARY);
 fpstream *f=new fpstream(h);
 #else
 fpstream *f=new fpstream(name,CLY_IOSIn | CLY_IOSBin);
 #endif

 if (!f)
    messageBox(__("Could not open project file"), mfOKButton | mfError);
 else
   {
    char buffer[80];
   
    f->readString(buffer,80);
    if (strcmp(buffer,Signature)!=0)
       messageBox(__("Wrong project file."), mfOKButton | mfError);
    else
      {
       *f >> LoadingPrjVersion;
       ushort wS, hS;
       if (LoadingPrjVersion>4)
          *f >> wS >> hS;
       else
         {
          wS=TScreen::getCols();
          hS=TScreen::getRows();
         }
       *f >> prjWin;
       if (LoadingPrjVersion>6)
         {
          char aux;
          *f >> aux;
          if (ProjectList)
             ProjectList->changeSorting(aux);
         }
       prjWin->wS=wS;
       prjWin->hS=hS;

       if (LoadingPrjVersion>1)
          SDGInterfaceReadData(f);
       // KEEP it at the end, the load is conditional.
       // We don't load it if libmigdb doesn't exist.
       if (LoadingPrjVersion>=8)
          DebugReadData(*f);
       if (prjWin)
          prjWin->setFileName(name);
       if (GetAutoGenMode()==stfAutoCentral)
          ProjectInsertAutoTagFiles();
      }
    if (!f)
       messageBox(__("Error reading project file"), mfOKButton | mfError);
    else
       editorApp->SetTitle(__("Project: "),name);
    f->close();
   }
 delete f;
}

static void UpdateResume(void *p, void *)
{
 PrjItem *item=(PrjItem *)p;
 TCEditWindow *win=IsAlreadyOnDesktop(item->name);
 if (win)
   {
    win->FillResume(item->resume);
    // Indicate is used
    item->resume.prj_flags|=1;
   }
}

static void SaveOnlyProject(void)
{
 fpstream *f=new fpstream(prjWin->getFileName(),CLY_IOSOut | CLY_IOSBin);

 if (f)
   {
    // Update the information about the windows
    ProjectList->forEach(UpdateResume,0);
    // Save a signature to identify the file
    f->writeString(Signature);
    // Save the version & project
    ushort wS=TScreen::getCols(), hS=TScreen::getRows();
    *f << TEditorProjectWindow::Version << wS << hS << prjWin
       << (char)ProjectList->getSortMode();
    SDGInterfaceSaveData(f);
    DebugSaveData(*f);
    if (!f)
      {
       messageBox(__("Could not save the project."), mfOKButton | mfError);
       ::remove(prjWin->getFileName());
      }
    else
       f->close();
   }
 delete f;
}

static
void HideDesktop(const char *s, int DesktopFilesOptions)
{
 // In UNIX the file name changes and we must have space for it.
 char buf[PATH_MAX];
 strcpy(buf,s);
 if (DesktopFilesOptions & dstHide)
    MakeFileHidden(buf);
 else
    // If we are creating a non-hidden file be sure we don't left a hidden
    // one. That's possible under UNIX.
    RemoveFileHidden(buf);
}

void SaveProject(void)
{
 int DesktopFilesOptions=GetDSTOptions();
 int remove=0,makeBackUp=0;
 char *s=0;

 if (PrjExists())
   {
    SaveOnlyProject();
    s=strdup(prjWin->getFileName());
    if (s)
      {
       ReplaceExtension(s,DeskTopFileExt,ProjectFileExt);
       remove=1;
      }
   }
 if (!s)
   {
    if ((DesktopFilesOptions & dstCreate) || DstLoadedHere)
       s=(char *)cDeskTopFileName;
    else
      {// When we use just one desktop file try to back-up it
       makeBackUp=1;
       s=ExpandHomeSave(cDeskTopFileName);
      }
   }
 editorApp->saveDesktop(s,makeBackUp);
 HideDesktop(s,DesktopFilesOptions);
 if (remove)
    free(s);
}

void SaveDesktopHere(void)
{
 editorApp->saveDesktop(cDeskTopFileName,0);
 HideDesktop(cDeskTopFileName,GetDSTOptions());
}

static int HaveExtention(char *name)
{
 char *slash=strrchr(name,'/');
 char *point=strrchr(name,'.');
 if (slash)
    return point && point>slash;
 return point!=NULL;
}

void OpenProject(char *name, int preLoad)
{
 char *s,fname[PATH_MAX];

 if (!name)
   {
    strcpy(fname,"*" ProjectFileExt);
    if (GenericFileDialog(__("Open Project"),fname,0,hID_ProjectFiles)!=cmCancel)
      {
       if (!HaveExtention(fname))
          strcat(fname,ProjectFileExt);
       s=fname;
      }
    else
       return;
   }
 else
   {
    strcpy(fname,name);
    s=fname;
   }

 // The user could specify it without extension
 int prjFound=edTestForFile(s);
 if (!prjFound && !strstr(s,".epr")) // TODO: strstr isn't the best, .epr could be in a directory name
   {
    strcat(s,".epr");
    prjFound=edTestForFile(s);
   }
 if (prjFound)
   { // Load it
    if (!preLoad) CloseProject(0);
    ReplaceExtension(s,DeskTopFileExt,ProjectFileExt);
    char *hidden=0;
    Boolean loaded=False;
    if (!edTestForFile(s))
      {
       hidden=MakeItHiddenName(s);
       if (hidden)
         {
          if (edTestForFile(hidden))
             loaded=TSetEditorApp::retrieveDesktop(editorApp,hidden,True,preLoad);
          delete[] hidden;
         }
      }
    else
       loaded=TSetEditorApp::retrieveDesktop(editorApp,s,True,preLoad);
    if (!loaded)
       TSetEditorApp::loadEditorDesktop(0,0,0,preLoad);
    if (preLoad)
       return;
    LoadProject(ReplaceExtension(s,ProjectFileExt,DeskTopFileExt));
   }
 else
   { // Is a new one
    if (preLoad)
      {
       TSetEditorApp::loadEditorDesktop(0,NULL,0,preLoad);
       return;
      }
    CloseProject(1);
    prjWin=new TDskWinPrj(s);
    editorApp->SetTitle(__("Project: "),s);
   }
 if (prjWin && prjWin->window)
   {
    editorApp->deskTop->lock();
    InsertInOrder(editorApp->deskTop,prjWin);
    // prjWin coordinates correction
    TPoint dS;
    dS.x=TScreen::getCols()-prjWin->wS;
    dS.y=TScreen::getRows()-prjWin->hS;
    if (dS.x!=0 || dS.y!=0)
      {
       TRect  r;
       r=prjWin->view->getBounds();
       r.a.y+=dS.y;
       r.b.y+=dS.y;
       r.b.x+=dS.x;
       if (r.a.y<0) r.a.y=0;
       prjWin->view->changeBounds(r);
      }
    editorApp->deskTop->unlock();

    TSetEditorApp::edHelper->addNonEditor(prjWin);
    editorApp->enableCommand(cmeClosePrj);
    editorApp->enableCommand(cmeSavePrj);
    editorApp->enableCommand(cmeImportPrj);
    editorApp->enableCommand(cmeExportPrj);
    editorApp->enableCommand(cmeSDG);
    editorApp->enableCommand(cmeSelWinPrj);
   }
 else
    prjWin=NULL;
}

// It close a project and the dektop
void CloseProject(int openDesktop)
{
 editorApp->disableCommand(cmeClosePrj);
 editorApp->disableCommand(cmeSavePrj);
 editorApp->disableCommand(cmeImportPrj);
 editorApp->disableCommand(cmeExportPrj);
 editorApp->disableCommand(cmeSDG);
 editorApp->disableCommand(cmeSelWinPrj);
 if (PrjExists())
   {
    // Save the actual state
    SaveProject();
   }
 // Abort any debug session
 // Note: The interactive cmeOpenPrj and cmeClosePrj asks for confirmation,
 // shouldn't be a surprise ;-)
 TSetEditorApp::DebugDeInitVars();
 // Close all the DeskTop windows
 CLY_destroy(TSetEditorApp::edHelper);
 TSetEditorApp::edHelper=0;
 prjWin=0;
 if (GetAutoGenMode()==stfAutoCentral)
    RemoveAutoGenerated();
 // Load a desktop, but not a project
 if (openDesktop)
    TSetEditorApp::loadEditorDesktop(0);
}


// That's the interface with the SDG module.
// These routines must provide the buffers with sources from the project

static ccIndex CountFiles;
static ccIndex CantFiles;

char *DskPrjGetNextFile(int &l, int &MustBeDeleted, char *FileName)
{
 FILE *f;
 char *buffer,*pos,*name;
 TCEditWindow *ed;

 if (CountFiles<CantFiles)
   {
    name=((PrjItem *)(ProjectList->at(CountFiles)))->name;
    CountFiles++;
    ed=IsAlreadyOnDesktop(name);
    if (ed)
      {
       buffer=ed->editor->buffer;
       l=ed->editor->bufLen;
       MustBeDeleted=0;
      }
    else
      {
       // Read the file
       f=fopen(name,"rt");
       if (!f)
         {
          messageBox(mfOKButton | mfError,__("Failed to open the file %s"),name);
          return NULL;
         }
      
       l=filelength(fileno(f))+1;
       buffer=new char[l];
       if (!buffer)
         {
          fclose(f);
          return NULL;
         }
       fread(buffer,l,1,f);
       buffer[l-1]=0;
       fclose(f);
       MustBeDeleted=1;
      }

    // Let just the filename
    pos=strrchr(name,'/');
    if (pos)
       pos++;
    else
       pos=name;
    strcpy(FileName,pos);
    return buffer;
   }
 return NULL;
}

// Initialize the counter to 0
// 1 if error
int DskPrjSDGInit(void)
{
 if (PrjExists())
   {
    CountFiles=0;
    CantFiles=ProjectList->getCount();
    if (CantFiles)
       return 0;
   }
 return 1;
}

int AskForProjectResume(EditorResume *r,char *fileName)
{
 ccIndex pos;

 if (!fileName || !PrjExists())
    return 0;
 if (ProjectList->Search(fileName,pos))
   {
    EditorResume *p=&(((PrjItem *)(ProjectList->at(pos)))->resume);
    if (p->prj_flags & 1)
      {
       CopyEditorResume(r,p);
       return 4;
      }
   }
 return 0;
}

void UpdateProjectResumeFor(char *fileName, TCEditWindow *p)
{
 ccIndex pos;

 if (PrjExists() && ProjectList->Search(fileName,pos))
   {
    EditorResume &r=((PrjItem *)(ProjectList->at(pos)))->resume;
    p->FillResume(r);
    r.prj_flags|=1;
   }
}

int IsPrjOpened()
{
 return PrjExists();
}

int IsPrjVisible()
{
 if (!PrjExists())
    return 0;
 return prjWin->view->state & sfVisible;
}

int IsPrjZoomed()
{
 if (!PrjExists())
    return 0;
 TRect dkt=TApplication::deskTop->getExtent();
 TRect size=prjWin->window->getBounds();
 return dkt==size;
}

struct FileTm
{
 FILE  *f;
 time_t t;
 int    c;
 uint32 mask;
};

static
void PrintName(void *p, void *data)
{
 FileTm *st=(FileTm *)data;
 fprintf(st->f," \"%s\" ",((PrjItem *)p)->name);
 st->c++;
}

static
void PrintNameLine(void *p, void *data)
{
 FileTm *st=(FileTm *)data;
 fprintf(st->f,"%s\n",((PrjItem *)p)->name);
 st->c++;
}

static
void PrintNameTm(void *pt, void *data)
{
 FileTm *st=(FileTm *)data;
 PrjItem *p=(PrjItem *)pt;
 struct stat stS;

 if ((p->forceTarget & st->mask) ||
     (stat(p->name,&stS)==0 && difftime(stS.st_mtime,st->t)>0.0))
   {
    fprintf(st->f,"%s\n",p->name);
    st->c++;
   }
}

static
void ClearTargets(void *pt, void *data)
{
 FileTm *st=(FileTm *)data;
 PrjItem *p=(PrjItem *)pt;
 p->forceTarget&= ~st->mask;
}

static
void SetTargets(void *pt, void *data)
{
 FileTm *st=(FileTm *)data;
 PrjItem *p=(PrjItem *)pt;
 p->forceTarget|=st->mask;
}

/**[txh]********************************************************************

  Description:
  Writes all the names of the project items to the stream f. The names are
separated by spaces.@p
  Used by the Grep Interface.@p

  Returns:
  The number of names sent to the stream.

***************************************************************************/

int WriteNamesOfProjectTo(FILE *f, unsigned mode)
{
 struct FileTm st;
 st.f=f;
 st.c=0;
 if (PrjExists() && ProjectList)
   {
    switch (mode)
      {
       case wnopEspaceSep:
            ProjectList->forEach(PrintName,&st);
            break;
       case wnopLineSep:
            ProjectList->forEach(PrintNameLine,&st);
            break;
      }
   }
 return st.c;
}

int WriteNamesOfProjectToTime(FILE *f, time_t timeT, uint32 targetMask)
{
 struct FileTm st;
 st.f=f;
 st.c=0;
 st.t=timeT;
 st.mask=targetMask;
 if (PrjExists() && ProjectList)
    ProjectList->forEach(PrintNameTm,&st);

 return st.c;
}

int ClearForceTargetBits(uint32 bits)
{
 struct FileTm st;
 st.c=0;
 st.mask=bits;
 if (PrjExists() && ProjectList)
    ProjectList->forEach(ClearTargets,&st);

 return st.c;
}

int SetForceTargetBits(uint32 bits)
{
 struct FileTm st;
 st.c=0;
 st.mask=bits;
 if (PrjExists() && ProjectList)
    ProjectList->forEach(SetTargets,&st);

 return st.c;
}


/**[txh]********************************************************************

  Description:
  Offers a dialog to choose a file name and exports the project items to
the selected file. One project item per line.
  
***************************************************************************/

void ExportProjectItems()
{
 if (!(PrjExists() && ProjectList))
    return;
 char buffer[PATH_MAX];
 strcpy(buffer,"*.txt");

 if (GenericFileDialog(__("Export project items"),buffer,0,
     hID_ExportProjectItems,fdDialogForSave)==cmCancel)
    return;

 if (edTestForFile(buffer) &&
     doEditDialogLocal(edFileExists,buffer,0)==cmNo)
    return;

 FILE *f=fopen(buffer,"wt");
 if (!f)
   {
    TCEditor::editorDialog(edCreateError,buffer);
    return;
   }
 WriteNamesOfProjectTo(f,wnopLineSep);
 int err=ferror(f);
 if (fclose(f) || err)
    TCEditor::editorDialog(edWriteError,buffer);
}

/**[txh]********************************************************************

  Description:
  Asks for a file to read project items. The items are only added, never
removed and only if the referred file already exists. The routine uses the
facilities to detect duplicated files. The files are added "as-is", relative
files are preserved and absolute files are added absolute.
  
***************************************************************************/

void ImportProjectItems()
{
 if (!(PrjExists() && ProjectList))
    return;
 char buffer[PATH_MAX];
 strcpy(buffer,"*.txt");

 if (GenericFileDialog(__("Import project items"),buffer,0,
     hID_ExportProjectItems,fdSelectButton)==cmCancel)
    return;

 unsigned rejected=0, repeated=0, added=0;
 FILE *f=fopen(buffer,"rt");
 if (!f)
   {
    messageBox(__("Unable to open file"),mfError | mfOKButton);
    return;
   }
 while (!feof(f))
   {
    if (fgets(buffer,PATH_MAX,f))
      {
       char *s;
       for (s=buffer; *s && *s!='\r' && *s!='\n'; s++);
       *s=0;
       char *dest=buffer;
       if (!CheckIfPathAbsolute(buffer) && ProjectList->referenceCurDelta)
         {// The project was loaded from another directory, not curdir
          dest=ProjectList->applyPrjPath(buffer);
         }
       if (edTestForFile(dest))
         {
          ccIndex pos;
          if (ProjectList->addFile(dest,pos))
             added++;
          else
             repeated++;
         }
       else
          rejected++;
       if (dest!=buffer)
          string_free(dest);
      }
   }
 prjWin->window->list->setRange(ProjectList->getCount());
 prjWin->window->list->drawView();
 int err=ferror(f);
 if (fclose(f) || err)
    TCEditor::editorDialog(edReadError,buffer);

 messageBox(mfInformation|mfOKButton,
            __("Results: added %d, already included: %d, rejected %d"),
            added,repeated,rejected);
}

void ProjectInsertAutoTagFiles()
{
 if (!ProjectList)
    return;
 if (ProjectList->referenceCurDelta)
   {
    char *s=ProjectList->applyPrjPath("tags");
    InsertAutoGenerated(s);
    string_free(s);
   }
 else
    InsertAutoGenerated("tags");
}

Boolean ProjectGetSize(TRect &r)
{
 if (!prjWin)
    return False;
 TRect dkt=TApplication::deskTop->getExtent();
 TRect size=prjWin->window->getBounds();
 if (dkt==size)
    r=prjWin->window->zoomRect;
 else
    r=size;
 return True;
}

char *GetRelIfFileInPrj(char *name)
{
 if (!prjWin || !name)
    return NULL;
 ccIndex pos;
 char *relName;
 // :-P
 ((TPrjItemColl *)prjWin->window->list->list())->addFile(name,pos,0,&relName);

 return relName;
}

char *GetAbsForNameInPrj(const char *name)
{
 ccIndex pos;

 if (PrjExists() && ProjectList->Search((char *)name,pos))
    return ((PrjItem *)(ProjectList->at(pos)))->name;
 return NULL;
}

void ProjectApplyToItems(ccAppFunc action, void *arg)
{
 if (PrjExists() && ProjectList)
    ProjectList->forEach(action,arg);
}

void ProjectGetNameFromItem(void *p, char *dest, int size)
{
 PrjItem *st=(PrjItem *)p;

 if (ProjectList->referenceCurDelta)
   {// The project was loaded from another directory, not curdir
    char *ori=ProjectList->applyPrjPath(st->name);
    strncpyZ(dest,ori,size);
    string_free(ori);
   }
 else
    strncpyZ(dest,st->name,size);
}

