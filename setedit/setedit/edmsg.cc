/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#define Uses_string
#define Uses_stdio
#define Uses_access
#define Uses_unistd

#define Uses_TDialog
#define Uses_TScrollBar
#define Uses_TProgram
#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_TWindowList
#define Uses_TListBox
#define Uses_TSOSCol
#define Uses_TSOSListBox
#define Uses_SOStack
#define Uses_MsgBox
#define Uses_TKeys
#define Uses_TCEditor_Commands
#define Uses_TCEditor_External
#define Uses_TCEditor
#define Uses_FileOpenAid
#define Uses_MsgBox
#define Uses_TVCodePage
#define Uses_TScreen
#define Uses_TDeskTop
#define Uses_TVOSClipboard
#include <ceditor.h>
#define Uses_TSOSListBoxMsg
#include <edmsg.h>
#include <dskwin.h>
#include <dskmessa.h>
#define Uses_SETAppConst
#define Uses_SETAppHelper
#define Uses_SETAppVarious
#define Uses_TSetEditorApp
#include <setapp.h>
#include <splinman.h>
#include <codepage.h>
#include <editcoma.h>

static TEdMsgDialog   *MsgWindow=NULL;
static TSOSListBoxMsg *MsgList  =NULL;
static SOStack        *Stack    =NULL;
static TSOSCol        *MsgCol   =NULL;
static TRect MsgWindowRect(-1,-1,-1,-1);

unsigned TSOSListBoxMsg::opsEnd=lbmReachedMsg;
unsigned TSOSListBoxMsg::opsBeep=1;

static
void ResetHz()
{
 if (MsgList->hScrollBar)
    MsgList->hScrollBar->setValue(0);
}

TEdMsgDialog::TEdMsgDialog(const TRect &aR,const char *t) :
    TWindowInit(TEdMsgDialog::initFrame),
    TDialog(aR,t)
{
 TScrollBar *scrollbar;
 TRect r;
 flags = wfMove | wfGrow | wfZoom | wfClose;
 growMode = gfGrowLoY | gfGrowHiX | gfGrowHiY;
 r=getExtent();
 r.grow(-1,-1);
 scrollbar=standardScrollBar(sbVertical | sbHandleKeyboard);
 MsgList = new TSOSListBoxMsg(r,1,scrollbar);
 MsgList->growMode = gfGrowHiX | gfGrowHiY;
 scrollbar=standardScrollBar(sbHorizontal | sbHandleKeyboard);
 scrollbar->setParams(0,0,1000-(r.b.x-r.a.x),40,1);
 MsgList->hScrollBar = scrollbar;
 insert(MsgList);
 helpCtx=hcMessageWindow;
}

void TEdMsgDialog::changeBounds(const TRect &r)
{
  TDialog::changeBounds(r);
  MsgWindowRect = r;
}

void TEdMsgDialog::close(void)
{
 hide();
}

void TEdMsgDialog::handleEvent(TEvent& event)
{
 if (event.what==evKeyDown && event.keyDown.keyCode==kbEsc)
   {
    close();
    clearEvent(event);
    return;
   }
 TDialog::handleEvent(event);
}

TEdMsgDialog::~TEdMsgDialog()
{
 if (Stack)
   {
    delete Stack;
    Stack=0;
   }
 if (MsgCol)
   {
    delete MsgCol;
    MsgCol=0;
   }
}

static int avoidFocusAction=0;

void TSOSListBoxMsg::focusItem(ccIndex item)
{
 ccIndex old=focused;
 TSOSListBox::focusItem(item);
 if (!avoidFocusAction && item!=old)
   {
    stkHandler aux=(stkHandler)(list()->at(focused));
    aux=Stack->GetPreviousOf(aux);
    FileInfo *fI=(FileInfo *)(Stack->GetPointerOf(aux));
    if (fI->Line>=0)
      {
       //TApplication::deskTop->lock();
       char *fileName=Stack->GetStrOf(Stack->GetPreviousOf(aux));
       ShowFileLine(SpLineGetNewValueOf(fI->Line,fileName),fileName);
       //this->owner->select();
       //TApplication::deskTop->unlock();
      }
   }
}

void TSOSListBoxMsg::selectItem(ccIndex item)
{
 if (item>=list()->getCount()) return;
 TSOSListBox::selectItem(item);
 selectOK=0;
 if (!avoidFocusAction)
   {
    stkHandler aux=(stkHandler)(list()->at(focused));
    char *msg=Stack->GetStrOf(aux);
    aux=Stack->GetPreviousOf(aux);
    FileInfo *fI=(FileInfo *)(Stack->GetPointerOf(aux));
    if (fI->Line>=0)
      {
       char *fileName=Stack->GetStrOf(Stack->GetPreviousOf(aux));
       selectOK=GotoFileLine(SpLineGetNewValueOf(fI->Line,fileName),fileName,msg,
                             fI->offset,fI->len);
      }
   }
}

void TSOSListBoxMsg::setState(uint16 aState, Boolean enable)
{
 TSOSListBox::setState(aState,enable);
 if (aState==sfActive)
    updateCommands(state & sfActive);
}

void TSOSListBoxMsg::updateCommands(int enable)
{
 TCollection *l=list();

 if (MsgList && MsgList->haveJumpLines)
   { // Don't disable it, they must be available even if the window isn't selected
    enableCommand(cmeNextMessage);
    enableCommand(cmePrevMessage);
   }
 else
   {
    disableCommand(cmeNextMessage);
    disableCommand(cmePrevMessage);
   }

 if (enable && l && l->getCount()!=0)
   {
    enableCommand(cmcCopy);
    enableCommand(cmcCopyClipWin);
    enableCommand(cmcSaveAs);
   }
 else
   {
    disableCommand(cmcCopy);
    disableCommand(cmcCopyClipWin);
    disableCommand(cmcSaveAs);
   }
}

void TSOSListBoxMsg::saveAs()
{
 char fileName[PATH_MAX];
 strcpy(fileName,"messages.txt");

 // Hey man, just reuse code ;-)
 if (TCEditor::editorDialog(edSaveAs,fileName)!=cmCancel)
    save(fileName);
}

void TSOSListBoxMsg::save(char *name)
{ // Reusing the editor's dialogs this function is very complet
 if (access(name,F_OK)==0 && TCEditor::editorDialog(edFileExists,name,0)==cmNo)
    return;
 FILE *f=fopen(name,"wt");
 if (!f)
   {
    TCEditor::editorDialog(edCreateError,name);
    return;
   }
 int c=MsgCol->getCount();
 for (int i=0; i<c; i++)
    {
     fputs(MsgCol->atStr(i),f);
     fputs("\n",f);
    }
 if (ferror(f))
    TCEditor::editorDialog(edWriteError,name);
 fclose(f);
}

void TSOSListBoxMsg::copyClipboard(Boolean osClipboard)
{
 if (!TCEditor::clipboard)
    return;

 int c=MsgCol->getCount();
 int len=1,i,lacu;
 for (i=0; i<c; i++)
     len+=strlen(MsgCol->atStr(i))+CLY_LenEOL;

 char *buffer=new char[len];
 if (!buffer)
   {
    TCEditor::editorDialog(edOutOfMemory);
    return;
   }

 for (lacu=0, i=0; i<c; i++)
    {
     char *s=MsgCol->atStr(i);
     strcpy(buffer+lacu,s);
     lacu+=strlen(s);
     strcpy(buffer+lacu,CLY_crlf);
     lacu+=CLY_LenEOL;
    }

 if (osClipboard)
   {
    if (TVOSClipboard::isAvailable())
       TVOSClipboard::copy(0,buffer,lacu);
   }
 else
    TCEditor::clipboard->insertText(buffer,lacu,True);

 delete[] buffer;
}


void TSOSListBoxMsg::handleEvent(TEvent& event)
{
 if ((event.what==evKeyDown   && event.keyDown.keyCode==kbEnter) ||
     (event.what==evMouseDown && event.mouse.doubleClick))
   {
    selectItem(focused);
    clearEvent(event);
    return;
   }
 else
 if (event.what==evKeyDown && event.keyDown.keyCode==kbDel)
   {
    TCollection *l=list();
    if (l && focused<l->getCount())
      {
       l->atRemove(focused);
       int c=l->getCount();
       setRange(c);
       if (!c)
          updateCommands(0);
      }
    draw();
    clearEvent(event);
    return;
   }
 else
   if (event.what==evCommand)
     {
      switch (event.message.command)
        {
         case cmeNextMessage:
              selectNext();
              clearEvent(event);
              return;
         case cmePrevMessage:
              selectPrev();
              clearEvent(event);
              return;
         case cmcSaveAs:
              saveAs();
              clearEvent(event);
              break;
         case cmcCopy:
              copyClipboard(False);
              clearEvent(event);
              break;
         case cmcCopyClipWin:
              copyClipboard(True);
              clearEvent(event);
              break;
        }
     }
 TListBox::handleEvent(event);
}

int TSOSListBoxMsg::getLineOf(int pos)
{
 stkHandler aux=(stkHandler)(list()->at(pos));
 aux=Stack->GetPreviousOf(aux);
 FileInfo *fI=(FileInfo *)(Stack->GetPointerOf(aux));
 return fI->Line;
}

static inline
void MakeBeep()
{
 CLY_Beep();
}

void TSOSListBoxMsg::selectNext(int offset)
{
 if (!haveJumpLines) return;
 int nFocus=focused+offset;

 while (nFocus<range)
   {
    if (getLineOf(nFocus)>=0)
      {
       TSOSListBox::focusItem(nFocus);
       selectItem(focused);
       ResetHz();
       if (!selectOK)
          owner->select();
       return;
      }
    nFocus++;
   }
 if (opsBeep)
    MakeBeep();
 if (opsEnd==lbmReachedMsg)
    messageBox(__("Last message in the list"),mfInformation | mfOKButton);
 else if (opsEnd==lbmWrap)
    {// Go to the first
     focused=0;
     selectNext(0);
     return;
    }
 // Try to go to the last
 selectPrev(0);
}

void TSOSListBoxMsg::selectPrev(int offset)
{
 if (!haveJumpLines) return;
 int nFocus=focused-offset;

 while (nFocus>=0)
   {
    if (getLineOf(nFocus)>=0)
      {
       TSOSListBox::focusItem(nFocus);
       selectItem(focused);
       ResetHz();
       if (!selectOK)
          owner->select();
       return;
      }
    nFocus--;
   }
 if (opsBeep)
    MakeBeep();
 if (opsEnd==lbmReachedMsg)
    messageBox(__("First message in the list"),mfInformation | mfOKButton);
 else if (opsEnd==lbmWrap)
    {// Go to the first
     focused=range;
     selectPrev(1);
     return;
    }
 // Try to go to the first
 selectNext(0);
}

static void InsertInHelper(void)
{
 if (!SearchInHelper(dktMessage,MsgWindow))
   {
    TDskWinMessage *p=new TDskWinMessage(MsgWindow);
    AddNonEditorToHelper(p);
   }
}

TEdMsgDialog *EdMessageWindowInit(int Insert)
{
 SpLinesDeleteForId(idsplError);
 // If already initialized clean & return
 if (MsgWindow)
   {
    MsgCol->removeAll();
    MsgList->setRange(0);
    MsgList->haveJumpLines=0;
    Stack->Clean();
    MsgWindow->hide();
    if (Insert)
       InsertInHelper();
    return MsgWindow;
   }
 // Create an stack for the messages:
 Stack=new SOStack;
 if (!Stack)
    return MsgWindow;
 // Create the list:
 MsgCol=new TSOSCol(8,6,Stack);
 if (!MsgCol)
    return MsgWindow;
 // Create the dialog:
 if (MsgWindowRect.a.x==-1)
   {
    MsgWindowRect = TProgram::deskTop->getExtent();
    if (TSetEditorApp::geFlags & geVertWindows)
      {
       if (TSetEditorApp::geFlags & geRightSide)
          MsgWindowRect.a.x=MsgWindowRect.b.x-TSetEditorApp::widthVertWindows;
       else
          MsgWindowRect.b.x=TSetEditorApp::widthVertWindows;
      }
    else
       MsgWindowRect.a.y=MsgWindowRect.b.y-7;
   }
 MsgWindow=new TEdMsgDialog(MsgWindowRect,__("Message Window"));
 if (!MsgWindow)
   return MsgWindow;
 MsgList=MsgWindow->MsgList; // Global alias
 MsgList->newList(MsgCol);
 MsgList->haveJumpLines=0;

 if (Insert)
   {
    // Insert it hided because is empty
    TProgram::deskTop->lock();
    TProgram::deskTop->insert(MsgWindow);
    MsgWindow->hide();
    TProgram::deskTop->unlock();
    InsertInHelper();
   }

 return MsgWindow;
}


static
stkHandler InsertFileInfo(FileInfo &p)
{
 stkHandler aux=Stack->alloc(sizeof(FileInfo));
 if (aux!=stkNULL)
    memcpy(Stack->GetPointerOf(aux),&p,sizeof(FileInfo));
 return aux;
}

static
stkHandler addString(const char *str)
{
 const char *s=str;
 int x;

 for (x=0; *s; s++)
    {
     if (*s=='\t')
        x|=7;
     else if (*s=='\r')
        x++;
     x++;
    }
 x++;
 stkHandler stk=Stack->alloc(x);
 if (stk==stkNULL)
    return stk;

 char *d=Stack->GetStrOf(stk);
 for (x=0,s=str; *s; s++)
    {
     if (*s=='\t')
       {
        do
          {
           *(d++)=' ';
           x++;
          }
        while (x & 7);
       }
     else if (*s=='\r')
       {
        *(d++)='^';
        *(d++)='M';
        x++;
       }
     else
       {
        x++;
        *(d++)=*s;
       }
    }
 *d=0;

 return stk;
}

void EdShowMessageUpdate(unsigned Options)
{
 TProgram::deskTop->lock();
 MsgList->updateCommands(1);
 if (!(Options & edsmNoHzReset))
    ResetHz();
 MsgWindow->show();
 if ((Options & edsmDontSelect)==0)
    MsgWindow->select();
 MsgList->setRange(MsgCol->getCount());
 // Scroll it
 uint32 opsScr=Options & edsmScrollMask;
 if (opsScr==edsmEverScroll ||
     (opsScr==edsmScrollIfNoFocus && (MsgWindow->state & sfActive)==0))
   {
    avoidFocusAction++;
    MsgList->focusItem(MsgCol->getCount()-1);
    avoidFocusAction--;
   }
 MsgList->drawView();
 //MsgWindow->drawView(); It doesn't work, the above does
 TProgram::deskTop->unlock();
}

void EdShowMessageFile(const char *msg, FileInfo &fInfo, char *fileName,
                       unsigned Options)
{
 if (!MsgWindow || (Options & edsmRemoveOld))
    EdMessageWindowInit();
 if (!msg)
    return;
 stkHandler aux;
 if (fileName)
   {
    aux=Stack->addStr(fileName);
    if (aux==stkNULL)
       return;
   }
 aux=InsertFileInfo(fInfo);
 if (aux==stkNULL)
    return;
 aux=addString(msg);
 if (aux==stkNULL)
    return;
 MsgCol->insert(aux);
 if (fileName && fInfo.Line>=0)
   {
    SpLinesAdd(fileName,fInfo.Line,idsplError,
               Options & edsmUpdateSpLines ? True : False);
    MsgList->haveJumpLines++;
   }

 if (Options & edsmDontUpdate)
    return;
 EdShowMessageUpdate(Options);
}

void EdShowMessage(const char *msg, Boolean remove_old, Boolean resetHz)
{
 FileInfo dummy;
 dummy.Line=-1;
 EdShowMessageFile(msg,dummy,0,(remove_old ? edsmRemoveOld : 0) |
                   (resetHz ? edsmNoHzReset : 0));
}

void EdShowMessageI(const char *msg, Boolean remove_old, Boolean resetHz)
{
 char *aux=TVIntl::getTextNew(msg);
 EdShowMessage(aux,remove_old,resetHz);
 DeleteArray(aux);
}

void EdShowMessage(const char *msg, unsigned Options)
{
 FileInfo dummy;
 dummy.Line=-1;
 EdShowMessageFile(msg,dummy,0,Options);
}

void EdShowMessageI(const char *msg, unsigned Options)
{
 char *aux=TVIntl::getTextNew(msg);
 EdShowMessage(aux,Options);
 DeleteArray(aux);
}

void EdJumpToMessage(ccIndex item)
{
 if (!MsgWindow)
    EdMessageWindowInit();
 if (item<MsgCol->getCount())
   {
    avoidFocusAction=1;
    ResetHz();
    MsgList->focusItem(item);
    avoidFocusAction=0;
   }
}

void EdJumpToFirstError(void)
{
 if (!MsgWindow || !MsgList)
    return;
 MsgList->focusItem(0);
 EdMessageSelectNext();
}

int EdMessageCantMessages(void)
{
 return MsgCol->getCount();
}

void EdMessageSelectNext(void)
{
 if (MsgList && MsgList->range)
    MsgList->selectNext();
}

void EdMessageSelectPrev(void)
{
 if (MsgList && MsgList->range)
    MsgList->selectPrev();
}

Boolean EdMessageGetSize(TRect &r)
{
 if (!MsgWindow)
    return False;
 TRect dkt=TApplication::deskTop->getExtent();
 TRect size=MsgWindow->getBounds();
 if (dkt==size)
    r=MsgWindow->zoomRect;
 else
    r=size;
 return True;
}

