/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*****************************************************************************

  TCEditor class by SET.

  E-Mail: salvador@inti.gov.ar or set@ieee.org or set@computer.org
  
  Telephone: (+5411) 4759-0013
  
  Postal Address:
  Salvador E. Tropea
  CurapaligÅe 2124
  (1678) Caseros - 3 de Febrero
  Prov: Buenos Aires
  Argentina

  Contributors:
  Robert Hîhne    (Robert.Hoehne@Mathematik.TU-Chemnitz.DE)
  Marek Habersack (grendel@ananke.amu.edu.pl)
  Molnar Laszlo   (molnarl@postabank.hu)

*****************************************************************************/

// That's the first include because is used to configure the editor.
#include <ceditint.h>

#define Uses_string
#define Uses_stdio
#define Uses_stdlib
#define Uses_alloca
#define Uses_ctype
#define Uses_AllocLocal
#define Uses_time
#define Uses_fcntl
#define Uses_sys_stat
#define Uses_unistd
#define Uses_regex
#define Uses_utime
#define Uses_filelength
#define Uses_itoa
#define Uses_getline
#define Uses_access
#define Uses_snprintf

#define Uses_TKeys
#define Uses_TFindCDialogRec
#define Uses_TReplaceCDialogRec
#define Uses_TReplaceDialogRec
#define Uses_opstream
#define Uses_ipstream
#define Uses_TStreamableClass
#define Uses_TMacroCollection
#define Uses_TStringCollection
#define Uses_MsgBox
#define Uses_TGroup
#define Uses_TPalette
#define Uses_TCommandSet
#define Uses_TFileDialog
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TGKey
#define Uses_TColorCommands
#define Uses_TSubMenu
#define Uses_TMenuBox
#define Uses_TStringableListBox
#define Uses_TVOSClipboard
#define Uses_TVCodePage

#define Uses_TCEditor
#define Uses_LineLengthArray
#define Uses_TFindCDialogRec
#define Uses_TReplaceCDialogRec
#define Uses_TCEditor_Internal
#define Uses_TCEditor_External
#define Uses_TCEditor_Commands
#define Uses_TStringable
#define Uses_ProgBar
#define Uses_TScreen
#include <ceditor.h>
#include <tv/tvconfig.h>
#include <edhists.h>
#include <slpinter.h>
#include <bufun.h>
#include <loadshl.h>
#include <advice.h>
#include <splinman.h>

#include <setconst.h>

#include <loadkbin.h>
#include <sys/types.h>
#define Uses_GZInterfaceOnly
#include <gzfiles.h>
#include <pathtool.h>
#ifdef TVOSf_Linux
#include <sys/time.h>
#endif
#include <rhutils.h>

#ifdef STANDALONE
// The code page support is only for the standalone version
#include <codepage.h>
#endif

#define Uses_TKeyTranslate
#include <keytrans.h>

#define DEBUG

#define flushLine()  if (IslineInEdition) MakeEfectiveLineInEdition()
#define flushLine2(a)  if (a->IslineInEdition) a->MakeEfectiveLineInEdition()

#define CheckScrollLockCenters ((TGKey::getShiftState() & kbScrollLockToggle) && (editorFlags & efScrollLock))

static unsigned LineMeassureC(char *s, char *end, uint32 &Attr, uint32 *extra=0);
static unsigned LineMeassurePascal(char *s, char *end, uint32 &Attr, uint32 *extra=0);
static unsigned LineMeassureClipper(char *s, char *end, uint32 &Attr, uint32 *extra=0);
static unsigned LineMeassureGeneric(char *s, char *end, uint32 &Attr, uint32 *extra=0);
static void readBlock(TCEditor *editor);
static void writeBlock(TCEditor *editor);
static uint32 MakeItGranular( uint32 value );

#define DecWithWrap(a,b) if (a) a--; else a=b-1
#define IncWithWrap(a,b) a++; if (a==b) a=0

#undef min
inline uint32 min( uint32 u1, uint32 u2 )
{
 return u1 < u2 ? u1 : u2;
}

static int ClassInitialized=0;

static
void DeInitTCEditor(void)
{
 CLY_destroy(TCEditor::PMColl);
#ifdef STANDALONE
 DestroySHShortCutTables();
#endif
 TCEditor::FreeRegExMemory();
 ClassInitialized=0;
}

/**[txh]********************************************************************

  Description:
  It disables all the editor's commands. Just a few broadcasts and the
global options remains. You must pass the a copy of the actual commands to
the function, the routine will disable the commands and call the static
member of TView to make it effective.

***************************************************************************/

static
void DisableCommands(TCommandSet &cmdsAux)
{
 cmdsAux.disableCmd(cmbBaseNumber,cmbBaseNumber+cmbLastCommand);
 // Let the broadcasts enabled
 cmdsAux.enableCmd(cmcSetGlobalOptions);
 cmdsAux.enableCmd(cmcColorsChanged);  // Is really needed?
 TView::setCommands(cmdsAux);
}

/****************************************************************************

   Function: int InitTCEditor(char *s,Boolean force)

   Type: Normal function.

   Objetive: Intialize all the things needed by the class.
             The function is automagically called by the constructor if never
   was called but can be called by hand to force some things.

   Things that this function make:
   1) Loads the pseudo macros file.
   2) Creates the pseudo hash tables for the reserved words (if STANDALONE).
   3) Disable all the editor commands.

   Parameters:
   char *s: The name of the Pseudo Macros file.
   Boolean force: If true forces the initialization.

   Returns:
   0 if all OK or the class is already initialized.
   Flags:
   1 The Pseudo macros couldn't be loaded.

   by SET.

****************************************************************************/

int InitTCEditor(char *s,Boolean force)
{
 int ret=0;

 if (!ClassInitialized || force)
   {
    if (!TCEditor::PMColl)
       TCEditor::PMColl=new TPMCollection(32,8);
    if (LoadPseudoMacroFile(ExpandFileNameToThePointWhereTheProgramWasLoaded(s),*TCEditor::PMColl)==False)
       ret|=1;
    TView::getCommands(TCEditor::cmdsAux);
    DisableCommands(TCEditor::cmdsAux);
#ifdef STANDALONE
    CreateSHShortCutTables();
#endif

    ClassInitialized=1;
    atexit(DeInitTCEditor);
   }

 return ret;
}


/****************************************************************************

   Function: TCEditor( const TRect& bounds,TScrollBar *aHScrollBar,
                  TScrollBar *aVScrollBar,TSIndicator *aIndicator,
                  uint32 aBufSize )

   Type: TCEditor member.

   Objetive: Constructor of the Class

   Parameters:
   TRect &bounds: Original size and location for the editor.
   TScrollBar *aHScrollBar
   TScrollBar *aVScrollBar
   TIndicator *aIndicator:  Pointers to objects for scroll bars & info.
   uint32 aBufSize: Starting size of the editor buffer.

   by SET

****************************************************************************/

TCEditor::TCEditor( const TRect& bounds,
                  TScrollBar *aHScrollBar,
                  TScrollBar *aVScrollBar,
                  TSIndicator *aIndicator,
                  const char *aFileName,
                  Boolean openRO ) :
    TViewPlus(bounds),
    hScrollBar(aHScrollBar),
    vScrollBar(aVScrollBar),
    indicator(aIndicator),
    bufSize(4050),
    canUndo(True),
    selecting(False),
    overwrite(False),
    NoNativeEOL(False),
    IsaCompressedFile(gzNoCompressed),
    lockCount(0),
    updateFlags(0),
    keyState(0),
    bufEdit(0),         // Not buffer allocated
    bufEditLen(0),       // zero length
    GenericSHL(0),
    CrossCurInCacheC(False),
    CrossCurInCacheR(False),
    IsStatusLineOn(False),
    IsFoundOn(False),
    IsHLCOn(False),
    SpecialLines(NULL),
    DiskTime(0)
{
 InitTCEditor("pmacros.pmc",False);
 // Initialize the mode of edition
 UseTabs=staticUseTabs;          // Don't put Tabs, indent
 autoIndent=staticAutoIndent;
 indentSize=staticIndentSize;
 intelIndent=staticIntelIndent;
 tabSize=staticTabSize;
 PersistentBlocks=staticPersistentBlocks;
 CrossCursorInCol=staticCrossCursorInCol;
 CrossCursorInRow=staticCrossCursorInRow;
 ShowMatchPair=staticShowMatchPair;
 ShowMatchPairFly=staticShowMatchPairFly;
 ShowMatchPairNow=staticShowMatchPairNow;
 TransparentSel=staticTransparentSel;
 OptimalFill=staticOptimalFill;
 WrapCol=staticWrapCol;
 WrapLine=staticWrapLine;
 SeeTabs=staticSeeTabs;
 NoInsideTabs=staticNoInsideTabs;
 TabIndents=staticTabIndents;
 BackSpUnindents=staticBackSpUnindents;
 UseIndentSize=staticUseIndentSize;
 DontPurgeSpaces=staticDontPurgeSpaces;
 ColumnMarkers=staticColumnMarkers;
 colMarkers=CopyColMarkers(staticColMarkers);
 forceNextTimeCheck=False;
 isDisassemblerEditor=0;
 
 CrossCursorY2=size.y;
 CrossCursorX2=size.x;
 CrossCursorCol=0;
 CrossCursorRow=0;

 isReadOnly=openRO;

 /* TSIndicators uses a pointer to the editor, so link it */
 if (indicator) // Some routines creates temporal editors without indicators
    // or even scroll bars so we must check before accessing.
    indicator->editor=this;

 TurnOffHighLight();
 LineMeassure=LineMeassureC;

 growMode = gfGrowHiX | gfGrowHiY;
 options |= ofSelectable;
 eventMask = evMouseDown | evKeyDown | evCommand | evBroadcast;
 showCursor();
 initBuffer();
 if ( buffer != 0 )
    isValid = True;
 else
   {
    editorDialog( edOutOfMemory );
    bufSize = 0;
    isValid = False;
   }
 setBufLen(0);

 CLY_GetDefaultFileAttr(&ModeOfFile);

 // File part
 if (aFileName==0)
   {
    fileName[0]=EOS;
    SHLTransferDefaultsNewFile(*this);
    FailedToLoad=False;
   }
 else
   {
    strcpy(fileName,aFileName);
    if (isValid)
       isValid=loadFile(True);
   }
}


/****************************************************************************

   Function: ~TCEditor()

   Type: TCEditor member.

   Objetive: Destructor of the Class.
             Deletes the buffer used to edit a line.

   doneBuffer deletes the editor buffer (? Called by ~TView).

   by SET.

****************************************************************************/

TCEditor::~TCEditor()
{
 if (bufEdit)
    free(bufEdit);
 flushUndoInfo();
 delete selRectClip;
 delete[] colMarkers;
}

/****************************************************************************

   Function: shutDown()

   Type: TCEditor member.

   Objetive: Just to shutdown the editor.

****************************************************************************/

void TCEditor::shutDown()
{
 doneBuffer();
 TView::shutDown();
}

/****************************************************************************

   Function: changeBounds( const TRect& bounds )

   Type: TCEditor member.

   Objetive: Adjust the delta in the text when the bounds of the window are
   changed.

   by SET

****************************************************************************/

void TCEditor::changeBounds( const TRect& bounds )
{
 setBounds(bounds);
 delta.x = max(0, min(delta.x, limit.x - size.x));
 delta.y = max(0, delta.y);
 if (curPos.y>=delta.y+size.y)
    delta.y=curPos.y-size.y+1;
 if (curPos.y<delta.y)
    delta.y=curPos.y;
 update(ufView);
}

/****************************************************************************

   Function: Boolean clipCopy()

   Type: TCEditor member.

   Objetive: Copy the selected text to the clipboard.

   Returns: True if OK.
   
****************************************************************************/

Boolean TCEditor::clipCopy()
{
 Boolean res = False;
 flushLine();
 if ( (clipboard != 0) && (clipboard != this) )
   {
    res = clipboard->insertFrom(this);
    selecting = False;
    update(ufUpdate);
   }
 return res;
}

/**[txh]********************************************************************

  Description:
  Copies the selection to the OS clipboard. Originally designed to be used
for Windows.

  Return:
  Boolean True if all was OK.

***************************************************************************/

Boolean TCEditor::clipWinCopy(int id)
{
 Boolean res=False;
 if (hasSelection())
   {
    if (!TVOSClipboard::isAvailable())
      {
       messageBox(__("Sorry but no OS specific clipboard is available"),mfError | mfOKButton);
       return False;
      }
    flushLine();
    res=TVOSClipboard::copy(id,buffer+selStart,selEnd-selStart) ? True : False;
    if (!res)
      {
       messageBox(mfError | mfOKButton,__("Error copying to clipboard: %s"),
                  TVOSClipboard::getError());
       return False;
      }
    // id == 1 => "selection clipboard", in this case we don't want to stop the selection.
    // I'm not sure if we really want to stop the selection in other cases, but that's the
    // old behavior.
    if (!id)
       selecting=False;
   }
 return res;
}

/****************************************************************************

   Function: void clipCut()

   Type: TCEditor member.

   Objetive: Cut the selected text to the clipboard.

****************************************************************************/

void TCEditor::clipCut()
{
 if (isReadOnly) return;
 flushLine();
 if (clipCopy())
    deleteSelect();
}

/****************************************************************************

   Function: void clipPaste()

   Type: TCEditor member.

   Objetive: Paste from the clipboard.

   Modified to avoid insertions at the limit of the line capability.

****************************************************************************/

void TCEditor::clipPaste()
{
 if (isReadOnly) return;
 flushLine();
 if ( (clipboard != 0) && (clipboard != this) && curPos.x<(MaxLineLen-1))
    insertFrom(clipboard);
}

/**[txh]********************************************************************

  Description:
  Pastes the Windows clipboard in the editor. If not persistent deletes
the selection. The pasted text is selected only if persistent blocks are
enabled. This behavior is what a windows user spect and was modified by
suggestion of Anatoli Soltan (Win32 porter).

***************************************************************************/

void TCEditor::clipWinPaste(int id)
{
 if (isReadOnly)
    return;
 if (!TVOSClipboard::isAvailable())
   {
    messageBox(__("Sorry but no OS specific clipboard is available"),mfError | mfOKButton);
    return;
   }
 flushLine();
 if (curPos.x<(MaxLineLen-1))
   {
    unsigned size;
    char *p=TVOSClipboard::paste(id,size);
    if (p)
      {
       if (!PersistentBlocks && hasSelection())
         { // Save the selected text in the windows clipboard
          // That isn't what a windows user spect so I disabled it
          //if (!WINOLDAP_SetClipboard(buffer+selStart,selEnd-selStart))
          //   return;
          deleteSelect();
         }
       insertBuffer(p,0,size,canUndo,PersistentBlocks,False);
       DeleteArray(p);
       trackCursor(False);
      }
    else
       messageBox(mfError | mfOKButton,__("Error pasting from clipboard: %s"),
                  TVOSClipboard::getError());
   }
}

/**[txh]********************************************************************

  Description:
  Copies the selected text into a special file used as clipboard. Specially
useful when the OS lacks a robust clipboard mechanism.
  
***************************************************************************/

Boolean TCEditor::clipFileCopy()
{
 Boolean res=False;
 if (hasVisibleSelection())
   {
    flushLine();
    char *name=ExpandHomeSave("clipboard");
    if (name)
      {
       FILE *f=fopen(name,"wb");
       if (f)
         {
          if (fwrite(buffer+selStart,selEnd-selStart,1,f)!=1)
             editorDialog(edWriteError,name);
          else
             res=True;
          fclose(f);
         }
       else
         editorDialog(edCreateError,name);
      }
    selecting=False;
   }
 return res;
}

/**[txh]********************************************************************

  Description:
  Pastes text from a special file used as clipboard. Specially useful when
the OS lacks a robust clipboard mechanism.
  
***************************************************************************/

void TCEditor::clipFilePaste()
{
 if (isReadOnly)
    return;
 flushLine();
 if (curPos.x<(MaxLineLen-1))
   {
    char *name=ExpandHomeSave("clipboard");
    if (edTestForFile(name))
      {
       FILE *f=fopen(name,"rb");
       if (f)
         {
          long fsize=filelength(fileno(f));
          char *p=new char[fsize];
          if (fread(p,fsize,1,f)==1)
            {
             if (!PersistentBlocks && hasSelection())
                deleteSelect();
             insertBuffer(p,0,fsize,canUndo,PersistentBlocks,False);
             delete[] p;
            }
          else
             editorDialog(edReadError,name);
         }
       else
          editorDialog(edReadError,name);
      }
   }
}

/****************************************************************************

   Function: Boolean clipReplace()

   Type: TCEditor member.

   Objetive: Cut to clipboard and paste from clipboard.

   by SET.

****************************************************************************/

Boolean TCEditor::clipReplace()
{
 if (isReadOnly) return False;
 flushLine();
 if ((clipboard!=0) && (clipboard!=this))
   {
    if (clipboard->insertBuffer(buffer,selStart,selEnd-selStart,False,False,True))
      {
       deleteSelect();
       return insertFrom(clipboard);
      }
   }
 return False;
}



/****************************************************************************

   Function: void convertEvent( TEvent& event )

   Type: TCEditor member.

   Objetive: Translate a key convination to a command.

   Parameters:
   TEvent& event: The event.

   by SET.

****************************************************************************/

void TCEditor::convertEvent( TEvent& /*event*/ )
{
}

/****************************************************************************

   Function: Boolean cursorVisible()

   Type: TCEditor member.

   Objetive: Make visible the cursor.

   From Borland's TV 1.03.

****************************************************************************/

Boolean TCEditor::cursorVisible()
{
 return ((curPos.y >= delta.y) && (curPos.y < delta.y + size.y)) ? True : False;
}


/****************************************************************************

   Function: void deleteRange( uint32 startPtr,uint32 endPtr,
                               Boolean delSelect )

   Type: TCEditor member.

   Objetive: Delete the piece of text from startPtr to endPtr or the selected
   text.

   Parameters:
   startPtr endPtr: Range to delete.
   delSelect: If True and there is a selection deletes the selected text.

   This function is only for compatibility

   by SET.

****************************************************************************/

void TCEditor::deleteRange( uint32 startPtr,
                            uint32 endPtr,
                            Boolean delSelect
                         )
{
 if ( hasSelection() && delSelect )
    deleteSelect();
 else
    deleteRange(buffer+startPtr,buffer+endPtr);
}


/****************************************************************************

   Function: void deleteSelect()

   Type: TCEditor member.

   Objetive: Delete the selected text.

   by SET.

****************************************************************************/

void TCEditor::deleteSelect()
{
 if (hasSelection())
    deleteRange(buffer+selStart,buffer+selEnd);
}


/****************************************************************************

   Function: void doneBuffer()

   Type: TCEditor member.

   Objetive: Delete the editor buffer.

   From Borland's TV 1.03.

****************************************************************************/

void TCEditor::doneBuffer()
{
 free(buffer);
}

/****************************************************************************

   Function: void doSearchReplace()

   Type: TCEditor member.

   Objetive: Make a Search or Replace in the text.

****************************************************************************/

Boolean TCEditor::doSearchReplace()
{
 int i;
 int oldPromptOnReplace=editorFlags & efPromptOnReplace;
 Boolean ret=False, needsUnlock=True;

 lock();
 do
  {
   i=cmCancel;
   if (!search(findStr,editorFlags))
     {
      needsUnlock=False;
      unlock();
      if (!(editorFlags & efNoFindFailMsg) &&
          (editorFlags & (efReplaceAll | efDoReplace))!=(efReplaceAll | efDoReplace))
         editorDialog(edSearchFailed);
      ret=False;
     }
   else
     {
      if (editorFlags & efDoReplace)
        {
         i=cmYes;
         if (editorFlags & efPromptOnReplace)
           {
            unlock();
            TPoint c=makeGlobal(cursor);
            i=editorDialog(edReplacePrompt,&c);
            lock();
           }
         if (i==cmOK)
           { // Used to signal ALL
            i=cmYes;
            editorFlags&=~efPromptOnReplace;
           }
         if (i==cmYes)
           {
            lockUndo();
            int mustDelete;
            uint32 l;
            char *repl=GetTheReplace(mustDelete,l);
            // The delete MUST be after retreiving the match
            deleteRange(buffer+selStartF,buffer+selEndF);
            if (repl)
              {
               insertText(repl,l,False);
               if (mustDelete)
                  delete repl;
              }
            trackCursor(False);
            StartOfSearch=selStartF+l;
            unlockUndo();
            ret=True;
           }
         else
            StartOfSearch=selEndF;
        }
      else
         ret=True;
     }
  }
 while(i!=cmCancel && (editorFlags & efReplaceAll)!=0);

 if (needsUnlock)
    unlock();
 // Restore it in case the ALL changed the value
 editorFlags|=oldPromptOnReplace;
 return ret;
}

/****************************************************************************

   Function: int TestPropagation(uint32 OldAttr,uin16 NewAttr,
                                 char *proxLine, uint32 proxLineNum)

   Type: TCEditor member.

   Objetive: Check if the changes made in a line affects the next lines and
   if is the case update all the affected lines.
     This function updates the syntax highlight flags, for example if you
   start a comment in a line this produce a propagation.

   Parameters:
   OldAttr: Old syntax flags for this line.
   NewAttr: New syntax flags for this line.
   proxLine: a pointer to the next line.
   proxLineNum: the number of the next line.

   by SET.

****************************************************************************/

int TCEditor::TestPropagation(uint32 OldAttr,uint32 NewAttr,
                              char *proxLine, uint32 proxLineNum)
{
 uint32 PrevAttr;

 CacheSyntaxHLData(GenericSHL);

 if ((OldAttr & FilterProp)!=(NewAttr & FilterProp))
   { // The changes afects the rest of the file
    PrevAttr=NewAttr;
    do
     {
      if (proxLineNum>totalLines)
         break;
      OldAttr=lenLines.getAttr(proxLineNum);
      proxLine+=LineMeassure(proxLine,proxLine+lenLines[proxLineNum],PrevAttr,NULL);
      lenLines.setAttr(proxLineNum,PrevAttr);
      proxLineNum++;
     }
    while (OldAttr!=PrevAttr && (unsigned)(proxLine-buffer)<bufLen);
    return 1;
   }
 return 0;
}

/****************************************************************************

   Function: void doUpdate()
             
   Type: TCEditor member.

   Objetive: Update the screen according to updateFlags.

   by SET.

   ToDo:
   1st step) Avoid a constant LineMeassure, call it only when a special key
   is pressed or when some special thing is deleted.
   2nd step) ? Never call to LineMeassure, keep track of the syntax at
   cursor position.

****************************************************************************/

void TCEditor::doUpdate()
{
 CacheSyntaxHLData(GenericSHL);
 if (updateFlags) // Only if needed
   {
    // moves the hardware cursor
    setCursor(curPos.x-delta.x,curPos.y-delta.y);

    // Repair the highligthed position
    if (IsHLCOn)
      {
       if (updateFlags & (ufLine | ufFound | ufStatus | ufHLChar | ufClHLCh))
         {
          int y=YHLCc-delta.y;
          int x=XHLCc-delta.x;
          if (x>=0 && x<size.x && y>=0 && y<size.y)
             setAttrOfCoor(x,y,OldHLAttr);
          y=YHLCo-delta.y;
          x=XHLCo-delta.x;
          if (x>=0 && x<size.x && y>=0 && y<size.y)
             setAttrOfCoor(x,y,OldHLAttro);
          IsHLCOn=False;
         }
       else if (updateFlags & ufView) // It cleans the situation
         IsHLCOn=False;
      }

    unsigned selAuxE=0,selAuxS=0;
    Boolean  oldSelHided=selHided;

    // Used for a temporal select when the search was OK.
    if (updateFlags & ufFound)
      { // Put the selection of the found
       selAuxE=selEnd;
       selAuxS=selStart;
       selEnd=selEndF;
       selStart=selStartF;
       selHided=False;
       IsFoundOn=True;
      }
    else
      {
       if (IsFoundOn)
         { // Put away the fake select
          updateFlags|=ufView;
          IsFoundOn=False;
         }
      }

    if (IsStatusLineOn)
      {
       if (!(updateFlags & ufView))
         {// Repair the damage of the status line
          int y=delta.y;
          unsigned p=drawPtr;
          if (StatusLinePos)
            {
             int i=size.y-1;
             for (;i; y++, --i)
                 p+=lenLines.safeLen(y);
            }
          drawLines(y,1,p);
         }
       // In any case it isn't there anymore
       IsStatusLineOn=False;
      }

    if (updateFlags & ufView)
      {
       if (IslineInEdition && (updateFlags & ufLine))
         { // When we type in the last column the editor forces a full draw and we must
           // test for propagation even when isn't an ufLine alone.
          uint32 attr;
          if (curPos.y)
             attr=lenLines.getAttr(curPos.y-1);
          else
             attr=0;
          LineMeassure(bufEdit,inEditPtr+restCharsInLine,attr,NULL);
          TestPropagation(attrInEdit,attr,curLinePtr+lenLines[curPos.y],curPos.y+1);
          attrInEdit=attr;
         }
       drawView(); // All the window
      }
    else
       if (updateFlags & ufLine)
         {
          if (IslineInEdition)
            { // The following is a test and can be optimized a lot
              uint32 attr;
              if (curPos.y)
                 attr=lenLines.getAttr(curPos.y-1);
              else
                 attr=0;
              LineMeassure(bufEdit,inEditPtr+restCharsInLine,attr,NULL);
              if (TestPropagation(attrInEdit,attr,curLinePtr+lenLines[curPos.y],curPos.y+1))
                {
                 attrInEdit=attr;
                 drawView();
                 updateFlags|=ufView;
                }
              else
                {
                 attrInEdit=attr;
                 drawLines( curPos.y, 1, (uint32)(curLinePtr-buffer) );
                }
            }
          else
             drawLines( curPos.y, 1, (uint32)(curLinePtr-buffer) ); // only this line
         }

    // For the find & replace
    if (updateFlags & ufFound)
      { // Let OK the select
       selEnd=selAuxE;
       selStart=selAuxS;
       selHided=oldSelHided;
      }

    if (CrossCursorInCol || CrossCursorInRow)
       updateCrossCur();

    // The status line
    if (updateFlags & ufStatus)
      { // Put it and record the state
       if (curPos.y==delta.y+size.y-1)
         {
          StatusLinePos=0;
          writeLine(0,0,size.x,1,StatusLine);
         }
       else
         {
          StatusLinePos=1;
          writeLine(0,size.y-1,size.x,1,StatusLine);
         }
       IsStatusLineOn=True;
      }

    if (updateFlags & ufHLChar)
      {
       int y=YHLCC-delta.y;
       int x=XHLCC-delta.x;
       char c=getColor(cMPHighL);
       IsHLCOn=True;
       getAttrsOfCol(x,y,y,&OldHLAttr);
       setAttrOfCoor(x,y,c);
       y=YHLCO-delta.y;
       x=XHLCO-delta.x;
       getAttrsOfCol(x,y,y,&OldHLAttro);
       setAttrOfCoor(x,y,c);
       YHLCc=YHLCC; XHLCc=XHLCC;
       YHLCo=YHLCO; XHLCo=XHLCO;
      }

    // rest of the things
    if ( hScrollBar != 0 )
       hScrollBar->setParams(delta.x, 0, limit.x - size.x, size.x / 2, 1);
    if ( vScrollBar != 0 )
       vScrollBar->setParams(delta.y, 0, limit.y, size.y - 1, 1); // - size.y cutted
    if (indicator)
       indicator->drawView();
    if ( (state & sfActive) != 0 )
       updateCommands();
    updateFlags = 0;
   }
}

/****************************************************************************

   Function: void updateCrossCur(void)

   Type: TCEditor member.

   Objetive: Update the extended cursor (column, row or cross).

     The basic routine is very simple, the actual routine is a little
   complicated because is optimized to allow some redraws.

   by SET.

****************************************************************************/

void TCEditor::updateCrossCur(void)
{
 int i;
 int xAnt=CrossCursorCol;
 int xAct=curPos.x-delta.x;
 int yAnt=CrossCursorRow;
 int yAct=curPos.y-delta.y;
 int updateCol=1;
 int updateRow=1;
 char color=getColor(cCrossCur);

 // Erase the old Col-cursor
 if (CrossCursorInCol && CrossCursorCol>=0  && CrossCursorCol<size.x)
   {
    if (updateFlags & ufView)
       // If the editor was redrawed there is no need to do this
       CrossCursorY2=size.y;
    else
      {
       if (CrossCurInCacheC)
         { // Only if we have the info in the cache
          if (xAnt!=xAct)
            { // If the col was changed make a full erase
             if (updateFlags & ufLine)
               {
                // if the line was redrawed don't touch it
                for (i=0; i<CrossCursorY2; i++)
                    if (i!=yAct)
                       setAttrOfCoor(CrossCursorCol,i,CrossCursorBuf[i]);
               }
             else
               {
                // full column erase
                for (i=0; i<CrossCursorY2; i++)
                    setAttrOfCoor(CrossCursorCol,i,CrossCursorBuf[i]);
               }
            }
          else
            {
             // erase only the last row, but only if the line is unmodified
             if (!(updateFlags & ufLine))
                setAttrOfCoor(xAct,yAct,CrossCursorBuf[yAct]);
             updateCol=0;
            }
         }
       else
          CrossCursorY2=size.y;
      }
   }

 // Erase the old Row-cursor
 if (CrossCursorInRow && CrossCursorRow>=0 && CrossCursorRow<size.y)
   {
    if (updateFlags & (ufView | ufLine))
       // If the editor was redrawed there is no need to do this
       CrossCursorX2=size.x;
    else
      {
       if (CrossCurInCacheR)
         { // Only if we have the info in the cache
          if (yAnt!=yAct)
            { // If the row was changed make a full erase
             for (i=0; i<CrossCursorX2; i++)
                 setAttrOfCoor(i,CrossCursorRow,CrossCursorBufR[i]);
            }
          else
            {
             // erase only the last col, but only if the value changed
             if (xAct!=xAnt)
                setAttrOfCoor(xAct,yAct,CrossCursorBufR[xAct]);
             updateRow=0;
            }
         }
       else
          CrossCursorX2=size.x;
      }
   }

 CrossCursorCol=xAct;
 CrossCursorRow=yAct;

 // Draw the new Col-cursor
 if (CrossCursorInCol && CrossCursorCol>=0 && CrossCursorCol<size.x)
   {
    int i;

    if (updateCol)
      {
       // If a full erase was done make a full draw
       // First try to copy all in the cache
       if (getAttrsOfCol(CrossCursorCol,0,CrossCursorY2,CrossCursorBuf))
          CrossCurInCacheC=False;
       else
         {
          // If all is in the cache draw the col, but not where the cursor is
          for (i=0; i<CrossCursorY2; i++)
              if (i!=CrossCursorRow)
                 setAttrOfCoor(CrossCursorCol,i,color);
          CrossCurInCacheC=True;
         }
      }
    else
      {
       // If only one character was erased draw only one
       if (getAttrsOfCol(CrossCursorCol,yAnt,yAnt,&CrossCursorBuf[yAnt]))
          CrossCurInCacheC=False;
       else
         {
          if (yAnt!=yAct)
             setAttrOfCoor(CrossCursorCol,yAnt,color);
          CrossCurInCacheC=True;
         }
      }
   }

 // Draw the new Row-cursor
 if (CrossCursorInRow && CrossCursorRow>=0 && CrossCursorRow<size.y)
   {
    int i;

    if (updateRow)
      {
       if (getAttrsOfRow(0,CrossCursorX2,CrossCursorRow,CrossCursorBufR))
          CrossCurInCacheR=False;
       else
         {
          for (i=0; i<CrossCursorX2; i++)
              if (i!=CrossCursorCol)
                 setAttrOfCoor(i,CrossCursorRow,color);
          CrossCurInCacheR=True;
         }
      }
    else
      {
       if (getAttrsOfRow(xAnt,xAnt,CrossCursorRow,&CrossCursorBufR[xAnt]))
          CrossCurInCacheR=False;
       else
         {
          setAttrOfCoor(xAnt,CrossCursorRow,color);
          CrossCurInCacheR=True;
         }
      }
   }
}

/****************************************************************************

   Function: void draw()

   Type: TCEditor member.

   Objetive: Update the content of the window (the text, not the rest).

   by SET.

****************************************************************************/

void TCEditor::draw()
{
 AdjustDrawPtr();
 drawLines( drawLine, size.y, drawPtr );
}

/****************************************************************************

   Function: void AdjustDrawPtr(void)

   Type: TCEditor member.

   Objetive: Update the current pointer for drawing.
     The drawPtr pointer keeps track of the position that correspond to the
   drawLine variable, this variable is the first line drawed by drawLine by
   draw. This value is adjusted each time the delta.Y is modified in the
   window. The calculus is made based on the assumption that the actual
   values: drawLine and drawPtr are OK, some routines can modify the buffer
   invalidating this pair of values, under such situations is necesary to
   make these two values coherents, a method is make the both 0, but then
   this routine must spend more time to recalculate the actual values,
   another way is move these values to a point that will not be modified in
   the routine. To see an example see UnIndentBlock.

   by SET.

****************************************************************************/

void TCEditor::AdjustDrawPtr(void)
{
 unsigned deltaY=delta.y;

 if ( drawLine!=deltaY )
   {
    // Adjust the pointer drawPtr
    if (drawLine>deltaY)
       for (;drawLine>deltaY; drawLine--)
           drawPtr-=lenLines.safeLen(drawLine-1);
    else
       for (;drawLine<deltaY; drawLine++)
           drawPtr+=lenLines.safeLen(drawLine);
   }
}


/****************************************************************************

   Function: uint32 LenWithoutCRLF(uint32 y, char *cl)

   Type: TCEditor member.

   Objetive: Compute the length of a line, but excluding the '\r','\n' at the
   end of the line.

   14/05/97 Mod. to support only '\n' by Robert.

   by SET.

****************************************************************************/

uint32 TCEditor::LenWithoutCRLF(uint32 y, char *cl)
{
 unsigned len=lenLines.safeLen(y);
 /* Remove at first LF */
 if (len>0 && cl[len-1]=='\n') len--;
 /* and now CR on DOS-files */
 if (len>0 && cl[len-1]=='\r') len--;

 return len;
}

/****************************************************************************

   Function: void drawLines( int y, int count, uint32 linePtr )
             
   Type: TCEditor member.

   Objetive: Draw some lines in the screen.

   Parameters:
   y: pos in the file (not the window)
   count: number of lines to draw
   linePtr: offset of the start of the line in the buffer

   by SET.

   ToDo:
   If necesary a best patch for the draw of the line in edition.

****************************************************************************/

void TCEditor::drawLines( int y, int count, uint32 linePtr )
{
 uint16 color = getColor(0x0201);
 unsigned yInFile=y;
 Boolean FirstEmpty=True;
 y-=delta.y;
 unsigned width=delta.x+size.x;
 int IsPostRectOn=0;   // Indicates that there is no need to paint the rect
 char *bc;             // Alias to access to the buffer with chars
 int OffXr1=0,OffXr2=0,Off;
 char ColRect=0;

 if (!colorsCached)
   {
    CacheColors();
    colorsCached=1;
   }
 CacheSyntaxHLData(GenericSHL);

 // Set IsPostRectOn only if really needed
 if (hasRectSel())
   {
    if (Xr1<(int)width && Xr2>=delta.x && Yr1<delta.y+size.y && Yr2>=delta.y)
      {
       IsPostRectOn=1;
       OffXr1=(Xr1<<1)+1;
       OffXr2=((min(Xr2-1,(int)width))<<1)+1;
       ColRect=getColor(cRectSel);
      }
   }

 // temporal buffer, ever 1 more than the width to avoid problems
 ushort *b=(ushort *)alloca((width+tabSize+16+1)<<1);
 //ushort b[maxLineLenBuff]; Old fix-length version.
 if (b==NULL) abort();
 bc=(char *)b;

 while ( count-- > 0 )
    {
     if (yInFile<=totalLines)
       {
        if (IslineInEdition && yInFile==(unsigned)curPos.y)
          {
           char *bb=buffer;
           unsigned s=selStart,e=selEnd;
           unsigned bs=bufLen;
           bufLen=delta.x+size.x;
           selStart=selLineStart;
           selEnd=selLineEnd;
           buffer=bufEdit;
           (this->*formatLinePtr)(b,0,width,color,(unsigned)(inEditPtr+restCharsInLine-bufEdit),
                                  attrInEdit,yInFile,ColumnMarkers ? colMarkers : 0);
           buffer=bb;
           selEnd=e;
           selStart=s;
           bufLen=bs;
          }
        else
           (this->*formatLinePtr)(b,linePtr,width,color,
                                  LenWithoutCRLF(yInFile,buffer+linePtr),
                                  lenLines.getAttr(yInFile),yInFile,
                                  ColumnMarkers ? colMarkers : 0);

        if (IsPostRectOn)
          {
           if (yInFile>=(unsigned)Yr1 && yInFile<=(unsigned)Yr2)
              for (Off=OffXr1; Off<=OffXr2; Off+=2)
                  bc[Off]=ColRect;
          }

       {/* Paint breakpoint and CPU lines. */
        int i,off,j, cnt;
        if (SpecialLines)
          {
           cnt=SpecialLines->getCount();
           for (i=0; i<cnt; i++)
              {
               stSpLine *p=SpecialLines->At(i);
               if (p->nline==(int)yInFile)
                 {
                  int color=0;
                  if (p->id==idsplBreak)
                     color=getColor(cBreak) & 0xF0;
                  else if (p->id==idsplRunLine)
                     color=getColor(cCPU) & 0xF0;
                  if (color)
                    {
                     int colAvoid=color>>4, colFg;

                     for (off=delta.x, j=size.x; j; off++,j--)
                        {
                         colFg=bc[off*2+1] & 0xF;
                         if (colFg==colAvoid)
                            colFg=(colFg+1) & 0xF;
                         bc[off*2+1]=colFg | color;
                        }
                     break;
                    }
                 }
              }
          }
       }

        writeLine(0,y, size.x, 1, &b[delta.x]);
        // Adjust the pointer linePtr
        linePtr = linePtr+lenLines[yInFile++];
       }
     else
       { // Empty lines
        if (FirstEmpty)
          { // If is the first make the empty line
           FirstEmpty=False;
           ushort Val;
           ((char *)&Val)[0]=0x20;
           ((char *)&Val)[1]=color;
           for (int i=size.x; i;)
               b[--i]=Val;
          }
        writeLine(0,y, size.x, 1, b);
        yInFile++;
       }
     y++;
    }
}


/**[txh]********************************************************************

  Description:
  Puts text in the status line removing tabs and stoping in \r or \n. The
buffer is altered. The string must be ASCIIZ.

***************************************************************************/

void TCEditor::setStatusLine(char *s)
{
 int l;
 for (l=0; s[l] && s[l]!='\n' && s[l]!='\r'; l++)
    {
     if (s[l]=='\t')
        s[l]=' ';
    }
 char color=getColor(cStatusLi);

 if (l>=setMaxScreenX)
    l=setMaxScreenX-1;

 int fill=setMaxScreenX-1-l;
 char *b=StatusLine;

 while (l--)
   {
    *(b++)=*(s++);
    *(b++)=color;
   }

 while (fill--)
   {
    *(b++)=' ';
    *(b++)=color;
   }

 update(ufStatus);
}


static unsigned PipeOrigin;
static char    *PipeBuf;
static unsigned PipeBufLen;

int PipeTCEditor(unsigned PosRel)
{
 if (PosRel+PipeOrigin<PipeBufLen)
    return PipeBuf[PosRel+PipeOrigin];
 return -1;
}

/****************************************************************************

   Function: void find()

   Type: TCEditor member.

   Objetive: Make the dialog and search in the text.

   by SET.

****************************************************************************/

void TCEditor::find()
{
 char *Word;

 if ((Word=WordUnderCursor(80))!=NULL)
   {
    strcpy(findStr,Word);
    delete[] Word;
   }

 editorFlags&=~efOptimizedRegex;
 TFindCDialogRec findRec(findStr,editorFlags,SearchInSel,FromWhere);
 TRegexDialogRec regexRec;
 regexRecCreate(regexRec);

 if (editorDialog(edFind,&findRec,&regexRec)!=cmCancel)
   {
    regexRecUpdate(regexRec);
    strcpy( findStr, findRec.find );
    editorFlags = findRec.options & ~efDoReplace;
    SearchInSel = findRec.in_sel;
    FromWhere = findRec.from;
    if (FromWhere)
       StartOfSearch=0; // All
    else
       StartOfSearch=(unsigned)(ColToPointer()-buffer);
    if (CompileSearch(findStr))
       return;
    doSearchReplace();
   }
}

/**[txh]********************************************************************

  Description:
  Gets the word under the cursor position. You must specify the maximun
length of the returned string.@p
  The routine supports the pipe feature to connect the word with an input
line object.

  Return:
  A pointer to the string (a new allocated one) or NULL if the cursor isn't
over a word.

***************************************************************************/


char *TCEditor::WordUnderCursor(uint32 maxLength, unsigned options)
{
 char *word,*aux;
 char *wordStart,*wordEnd,*i;
 unsigned l;

 if (IslineInEdition)
    MakeEfectiveLineInEdition();

 char *s=ColToPointer();
 char *end=buffer+bufLen;

 // Set-Up the pipe
 PipeOrigin=(unsigned)(s-buffer);
 PipeBuf=buffer;
 PipeBufLen=bufLen;

 if (options & wucCanStartColon)
   {
    if (!isWordCharColon(*s))
      {
       if ((options & wucTakeOneLeft) && s>buffer)
          s--;
       if (!isWordCharColon(*s))
          return NULL;
      }
   }
 else
   {
    if (!isWordChar(*s))
      {
       if ((options & wucTakeOneLeft) && s>buffer)
          s--;
       if (!isWordChar(*s))
          return NULL;
      }
   }

 if (options & wucIncludeColon)
   { // For C++ members
    wordStart=s;
    while (--wordStart>buffer && isWordCharColon(*wordStart));
    if (wordStart!=buffer || !isWordCharColon(*wordStart))
       wordStart++;
   
    wordEnd=s;
    while (++wordEnd<end && isWordCharColon(*wordEnd));
    if (wordEnd!=end || !isWordCharColon(*wordEnd))
       wordEnd--;
   }
 else
   {
    wordStart = s;
    while (--wordStart>buffer && isWordChar(*wordStart));
    if (wordStart!=buffer || !isWordChar(*wordStart))
       wordStart++;
   
    wordEnd = s;
    while (++wordEnd<end && isWordChar(*wordEnd));
    if (wordEnd!=end || !isWordChar(*wordEnd))
       wordEnd--;
   }

 // Adjust the pipe
 PipeOrigin=(unsigned)(wordStart-buffer);

 l=(unsigned)(wordEnd-wordStart+2);
 if (l>maxLength)
    return NULL;
 word=new char[l];

 for (i=wordStart,aux=word; i<=wordEnd; i++)
     *aux++ = *i;
 *aux=0;

 return word;
}

/**[txh]********************************************************************

  Description:
  Select the word under cursor.
  
***************************************************************************/

void TCEditor::SelWordUnderCursor(void)
{
 char *p=ColToPointer();
 char *end=buffer+bufLen;

 if (isWordChar(*p))
   {
    // Walk backward to the start of a word
    for (;p>buffer && isWordChar(*p); --p);
    if (p!=buffer) p++;
   }
 else
   {
    if (0) // This is the original code. It ever takes the next word.
      {
       // if isn't in a word walk forward
       for (;p<end && !isWordChar(*p); ++p);
       if (!isWordChar(*p))
          return;
      }
    else
      { // This code tries to be a little bit smarter and takes the closest word.
       unsigned distF, distB;
       char *forward=p, *back=p;
       // Search forward
       for (;forward<end && !isWordChar(*forward); ++forward);
       if (isWordChar(*forward))
          distF=forward-p;
       else
          distF=bufLen+1;
       // Search backward
       for (;back>buffer && !isWordChar(*back); --back);
       if (isWordChar(*back))
          distB=p-back;
       else
          distB=bufLen+1;
       for (;back>buffer && isWordChar(*back); --back);
       if (back!=buffer) back++;
       // Take one
       if (distF==distB && distF==bufLen+1)
          return;
       if (distF<=distB)
          p=forward;
       else
          p=back;
      }
   }
 selStartOffSet=selStart=(uint32)(p-buffer);
 // Now forward to the end
 for (;p<end && isWordChar(*p); ++p);
 selEnd=(uint32)(p-buffer);
}

/****************************************************************************

   Function: void MoveToMouse( TPoint m, uchar selMode )

   Type: TCEditor member.

   Objetive: Move the cursor to the position pointed by the mouse updating
   the selection if necesary.
     Handle the double click to select the word under cursor.

   Parameter:                             
   TPoint m: The point where the mouse is.
   uchar selMode: The flags for the selection mode.

   by SET.

****************************************************************************/

void TCEditor::MoveToMouse( TPoint m, uchar selMode )
{
 TPoint mouse = makeLocal( m );
 mouse.x = max(0, min(mouse.x, size.x - 1));
 mouse.y = max(0, min(mouse.y, size.y - 1));

 MoveCursorTo(mouse.x+delta.x,mouse.y+delta.y);
 if (selMode & smDontSel)
   {
    update(ufUpdate);
    return;
   }
 if (selMode & smExtend)
   { // Extends the selection
    unsigned selAux=(unsigned)(ColToPointer()-buffer); // To where?
    if (selAux>bufLen)
       selAux=bufLen;
    if (selAux>selStartOffSet)
      {
       selEnd=selAux;
       selStart=selStartOffSet;
      }
    else
      {
       selEnd=selStartOffSet;
       selStart=selAux;
      }
   }
 else
   { // Starts a selection
    if (selMode & smDouble)
      { // Select the word under cursor or the nearest
       SelWordUnderCursor();
      }
    else
      {
       int dif;
       selStart=(uint32)(ColToPointer(dif)-buffer);
       if (dif>0)
          selStart--;
       if (selStart>bufLen)
          selStart=bufLen;
       selEnd=selStartOffSet=selStart;
      }
    selHided=False;
   }
 update(ufView);
}

/**[txh]********************************************************************

  Description:
  Return the palette for the editor. Currently that's a palette created by
Robert that have space for all the colors used by the editor.
  
  Return: a reference to the palette.
  
***************************************************************************/

TPalette& TCEditor::getPalette() const
{
 static TPalette palette(cpEditor,sizeof(cpEditor)-1);
 return palette;
}

/****************************************************************************

   Function: void checkScrollBar( const TEvent& event, TScrollBar *p,
                                  int& d )
             
   Type: TCEditor member.

   Objetive: ? Check if the event is a change in the Scroll bars.

   From Borland's TV 1.03.

****************************************************************************/

void TCEditor::checkScrollBar( const TEvent& event,
                              TScrollBar *p,
                              int& d
                            )
{
 if ( (event.message.infoPtr == p) && (p->value != d) )
   {
    d = p->value;
    if (IslineInEdition)
       MakeEfectiveLineInEdition();
    update( ufView );
   }
}

/****************************************************************************

   Function: int IsFirstCharInLine(void)
             
   Type: TCEditor member.

   Objetive: Check if the cursor is over the first non-blanc char in the
   line.
     Works when a line is in edition and when isn't.

   Return: 0 not OK.

   by SET

****************************************************************************/

int TCEditor::IsFirstCharInLine(void)
{
 if (IslineInEdition)
   {
    char *s=bufEdit;

    for (;*s && s!=inEditPtr && ucisspace(*s); s++);
    return s==inEditPtr;
   }
 char *s=curLinePtr;
 uint32 l=LenWithoutCRLF(curPos.y,curLinePtr),x,xpos=curPos.x,i;

 for (x=0,i=0; x<xpos && i<l && ucisspace(*s); s++,i++)
    { AdvanceWithTab(*s,x); }
 return x==xpos;
}

/****************************************************************************

   Function: int GoFirstCharInLine(void)

   Type: TCEditor member.

   Objetive: Put the cursor on the first non-blank character in the line, if
   exists.

   Return: 0 not OK.

   by SET

****************************************************************************/

int TCEditor::GoFirstCharInLine(void)
{
 char *s;
 uint32 l,x,i;

 if (IslineInEdition)
   {
    s=bufEdit;
    l=(uint32)(inEditPtr-bufEdit+restCharsInLine);
   }
 else
   {
    s=curLinePtr;
    l=LenWithoutCRLF(curPos.y,curLinePtr);
   }

 for (x=0,i=0; i<l && ucisspace(*s); s++,i++)
    { AdvanceWithTab(*s,x); }
 if (i<l)
   {
    curPos.x=x;
    update(ufUpdate);
    return 1;
   }
 return 0;
}

/****************************************************************************

   Function: void handleEvent( TEvent& event )

   Type: TCEditor member.

   Objetive: Is the main switch/case of the class.

   by SET

   ToDo: Put in separated routines the cases.

****************************************************************************/

void TCEditor::handleEvent( TEvent& event )
{
 TView::handleEvent(event);
 switch(event.what)
    {
     case evMouseDown:
          handleMouse(event);
          break;

     case evKeyDown:
          handleKey(event);
          break;

     case evCommand:
          if (handleCommand(event.message.command))
             clearEvent(event);
          break;

     case evBroadcast:
          switch(event.message.command)
            {
             case cmScrollBarChanged:
                  checkScrollBar(event,hScrollBar,delta.x);
                  checkScrollBar(event,vScrollBar,delta.y);
                  break;
     
             case cmcSetGlobalOptions:
                  ExpandGlobalOptionsLocally((GlobalOptionsRect *)event.message.infoPtr);
                  return;

             case cmUpdateColorsChanged:
             case cmcColorsChanged:
                  CacheColors();
                  break;

             default:
                  return;
            }
          clearEvent(event);
          break;
    }
}


void TCEditor::handleKey(TEvent &event)
{
 // Hack: disassembler windows must work as a widget
 if (isDisassemblerEditor && owner && owner->owner &&
     event.keyDown.keyCode==kbTab)
   {    
    owner->owner->selectNext(True);
    clearEvent(event);
    return;
   }
 // First translate it to a command
 KeyTNode node;
 int ret=KeyTrans.get(event.keyDown.keyCode,&node);
 
 if (!ret)
   { // This key haven't any assigment
    unsigned char Character;
    Character=event.keyDown.charScan.charCode;
    // Filter strange characters
    //if (Character=='\t' || (Character>=32 && Character<255))
    if (event.keyDown.keyCode==kbTab)
       Character='\t';
    if (!isReadOnly && Character && Character!='\n' && Character!='\r')
      {
       lock();
       CutIfNotPersistent();
       //addToUndo(undoPutChar,(void *)&Character);
       if (!IslineInEdition)
          EditLine();
       if (IslineInEdition)
         {
          InsertCharInLine(Character);
          if (Recording)
             MacroArray[MacroCount++]=0xF0000+Character;
          update(ufLine);
          if (ShowMatchPairFly && ShowMatchPairNow)
             SearchMatchOnTheFly();
         }
       unlock();
      }
    else
      return;
   }
 else
 if (ret>0)
   {
    KeyTSeq *se;
    int i;
    switch (node.flags)
      {
       case kbtIsComm:
            handleCommand(node.d.command);
            break;
       case kbtIsMacro:
            lock();
            SLPSearchMacro(this,node.d.macro,False);
            unlock();
            break;
       case kbtIsSeq:
            se=node.d.sequence;
            for (i=0; i<se->cant; i++)
                handleCommand(se->commands[i]);
            break;
      }
   }
 // r<0 is part of a sequence so just clear the event
 clearEvent(event);
}

void TCEditor::handleMouse(TEvent &event)
{
 // Mouse events, Full Ok Level 1
 uchar selectMode=0;

 flushLine();
 if (RightClickMenu && event.mouse.buttons==mbRightButton)
   {// Move the cursor so WordUnderCursor can get the pointed word
    MoveToMouse(event.mouse.where,smDontSel);
    // Pop-up the box
    TRect dkt=TProgram::deskTop->getExtent();
    TMenuBox *mbox=new TMenuBox(TRect(event.mouse.where.x-2,event.mouse.where.y-2,
                                dkt.b.x,dkt.b.y),RightClickMenu->subMenu,0);
    unsigned command=TProgram::deskTop->execView(mbox);
    CLY_destroy(mbox);
    if (command)
       message(TProgram::application,evCommand,command,0);
    return;
   }

 if (event.mouse.buttons==mbMiddleButton && TVOSClipboard::isAvailable()>1)
   {// Uncommenting the next line the text is pasted at the mouse position.
    // It looks a good idea, but isn't good in practice.
    //MoveToMouse(event.mouse.where,smDontSel);
    clipWinPaste(1);
    return;
   }

 if (event.mouse.buttons==mbButton4)
   {
    ScrollLinesUp(5);
    return;
   }

 if (event.mouse.buttons==mbButton5)
   {
    ScrollLinesDown(5);
    return;
   }

 if (event.mouse.doubleClick)
    selectMode|=smDouble;
 do
   {
    lock();
    if (event.what==evMouseAuto)
      {
       TPoint mouse = makeLocal( event.mouse.where );
       TPoint d = delta;
       if (mouse.x<0)
          d.x--;
       if (mouse.x>=size.x)
          d.x++;
       if (mouse.y<0)
          d.y--;
       if (mouse.y>=size.y)
          d.y++;
       scrollTo(d.x, d.y);
      }
    MoveToMouse(event.mouse.where,selectMode);
    //setCurPtr(getMousePtr(event.mouse.where), selectMode);
    selectMode|=smExtend;
    if (ShowMatchPairFly && ShowMatchPairNow)
       SearchMatchOnTheFly();
    unlock();
   }
 while (mouseEvent(event,evMouseMove+evMouseAuto));
 clearEvent(event);
 if (TVOSClipboard::isAvailable()>1)
    clipWinCopy(1);
}

void TCEditor::ScrollLinesUp(int lines)
{
 CheckForShiftSelection();
 addToUndo(undoInMov);
 MoveLinesUp(lines);
 delta.y=max(delta.y-lines,0);
 if (NoInsideTabs)
    curPos.x=FixPosCharLeft();
 update(ufView);
}

void TCEditor::ScrollLinesDown(int lines)
{
 CheckForShiftSelection();
 addToUndo(undoInMov);
 MoveLinesDown(lines);
 delta.y=min(delta.y+lines,limit.y);
 if (NoInsideTabs)
    curPos.x=FixPosCharLeft();
 update(ufView);
}

/**[txh]********************************************************************

  Description:
  Used to show something in the status line from a function that knows
nothing about TCEditors.
  
***************************************************************************/

static
void CallBackStatusLine(const char *msg, void *obj)
{
 TCEditor *e=(TCEditor *)obj;
 e->setStatusLine((char *)msg);
}

int TCEditor::handleCommand(ushort command)
{
 Boolean centerCursor=(!cursorVisible()) ? True : False;
 int i; // To be used as iterator in any of the case in the switch
 int cursorMoved=0;
 TPoint oldCurPos;

 /* Note: Most of the routines check isReadOnly to avoid problems if in some
    way the command is processed without using this routine.
    Only the routines made in the case need the test.
    The check is very redundant */
 switch (command)
   {
    // First commands without locking, I must resee it
    case cmcFind:
         flushLine();
         lastCurPos=curPos;
         find();
         updateCommands(); // In case the user copied/pasted
         break;
  
    case cmcReplace:
         flushLine();
         lastCurPos=curPos;
         replace();
         updateCommands(); // In case the user copied/pasted
         break;
  
    // ^L
    case cmcSearchAgain:
         flushLine();
         lastCurPos=curPos;
         StartOfSearch=(unsigned)(ColToPointer()-buffer)+1;
         doSearchReplace();
         break;

    case cmcForceMatchPairHL:
         SearchMatchOnTheFly();
         break;
 
    default:
        lock();
        lockUndo();
        oldCurPos=curPos;
        switch(command)
          {
           case cmcSelectOn:
                flushLine();
                SetStartOfSelecting((uint32)(ColToPointerPost()-buffer));
                break;
       
           case cmcSelectOff:
                selecting=False;
                break;
         
           // ^KY or ShiftDel Full Level 2
           case cmcCut:
                clipCut();
                break;
 
           // Full Level 2
           case cmcCopy:
                clipCopy();
                break;
 
           // Full Level 2
           case cmcPaste:
                if (PersistentBlocks)
                   clipPaste();
                else
                  {
                   clipReplace();
                   selEnd=selStart;
                  }
                break;
 
           // Alt+Backspace
           case cmcUndo:
                undo();
                break;
 
           // No key
           case cmcRedo:
                redo();
                break;
 
           // Ctrl+Del, Full Level 2
           case cmcClear:
                flushLine();
                deleteSelect();
                break;
 
           // ^KC
           case cmcCopyBlock:
                if (isReadOnly) break;
                if (PersistentBlocks && hasSelection())
                  {
                   if (!selHided)
                     {
                      flushLine();
                      char *s;
                      unsigned l=selEnd-selStart;
                      s=new char[l];
                      if (s)
                        {
                         memcpy(s,buffer+selStart,l);
                         insertBuffer(s,0,l,True,True,False);
                         delete s;
                        }
                     }
                   else
                     {
                      selHided=False;
                      update(ufView);
                     }
                  }
                break;
 
           // Full Ok level 2 + selExtend
           case cmcCharLeft:
                if (curPos.x>0)
                  {
                   CheckForShiftSelection();
                   addToUndo(undoInMov);
                   if (IslineInEdition)
                     {
                      curPos.x--;
                      if (*inEditPtr=='\t')
                        {
                         if (curPos.x<LineWidth(bufEdit,inEditPtr))
                           {
                            inEditPtr--;
                            restCharsInLine++;
                           }
                        }
                      else // if (tab)
                        {
                         inEditPtr--;
                         restCharsInLine++;
                        }
                      if (NoInsideTabs && *inEditPtr=='\t')
                         curPos.x=LineWidth(bufEdit,inEditPtr);
                      ClearSelIfNonPers();
                     }
                   else // if (Line in edition)
                     {
                      if (NoInsideTabs)
                         curPos.x=PosLeftChar();
                      else
                         curPos.x--;
                      cursorMoved=1;
                     }
                   update(ufUpdate);
                  }
                break;
 
           // Full Ok level 2 + selExtend
           case cmcCharRight:
                if (curPos.x<MaxLineLen)
                  {
                   CheckForShiftSelection();
                   if (IslineInEdition)
                     {
                      addToUndo(undoInMov);
                      if (*inEditPtr)
                        {
                         curPos.x++;
                         if (*inEditPtr=='\t')
                           {
                            if (NoInsideTabs)
                              {
                               inEditPtr++;
                               restCharsInLine--;
                               curPos.x=LineWidth(bufEdit,inEditPtr);
                              }
                            else // if (NoInsideTabs)
                              {
                               if (IsATabPos(curPos.x))
                                 {
                                  inEditPtr++;
                                  restCharsInLine--;
                                 }
                              }
                           }
                         else // if (*inEditPtr=='\t')
                           {
                            inEditPtr++;
                            restCharsInLine--;
                           }
                         update(ufUpdate);
                        }
                      else
                        {
                         InsertCharInLine(32);
                         restCharsInLine=0;
                         update(ufLine);
                        }
                      ClearSelIfNonPers();
                     }
                   else // if (IslineInEdition)
                     {
                      addToUndo(undoInMov);
                      curPos.x++;
                      if (NoInsideTabs)
                        {
                         int dif;
                         ColToPointer(dif);
                         if (dif>0)
                            curPos.x+=dif;
                        }
                      cursorMoved=1;
                     }
                  }
                break;
 
           // Full OK Level 2 + selExtend
           case cmcWordLeft:
                if (curPos.y>0 || curPos.x>0)
                  {
                   CheckForShiftSelection();
                   if (IslineInEdition) // This can be optimized
                      MakeEfectiveLineInEdition();
                   addToUndo(undoInMov);
                   prevWord();
                   cursorMoved=1;
                  }
                break;
 
           // Full OK Level 2 + selExtend
           case cmcWordRight:
                CheckForShiftSelection();
                if (IslineInEdition) // This can be optimized
                   MakeEfectiveLineInEdition();
                addToUndo(undoInMov);
                nextWord();
                cursorMoved=1;
                break;

           case cmcGoEndOfWord:
                cursorMoved=goEndWord();
                break;
 
           // Full OK Level 2 + selExtend
           case cmcLineStart:
                CheckForShiftSelection();
                addToUndo(undoInMov);
                if (IslineInEdition)
                  {
                   restCharsInLine+=(int)(inEditPtr-bufEdit);
                   inEditPtr=bufEdit;
                   // We purge spaces so we should show it now if tabs are involved
                   // and visible.
                   if (SeeTabs)
                     {
                      int tabs=0;
                      char *s,*e;
                      for (s=bufEdit, e=bufEdit+restCharsInLine; s<e && isspace(*s); s++)
                          if (*s=='\t') tabs++;
                      if (s==e && tabs)
                        {
                         restCharsInLine=0;
                         *inEditPtr=0;
                         update(ufLine);
                        }
                     }
                  }
                curPos.x=0;
                cursorMoved=1;
                break;
 
           // Full OK Level 2 + selExtend
           case cmcLineOrScrEnd: // It moves to the last visible column (g-end in VI)
           case cmcLineEnd:
               {
                CheckForShiftSelection();
                addToUndo(undoInMov);
                int len=0;
                i=0;
                if (IslineInEdition)
                  {
                   if (DontPurgeSpaces)
                     { // Real end of line
                      len=curPos.x;
                      while (*inEditPtr)
                        {
                         AdvanceWithTab(*inEditPtr,len);
                         inEditPtr++;
                        }
                      restCharsInLine=0;
                     }
                   else
                     { // Calculate the "visible" length of the line and purge the rest
                      char *auxPos=bufEdit;
                      char *lastUsedPos=auxPos;
                      int  lastUsedX=0;
                      while (*auxPos)
                         {
                          AdvanceWithTab(*auxPos,len);
                          if (!ucisspace(*auxPos))
                            {
                             lastUsedPos=auxPos;
                             lastUsedX=len;
                            }
                          auxPos++;
                         }
                      // Purge any spaces at the end of the line
                      restCharsInLine=0;
                      if (!ucisspace(*lastUsedPos))
                         lastUsedPos++;
                      inEditPtr=lastUsedPos;
                      *lastUsedPos=0;
                      len=lastUsedX;
                      // If the user is looking tabs perhaps we purged some so update it
                      if (SeeTabs)
                         update(ufLine);
                     }
                  }
                else
                  {
                   int c=LenWithoutCRLF(curPos.y,curLinePtr);
                   while (c--)
                      {
                       AdvanceWithTab(curLinePtr[i],len);
                       i++;
                      }
                  }
                if (command==cmcLineOrScrEnd && len>delta.x+size.x-1)
                   curPos.x=delta.x+size.x-1;
                else
                   curPos.x=len;
                cursorMoved=1;
               }
                break;
 
           // Full OK level 2 + selExtend
           case cmcLineUp:
               if (curPos.y>0)
                 {
                  CheckForShiftSelection();
                  addToUndo(undoInMov);
                  if (IslineInEdition)
                     MakeEfectiveLineInEdition();
                  curLinePtr-=lenLines[--curPos.y];
                  if (NoInsideTabs)
                     curPos.x=FixPosCharLeft();
                  cursorMoved=1;
                 }
               break;
 
           // Full OK level 2 + selExtend
           case cmcLineDown:
               if ((unsigned)curPos.y<totalLines)
                 {
                  CheckForShiftSelection();
                  addToUndo(undoInMov);
                  if (IslineInEdition)
                     MakeEfectiveLineInEdition();
                  curLinePtr+=lenLines[curPos.y++];
                  if (NoInsideTabs)
                     curPos.x=FixPosCharLeft();
                  cursorMoved=1;
                 }
               break;
 
           // Full Ok level 2 (Pg Up) + selExtend
           case cmcPageUp:
                ScrollLinesUp(size.y-1);
                cursorMoved=1;
                break;
 
           // Full Ok level 1 (Pg Down) + selExtend
           case cmcPageDown:
                ScrollLinesDown(size.y-1);
                cursorMoved=1;
                break;
 
           // Full Ok level 2 (^Pg Up) + selExtend
           case cmcTextStart:
                CheckForShiftSelection();
                addToUndo(undoInMov);
                if (IslineInEdition)
                   MakeEfectiveLineInEdition();
                curPos.x=0;
                curPos.y=0;
                curLinePtr=buffer;
                cursorMoved=1;
                break;
 
           // Full Ok level 2 (^Pg Down) + selExtend
           case cmcTextEnd:
                JumpEndOfText();
                cursorMoved=1;
                break;
 
           // Full OK level 2
           case cmcNewLine:
                newLine();
                break;
 
           // Full Ok level 2 (Backspace)
           case cmcBackSpace:
                BackSpace();
                break;
 
 
           // Full Ok level 2 (Delete)
           case cmcDelCharClear:
           case cmcDelChar:
                if (isReadOnly)
                   break;
                showMatchPairFlyCache=NULL;
                if (!PersistentBlocks && hasSelection())
                  {
                   if (command==cmcDelChar)
                      clipCut();
                   else
                     { // For people that wants clear
                      flushLine();
                      deleteSelect();
                     }
                   break;
                  }
                if (IslineInEdition && restCharsInLine)
                  { // easy
                   addToUndo(undoDelCharDel,inEditPtr);
                   int wasATab=*inEditPtr=='\t';
                   memcpy(inEditPtr,inEditPtr+1,restCharsInLine);
                   if (wasATab)
                      RecalculateXofLineInEdit();
                   restCharsInLine--;
                   MarkAsModified();
                   update(ufLine);
 
                   unsigned PosOfIns=(unsigned)(inEditPtr-bufEdit);
                   AdjustLineSel(PosOfIns,-1,False,False);
 
                   // Update the markers
                   for (i=0; i<10; i++)
                      {
                       int Pos=MarkersInLine[i];
                       if (Pos>=0 && (unsigned)Pos>PosOfIns)
                          MarkersInLine[i]--;
                      }
                  }
                else
                  {
                   // Could be optimized
                   if (IslineInEdition)
                      MakeEfectiveLineInEdition();
 
                   // 2 cases: 1) in the line 2) outside
                   int x=LineWidth();
                   if (x>curPos.x)
                     { // OK in the line
                      EditLine();
                      if (!IslineInEdition)
                         break;
                      addToUndo(undoDelCharDel,inEditPtr);
                      int wasATab=*inEditPtr=='\t';
                      memcpy(inEditPtr,inEditPtr+1,restCharsInLine);
                      if (wasATab)
                         RecalculateXofLineInEdit();
                      restCharsInLine--;
                      MarkAsModified();
                      update(ufLine);
                      unsigned PosOfIns=(unsigned)(inEditPtr-bufEdit);
                      AdjustLineSel(PosOfIns,-1,False,False);
                      // Update the markers
                      for (i=0; i<10; i++)
                         {
                          int Pos=MarkersInLine[i];
                          if (Pos>=0 && (unsigned)Pos>PosOfIns)
                             MarkersInLine[i]--;
                         }
                     }
                   else
                     { // No!!!
                      if ((unsigned)curPos.y<totalLines)
                        {
                         // Expand the line
                         int ToInsert=curPos.x-x;
                         //curPos.x=x;
                         // delete the CR-LF
                         deleteRange(curLinePtr+LenWithoutCRLF(curPos.y,curLinePtr),curLinePtr+lenLines[curPos.y]);
                         if (ToInsert)
                            insertSpaces(ToInsert,x);
                         // To clean extra spaces if exist
                         EditLine();
                        }
                     }
                  }
                break;
 
           // Full Ok level 2 (^T)
           case cmcDelWord:
                {
                 if (isReadOnly)
                    break;
                 ClearSelIfNonPers();
                 if (IslineInEdition)
                   {
                    if (restCharsInLine==0) // If is the end of line
                       MakeEfectiveLineInEdition();
                    else
                      {
                       char *p=inEditPtr;
 
                       if (!ucisspace(*p))
                         {
                          if (isWordChar(*p))
                             while (*p && isWordChar(*p)) p++;
                          else
                             p++;
                         }
                       /*if (!isWordChar(*p))
                          while ( *p &&  !isWordChar(*p) ) p++;
                       else
                          while ( *p &&  isWordChar(*p) ) p++;*/
                       while (*p && ucisspace(*p)) p++;
                       restCharsInLine-=(int)(p-inEditPtr);
                       deleteRangeLineInEdition(inEditPtr,p,-1);
                       break;
                      }
                   }
                 int dif;
                 char *s=ColToPointer(dif);
                 addToUndo(undoPreDelete);
                 deleteRange(s,s+nextCWord());
                 if (dif<0)
                    insertSpaces(-dif,curPos.x);
                }
               break;
 
           // ^BackSpace, Full Ok level 2
           case cmcDelPrevWord:
               {
                if (isReadOnly) break;
                ClearSelIfNonPers();
                if (IslineInEdition)
                   MakeEfectiveLineInEdition();
                char *s=ColToPointer();
                deleteRange(s+prevWord(False),s);
               }
                break;
 
           // Full Ok level 2 (^QH)
           case cmcDelStart:
                if (isReadOnly) break;
                ClearSelIfNonPers();
                if (IslineInEdition)
                   deleteRangeLineInEdition(bufEdit,inEditPtr,0);
                else
                   deleteRange(curLinePtr,ColToPointer());
                break;
 
           // ^QM
           case cmcChooseMacro:
                flushLine();
                SLPInterfaceRun(this);
                break;
 
           case cmcRepeatMacro:
                flushLine();
                SLPInterfaceReRun(this);
                break;
 
           // Full Ok level 2 (^QY)
           case cmcDelEnd:
                if (isReadOnly) break;
                ClearSelIfNonPers();
                if (IslineInEdition)
                  {
                   deleteRangeLineInEdition(inEditPtr,inEditPtr+restCharsInLine,-1);
                   restCharsInLine=0;
                  }
                else
                  {
                   char *s=ColToPointer();
                   if (CLY_IsntEOL(*s))
                      deleteRange(s,curLinePtr+LenWithoutCRLF(curPos.y,curLinePtr));
                  }
                break;
 
           // Full Ok level 2 (^Y)
           case cmcDelLine:
                if (isReadOnly) break;
                ClearSelIfNonPers();
                if (IslineInEdition)
                   MakeEfectiveLineInEdition();
                if ((uint32)curPos.y!=totalLines)
                   deleteRange(curLinePtr,curLinePtr+lenLines[curPos.y]);
                else
                   deleteRange(curLinePtr,curLinePtr+LenWithoutCRLF(curPos.y,curLinePtr));
                break;
 
           // ^KB, OK Level 2
           case cmcStartSelect:
                addToUndo(undoCutInMov,NULL);
                if (IslineInEdition)
                  {
                   uint32 posL=(uint32)(curLinePtr-buffer);
                   selLineStart=(uint32)(inEditPtr-bufEdit);
                   selNewStart=posL+selLineStart;
                   selStart=posL;
                  }
                else
                   selStart=(uint32)(ColToPointer()-buffer);
                selHided=False;
                update(ufView);
                break;
 
           // ^KK, OK Level 2
           case cmcEndSelect:
                if (IslineInEdition)
                  {
                   uint32 posL=(uint32)(curLinePtr-buffer);
                   selLineEnd=(uint32)(inEditPtr-bufEdit);
                   selNewEnd=posL+selLineEnd;
                   selEnd=posL;
                  }
                else
                   selEnd=(uint32)(ColToPointer()-buffer);
                selHided=False;
                update(ufView);
                addToUndo(undoCutInMov,NULL);
                break;
 
           // ^KH Full Ok Level 2
           case cmcHideSelect:
                addToUndo(undoCutInMov,NULL);
                hideSelect();
                break;
 
           // New ^Home Full Ok Level 2 + selExtend
           case cmcFirstLineInScreen:
                CheckForShiftSelection();
                addToUndo(undoInMov);
                MoveLinesUp(curPos.y-delta.y);
                if (NoInsideTabs)
                   curPos.x=FixPosCharLeft();
                cursorMoved=1;
                break;
 
           // New ^End Full Ok Level 2 + selExtend
           case cmcLastLineInScreen:
                CheckForShiftSelection();
                addToUndo(undoInMov);
                MoveLinesDown(delta.y+size.y-curPos.y-1);
                if (NoInsideTabs)
                   curPos.x=FixPosCharLeft();
                cursorMoved=1;
                break;

           // It moves to the last visible column
           case cmcLastColInScreen:
                i=delta.x+size.x-1;
                if (curPos.x<i)
                  {
                   flushLine()
                   CheckForShiftSelection();
                   addToUndo(undoInMov);
                   curPos.x=i;
                   cursorMoved=1;
                  }
                break;
 
           // ^O
           case cmcIndentMode:
                autoIndent = (!autoIndent) ? True : False;
                break;
 
           case cmcIntelIndentMode:
                intelIndent=(intelIndent) ? False : True;
                break;
 
           // ^V
           case cmcInsMode:
                toggleInsMode(True);
                break;
 
           // ^KI Full Ok level 2
           case cmcIndentBlkOne:
                IndentBlock(" ");
                break;
 
           // ^KU Full Ok level 2
           case cmcUnIndentBlkOne:
                UnIndentBlock(1);
                break;
 
           // ^KTab Full Ok level 2
           case cmcIndentBlk:
                if (UseTabs)
                   IndentBlock("\t");
                else
                   IndentBlock(0);
                break;
 
           // ^KShiftTab Full Ok level 2
           case cmcUnIndentBlk:
                if (UseTabs)
                   UnIndentBlock(1);
                else
                   UnIndentBlock(0);
                break;

           case cmcArbitraryIndent:
                ArbitraryIndent();
                break;

           case cmcCommentIndent:
                CommentIndent();
                break;

           case cmcCommentUnIndent:
                CommentUnIndent();
                break;
 
           // ^ShiftIns Full Ok level 2
           case cmcReplaceSelect:
                clipReplace();
                break;
 
           // ^Kn Without Undo
           case cmcPutMark0:
           case cmcPutMark1:
           case cmcPutMark2:
           case cmcPutMark3:
           case cmcPutMark4:
           case cmcPutMark5:
           case cmcPutMark6:
           case cmcPutMark7:
           case cmcPutMark8:
           case cmcPutMark9:
                SetMarker(command-cmcPutMark0);
                break;
 
           // ^Qn
           case cmcGotoMark0:
           case cmcGotoMark1:
           case cmcGotoMark2:
           case cmcGotoMark3:
           case cmcGotoMark4:
           case cmcGotoMark5:
           case cmcGotoMark6:
           case cmcGotoMark7:
           case cmcGotoMark8:
           case cmcGotoMark9:
                ClearSelIfNonPers();
                addToUndo(undoInMov);
                flushLine();
                GotoOffSet(Markers[command-cmcGotoMark0]);
                update(ufView);
                centerCursor=True;
                break;

           case cmcPushCursorPos:
                PushCursorPosition();
                break;

           case cmcPopCursorPos:
                if (PopCursorPosition())
                   centerCursor=True;
                break;
 
           // ^[  Full Ok level 2
           case cmcSearchStart:
                CheckForShiftSelection();
                if (IslineInEdition)
                   MakeEfectiveLineInEdition();
                {
                 int Pos;
                 if ((Pos=SearchOpenSymbol('{','}'))!=-1)
                   {
                    addToUndo(undoInMov);
                    GotoOffSet(Pos);
                    ClearSelIfNonPers();
                    update(ufUpdate);
                   }
                }
                 break;
 
           // ^]  Full Ok level 2
           case cmcSearchEnd:
                CheckForShiftSelection();
                if (IslineInEdition)
                   MakeEfectiveLineInEdition();
                {
                 int Pos;
                 if ((Pos=SearchCloseSymbol('{','}'))!=-1)
                   {
                    addToUndo(undoInMov);
                    GotoOffSet(Pos);
                    ClearSelIfNonPers();
                    update(ufUpdate);
                   }
                }
                break;
 
           // ^(  Full Ok level 2
           case cmcSearchOpPar:
                CheckForShiftSelection();
                if (IslineInEdition)
                   MakeEfectiveLineInEdition();
                {
                 int Pos;
                 if ((Pos=SearchOpenSymbol('(',')'))!=-1)
                   {
                    addToUndo(undoInMov);
                    GotoOffSet(Pos);
                    ClearSelIfNonPers();
                    update(ufUpdate);
                   }
                }
                break;
 
           // ^)  Full Ok level 2
           case cmcSearchClPar:
                CheckForShiftSelection();
                if (IslineInEdition)
                   MakeEfectiveLineInEdition();
                {
                 int Pos;
                 if ((Pos=SearchCloseSymbol('(',')'))!=-1)
                   {
                    addToUndo(undoInMov);
                    GotoOffSet(Pos);
                    ClearSelIfNonPers();
                    update(ufUpdate);
                   }
                }
                break;
 
           // ^{ is for [  Full Ok level 2
           case cmcSearchOpCor:
                CheckForShiftSelection();
                if (IslineInEdition)
                   MakeEfectiveLineInEdition();
                {
                 int Pos;
                 if ((Pos=SearchOpenSymbol('[',']'))!=-1)
                   {
                    addToUndo(undoInMov);
                    GotoOffSet(Pos);
                    ClearSelIfNonPers();
                    update(ufUpdate);
                   }
                }
                break;
 
           // ^} is for ]  Full Ok level 2
           case cmcSearchClCor:
                CheckForShiftSelection();
                if (IslineInEdition)
                   MakeEfectiveLineInEdition();
                {
                 int Pos;
                 if ((Pos=SearchCloseSymbol('[',']'))!=-1)
                   {
                    addToUndo(undoInMov);
                    GotoOffSet(Pos);
                    ClearSelIfNonPers();
                    update(ufUpdate);
                   }
                }
                break;
 
           // ^QEsc Full Ok level 2
           case cmcSearchComplement:
                CheckForShiftSelection();
                if (IslineInEdition)
                   MakeEfectiveLineInEdition();
                {
                 int Pos;
                 char *s=ColToPointer();
                 switch (*s)
                   {
                    case '}':
                         Pos=SearchOpenSymbol('{','}');
                         break;
                    case '{':
                         Pos=SearchCloseSymbol('{','}');
                         break;
                    case ')':
                         Pos=SearchOpenSymbol('(',')');
                         break;
                    case '(':
                         Pos=SearchCloseSymbol('(',')');
                         break;
                    case ']':
                         Pos=SearchOpenSymbol('[',']');
                         break;
                    case '[':
                         Pos=SearchCloseSymbol('[',']');
                         break;
                    default:
                         Pos=-1;
                   }
                 if (Pos!=-1)
                   {
                    addToUndo(undoInMov);
                    GotoOffSet(Pos);
                    ClearSelIfNonPers();
                    update(ufUpdate);
                   }
                }
                 break;

           // Ctrl+Space Full Ok level 2
           case cmcExpandCode:
                ClearSelIfNonPers();
                ExpandMacro();
                break;
 
           // ^KM Full Ok level 2
           case cmcToUpper:
                BlockToUpper(True);
                break;
 
           // ^KO Full Ok level 2
           case cmcToLower:
                BlockToLower(True);
                break;

           case cmcToggleCharCase:
                SetCharCase(2);
                break;

           case cmcInvertCase:
                BlockInvertCase();
                break;

           case cmcAltCase:
                BlockAltCase();
                break;

           // ^Tab Ok level 2
           case cmcSmartIndent:
                {
                 if (isReadOnly)
                    break;
                 flushLine();
 
                 int Pos;
                 if ((Pos=SearchOpenSymbol('{','}'))!=-1)
                   {
                    ClearSelIfNonPers();
                    char *s=Pos+buffer;
                    // Back to the start
                    for (;*s!='\n' && s!=buffer; --s);
                    if (*s=='\n') s++;
 
                    char *s1=Pos+buffer;
                    uint32 x;
                    // Forward to the pos
                    for (x=0; s!=s1; ++s)
                       { AdvanceWithTab(*s,x); }
                    ++x;
 
                    if (x>(uint32)curPos.x)
                      {
                       EditLine();
                       if (!IslineInEdition)
                          break;
                       Boolean oldOverW=overwrite;
                       overwrite=False;
                       for (x-=curPos.x;x;--x) InsertCharInLine(32);
                       overwrite=oldOverW;
                      }
                    update(ufLine);
                   }
                }
                break;
 
           // ^ShiftTab Ok level 2
           case cmcSmartUnIndent:
                {
                 /*if (!IsFirstCharInLine())
                    break;*/
                 if (isReadOnly || !GoFirstCharInLine())
                    break;
                 if (IslineInEdition)
                    MakeEfectiveLineInEdition();
 
                 int Pos;
                 if ((Pos=SearchOpenSymbol('{','}'))!=-1)
                   {
                    ClearSelIfNonPers();
                    char *s=Pos+buffer;
                    // Back to the start
                    for (;*s!='\n' && s!=buffer; --s);
                    if (*s=='\n') s++;
 
                    char *s1=Pos+buffer;
                    uint32 x;
                    // Forward to the pos
                    for (x=0; s!=s1; ++s)
                       { AdvanceWithTab(*s,x); }
                    ++x;
 
                    if ((uint32)curPos.x>x)
                      {
                       Boolean oldOverW=overwrite;
                       overwrite=False;
                       EditLine();
                       while ((uint32)curPos.x>x)
                         BackSpace();
                       while ((uint32)curPos.x<x)
                         InsertCharInLine(' ');
                       overwrite=oldOverW;
                      }
                    update(ufLine);
                   }
                }
                break;
 
           // ^QL Without Undo
           case cmcSelLength:
                ShowLength();
                break;
 
           // ^J Full Ok Level 2
           case cmcGotoEditorLine:
                {
                 int line=curPos.y+1;
                 if (editorDialog(edGotoLine,&line))
                   {
                    GoAndSelectLine(line);
                    centerCursor=True;
                   }
                 break;
                }
 
           //  Full Ok Level 2
           case cmcJumpToPrototype:
           case cmcJumpToFunction:
                {
                 int line;
                 char *Word;

                 CacheSyntaxHLData(GenericSHL);
                 Word=WordUnderCursor(80/*,wucIncludeColon*/);
                 if (editorDialog(command==cmcJumpToFunction ? edJumpToFunction :
                     edJumpToPrototype,&line,buffer,bufLen,Word,fileName,strC.Name))
                   {
                    if (line<0)
                       line=0;
                    if (line>limit.y)
                       line=limit.y;
                    MoveCursorTo(0,--line,True);
                    selStartF=(uint32)(curLinePtr-buffer);
                    selEndF=selStartF+LenWithoutCRLF(line,curLinePtr);
                    centerCursor=True;
                    update(ufUpdate|ufFound|ufLine);
                   }
                 delete[] Word;
                 break;
                }

           case cmcWhichFunctionIs:
                ShowWhichFunInStatus();
                break;

           // ^QB Full Ok Level 2
           case cmcGoBeginBlock:
                if (hasVisibleSelection())
                  {
                   addToUndo(undoInMov);
                   if (IslineInEdition)
                      MakeEfectiveLineInEdition();
                   GotoOffSet(selStart);
                   update(ufUpdate);
                  }
                break;
 
           // ^QK Full Ok Level 2
           case cmcGoEndBlock:
                if (hasVisibleSelection())
                  {
                   addToUndo(undoInMov);
                   if (IslineInEdition)
                      MakeEfectiveLineInEdition();
                   GotoOffSet(selEnd);
                   update(ufUpdate);
                  }
                break;
 
           // ^KL Full Ok Level 2
           case cmcMarkLine:
                MarkLine(True);
                break;
 
           // ^KT Full Ok Level 2
           case cmcMarkWord:
                if (IslineInEdition)
                   MakeEfectiveLineInEdition();
                addToUndo(undoCutInMov,NULL);
                selHided=False;
                SelWordUnderCursor();
                update(ufView);
                addToUndo(undoCutInMov,NULL);
                break;
 
           // ^KV Full OK Level 2
           case cmcMoveBlock:
                if (!isReadOnly && hasVisibleSelection() && PersistentBlocks)
                  {
                   flushLine();
                   unsigned pos=(unsigned)(ColToPointer()-buffer);
                   unsigned l=selEnd-selStart;
                   if (pos<selStart)
                     {
                      char *s=new char[l];
                      if (s)
                        {// Temporal copy, if we use the buffer then the selected
                         // text is moved during the insertion.
                         memcpy(s,buffer+selStart,l);
                         uint32 x,y;
                         x=curPos.x; y=curPos.y;
                         deleteSelect();
                         MoveCursorTo(x,y);
                         insertBuffer(s,0,l,True,True,False);
                         update(ufView);
                        }
                     }
                   else
                     if (pos>=selEnd)
                       {
                        uint32 st,e;
                        st=selStart; e=selEnd;
                        insertBuffer(buffer,selStart,l,True,True,False);
                        deleteRange(buffer+st,buffer+e);
                        GotoOffSet(staticNoMoveToEndPaste ? selStart : selEnd);
                        update(ufView);
                       }
                  }
                break;
 
           // ^W  Without Undo
           case cmcScrollDown:
                if (delta.y>0)
                  {
                   if (IslineInEdition)
                      MakeEfectiveLineInEdition();
                   delta.y--;
                   if (curPos.y>=delta.y+size.y)
                     {
                      MoveLinesUp(1);
                      if (NoInsideTabs)
                         curPos.x=FixPosCharLeft();
                     }
                   update(ufView);
                  }
                break;
 
           // ^Z  Without Undo
           case cmcScrollUp:
                if ((unsigned)delta.y<totalLines)
                  {
                   if (IslineInEdition)
                      MakeEfectiveLineInEdition();
                   delta.y++;
                   if (curPos.y<delta.y)
                     {
                      MoveLinesDown(1);
                      if (NoInsideTabs)
                         curPos.x=FixPosCharLeft();
                     }
                   update(ufView);
                  }
                break;
 
           // From the menu
           case cmcSetLocalOptions:
                {
                 LocalOptionsRect temp1;
                 ShlDiagBox temp2;
                 temp1.t1=CompactFlags();
                 temp2.t2=SyntaxHL;
                 temp2.items=SHLGenList;
                 temp2.selection=SHLConvValToPos(GenericSHL);
                 sprintf(temp1.tab,"%2u",tabSize);
                 sprintf(temp1.ind,"%2u",indentSize);
                 sprintf(temp1.wcol,"%3u",WrapCol);
                 ColMarkers2Str(colMarkers,temp1.colMarkers,colMarkersStrLen-1);
                 if (editorDialog(edSetLocalOptions,&temp1,&temp2))
                   {
                    ExpandFlags(temp1.t1);
                    SetHighlightTo((shlState)temp2.t2,SHLConvPosToVal(temp2.selection));
                    tabSize=max(atoi(temp1.tab),1);
                    indentSize=max(atoi(temp1.ind),1);
                    WrapCol=max(atoi(temp1.wcol),8);
                    delete[] colMarkers;
                    colMarkers=Str2ColMarkers(temp1.colMarkers);
                    update(ufView);
                    // If using a syntax highlight and tabs the user will most probably
                    // want to indent. Give advice about coherent indentation settings
                    // for tab users.
                    if (SyntaxHL!=shlNoSyntax && UseTabs &&
                        (!autoIndent || intelIndent || !OptimalFill ||
                        !NoInsideTabs || TabIndents || UseIndentSize || BackSpUnindents))
                       GiveAdvice(gadvTabsOps);
                   }
                }
                break;
 
           // From the menu
           case cmcSetGlobalOptions:
                SetGlobalOptions();
                break;
 
           case cmcExpandAllTabs:
                ExpandAllTabs(True);
                break;
 
           case cmcExpandAllTabsNi:
                ExpandAllTabs(False);
                break;
 
           case cmcCompactBuffer:
                CompactBuffer(True);
                break;
 
           case cmcCompactBufferNi:
                CompactBuffer(False);
                break;
 
           case cmcRecordMacro:
                Recording=True;
                MacroCount=0;
                break;
 
           case cmcStopMacro:
                Recording=False;
                break;
 
           case cmcPlayMacro:
                Recording=False;
                unlock(); // Let all the updates to update the syntax hl.
                for (i=0; i<MacroCount; i++)
                   {
                    TEvent e;
                    if (MacroArray(i)>=0xF0000)
                      {
                       e.what=evKeyDown;
                       e.keyDown.charScan.charCode=MacroArray(i)-0xF0000;
                       // Avoid triggering a command
                       e.keyDown.keyCode=0;
                       e.keyDown.shiftState=0;
                       handleEvent(e);
                      }
                    else
                      {
                       e.what=evCommand;
                       e.message.command=MacroArray(i) & 0xFFFF;
                       handleEvent(e); // It could be handleCommand for speed but if
                                       // I make it nobody can inherit clearly
                      }
                   }
                // Sanity
                selecting=False;
                break;
 
           case cmcSelRectStart:
                FillUndoForRectangularStartEnd(undoRectStart);
                Xr1=curPos.x;
                Yr1=curPos.y;
                selRectHided=False;
                updateRectCommands();
                update(ufView);
                break;
 
           case cmcSelRectEnd:
                FillUndoForRectangularStartEnd(undoRectEnd);
                Xr2=curPos.x;
                Yr2=curPos.y;
                selRectHided=False;
                updateRectCommands();
                update(ufView);
                break;
 
           case cmcSelRectCopy:
                flushLine();
                selRectCopy(); // It updates cmcSelRectPaste command
                update(ufUpdate);
                break;

           case cmcSelRectPaste:
                if (selRectClip)
                  {
                   flushLine();
                   selRectPaste(selRectClip,curPos.x,curPos.y);
                   updateRectCommands();
                  }
                break;
 
           case cmcSelRectDel:
                flushLine();
                selRectDelete(Xr1,Yr1,Xr2,Yr2);
                updateRectCommands();
                break;
 
           case cmcSelRectCut:
                if (isReadOnly) break;
                flushLine();
                selRectCopy();
                selRectDelete(Xr1,Yr1,Xr2,Yr2);
                updateRectCommands();
                break;
 
           case cmcSelRectMove:
                if (isReadOnly) break;
                flushLine();
                {
                 int X=curPos.x,Y=curPos.y;

                 if (unsigned(Y+Yr2-Yr1+1)>totalLines)
                    editorDialog(edRectOutside);
                 else
                   {
                    selRectCopy();
                    selRectDelete(Xr1,Yr1,Xr2,Yr2);
                    selRectPaste(selRectClip,X,Y);
                   }
                 updateRectCommands();
                }
                break;
 
           case cmcSelRectHide:
                addToUndo(undoRectHide,(void *)&selRectHided);
                selRectHided=Boolean(!selRectHided);
                updateRectCommands();
                update(ufView);
                break;

           case cmcSelRectToUpper:
                selRectToUpper();
                break;
 
           case cmcSelRectToLower:
                selRectToLower();
                break;
 
           case cmcLastPosCur:
                if (UndoSt==undoInMov)
                  {
                   flushLine();
                   MoveCursorTo(UndoArray[UndoActual].X,UndoArray[UndoActual].Y);
                   cursorMoved=1;
                  }
                break;
 
           case cmcToggleMoveOnPaste:
                staticNoMoveToEndPaste = staticNoMoveToEndPaste ? False : True;
                break;

           case cmcWrapOn:
                WrapLine=True;
                break;

           case cmcWrapOff:
                WrapLine=False;
                break;

           case cmcWrapToggle:
                WrapLine=WrapLine ? False : True;
                break;

           case cmcMouseToggle:
                #ifdef SEOSf_Linux
                // That's very Linux specific: If I suspend the mouse TVision
                // closes the connection with the gpm server and the mouse
                // behaves as default allowing the gpm paste mechanism
                if (TMouse::present())
                   TMouse::suspend();
                else
                   TMouse::resume();
                #endif
                break;
 
           case cmcGenCodeForMacro:
                MacroGenerateCode();
                break;
 
           case cmcProfileEditor:
                ProfileEditor();
                break;
 
           case cmcLoadFileUnderCur:
                flushLine();
                LoadFileUnderCursor(curLinePtr,ColToPointer(),LenWithoutCRLF(curPos.y,curLinePtr),
                                    CallBackStatusLine,this,fileName);
                break;
 
           // ------ From the File -------
           case cmcSave:
                if (modified)
                   save();
                break;
 
           case cmcSaveAs:
                saveAs();
                break;

           case cmcSaveSameTime:
                saveSameTime();
                break;
 
           case cmcReadBlock:
                readBlock(this);
                break;
 
           case cmcWriteBlock:
                writeBlock(this);
                break;
 
           case cmcSaveAsConvertEOL:
                saveAsConvertEOL();
                break;
 
           case cmcSaveAsNoConvertEOL:
                saveAsNoConvertEOL();
                break;
 
           case cmcCopyClipWin:
                clipWinCopy(0);
                break;
 
           case cmcPasteClipWin:
                clipWinPaste(0);
                break;

           case cmcCutClipWin:
                if (clipWinCopy(0) && !isReadOnly)
                   deleteSelect();
                break;
                
           case cmcCopyClipFile:
                clipFileCopy();
                break;
 
           case cmcPasteClipFile:
                clipFilePaste();
                break;

           case cmcPasteEmacsMode:
                PasteEmacsMode();
                break;
           /*case cmcFirstMacro ... cmcLastMacro:
                SLPSearchMacro(this,TranslateCommandToMacroName(event.message.command));
                break;*/

           case cmcChoosePMacrosList:
                ChoosePMacroFromList();
                break;

           case cmcQuotedPrintDecode:
                QuotedPrintDecode();
                break;

           case cmcInsertKeyName:
                InsertKeyName();
                break;
 
           case cmcRunSel_sLisp:
                SLPInterfaceRunSelection(this);
                break;

           case cmcRunEnter_sLisp:
                RunSLispAsk();
                break;                

           case cmcJumpLastCursorPos:
                MoveCursorTo(lastCurPos.x,lastCurPos.y,True);
                cursorMoved=1;
                centerCursor=True;
                break;

           case cmcInsertNewLine:
                flushLine();
                insertBuffer(CLY_crlf,0,CLY_LenEOL,canUndo,False,False);
                break;

           default:
               unlock();
               unlockUndo();
               return 0;
           }
        if (CheckScrollLockCenters)
           centerCursor=True;
        trackCursor(centerCursor);

        if (cursorMoved)
          {
           lastCurPos=oldCurPos;
           if (selecting)
             {
              UpdateSelecting();
              update(ufView);
             }
           else
             {
              update(ufUpdate);
              ClearSelIfNonPers();
             }
          }
        if (ShowMatchPairFly && ShowMatchPairNow)
           SearchMatchOnTheFly();
        unlock();
        unlockUndo();
        break;
    }
 if (Recording && command!=cmcRecordMacro &&
     command!=cmcForceMatchPairHL) // This command is sent periodically, if we
                                   // record it the macro becomes huge
   {
    if (command==(unsigned)cmcSelectOn && MacroCount &&
        MacroArray[MacroCount-1]==(unsigned)cmcSelectOff)
       MacroCount--; // If that's an Sel On after a Sel Off kill the Off
    else
       MacroArray[MacroCount++]=command;
   }
 return 1;
}


/**[txh]********************************************************************

  Description:
  Select the current line and optionally move the cursor to the beggining.
  
***************************************************************************/

void TCEditor::MarkLine(Boolean moveCursor)
{
 flushLine();
 if (moveCursor)
   {
    addToUndo(undoInMov);
    curPos.x=0;
   }
 selHided=False;
 selStart=(uint32)(curLinePtr-buffer);
 selEnd=selStart+lenLines[curPos.y];
 update(ufView);
}

/**[txh]********************************************************************

  Description:
  Gets the sLisp code under the cursor position. You must specify the
maximun length of the returned string.@p
  The routine supports the pipe feature to connect the word with an input
line object.

  Return:
  A pointer to the string (a new allocated one) or NULL if the cursor isn't
over sLisp code.

***************************************************************************/

char *TCEditor::sLispUnderCursor(uint32 maxLength)
{
 flushLine();

 // Set-Up the pipe
 char *s=ColToPointer();
 PipeOrigin=(unsigned)(s-buffer);
 PipeBuf=buffer;
 PipeBufLen=bufLen;

 int PosStart,PosEnd;
 if (*s=='(')
    PosStart=s-buffer;
 else
    if ((PosStart=SearchOpenSymbol('(',')'))==-1)
       return NULL;
 if (*s==')')
    PosEnd=s-buffer;
 else
    if ((PosEnd=SearchCloseSymbol('(',')'))==-1)
       return NULL;

 // Adjust the pipe
 PipeOrigin=PosStart;

 unsigned l=PosEnd-PosStart+1;
 if (l>maxLength-1)
    return NULL;
 char *code=new char[l+1];

 memcpy(code,buffer+PosStart,l);
 code[l]=0;
 return code;
}

void TCEditor::RunSLispAsk()
{
 char *Code;
 
 Code=sLispUnderCursor(maxRunAskCode);
 SLPInterfaceRunAsk(this,Code);
 delete[] Code;
}

void TCEditor::InsertKeyName()
{
 unsigned short k=TCEditor_SelectAKey();
 if (k)
   {
    char b[tktMaxKeyName];
    TCEditor_MakeKeyName(b,k);
    insertText(b,strlen(b),True);
   }
}

/**[txh]********************************************************************

  Description:
  Displays the name of the function where we are positioned in the status
line.

***************************************************************************/

void TCEditor::ShowWhichFunInStatus()
{
 flushLine();
 CreateFunctionList(buffer,bufLen,fileName,modifiedCounter,strC.Name);
 int s,e;
 char *n;
 if (SearchFunctionByLine(curPos.y,s,e,n))
    setStatusLine(n);
 //DestroyFunctionList(); Just left it, it could be needed soon
}

/**[txh]********************************************************************

  Description:
  Indents a block asking the indentation text to the user.

***************************************************************************/

void TCEditor::ArbitraryIndent()
{
 char b[maxArbitraryStrLen+2]; *b=0;
 if (isReadOnly || !hasVisibleSelection() ||
     editorDialog(edArbitraryIndent,b,maxArbitraryStrLen)!=cmOK)
    return;
 if (*b)
    IndentBlock(b,True);
}

/**[txh]********************************************************************

  Description:
  Indents a block using a line comment if available. If nothing is selected
the line where the cursor is located is selected and then indented.

***************************************************************************/

void TCEditor::CommentIndent()
{
 int l=TCEditor::strC.lEOLCom1; // For clarity, I hope gcc is smart enough
 if (isReadOnly || !l)
    return;
 if (!hasVisibleSelection())
    MarkLine(True);
 AllocLocalStr(b,l+2);
 strncpy(b,TCEditor::strC.EOLCom1,l);
 b[l]=' ';
 b[l+1]=0;
 IndentBlock(b,True);
}

/**[txh]********************************************************************

  Description:
  Unindents a block using a line comment if available. No check to see if
each line really starts with the comment is done. If nothing is selected
the line where the cursor is located is selected and then indented.

***************************************************************************/

void TCEditor::CommentUnIndent()
{
 int l=TCEditor::strC.lEOLCom1; // For clarity, I hope gcc is smart enough
 if (isReadOnly || !l)
    return;

 if (!hasVisibleSelection())
    MarkLine(False);

 // Update the line and go to the first selected line
 flushLine();
 addToUndo(undoInMov);
 unsigned Pos=JumpLineStartSelection();

 // Verify that at least the first line is already indented
 AllocLocalStr(b,l+2);
 strncpy(b,TCEditor::strC.EOLCom1,l);
 b[l]=' ';
 b[l+1]=0;
 if (strncmp(curLinePtr,b,l+1)!=0)
    return; // Nope, abort.

 // How many lines?
 unsigned Lines,PosAux,y;
 for (Lines=0,y=curPos.y,PosAux=Pos; PosAux<selEnd-1;)
    {
     PosAux+=lenLines[y++];
     Lines++;
    }
 // Mark the begining of the rectangular block
 FillUndoForRectangularStartEnd(undoRectStart);
 Xr1=0;
 Yr1=curPos.y;
 selRectHided=False;
 // Mark the end of the rectangular block
 FillUndoForRectangularStartEnd(undoRectEnd);
 Xr2=l+1;
 Yr2=curPos.y+Lines-1;
 // Delete the rectangle
 selRectDelete(Xr1,Yr1,Xr2,Yr2);
 updateRectCommands();
}

static
unsigned char ASCII2Hex(const char *s)
{
 unsigned ret;
 ret=(*s > '9' ? *s-'A'+10 : *s-'0')*16;
 s++;
 ret+=*s > '9' ? *s-'A'+10 : *s-'0';
 return ret;
}

/**[txh]********************************************************************

  Description:
  Decodes the current block from MIME Quoted Printable format.

***************************************************************************/

void TCEditor::QuotedPrintDecode()
{
 if (isReadOnly || !hasVisibleSelection()) return;
 flushLine();

 int len=selEnd-selStart;
 char *bf=new char[len];
 unsigned i=selStart,j=0;
 while (i<selEnd)
   {
    if (buffer[i]=='=' && i+2<selEnd)
      {
       if (strncmp(buffer+i+1,CLY_crlf,CLY_LenEOL)==0)
          i+=CLY_LenEOL+1;
       else
         {
          bf[j++]=ASCII2Hex(buffer+i+1);
          i+=3;
         }
      }
    else
      bf[j++]=buffer[i++];
   }
 deleteSelect();
 insertText(bf,j,True);
 delete[] bf;
}

#ifdef STANDALONE
/**[txh]********************************************************************

  Description:
  Remaps the characters from code page sourID to code page destID. It is
high level.

***************************************************************************/

void TCEditor::RemapCodePageBuffer(int sourID, int destID, unsigned ops,
                                   Boolean allowUndo)
{
 if (isReadOnly)
    return;

 if (ops & rbgOnlySelected)
   {
    if (!hasVisibleSelection())
       return;
    flushLine();
    char *s=buffer+selStart;
    char *end=buffer+selEnd;
    // Save all the undo info
    UndoCell un;
    if (allowUndo)
      {
       BlockUndoInfoStartFill(un,undoRecodeBlock,s,end);
       RecodeInfo *p=new RecodeInfo;
       p->sourID=sourID; p->destID=destID; p->ops=ops;
       un.s2.Recode=p;
      }
    TVCodePage::RemapBufferGeneric(sourID,destID,(uchar *)s,selEnd-selStart,ops);
    // Add the undo info to the array
    if (allowUndo)
       BlockUndoInfoEndFill(un);
   }
 else
   {
    if (editorDialog(edActionWOUndo)!=cmYes)
       return;
    flushLine();
    TVCodePage::RemapBufferGeneric(sourID,destID,(uchar *)buffer,bufLen,ops);
    flushUndoInfo();
   }

 MarkAsModified();
 update(ufView);
}
#else
void TCEditor::RemapCodePageBuffer(int , int , unsigned , Boolean )
{}
#endif

/**[txh]********************************************************************

  Description:
  Highlights the matching (, ), [, ], { or } under the cursor.

***************************************************************************/

Boolean TCEditor::SearchMatchOnTheFly()
{
 static int oldX,oldY;

 // Don't do it again /*IsHLCOn && */
 if (showMatchPairFlyCache==this && curPos.x==oldX && curPos.y==oldY) return False;

 showMatchPairFlyCache=this;
 oldX=curPos.x;
 oldY=curPos.y;

 if (IslineInEdition)
   { // Don't flush the line if that isn't really necesary
    if (!*inEditPtr || !strchr("{}()[]",*inEditPtr)) return False;
   }
 int wasInEdition=0;
 if (IslineInEdition)
   {
    MakeEfectiveLineInEdition();
    wasInEdition=1;
   }
 int Pos, dif;
 char *s=ColToPointer(dif);
 // Outside the line or inside a tab
 if (dif)
    return False;
 switch (*s)
   { // Note: s+1 because the routines are designed assuming the cursor is passing the char
    case '}':
         Pos=SearchOpenSymbolXY('{','}',XHLCC,YHLCC,s+1);
         break;
    case '{':
         Pos=SearchCloseSymbolXY('{','}',XHLCC,YHLCC,s+1);
         break;
    case ')':
         Pos=SearchOpenSymbolXY('(',')',XHLCC,YHLCC,s+1);
         break;
    case '(':
         Pos=SearchCloseSymbolXY('(',')',XHLCC,YHLCC,s+1);
         break;
    case ']':
         Pos=SearchOpenSymbolXY('[',']',XHLCC,YHLCC,s+1);
         break;
    case '[':
         Pos=SearchCloseSymbolXY('[',']',XHLCC,YHLCC,s+1);
         break;
    default:
         Pos=-1;
   }
 if (Pos!=-1)
   {
    int y=YHLCC-delta.y;
    int x=XHLCC-delta.x;
    XHLCO=curPos.x;
    YHLCO=curPos.y;
    if (x>=0 && x<size.x && y>=0 && y<size.y)
      {
       if (IsStatusLineOn)
          update(ufHLChar | ufStatus);
       else
          update(ufHLChar);
      }
    if (wasInEdition && ShowMatchPairNow)
       EditLine();
   }
 return True;
}

/**[txh]********************************************************************

  Description:
  Pastes the Emacs mode comment at the start of the file.

***************************************************************************/

void TCEditor::PasteEmacsMode()
{
 if (isReadOnly) return;
 int sizeSt, sizeEnd;
 char *text=SHLConstructEmacsModeComment(*this,sizeSt,sizeEnd);

 if (text)
   {
    int lenText=strlen(text);
    // Check if already there
    char buf[MaxExtension];
    int start,end;
    if (TakeCommentEmacs(buffer,bufLen,buf,NULL,&start,&end))
      {
       int stComp=start-1-sizeSt;
       // Is there, check if we pasted it:
       if (stComp>=0 && strncmp(buffer+stComp,text,sizeSt)==0 &&
           (!sizeEnd || ((unsigned)end+3+sizeEnd<bufLen &&
             strncmp(text+lenText-CLY_LenEOL-sizeEnd,buffer+end+3,sizeEnd+CLY_LenEOL)==0)))
         {// Yes, delete it
          deleteRange(buffer+stComp,buffer+end+3+sizeEnd+CLY_LenEOL,canUndo);
         }
       else
         {// Nope, is too risky to silently delete it.
          //printf("Actual mode definition: %s\n",buffer+stComp);
          messageBox(__("Emacs mode already pasted without this editor"),mfError|mfOKButton);
          ::free(text);
          return;
         }
      }
    else
      {
       // Beggining of text
       handleCommand(cmcTextStart);
       // Avoid pasting before a `bangline'
       if (bufLen>3 && *buffer=='#' && buffer[1]=='!')
         {// One line down
          handleCommand(cmcLineDown);
          if (curPos.y==0)
            {// It failed, one line file
             handleCommand(cmcLineEnd);
             newLine();
            }
         }
      }

    insertBuffer(text,0,lenText,canUndo,True,False);
    ::free(text);
   }
}

/**[txh]********************************************************************

  Description:
  It shows the length of the selection in a dialog box.

***************************************************************************/

void TCEditor::ShowLength()
{
 flushLine();
 if (hasSelection() && !selHided)
   {
    unsigned pos=selStart,lines=1;
    while (pos<selEnd)
      {
       if (buffer[pos++]=='\n')
          lines++;
      }
    //editorDialog(edLineLenght,selEnd-selStart,lines);
    int l=size.x+1;
    AllocLocalStr(b,l);
    TVIntl::snprintf(b,l,__("%d bytes selected, in %d"),selEnd-selStart,lines);
    setStatusLine(b);
   }
}

/**[txh]********************************************************************

  Description:
  Sets the indicated marker to the current cursor position.

***************************************************************************/

void TCEditor::SetMarker(unsigned marker)
{
 if (IslineInEdition)
    MarkersInLine[marker]=(int)(inEditPtr-bufEdit);
 else
    Markers[marker]=(unsigned)(ColToPointer()-buffer);
}

/**[txh]********************************************************************

  Description:
  Pastes the code for the recorded macro in the editor buffer.

***************************************************************************/

void TCEditor::MacroGenerateCode(void)
{
 char buf[MaxRecMacroLen*2];
 int pos,i,len;

 Recording=False;
 if (isReadOnly || !MacroCount)
    return;
 flushLine();
 len=sprintf(buf,"(defmacro 'Recorded macro'%s (eval%s",CLY_crlf,CLY_crlf);
 insertText(buf,len,False);
 for (i=0; i<MacroCount; i++)
    {
     if (MacroArray(i)>=0xF0000)
       {
        char val;

        strcpy(buf,"  (InsertText \"");
        pos=strlen(buf);
        for (; MacroArray(i)>=0xF0000 && i<MacroCount; i++)
           {
            val=MacroArray(i)-0xF0000;
            if (val=='\t')
              {
               buf[pos++]='\\';
               buf[pos++]='t';
              }
            else
              {
               if (val=='\"')
                  buf[pos++]='\\';
               buf[pos++]=val;
              }
           }
        buf[pos]=0;
        strcat(buf,"\")");
        strcat(buf,CLY_crlf);
        insertText(buf,strlen(buf),False);
        if (i<MacroCount)
           i--;
       }
     else
       {
        MacroArray(i);
        len=sprintf(buf,"  (SendCommands");
        insertText(buf,len,False);
        for (; MacroArray(i)<0x10000 && i<MacroCount; i++)
           {
            if ((i & 3)==3)
              {
               len=sprintf(buf,"%s  ",CLY_crlf);
               insertText(buf,len,False);
              }
            len=sprintf(buf," cmc%s",TranslateEdCommand(MacroArray(i) & 0xFFFF));
            insertText(buf,len,False);
           }
        len=sprintf(buf,")%s",CLY_crlf);
        insertText(buf,len,False);
        if (i<MacroCount)
           i--;
       }
    }
 len=sprintf(buf," )%s)%s",CLY_crlf,CLY_crlf);
 insertText(buf,len,False);
}

/**[txh]********************************************************************

  Description:
  Moves the cursor to the end of the file. High level, but doesn't force a
redraw, you should take care about.

***************************************************************************/

void TCEditor::JumpEndOfText()
{
 CheckForShiftSelection();
 addToUndo(undoInMov);
 flushLine();
 MoveLinesDown(totalLines-curPos.y);
 unsigned chars=LenWithoutCRLF(curPos.y,curLinePtr);
 unsigned count,x;
 for (count=0,x=0; count<chars; count++)
    { AdvanceWithTab(curLinePtr[count],x); }
 curPos.x=x;
}


/**[txh]**********************************************************************

  Description:
  Goes to the specified 'line' number and selects the line. Is used by
cmcGotoEditorLine.
@p
  by SET

*****************************************************************************/

void TCEditor::GoAndSelectLine(int line, Boolean selectLine)
{
 if (line<0)
    line=0;
 if (line>limit.y)
    line=limit.y;
 MoveCursorTo(0,--line,True);
 if (selectLine)
   {
    /* Use selStartF and selEndF here so any real selection
       will not removed. */
    selStartF=(uint32)(curLinePtr-buffer);
    /* Use the total length of the line to higlight it
       from the total left to total right */
    selEndF=selStartF+lenLines[line];
    update(ufUpdate|ufFound|ufLine);
   }
 else
    update(ufUpdate);
}

/****************************************************************************

   Function: void ProfileEditor(void)

   Type: TCEditor member.

   Objetive: Compare the speed of different modes.

   by SET.
   15/05 Version with clock by Robert.

****************************************************************************/

void TCEditor::ProfileEditor(void)
{
 if (limit.y<1000)
   {
    messageBox(__("Use a file with 1000 lines or more for that"),mfError | mfOKButton);
    return;
   }
#if 0
 // UClock is more accurate under DOS so I keep that
 int oldDeltaY=delta.y;
 int y=0;
 uclock_t t1,t2;
 char buf[80];

 t1=uclock();
 while (y<limit.y)
   {
    update(ufView);
    delta.y=y;
    doUpdate();
    y++;
   }
 t2=uclock();
 messageBox(mfOKButton,__("Speed: %f lines/second"),(y/((t2-t1)/(double)UCLOCKS_PER_SEC)));
 delta.y=oldDeltaY;
 update(ufView);
#else
 #if defined(SECompf_djgpp) || defined(TVComp_BCPP)
 // That's valid only under DOS where clock is absolute, in UNIX clock
 // gives information about the CPU we consumed
 int oldDeltaY=delta.y;
 int y=0;
 clock_t t1,t2;
 double secs;

 t1=clock();
 while (y<limit.y)
   {
    update(ufView);
    delta.y=y;
    doUpdate();
    y++;
   }
 t2=clock();
 secs=(t2-t1)/(double)CLOCKS_PER_SEC;
 messageBox(mfOKButton,__("Time: %f seconds\nSpeed: %f lines/second\n%f chars/sec"),
            secs,y/secs*size.y,y/secs*size.y*size.x);
 delta.y=oldDeltaY;
 update(ufView);
 #endif
 #ifdef SEOSf_Linux
 int oldDeltaY=delta.y;
 int y=0;
 clock_t t1,t2;
 struct timeval T1,T2;
 double secs,secs2;

 t1=clock();
 gettimeofday(&T1,0);
 while (y<limit.y)
   {
    update(ufView);
    delta.y=y;
    doUpdate();
    y++;
   }
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
 messageBox(mfOKButton,__("Time: %f seconds\nSpeed: %f lines/second\n%5.2f%% Editor\n%f chars/sec"),
            secs,y/secs*size.y,secs2/secs*100,y/secs*size.y*size.x);
 delta.y=oldDeltaY;
 update(ufView);
 #endif
#endif
}

/**[txh]********************************************************************

  Description:
  Writes the code in HTML to the provided stream. The pal parameter is a
pointer to a 16 entries array containing the RGB palette to be used. The
flags parameters are used to fine tune some details.@p

xhtmlTitle:      File name as title.@*
xhtmlBackground: Same background color as the editor.@*
xhtmlMonoFont:   Monospacied font.@*
xhtmlBoldFont:   Bold attribute.@*
xhtmlUseCSS:     Use CSS and HTML 4.01.@*

***************************************************************************/

void TCEditor::SourceToHTML(FILE *f, unsigned *pal, unsigned flags)
{
 if (flags & xhtmlUseCSS)
    SourceToHTML_CSS(f,pal,flags);
 else
    SourceToHTML_Old(f,pal,flags);
}

/**[txh]********************************************************************

  Description:
  This version uses plain old HTML. @x{::SourceToHTML}.
  
***************************************************************************/

void TCEditor::SourceToHTML_Old(FILE *f, unsigned *pal, unsigned flags)
{
 flushLine();

 ushort *b=(ushort *)malloc(limit.x*sizeof(ushort));
 if (!b)
    return;
 int bSize=limit.x,thisSize,x,i;
 uint32 linePtr=0;
 uint16 color=getColor(0x0201);
 uchar *bc=(uchar *)b;
 uchar col,antcol=0x10,fontOpen=0,val;

 // Head
 fputs("<HTML><HEAD>\n",f);
 fputs("<Meta name=\"GENERATOR\" content=\"SETEdit "TCEDITOR_VERSION_STR"\">\n",f);
 // Name of file
 if (flags & xhtmlTitle)
    fprintf(f,"<Title>%s</Title>",fileName);
 // End of head
 fputs("</HEAD>",f);
 // Background color as in the editor
 if (flags & xhtmlBackground)
    fprintf(f,"<Body BGColor=#%06X>\n",pal[(color>>4) & 0xF]);
 // That's a normal monospacied font
 if (flags & xhtmlMonoFont)
    fputs("<Font face=\"Courier New\">\n",f);
 // Bold is needed or the letters become hard to read
 if (flags & xhtmlBoldFont)
    fputs("<b>",f);
 for (int y=0; y<limit.y; y++)
    {
     char *s=buffer+linePtr;
     thisSize=LenWithoutCRLF(y,s);
     for (x=0,i=0; i<thisSize; s++,i++)
        { AdvanceWithTab(*s,x); }
     if (x>bSize)
       {
        b=(ushort *)realloc(b,x*sizeof(ushort));
        if (!b) return;
        bc=(uchar *)b;
        bSize=x;
       }
     (this->*formatLinePtr)(b,linePtr,x,color,x,lenLines.getAttr(y),y,0);
     for (i=0; i<x; i++)
        {
         col=bc[i*2+1] & 0xF;
         val=bc[i*2];
         if ((flags & xhtmlUseColors) && col!=antcol && val!=' ')
           {
            if (fontOpen)
               fputs("</Font>",f);
            else
               fontOpen=1;
            fprintf(f,"<Font color=#%06X>",pal[col]);
            antcol=col;
           }
         switch (val)
           {
            case ' ':
                 if (i && i+1<x && bc[(i+1)*2]!=' ')
                    fputc(' ',f);
                 else
                    fputs("&nbsp;",f);
                 break;
            case '>':
                 fputs("&gt;",f);
                 break;
            case '<':
                 fputs("&lt;",f);
                 break;
            default:
                 fputc(val,f);
           }
        }
     fputs("<br>\n",f);
     linePtr+=lenLines[y];
    }
 if (fontOpen)
    fputs("</Font>",f);
 if (flags & xhtmlMonoFont)
    fputs("</Font>",f);
 fputs("</Body></HTML>",f);

 free(b);
}

const char *TCEditor::shlNames[]=
{
 "",
 "Normal",
 "Marked",
 "Comment",
 "Reserved",
 "Ident",
 "Symbol",
 "String",
 "Integer",
 "Float",
 "Octal",
 "Hex",
 "Char",
 "Pre",
 "Illegal",
 "User",
 "CPU",
 "Break",
 "Symbol2",
 "CrossCur",
 "StatusLi",
 "MPHighL",
 "RectSel",
 "OddTab",
 "EvenTab",
 "ColMark"
};

/**[txh]********************************************************************

  Description:
  This variant uses CSS and HTML 4.01 Strict. @x{::SourceToHTML}.

***************************************************************************/

void TCEditor::SourceToHTML_CSS(FILE *f, unsigned *pal, unsigned flags)
{
 flushLine();

 ushort *b=(ushort *)malloc(limit.x*sizeof(ushort));
 if (!b)
    return;
 int bSize=limit.x,thisSize,x,i;
 uint32 linePtr=0;
 uint16 color=getColor(0x0201);
 uchar *bc=(uchar *)b;
 uchar col,antcol=0x10,fontOpen=0,val;
 uchar aColors[cNumColors];
 Boolean oldSel=selHided;
 selHided=True; // It interferes with the painting

 // Head
 fputs("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\n<HTML><HEAD>\n",f);
 // Name of file
 if (flags & xhtmlTitle)
    fprintf(f,"<Title>%s</Title>",fileName);
 // Common options, needs some adjusts, could be better
 fputs("<Meta name=\"GENERATOR\" content=\"SETEdit "TCEDITOR_VERSION_STR"\">\n",f);
 fputs("<meta name=\"resource-type\" content=\"document\">\n",f);
 fputs("<meta name=\"distribution\" content=\"global\">\n",f);
 fputs("<meta http-equiv=\"Content-Style-Type\" content=\"text/css\">\n",f);
 fputs("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n",f);
 // Embedded stylesheet, the user should move it to a file
 fputs("<style type=\"text/css\">\n<!--\n",f);
 // Background color as in the editor
 if (flags & xhtmlBackground)
    fprintf(f,"body { background:#%06X }\n",pal[(color>>4) & 0xF]);
 // A tricky but compact solution:
 for (i=1; i<cNumColors; i++)
    {
     aColors[i]=col=getColor(i);
     fprintf(f,"b.%s { color:#%06X; background:#%06X; ",shlNames[i],
             pal[col & 0xF],pal[col>>4]);
     // Bold is needed or the letters become hard to read
     if (!(flags & xhtmlBoldFont))
        fputs("font-weight:normal; ",f);
     // That's a normal monospacied font
     if (flags & xhtmlMonoFont)
        fputs("font-family:Courier New; ",f);
     fputs("}\n",f);
    }
 fputs("-->\n</style>\n",f);
 // End of head
 fputs("</HEAD><Body><p>",f);
 ColorsCacheToIDs();
 for (int y=0; y<limit.y; y++)
    {
     char *s=buffer+linePtr;
     thisSize=LenWithoutCRLF(y,s);
     for (x=0,i=0; i<thisSize; s++,i++)
        { AdvanceWithTab(*s,x); }
     if (x>bSize)
       {
        b=(ushort *)realloc(b,x*sizeof(ushort));
        if (!b) return;
        bc=(uchar *)b;
        bSize=x;
       }
     (this->*formatLinePtr)(b,linePtr,x,color,x,lenLines.getAttr(y),y,0);
     for (i=0; i<x; i++)
        {
         col=bc[i*2+1];
         val=bc[i*2];
         if ((flags & xhtmlUseColors) && col!=antcol &&
             (val!=' ' || (aColors[col] & 0xF0)!=(aColors[antcol] & 0xF0)))
           {
            if (fontOpen)
               fputs("</b>",f);
            else
               fontOpen=1;
            fprintf(f,"<b class=%s>",shlNames[col]);
            antcol=col;
           }
         switch (val)
           {
            case ' ':
                 if (i && i+1<x && bc[(i+1)*2]!=' ')
                    fputc(' ',f);
                 else
                    fputs("&nbsp;",f);
                 break;
            case '>':
                 fputs("&gt;",f);
                 break;
            case '<':
                 fputs("&lt;",f);
                 break;
            default:
                 fputc(val,f);
           }
        }
     fputs("<br>\n",f);
     linePtr+=lenLines[y];
    }
 if (fontOpen)
    fputs("</b>",f);
 fputs("</Body></HTML>",f);

 free(b);
 CacheColors();
 selHided=oldSel;
}

/****************************************************************************

   Function: void SetGlobalOptions(void)

   Type: TCEditor member.

   Objetive: Sets the statics members calling to the dialog box.

   by SET.

****************************************************************************/

void TCEditor::SetGlobalOptions(void)
{
 struct GlobalOptionsRect temp;

 CompactGlobalOptions(&temp);

 if (editorDialog(edSetGlobalOptions,&temp))
   {
    ExpandGlobalOptions(&temp);
    // If using tabs the user will most probably want to indent. Give advice about
    // coherent indentation settings for tab users.
    if (staticUseTabs &&
        (!staticAutoIndent || staticIntelIndent || !staticOptimalFill ||
        !staticNoInsideTabs || staticTabIndents || staticUseIndentSize ||
        staticBackSpUnindents))
       GiveAdvice(gadvTabsOps);
    //update(ufView);
   }
}


/**[txh]********************************************************************

  Description:
  Sets the statics members according to the values of the dialog box.@p
  temp: The structure filled by the dialog box.

***************************************************************************/

void TCEditor::ExpandGlobalOptions(GlobalOptionsRect *temp)
{
 #define S(var) static##var=(temp->t1 & go##var) ? True : False;
 S(AutoIndent);
 S(UseTabs);
 S(PersistentBlocks);
 S(IntelIndent);
 S(CrossCursorInCol);
 S(CrossCursorInRow);
 S(ShowMatchPair);
 S(ShowMatchPairFly);
 S(ShowMatchPairNow);
 S(NoMoveToEndPaste);
 S(TransparentSel);
 S(OptimalFill);
 S(WrapLine);
 S(SeeTabs);
 S(NoInsideTabs);
 S(TabIndents);
 S(UseIndentSize);
 S(DontPurgeSpaces);
 S(BackSpUnindents);
 S(ColumnMarkers);
 #undef S
 if (temp->t1 & goScrollLock)
    editorFlags|=efScrollLock;
 else
    editorFlags&=~efScrollLock;
 staticTabSize=max(atoi(temp->tab),1);
 staticIndentSize=max(atoi(temp->ind),1);
 staticWrapCol=max(atoi(temp->wcol),8);
 delete[] staticColMarkers;
 staticColMarkers=Str2ColMarkers(temp->colMarkers);
}


/**[txh]********************************************************************

  Description:
  Compact the statics members.@p
  temp: The structure to be used by the dialog box.

***************************************************************************/

void TCEditor::CompactGlobalOptions(GlobalOptionsRect *temp)
{
 temp->t1 = 0;
 #define S(var) if(static##var) temp->t1|=go##var;
 S(AutoIndent);
 S(UseTabs);
 S(PersistentBlocks);
 S(IntelIndent);
 S(CrossCursorInCol);
 S(CrossCursorInRow);
 S(ShowMatchPair);
 S(ShowMatchPairFly);
 S(ShowMatchPairNow);
 S(NoMoveToEndPaste);
 S(TransparentSel);
 S(OptimalFill);
 S(WrapLine);
 S(SeeTabs);
 S(NoInsideTabs);
 S(TabIndents);
 S(UseIndentSize);
 S(DontPurgeSpaces);
 S(BackSpUnindents);
 S(ColumnMarkers);
 #undef S
 if (editorFlags & efScrollLock) temp->t1|=goScrollLock;

 sprintf(temp->tab,"%2u",staticTabSize);
 sprintf(temp->ind,"%2u",staticIndentSize);
 sprintf(temp->wcol,"%3u",staticWrapCol);
 ColMarkers2Str(staticColMarkers,temp->colMarkers,colMarkersStrLen-1);
}


/**[txh]********************************************************************

  Description:
  Sets the normal members according to the values of the dialog box.@p
  temp: The structure filled by the dialog box.

***************************************************************************/

void TCEditor::ExpandGlobalOptionsLocally(GlobalOptionsRect *temp)
{
 Boolean oldTra=TransparentSel;

 autoIndent      = (temp->t1 & goAutoIndent) ? True : False;
 UseTabs         = (temp->t1 & goUseTabs) ? True : False;
 PersistentBlocks= (temp->t1 & goPersistentBlocks) ? True : False;
 intelIndent     = (temp->t1 & goIntelIndent) ? True : False;
 CrossCursorInCol= (temp->t1 & goCrossCursorInCol) ? True : False;
 CrossCursorInRow= (temp->t1 & goCrossCursorInRow) ? True : False;
 ShowMatchPair   = (temp->t1 & goShowMatchPair) ? True : False;
 ShowMatchPairFly= (temp->t1 & goShowMatchPairFly) ? True : False;
 ShowMatchPairNow= (temp->t1 & goShowMatchPairNow) ? True : False;
 TransparentSel  = (temp->t1 & goTransparentSel) ? True : False;
 OptimalFill     = (temp->t1 & goOptimalFill) ? True : False;
 WrapLine        = (temp->t1 & goWrapLine) ? True : False;
 SeeTabs         = (temp->t1 & goSeeTabs) ? True : False;
 NoInsideTabs    = (temp->t1 & goNoInsideTabs) ? True : False;
 TabIndents      = (temp->t1 & goTabIndents) ? True : False;
 UseIndentSize   = (temp->t1 & goUseIndentSize) ? True : False;
 DontPurgeSpaces = (temp->t1 & goDontPurgeSpaces) ? True : False;
 BackSpUnindents = (temp->t1 & goBackSpUnindents) ? True : False;
 ColumnMarkers   = (temp->t1 & goColumnMarkers) ? True : False;

 staticNoMoveToEndPaste=(temp->t1 & goNoMoveToEndPaste) ? True : False;
 tabSize=max(atoi(temp->tab),1);
 indentSize=max(atoi(temp->ind),1);
 WrapCol=max(atoi(temp->wcol),8);
 delete[] colMarkers;
 colMarkers=Str2ColMarkers(temp->colMarkers);
 if (TransparentSel!=oldTra)
    SetHighlightTo(SyntaxHL,GenericSHL);
 update(ufView);
}

/**[txh]********************************************************************

  Description:
  Calculates the offset of the start of a line in the buffer.
  
  Return: The offset.
  
***************************************************************************/

unsigned TCEditor::GetOffSetOffLine(int y)
{
 return GetOffsetGeneric(y,curPos.y,(unsigned)(curLinePtr-buffer));
}

/**[txh]********************************************************************

  Description:
  Computes the offset for a specified line @var{y}. You have to specify an
already know reference. The @var{yRef} is the line for the reference and
@var{lOff} is its offset.
  
  Return: The offset of the indicated line in the buffer.
  
***************************************************************************/

unsigned TCEditor::GetOffsetGeneric(int y, int yRef, unsigned lOff)
{
 int deltaCur=y-yRef;

 if (deltaCur==0)
    return lOff;

 if (abs(deltaCur)>y)
   {
    unsigned o=0;
    int i=0;
    while (y--)
      o+=lenLines[i++];
    return o;
   }
 else
   {
    if (deltaCur<0)
      {
       deltaCur=-deltaCur;
       unsigned o=lOff;
       int i=yRef;
       while (deltaCur--)
          o-=lenLines[--i];
       return o;
      }
    else
      {
       unsigned o=lOff;
       int i=yRef;
       while (deltaCur--)
          o+=lenLines[i++];
       return o;
      }
   }
}

char *LineHandler::getLine(int y, unsigned &len)
{
 if (!isReady() || (unsigned)y>ed->totalLines)
    return NULL;
 offset=ed->GetOffsetGeneric(y,line,offset);
 line=y;
 len=ed->lenLines[line];
 return ed->buffer+offset;
}

/**[txh]********************************************************************

  Description:
  This function creates a new copy of a rectangular structure. Is called by
the undo/redo stuff to undo/redo the rectangle copy operation.

***************************************************************************/

static
struct selRecSt *DuplicateRectSt(char *block)
{
 if (!block)
    return 0;

 struct selRecSt *p=(struct selRecSt *)block;

 int Width=p->Xr2-p->Xr1;
 int Height=p->Yr2-p->Yr1+1;
 unsigned size=(unsigned)Width*(unsigned)Height+sizeof(struct selRecSt);
 struct selRecSt *auxR;

 // Try to get the memory
 auxR=(struct selRecSt *)(new char[size]);
 memcpy(auxR,block,size);

 return auxR;
}

/**[txh]********************************************************************

  Description:
  Converts all the characters in the selected rectangle to upper case. Is
a highlevel routine and is simple instead of fast. Side effect: tabs are
converted to spaces.

***************************************************************************/

void TCEditor::selRectToUpper()
{
 if (isReadOnly) return;
 flushLine();

 selRectCopy();
 selRectDelete(Xr1,Yr1,Xr2,Yr2);

 unsigned size=(selRectClip->Xr2-selRectClip->Xr1)*(selRectClip->Yr2-selRectClip->Yr1+1);
 char *s=selRectClip->s;
 char *end=s+size;
 for (; s<end; s++)
     *s=TVCodePage::toUpper(*s);

 selRectPaste(selRectClip,Xr1,Yr1);
}

/**[txh]********************************************************************

  Description:
  Converts all the characters in the selected rectangle to lower case. Is
a highlevel routine and is simple instead of fast. Side effect: tabs are
converted to spaces.

***************************************************************************/

void TCEditor::selRectToLower()
{
 if (isReadOnly) return;
 flushLine();

 selRectCopy();
 selRectDelete(Xr1,Yr1,Xr2,Yr2);

 unsigned size=(selRectClip->Xr2-selRectClip->Xr1)*(selRectClip->Yr2-selRectClip->Yr1+1);
 char *s=selRectClip->s;
 char *end=s+size;
 for (; s<end; s++)
     *s=TVCodePage::toLower(*s);

 selRectPaste(selRectClip,Xr1,Yr1);
}

/****************************************************************************

   Function: Boolean selRectCopy()

   Type: TCEditor member.

   Objetive: Copy the selected rectangle into a buffer.

   Returns: False on error.

   by SET.

****************************************************************************/

Boolean TCEditor::selRectCopy(Boolean allowUndo)
{
 // Avoid to be out of buffer
 if ((unsigned)Yr2>totalLines)
    Yr2=totalLines;

 if (!hasRectSel())
    return True;

 int Width=Xr2-Xr1;
 int Height=Yr2-Yr1+1;
 unsigned size=(unsigned)Width*(unsigned)Height+sizeof(struct selRecSt);
 struct selRecSt *auxR;

 // Try to get the memory
 if ((auxR=(struct selRecSt *)malloc(size))==0)
    return False;

 // A pointer to the buffer area
 char *b=(char *)(auxR)+sizeof(struct selRecSt);

 auxR->Xr1=Xr1;
 auxR->Xr2=Xr2;
 auxR->Yr1=Yr1;
 auxR->Yr2=Yr2;

 int y,x;
 char *s=buffer+GetOffSetOffLine(Yr1);
 char *sy=s;
 int w;
 char c=' ';
 // Scan the lines inside the rectangle
 for (y=Yr1; y<=Yr2; y++)
    {
     // Initialize vars
     w=0; x=0;
     s=sy;
     // fill untill you reach the right side of the rect
     while (x<Xr2)
       {
        // w is the width of the actual char
        if (!w)
          {
           c=*s;
           s++;
           if (c=='\t')
             { // the tabs don't have 1 char of width
              w=tabSize-(x%tabSize);
              c=' ';
             }
           else
             if (c=='\r' || c==0 || c=='\n')
               { // the end of line is like infinit blanks
                w=MaxLineLen+1;
                c=' ';
               }
             else
               w=1; // The normal case
          }
        if (x>=Xr1)
          { // If is inside the rect copy it
           *b=c;
           b++;
          }
        x++;
        w--;
       }
     // Point to the next line.
     sy+=lenLines[y];
    }

 // Store it when we have it finished
 if (allowUndo)
    addToUndo(undoRectCopy,auxR);
 delete selRectClip;
 selRectClip=auxR;
 enableCommand(cmcSelRectPaste);

 return True;
}


/****************************************************************************

   Function: void EnsureXDontTab(char *s,int x,int w,char **stop)

   Type: TCEditor member.

   Objetive: Ensure that the zone x to x+w is free of tabs, if a tab is
   found convert it to spaces.
     This routine is very delicated and was made in an easy, but no fast,
   way.

   Notes:
   canUndo must be False to avoid multiple undos.
   Can't be called with a line under edition.

   Parameters:
   s: pointer to the start of the line.
   x: x to start checking.
   w: width of the region.
   stop: returns from where the block delete must delete to eat all the extra
         chars, is NULL is there is no need to delete anything.

   Returns:
   0 process OK.
   1 end of line before the end of process.

   by SET.

****************************************************************************/

int TCEditor::EnsureXDontTab(char *s,int x,int w,char **stop)
{
 int X=0;
 int oldX=0;
 char *lastUsed=s;

 if (isReadOnly)
    return 0;
 while (X<x)
   {
    oldX=X;
    AdvanceWithTab(*s,X);
    if (!ucisspace(*s))
       lastUsed=s;
    s++;
    if (!*s || *s=='\r' || *s=='\n')
      {
       *stop=NULL;
       return 1;
      }
   }
 *stop=s;
 if (X!=x)
   {
    unsigned o=(unsigned)(s-buffer);
    deleteRange(s-1,s,False);
    insertSpaces(X-oldX,0,False);
    s=buffer+o+x-oldX;
   }
 while (w--)
   {
    if (*s=='\t')
      {
       unsigned o=(unsigned)(s-buffer);
       deleteRange(s,s+1,False);
       insertSpaces(tabSize-(x%tabSize),0,False);
       s=buffer+o;
      }
    if (!*s || *s=='\r' || *s=='\n')
      {
       *stop=lastUsed+1;
       return 1;
      }
    s++;
    x++;
   }
 return 0;
}

/****************************************************************************

   Function: Boolean selRectPaste(struct selRecSt *st, int X, int Y)

   Type: TCEditor member.

   Objetive: Paste the previously copied rectangle.

   Parameters:
   st: The struct that contains the rectangle.
   X,Y: The insertion point.

   Returns: False on error.

   by SET.

****************************************************************************/

Boolean TCEditor::selRectPaste(struct selRecSt *st, int X, int Y, Boolean allowUndo)
{
 if (isReadOnly) return False;

 int Width=st->Xr2-st->Xr1;
 int Height=st->Yr2-st->Yr1+1;

 if ((unsigned)(Y+Height)>totalLines)
   {
    editorDialog(edRectOutside);
    return False;
   }

 // Filled by hand
 struct UndoCell un;
 if (allowUndo)
   {
    addToUndo(undoPreCopyInfo);
    UndoSaveStartState(un);
   }

 if (X!=curPos.x || Y!=curPos.y)
    MoveCursorTo(X,Y);

 int y=Y;
 char *b=st->s;
 char *stop;
 int cant;

 if (allowUndo)
    if (!FillUndoForRectangularPasteClear(Height,un,undoRectPaste))
       return False;

 Boolean oldCanUndo=canUndo;
 canUndo=False;
 while (Height--)
   {
    if (EnsureXDontTab(curLinePtr,X,0,&stop))
      { // The insertion is outside of the real line, don't create extra spaces
       char *sb=b,*sc=b;
       int i=Width;
       while (i--)
         {
          if (!ucisspace(*sb))
             sc=sb+1;
          sb++;
         }
       cant=(int)(sc-b);
      }
    else
       cant=Width;
    curPos.x=X;
    if (cant)
       insertBuffer(b,0,cant,False,False,False);
    b+=Width;
    curLinePtr+=lenLines[y];
    y++;
    curPos.y=y;
    curPos.x=X;
   }
 Xr1=X;
 Xr2=X+Width;
 Yr1=Y;
 Yr2=Y+st->Yr2-st->Yr1;
 selRectHided=False;
 MoveCursorTo(X,Y);
 canUndo=oldCanUndo;
 return True;
}


/****************************************************************************

   Function: void UndoRectangularPasteClear(UndoCell &un)

   Type: TCEditor member.

   Objetive: Undoes the action made by a paste of a rectangle.
   Is in a separated function because that's a relative complex undo (too
   much variables).

   Parameters:
   un: The UndoCell information.

   The information is stored in s with the following structure:
   char [...]  Buffer with the text affected (the length is in un.Length).
   struct selRectSt of the current state

   by SET.

****************************************************************************/

void TCEditor::UndoRectangularPasteClear(UndoCell &un)
{
 char *s=un.s;
 unsigned size=un.Length;
 struct selRecSt *sr=(struct selRecSt *)(s+size);
 int Y1=sr->Yr1;
 int Y2=sr->Yr2;
 unsigned Height=Y2-Y1+1;
 Y1=sr->Ycur; // In the undo for Paste the cursor isn't in the selection

 char *sLine=buffer+GetOffSetOffLine(Y1);
 char *end_line=sLine;

 // to end_line must be saved
 int y=Y1;
 while (Height--)
   end_line+=lenLines[y++];

 // Put away all the zone
 deleteRange(sLine,end_line,False);
 // Put in the original one
 insertBuffer(s,0,size,False,False,False);

 // Recreate the rectangular selection
 Xr1=sr->Xr1;
 Yr1=sr->Yr1;
 Xr2=sr->Xr2;
 Yr2=sr->Yr2;
 selRectHided=sr->selHide;

 MoveCursorTo(un.X,un.Y);
}


/****************************************************************************

   Function: Boolean FillUndoForRectangularPasteClear(int Height,
             struct UndoCell &un,UndoState st)

   Type: TCEditor member.

   Objetive: Fills the undo cell with the information necesary to undo a
   Paste or a Clear of a rectangular area.

   Parameters:
   Height: of the rectangle.
   un: The UndoCell.
   st: to difference the paste from the Del.

   by SET.

****************************************************************************/

Boolean TCEditor::FillUndoForRectangularPasteClear(int Height,struct UndoCell &un,
        UndoState st)
{
 // from curLinePtr
 char *end_line=curLinePtr;
 unsigned h=Height;
 int y=curPos.y;
 char *aux;

 // to end_line must be saved
 while (h--)
   end_line+=lenLines[y++];

 // Allocate memory for that
 unsigned size=(unsigned)(end_line-curLinePtr);
 unsigned sizer=sizeof(struct selRecSt);
 aux=new char[size+sizer];
 if (aux==NULL)
    return False;

 // Save the info of the actual rectangle
 struct selRecSt stact;
 stact.Xr1=Xr1;
 stact.Yr1=Yr1;
 stact.Xr2=Xr2;
 stact.Yr2=Yr2;
 stact.Ycur=curPos.y;
 stact.selHide=selRectHided;

 memcpy(aux,curLinePtr,size);
 memcpy(aux+size,&stact,sizer);

 un.s=aux;
 un.Type=st;
 un.Length=size;

 addToUndo(undoPostCopyInfo,&un);
 return True;
}


/****************************************************************************

   Function: Boolean FillUndoForRectangularStartEnd(UndoState st)

   Type: TCEditor member.

   Objetive: Adds to the undo the info for start and end of rectangular
   selection.

   Parameters:
   st: to difference the start from the end.

   by SET.

****************************************************************************/

Boolean TCEditor::FillUndoForRectangularStartEnd(UndoState st)
{
 struct selRecSt *stact=new selRecSt;

 if (stact==NULL)
    return False;

 if (st==undoRectStart)
   {
    stact->Xr1=Xr1;
    stact->Yr1=Yr1;
    stact->Xr2=curPos.x;
    stact->Yr2=curPos.y;
   }
 else
   {
    stact->Xr1=curPos.x;
    stact->Yr1=curPos.y;
    stact->Xr2=Xr2;
    stact->Yr2=Yr2;
   }
 stact->selHide=selRectHided;

 addToUndo(st,stact);
 return True;
}


/****************************************************************************

   Function: Boolean selRectDelete(int X1, int Y1, int X2, int Y2)

   Type: TCEditor member.

   Objetive: Deletes (Clear) the specified rectangle.

   Parameters:
   X1,Y1: Top left.
   X2,Y2: Bottom right.

   Returns: False on error.

   by SET.

****************************************************************************/

Boolean TCEditor::selRectDelete(int X1, int Y1, int X2, int Y2, Boolean allowUndo)
{
 if (isReadOnly) return False;
 int Width=X2-X1;
 int Height=Y2-Y1+1;

 // Filled by hand
 struct UndoCell un;

 if (allowUndo)
   {
    addToUndo(undoPreCopyInfo);
    UndoSaveStartState(un);
   }

 if (X1!=curPos.x || Y1!=curPos.y)
   {
    MoveCursorTo(X1,Y1);
    if (curPos.y<delta.y)
      {
       trackCursor(True);
       AdjustDrawPtr();   // because we will modify the rest of the lines
      }
   }

 if (allowUndo)
    if (!FillUndoForRectangularPasteClear(Height,un,undoRectDel))
       return False;

 char *stop;

 while (Height--)
   {
    if (EnsureXDontTab(curLinePtr,X1,Width,&stop))
      {
       if (stop!=NULL && CLY_IsntEOL(*stop)) // If stop==NULL nothing to do
         { // Delete from stop to the end of the line
          deleteRange(stop,curLinePtr+LenWithoutCRLF(curPos.y,curLinePtr),False);
         }
      }
    else
      {
       deleteRange(stop,stop+Width,False);
      }
    curLinePtr+=lenLines[curPos.y++];
   }
 MoveCursorTo(X1,Y1);
 selRectHided=True;
 return True;
}


/**[txh]********************************************************************

  Description:
  Compact the Boolean flags of the editor into a word.

  Return:
  The compacted word.

***************************************************************************/

uint32 TCEditor::CompactFlags(void)
{
 uint32 t=0;
 if (overwrite)        t|=loOverwrite;
 if (autoIndent)       t|=loAutoIndent;
 if (UseTabs)          t|=loUseTabs;
 if (PersistentBlocks) t|=loPersistentBlocks;
 if (intelIndent)      t|=loIntelIndent;
 if (CrossCursorInCol) t|=loCrossCursorInCol;
 if (CrossCursorInRow) t|=loCrossCursorInRow;
 if (ShowMatchPair)    t|=loShowMatchPair;
 if (ShowMatchPairFly) t|=loShowMatchPairFly;
 if (ShowMatchPairNow) t|=loShowMatchPairNow;
 if (TransparentSel)   t|=loTransparentSel;
 if (OptimalFill)      t|=loOptimalFill;
 if (WrapLine)         t|=loWrapLine;
 if (SeeTabs)          t|=loSeeTabs;
 if (NoInsideTabs)     t|=loNoInsideTabs;
 if (TabIndents)       t|=loTabIndents;
 if (UseIndentSize)    t|=loUseIndentSize;
 if (DontPurgeSpaces)  t|=loDontPurgeSpaces;
 if (BackSpUnindents)  t|=loBackSpUnindents;
 if (ColumnMarkers)    t|=loColumnMarkers;

 return t;
}


/**[txh]********************************************************************

  Description:
  Expand a word to the Boolean flags.

***************************************************************************/

void TCEditor::ExpandFlags(uint32 t, Boolean allowUndo)
{
 if (overwrite!=((t & loOverwrite) ? True : False))
    toggleInsMode(allowUndo);
 autoIndent       = (t & loAutoIndent)       ? True : False;
 UseTabs          = (t & loUseTabs)          ? True : False;
 PersistentBlocks = (t & loPersistentBlocks) ? True : False;
 intelIndent      = (t & loIntelIndent)      ? True : False;
 CrossCursorInCol = (t & loCrossCursorInCol) ? True : False;
 CrossCursorInRow = (t & loCrossCursorInRow) ? True : False;
 ShowMatchPair    = (t & loShowMatchPair)    ? True : False;
 ShowMatchPairFly = (t & loShowMatchPairFly) ? True : False;
 ShowMatchPairNow = (t & loShowMatchPairNow) ? True : False;
 TransparentSel   = (t & loTransparentSel)   ? True : False;
 OptimalFill      = (t & loOptimalFill)      ? True : False;
 WrapLine         = (t & loWrapLine)         ? True : False;
 SeeTabs          = (t & loSeeTabs)          ? True : False;
 NoInsideTabs     = (t & loNoInsideTabs)     ? True : False;
 TabIndents       = (t & loTabIndents)       ? True : False;
 UseIndentSize    = (t & loUseIndentSize)    ? True : False;
 DontPurgeSpaces  = (t & loDontPurgeSpaces)  ? True : False;
 BackSpUnindents  = (t & loBackSpUnindents)  ? True : False;
 ColumnMarkers    = (t & loColumnMarkers)    ? True : False;
}


/**[txh]********************************************************************

  Description:
  Sets the cursor position to the start of the buffer. Is called when you
modified the whole buffer and want to recalculate the shl attributes, line
lengths, etc.

***************************************************************************/

void TCEditor::ResetCursorPosition(void)
{
 buffer[bufLen]=0;
 RecalculateLineAttributes();
 curLinePtr=buffer;
 curPos.x=curPos.y=0;
 selStart=selEnd=0;
 drawLine=drawPtr=0;
 MarkAsModified();
}

/****************************************************************************

   Function: void ExpandAllTabs(void)

   Type: TCEditor member.

   Objetive: Convert all the tabs in the file to spaces.
     The function uses a temporal file to expand the buffer, after the
   operation the cursor is positioned at the top of the file, the selection
   is cleaned and the lenLines array is updated.

   by SET.

****************************************************************************/

void TCEditor::ExpandAllTabs(Boolean interactive)
{
 char *s,*end;
 int x,aux;
 long l;
 FILE *f;

 if (isReadOnly)
    return;
 if (interactive && editorDialog(edActionWOUndo)!=cmYes)
    return;

 char *n=unique_name("tb");
 if (!n)
   {
    editorDialog(edCreateTMPError);
    return;
   }
 flushLine();
 f=fopen(n,"w+b");
 if (f)
   {
    end=buffer+bufLen;
    for (s=buffer,x=0; s<end; s++)
       {
        if (*s=='\t')
          {
           aux=tabSize-(x%tabSize);
           x+=aux;
           while (aux--)
             putc(' ',f);
          }
        else
          {
           putc(*s,f);
           if (*s=='\n')
              x=0;
           else
              x++;
          }
       }
    l=ftell(f);
    if (!ferror(f))
      {
       if ((uint32)l!=bufLen && setBufSize((uint32)l+1))
         {
          rewind(f);
          fread(buffer,(size_t)l,1,f);
          if (!ferror(f))
            {
             bufLen=(uint32)l;
             ResetCursorPosition();
             flushUndoInfo();
             update(ufView);
            }
         }
      }
    fclose(f);
    remove(n);
   }
 free(n);
}


/****************************************************************************

   Function: void CompactBuffer(void)

   Type: TCEditor member.

   Objetive: Compact the buffer using tabs to get a smaller file.

   Modified to let the comments and strings untouched as NIK pointed.

   by SET.

****************************************************************************/

void TCEditor::CompactBuffer(Boolean interactive)
{
 int y,x,x1,x2;
 char *s,*r,*end;
 int inString=0,inComment=0,inCppComment;
 int /*extString=0,*/extCppComment=0;

 if (isReadOnly)
    return;
 if (interactive && editorDialog(edActionWOUndo)!=cmYes)
    return;

 if (IslineInEdition) // I forgot it and I saw it with a report of Leon
    MakeEfectiveLineInEdition();

 end=buffer+bufLen;
 for (y=0, s=r=buffer; y<=(int)totalLines; y++)
    {
     x=0;
     //inString=extString;
     inCppComment=extCppComment;
     //extString=0;
     extCppComment=0;
     while (*s!='\n' && s<end)
       {
        if (*s==' ' && !inString && !inComment && !inCppComment)
          {
           x1=x;
           while (*s==' ')
             {
              x++;
              s++;
             }
           while (x1<x)
             {
              x2=NextTabPos(x1);
              if (x2<=x && x2-x1>1)
                {
                 *r='\t';
                 r++;
                 x1=x2;
                }
              else
                {
                 while (x1<x2 && x1<x)
                   {
                    *r=' ';
                    r++;
                    x1++;
                   }
                }
             }
          }
        else
          {
           while ((*s!=' ' || inComment || inString || inCppComment) && *s!='\n' && s<end)
             {
              switch (*s)
                {
                 case '\"':
                      if (!inComment && !inCppComment)
                         inString=!inString;
                      break;

                 case '/':
                      if (!inComment && !inString && !inCppComment)
                        {
                         *r=*s; x++; s++; r++;
                         if (*s=='*')
                            inComment=1;
                         else
                            if (*s=='/')
                               inCppComment=1;
                        }
                      break;

                 case '*':
                      if (inComment)
                        {
                         if (*(s+1)=='/')
                           {
                            *r=*s; x++; s++; r++;
                            inComment=0;
                           }
                        }
                      break;

                 case '\\':
                      if (inString)
                        { // Just skip one char
                         *r=*s; x++; s++; r++;
                         //if (*s=='\r' || *s=='\n') The strings are extended automatically
                         //   extString=1;
                        }
                      else
                      if (inCppComment)
                        { // Skip & test for extention
                         *r=*s; x++; s++; r++;
                         if (*s=='\r' || *s=='\n')
                            extCppComment=1;
                        }
                      break;
                }
              AdvanceWithTab(*s,x);
              *r=*s;
              s++; r++;
             }
          }
       }
     if (s<end)
       {
        *r=*s;
        s++; r++;
       }
    }
 if ((unsigned)(r-buffer)!=bufLen) // Any modification
   {
    bufLen=(uint32)(r-buffer);
    ResetCursorPosition();
    flushUndoInfo();
    update(ufView);
   }
}


/****************************************************************************

   Function: void InsertCharInLine(char cVal, Boolean allowUndo)

   Type: TCEditor member.

   Objetive: Insert a char in the edited line.
   Keeps track on:
   1) The selected text.
   2) The markers.
   3) The fucking ASCII 9 (Tab).
   4) The real tabs ;).
   5) The overwrite mode.
   6) The undo.

   Parameter:
   cVal: char to insert.

   by SET.

****************************************************************************/

void TCEditor::InsertCharInLine(char cVal, Boolean allowUndo)
{
 if (!IslineInEdition)
    return;
 // Tab in indent mode
 // Note: If we are in undo mode (!allowUndo) tabs are realtabs
 if (cVal=='\t' && !UseTabs && allowUndo)
   {
    int X;
    if (TabIndents)
      {
       int Xact=curPos.x;
       X=0;
       if (curPos.y>0)
         { // Search a hole in the last line
          char *s=curLinePtr-lenLines[curPos.y-1];
   
          do
           {
            for (;CLY_IsntEOL(*s) && !ucisspace(*s); s++) // While letters
                { AdvanceWithTab(*s,X); }
            for (;CLY_IsntEOL(*s) && ucisspace(*s); s++)  // While spaces
                { AdvanceWithTab(*s,X); }
           }
          while (CLY_IsntEOL(*s) && X<=Xact); // to a mayor X or the end of line
          if (X>Xact)
             X-=Xact;
          else
             X=NextTabPos(Xact)-Xact;
         }
       else
         X=NextTabPos(Xact)-Xact;
      }
    else
      {
       int Xact=curPos.x;
       if (UseIndentSize)
          X=NextIndentPos(Xact)-Xact;
       else
          X=NextTabPos(Xact)-Xact;
      }
    for (;X;--X) InsertCharInLine(' ');
    return;
   }

 if (overwrite)
   {
    char vals[2];
    if (allowUndo)
      {
       vals[0]=cVal;
       vals[1]=*inEditPtr;
       // Don't add to the undo yet, we could need a move
       lockUndo();
      }

    if (*inEditPtr)
      {
       if (*inEditPtr=='\t') // Is over a Tab?
         {
          if (allowUndo)
             addToUndo(undoInMov);
          curPos.x=LineWidth(bufEdit,inEditPtr);
         }
       *inEditPtr++=cVal;
       restCharsInLine--; // Why?!, if we advance a char we have one less in the line
       //return;
      }
    else
      { // At the end of line
       if (AdjustBufEditFor((int)(inEditPtr-bufEdit+1)))
          return;
       *inEditPtr++=cVal;
       *inEditPtr=0;
      }
    if (allowUndo)
      {
       addToUndo(undoOvrPutChar,(void *)&vals[0]);
       unlockUndo();
      }
   }
 else
   {
    unsigned PosOfIns=(unsigned)(inEditPtr-bufEdit);

    if (allowUndo)
       addToUndo(undoPutChar,(void *)&cVal);

    if (AdjustBufEditFor((int)(inEditPtr-bufEdit+restCharsInLine+1)))
       return;
    if (*inEditPtr)
      {
       if (*inEditPtr=='\t') // Is over a Tab?
          curPos.x=LineWidth(bufEdit,inEditPtr);
       memmove(inEditPtr+1,inEditPtr,restCharsInLine+1);
       *inEditPtr++=cVal;
      }
    else
      { // At the end of line
       *inEditPtr++=cVal;
       *inEditPtr=0;
      }

    // Update the markers
    for (int i=0; i<10; i++)
       {
        int Pos=MarkersInLine[i];
        if (Pos>=0 && (unsigned)Pos>=PosOfIns)
           MarkersInLine[i]++;
       }

    AdjustLineSel((uint32)(inEditPtr-bufEdit-1),1);
   }
 AdvanceWithTab(cVal,curPos.x);
 // Move the screen if the position is outside
 if ((delta.x+size.x-1)<curPos.x)
   {
    delta.x=curPos.x-size.x+8;
    // Adjust the limit or the scroll bar will neutralize the increment
    if (limit.x<delta.x+size.x)
       limit.x=delta.x+size.x;
    update(ufView);
   }
 if (curPos.x<delta.x || curPos.y>=delta.y+size.y || curPos.y<delta.y)
    trackCursor(True);
 // If the character is outside the limit adjust it
 if (curPos.x>=limit.x)
    limit.x=curPos.x+1;

 if (WrapLine && !ucisspace(cVal) && curPos.x>=WrapCol)
   {
    // Go back in the line searching a gap
    char *s=inEditPtr;
    while (!ucisspace(*s) && s>bufEdit) s--;
    if (s!=bufEdit)
      { // If found
       // Calculate how many spaces before
       int dif=inEditPtr-s-1;
       // Flush the line
       MakeEfectiveLineInEdition();
       // Move back
       addToUndo(undoInMov);
       GotoOffSet(ColToPointer()-buffer-dif);
       // Insert a CR
       insertText(CLY_crlf,CLY_LenEOL,False);
       // Go to the correct position
       addToUndo(undoInMov);
       GotoOffSet(ColToPointer()-buffer+dif);
       // Make it visible if the screen scrolled
       trackCursor(False);
       delta.x = 0;
       update(ufView);
      }
   }
 // This must be *after* the wrap because:
 // 1) Can flush the line and it complicates the wrap.
 // 2) The pair could be moved.
 if (ShowMatchPair && cVal && strchr("{}()[]",cVal))
   {
    int pos=-2;

    flushLine();
    switch (cVal)
      {
       case '}':
            pos=SearchOpenSymbolXY('{','}',XHLCC,YHLCC);
            break;
       case ')':
            pos=SearchOpenSymbolXY('(',')',XHLCC,YHLCC);
            break;
       case ']':
            pos=SearchOpenSymbolXY('[',']',XHLCC,YHLCC);
            break;
       case '{':
            pos=SearchCloseSymbolXY('{','}',XHLCC,YHLCC);
            break;
       case '(':
            pos=SearchCloseSymbolXY('(',')',XHLCC,YHLCC);
            break;
       case '[':
            pos=SearchCloseSymbolXY('[',']',XHLCC,YHLCC);
            break;
      }
    if (pos!=-2)
      {
       if (pos==-1)
         {
          char *s=TVIntl::getTextNew(__("No match found"));
          setStatusLine(s);
          DeleteArray(s);
         }
       else
         {
          int y=YHLCC-delta.y;
          int x=XHLCC-delta.x;
          XHLCO=curPos.x-1;
          YHLCO=curPos.y;
          if (x>=0 && x<size.x && y>=0 && y<size.y)
             update(ufHLChar);
          else
            {
             char bufaux[80];
             char *fmt=TVIntl::getTextNew(__("Match found at line %d column %d."));
             CLY_snprintf(bufaux,80,fmt,YHLCC+1,XHLCC+1);
             DeleteArray(fmt);
             setStatusLine(bufaux);
            }
         }
      }
   }
 MarkAsModified();
}

/****************************************************************************

   Function: void AdjustLineSel(uint32 pos,int dif)

   Type: TCEditor member.

   Objetive: Adjusts the selLineStart, selNewStart, selLineEnd and selNewEnd
   vars when is needed.

   Parameters:
   pos: Position of the modification.
   dif: Difference of length.

   by SET.

****************************************************************************/

void TCEditor::AdjustLineSel(uint32 pos,int dif, Boolean IncludeStart, Boolean toLeft)
{
  if (selNewStart<selNewEnd)
   {
    int CondStart,CondEnd;
    if (IncludeStart)
      {
       CondStart=pos<=selLineStart;
       CondEnd=pos<=selLineEnd;
      }
    else
      {
       CondStart=pos<selLineStart;
       CondEnd=pos<selLineEnd;
      }

    if (CondStart)
      {
       selLineStart+=dif; selNewStart+=dif;
       selLineEnd+=dif;   selNewEnd+=dif;
      }
    else
      if (CondEnd)
        {
         selLineEnd+=dif;
         selNewEnd+=dif;
         // If we are deleting characters to left we can delete the point that's the
         // start of the selection and that's must be taked in count.
         if (toLeft)
           {
            int difSt=pos-selLineStart+dif;
            if (dif<0 && difSt<0)
              {
               selLineStart+=difSt;
               selNewStart+=difSt;
              }
           }
        }
      else
        {
         // If we are deleting characters to left we can enter inside the selection
         // even when pos isn't inside.
         if (toLeft)
           {
            int difSt=pos-selLineEnd+dif;
            if (dif<0 && difSt<0)
              {
               selLineEnd+=difSt;
               selNewEnd+=difSt;
              }
           }
        }
   }
}

/**[txh]********************************************************************

  Description:
  This is a helper function for routines that damages a block of text like
BlockToUpper. It fills the undo info.
  
***************************************************************************/

void TCEditor::BlockUndoInfoStartFill(UndoCell &un, UndoState type,
                                      char *start, char *end)
{
 addToUndo(undoPreCopyInfo);
 UndoSaveStartState(un);
 un.Type=type;
 int l=(int)(end-start);
 un.Length=l;
 un.s=new char[l];
 memcpy(un.s,start,l);
}

/**[txh]********************************************************************

  Description:
  This is a helper function for routines that damages a block of text like
BlockToUpper. It commits the undo info.
  
***************************************************************************/

void TCEditor::BlockUndoInfoEndFill(UndoCell &un)
{
 UndoSaveFinalState(un);
 addToUndo(undoPostCopyInfo,&un);
}

/**[txh]********************************************************************

  Description:
  Converts all the characters of the selected block to uppercase.
  
***************************************************************************/

void TCEditor::BlockToUpper(Boolean allowUndo)
{
 BlockCaseChange(undoToUpper,allowUndo);
}

/**[txh]********************************************************************

  Description:
  Converts all the characters of the selected block to lowercase.
  
***************************************************************************/

void TCEditor::BlockToLower(Boolean allowUndo)
{
 BlockCaseChange(undoToLower,allowUndo);
}

/**[txh]********************************************************************

  Description:
  Changes lowercase characters into uppercase characters and viceversa.
  
***************************************************************************/

void TCEditor::BlockInvertCase(Boolean allowUndo)
{
 BlockCaseChange(undoInvertCase,allowUndo);
}

/**[txh]********************************************************************

  Description:
  Alternate characters, one uppercase and the next lowercase.
  
***************************************************************************/

void TCEditor::BlockAltCase(Boolean allowUndo)
{
 BlockCaseChange(undoAltCase,allowUndo);
}

/**[txh]********************************************************************

  Description:
  Converts all the characters of the selected block to a desired *case.
Is the base for other routines like BlockToLower and BlockToUpper.
  
***************************************************************************/

void TCEditor::BlockCaseChange(UndoState action, Boolean allowUndo)
{
 int ff;
 if (isReadOnly) return;
 if (hasSelection() && !selHided)
   {
    flushLine();
    char *s=buffer+selStart;
    char *end=buffer+selEnd;

    // Save all the undo info
    UndoCell un;
    if (allowUndo)
       BlockUndoInfoStartFill(un,action,s,end);

    switch (action)
      {
       case undoToLower:
            for (; s<end; s++)
                 *s=TVCodePage::toLower(*s);
            break;
       case undoToUpper:
            for (; s<end; s++)
                 *s=TVCodePage::toUpper(*s);
            break;
       case undoInvertCase:
            for (; s<end; s++)
                if (TVCodePage::isLower(*s))
                   *s=TVCodePage::toUpper(*s);
                else
                   *s=TVCodePage::toLower(*s);
            break;
       case undoAltCase:
            ff=0;
            for (; s<end; s++)
                if (TVCodePage::isAlpha(*s))
                  {
                   if (ff & 1)
                      *s=TVCodePage::toLower(*s);
                   else
                      *s=TVCodePage::toUpper(*s);
                   ff++;
                  }
            break;
       default:
            return;
      }
    MarkAsModified();

    // Add the undo info to the array
    if (allowUndo)
       BlockUndoInfoEndFill(un);

    update(ufView);
   }
}

/**[txh]********************************************************************

  Description:
  Changes the case of the char under cursor. High level. 0=lower, 1=upper
and 2=toggle.

***************************************************************************/

void TCEditor::SetCharCase(int option)
{
 if (isReadOnly) return;
 flushLine();
 int dif;
 char *s=ColToPointer(dif);
 if (dif) // Only if over a char
    return;
 uchar upper=TVCodePage::toUpper(*s);
 uchar lower=TVCodePage::toLower(*s),result;
 switch (option)
   {
    case 0:
         result=lower;
         break;
    case 1:
         result=upper;
         break;
    default:
         result=(*s==upper) ? lower : upper;
   }
 if (result!=0xFF && result && result==*s) // If no change needed just return
    return;
 deleteRange(s,s+1);
 insertBuffer((char *)&result,0,1,True,False,False);
}

/****************************************************************************

   Function: int SearchOpenSymbol(char open, char close)

   Type: TCEditor member.

   Objetive: Search the offset of the { where the cursor is.

   Parameters:
   open: ASCII of the open.
   close: ASCII of the close.

   Return: The offset or -1 if the cursor isn't inside a {}.

   by SET.

****************************************************************************/

int TCEditor::SearchOpenSymbol(char open, char close)
{
 unsigned Count=1;
 char *s=ColToPointer();
 char *start=curLinePtr;
 int  y=curPos.y;
 uint32 extraStart=SyntaxHighlightExtraFor(start,s,y);

 while (s!=buffer)
   {
    if (*--s==open)
      {
       uint32 extraEnd=SyntaxHighlightExtraFor(start,s,y);
       if (extraStart==extraEnd && !--Count)
          return (int)(s-buffer);
      }
    else
      {
       if (*s==close)
         {
          uint32 extraEnd=SyntaxHighlightExtraFor(start,s,y);
          if (extraStart==extraEnd)
             Count++;
         }
       else
          if (*s=='\n')
            {
             y--;
             if (y>=0)
                start-=lenLines[y];
            }
      }
   }
 return -1;
}

/**[txh]********************************************************************

  Description:
  Search the offset, X and Y of the match pair for open indicated by close.@*
  Parameters:@*
  open: ASCII of the open.@*
  close: ASCII of the close.@*
  X,Y: Coordinates of the match.@*
  pos: [Optional] value returned by ColToPointer(). It avoids recalculating
this value.@*

  Return:
  The offset or -1 if the cursor isn't inside a {}.

***************************************************************************/

int TCEditor::SearchOpenSymbolXY(char open, char close, int &X, int &Y, char *pos)
{
 unsigned Count=1;
 char *s=pos ? pos : ColToPointer(),*start=curLinePtr;

 if (s==buffer)
    return -1;

 // Compute the SHL for this char
 uint32 extraStart=SyntaxHighlightExtraFor(start,s,curPos.y);

 s--;
 Y=curPos.y;
 X=0;
 while (s>buffer)
   {
    s--;
    if (*s==open)
      {
       uint32 extraEnd=SyntaxHighlightExtraFor(start,s,Y);
       if (extraEnd==extraStart && !--Count)
         {
          while (start!=s)
            {
             AdvanceWithTab(*start,X);
             start++;
            }
          return (int)(s-buffer);
         }
      }
    else
      {
       if (*s==close)
         {
          uint32 extraEnd=SyntaxHighlightExtraFor(start,s,Y);
          if (extraEnd==extraStart) Count++;
         }
       else
         {
          if (*s=='\n')
            {
             Y--;
             if (Y>=0)
                start-=lenLines[Y];
            }
         }
      }
   }
 return -1;
}

/**[txh]********************************************************************

  Description:
  Search the offset of the } where the cursor is.@*
  Parameters:@*
  open: ASCII of the open.@*
  close: ASCII of the close.@*

  Return:
  The offset or -1 if the cursor isn't inside a {}.

***************************************************************************/

int TCEditor::SearchCloseSymbolXY(char open, char close, int &X, int &Y, char *pos)
{
 unsigned Count=1;
 char *s=pos ? pos : ColToPointer();
 char *end=buffer+bufLen,*lastl;

 X=0;
 Y=curPos.y;
 lastl=curLinePtr;

 // Compute the SHL for this char
 uint32 extraStart=SyntaxHighlightExtraFor(lastl,s,Y);

 if (s!=end)
   {
    while (s!=end)
      {
       if (*s==close)
         {
          uint32 extraEnd=SyntaxHighlightExtraFor(lastl,s,Y);
          if (extraEnd==extraStart && !--Count)
            {
             while (lastl!=s)
               {
                AdvanceWithTab(*lastl,X);
                lastl++;
               }
             return (int)(s-buffer);
            }
         }
       else
         {
          if (*s==open)
            {
             uint32 extraEnd=SyntaxHighlightExtraFor(lastl,s,Y);
             if (extraEnd==extraStart) Count++;
            }
          else
             if (*s=='\n')
               {
                Y++;
                lastl=s+1;
               }
         }
       s++;
      }
   }
 return -1;
}

/****************************************************************************

   Function: int SearchCloseSymbol(char open, char close)

   Type: TCEditor member.

   Objetive: Search the offset of the } where the cursor is.

   Parameters:
   open: ASCII of the open.
   close: ASCII of the close.

   Return: The offset or -1 if the cursor isn't inside a {}.

   by SET.

****************************************************************************/

int TCEditor::SearchCloseSymbol(char open, char close)
{
 unsigned Count=1;
 int dif;
 char *s=ColToPointer(dif);
 char *end=buffer+bufLen;
 char *lastl=curLinePtr;
 int   y=curPos.y;
 // Compute the SHL for this char
 uint32 extraStart=SyntaxHighlightExtraFor(lastl,s,y);

 if (s!=end)
   {
    // If the cursor is in a Tab check the first char
    if (dif<=0) s++;
    while (s!=end)
      {
       if (*s==close)
         {
          uint32 extraEnd=SyntaxHighlightExtraFor(lastl,s,y);
          if (extraEnd==extraStart && !--Count)
             return (int)(s-buffer);
         }
       else
         {
          if (*s==open)
            {
             uint32 extraEnd=SyntaxHighlightExtraFor(lastl,s,y);
             if (extraEnd==extraStart) Count++;
            }
          else
             if (*s=='\n')
               {
                y++;
                lastl=s+1;
               }
         }
       s++;
      }
   }
 return -1;
}


/****************************************************************************

   Function: void MakeEfectiveLineInEdition(void)

   Type: TCEditor member.

   Objetive: Put the line in edition inside the buffer.

   by SET.

****************************************************************************/

void TCEditor::MakeEfectiveLineInEdition(void)
{
 int actual,lastChar;
 int SpacesEated;
 char *s;

 if (DontPurge || DontPurgeSpaces) // Undo or user option
   {
    for (s=bufEdit,lastChar=0; *s; s++)
        lastChar++;
    actual=lastChar;
    SpacesEated=0;
   }
 else
   {
    for (s=bufEdit,actual=lastChar=0; *s; s++)
       {
        lastChar++;
        if (*s!='\t' && *s!=' ')
           actual=lastChar;
       }
    SpacesEated=lastChar-actual;
    // In case we are killing tabs and they are visible
    if (SpacesEated)
      {
       if (SeeTabs)
          update(ufView); // Not just the line, we are leaving it!
       if (selLineEnd>0)
          // Use the position we had before eating!
          AdjustLineSel(lastChar,-SpacesEated);
      }
   }
 #ifdef CLY_UseCrLf
 bufEdit[actual++]='\r';
 #endif
 bufEdit[actual++]='\n';
 bufEdit[actual]=0;

 int old=lenLines[lineInEdition];
 int dif=actual-old;

 if (actual>old)
   {
    if (bufSize<bufLen+dif)
       setBufSize(bufLen+dif);
   }
 if (actual!=old)
    memmove(curLinePtr+actual,curLinePtr+old,(size_t)(bufLen-(curLinePtr+old-buffer)));

 unsigned curLineOff=(unsigned)(curLinePtr-buffer);
 if (selNewStart!=selStart || selNewEnd!=selEnd)
   {
    selStart=selNewStart;
    selEnd=selNewEnd;
    updateFlags|=ufView;
   }

 // Translate the markers
 for (int i=0; i<10; i++)
    {
     int Pos=MarkersInLine[i];
     if (Pos>=0) // It's in this line
        Markers[i]=Pos+curLineOff;
     else
       {
        Pos=Markers[i];
        if ((unsigned)Pos>curLineOff) // It's beyond this line
           Markers[i]+=dif;
       }
    }

 memcpy(curLinePtr,bufEdit,actual);

 IslineInEdition=False;
 lenLines.setAll(lineInEdition,actual,attrInEdit);
 bufLen+=dif;
}

/**[txh]********************************************************************

  Description:
  It searchs a trigger in the PMColl pmacros collection. If isn't there it
tries to search in the TPMacrosCollection of the actual syntax highlight.
Additionally if the file isn't loaded it loads the file.

  Return:
  A void * to the PMacroStr if found or 0 if not.

***************************************************************************/

void *TCEditor::SearchPMTrigger(char *trg)
{
 void *ret=0;
 struct strSHL *s;
 ccIndex pos;

 // Search in the global list
 if (PMColl->search(trg,pos))
    ret=PMColl->at(pos);
 else
   { // If not try in the language specific list
    // Check if a SHL is used
    if (SHLValueSelected>=0 && SHLArray)
      {
       s=&SHLArray[SHLValueSelected];
       // Check if the pmacros is loaded
       if (!s->PM)
         { // Now if not loaded check if a name was provided
          if (s->PMacros)
            {
             s->PM=new TPMCollection(32,8);
             LoadPseudoMacroFile(ExpandFileNameToThePointWhereTheProgramWasLoaded(s->PMacros),*(s->PM));
            }
         }
       if (s->PM && s->PM->search(trg,pos))
          ret=s->PM->at(pos);
      }
   }
 return ret;
}

/**[txh]********************************************************************

  Description:
  Shows a list with the available pseudo macros to choose one from the list.
That's basically to help people remember the available pmacros but I think
isn't a good idea to use it all the time, just to learn what's available.

***************************************************************************/

void TCEditor::ChoosePMacroFromList(void)
{
 if (isReadOnly)
    return;

 TPMCollection *col=0;

 // Check if a SHL is used
 if (SHLValueSelected>=0 && SHLArray)
   {
    struct strSHL *s=&SHLArray[SHLValueSelected];
    // Check if the pmacros is loaded
    if (!s->PM)
      { // Now if not loaded check if a name was provided
       if (s->PMacros)
         {
          s->PM=new TPMCollection(32,8);
          LoadPseudoMacroFile(ExpandFileNameToThePointWhereTheProgramWasLoaded(s->PMacros),*(s->PM));
         }
      }
    col=s->PM;
   }
 if (!col)
    col=PMColl;
 if (!col)
   {
    editorDialog(edNoPMacrosAvail);
    return;
   }

 TStringableListBoxRec box;
 box.items=col;
 box.selection=0;
 if (editorDialog(edChoosePMacro,&box)==cmOK)
   {
    flushLine();
    ExpandPMacro(col->at(box.selection),0);
   }
}

/****************************************************************************

   Function: void ExpandMacro(void)
             
   Type: TCEditor member.

   Objetive: Expand a pseudo-macro. The routines looks 2 bytes before the
   cursor position.

   by SET.

****************************************************************************/

void TCEditor::ExpandMacro(void)
{
 char Trg[4];

 if (isReadOnly)
    return;
 flushLine();
 char *s=ColToPointer();

 if (s-buffer>=2)
   {
    s-=2;

    // Search if there are a trigger for it
    Trg[0]=s[0]; Trg[1]=s[1]; Trg[2]=0;
    ExpandPMacro(SearchPMTrigger(Trg),s);
   }
}

const unsigned MaxAuxMarker=3;

/**[txh]********************************************************************

  Description:
  Low level pmacro expansion, is called only by high-level members when they
know that's safe and know what to expand.

***************************************************************************/

void TCEditor::ExpandPMacro(void *pm, char *s)
{
 unsigned AuxMarkers[MaxAuxMarker];
 unsigned i;
 PMacroStr *d=(PMacroStr *)pm;

 if (d)
   {
    unsigned Pos,Val;
    int XCur=-1,YCur=0;
 
    if (!d->str) return;
    // Change to the correct mode
    uint32 oldFlags=CompactFlags();
    ExpandFlags(d->flags);
 
    memset(AuxMarkers,0,MaxAuxMarker*sizeof(unsigned));
    NotExpandingMacro=False;
    if (!s)
       s=ColToPointer();
    else
      {
       BackSpace();
       BackSpace();
      }
    for (s=d->str; *s; s++)
      {
       switch (*s)
         {
          case '\n': newLine();
                     break;
          case '\b': BackSpace();
                     break;
          case '@':  if (s[1] == '@') // the user want to insert a '@'
                       {
                        if (!IslineInEdition)
                           EditLine();
                        InsertCharInLine(*s++);
                        break;
                       }
                     if (IslineInEdition)
                       {
                        Pos=(unsigned)((curLinePtr-buffer)+(inEditPtr-bufEdit));
                       }
                     else
                        Pos=(unsigned)(ColToPointer()-buffer);
                     Val=*++s-0x30;
                     if (Val)
                       {
                        Val--;
                        if (Val<MaxAuxMarker)
                           AuxMarkers[Val]=Pos;
                       }
                     else
                       {
                        XCur=curPos.x;
                        YCur=curPos.y;
                       }
                     break;
          default:
                  if (!IslineInEdition)
                     EditLine();
                  InsertCharInLine(*s);
         }
      }
    NotExpandingMacro=True;
    if (IslineInEdition)
       MakeEfectiveLineInEdition();
    if (XCur!=-1)
       MoveCursorTo(XCur,YCur,True);
    for (i=0; i<MaxAuxMarker; i++)
        if (AuxMarkers[i])
           Markers[i+7]=AuxMarkers[i];
 
    // return to the original mode
    ExpandFlags(oldFlags);
 
    update(ufView);
   }
}

#define Block ((const char *)(block))

/****************************************************************************

   Function: void hideSelect()
             
   Type: TCEditor member.

   Objetive: Hide the selected area.

   by SET.

****************************************************************************/

void TCEditor::hideSelect()
{
 selecting = False;
 selHided=selHided ? False : True;
 update(ufView);
}


/****************************************************************************

   Function: void MoveLinesUp(int i)
             
   Type: TCEditor member.

   Objetive: Move the cursor i lines up.

   by SET.

****************************************************************************/

void TCEditor::MoveLinesUp(int i)
{
 if (IslineInEdition)
    MakeEfectiveLineInEdition();
 for (; i && curPos.y; --i)
     curLinePtr-=lenLines[--curPos.y];
 update(ufUpdate);
}

/****************************************************************************

   Function: void MoveLinesDown(int i)
             
   Type: TCEditor member.

   Objetive: Move the cursor i lines down.

   by SET.

****************************************************************************/

void TCEditor::MoveLinesDown(int i)
{
 if (IslineInEdition)
    MakeEfectiveLineInEdition();
 for (; i && (unsigned)curPos.y<totalLines; --i)
     curLinePtr+=lenLines[curPos.y++];
 update(ufUpdate);
}

/****************************************************************************

   Function: void initBuffer()

   Type: TCEditor member.

   Objetive: Allocate memory for the buffer according to bufSize.

   From Borland's TV 1.03.

****************************************************************************/

void TCEditor::initBuffer()
{
 buffer=(char *)malloc(bufSize);
 if (bufSize>6)
   {
    /* That's a test to initialize the buffer and catch bugs */
    unsigned i;
    for (i=0; i<bufSize-6; i+=6)
        strcpy(&buffer[i],"bug   ");
   }
}

/****************************************************************************

   Function: BufPlusLen *CreateBufPlusLen(char *s,unsigned l)

   Type: static function

   Objetive: Create a copy of a buffer including the length information
             inside, like a Pascal string.

   Parameters:
   char *s: The buffer.
   unsigned l: The length.

   Return: a BufPlusLen structure pointer to the allocated memory.

   by SET.

****************************************************************************/

static BufPlusLen *CreateBufPlusLen(char *s,unsigned l)
{
 BufPlusLen *p;

 p=(BufPlusLen *)malloc(l+sizeof(BufPlusLen));
 if (p)
   {
    p->len=l;
    memcpy(p->s,s,l);
   }
 return p;
}


static unsigned CalcNeededCharsToFill(int X1, int X2, int tabSize, Boolean OptimalFill)
{
 if (X2<=X1) return 0; // sanity check

 if (OptimalFill)
   {
    // Calculate the number of tabs to add
    unsigned tabs=X2/tabSize-X1/tabSize;
    if (tabs)
       return tabs+X2%tabSize; // The rest if filled with spaces
    // if no tabs the difference of columns is the number of spaces
   }
 return X2-X1;
}


static void FillGapInBuffer(int X1, int X2, char *s, int tabSize, Boolean OptimalFill)
{
 int x;
 if (X2<=X1) return; // sanity check

 if (OptimalFill)
   {
    while (X1<X2)
      {
       x=NextTabPos(X1);
       if (x<=X2 && x-X1>1)
         {
          *s='\t';
          s++;
          X1=x;
         }
       else
         {
          while (X1<X2 && X1<x)
            {
             *s=' ';
             s++;
             X1++;
            }
         }
      }
    return;
   }
 memset(s,' ',X2-X1);
}


/**[txh]********************************************************************

  Description:
  Helper function to recompute the shl of a modified block.

***************************************************************************/

void TCEditor::UpdateSyntaxHLBlock(unsigned firstLine, char *firstTouchedP,
                                   unsigned lastLine)

{
 uint32 attr;
 char *end=buffer+bufLen;
 // Prev. line attr
 if (firstLine)
    attr=lenLines.getAttr(firstLine-1);
 else
    attr=0;
 CacheSyntaxHLData(GenericSHL);
 // Recalculate for the inserted block
 for (;firstLine<=lastLine;firstLine++)
    {
     firstTouchedP+=LineMeassure(firstTouchedP,end,attr,NULL);
     lenLines.setAttr(firstLine,attr);
    }
 // Test for propagation:
 // There are more lines?
 if (firstLine<=totalLines)
   {
    // Yes, recalculate the following line
    firstTouchedP+=LineMeassure(firstTouchedP,end,attr,NULL);
    // Test if there are a propagation
    TestPropagation(lenLines.getAttr(firstLine),attr,firstTouchedP,firstLine+1);
    // Set the recalculated value
    lenLines.setAttr(firstLine,attr);
   }
}


/****************************************************************************

   Function: Boolean insertBuffer( char *p, uint32 offset, uint32 length,
                                   Boolean allowUndo, Boolean selectText,
                                   Boolean moveToEnd )

   Type: TCEditor member.

   Objetive: Insert a text in the buffer.

   Parameters:
   p: pointer to the source buffer.
   offset: offset in the source buffer.
   length: of the string to insert.
   allowUndo: if True the action is saved in the Undo.
   selecText: True => the insertion becomes selected.
   moveToEnd: If True the cursor is moved to the end of the insertion.

   Return: True if all OK.

   by SET.

****************************************************************************/

Boolean TCEditor::insertBuffer( char *p,
                               uint32 offset,
                               uint32 length,
                               Boolean allowUndo,
                               Boolean selectText,
                               Boolean moveToEnd
                             )
{
 if (isReadOnly)
    return False;
 if (!length)
    return True;

 struct stUndoInsert undoSt;
 undoSt.Eated=0;
 if (allowUndo)  // Just remember the actual selected area
    addToUndo(undoPreInsert,NULL);

 // To allow copying from the same buffer (only works if the insertion point
 // is greater than p+offtset+length).
 // When we do a realloc we know we need to re-compute the pointer.
 Boolean copyFromItself=(Boolean)(p==buffer);

 if (bufLen==0 && bufSize==0)
   { // It's a new buffer?
    bufSize=MakeItGranular(length);
    initBuffer();
    setBufLen(0);
    if (copyFromItself)
       // Pointer could be changed
       p=buffer;
   }

 int lar=LenWithoutCRLF(curPos.y,curLinePtr);
 // This information is to complete the one in lar
 int CRLFLenOfTheLine=lenLines[curPos.y]-lar;

 // Calculate the cursor position inside the line
 char *s=curLinePtr;
 int i,x,extraSpaces,pF;
 int TotalToAdd;
 int IncludeFirstLine=1; // The first line is included when SpecialLines is updated
 p+=offset; // That's the real source

 // Walk in the line trying to reach the cursor position to translate that
 // into an offset in the line called pF.
 for (pF=0,x=0; pF<lar && x<curPos.x; s++)
    {
     AdvanceWithTab(*s,x);
     if (IncludeFirstLine && !ucisspace(*s)) // That's for the SpecialLines
        IncludeFirstLine=0;
     pF++;
    }
 // is outside the real line? AND the inserted text won't destroy the new
 // spaces (| we won't purge spaces).
 if (x<curPos.x && (CLY_IsntEOL(*p) || DontPurgeSpaces))
    extraSpaces=CalcNeededCharsToFill(x,curPos.x,tabSize,OptimalFill);
 else
    extraSpaces=0;
 // s points to the "insertion point"
 s=curLinePtr+pF;
 TotalToAdd=extraSpaces+length;

 // insertion point as offset
 unsigned point=(unsigned)(s-buffer);

 if (allowUndo && !DontPurgeSpaces && CLY_IsEOL(*p)) // Don't purge spaces if we can't undo
   { // Search for spaces at the end of the new inserted line
    char *s1;
    for (s1=s-1; (*s1==' ' || *s1=='\t') && s1>=curLinePtr; s1--);

    if (s1!=(s-1))
      {
       int dif=(int)(s-s1-1);
       s=s1+1;
       TotalToAdd-=dif;
       pF-=dif;
       lar-=dif;
       undoSt.Eated=CreateBufPlusLen(s,dif);
      }
   }

 // Is the buffer large enough?
 {
  uint32 DeltaS=(uint32)(s-buffer); // If we need more space the pointer change
  //uint32 DeltaCL=(uint32)(curLinePtr-buffer);
  if (bufSize<bufLen+TotalToAdd+1) // 1 to keep space for a \x0
    {
     if (!setBufSize(bufLen+TotalToAdd+1))
        return False;
     if (copyFromItself)
        // Pointer could be changed
        p=buffer+offset;
    }
  s=buffer+DeltaS;
  //curLinePtr=buffer+DeltaCL;
 }
 undoSt.s=s;

 lenLines.set(curPos.y,pF);

 // * Insert the buffer
 // first make a hole
 unsigned holeSize=(unsigned)(bufLen-(s-buffer));
 if (holeSize)
   {
    if (TotalToAdd>0)
       memmove(s+TotalToAdd,s,holeSize);
    else
       if (TotalToAdd)
          memmove(s,s-TotalToAdd,holeSize);
   }

 // Update the selection pointers
 if (selectText)
   {
    selStart=(uint32)(s-buffer);
    selEnd=selStart+TotalToAdd;
    selStart+=extraSpaces;
    selStartOffSet=selStart;
    selHided=False;
   }
 else
   {
    if (hasSelection())
      {
       if (point<selStart)
         { // before
          selStart+=TotalToAdd;
          selEnd+=TotalToAdd;
          updateFlags|=ufView;
         }
       else
         if (point<selEnd)
           { // inside
            selEnd+=TotalToAdd;
            updateFlags|=ufView;
           }
         // else beyond
      }
   }

 // Update markers
 for (i=0; i<10; i++)
    {
     if (Markers[i]>point)
        Markers[i]+=TotalToAdd;
    }

 if (extraSpaces)
   {
    FillGapInBuffer(x,curPos.x,s,tabSize,OptimalFill);
    memcpy(s+extraSpaces,p,length);
    lenLines.set(curPos.y,lenLines[curPos.y]+extraSpaces);
   }
 else
    memcpy(s,p,length);

 if (allowUndo)
   {
    undoSt.l=length;
    addToUndo(undoInsert,(void *)&undoSt);
   }

 bufLen+=TotalToAdd;
 if (!holeSize)
    buffer[bufLen]=0;

 // Walk in the inserted text to see the changes in line lengths
 // Note: x is y.
 int inFirstLine;
 unsigned chars;
 unsigned firstTouchedLine=curPos.y;
 char *firstTouchedP=curLinePtr;

 if (isClipboard())
    moveToEnd=True;

 if (moveToEnd || !staticNoMoveToEndPaste)
   {
    for (s=p,i=0,x=curPos.y,inFirstLine=1,chars=0; (unsigned)i<length; i++,s++,chars++)
       {
        // Is a line feed?
        if (*s=='\n')
          {
           // Is the first line?
           if (inFirstLine)
             {
              // Adjust the len of the first line
              lenLines.set(x,lenLines[x]+chars+1);
              // No more this
              inFirstLine=0;
              // Put the rest of the chars in the following line inserting one
              lenLines.insert(x+1,lar+CRLFLenOfTheLine-pF);
             }
           else
              // Only insert a line
              lenLines.insert(x,chars);
           curLinePtr+=lenLines[x++]; // Move the pointer and the cursor
           curPos.x=0;
           chars=0;
           totalLines++;
          }
        else
          if (*s=='\t')
             MoveWithTab(curPos.x);
          else
             if (CLY_IsntEOL(*s))
                curPos.x++;
       }
    curPos.y=x;
    limit.y=totalLines+1;
   }
 else
   {
    for (s=p,i=0,x=curPos.y,inFirstLine=1,chars=0; (unsigned)i<length; i++,s++,chars++)
       {
        // Is a line feed?
        if (*s=='\n')
          {
           // Is the first line?
           if (inFirstLine)
             {
              // Adjust the len of the first line
              chars+=lenLines[x]+1;
              lenLines.set(x,chars);
              // No more this
              inFirstLine=0;
              // Put the rest of the chars in the following line inserting one
              lenLines.insert(x+1,lar+CRLFLenOfTheLine-pF);
             }
           else
             // Only insert a line
             lenLines.insert(x,chars);
           // Update the maximun width
           if ((int)chars>limit.x)
              limit.x=chars;
           x++;
           chars=0;
           totalLines++;
          }
       }
    // Update the maximun width
    if ((int)chars>limit.x)
       limit.x=chars;
    limit.y=totalLines+1;
   }
 // There are characters added to the last line?
 if (chars)
    if (inFirstLine)
      {// Only a little text inside the line
       lenLines.set(x,lar+CRLFLenOfTheLine+extraSpaces+chars);
      }
    else
       lenLines.set(x,lenLines[x]+chars-1);

 // If needed adjust the SpecialLines array
 if (SpecialLines)
   {
    int fromLine=(int)firstTouchedLine+1;      // Included
    int toLine=x+1;                     // Not Included
    int dif=toLine-fromLine,i;
    if (IncludeFirstLine)
       fromLine--;
    int cnt=SpecialLines->getCount();
    for (i=0; i<cnt; i++)
       {
        stSpLine *p=SpecialLines->At(i);
        if (p->nline>=fromLine)
           p->nline+=dif;
       }
   }

 UpdateSyntaxHLBlock(firstTouchedLine,firstTouchedP,x);

 if (!isClipboard())
    MarkAsModified();
 update(ufView);

 return True;
}

/**[txh]********************************************************************

  Description:
  Copy the selection to a buffer (b). Stops in the first '\n'. The size is
limited by l. The string is ASCIIZ. The routine will use l bytes plus one
for the 0 if necesary.
@p
  Is used to copy from the clipboard to a TInputLinePiped object.

  Return: The number of bytes copied to the buffer.

***************************************************************************/

unsigned TCEditor::CopySelToBuffer(char *b, unsigned l)
{
 unsigned copied=0;
 if (hasSelection())
   {
    char *start=buffer+selStart;
    char *end=buffer+selEnd;

    for (;copied<l && CLY_IsntEOL(*start) && start<end; copied++, start++, b++)
        *b=*start;
    *b=0;
   }
 return copied;
}

/****************************************************************************

   Function: void deleteRangeLineInEdition(char *from,char *to,int x);

   Type: TCEditor member.

   Objetive: Delete a piece of the buffer in edition.
   Includes from but not to, deletes to-from chars.

   Parameters:
   from: From where.
   to: To where.
   x: -1 => No change, else the new x position.

   This routine must be finished.

   by SET.

****************************************************************************/

void TCEditor::deleteRangeLineInEdition(char *from,char *to,int x)
{
 if (isReadOnly)
    return;
 addToUndo(undoPreDelete,from);
 if (x>=0)
    curPos.x=x;
 addToUndo(undoDelete,to);
 memcpy(from,to,restCharsInLine+1);

 // Update markers
 int start=(int)(from-bufEdit);
 int end=(int)(to-bufEdit);
 int dif=end-start,i;
 for (i=0; i<10; i++)
    {
     int Pos=MarkersInLine[i];
     if (Pos>=0)
       {
        if (Pos>=start && Pos<end)
           MarkersInLine[i]=start;
        else
          if (Pos>=end)
             MarkersInLine[i]-=dif;
       }
    }
 AdjustLineSel(start,-dif,True,x>=0 ? True : False); // I'm not sure about the True
 // The x>=0 is a little complex, if x<0 (-1) the cursor isn't moved so we are
 // deleting to the right of the line, so the toLeft parameter MUST be False.
 // ^T and ^QY deletes the right part and pass -1, ^QH deletes the left part.
 MarkAsModified();
 update(ufLine);
}

/****************************************************************************

   Function: void deleteRange(char *from,char *to,Boolean allowUndo)

   Type: TCEditor member.

   Objetive: Delete a piece of the buffer.
   Includes from but not to, deletes to-from chars.

   Parameters:
   from: From where.
   to: To where.
   allowUndo: True if the action is recorded in the undo array.

   by SET.

****************************************************************************/

void TCEditor::deleteRange(char *from,char *to, Boolean allowUndo)
{
 if (isReadOnly || from>=to)
    return;
 CacheSyntaxHLData(GenericSHL);
 showMatchPairFlyCache=NULL;

 // Fix to pointer if that's outside the buffer
 if ((unsigned)(to-buffer)>bufLen)
    to=buffer+bufLen;

 char *fromOrig=from;
 // If the block will let the end of the current line exposed see if
 // there are spaces at the end and eat it.
 if (!DontPurgeSpaces && (*to=='\r' || *to=='\n' || !*to))
   {
    while (from!=buffer)
      {
       --from;
       if (*from!=' ' && *from!='\t')
         {
          from++;
          break;
         }
      }
   }

 if (allowUndo)
    addToUndo(undoPreDelete,from);

 // Put the cursor in "from"
 int y=0,x=0;
 char *pos=buffer;
 while (pos<=from)
    pos+=lenLines[y++];
 curPos.y=y-1;
 curLinePtr=pos-lenLines[curPos.y];
 pos=curLinePtr;
 while (pos!=fromOrig)
   {
    AdvanceWithTab(*pos++,x);
   }
 curPos.x=x;
 int IncludeFirstLine = x==0;

 // If the section invalidates the drawPtr force a full recalculation
 if (drawPtr>=selStart)
    drawLine=drawPtr=0;

 // Correct the line lengths
#if 0
 // Old version
 for (pos=from,y=curPos.y; pos<to; pos++)
    {
     lenLines.elArray[y]--;
     if (*pos=='\n')
       {
        if ((unsigned)y<totalLines)
          {
           lenLines.elArray[y]+=lenLines.elArray[y+1];
           lenLines.del(y+1);
          }
        else
          {
           if (curPos.y)
              curLinePtr-=lenLines[--curPos.y];
          }
        if (totalLines)
           totalLines--;
        limit.y=totalLines+1;
       }
    }
#else
 // New optimized one
 y=curPos.y;
 uint32 lenOfThisLine=lenLines[y];
 uint32 nextLine=y+1;
 for (pos=from; pos<to; pos++)
    {
     lenOfThisLine--;
     if (*pos=='\n')
       {
        if ((unsigned)y<totalLines)
          {
           lenOfThisLine+=lenLines[nextLine++];
          }
        else
          {
           if (curPos.y)
              curLinePtr-=lenLines[--curPos.y];
          }
        if (totalLines)
           totalLines--;
       }
    }
 lenLines.set(y,lenOfThisLine);
 if ((nextLine-y)>1)
   { // If we need to delete lines
    lenLines.deleteRange(y+1,nextLine-1);
    // If needed adjust the SpecialLines array
    if (SpecialLines)
      {
       int fromLine=y+1;      // Included
       int toLine=nextLine;   // Not Included
       int l,dif=toLine-fromLine,i;
       if (IncludeFirstLine)
          fromLine--;
       if (*(pos-1)=='\n')
          toLine--;
       int cnt=SpecialLines->getCount();
       for (i=0; i<cnt; i++)
          {
           stSpLine *p=SpecialLines->At(i);
           l=p->nline;
           if (l>=toLine)
              p->nline-=dif;
           else
              if (l>=fromLine)
                 p->nline=-1;
          }
      }
   }
 limit.y=totalLines+1;
#endif

 // Correct the Markers
 {
  int i;
  unsigned ToPoint=(unsigned)(to-buffer);
  unsigned FromPoint=(unsigned)(from-buffer);
  for (i=0; i<10; i++)
     {
      if (Markers[i]>ToPoint)
         Markers[i]-=ToPoint-FromPoint;
      else
         if (Markers[i]>FromPoint)
            Markers[i]=FromPoint;
     }
 }

 if (allowUndo)
    addToUndo(undoDeleteBuf,to);

 // do the work
 CLY_memcpy(from,to,(size_t)(bufLen-(to-buffer)));
 bufLen-=(unsigned)(to-from);

 // Correct the syntax of the line and test for propagation
 {
  uint32 attr;
  char *s=curLinePtr;
  uint32 y=curPos.y;

  // get the previous line attr
  if (y)
     attr=lenLines.getAttr(y-1);
  else
     attr=0;
  // recalculate the attr of the actual (modified) line
  s+=LineMeassure(s,s+lenLines[y],attr,NULL);
  lenLines.setAttr(y,attr);
  // There are more lines?
  if ((uint32)curPos.y<totalLines)
    {
     // Yes, recalculate the following line
     s+=LineMeassure(s,s+lenLines[++y],attr,NULL);
     // Test if there are a propagation
     TestPropagation(lenLines.getAttr(y),attr,s,y+1);
     // Set the recalculated value
     lenLines.setAttr(y,attr);
    }
 }

 if (hasSelection())
   {
    uint32 pos1=(uint32)(from-buffer);
    unsigned pos2=(unsigned)(to-buffer);
    unsigned diff=pos2-pos1;
    if (pos1<selStart)
      {
       if (pos2<=selStart)
         { // pos1<selStart && pos2<=selEnd
          selStart-=diff;
          selEnd-=diff;
         }
       else
         {
          if (pos2>selEnd) // All is inside
             selEnd=selStart=0;
          else
            { // a part
             selStart=pos1;
             selEnd-=diff;
            }
         }
      }
    else
      { // >= selStart
       if (pos1<selEnd)
         {
          if (pos2<=selEnd)
             selEnd-=diff;
          else
             selEnd=pos1;
         }
      }
   }

 MarkAsModified();
 update(ufView);
}


/****************************************************************************

   Function: int LineWidth()

   Type: TCEditor member.

   Objetive: Compute the length of the current line, taking care about tabs.

   Return: The length.

   by SET.

****************************************************************************/

int TCEditor::LineWidth()
{
 char *s;
 int x,off,lar=LenWithoutCRLF(curPos.y,curLinePtr);

 for (s=curLinePtr,x=off=0; off<lar; off++,s++)
    {
     AdvanceWithTab(*s,x);
    }
 return x;
}

/****************************************************************************

   Function: int LineWidth(char *s, char *d)
   
   Type: TCEditor member.

   Objetive: Compute the length of the current line, taking care about tabs.

   Parameters:
   s: pointer to the start of the line.
   d: pointer to the end of the line.

   Return: The length.

   by SET.

****************************************************************************/

int TCEditor::LineWidth(char *s, char *d)
{
 int lar=(int)(d-s);
 int x,off;

 for (x=off=0; off<lar; off++,s++)
    {
     AdvanceWithTab(*s,x);
    }
 return x;
}


/****************************************************************************

   Function: Boolean insertFrom( TEditor *editor )

   Type: TCEditor member.

   Objetive: Insert the selected text of another editor in this editor.

   Return: True = OK.

   From Borland's TV 1.03.

****************************************************************************/

Boolean TCEditor::insertFrom( TCEditor *editor )
{
 return insertBuffer( editor->buffer,
                      editor->selStart,
                      editor->selEnd - editor->selStart,
                      canUndo,
                      True,
                      False
                    );
}

/****************************************************************************

   Function: Boolean insertText( const void *text, unsigned length,
                                 Boolean selectText )

   Type: TCEditor member.

   Objetive: Insert text from another buffer.

   Parameters:
   text: source buffer.
   length: of the text.
   selectText: if the text will be selected after the operation.

   Return: True = OK.

   From Borland's TV 1.03.

****************************************************************************/

Boolean TCEditor::insertText( const void *text, unsigned length, Boolean selectText )
{
 return insertBuffer( (char *)text, 0, length, canUndo, selectText);
}

/****************************************************************************

   Function: void insertSpaces( unsigned length, Boolean canUseTabs )

   Type: TCEditor member.

   Objetive: Insert some spaces in the text.

   Parameters:
   length: number of spaces to insert.
   canUseTabs: True if tabs can be used.

   Note: Don't call this function when there is a line in edition.

   by SET.

****************************************************************************/

void TCEditor::insertSpaces( unsigned length, int X1, Boolean canUseTabs )
{
 if (OptimalFill && canUseTabs)
   {
    unsigned l=CalcNeededCharsToFill(X1,X1+length,tabSize,OptimalFill);
    if (AdjustBufEditFor(l))
       return;
    FillGapInBuffer(X1,X1+length,bufEdit,tabSize,OptimalFill);
    insertText(bufEdit,l,False);
   }
 else
   {
    if (AdjustBufEditFor(length))
       return;
    memset(bufEdit,' ',length);
    insertText(bufEdit,length,False);
   }
}

/****************************************************************************

   Function: uint32 lineMove( uint32 p, int count )

   Type: TCEditor member.

   Objetive: Move the cursor to a position, based on an offset plus a number
   of lines.

   Parameters:
   p: offset of the origin.
   count: number of lines from this point.

   Return:
   The new offset of the cursor in the buffer.

   by SET.

****************************************************************************/

uint32 TCEditor::lineMove( uint32 p, int count )
{
 GotoOffSet(p);
 MoveCursorTo(curPos.x,curPos.y+count);
 return (uint32)(ColToPointer()-buffer);
}



/****************************************************************************

   Function: static int AnalizeLineForIndent(char *s,int x,Boolean &mu,int l,
                                             int tabSize, int avail)

   Type: Normal function.

   Objetive: That's the Intelligent C indent function.

   by SET.

****************************************************************************/

static int AnalizeLineForIndent(char *s,int x,Boolean &mu,int l, int tabSize,
                                int avail)
{
 int Xlocal=x;
 char *ori=s;

 if (*s=='/')
   {
    l--; s++; Xlocal++;
    // If C++ comment ret x.
    if (!l || *s=='/')
       return x;
    // If C comment eat it
    if (*s=='*')
      {
       l--; s++; Xlocal++;
       while (l)
         {
          if (*s=='*')
            {
             l--; s++; Xlocal++;
             if (*s=='/')
                break;
            }
          if (l)
            {
             AdvanceWithTab(*s,Xlocal);
             l--; s++;
            }
         }
       // If the comment continues in the next line ret x.
       if (!l)
          return x;
       // eat all the spaces
       do
        {
         AdvanceWithTab(*s,Xlocal);
         l--; s++;
        }
       while (l && ucisspace(*s));
       if (!l)
          return x;
      }
    else
      return x; // If is only / ret x.
   }

 char *startWord=s;
 int lenWord=0;
 int xStartWord=Xlocal;

 // Get the first word in the line
 while (l && isWordChar(*s))
   {
    lenWord++;
    l--; s++; Xlocal++;
   }

 // Special cases
 if (!lenWord)
   {
    if (*s=='{')
       return Xlocal+1;
    if (*s=='}' && Xlocal)
      {
       mu=True;
       return x;
      }
   }

 // eat spaces
 while (l && ucisspace(*s))
   {
    AdvanceWithTab(*s,Xlocal);
    l--; s++;
   }

 // Analize: 1) The balance of (
 //          2) The last usefull char
 //          3) The column of the first ( , in fact the first non-blank after
 int numPar=0;
 char lastUseful=0;
 int xFirstPar=-1;
 while (l)
   {
    switch (*s)
      {
       case '(':
            numPar++;
            lastUseful=*s;
            if (xFirstPar<0)
              {
               while (l>1 && ucisspace(*(s+1)))
                 {
                  AdvanceWithTab(*s,Xlocal);
                  l--; s++;
                 }
               xFirstPar=Xlocal+1;
              }
            break;

       case ')':
            numPar--;
            lastUseful=*s;
            break;

       case '\"':
            do
             {
              AdvanceWithTab(*s,Xlocal);
              l--; s++;
              if (*s=='\\')
                {
                 l--; s++; Xlocal++;
                }
              else
                if (*s=='\"')
                  {
                   lastUseful=*s;
                   break;
                  }
             }
            while (l);
            break;

       case '\'':
            do
             {
              AdvanceWithTab(*s,Xlocal);
              l--; s++;
              if (*s=='\\')
                {
                 l--; s++; Xlocal++;
                }
              else
                if (*s=='\'')
                  {
                   lastUseful=*s;
                   break;
                  }
             }
            while (l);
            break;

       case '/':
            if (l>1 && s[1]=='/')
              { // EOL comment
               l=0;
               break;
              }
            if (l>1 && s[1]=='*')
              { // C comment
               l--; s++; Xlocal++; // Skip the *
               do
                 {
                  l--; s++;
                  AdvanceWithTab(*s,Xlocal);
                  if (l && *s=='*')
                    {
                     l--; s++; Xlocal++; // Skip the *
                     if (*s=='/')
                        break;
                    }
                 }
               while (l);
              }
            break;

       default:
            if (!ucisspace(*s))
               lastUseful=*s;
      }
    if (l)
      {
       AdvanceWithTab(*s,Xlocal);
       l--; s++;
      }
   }

 // Here we have a lot of information about the line

 // More ( than ), then goto this col
 if (numPar>0)
    return xFirstPar;

 //   More ) than (, then try to find the command that started all, is
 // heuristic there is no way to make that perfect.
 if (numPar<0)
   {
    // Search the ( than balance the line
    --ori;
    while (avail)
      {
       if (*ori=='(')
         {
          numPar++;
          if (!numPar)
             break;
         }
       else
         if (*ori==')')
            numPar--;
       avail--;
       ori--;
      }
    if (numPar) // unbalanced
      {
       mu=True;
       return x;
      }

    // Now continue until we reach the start of the line
    char *LastUsed=ori;
    --ori;
    --avail;
    while (avail)
      {
       if (*ori=='\n')
          break;
       if (isWordChar(*ori))
          LastUsed=ori;
       if (*ori=='(')
          numPar++;
       else
         if (*ori==')')
            numPar--;
       ori--;
       avail--;
      }

    // The whole line balances it?
    if (numPar)
      { // Nope
       return x;
      }

    // Now try to find the first word
    startWord=LastUsed;
    s=LastUsed;
    lenWord=0;
    xStartWord=0;

    while (isWordChar(*s))
      {
       lenWord++;
       s++;
      }

    // Is a word?
    if (!lenWord)
      { // Nope
       return x;
      }

    // Now we have the word so what's the X position?
    ori++;
    while (ori<LastUsed)
      {
       AdvanceWithTab(*ori,xStartWord);
       ori++;
      }
    x=xStartWord;
   }

 // To help in the next
 int notHaveColon = (lastUseful!=';');

 // Now about the first word
 // A pseudo hand-made hash
 switch (lenWord)
   {
    case 2: if (notHaveColon && ((*startWord=='d' && startWord[1]=='o') ||
                (*startWord=='i' && startWord[1]=='f')))
               return xStartWord+2;
            break;

    case 3: if (notHaveColon && *startWord=='f' && startWord[1]=='o' &&
                startWord[2]=='r')
               return xStartWord+3;
            break;

    case 4: if (*startWord=='e')
              {
               if (strncmp(startWord+1,"lse",3)==0)
                  return xStartWord+2;
               break;
              }
            if (*startWord=='c')
              {
               if (strncmp(startWord+1,"ase",3)==0)
                  return xStartWord+5;
               break;
              }
            break;

    case 5: if (notHaveColon && *startWord=='w')
              {
               if (strncmp(startWord+1,"hile",4)==0)
                  return xStartWord+2;
               break;
              }
            if (*startWord=='b')
              {
               if (strncmp(startWord+1,"reak",4)==0)
                 {
                  mu=True;
                  return x;
                 }
               break;
              }
            break;

    case 6: if (*startWord=='r')
              {
               if (strncmp(startWord+1,"eturn",5)==0)
                 {
                  mu=True;
                  return x;
                 }
               break;
              }
            if (*startWord=='s')
              {
               if (strncmp(startWord+1,"witch",5)==0)
                  return xStartWord+2;
               break;
              }
            break;

    case 7: if (*startWord=='d')
              {
               if (strncmp(startWord+1,"efault",6)==0)
                  return xStartWord+5;
               break;
              }
            break;
   }

 return x;
}


/****************************************************************************

   Function: void newLine()

   Type: TCEditor member.

   Objetive: Put a 13+10 in the buffer.
             Autoindenta.

   by SET.

****************************************************************************/

void TCEditor::newLine()
{
 if (isReadOnly)
    return;
 if (IslineInEdition) // This can be optimized
    MakeEfectiveLineInEdition();
 ClearSelIfNonPers();

 // Autoindent looks in the line before the ENTER, fuck!
 unsigned larThis=0,larAnt=0,firstUsedCol=0;
 char *prevLine=curLinePtr,*firstUsedPos=curLinePtr;
 if (!intelIndent && autoIndent)
   {
    int lineAnalize=curPos.y;

    larAnt=larThis=lenLines[lineAnalize];
    while (lineAnalize>=0)
     {
      if (larAnt>CLY_LenEOL)
        { // This line seems to have something, check it
         firstUsedPos=prevLine;
         while (*firstUsedPos==' ' || *firstUsedPos=='\t')
           {
            AdvanceWithTab(*firstUsedPos,firstUsedCol);
            firstUsedPos++;
           }
         if (*firstUsedPos!='\n' && *firstUsedPos!='\r')
            break; // if really contains something stop searching
        }
      firstUsedCol=0;
      if (lineAnalize)
        {
         larAnt=lenLines[lineAnalize-1];
         prevLine-=larAnt;
        }
      lineAnalize--;
     }
   }

 insertText(CLY_crlf,CLY_LenEOL,False);

 if (intelIndent && curPos.y>0)
   {
    // analize the last line
    unsigned firstUsedCol=0,firstColHere;
    int i;
    char *firstUsedPos,*firstUsedHere;

    larThis=lenLines[curPos.y];
    //   The following code searchs the first line located before than the
    // actual line that contains at least 1 character
    prevLine=curLinePtr;
    i=1;
    do
     {
      larAnt=lenLines[curPos.y-i];
      prevLine-=larAnt;
      i++;
     }
    while (i<=curPos.y && larAnt<=CLY_LenEOL);
    firstUsedPos=prevLine;

    //   This code search the position of the first used char and your
    // Column.
    while (*firstUsedPos==' ' || *firstUsedPos=='\t')
      {
       AdvanceWithTab(*firstUsedPos,firstUsedCol);
       firstUsedPos++;
       larAnt--;
      }

    // Move the cursor to the first char
    firstColHere=0;
    firstUsedHere=curLinePtr;

    // (The same but in the current line)
    while (*firstUsedHere==' ' || *firstUsedHere=='\t')
      {
       AdvanceWithTab(*firstUsedHere,firstColHere);
       firstUsedHere++;
      }
    //addToUndo(undoInMov);
    curPos.x=firstColHere;

    unsigned TargetCol=firstUsedCol;
    Boolean makeUnIndent=False;

    TargetCol=AnalizeLineForIndent(firstUsedPos,firstUsedCol,makeUnIndent,
                                   larAnt-CLY_LenEOL,tabSize,(int)(firstUsedPos-buffer));

    // Avoid a backspace at the start of the line
    if (!TargetCol && makeUnIndent)
       makeUnIndent=False;

#if 0
    int haveColon=*(curLinePtr-3)==';';

    if (*firstUsedPos=='{')
       TargetCol++;
    else
       if (((strncmp(firstUsedPos,"do",2)==0 || strncmp(firstUsedPos,"while",4)==0 ||
            strncmp(firstUsedPos,"if",2)==0) && !haveColon) ||
            strncmp(firstUsedPos,"else",4)==0 || strncmp(firstUsedPos,"switch",5)==0)
         {
          TargetCol+=2;
         }
       else
         if (strncmp(firstUsedPos,"case",4)==0 || strncmp(firstUsedPos,"default",7)==0)
           {
            /* Old: Is inpractic for switchs with labels
            while (*firstUsedPos!=':' && *firstUsedPos!='\n')
              {
               TargetCol++;
               firstUsedPos++;
              }
            if (*firstUsedPos==':')
              {
               do
                 {
                  TargetCol++;
                  firstUsedPos++;
                 }
               while (*firstUsedPos==' ' || *firstUsedPos=='\t');
              }*/
            TargetCol+=5;
           }
         else
           if (strncmp(firstUsedPos,"break",5)==0 || strncmp(firstUsedPos,"return",6)==0)
              makeUnIndent=True;
           else
             if (*firstUsedPos=='}' && TargetCol!=0)
                makeUnIndent=True;
             else
                if (strncmp(firstUsedPos,"for",3)==0 && !haveColon)
                   TargetCol+=3;
#endif

    if ((unsigned)curPos.x<TargetCol)
      {
       if (larThis>CLY_LenEOL)
         { // The line have chars
          insertSpaces(TargetCol-curPos.x,curPos.x);
         }
       else
         {// Is empty, only move the cursor
          curPos.x=TargetCol;
         }
      }
    if (makeUnIndent)
      {
       Boolean oldUseTabs=UseTabs;
       Boolean oldBackSpUnindents=BackSpUnindents;
       UseTabs=False; // Force the unindent even whe the user isn't in the
                      // rigth mode.
       BackSpUnindents=True;
       BackSpace();
       UseTabs=oldUseTabs;
       BackSpUnindents=oldBackSpUnindents;
      }
   }
 else
    if (autoIndent)
      {
       unsigned firstColHere;
       char *firstUsedHere;

       // Move the cursor to the first char
       firstColHere=0;
       firstUsedHere=curLinePtr;
   
       while (*firstUsedHere==' ' || *firstUsedHere=='\t')
         {
          AdvanceWithTab(*firstUsedHere,firstColHere);
          firstUsedHere++;
         }
       //addToUndo(undoInMov);
       curPos.x=firstColHere;
   
       if ((unsigned)curPos.x<firstUsedCol)
         {
          if (larThis>CLY_LenEOL)
            { // The line have chars
             insertSpaces(firstUsedCol-curPos.x,curPos.x);
            }
          else
            { // Is empty, only move the cursor
             curPos.x=firstUsedCol;
            }
         }
      }
}

/****************************************************************************

   Function: char *ColToPointer()

   Type: TCEditor member.

   Objetive: Returns a pointer to the "closest" position to curPos.x in the
   buffer.
   Note: more than one col reports the same pointer if the X pos is in a tab.
   Note: if the cursor is outside the buffer the pointer is to the \n.

   Return:
   A char pointer to the cursor position in the buffer.

   by SET.

****************************************************************************/

char *TCEditor::ColToPointer()
{
 char *s=curLinePtr,*end=buffer+bufLen;
 int x,xDest=curPos.x;

 for (x=0; CLY_IsntEOL(*s) && x<xDest && s<end; s++)
    {
     AdvanceWithTab(*s,x);
    }
 return s;
}

/****************************************************************************

   Function: char *ColToPointer(int &Dif)

   Type: TCEditor member.

   Objetive: Is a variant of ColToPointer()
   Dif is the difference between the point gived and the point desired.
   If Dif is negative the cursor was beyond the end of line
   If Dif is positive the cursor was in a tab

   Return:
   A char pointer to the cursor position in the buffer.

   by SET.

****************************************************************************/

char *TCEditor::ColToPointer(int &Dif)
{
 char *s=curLinePtr;
 int x,xDest=curPos.x;

 for (x=0; CLY_IsntEOL(*s) && x<xDest; s++)
    {
     AdvanceWithTab(*s,x);
    }
 Dif=x-xDest;
 return s;
}

/****************************************************************************

   Function: char *ColToPointerPost()

   Type: TCEditor member.

   Objetive: Is another variant of ColToPointer().
   The pointer is to the POSITION BEFORE THE \xA
                         ^^^^^^^^^^^^^^^^^^^
                         Some time ago this text was missing and was a
                         real funny bug.

   Return:
   A char pointer to the cursor position in the buffer.

   by SET.

****************************************************************************/

char *TCEditor::ColToPointerPost()
{
 char *s=curLinePtr;
 int x,xDest=curPos.x;

 for (x=0; CLY_IsntEOL(*s) && x<xDest; s++)
    {
     AdvanceWithTab(*s,x);
    }
 return s;
}

/**[txh]********************************************************************

  Description:
  This routine returns the x column for the character at the right of the
one above the cursor. If we are inside a tab it is != curPos.x-1.

***************************************************************************/

int TCEditor::PosLeftChar()
{
 char *s=curLinePtr,*end=buffer+bufLen;
 int x,xDest=curPos.x,xold;

 for (x=0,xold=0; CLY_IsntEOL(*s) && x<xDest && s<end; s++)
    {
     AdvanceWithTab(*s,x);
     if (x<xDest)
        xold=x;
    }
 if (x<xDest)
    return xDest-1;
 return xold;
}

/**[txh]********************************************************************

  Description:
  It returns curPos.x if the cursor isn't inside the tab. If not the column
returned is the one of the start of the tab.

***************************************************************************/

int TCEditor::FixPosCharLeft()
{
 char *s=curLinePtr,*end=buffer+bufLen;
 int x,xDest=curPos.x,xold;

 for (x=0,xold=0; CLY_IsntEOL(*s) && x<xDest && s<end; s++)
    {
     AdvanceWithTab(*s,x);
     if (x<=xDest)
        xold=x;
    }
 if (x<xDest)
    return xDest;
 return xold;
}

/****************************************************************************

   Function: void SetStartOfSelecting(uint32 startOffSet)

   Type: TCEditor member.

   Objetive: Set the start of the selected area.

   Parameter:
   startOffSet: new start offset.

   by SET.

****************************************************************************/

void TCEditor::SetStartOfSelecting(uint32 startOffSet)
{
 int selVisible=!selHided && hasSelection();

 if (startOffSet>bufLen)
    startOffSet=bufLen;
 if (!(selVisible && (startOffSet==selEnd || startOffSet==selStart)))
   {
    selStartOffSet=startOffSet;
    if (selVisible)
       update(ufView);
    else
       update(ufLine);
    selHided=False;
    // Make the selected area NULL because if the start of selection forced a
    // line flush we will repaint the screen but with selHided==False so the
    // old selection will be visible for a fraction of a second, that's annoying
    selEnd=selStart=selStartOffSet;
   }
 selecting=True;
}

/****************************************************************************

   Function: void UpdateSelecting(void)

   Type: TCEditor member.

   Objetive: Update the selected area according to the new position of the
   cursor.

   by SET.

****************************************************************************/

void TCEditor::UpdateSelecting(void)
{
 uint32 actualPos=(uint32)(ColToPointerPost()-buffer);

 if (actualPos>selStartOffSet)
   {
    selStart=selStartOffSet;
    selEnd=actualPos;
   }
 else
   {
    selStart=actualPos;
    selEnd=selStartOffSet;
   }
 // Sanity check
 if (selStart>bufLen)
    selStart=bufLen;
 if (selEnd>bufLen)
    selEnd=bufLen;

 if (TVOSClipboard::isAvailable()>1)
    clipWinCopy(1);
}

/****************************************************************************

   Function: void MoveCursor(char *ori,char *dest)

   Type: TCEditor member.

   Objetive: Moves the cursor from one point of the buffer to other based on
   pointers to this location, ori is the actual position of the cursor.
     The funtion computes the \n and the tabs.

   Parameters:
   ori: Pointer to the cursor position.
   dest: Pointer to the new cursor position.

   by SET.

****************************************************************************/

void TCEditor::MoveCursor(char *ori,char *dest)
{
 if (ori>dest)
   {
    while (ori>dest)
      {
       ori--;
       if (*ori=='\n')
          curLinePtr-=lenLines[--curPos.y];
      }
    char *s=curLinePtr;
    curPos.x=0;
    while (s!=dest)
      {
       AdvanceWithTab(*s++,curPos.x);
      }
   }
 else
    while (dest!=ori)
      {
       switch (*ori++)
         {
          case '\n': curLinePtr+=lenLines[curPos.y++];
                     curPos.x=0;
                     break;
          case '\t': MoveWithTab(curPos.x);
                     break;
          case '\r': break;
          default:   curPos.x++;
         }
      }
}


/****************************************************************************

   Function: int prevWord()

   Type: TCEditor member.

   Objetive: Move the cursor to the previous word in the text.

   Return:
   How many characters was moved the cursor.

   by SET.

****************************************************************************/

int TCEditor::prevWord(Boolean moveCursor)
{
 char *p=ColToPointer();
 if (p==buffer)
    return 0;
 char *ori=p;

 if (isWordChar(*p) && isWordChar(*(p-1)))
   { // If we are inside a word (not at start or outside)
    // Move to the start
    while (p>buffer &&  isWordChar(*p)) p--;
    // Adjust
    if (p<ori) p++;
   }
 else
   {
    // Get out of the start of the word or the EOL or we'll stay here
    if (isWordChar(*p) || CLY_IsEOL(*p))
       p--;
    // Skip the spaces but stop in EOLs
    while (p>buffer && !isWordChar(*p) && CLY_IsntEOL(*p)) p--;
    if (isWordChar(*p))
      { // We found a word
       // Go to the start
       while (p>buffer &&  isWordChar(*p)) p--;
       // Adjust
       if (!isWordChar(*p)) p++;
      }
    else
       // We found an EOL so if is a DOS file move to the CR or it will bomb
       if (p>buffer && *(p-1)=='\r') p--;
   }
 if (p>ori)
    p=ori;
 else
    if (moveCursor)
       MoveCursor(ori,p);
 return (int)(p-ori);
}

/****************************************************************************

   Function: int nextWord()

   Type: TCEditor member.

   Objetive: Move the cursor to the next word in the text.

   Return:
   How many characters was moved the cursor.
   See nextCWord() for comments, it almost the same but uses !isWord...
   instead of isspace.

   by SET.

****************************************************************************/

int TCEditor::nextWord()
{
 int dif;
 char *p=ColToPointer(dif);
 char *ori=p;
 char *end=buffer+bufLen;

 if (p==end)
    return 0;

 if (dif>0)
   {
    ori--;
    p--;
   }

 if (CLY_IsEOL(*p))
   {
    if (!CLY_IsTrueEOL(*p))
       do { p++; } while (p<end && !CLY_IsTrueEOL(*p));
    p+=CLY_LenEOL;
    if (p>=end)
       return 0;
   }
 else
   {
    if (!isWordChar(*p))
       while (p<end && CLY_IsntEOL(*p) && !isWordChar(*p)) p++;
    else
       while (p<end && isWordChar(*p)) p++;
   }
 if (CLY_IsntEOL(*p))
    while (p<end && !isWordChar(*p) && CLY_IsntEOL(*p)) p++;
 if (p==end) p--;
 if (ori>p)
    p=ori;
 else
    MoveCursor(ori,p);
 return (int)(p-ori);
}

/****************************************************************************

   Function: int nextCWord()

   Type: TCEditor member.

   Objetive: Move the cursor to the next word in the text but stoping in
   any symbol.
     Used by DeleteWord
     Here is how it works:
   1 If we are at the end of line skip the EOL and goto 5.
   2 else If we are in a space go to 5.
   3 else If we are in a symbol eat just one and go to 5.
   4 else skip the word.
   5 If we aren't at the EOL skip all the SPACES but stop if an EOL is reached.

   What's the effect?
   a) If we are at the end of a line the line is concatenated with the next
      line but the spaces are deleted.
   b) Never delete a word+symbols in one pass.
   c) Never delete space and more than one symbol in one pass.
   d) Ever stop at EOLs to avoid deleting various lines.

   That's how BC behaves (or very close to ...) and that's what I spect.

   Return:
   How many characters was moved the cursor.

   30/9/97:
   I changed the behavior to get even more compatibility with the BC behavior. Now
   the symbols aren't eated so quickly.

   by SET.

****************************************************************************/

int TCEditor::nextCWord()
{
 int dif;
 char *p=ColToPointer(dif);
 char *ori=p;
 char *end=buffer+bufLen;

 if (p==end)
    return 0;

 // Count the tab later
 if (dif>0)
   {
    //ori--;
    p--;
   }

 if (CLY_IsEOL(*p))
   { // If we are at the end of the line delete the EOL
    p+=CLY_LenEOL;
    if (p>=end)
       return 0;
   }
 else
   { // If not:
    if (!ucisspace(*p))
      {
       if (isWordChar(*p))
          // If we are in a word delete up-to the end
          while (p<end && isWordChar(*p)) p++;
       else
          p++;
      }
   }
 // Eat ONLY the remaining spaces
 if (CLY_IsntEOL(*p))
    while (p<end && ucisspace(*p) && CLY_IsntEOL(*p)) p++;
 // Here are some safety checks that don't remember when are needed
 if (p==end) p--;
 if (ori>p)
    p=ori;
 else
    MoveCursor(ori,p);
 return (int)(p-ori);
}

/**[txh]********************************************************************

  Description:
  Moves the cursor to the end of the word. If the cursor isn't inside a word
isn't moved. Is highlevel (flushLine and undo).

  Return:
  The number of characters moved.

***************************************************************************/

int TCEditor::goEndWord()
{
 flushLine();

 int dif;
 char *p=ColToPointer(dif);
 char *ori=p;
 char *end=buffer+bufLen;

 if (p>=end || dif || !isWordChar(*p))
    return 0;

 while (p<end && isWordChar(*p)) p++;
 addToUndo(undoInMov);
 MoveCursor(ori,p);

 return p-ori;
}

void TCEditor::Beep(void)
{
 CLY_Beep();
}


/****************************************************************************

   Function: void AdjustBufEditFor(int lar)

   Type: TCEditor member.

   Objetive: Adjust the length of the bufEdit buffer to support lar chars.
   limit.x is adjusted too. The function reallocates the buffer for this
   reason inEditPtr is updated.

   Parameters:
   lar: Desired legth.

   Returns:
   0 = OK.
   1 = Impossible.

   by SET.

****************************************************************************/

int TCEditor::AdjustBufEditFor(int lar)
{
 if (lar>bufEditLen)
   {
    char *s=bufEdit;

    if (lar>MaxLineLen)
      {
       Beep();
       return 1;
      }
    if (limit.x<=lar)
       limit.x=lar+1;
    bufEditLen=limit.x+min(DeltaLineLen,MaxLineLen-limit.x);
    if ((s=(char *)realloc(bufEdit,bufEditLen+4))!=NULL)
      {
       inEditPtr=s+(int)(inEditPtr-bufEdit);
       bufEdit=s;
       return 0;
      }
    Beep();
    return 1;
   }
 return 0;
}

/**[txh]********************************************************************
  Description:
  Computes inEditPtr value for the current curPos.x value.
***************************************************************************/

int TCEditor::ComputeXLineInEdition(Boolean alsoRestChars)
{
 int i;
 // Calculate the position inside the buffer
 for (inEditPtr=bufEdit, i=0; i<curPos.x && *inEditPtr;
      inEditPtr++)
    {
     AdvanceWithTab(*inEditPtr,i);
    }
 if (i>curPos.x) // Only when the cursor is over a tab
    inEditPtr--;
 if (alsoRestChars)
    for (restCharsInLine=0; inEditPtr[restCharsInLine]; restCharsInLine++);
 return i;
}

/****************************************************************************

   Function: void EditLine()

   Type: TCEditor member.

   Objetive: Start the edition of the line under the cursor.

   by SET.

****************************************************************************/

void TCEditor::EditLine()
{
 int i;

 lineInEdition=curPos.y;

 // Copy the line into the edition buffer
 int lar=LenWithoutCRLF(lineInEdition,curLinePtr);

 unsigned startLine=(unsigned)(curLinePtr-buffer);
 unsigned endLine=startLine+lar;

 // See if we have enough buffer for this line.
 if (AdjustBufEditFor(max(lar,limit.x-1)))
   {
    if (lar==MaxLineLen+1)
       return;
    if (editorDialog( edLineOverflow )==cmYes)
      {
       int X=curPos.x;
       addToUndo(undoInMov);
       Boolean oldAI=autoIndent;
       autoIndent=False;
       curPos.x=MaxLineLen;
       newLine();
       MoveCursorTo(X,curPos.y-1,True);
       autoIndent=oldAI;
       if (AdjustBufEditFor(max(lar,limit.x-1)))
          return;
      }
    else
       return;
   }

 IslineInEdition=True;
 memcpy(bufEdit,curLinePtr,lar);
 bufEdit[lar]=0;
 i=ComputeXLineInEdition();

 // Is large enough to hold the cursor pos?
 int added=0;
 if (i<curPos.x)
   { // Nop, insert spaces
    added=CalcNeededCharsToFill(i,curPos.x,tabSize,OptimalFill);
    FillGapInBuffer(i,curPos.x,bufEdit+lar,tabSize,OptimalFill);
    inEditPtr=bufEdit+lar+added;
    lar+=added;
    *inEditPtr=0;
   }
 restCharsInLine=lar-(int)(inEditPtr-bufEdit);

 selNewStart=selStart;
 selNewEnd=selEnd;

 if (hasSelection())
   {
    // If we added spaces and the spaces are inside of the seleccion take care about it
    if (added && selEnd>endLine)
      {
       if (selStart>endLine)
         {
          //added++;
          selNewEnd+=added;
          selNewStart+=added;
         }
       else
          selNewEnd+=added;
      }
    if (selStart<=startLine)
      {
       if (selNewEnd>startLine) // No for New
         {
          selLineStart=0;
          selLineEnd=selNewEnd-startLine; // No for New
         }
       else
         { selLineStart=selLineEnd=0; }
      }
    else
      {
       if (selNewStart<endLine) // No for New
         {
          selLineStart=selNewStart-startLine;  // No for New
          selLineEnd=selNewEnd-startLine; // No for New
         }
       else
         { selLineStart=selLineEnd=lar+1; } // Out off the line
      }
   }
 else
   {
    selLineStart=0;
    selLineEnd=0;
   }

 // Translate the markers
 for (i=0; i<10; i++)
    {
     unsigned Pos=Markers[i];
     if (startLine<=Pos && Pos<endLine)
        MarkersInLine[i]=Pos-startLine;
     else
        MarkersInLine[i]=-1;
    }

 // Copy the original attribute
 attrInEdit=lenLines.getAttr(lineInEdition);
}

/****************************************************************************

   Function: void replace()

   Type: TCEditor member.

   Objetive: Create the dialog for Search & Replace and execute it.

   by SET.

****************************************************************************/

void TCEditor::replace()
{
 char *Word;
 int ret;

 if (isReadOnly) return;
 if ((Word=WordUnderCursor(80))!=NULL)
   {
    strcpy(findStr,Word);
    delete[] Word;
   }

 editorFlags&=~efOptimizedRegex;
 TReplaceCDialogRec replaceRec(findStr,replaceStr,editorFlags,SearchInSel,FromWhere);
 TRegexDialogRec regexRec;
 regexRecCreate(regexRec);

 if ((ret=editorDialog(edReplace,&replaceRec,&regexRec))!=cmCancel)
   {
    regexRecUpdate(regexRec);
    strcpy( findStr, replaceRec.find );
    strcpy( replaceStr, replaceRec.replace );
    editorFlags = replaceRec.options | efDoReplace;
    if (ret==cmYes)
        editorFlags |= efReplaceAll;
    SearchInSel = replaceRec.in_sel;
    FromWhere = replaceRec.from;
    if (FromWhere)
       StartOfSearch=0; // All
    else
       StartOfSearch=(unsigned)(ColToPointer()-buffer);
    if (CompileSearch(findStr,replaceStr))
       return;
    doSearchReplace();
   }
}

/****************************************************************************

   Function: void scrollTo( int x, int y )

   Type: TCEditor member.

   Objetive: Adjust the delta to make the x,y point visible.

   Parameters:
   x,y cursor position.

   From Borland's TV 1.03.

****************************************************************************/

void TCEditor::scrollTo( int x, int y )
{
 x = max(0, min(x, limit.x - size.x));
 y = max(0, min(y, limit.y));  // - size.y cutted
 if ( x != delta.x || y != delta.y )
   {
    delta.x = x;
    delta.y = y;
    update(ufView);
   }
}


/**[txh]********************************************************************

  Description:
  Finds what line contains the desired offset. The rest value returns the
offset inside the line.

  Return:
  The line number.

***************************************************************************/

int TCEditor::FindLineForOffSet(unsigned offset, unsigned &rest)
{
 unsigned offCursor=curLinePtr-buffer;
 unsigned here,thisLine;
 int y;
 if (offset>offCursor)
   {// Search from the cursor
    here=offCursor;
    y=curPos.y;
    do
      {
       thisLine=lenLines[y++];
       here+=thisLine;
      }
    while (here<=offset);
    rest=thisLine-(here-offset);
    return y-1;
   }
 if (offset<offCursor/2)
   {// Search from start
    here=0; y=0;
    do
      {
       thisLine=lenLines[y++];
       here+=thisLine;
      }
    while (here<=offset);
    rest=thisLine-(here-offset);
    return y-1;
   }
 // Search backwards from the cursor
 here=offCursor;
 y=curPos.y;
 while (y>0 && here>offset)
   {
    thisLine=lenLines[--y];
    here-=thisLine;
   }
 rest=offset-here;
 return y+1;
}


/**[txh]********************************************************************

  Description:
  Finds the syntax highlight for a character pointed by an offset in the
file.
  
  Return: The shl flags.
  
***************************************************************************/

uint32 TCEditor::SyntaxHighlightForOffset(unsigned offset)
{
 if (SyntaxHL==shlNoSyntax)
    return 0;
 unsigned rest;
 int y=FindLineForOffSet(offset,rest);
 uint32 attr=lenLines.getAttr(y);
 LineMeassure(buffer+offset-rest,buffer+offset,attr,NULL);
 return attr;
}

/**[txh]********************************************************************

  Description:
  Finds the extra SHL attributes for a position. We must know the pointer
to the line start, a pointer to the character and the line number.
  
  Return: The extra attributes (in string, comment and/or preprocessor).
  
***************************************************************************/

uint32 TCEditor::SyntaxHighlightExtraFor(char *lineStart, char *posTarget,
                                         int line)
{
 if (SyntaxHL==shlNoSyntax)
    return 0;
 uint32 attr=line ? lenLines.getAttr(line-1) : 0;
 uint32 extra;
 LineMeassure(lineStart,posTarget,attr,&extra);
 return extra;
}

/****************************************************************************

   Function: Boolean search( const char *findStr, unsigned opts )

   Type: TCEditor member.

   Objetive: Make a Search with/out Replace calling scan and iScan.

   Parameters:
   findStr: The string to search.
   opts: flags to indicate the action and type of search.

   by SET.

****************************************************************************/

Boolean TCEditor::search(const char *, unsigned opts)
{
 unsigned pos,end_s;
 unsigned i=sfSearchFailed;
 int MatchLen=0,scrllCenter;

 if (SearchInSel)
   {
    pos=max(StartOfSearch,selStart);
    end_s=selEnd;
   }
 else
   {
    pos=StartOfSearch;
    end_s=bufLen;
   }

 do
  {
   if (pos<end_s)
      i=MakeASearch(&buffer[pos],end_s-pos,MatchLen);

   if (i!=sfSearchFailed)
     {
      i+=pos;
      int takeThisHit=1;
      // Whole Words, the following can be expressed in one line, but I doubt gcc
      // will generate better code; what I'm sure is that then is very hard to understand
      if (opts & efWholeWordsOnly)
        {// The letter before the first character can't be another letter
         if (i && isWordChar(bufChar(i-1)))
            takeThisHit=0;
         else // The letter after the match can't be another letter
            if (i+MatchLen<bufLen && isWordChar(bufChar(i+MatchLen)))
               takeThisHit=0;
        }
      // In/Outside comments
      if (takeThisHit && SyntaxHL!=shlNoSyntax &&
          (opts & (efSearchInComm | efSearchOutComm)))
        {// Find the attributes of this point
         uint32 attr=SyntaxHighlightForOffset(i);
         if (attr & IsInsideCom)
            takeThisHit=opts & efSearchInComm;
         else
            takeThisHit=opts & efSearchOutComm;
        }
      if (takeThisHit)
         {
          lock();
          selStartF=i;
          selEndF=i+MatchLen;
          addToUndo(undoInMov);
          GotoOffSet(selEndF);
          scrllCenter=CheckScrollLockCenters;
          trackCursor((scrllCenter || !cursorVisible()) ? True : False);
          GotoOffSet(i);
          trackCursor((scrllCenter || !cursorVisible()) ? True : False);
          if (opts & efShowFuncName)
             ShowWhichFunInStatus();
          update(ufView | ufFound);
          unlock();
          return True;
         }
      else
          pos=i+1;
     }
   }
  while(i!=sfSearchFailed);
 return False;
}

/**[txh]********************************************************************

  Description:
  Performes a regex search and jumps to the hit. This function uses the
editor's search machinery and restore the previous values before returning.
  
  Return: !=0 if a hit found.
  
***************************************************************************/

Boolean TCEditor::SearchAndJump(char *text, unsigned flags)
{
 // Back-up current options
 unsigned oldEdFlags=editorFlags;
 ushort   oldSearchInSel=SearchInSel;
 ushort   oldFromWhere=FromWhere;
 ushort   oldRegExStyle=RegExStyle;
 unsigned oldStartOfSearch=StartOfSearch;

 // Create new search
 editorFlags=flags;
 CompileSearch(text);
 SearchInSel=0;
 FromWhere=1;
 RegExStyle=efBasicRegEx;
 StartOfSearch=0;
 Boolean ret=search(text,flags);

 // Restore normal search
 SearchInSel=oldSearchInSel;
 FromWhere=oldFromWhere;
 RegExStyle=oldRegExStyle;
 editorFlags=oldEdFlags;
 StartOfSearch=oldStartOfSearch;
 CompileSearch(findStr);

 return ret;
}

/****************************************************************************

   Function: unsigned LineMeassureC(char *s, char *end, uint32 &Attr)
             unsigned LineMeassurePascal(char *s, char *end, uint32 &Attr)
             unsigned LineMeassureClipper(char *s, char *end, uint32 &Attr)
             unsigned LineMeassureGeneric(char *s, char *end, uint32 &Attr)

   Type: Static function.

   Objetive: Meassure the length of the line and calculate the syntax
   highlight flags.
     There are one for each syntax highlight system.

   Parameters:
   char *s: Pointer to the start of the line.
   char *end: Pointer to the end of the buffer.
   uint16 &Attr: In => Attribute of the previous line.
                 Out => Attribute of this line.

   Returns:
   The length of the line.

   Notes:
   for C:
   Added support for strings that have a \ at the end of a line (by Robert).
   Added support for // \ feature of gcc.

   by SET.

****************************************************************************/


static
unsigned LineMeassureC(char *s, char *end, uint32 &Attr, uint32 *extra)
{
 uint32 l=0;
 char *end2=end-1;
 int in_char=0;
 int in_string=0;
 int in_com=0;
 int in_prepro=0;
 int firstchar=1;
 uint32 attr=Attr;

 if (attr & ExtCom)
   {
    attr|=InsideCom | StartInCom;
    attr&=Not_ExtCom2;
    in_com=1;
   }
 else
   if (attr & ExtCom2)
     {
      attr|=InsideCom | StartInCom;
      attr&=Not_ExtCom2;
      in_com=1;
     }
   else
      attr&=Not_InsideCom & Not_StartInCom; // If not clear that

 if (!(attr & ExtPrepro))
    attr&=Not_Prepro;
 else
    in_prepro=1;

 if (attr & ExtString)
    in_string = 1;

 attr&=FilterHere;

 if (in_string)
    attr|=StartString;

 while (s<end && *s!='\n')
   {
    if (*s=='\'')
      {
       if (!in_com && !in_string)
         {
          in_char=!in_char;
          firstchar=0;
         }
      }
    else
      if (*s=='\"')
        {
         if (!in_com && !in_char)
           {
            in_string=!in_string;
            firstchar=0;
           }
        }
      else
        // A line ended with \ is concatenated with the next line
        if (*s=='\\' && (s==end2 || CLY_IsEOL(*(s+1))))
          {
           if (in_string)
              attr|=ExtString;
           else
             if (in_com) /*(attr & (ComInside | InsideCom | StartInCom))*/
                attr|=ExtCom2;
             else
                if (in_prepro)
                   attr|=ExtPrepro;
          }
        else
           if (*s=='\\' && (in_string || in_char))
             {
              s++;
              l++;
             }
           else
              if (!in_string && !in_char)
                {
                 switch (*s)
                   {
                    case '/':
                         if (!in_com && s<end2)
                           {
                            s++;
                            if (*s=='/')
                              {
                               in_com=1;
                               attr|=ComInside;
                               if (firstchar)
                                  attr|=InsideCom;
                               l++;
                              }
                            else
                              if (*s=='*')
                                {
                                 in_com=1;
                                 attr|=StartCom | ExtCom;
                                 if (firstchar)
                                    attr|=InsideCom;
                                 l++;
                                }
                              else
                                --s;
                           }
                         firstchar=0;
                         break;

                    case '*':
                         if (s<end2)
                           {
                            s++;
                            if (*s=='/' && !(attr & ComInside) && (attr & (ExtCom | InsideCom)))
                              {
                               in_com=0;
                               attr|=EndCom;
                               attr&=Not_ExtCom & Not_InsideCom;
                               l++;
                              }
                            else
                              --s;
                           }
                         firstchar=0;
                         break;

                    case '#':
                         if (firstchar)
                           {
                            firstchar=0;
                            in_prepro=1;
                            attr|=Prepro;
                           }
                         break;

                    default:
                         if (firstchar && !ucisspace(*s))
                            firstchar=0;
                   }
                }
    ++l;
    ++s;
   }
 // I saw that in a source of John (aka Fighteer), the string is propagated.
 if (in_string)
   {
    attr|=ExtString;
    if (in_prepro)
       attr|=ExtPrepro;
   }
 // Comments of type /* */ with more than 1 line in a preprocessor line
 if (in_prepro && (attr & ExtCom))
    attr|=ExtPrepro;
 Attr=attr;
 if (extra)
   {
    *extra=0;
    if (in_string) *extra|=InString;
    if (in_char)   *extra|=InString2;
    if (in_com)    *extra|=InComment;
    if (in_prepro) *extra|=InPrepro;
   }
#if 0
 if (l && *s=='\n') l++;
#else
 // Touched by Robert, I don't know if it's safe
 if (s<end && *s=='\n') l++;
#endif
 return l;
}


static
unsigned LineMeassurePascal(char *s, char *end, uint32 &Attr, uint32 *extra)
{
 uint32 l=0;
 char *end2=end-1;
 int in_string=0;
 int in_com1=0;
 int in_com2=0;
 int in_prepro = 0;
 int firstchar=1;
 uint32 attr=Attr;

 if (attr & ExtCom)
   {
    attr|=InsideCom | StartInCom;
    in_com1=1;
   }
 else
 if (attr & ExtCom2)
   {
    attr|=InsideCom2 | StartInCom2;
    in_com2=1;
   }
 else
    attr&=Not_InsideCom & Not_StartInCom & Not_InsideCom2 & Not_StartInCom2;

 if (!(attr & ExtPrepro))
    attr&=Not_Prepro;
 else
    in_prepro=1;

 attr&=FilterHere;

 while (s<end && *s!='\n')
   {
    if (*s=='\'')
      {
       if (!in_prepro && !in_com1 && !in_com2)
         {
          in_string=!in_string;
          firstchar=0;
         }
      }
    else
      if (*s=='\\' && (s==end2 || CLY_IsEOL(*(s+1))))
       {
         if (in_prepro)
             attr|=ExtPrepro;
       }
    else
      if (!in_string)
        {
         switch (*s)
           {
            case '#':
                 if (firstchar)
                   {
                    firstchar=0;
                    in_prepro=1;
                    attr|=Prepro;
                   }
                 break;

            case '(':
                 if (!in_com1 && !in_com2 && s<end2)
                   {
                    s++;
                    if (*s=='*')
                      {
                       in_com1=1;
                       attr|=StartCom | ExtCom;
                       if (firstchar)
                          attr|=InsideCom;
                       l++;
                      }
                    else
                      --s;
                   }
                 firstchar=0;
                 break;

            case '{':
                 if (!in_com1 && !in_com2)
                   {
                    in_com2=1;
                    attr|=StartCom2 | ExtCom2;
                    if (firstchar)
                       attr|=InsideCom2;
                   }
                 firstchar=0;
                 break;

            case '*':
                 if (s<end2 && !in_com2)
                   {
                    s++;
                    if (*s==')' && !(attr & ComInside) && (attr & (ExtCom | InsideCom)))
                      {
                       in_com1=0;
                       attr|=EndCom;
                       attr&=Not_ExtCom & Not_InsideCom;
                       l++;
                      }
                    else
                      --s;
                   }
                 firstchar=0;
                 break;

            case '}':
                 if (!in_com1 && !(attr & ComInside) && (attr & (ExtCom2 | InsideCom2)))
                   {
                    in_com2=0;
                    attr|=EndCom2;
                    attr&=Not_ExtCom2 & Not_InsideCom2;
                   }
                 firstchar=0;
                 break;

            default:
                 if (firstchar && !ucisspace(*s))
                    firstchar=0;
           }
        }
    ++l;
    ++s;
   }
 Attr=attr;
 if (extra)
   {
    *extra=0;
    if (in_string) *extra|=InString;
    if (in_com1 ||
        in_com2)   *extra|=InComment;
    if (in_prepro) *extra|=InPrepro;
   }
#if 0
 if (l && *s=='\n') l++;
#else
 if (*s=='\n') l++;
#endif
 return l;
}



static
unsigned LineMeassureClipper(char *s, char *end, uint32 &Attr, uint32 *extra)
{
 uint32 l=0;
 char *end2=end-1;
 char *end3=end-3;
 int in_char=0;
 int in_string=0;
 int in_com=0;
 int in_prepro=0;
 int firstchar=1;
 uint32 attr=Attr;

 if (attr & ExtCom)
   {
    attr|=InsideCom | StartInCom;
    in_com=1;
   }
 else
    attr&=Not_InsideCom & Not_StartInCom;
 if (!(attr & ExtPrepro))
    attr&=Not_Prepro;
 else
    in_prepro=1;
 attr&=FilterHere;

 while (s<end && *s!='\n')
   {
    if (*s=='\'')
      {
       if (!in_prepro && !in_com && !in_string)
         {
          in_char=!in_char;
          firstchar=0;
         }
      }
    else
      if (*s=='\"')
        {
         if (!in_prepro && !in_com && !in_char)
           {
            in_string=!in_string;
            firstchar=0;
           }
        }
      else
        if (!in_string && !in_char)
          {
           switch (*s)
             {
              case '/':
                   if (!in_com && s<end2)
                     {
                      s++;
                      if (*s=='/')
                        {
                         in_com=1;
                         attr|=ComInside;
                         if (firstchar)
                            attr|=InsideCom;
                         l++;
                        }
                      else
                        if (*s=='*')
                          {
                           in_com=1;
                           attr|=StartCom | ExtCom;
                           if (firstchar)
                              attr|=InsideCom;
                           l++;
                          }
                        else
                          --s;
                     }
                   firstchar=0;
                   break;
                   
              case '&':
                   if (!in_com && s<end2)
                     {
                      s++;
                      if (*s=='&')
                        {
                         in_com=1;
                         attr|=ComInside;
                         if (firstchar)
                            attr|=InsideCom;
                         l++;
                        }
                      else
                        --s;
                     }
                   firstchar=0;
                   break;

              case '*':
                   if (s<end2)
                     {
                      s++;
                      if (*s=='/' && !(attr & ComInside) && (attr & (ExtCom | InsideCom)))
                        {
                         in_com=0;
                         attr|=EndCom;
                         attr&=Not_ExtCom & Not_InsideCom;
                         l++;
                        }
                      else
                        if (!(attr & ComInside) && !(attr & InsideCom) &&
                             s<end3 && *s==' ' && *(s+1)=='-')
                          {
                           in_com=1;
                           attr|=ComInside;
                           if (firstchar)
                              attr|=InsideCom;
                           s+=2;
                           l+=3;
                          }
                        else
                          --s;
                     }
                   firstchar=0;
                   break;

              case ';':
                   if ((s==end2 || CLY_IsEOL(*(s+1))) && in_prepro)
                      attr|=ExtPrepro;
                   firstchar=0;
                   break;

              case '#':
                   if (firstchar)
                     {
                      firstchar=0;
                      in_prepro=1;
                      attr|=Prepro;
                     }
                   break;

              default:
                   if (firstchar && !ucisspace(*s))
                      firstchar=0;
             }
          }
    ++l;
    ++s;
   }
 Attr=attr;
 if (extra)
   {
    *extra=0;
    if (in_string) *extra|=InString;
    if (in_char)   *extra|=InString2;
    if (in_com)    *extra|=InComment;
    if (in_prepro) *extra|=InPrepro;
   }
#if 0
 if (l && *s=='\n') l++;
#else
 if (*s=='\n') l++;
#endif
 return l;
}

/*****************************************************************************

 That's the generic version, is a little complex but seems to work, at least
for C/C++.
 Some parts are like that: isComment() && CheckForSequence..., that means a
very important improvement in speed, without that the speed of the routine
is a dissaster in comparisson with the specialized C version.

*****************************************************************************/

static
unsigned LineMeassureGeneric(char *s, char *end, uint32 &Attr, uint32 *extra)
{
 uint32 l=0;
 char *end2=end-1;
 int in_string=0,in_string2=0,in_string3=0;
 int type_com=0;
 int in_prepro=0;
 int firstchar=1;
 // firstCol is realted to the FG1_EOLCInFirstCol, trick to avoid anding
 int firstCol1_1=1,firstCol2_1=1,firstCol1_2,firstCol2_2;
 int firstUse1_1=1,firstUse2_1=1,firstUse1_2,firstUse2_2;
 int escapeAnywhere;
 uint32 attr=Attr;
 char *start=s;

 // Set the Case Sensitive comparation status for check sequence
 CheckSeqCase=(TCEditor::strC.Flags1 & FG1_CaseSensitive)!=0;
 firstCol1_2 =(TCEditor::strC.Flags1 & FG1_EOLCInFirstCol1)==0;
 firstCol2_2 =(TCEditor::strC.Flags1 & FG1_EOLCInFirstCol2)==0;
 firstUse1_2 =(TCEditor::strC.Flags1 & FG1_EOLCInFirstUse1)==0;
 firstUse2_2 =(TCEditor::strC.Flags2 & FG2_EOLCInFirstUse2)==0;
 escapeAnywhere=(TCEditor::strC.Flags2 & FG2_EscapeAnywhere)!=0;
 // Is the continuation of a comment?
 if (attr & ExtCom) // Type 1
   {
    attr|=InsideCom | StartInCom; // So is inside and starts commented
    attr&=Not_ExtOneLineCom;      // Not extend single line
    type_com=1;
   }
 else
   if (attr & ExtCom2) // Type 2 idem
     {
      attr|=InsideCom2 | StartInCom2;
      attr&=Not_ExtOneLineCom;
      type_com=2;
     }
   else
     if (attr & ExtOneLineCom) // One line
       {
        attr|=InsideCom | StartInCom;
        attr&=Not_ExtOneLineCom;
        type_com=3;
       }
     else
        attr&=Not_InsideCom & Not_StartInCom; // If not clear that

 // Is the continuation of a preprocessor line?
 if (!(attr & ExtPrepro))
    attr&=Not_Prepro;  // No clean it
 else
    in_prepro=1;       // Yes

 // Is the continuation of an string?
 if (attr & ExtString)
    in_string=1;
 if (attr & ExtString2)
    in_string2=1;
 if (attr & ExtString3)
    in_string3=1;

 attr&=FilterHere;

 if (in_string)
   {
    attr|=StartString;
    if (s==end || *s=='\n') // That's a particular case where lines are empty
      {
       attr|=ExtString;
       if (in_prepro)
          attr|=ExtPrepro;
      }
   }
 if (in_string2)
   {
    attr|=StartString2;
    if (s==end || *s=='\n')
      {
       attr|=ExtString2;
       if (in_prepro)
          attr|=ExtPrepro;
      }
   }
 if (in_string3)
   {
    attr|=StartString3;
    if (s==end || *s=='\n')
      {
       attr|=ExtString3;
       if (in_prepro)
          attr|=ExtPrepro;
      }
   }


 while (s<end && *s!='\n')
   {
    if (/*!in_prepro && */!type_com) // Yes, strings exist inside preprocessor.
      {
       if (escapeAnywhere && *s==TCEditor::strC.Escape && CLY_IsntEOL(*(s+1)))
         {
          s+=2;
          continue;
         }
       if (isString(*s) || in_string)
         {
          if (!in_string)
            {
             s++;
             in_string=1;
            }
          firstchar=0;
          while (s<end && *s!='\n')
            {
             if (*s==TCEditor::strC.Escape)
               {
                if (!(s==end2 || CLY_IsEOL(*(s+1))))
                   s++;
               }
             else
               if (isString(*s))
                 {
                  in_string=0;
                  s++;
                  break;
                 }
             s++;
            }
          if (in_string)
            {
             attr|=ExtString;
             if (in_prepro)
                attr|=ExtPrepro;
            }
          continue;
         }
       // Repeated for string2
       if (isString2(*s) || in_string2)
         {
          if (!in_string2)
            {
             s++;
             in_string2=1;
            }
          firstchar=0;
          while (s<end && *s!='\n')
            {
             if (*s==TCEditor::strC.Escape)
               {
                if (!(s==end2 || CLY_IsEOL(*(s+1))))
                   s++;
               }
             else
               if (isString2(*s))
                 {
                  in_string2=0;
                  s++;
                  break;
                 }
             s++;
            }
          if (in_string2)
            {
             attr|=ExtString2;
             if (in_prepro)
                attr|=ExtPrepro;
            }
          continue;
         }
       // Repeated for string3
       if (isString3(*s) || in_string3)
         {
          if (!in_string3)
            {
             s++;
             in_string3=1;
            }
          firstchar=0;
          while (s<end && *s!='\n')
            {
             if (*s==TCEditor::strC.Escape)
               {
                if (!(s==end2 || CLY_IsEOL(*(s+1))))
                   s++;
               }
             else
               if (isString3(*s))
                 {
                  in_string3=0;
                  s++;
                  break;
                 }
             s++;
            }
          if (in_string3)
            {
             attr|=ExtString3;
             if (in_prepro)
                attr|=ExtPrepro;
            }
          continue;
         }
   
       if (isCharacter(*s))
         {
          s++;
          firstchar=0;
          while (s<end && *s!='\n')
            {
             if (*s==TCEditor::strC.Escape)
                s++;
             else
               if (isCharacter(*s))
                 {
                  s++;
                  break;
                 }
             s++;
            }
          continue;
         }
       if ((*s==TCEditor::strC.Preprocessor || *s==TCEditor::strC.Preprocessor2) &&
           firstchar)
         {
          in_prepro=1;
          attr|=Prepro;
          s++;
          continue;
         }
      }
    if (in_prepro && *s==TCEditor::strC.Escape && (s==end2 || CLY_IsEOL(*(s+1))))
      {
       attr|=ExtPrepro;
       s++;
       continue;
      }
    int available=end-s;
    if ((isComment1(*s) && CheckForSequenceNotFirst(TCEditor::strC.OpenCom1,TCEditor::strC.lOpenCom1,available,s))
        || type_com==1)
      {
       if (type_com!=1)
         {
          s+=TCEditor::strC.lOpenCom1;
          available-=TCEditor::strC.lOpenCom1;
          attr|=StartCom | ExtCom;
          if (firstchar)
             attr|=InsideCom;
         }
       while (s<end && *s!='\n')
         {
          if (isCloseComm1(*s) && CheckForSequenceNotFirst(TCEditor::strC.CloseCom1,TCEditor::strC.lCloseCom1,available,s))
            {
             s+=TCEditor::strC.lCloseCom1;
             attr|=EndCom;
             attr&=Not_ExtCom & Not_InsideCom;
             type_com=0;
             break;
            }
          available--;
          s++;
         }
       firstchar=0;
      }
    else
    if (isComment2(*s) && CheckForSequenceNotFirst(TCEditor::strC.OpenCom2,TCEditor::strC.lOpenCom2,available,s))
      {
       if (type_com!=2)
         {
          s+=TCEditor::strC.lOpenCom2;
          available-=TCEditor::strC.lOpenCom2;
          attr|=StartCom2 | ExtCom2;
          if (firstchar)
             attr|=InsideCom2;
         }
       while (s<end && *s!='\n')
         {
          if (isCloseComm2(*s) && CheckForSequenceNotFirst(TCEditor::strC.CloseCom2,TCEditor::strC.lCloseCom2,available,s))
            {
             s+=TCEditor::strC.lCloseCom2;
             attr|=EndCom2;
             attr&=Not_ExtCom2 & Not_InsideCom2;
             type_com=0;
             break;
            }
          available--;
          s++;
         }
       firstchar=0;
      }
    else
    if (type_com==3 || // Commented from the previous line
        // EOL Comment 1
        (firstCol1_1 && firstUse1_1 && isEOLComment(*s) &&
         CheckForSequence(TCEditor::strC.EOLCom1,TCEditor::strC.lEOLCom1,available,s))
        ||
        // EOL Comment 2
        (firstCol2_1 && firstUse2_1 && isEOLComment(*s) &&
         CheckForSequence(TCEditor::strC.EOLCom2,TCEditor::strC.lEOLCom2,available,s))
       )
      {
       if (type_com!=3)
         {
          attr|=ComInside;
          if (firstchar)
             attr|=InsideCom;
         }
       while (s<end && *s!='\n')
         {
          if (*s==TCEditor::strC.Escape && (s==end2 || *(s+1)=='\r'))
             attr|=ExtOneLineCom;
          s++;
         }
       firstchar=0;
      }
    else
    if (!type_com && isSpecialSymb(*s) && s+1<end && isSpecialSCon(s[1]))
      {// Special symbols acts like one atom
       s+=2;
       firstchar=0;
       continue;
      }
    else
    if (firstchar && !ucisspace(*s))
      {
       s++;
       firstchar=0;
      }
    else
      s++;
    firstCol1_1=firstCol1_2;
    firstCol2_1=firstCol2_2;
    if (!firstchar)
      {
       firstUse1_1=firstUse1_2;
       firstUse2_1=firstUse2_2;
      }
   }
 // Comments of type /* */ with more than 1 line in a preprocessor line
 if (in_prepro && (attr & (ExtCom | ExtCom2)))
    attr|=ExtPrepro;
 Attr=attr;
 if (extra)
   {
    *extra=0;
    if (in_string)  *extra|=InString;
    if (in_string2) *extra|=InString2;
    if (in_string3) *extra|=InString3;
    if (type_com)   *extra|=InComment;
    if (in_prepro)  *extra|=InPrepro;
   }
 l=s-start;
#if 0
 if (l && *s=='\n') l++;
#else
 if (*s=='\n') l++;
#endif
 return l;
}


/****************************************************************************

   Function: void setBufLen( uint32 length )

   Type: TCEditor member.

   Objetive: Set the length of the buffer, that's very different to the
   size, the length is the *used* part, the size is the amount of memory
   allocated (>= length).
     That forces an initialization of all the variables.

   Parameters:
   length: length of the buffer.

   by SET.

****************************************************************************/

void TCEditor::setBufLen( uint32 length )
{
 bufLen   = length;
 gapLen   = 0;
 selStart = 0;
 selEnd   = 0;
 curPtr   = 0;
 drawPtr  = 0;
 selStartOffSet = 0;
 modified=False;
 modifiedCounter=0; // It could be a random value
 IsStatusLineOn=False;
 IsFoundOn=False;
 IsHLCOn=False;

 uint32 lines   = 0,
        maxLen  = 0;  // To know how large must be the edition buffer
 //buffer[length] = 0;

 unsigned conta;
 int haveASCII0=0;
 for (conta=0; conta<length; conta++)
    {
     if (buffer[conta]=='\r')
       {
        conta++;
        if (conta==length)
          {
           setBufSize(length+1);
           bufLen++; length++;
           buffer[conta]='\n';
          }
        else
          if (buffer[conta]!='\n')
            {
             setBufSize(length+1);
             bufLen++; length++;
             memmove(buffer+conta+1,buffer+conta,length-conta);
             buffer[conta]='\n';
            }
       }
     else
       if (buffer[conta]==0)
          haveASCII0=1;
       #ifdef CLY_UseCrLf
       else
         if (buffer[conta]=='\n')
           {
            conta++;
            if (conta==length)
              {
               setBufSize(length+1);
               bufLen++; length++;
               buffer[conta-1]='\r';
               buffer[conta]='\n';
              }
            else
              {
               setBufSize(length+1);
               bufLen++; length++;
               memmove(buffer+conta,buffer+conta-1,length-conta+1);
               buffer[conta-1]='\r';
              }
           }
     #endif
    }

 if (haveASCII0)
    messageBox(__("This file contains ASCII 0 values, all the characters in this line after it become invisible. Be careful."),mfWarning | mfOKButton);

 if (bufLen)
   {
    char *s=buffer,*end=s+bufLen;
    uint32 ThisLine;
    uint32 Attr=0;

#ifndef PROFILE_LINE_MEASSURE
    CacheSyntaxHLData(GenericSHL);
    int useProgressBar=0,accumulated=0,chunkSize=0;
    if (bufLen>(1<<20))
      {
       useProgressBar=1;
       chunkSize=bufLen/110;
       ProgBar_Init(__("Parsing file"),bufLen);
      }
    while (s<end)
      {
       ThisLine=LineMeassure(s,end,Attr,NULL);
       if (ThisLine)
         {
          if (ThisLine>65535)
            {
             editorDialog(edLineTooLong);
             isValid=False;
             return;
            }
          lenLines.setAll(lines++,ThisLine,Attr);
          if (ThisLine>maxLen)
             maxLen=ThisLine;
         }
       s+=ThisLine;
       if (useProgressBar)
         {
          accumulated+=ThisLine;
          if (accumulated>chunkSize)
            {
             accumulated=0;
             ProgBar_UpDate(s-buffer);
            }
         }
      }
    if (useProgressBar)
       ProgBar_DeInit();
#else
    clock_t t0,t1,t2,t3;
    char buf[100];

    LineMeassure=LineMeassureC;
    t0=clock();
    while (s<end)
      {
       ThisLine=LineMeassure(s,end,Attr);
       if (ThisLine)
         {
          lenLines.setAll(lines++,ThisLine,Attr);
          if (ThisLine>maxLen)
             maxLen=ThisLine;
         }
       s+=ThisLine;
      }
    t1=clock();

    LineMeassure=LineMeassureGeneric;
    CacheSyntaxHLData(GenericSHL);
    s=buffer;
    end=s+bufLen;
    Attr=0;
    t2=clock();
    while (s<end)
      {
       ThisLine=LineMeassure(s,end,Attr);
       if (ThisLine)
         {
          lenLines.setAll(lines++,ThisLine,Attr);
          if (ThisLine>maxLen)
             maxLen=ThisLine;
         }
       s+=ThisLine;
      }
    t3=clock();

    messageBox(mfError | mfOKButton,"Tiempo para la de C: %u Tiempo para la generica: %u",t1-t0,t3-t2);
#endif
   }
 else
    lenLines.setAll(0,0,0);
 drawLine=0;            // First displayed line
 totalLines=lines ? lines-1 : 0;    // Total number of lines
 limit.y=lines;
 lineInEdition=0;       // Line number in edition process
 IslineInEdition=False; // There is a line under edition?
 curLinePtr=buffer;     // Pointer to the start of the line under the cursor
 delta.x=0;
 delta.y=0;               // Origin in window
 lastCurPos=curPos=delta; // Origin in file

 // Allocate the Edition buffer
 if (bufEdit && (maxLen>(uint32)bufEditLen))
   {
    free(bufEdit);
    bufEdit=0;
   }
 limit.x=max(maxLen,(uint32)MinLineLen);
 if (!bufEdit)
   {
    bufEditLen=limit.x;
    bufEdit=(char *)malloc(bufEditLen+4);
   }

 selLineStart=selLineEnd=selNewStart=selNewEnd=0;
 selHided=False;         // Hide the selection
 NotExpandingMacro=True;
 memset((void *)Markers,0,10*sizeof(uint32)); // Clean the markers

 // Initialize the undo thing.
 UndoArray[0].Type=UndoSt=undoNoUndo;
 UndoBase=UndoActual=UndoTop=0;
 undoLockCount=undoGroupCount=0;
 UndoArray[0].X=UndoArray[0].Y=0;
 UndoArray[0].Count=0;

 // Initialize the rect. sel
 Xr1=Yr1=Xr2=Yr2=0;
 selRectHided=True;
 selRectClip=NULL;

 update(ufView);
 return;
}

/****************************************************************************

   Function: Boolean setBufSize( uint32 newSize )

   Type: TCEditor member.

   Objetive: Set the size of the buffer, and allocate the memory for it.
             The function doesn't use realloc because is dangerous, if we
             use realloc and it fails we loose all the data.

   Parameters:
   newSize: obvious.

   Return:
   True if there is enough space.

   by SET.

****************************************************************************/

// Not 4096 because we will waste a lot of memory bacause libc allocates in
// powers of 2 and if we need 63000 then we will ask for 65536 + overhead
// and libc will allocate 131072 bytes, but if we use 4064 we will ask for
// 64800 + overhead and libc will allocate 65536.
#define Granularity 4064

static uint32 MakeItGranular( uint32 value )
{
 value+=Granularity;
 value-=value % Granularity;

 return value;
}

#if 0
extern "C" int _mstats(char *s);

Boolean TCEditor::setBufSize( uint32 newSize )
{
 newSize = MakeItGranular(newSize);

 _mstats("Antes");
 if (newSize!=bufSize)
   {
    // Make all in a way that allow a fail without loosing the data
    unsigned DeltaCl=(unsigned)(curLinePtr-buffer);
    char *temp;
    if ((temp=(char *)malloc(newSize))==0)
      {
       //delete temp; Why I put that?
       editorDialog( edOutOfMemory );
       return False;
      }
    memcpy( temp, buffer, min( newSize, bufSize ) );
    free(buffer);
    buffer=temp;
    bufSize=newSize;
    curLinePtr=buffer+DeltaCl;
   }
 _mstats("Despues");
 return True;
}
#endif

Boolean TCEditor::setBufSize( uint32 newSize )
{
 newSize = MakeItGranular(newSize);

 //_mstats("Antes");
 if (newSize!=bufSize)
   {
    // Make all in a way that allow a fail without loosing the data
    unsigned DeltaCl=(unsigned)(curLinePtr-buffer);
    char *temp;
    if ((temp=(char *)realloc(buffer,newSize))==0)
      {
       // Here I'm taking a risk, the pointer was passed to free but still
       // having the text so the user can save but the behavior of the
       // rest is unpredictable.
       editorDialog( edOutOfMemory );
       return False;
      }
    buffer=temp;
    bufSize=newSize;
    curLinePtr=buffer+DeltaCl;
   }
 //_mstats("Despues");
 return True;
}

/****************************************************************************

   Function: void setCmdState( uint16 command, Boolean enable )

   Type: TCEditor member.

   Objetive:
     Enables or disables the given command depending on whether enable is True
   or False and whether the editor is sfActive.
     The command is always disabled if the editor is not the selected view.
     Offers a convenient alternative to enableCommands and disableCommands.

   Parameters:
   command: The selected command (Copy, Paste, etc).
   enable: The new state.

   by Robert.

****************************************************************************/

void TCEditor::setCmdState( uint16 command, Boolean enable )
{
 if (enable && (state & sfActive))
    enableCommand(command);
 else
    disableCommand(command);
}


/****************************************************************************

   Function: void setSelect( uint32 newStart, uint32 newEnd,
                             Boolean curStart )

   Type: TCEditor member.

   Objetive:
     Sets the selection to the given offsets into the file, and redraws the
   view as needed.
     This member function will either place the cursor in front of or behind
   the selection, depending on the value (True or False, respectively) of
   curStart.

   by SET.

****************************************************************************/

void TCEditor::setSelect( uint32 newStart, uint32 newEnd, Boolean curStart )
{
 uint32 p;
 if ( curStart != 0 )
    p = newStart;
 else
    p = newEnd;

 uchar flags = ufUpdate;

 if ( newStart != selStart || newEnd != selEnd )
    if ( newStart != newEnd || selStart != selEnd )
       flags = ufView;

 if (p!=(uint32)(ColToPointer()-buffer))
    GotoOffSet(p);

 selStart = newStart;
 selEnd   = newEnd;
 selHided = False;
 update(flags);
}


/****************************************************************************

   Function: void setState( uint16 aState, Boolean enable )

   Type: TCEditor member.

   Objetive:
     Overrides TView::setState to hide and show the indicator and scroll bars.
     It first calls TView::setState to enable and disable commands. If you
   wish to enable and disable additional commands, override updateCommands
   instead.
     This is called whenever the command states should be updated.

****************************************************************************/

void TCEditor::setState(uint16 aState, Boolean enable)
{
 TView::setState(aState, enable);
 switch (aState)
   {
    case sfActive:
         if (hScrollBar)
            hScrollBar->setState(sfVisible,enable);
         if (vScrollBar)
            vScrollBar->setState(sfVisible,enable);
         if (indicator)
            indicator->setState(sfVisible,enable);
         // Note: Commands could be updated when sfFocused is indicated.
         // But it doesn't help at all. One could think it can help when a modal dialog
         // is used and it changes the stuff that affects commands. But modal dialogs are
         // executed through TView::execView and it first saves the current command set
         // and then restores it. If we process the commands during sfFocused the changes
         // are lost when execView restores the values.
         updateCommands(1);
         message(TProgram::application,evBroadcast,cmcEditorGotFocus,owner);
         forceNextTimeCheck=True;
         break;

    case sfExposed:
         if (enable)
            unlock();
   }
}

/****************************************************************************

   Function: void startSelect()

   Type: TCEditor member.

   Objetive: Set the start of the selection, but doesn't do more. See
   SetStartOfSelecting.

   by SET.

****************************************************************************/

void TCEditor::startSelect()
{
 selStart=(uint32)(ColToPointer()-buffer);
}

/****************************************************************************

   Function: void toggleInsMode(Boolean allowUndo)

   Type: TCEditor member.

   Objetive: Toggle the insertion/overwrite mode.

****************************************************************************/

void TCEditor::toggleInsMode(Boolean allowUndo)
{
 if (allowUndo)
    addToUndo(undoCutInMov);
 overwrite = (!overwrite) ? True : False;
 setState(sfCursorIns,overwrite);
 if (!TScreen::cursorShapes())
    update(ufUpdate);
}


/****************************************************************************

   Function: void trackCursor( Boolean center )

   Type: TCEditor member.

   Objetive: Adjust the delta to make the cursor position visible.

   Parameter:
   center: True => keep the cursor in the middle of the screen in the y axe.

   Notes:
     Adapted to limit the cursor movement

****************************************************************************/

void TCEditor::trackCursor( Boolean center )
{
 int x=curPos.x;

 if (x>=MaxLineLen)
   {
    Beep();
    curPos.x=x=MaxLineLen-1;
   }
 if (x>=limit.x)
    limit.x=x+1;

 if ( center )
    scrollTo( x - size.x + 1, curPos.y - size.y / 2);
 else
    scrollTo( max(x - size.x + 1, min(delta.x, x)),
              max(curPos.y - size.y + 1, min(delta.y, curPos.y)));
}

/****************************************************************************

   Function: void unlock()

   Type: TCEditor member.

   Objetive: Unlock the draw system and force an adecuated redraw.

   From Borland's TV 1.03.

****************************************************************************/

void TCEditor::unlock()
{
 if ( lockCount > 0 )
   {
    lockCount--;
    if ( lockCount == 0 )
        doUpdate();
   }
}

/****************************************************************************

   Function: void update( uchar aFlags )

   Type: TCEditor member.

   Objetive: Set the draw flags to the desired value, and make a redraw if
   the editor is unlocked.

   Parameter:
   aFlags: flags to activate.

   From Borland's TV 1.03.

****************************************************************************/

void TCEditor::update( uchar aFlags )
{
 updateFlags|=aFlags;
 if (!lockCount)
    doUpdate();
}


/**[txh]********************************************************************

  Description:
  Called whenever the commands should be updated. This is used to
enable and disable commands such as cmcUndo, cmcClip, and cmcCopy. The
function is basically called when the editor get/lose the focus and when
something is redrawed.@p
  When the editor get/lose the focus a full update is done, in includes all
the commands used by the editor. Normally only few commands are updated.
To reduce the overhead when a full update is done an auxiliar set of
commands is used and a special feature (added by me) is used to set/clear
all the commands in one call.

***************************************************************************/

void TCEditor::updateCommands(int full)
{
 if (full)
   { // We are getting or losing the focus so we must do a big work
    getCommands(cmdsAux);
   
    if (!(state & sfActive))
      { // We lost the focus, disable all
       DisableCommands(cmdsAux);
       return;
      }
    // We got the focus, enable all
    cmdsAux.enableCmd(cmbBaseNumber,cmbBaseNumber+cmbLastCommand);
    if (isClipboard())
      { // Restrict some stuff for the clipboard
       cmdsAux.disableCmd(cmcCut);
       cmdsAux.disableCmd(cmcCutClipWin);
       cmdsAux.disableCmd(cmcCopy);
       cmdsAux.disableCmd(cmcCopyClipFile);
       cmdsAux.disableCmd(cmcCopyClipWin);
       cmdsAux.disableCmd(cmcPaste);
       cmdsAux.disableCmd(cmcPasteClipFile);
       cmdsAux.disableCmd(cmcPasteClipWin);
       cmdsAux.disableCmd(cmcClear);
       cmdsAux.disableCmd(cmcUndo);
       cmdsAux.disableCmd(cmcRedo);
       cmdsAux.disableCmd(cmcSelRectPaste);
       cmdsAux.disableCmd(cmcSelRectDel);
       cmdsAux.disableCmd(cmcSelRectMove);
       cmdsAux.disableCmd(cmcSelRectCut);
       cmdsAux.disableCmd(cmcSelRectToUpper);
       cmdsAux.disableCmd(cmcSelRectToLower);
      }
    if (!hasRectSel())
      {
       cmdsAux.disableCmd(cmcSelRectCopy);
       cmdsAux.disableCmd(cmcSelRectDel);
       cmdsAux.disableCmd(cmcSelRectMove);
       cmdsAux.disableCmd(cmcSelRectCut);
       cmdsAux.disableCmd(cmcSelRectHide);
       cmdsAux.disableCmd(cmcSelRectToUpper);
       cmdsAux.disableCmd(cmcSelRectToLower);
      }
    if (!selRectClip)
       cmdsAux.disableCmd(cmcSelRectPaste);
    if (NoNativeEOL && (editorFlags & efSaveEOLasis))
       cmdsAux.disableCmd(cmcSaveAsConvertEOL);
    else
       cmdsAux.disableCmd(cmcSaveAsNoConvertEOL);
    if (SHLValueSelected<0 || SHLValueSelected>=SHLCant || !SHLArray ||
        !SHLArray[SHLValueSelected].EmacsModes)
       cmdsAux.disableCmd(cmcPasteEmacsMode);
    setCommands(cmdsAux);
   }

 if (!isClipboard())
   {
    setCmdState(cmcUndo,Boolean(UndoActual!=UndoBase));
    setCmdState(cmcRedo,Boolean(UndoActual<UndoTop));
   
    Boolean hs=hasVisibleSelection();
    Boolean oscli=TVOSClipboard::isAvailable() ? True : False;
    setCmdState(cmcCut,hs);
    setCmdState(cmcCutClipWin,(hs && oscli) ? True : False);
    setCmdState(cmcCopy,hs);
    setCmdState(cmcCopyClipFile,hs);
    setCmdState(cmcCopyClipWin,(hs && oscli) ? True : False);
    setCmdState(cmcClear,hs);
    setCmdState(cmcPaste,Boolean(clipboard && clipboard->hasSelection()));
    setCmdState(cmcPasteClipWin,oscli);
   }
}

/**[txh]********************************************************************

  Description:
  Updates the state of the rectangular selection commands. Is called by the
operations that alters the rectangular selection.

***************************************************************************/

void TCEditor::updateRectCommands()
{
 Boolean rs=hasRectSel();
 setCmdState(cmcSelRectCopy,rs);
 setCmdState(cmcSelRectHide,rs);
 rs=(rs==True && isClipboard()==False) ? True : False;
 setCmdState(cmcSelRectDel,rs);
 setCmdState(cmcSelRectMove,rs);
 setCmdState(cmcSelRectCut,rs);
 setCmdState(cmcSelRectToUpper,rs);
 setCmdState(cmcSelRectToLower,rs);
 // cmcSelRectPaste is an exception and is updated by selRectCopy()
}

/**[txh]********************************************************************

  Description:
  Moves the cursor to the x,y coordinate. If the @var{undo} argument is true
then the line in edition is flushed and the action is recorded in the undo.

***************************************************************************/

void TCEditor::MoveCursorTo(uint32 x, uint32 y, Boolean undo)
{
 if (undo)
   {
    flushLine();
    addToUndo(undoInMov);
   }
 if (y<(uint32)curPos.y)
    MoveLinesUp(curPos.y-y);
 else
    if (y>(uint32)curPos.y)
       MoveLinesDown(y-curPos.y);
 curPos.x=x;
}


// Circular stack routines

XYRingStack::XYRingStack()
{
 pushPos=basePos=0;
}

inline
void incWithWrap(int &value, int max)
{
 value++;
 if (value==max)
    value=0;
}

inline
void decWithWrap(int &value, int max)
{
 value--;
 if (value<0)
    value=max-1;
}

void XYRingStack::push(uint32 x, uint32 y)
{
 Xarray[pushPos]=x;
 Yarray[pushPos]=y;
 incWithWrap(pushPos,MaxXYRingStack);
 if (pushPos==basePos)
    incWithWrap(basePos,MaxXYRingStack);
}

int XYRingStack::pop(uint32 &x, uint32 &y)
{
 if (pushPos==basePos)
    return 0;
 decWithWrap(pushPos,MaxXYRingStack);
 x=Xarray[pushPos];
 y=Yarray[pushPos];
 return 1;
}

/**[txh]********************************************************************

  Description:
  Stores the current cursor position in a circular stack.

***************************************************************************/

void TCEditor::PushCursorPosition()
{
 CurPosStack.push(curPos.x,curPos.y);
}

/**[txh]********************************************************************

  Description:
  Restores the current cursor position from a circular stack.

***************************************************************************/

int TCEditor::PopCursorPosition()
{
 uint32 x,y;

 if (CurPosStack.pop(x,y))
   {
    ClearSelIfNonPers();
    MoveCursorTo(x,y,True);
    return 1;
   }
 return 0;
}


/****************************************************************************

   Function: void RecalculateLineAttributes(void)

   Type: TCEditor member.

   Objetive: Recalculates the syntax highlight attributes of each line in
   the editor. Called by SetHighlightTo when there is a change in the
   syntax highlight system.

   by SET.

****************************************************************************/

void TCEditor::RecalculateLineAttributes(void)
{
 uint32 lines=0;

 if (bufLen)
   {
    char *s=buffer,*end=s+bufLen;
    uint32 ThisLine,Attr=0;

    CacheSyntaxHLData(GenericSHL);
    while (s<end)
      {
       ThisLine=LineMeassure(s,end,Attr,NULL);
       if (ThisLine)
          lenLines.setAll(lines++,ThisLine,Attr);
       s+=ThisLine;
      }
   }
}

/****************************************************************************

   Function: void SetHighlightTo(shlState sHL)

   Type: TCEditor member.

   Objetive: Set the selected syntax highligth mode

   by SET.

****************************************************************************/

void TCEditor::SetHighlightTo(shlState sHL, int subSHL)
{
 switch (sHL)
   {
    case shlNoSyntax:
         TurnOffHighLight();
         SHLValueSelected=-1;
         break;
    case shlCSyntax:
         TurnOnCHighLight();
         SHLValueSelected=SHLTableUse[sHL];
         if (LineMeassure!=LineMeassureC)
           {
            LineMeassure=LineMeassureC;
            RecalculateLineAttributes();
           }
         break;
    case shlPascalSyntax:
         TurnOnPascalHighLight();
         SHLValueSelected=SHLTableUse[sHL];
         if (LineMeassure!=LineMeassurePascal)
           {
            LineMeassure=LineMeassurePascal;
            RecalculateLineAttributes();
           }
         break;
    case shlClipperSyntax:
         TurnOnClipperHighLight();
         SHLValueSelected=SHLTableUse[sHL];
         if (LineMeassure!=LineMeassureClipper)
           {
            LineMeassure=LineMeassureClipper;
            RecalculateLineAttributes();
           }
         break;
    case shlGenericSyntax:
         TurnOnGenericHighLight();
         SHLValueSelected=subSHL;
         if (LineMeassure!=LineMeassureGeneric || subSHL!=GenericSHL)
           {
            GenericSHL=subSHL;
            LineMeassure=LineMeassureGeneric;
            RecalculateLineAttributes();
           }
         break;
    default: messageBox(__("Unhandled syntax highlighting"),mfError | mfOKButton);
   }
}

/****************************************************************************

   Function: void write( opstream& os )

   Type: TCEditor member.

   Objetive: Write the asociated data to the class.

****************************************************************************/

void TCEditor::write( opstream& os )
{
 TView::write( os );
 os << (unsigned)TCEDITOR_VERSION << hScrollBar << vScrollBar << indicator
    << bufSize << (int)canUndo;
 os.writeString( fileName );
 os << selStart << selEnd   // The selected area
    << curPos.x << curPos.y // The real position, a pointer can point to
                            // a non existing position but the cursor can
                            // be in a X position where there is no text.
    << (uchar)selHided      // Needed to know if the selection is hided or not.
                            // The mode of edition
    << (uchar)SyntaxHL
 ;
 os << CompactFlags() << tabSize << indentSize;

 SaveColMarkers(os,colMarkers);  // 0.4.50
 os << WrapCol; // From v0.4.1 the wrap thing

 // From v0.4.27 the generic shl. name (was type from 0.3.1 to 0.4.26)
 os.writeString(SHLNameOf(GenericSHL));

 // Markers
 os.writeBytes(Markers,sizeof(Markers));

 // From v0.2.11
 os << Xr1 << Yr1 << Xr2 << Yr2 << (uchar)selRectHided;

 // From v0.2.13
 os << (uchar)staticNoMoveToEndPaste;

 // From v0.4.19
 os << (uchar)isReadOnly;
}

static inline
unsigned MoveFlags(unsigned flags, unsigned mask, unsigned pos)
{
 return (flags & mask) | ((flags & (~mask))<<pos);
}

/**[txh]********************************************************************

  Description:
  Reads the asociated data to the class.

***************************************************************************/

void *TCEditor::read( ipstream& is )
{
 TView::read(is);
 int temp;

 is >> LoadingVersion >> hScrollBar >> vScrollBar;

 if (LoadingVersion<=0x420)
   { /* Old versions used a diferent indicator */
    is >> OldIndicator;
    indicator=new TSIndicator(OldIndicator->getBounds());
   }
 else
   {
    OldIndicator=0;
    is >> indicator;
   }
 indicator->editor=this;

 is >> bufSize >> temp;
 canUndo=(temp) ? True : False;
 selecting =False;
 overwrite =False;
 autoIndent=False;
 keyState=0;
 if (DontLoadFile) bufSize=0;
 initBuffer();
 if (buffer)
    isValid=True;
 else
    {
     TCEditor::editorDialog(edOutOfMemory,0);
     bufSize=0;
    }
 lockCount=0;
 //lock();
 bufEdit=NULL; // must be initialized before setBufLen()
 bufEditLen=0;
 // Normally the files aren't compressed, if the file is really compressed we will
 // know it in loadFile
 IsaCompressedFile=gzNoCompressed;
 TurnOffHighLight(); // Here only to ensure that the functions pointers are
                     // pointing to some valid place.
 LineMeassure=LineMeassureC;
 GenericSHL=0;
 // The setBufLen member does a call to update. So we must initialize some
 // stuff related to the drawing machinery.
 updateFlags=0;
 CrossCursorInCol=CrossCursorInRow=False;
 // We aren't a disassembler window
 isDisassemblerEditor=0;
 setBufLen(0);

 SpecialLines=NULL;

 is.readString(fileName,sizeof(fileName));
 if (isValid)
   {
    uint32 sStart, sEnd;
    int X,Y;
    uchar sh,shl,subshl;
    uint32 flags;
    is >> sStart >> sEnd >> X >> Y >> sh >> shl;
    if (LoadingVersion>=0x443)
       is >> flags >> tabSize >> indentSize;
    else
      {
       uint16 oldFlags;
       is >> oldFlags >> tabSize;
       indentSize=4;
       flags=oldFlags;
      }

    if (LoadingVersion<0x450)
       colMarkers=CopyColMarkers(staticColMarkers);
    else
       colMarkers=LoadColMarkers(is);

    if (LoadingVersion>=0x401)
       is >> WrapCol;
    else
       WrapCol=60;

    if (LoadingVersion<0x427)
       is >> subshl;
    else
      {
       char *name=is.readString();
       subshl=SHLNumberOf(name); // Must be unsigned! (unsigned)-1
       delete[] name;
      }
    // Avoid problems if the value isn't valid
    if (subshl>=SHLCant)
       shl=shlNoSyntax;

    if (LoadingVersion<0x432)
       flags=MoveFlags(flags,0xFF,1); // Inserted: ShowMatchPairFly
    if (LoadingVersion<0x445)
       flags=MoveFlags(flags,0x1FF,1); // Inserted: ShowMatchPairNow
    // Now we have all like now restore some stuff that was implicit
    if (LoadingVersion<0x440)
       flags|=loTabIndents;
    if (LoadingVersion<0x445 && !(flags & loUseTabs))
       flags|=loBackSpUnindents;
    if (LoadingVersion<0x450 && staticColumnMarkers)
       flags|=loColumnMarkers;
    ExpandFlags(flags,False);
    SetHighlightTo((shlState)shl,subshl);
    isReadOnly=False; // Will be read later
    if (!DontLoadFile)
       isValid = loadFile();

    // Markers after the loadFile because loadFile cleans the markers
    is.readBytes(Markers,sizeof(Markers));

    char aux;
    is >> Xr1 >> Yr1 >> Xr2 >> Yr2 >> aux;
    selRectHided=Boolean(aux);

    is >> aux;
    staticNoMoveToEndPaste=Boolean(aux);
    if (LoadingVersion>=0x419)
      {
       is >> aux;
       isReadOnly=Boolean(aux);
      }
    else
      isReadOnly=False;

    CrossCursorY2=size.y;
    CrossCursorX2=size.x;
    CrossCursorCol=X;
    CrossCursorRow=Y;
   
    if (X>limit.x)
       limit.x=X+1;
    selHided=(sh) ? True : False;
    if (sEnd<=bufLen)
      {
       selStart=sStart;
       selEnd=sEnd;
       selStartOffSet=sStart;
      }

    if (isValid && !DontLoadFile)
      {
       MoveCursorTo(X,Y);
       trackCursor(True);
       update(ufView);
      }
    else
      { // Setup this so we can construct a good resume
       curPos.x=X;
       curPos.y=Y;
      }
   }
 return this;
}


/****************************************************************************

   Function: void GotoOffSet(uint32 o)

   Type: TCEditor member.

   Objetive: Move the cursor position to an offset in the buffer.

   Parameter:
   o: destination offset.

   by SET.

****************************************************************************/

void TCEditor::GotoOffSet(uint32 o)
{
 uint32 Acu=0;
 uint32 y=0;
 char *s;

 if (o>bufLen) return;

 // Could use FindLineForOffSet, but isn't enough tested (july 1999)
 for (;y<totalLines && Acu<o; y++)
     Acu+=lenLines[y];

 if (Acu>o)
    Acu-=lenLines[--y];
 curPos.y=y;
 s=curLinePtr=buffer+Acu;
 Acu=o-Acu;
 for (curPos.x=0; Acu; --Acu,s++)
     { AdvanceWithTab(*s,curPos.x); }
}

/**[txh]********************************************************************

  Description:
  Helper function to put the cursor in the first line of the selection.
The first line is also selected and the position is adjusted to the
begining of the line.

***************************************************************************/

unsigned TCEditor::JumpLineStartSelection()
{
 unsigned Pos=(unsigned)(curLinePtr-buffer);
 // Is the cursor in the first line of the selection?
 if (selStart<Pos || selStart>Pos+lenLines[curPos.y])
   { // Nop
    GotoOffSet(selStart);
    if (curPos.y<delta.y)
      {
       trackCursor(True);
       AdjustDrawPtr();
      }
   }

 // Force to select the whole first line
 Pos=(unsigned)(curLinePtr-buffer);
 selStart=Pos;
 return Pos;
}

/****************************************************************************

   Function: void IndentBlock(uint32 Amount, char Fill, Boolean allowUndo)

   Type: TCEditor member.

   Objetive: indent a block.
   If Amount==0 indents to the first hole of the previous line.

   Parameter:
   Amount: number of chars to insert in each line.
   Fill: the character to insert (' ' or tab).
   allowUndo: if the operation is recorded in the undo array.

   by SET.

****************************************************************************/

void TCEditor::IndentBlock(char *Fill, Boolean allowUndo)
{
 if (isReadOnly) return;
 if (hasVisibleSelection())
   {
    // This code is pretty Waco:
    flushLine();

    if (allowUndo)
       addToUndo(undoPre1IndBlock,NULL);
    unsigned Pos=JumpLineStartSelection();
    // Save info to update de shl
    unsigned firstLine=curPos.y;
    char *firstTouchedP=curLinePtr;

    unsigned Amount;

    if (Fill)
       Amount=strlen(Fill);
    else
      {
       // That's an special case: Indent all the block taking as reference
       // the first line.
       int Xact,X=0;
       char *s=curLinePtr;

       // Search the column of the first non-blank in the first line of the block
       for (Xact=curPos.x; CLY_IsntEOL(*s) && ucisspace(*s); s++)
           { AdvanceWithTab(*s,Xact); }
       if (curPos.y>0)
         { // Search a hole in the last line
          s=curLinePtr-lenLines[curPos.y-1];

          do
           {
            for (;CLY_IsntEOL(*s) && !ucisspace(*s); s++) // While letters
                { AdvanceWithTab(*s,X); }
            for (;CLY_IsntEOL(*s) && ucisspace(*s); s++)  // While spaces
                { AdvanceWithTab(*s,X); }
           }
          while (CLY_IsntEOL(*s) && X<=Xact); // to a mayor X or the end of line
          if (X>Xact)
             Amount=X-Xact;
          else
             Amount=NextTabPos(Xact)-Xact;
         }
       else
         Amount=NextTabPos(Xact)-Xact;
       Fill=(char *)alloca(Amount+1);
       memset(Fill,' ',Amount);
       Fill[Amount]=0;
      }

    // How many lines?
    unsigned Lines,PosAux,y;
    for (Lines=0,y=curPos.y,PosAux=Pos; PosAux<selEnd;)
       {
        PosAux+=lenLines[y++];
        Lines++;
       }
    unsigned lastLine=firstLine+Lines;

    // Now let's go, do the work.

    // 1) Insert Lines bytes
    // Is the buffer large enough?
    unsigned Total=Lines*Amount;
    if (bufSize<bufLen+Total)
       setBufSize(bufLen+Total);
    memmove(buffer+PosAux+Total,buffer+PosAux,bufLen-PosAux);

    // 2) Update the selection and Buffer Length
    selEnd+=Total;
    bufLen+=Total;

    if (allowUndo)
      {
       addToUndo(undoPre2IndBlock,&Amount);
       addToUndo(undoIndBlock,Fill);
      }

    // 3) Move the lines
    unsigned Length=lenLines[--y];
    for (Pos=PosAux; Lines; --Lines)
       {
        Pos-=Length;
        // Hey! don't forget the Markers
        for (int i=0; i<10; i++)
           {
            if (Markers[i]>=Pos && Markers[i]<(Pos+Length))
               Markers[i]+=Total;
           }
        memmove(buffer+Pos+Total,buffer+Pos,Length);
        memcpy(buffer+Pos+Total-Amount,Fill,Amount);
        //buffer[Pos+Lines-1]=32;
        lenLines.set(y,Length+Amount);
        Length=lenLines[--y];
        Total-=Amount;
       }

    UpdateSyntaxHLBlock(firstLine,firstTouchedP,lastLine);
    MarkAsModified();
    update(ufView);
   }
}

/****************************************************************************

   Function: uint32 IsUnIndentable(char *buffer,uint32 Pos,uint32 Amount)

   Type: Static function.

   Objetive: Check if the line can be unIndented Amount bytes.

   Parameter:
   buffer: The buffer.
   Pos: The offset of the line in the buffer.
   Amount: The number of bytes to delete.

   Return:
   The quantity to unIndent (Amount if there is enough space or the maximun
   available).

   by SET.

****************************************************************************/

static uint32 IsUnIndentable(char *buffer,uint32 Pos,uint32 Amount)
{
 uint32 i;
 buffer+=Pos;
 for (i=0; i<Amount; i++,buffer++)
    {
     if (*buffer!=' ' && *buffer!='\t')
        return i;
    }
 return Amount;
}

/****************************************************************************

   Function: void UnIndentBlock(uint32 Amount, Boolean allowUndo)

   Type: TCEditor member.

   Objetive: Unindent a block.
   If Amount==0 unindents to the first available start of block.

   Parameter:
   Amount: number of chars to delete in each line.
   allowUndo: if the operation is recorded in the undo array. When that's
              false the routine ever eats Amount spaces because is supposed
              to be called from the undo!

   by SET.

****************************************************************************/

void TCEditor::UnIndentBlock(uint32 Amount, Boolean allowUndo)
{
 int i;
 UndoCell un;

 if (isReadOnly) return;
 if (hasSelection() && !selHided)
   {
    flushLine();

    if (allowUndo)
      {
       addToUndo(undoPreCopyInfo);
       UndoSaveStartState(un);
       un.Type=undoUnIndBlock;
      }

    unsigned Pos=JumpLineStartSelection();
    char *firstTouchedP=curLinePtr;
    un.selStart=Pos;

    if (Amount==0)
      { // That's an special case: UnIndent the block using the first line as
        // reference.
        char *s=curLinePtr,*p;
        int Y,X,Xact;

        // Search the column of the first non-blank in the first line of the block
        for (Xact=curPos.x; CLY_IsntEOL(*s) && ucisspace(*s); s++)
            { AdvanceWithTab(*s,Xact); }
        s=curLinePtr;
        X=Xact;
        Y=curPos.y;
        while (Y && X>=Xact)
          {
           s-=lenLines[--Y];
           for (p=s,X=0; CLY_IsntEOL(*p) && ucisspace(*p) && X<Xact; p++)
               { AdvanceWithTab(*p,X); }
          }
        if (X>=Xact)
           Amount=Xact;
        else
           Amount=Xact-X;
      }
    // If still in 0 => nothing to do
    if (Amount==0)
       return;
    MarkAsModified();

    unsigned Length=selEnd-selStart;
    char *buf=new char[Length+sizeof(uint32)];
    if (buf==NULL) return;
    memcpy(buf+sizeof(uint32),buffer+selStart,Length);
    *(uint32 *)buf=Amount;
    un.Length=Length;
    un.s=buf;

    // Now let's go, do the work.
    unsigned PosAux,y;
    unsigned Eated=0;
    unsigned bytesToEat;
    unsigned firstLine=curPos.y;
    for (y=curPos.y, PosAux=Pos; Pos<selEnd; ++y)
       {
        Length=lenLines[y];
        // Is UnIndentable?
        if ((bytesToEat=allowUndo ? IsUnIndentable(buffer,Pos,Amount) : Amount)!=0)
          { // Move plus del
           // Adjust the Markers
           for (i=0; i<10; i++)
              {
               if (Markers[i]>Pos && Markers[i]<(Pos+Length))
                  Markers[i]-=(Eated+bytesToEat);
               else if (Markers[i]==Pos)
                       Markers[i]-=Eated;
              }
           Eated+=bytesToEat;
           Length-=bytesToEat;
           memmove(buffer+PosAux,buffer+Pos+bytesToEat,Length);
           lenLines.set(y,Length);
           Pos+=bytesToEat;
          }
        else
          { // Only Move
           memmove(buffer+PosAux,buffer+Pos,Length);
           // Adjust the Markers
           for (i=0; i<10; i++)
              {
               if (Markers[i]>=Pos && Markers[i]<(Pos+Length))
                  Markers[i]-=Eated;
              }
          }
        PosAux+=Length;
        Pos+=Length;
       }
    if (Eated)
      {
       memcpy(buffer+PosAux,buffer+PosAux+Eated,bufLen-(PosAux+Eated));
       bufLen-=Eated;
       selEnd-=Eated;
       UpdateSyntaxHLBlock(firstLine,firstTouchedP,y);
      }

    if (allowUndo)
      {
       UndoSaveFinalState(un);
       addToUndo(undoPostCopyInfo,&un);
      }

    update(ufView);
   }
}


/****************************************************************************

   Function: void RecalculateXofLineInEdit(void)

   Type: TCEditor member.

   Objetive: Calculate the X position based on the inEditPtr pointer.
     This routine should be called only when the line is under edition and you
   need to recalculate the X position.
     Called by BackSpace and Delete when the user deletes a Tab.

   by SET.

****************************************************************************/

void TCEditor::RecalculateXofLineInEdit(void)
{
 char *s=bufEdit;
 int X=0;
 while (s!=inEditPtr)
   {
    AdvanceWithTab(*s,X);
    s++;
   }
 curPos.x=X;
}


/****************************************************************************

   Function: void BackSpace(Boolean allowUndo)

   Type: TCEditor member.

   Objetive: Just the action for the backspace key.

   by SET.

****************************************************************************/

void TCEditor::BackSpace(Boolean allowUndo)
{
 int WidthFirstChar;

 if (isReadOnly) return;
 if (!PersistentBlocks && hasSelection())
   {
    clipCut();
    return;
   }
 WidthFirstChar = ((IslineInEdition ? *bufEdit : *curLinePtr)=='\t') ? tabSize : 1;
 if (curPos.x<WidthFirstChar) // Is the first char in the line
   {
    if (curLinePtr!=buffer)
      {
       flushLine();
       deleteRange(curLinePtr-CLY_LenEOL,curLinePtr,allowUndo);
      }
   }
 else
   {
    if (!IslineInEdition)
      {
       EditLine();
       if (!IslineInEdition)
          return;
      }
    uint32 PosOfIns=(uint32)(inEditPtr-bufEdit);
    if (BackSpUnindents && IsFirstCharInLine())
      { // UnIndent
       char *s=curLinePtr,*p;
       int Y,X,Xact;
       UndoDestroy temp=destBack;

       // The undo is very complex because the routine can replace tabs
       // with spaces and is imposible to reconstruct if we don't save
       // a total copy of the line.
       if (allowUndo)
          addToUndo(undoDestroyLine,(void *)&temp);
       X=Xact=curPos.x;
       Y=curPos.y;
       while (Y && X>=Xact)
         {
          s-=lenLines[--Y];
          for (p=s,X=0; CLY_IsntEOL(*p) && ucisspace(*p) && X<Xact; p++)
              { AdvanceWithTab(*p,X); }
          if (CLY_IsEOL(*p))
             X=Xact;   // if that's an empty line force to ignore it
         }
       if (X>=Xact)
          X=0;
       //if (X>(inEditPtr-bufEdit))
       memmove(bufEdit+X,inEditPtr,restCharsInLine+1);
       memset(bufEdit,32,X);

       int dif=X-(int)PosOfIns;
       AdjustLineSel(PosOfIns,dif,True);

       // Update the markers
       for (int i=0; i<10; i++)
          {
           int Pos=MarkersInLine[i];
           if (Pos>=0)
             {
              if ((unsigned)Pos>=PosOfIns)
                 MarkersInLine[i]+=dif;
              else if (Pos>=X)
                      MarkersInLine[i]=X;
             }
          }

       inEditPtr=bufEdit+X;
       curPos.x=X;
      }
    else
      {
       if (allowUndo)
          addToUndo(undoDelChar,inEditPtr-1);
       // If one of the 2 chars is a tab the X must be recalculated
       int wasATab=*(inEditPtr-1)=='\t' || *inEditPtr=='\t';
       CLY_memcpy(inEditPtr-1,inEditPtr,restCharsInLine+1);
       AdjustLineSel((uint32)(inEditPtr-bufEdit),-1,True);

       // Update the markers
       for (int i=0; i<10; i++)
          {
           int Pos=MarkersInLine[i];
           if (Pos>=0 && (unsigned)Pos>=PosOfIns)
              MarkersInLine[i]--;
          }
       inEditPtr--;
       if (wasATab)
          RecalculateXofLineInEdit();
       else
          curPos.x--;
      }
    MarkAsModified();
    update(ufLine);
   }
 if (curPos.x<delta.x)
   {
    if (delta.x>7)
       delta.x-=8;
    else
       delta.x=0;
    update(ufView);
   }
}


/****************************************************************************

   Function: void unlockUndo(void)

   Type: TCEditor member.

   Objetive: Manages the undoGroupCount variable to allow the undo grouping.

   by SET.

****************************************************************************/

void TCEditor::unlockUndo(void)
{
 if (undoLockCount>0)
   {
    undoLockCount--;
    if (!undoLockCount)
       undoGroupCount=0;
   }
}


/****************************************************************************

   Function: void undo()

   Type: TCEditor member.

   Objetive: Undoes some action.

   by SET.

****************************************************************************/

void TCEditor::undo()
{
 if (UndoActual!=UndoBase)
   {
    int count=UndoArray[UndoActual].Count;
    do
      {
       undoOneAction();
      }
    while (count--);
   }
}

//#define DISABLE_INSERT_PATCH

void TCEditor::undoOneAction()
{
 if (UndoActual!=UndoBase)
   {
    UndoCell &un=UndoArray[UndoActual];
    char *s;

    if (IslineInEdition)
      {
       #ifndef DISABLE_INSERT_PATCH
       /* Never purge spaces if they are part of the undo */
       if (un.Type==undoInsert)
          DontPurge=1;
       #endif
       MakeEfectiveLineInEdition();
       DontPurge=0;
      }

    // Is a change, first save the end parameters
    if (UndoSt!=undoNoUndo)
       UndoSaveFinalState(un);

    switch (un.Type)
      {
       case undoInMov:
            MoveCursorTo(un.X,un.Y);
            update(ufUpdate);
            break;

       case undoPutChar:
            {
             int d;
             char *s=ColToPointer(d);
             if (d<0)
               {
                if (-d<un.Length)
                   deleteRange(s-un.Length-d,s,False);
               }
             else
                deleteRange(s-un.Length,s,False);
             EditLine();
            }
            //MoveCursorTo(un.X,un.Y);
            break;

       case undoOvrPutChar:
            MoveCursorTo(un.X,un.Y);
            if (overwrite!=(((un.Flags & undoOverWrite)!=0) ? True : False))
               toggleInsMode(False);
            EditLine();
            {
             int i;
             for (i=0; i<un.Length; i++)
                 InsertCharInLine(un.s2.s[i],False);
            }
            MoveCursorTo(un.X,un.Y);
            ComputeXLineInEdition(True);
            update(ufLine);
            break;

       case undoDelCharDel:
            insertBuffer( un.s, 0, un.Length, False, False, False );
            MoveCursorTo(un.X,un.Y);
            break;

       case undoDelChar:
            insertBuffer( un.s, 0, un.Length, False, False, False );
            MoveCursorTo(un.X,un.Y);
            break;

       case undoDelete:
            MoveCursorTo(un.Xf,un.Yf);
            insertBuffer(un.s,0,un.Length,False,False,False);
            MoveCursorTo(un.X,un.Y);
            break;

       case undoDeleteBuf:
            GotoOffSet(un.s2.OffSet);
            insertBuffer(un.s,0,un.Length,False,False,False);
            MoveCursorTo(un.X,un.Y);
            break;

       case undoInsert:
           {
            MoveCursorTo(un.X,un.Y);
            int dif;
            s=ColToPointer(dif);
            deleteRange(s,s+un.Length,False);
            if (un.s2.l)
               insertBuffer(un.s2.BufL->s,0,un.s2.BufL->len,False,False,True);
            #ifndef DISABLE_INSERT_PATCH
            else
               // Reconstruct the potentially damaged spaces
               if (dif<0)
                 {
                  MoveCursorTo(un.X,un.Y);
                  EditLine();
                 }
            #endif
           }
            break;

       case undoDestroyLine:
            MoveCursorTo(un.X,un.Y);
            deleteRange(curLinePtr,curLinePtr+LenWithoutCRLF(un.Y,curLinePtr),False);
            // We must be sure the line is inserted at the beginning
            MoveCursorTo(0,un.Y);
            insertBuffer(un.s,0,un.Length,False,False,False);
            MoveCursorTo(un.X,un.Y);
            if (un.Flags & undoLineInEd)
               EditLine();
            break;

       case undoIndBlock:
            selStart=un.selStartf;
            selEnd=un.selEndf;
            selHided=False;
            UnIndentBlock(un.Length,False);
            MoveCursorTo(un.X,un.Y);
            break;

       case undoUnIndBlock:
            deleteRange(buffer+un.selStartf,buffer+un.selEndf,False);
            insertBuffer(un.s,sizeof(uint32),un.Length,False,False,False);
            MoveCursorTo(un.X,un.Y);
            break;

       case undoToUpper:
       case undoToLower:
       case undoRecodeBlock:
       case undoInvertCase:
       case undoAltCase:
            deleteRange(buffer+un.selStartf,buffer+un.selEndf,False);
            insertBuffer(un.s,0,un.Length,False,False,False);
            MoveCursorTo(un.X,un.Y);
            break;

       case undoRectDel:
       case undoRectPaste:
            UndoRectangularPasteClear(un);
            break;

       case undoRectCopy:
            delete selRectClip;
            selRectClip=DuplicateRectSt(un.s);
            update(ufUpdate);
            break;

       case undoRectHide:
            selRectHided=Boolean(un.Length);
            update(ufView);
            break;

       case undoRectStart:
            {
             selRecSt *s=(selRecSt *)un.s;
             Xr1=s->Xr1;
             Yr1=s->Yr1;
             update(ufView);
            }
            break;

       case undoRectEnd:
            {
             selRecSt *s=(selRecSt *)un.s;
             Xr2=s->Xr2;
             Yr2=s->Yr2;
             update(ufView);
            }
            break;

       default:
            messageBox(__("Unhandled undo"),mfError | mfOKButton);
      }

    Boolean newBool=((un.Flags & undoModified)!=0) ? True : False;
    if (modified!=newBool)
      {
       modified=newBool;
       // Note: we don't restore the modifiedCounter, currently the use of this
       // variable is limited so it doesn't affect, but perhaps a write from scratch
       // should simply use the counter.
       update(ufUpdate);
      }
    newBool=((un.Flags & undoSelHided)!=0) ? True : False;
    if (un.selStart!=selStart || un.selEnd!=selEnd || selHided!=newBool)
      {
       selStart=un.selStart;
       selEnd=un.selEnd;
       selHided=newBool;
       update(ufView);
      }
    newBool=((un.Flags & undoSelRecHided)!=0) ? True : False;
    if (newBool!=selRectHided)
      {
       selRectHided=newBool;
       update(ufView);
      }
    if (IslineInEdition)
      {
       selNewStart=un.selStart;
       selNewEnd=un.selEnd;
      }
    if (overwrite!=(((un.Flags & undoOverWrite)!=0) ? True : False))
       toggleInsMode(False);

    UndoSt=undoNoUndo;

    DecWithWrap(UndoActual,MAX_UNDO);
   }
}

/**[txh]********************************************************************

  Description:
  It sets the modified flag for all the previous entries in the array.
That's called after a save.

***************************************************************************/

void TCEditor::RevertModifFlagInUndo()
{
 int pos;

 for (pos=0; pos<MAX_UNDO; pos++)
     UndoArray[pos].Flags|=undoModified;
}

/****************************************************************************

   Function: void redo(void)

   Type: TCEditor member.

   Objetive: Redoes an action.

   by SET.

****************************************************************************/

void TCEditor::redo(void)
{
 if (UndoActual!=UndoTop)
   {
    char *s;

    if (IslineInEdition)
       MakeEfectiveLineInEdition();

    IncWithWrap(UndoActual,MAX_UNDO);
    UndoSt=undoNoUndo;
    UndoCell &un=UndoArray[UndoActual];

    switch (un.Type)
      {
       case undoInMov:
            MoveCursorTo(un.Xf,un.Yf);
            update(ufUpdate);
            break;

       case undoPutChar:
            MoveCursorTo(un.X,un.Y);
            insertBuffer( un.s, 0, un.Length, False, False, False );
            break;

       case undoOvrPutChar:
            MoveCursorTo(un.X,un.Y);
            if (overwrite!=(((un.Flags & undoOverWrite)!=0) ? True : False))
               toggleInsMode(False);
            EditLine();
            {
             int i;
             for (i=0; i<un.Length; i++)
                 InsertCharInLine(un.s[i],False);
            }
            update(ufLine);
            break;

       case undoDelCharDel:
            s=ColToPointer();
            deleteRange(s,s+un.Length,False);
            break;

       case undoDelChar:
            s=ColToPointer();
            deleteRange(s-un.Length,s,False);
            break;

       case undoDelete:
            MoveCursorTo(un.X,un.Y);
            s=ColToPointer();
            deleteRange(s,s+un.Length,False);
            MoveCursorTo(un.Xf,un.Yf);
            break;

       case undoDeleteBuf:
            s=buffer+un.s2.OffSet;
            deleteRange(s,s+un.Length,False);
            MoveCursorTo(un.Xf,un.Yf);
            break;

       case undoInsert:
            MoveCursorTo(un.X,un.Y);
            insertBuffer( un.s, 0, un.Length, False, False, False );
            MoveCursorTo(un.Xf,un.Yf);
            break;

       case undoDestroyLine:
            MoveCursorTo(un.X,un.Y);
            BackSpace(False);
            break;

       case undoIndBlock:
            selStart=un.selStart;
            selEnd=un.selEnd;
            selHided=False;
            IndentBlock(un.s,False);
            MoveCursorTo(un.Xf,un.Yf);
            break;

       case undoUnIndBlock:
            UnIndentBlock(*((uint32 *)un.s),False);
            break;

       case undoToUpper:
            BlockToUpper(False);
            break;

       case undoToLower:
            BlockToLower(False);
            break;

       case undoInvertCase:
            BlockInvertCase(False);
            break;

       case undoAltCase:
            BlockAltCase(False);
            break;

       case undoRecodeBlock:
            RemapCodePageBuffer(un.s2.Recode->sourID,un.s2.Recode->destID,
                                un.s2.Recode->ops,False);
            break;

       case undoRectDel:
            selRectDelete(Xr1,Yr1,Xr2,Yr2,False);
            break;

       case undoRectPaste:
            selRectPaste(selRectClip,curPos.x,curPos.y,False);
            break;

       case undoRectCopy:
            delete selRectClip;
            selRectClip=DuplicateRectSt(un.s2.s);
            update(ufUpdate);
            break;

       case undoRectHide:
            selRectHided=Boolean(!selRectHided);
            update(ufView);
            break;

       case undoRectStart:
            {
             selRecSt *s=(selRecSt *)un.s;
             Xr1=s->Xr2;
             Yr1=s->Yr2;
             update(ufView);
            }
            break;

       case undoRectEnd:
            {
             selRecSt *s=(selRecSt *)un.s;
             Xr2=s->Xr1;
             Yr2=s->Yr1;
             update(ufView);
            }
            break;

       default:
            messageBox(__("Unhandled redo"),mfError | mfOKButton);
      }

    Boolean newBool=((un.Flags & undoModifiedF)!=0) ? True : False;
    if (modified!=newBool)
      {
       modified=newBool; // See comment in similar sentence
       update(ufUpdate);
      }
    newBool=((un.Flags & undoSelHidedF)!=0) ? True : False;
    if (un.selStartf!=selStart || un.selEndf!=selEnd || selHided!=newBool)
      {
       selStart=un.selStartf;
       selEnd=un.selEndf;
       selHided=newBool;
       update(ufView);
      }
    newBool=((un.Flags & undoSelRecHidedF)!=0) ? True : False;
    if (newBool!=selRectHided)
      {
       selRectHided=newBool;
       update(ufView);
      }
    if (overwrite!=(((un.Flags & undoOverWriteF)!=0) ? True : False))
       toggleInsMode(False);

    // Recall the function until we finish with the chain
    if (UndoActual!=UndoTop)
      {
       int aux=UndoActual;
       IncWithWrap(aux,MAX_UNDO);

       while (UndoArray[aux].Count)
         {
          redo();
          if (UndoActual==UndoTop) break;
          IncWithWrap(aux,MAX_UNDO);
         }
      }
   }
}



/****************************************************************************

   Function: void UndoSaveFinalState(UndoCell &un)

   Type: TCEditor member.

   Objetive: Fill the undo structure with the status of the editor at the
   end of the action.

   by SET.

****************************************************************************/

void TCEditor::UndoSaveFinalState(UndoCell &un)
{
 un.Xf=curPos.x;
 un.Yf=curPos.y;
 if (IslineInEdition)
   {
    un.selStartf=selNewStart;
    un.selEndf=selNewEnd;
   }
 else
   {
    un.selStartf=selStart;
    un.selEndf=selEnd;
   }
 if (selHided)        un.Flags|=undoSelHidedF;
 if (selRectHided)    un.Flags|=undoSelRecHidedF;
 if (modified)        un.Flags|=undoModifiedF;
 if (overwrite)       un.Flags|=undoOverWriteF;
 if (IslineInEdition) un.Flags|=undoLineInEd;
}


/****************************************************************************

   Function: void UndoSaveStartState(UndoCell &un)

   Type: TCEditor member.

   Objetive: Fill the undo structure with the status of the editor at the
   start of the action.

   by SET.

****************************************************************************/

void TCEditor::UndoSaveStartState(UndoCell &un)
{
 un.X=curPos.x;
 un.Y=curPos.y;
 if (IslineInEdition)
   {
    un.selStart=selNewStart;
    un.selEnd=selNewEnd;
   }
 else
   {
    un.selStart=selStart;
    un.selEnd=selEnd;
   }
 un.Flags=0;
 if (selHided)        un.Flags|=undoSelHided;
 if (selRectHided)    un.Flags|=undoSelRecHided;
 if (modified)        un.Flags|=undoModified;
 if (overwrite)       un.Flags|=undoOverWrite;
 if (IslineInEdition) un.Flags|=undoLineInEd;
}

/****************************************************************************

   Function: void addToUndo(UndoState st, void *p)

   Type: TCEditor member.

   Objetive: Add an action to the undo list.

   by SET.

****************************************************************************/

void TCEditor::addToUndo(UndoState st, void *p)
{
 unsigned cont;

 UndoCell &un=UndoArray[UndoActual];

 if (un.Count<=undoGroupCount)
 if (st==UndoSt)
   {
    switch (st) // Grouped things
      {
       case undoInMov: // The cursor movement is grouped
            return;
       // The chars inserted one by one are stored in one cell
       case undoDelCharDel:
       case undoPutChar:
            if (un.Length && !(un.Length%UNDO_CHARS_SIZE))
              {
               char *s=(char *)realloc(un.s,un.Length+UNDO_CHARS_SIZE);
               UndoArray[UndoActual].s=s;
               s[un.Length++]=*((char *)p);
               return;
              }
            un.s[un.Length++]=*((char *)p);
            return;

       case undoOvrPutChar:
            if (un.Length && !(un.Length%UNDO_CHARS_SIZE))
              {
               UndoArray[UndoActual].s=(char *)realloc(un.s,un.Length+UNDO_CHARS_SIZE);
               UndoArray[UndoActual].s2.s=(char *)realloc(un.s2.s,un.Length+UNDO_CHARS_SIZE);
               UndoArray[UndoActual].s[un.Length]=*((char *)p);
               UndoArray[UndoActual].s2.s[un.Length++]=*((char *)p+1);
               return;
              }
            un.s[un.Length]=*((char *)p);
            un.s2.s[un.Length++]=*((char *)p+1);
            return;

       // The same but in reverse order
       case undoDelChar:
            if (un.Length && !(un.Length%UNDO_CHARS_SIZE))
              {
               char *s=(char *)realloc(un.s,un.Length+UNDO_CHARS_SIZE);
               UndoArray[UndoActual].s=s;
               memmove(s+1,s,un.Length++);
               s[0]=*((char *)p);
               return;
              }
            memmove(un.s+1,un.s,un.Length++);
            un.s[0]=*((char *)p);
            return;

       // If he repeat the pre-delete info simply ingnore it, that's made to
       // allow a preset of the data from some place different to deleteRange.
       // BUT not the pointer.
       case undoPreDelete:
            un.s=(char *)p;
            un.s2.l=0;
            return;

       // Second time is the final X,Y
       //case undoDifSpaces:
       //     return;
       default:
            break;
      }
   }

 // undoToUpper undoToLower undoUnIndBlock undoRectPaste undoRectDel
 // undoRecodeBlock undoInvertCase undoAltCase
 if (st==undoPostCopyInfo)
   {
    memcpy(&UndoArray[UndoActual],p,sizeof(UndoCell));
    UndoArray[UndoActual].Count=undoGroupCount;
    if (undoLockCount)
       undoGroupCount++;
    return;
   }

 // First cames a predelete and then the delete
 if (st==undoDelete)
   {
    char *s=(char *)UndoArray[UndoActual].s;
    unsigned l=(unsigned)((char *)p-s);

    un.Length=l;
    un.s=new char[l];
    UndoSt=un.Type=st;
    memcpy(un.s,s,l);
    return;
   }

 // First comes a predelete and then the deletebuf
 if (st==undoDeleteBuf)
   {
    char *s=(char *)UndoArray[UndoActual].s;
    un.s2.OffSet=(unsigned)((char *)s-buffer); // Where to place the buffer

    unsigned l=(unsigned)((char *)p-s);
    un.Length=l;                // How many bytes

    un.s=new char[l];
    UndoSt=un.Type=st;
    memcpy(un.s,s,l);
    return;
   }

 // First cames a preinsert and then the insert
 if (st==undoInsert)
   {
    // A pointer to the data
    struct stUndoInsert *stI=(struct stUndoInsert *)p;

    uint32 l=un.Length=stI->l;
    un.s=new char[l];
    memcpy(un.s,stI->s,l);
    un.s2.BufL=stI->Eated;
    UndoSt=un.Type=st;
    return;
   }


 if (st==undoPre2IndBlock)
   {
    un.Length=*((uint32 *)p);
    return;
   }

 if (st==undoIndBlock)
   {
    un.s=newStr((char *)p);
    UndoSt=un.Type=st;
    return;
   }

 // Is a change, first save the end parameters first
 UndoSaveFinalState(un);

 // Increment the pointers
 IncWithWrap(UndoActual,MAX_UNDO);
 if (UndoActual==UndoBase)
   {
    do
      {
       freeUndoCell(UndoBase);
       IncWithWrap(UndoBase,MAX_UNDO);
      }
    while (UndoArray[UndoBase].Count && UndoActual!=UndoBase);
   }
 IncWithWrap(UndoTop,MAX_UNDO);
 while (UndoTop!=UndoActual)
   {
    DecWithWrap(UndoTop,MAX_UNDO);
    freeUndoCell(UndoTop);
   }

 UndoSt=st;
 UndoCell &un2=UndoArray[UndoActual];
 un2.Type=st;
 // Increment now, some cases returns.
 un2.Count=undoGroupCount;

 switch (st)
   {
    case undoPre1IndBlock:
    case undoInMov:
         UndoSaveStartState(un2);
         break;

    case undoOvrPutChar:
         un2.s2.s=new char[UNDO_CHARS_SIZE];
         un2.s2.s[0]=*((char *)p+1);
    case undoDelChar:
    case undoDelCharDel:
    case undoPutChar:
         un2.Length=1;
         un2.s=new char[UNDO_CHARS_SIZE];
         un2.s[0]=*((char *)p);
         UndoSaveStartState(un2);
         break;

    case undoRectCopy:
         un2.s=(char *)DuplicateRectSt((char *)selRectClip);
         un2.s2.s=(char *)DuplicateRectSt((char *)p);
         UndoSaveStartState(un2);
         break;

    case undoRectStart:
    case undoRectEnd:
    case undoPreDelete:
         un2.s=(char *)p;
         UndoSaveStartState(un2);
         break;

    case undoPreInsert:
         UndoSaveStartState(un2);
         break;

    // When we eat or put spaces like in the unindent in backspace
    /*
    case undoDifSpaces:
         un2.Length=*((int *)p);
         UndoSaveStartState(un2);
         break;*/

    // That's only to put a point between two movements
    case undoCutInMov:
         UndoSt=un2.Type=undoInMov;
         UndoSaveStartState(un2);
         break;

    case undoDestroyLine:
         cont=(unsigned)(inEditPtr-bufEdit+restCharsInLine);
         un2.Length=cont;
         un2.s=new char[cont+1];
         memcpy(un2.s,bufEdit,cont+1);
         UndoSaveStartState(un2);
         break;

    case undoRectHide:
         un.Length=(int)*((Boolean *)p);
         break;

    // That's used by the routines that handle the undo by itself
    case undoPreCopyInfo:
         return;
    default:
         break;
   }

 if (undoLockCount)
    undoGroupCount++;
}

/**[txh]********************************************************************

  Description:
  This function release all the undo information. Must be called by routines
that doesn't have undo to avoid a mess.

***************************************************************************/

void TCEditor::flushUndoInfo()
{
 while (UndoBase!=UndoTop)
   {
    freeUndoCell(UndoTop);
    DecWithWrap(UndoTop,MAX_UNDO);
   }
 freeUndoCell(UndoTop);
 UndoActual=UndoBase;
}

/****************************************************************************

   Function: void freeUndoCell(int Index)

   Type: TCEditor member.

   Objetive: Eliminates an undo cell in the list.

   by SET.

****************************************************************************/

void TCEditor::freeUndoCell(int Index)
{
 switch (UndoArray[Index].Type)
   {
    // This must free the allocated memory
    case undoRectCopy:
    case undoOvrPutChar:
    case undoRecodeBlock:
         delete UndoArray[Index].s2.s;
    case undoAltCase:
    case undoDelChar:
    case undoDelCharDel:
    case undoDelete:
    case undoDeleteBuf:
    case undoIndBlock:
    case undoInsert:
    case undoInvertCase:
    case undoPutChar:
    case undoRectDel:
    case undoRectEnd:
    case undoRectPaste:
    case undoRectStart:
    case undoToLower:
    case undoToUpper:
    case undoUnIndBlock:
         DeleteArray(UndoArray[Index].s);
         break;
    default:
         break;
   }
 UndoArray[Index].Type=undoNoUndo;
}

void TCEditor::TurnOnCHighLight(void)
{
 formatLinePtr=&TCEditor::formatLineHighLight;
 SyntaxHL=shlCSyntax;
}

void TCEditor::TurnOnPascalHighLight(void)
{
 formatLinePtr=&TCEditor::formatLineHighLightPascal;
 SyntaxHL=shlPascalSyntax;
}

void TCEditor::TurnOnClipperHighLight(void)
{
 formatLinePtr=&TCEditor::formatLineHighLightClipper;
 SyntaxHL=shlClipperSyntax;
}

void TCEditor::TurnOnGenericHighLight(void)
{
 formatLinePtr=&TCEditor::formatLineHighLightGeneric;
 SyntaxHL=shlGenericSyntax;
}

/**[txh]********************************************************************

  Description:
  This function copies the syntax highlight information selected by id to
an static cache. That's in this way because then editorfo.cc have references
to this static buffer symplifying the addressing mode and hence using less
CPU registers.
@p
  All the functions that needs to use the cached value must call this member
before. If the value is already cached it returns inmediatly. That means
is fast.

***************************************************************************/

void TCEditor::CacheSyntaxHLData(int id)
{
 if ((id!=strCid || SyntaxHL!=strCtype) && SHLArray && SHLValueSelected>=0)
   {
    strSHL *s=&SHLArray[SHLValueSelected];
    // If the user words aren't loaded do it now
    if (!(s->Flags1 & FG1_UserWordsTried))
       LoadUserWords(s,id);
    // If the keywords aren't loaded now is time
    if (!s->Keywords)
       LoadSyntaxHighLightKeywords(*s);
    memcpy(&strC,s,sizeof(strSHL));
    // Check if we use the internal version
    /*if (SyntaxHL==shlGenericSyntax)
       memcpy(&strC,s,sizeof(strSHL));
    else
      { // Cache only a few things
       strC.Name=s->Name;
       strC.Flags1=s->Flags1;
       strC.UserWords=s->UserWords;
       strC.SearchUserWords=s->SearchUserWords;
      }*/
    // Memorize what is cached
    strCid=id;
    strCtype=SyntaxHL;
   }
}

/****************************************************************************

   Function: readBlock(TCEditor *editor)

   Type: Normal function.

   Objetive: Used to read a file and insert it as a block in the text.

   from Robert adapted to the new class.

****************************************************************************/

static void readBlock(TCEditor *editor)
{
 char fname[PATH_MAX];

 if (editor->isReadOnly)
    return;

 strcpy(fname,"*");

 if (TCEditor::editorDialog(edReadBlock,fname)!=cmCancel)
   {
    flushLine2(editor);
    // Save the cursor pos
    uint32 c = (uint32)(editor->ColToPointer()-editor->buffer);
    //  Create a new editor for this file, this is made in this way to get
    // an automatic conversion for UNIX files.
    TCEditor *feditor = new TCEditor(TRect(0,0,1,1),NULL,NULL,NULL,fname);
    // If non-persistent blocks kill the selection
    if (!editor->PersistentBlocks && editor->hasSelection())
       editor->clipCut();
    editor->lock();
    editor->selHided=False;
    // Insert all the new editor
    editor->insertBuffer(feditor->buffer,0,feditor->bufLen,True,True);
    // Destroy the temporal editor
    delete feditor;
    // Put the cursor in the original position
    editor->GotoOffSet(c);
    editor->trackCursor(False);
    // Update the view
    editor->unlock();
    editor->update(ufView);
   }
}

/****************************************************************************

   Function: writeBlock(TCEditor *editor)

   Type: Normal function.

   Objetive: Used to write a block to a file.

   from Robert adapted to the new class.

****************************************************************************/

static void writeBlock(TCEditor *editor)
{
 if (editor->selHided || !editor->hasSelection())
    return;

 char fname[PATH_MAX];
 strcpy(fname,"*");

 if (TCEditor::editorDialog(edSaveBlock,fname)!=cmCancel)
   {
    FILE *f;
    struct stat st;
    int error=0;
    if (stat(fname,&st)==0)
      {
       if (S_ISREG(st.st_mode))
         {
          if (TCEditor::editorDialog(edFileExists,fname,0)!=cmYes)
             return;
         }
       else
         {
          if (TCEditor::editorDialog(edFileNoFile,fname)!=cmYes)
             return;
         }
      }
    f=fopen(fname,"w+b");
    if (f)
      {
       flushLine2(editor);
       fwrite(editor->buffer+editor->selStart,1,editor->selEnd-editor->selStart,f);
       error=ferror(f);
       fclose(f);
      }
    else
      error=1;
    if (error)
       TCEditor::editorDialog(edWriteError,fname);
   }
}

/**[txh]********************************************************************

  Description:
  Saves the contents of the buffer to a temporal file and returns the name
of the file. The file have "di" as prefix.
  
  Return: A malloced name for the temporal file containing the buffer. If it
failed NULL is returned.
  
***************************************************************************/

char *TCEditor::saveToTemp()
{
 char *name=unique_name("di",NULL);
 if (!name)
    return NULL;
 FILE *f=fopen(name,"wt");
 int error=0;
 if (f)
   {
    flushLine();
    fwrite(buffer,1,bufLen,f);
    error=ferror(f);
    fclose(f);
   }
 else
   error=1;
 if (error)
   {
    editorDialog(edWriteError,name);
    free(name);
    return NULL;
   }
 return name;
}

static
void GetFileMode(CLY_mode_t *mode, struct stat *statVal, char *fileName)
{
 CLY_GetFileAttributes(mode,statVal,fileName);
 // We will modify the file so we set this bit in advance
 CLY_FileAttrModified(mode);
}

static
FILE *ExpandToTempIfNeeded(FILE *f, char *&temp, char *name,
                           int &IsCompressed)
{
 IsCompressed=GZFiles_IsGZ(f);
 if (!IsCompressed)
    return f;
 fclose(f);
 char *tmp=unique_name("gz");
 if (!tmp)
   {
    TCEditor::editorDialog(edCreateTMPError);
    return NULL;
   }

 if (GZFiles_ExpandHL(tmp,name))
    return NULL;

 temp=tmp;
 return fopen(tmp,"rb");
}

/****************************************************************************

   Function: Boolean loadFile()

   Type: TCEditor member.

   Objetive: Loads a file into the editor buffer, checks if is a UNIX file,
             in this case the file is converted to DOS style.

   Returns: True if the file was loaded. False if an error ocurred. If the
   file doesn't exist it assumes a new one is wanted, an empty editor is
   created and True is returned, but the FailedToLoad flag is set.

   Originally by Robert.
   5/97 Mod. by Robert to (1) Use fopen (2) Only convert files in DOS
        (3) Take in count that Linux can convert DOS files on MS-DOS mounted
        drives.

****************************************************************************/

#ifdef CLY_UseCrLf
#define RemoveTemporal() if (wasCompressed) { remove(wasCompressed); \
                                              free(wasCompressed); \
                                              wasCompressed=0; } \
                         if (!crfound) remove(tmp); \
                         string_free(tmp);
#else
#define RemoveTemporal() if (wasCompressed) { remove(wasCompressed); \
                                              free(wasCompressed); \
                                              wasCompressed=0; } \
                         if (crfound) remove(tmp); \
                         string_free(tmp);
#endif

Boolean TCEditor::loadFile(Boolean setSHL)
{
 int i,crfound=0;
 char *tmp=0;
 struct stat s;
 char *wasCompressed=0;

 FailedToLoad=False;
 DiskTime=0;
 lastTimeCheck=0;
 NoNativeEOL=False;
 // This structure differentiate the file
 FillEditorId(&EditorId);

 // We need some information about the file later. First I tried using fstat
 // but under DOS stat can give much more accurate information because we
 // provide the name of the file. In particular fstat fails under DOS+NETX
 if (stat(fileName,&s)!=0)
   { // I don't know if that's the best, I just assume the file isn't there
    if (setSHL)
       SHLSelect(*this,buffer,0);
    setBufLen(0);
    FailedToLoad=True;
    return True;
   }
 // Get the current attributes/rights
 GetFileMode(&ModeOfFile,&s,fileName);
 FILE *f=fopen(fileName,"rb");

 if (!f)
   { // New file, it's OK
    if (setSHL)
       SHLSelect(*this,buffer,0);
    setBufLen(0);
    FailedToLoad=True;
    return True;
   }
   
 // Get the modification time (from stat)
 DiskTime=s.st_mtime;
 time(&lastTimeCheck);
 FillEditorId(&EditorId,0,&s);
 // Check if we can write
 if (CLY_FileAttrIsRO(&ModeOfFile))
   {
    if (!isReadOnly && !(editorFlags & efDoNotWarnRO) &&
        editorDialog(edIsReadOnly,fileName)==cmYes)
      {
       // Close it, DOS 6 fails if we change the mode while opened
       fclose(f);
       CLY_FileAttrReadWrite(&ModeOfFile);
       if (!CLY_SetFileAttributes(&ModeOfFile,fileName))
         {
          editorDialog(edStillReadOnly);
          // Check if the user wants to mark them as R.O.
          if (editorFlags & efROasRO)
             isReadOnly=True;
          // Undo the change to reflect reality
          CLY_FileAttrReadOnly(&ModeOfFile);
         }
       f=fopen(fileName,"rb");
       if (!f)
         {
          editorDialog(edReadError,fileName);
          return False;
         }
      }
    else
      // Check if the user wants to mark them as R.O.
      if (editorFlags & efROasRO)
         isReadOnly=True;
   }

 Boolean oldBusy=TScreen::showBusyState(True);
 f=ExpandToTempIfNeeded(f,wasCompressed,fileName,IsaCompressedFile);
 if (!f)
   {
    TScreen::showBusyState(oldBusy);
    editorDialog(edReadError,fileName);
    return False;
   }

 // Check for a unix text file (is a heuristic, because only 1024 chars checked
 {
  char tmpbuf[1024];
  memset(tmpbuf,0,1024);
  long fsize=filelength(fileno(f));
  if (fsize > 1024) fsize=1024;
  fread(tmpbuf,fsize,1,f);
  for (i=0; i<fsize; i++)
     {
      if (tmpbuf[i]==13)
         crfound=1;
      else
        if (tmpbuf[i]==10)
           break;
     }
  #ifdef CLY_UseCrLf
  // DOS: Check if the file is in UNIX format, in this case convert it
  if (crfound)
     fseek(f,0,SEEK_SET);
  else
    {
     /* This uses the feature of DJGPP to convert automatically
        LF's to CR/LF's when writing in TEXT-mode */
     FILE *ftemp;
     NoNativeEOL=True;
     fseek(f,0,SEEK_SET);
     tmp=unique_name("ed");
     ftemp=fopen(tmp,"w+t");
     while ((fsize = fread(tmpbuf,1,1024,f)) > 0)
       fwrite(tmpbuf,1,fsize,ftemp);
     fclose(ftemp);
     fclose(f);
     f=fopen(tmp,"rb");
    }
  #else
  // UNIX: Check if the file is in DOS format, in this case convert it
  if (crfound)
    {
     FILE *ftemp;
     NoNativeEOL=True;
     fseek(f,0,SEEK_SET);
     tmp=unique_name("ed");
     ftemp=fopen(tmp,"w+t");

     ssize_t len;
     char *line=0;
     size_t lenLine=0;
     while ((len=CLY_getline(&line,&lenLine,f))!=-1)
       {
        if (len>=2 && line[len-2]=='\r')
          {
           len--;
           line[len-1]='\n';
          }
        fwrite(line,len,1,ftemp);
       }
     free(line);

     fclose(ftemp);
     fclose(f);
     f=fopen(tmp,"rb");
    }
  else
     fseek(f,0,SEEK_SET);
  #endif
 }
 unsigned long fSize=filelength(fileno(f));
 if (!setBufSize(fSize))
   {
    TScreen::showBusyState(oldBusy);
    editorDialog(edOutOfMemory);
    RemoveTemporal();
    return False;
   }
 unsigned long realRead;
 /* On linux you can (like I) mount an MS-DOS filesystem with
    conv=auto, which converts CR/LF to LF. In that case
    realRead < fSize [Robert] */
 realRead=fread(buffer,1,fSize,f);
 fclose(f);
 if (realRead==0 && fSize!=0) // 1999/04/08: fSize=0 is not an error ;-)
   {
    TScreen::showBusyState(oldBusy);
    editorDialog(edReadError,fileName);
    RemoveTemporal();
    return False;
   }
 if (setSHL)
    SHLSelect(*this,buffer,realRead);
 isValid=True;
 setBufLen(realRead);
 TScreen::showBusyState(oldBusy);
 RemoveTemporal();
 return isValid;
}

Boolean TCEditor::reLoadFile()
{// Free memory we won't use anymore
 flushUndoInfo();
 // Load the file again
 return loadFile();
}

/**[txh]********************************************************************

  Description: 
  Saves the editor buffer to disk, this file just desides if we need to call
saveAs or saveFile.
  
  Return: True on success.
  
***************************************************************************/

Boolean TCEditor::save()
{
 if (isReadOnly)
    return False;
 if (*fileName==EOS)
    return saveAs();
 else
    return saveFile();
}

Boolean TCEditor::saveSameTime()
{
 if (isReadOnly)
    return False;
 if (DiskTime && *fileName!=EOS)
    return saveFile(False,False,True);
 editorDialog(edNotFromDisk);
 return False;
}

/* No longer used now the user sees the name s/he wants and I use the device and inode
   to differentiate the file (suggested by Eli)
#ifdef TVCompf_djgpp
static void FExpand(char *name)
{
 if (_USE_LFN)
    return;
 char tmp[PATH_MAX];
 // First force 8+3 if not LFN
 _truename(name,tmp);
 // Now force Unix style names
 fexpand(tmp);
 strcpy(name,tmp);
}
#else
inline
static void FExpand(char *name)
{
 fexpand(name);
}
#endif
*/

/****************************************************************************

   Function: Boolean saveAs()

   Type: TCEditor member.

   Objetive: Saves the editor buffer to disk as a new file. Call with true
             to save as UNIX. There are a shortcut called saveAsUnix.

   by SET.

****************************************************************************/

Boolean TCEditor::saveAs(Boolean ConvertEOL, Boolean AvoidAutoConvert)
{
 Boolean revertToReadOnly=False;
 if (isReadOnly)
   {
    CLY_FileAttrReadWrite(&ModeOfFile);
    isReadOnly=False;
    revertToReadOnly=True;
   }
 Boolean res=False;
 char *oldName=0;

 oldName=strdup(fileName);
 if (editorDialog(edSaveAs,fileName)!=cmCancel)
   {
    if (access(fileName,F_OK)==0)
      {
       // If the user is trying to save the same file in UNIX format let do it
       if (editorDialog(edFileExists,fileName,(ConvertEOL || AvoidAutoConvert)
           && strcmp(oldName,fileName)==0)!=cmYes)
         {
          strcpy(fileName,oldName);
          free(oldName);
          return False;
         }
      }
    if (IsaCompressedFile && editorDialog(edFileCompMant)==cmNo)
       IsaCompressedFile=0;
    if ((res=saveFile(ConvertEOL,AvoidAutoConvert))==True)
      {
       message(owner,evBroadcast,cmcUpdateTitle,0);
       SHLSelect(*this,buffer,bufLen);
       update(ufView);
       revertToReadOnly=False;
      }
    else
      strcpy(fileName,oldName);
    if (isClipboard())
       *fileName=EOS;
   }
 if (revertToReadOnly)
   {
    CLY_FileAttrReadOnly(&ModeOfFile);
    isReadOnly=True;
   }
 free(oldName);
 return res;
}


/*static void writeBlock( ofstream& f, char *buf, unsigned len )
{
 while( len > 0 )
  {
   int l = len < INT_MAX ? len : INT_MAX;
   f.write( buf, l );
   buf += l;
   len -= l;
  }
} No longer used*/

const int bkpMoved=1,bkpCopied=0,bkpNone=2;

/**[txh]********************************************************************

  Description:
  Used to copy the original file to the back-up. It will just change the
name for regular files, but if the file is a symlink it will copy the file.

  Return:
  !=0 on success

***************************************************************************/

static
int FileCopyOrMove(char *file, char *newFile, int &whichUsed)
{
 if (strcmp(file,newFile)==0)
    return 0;
 if (IsASoftLink(file))
   {
    whichUsed=bkpCopied;
    return FileCopy(file,newFile);
   }
 whichUsed=bkpMoved;
 #ifdef SEOS_Win32
 unlink(newFile);
 #endif
 return rename(file,newFile)==0;
}

/**[txh]********************************************************************

  Description: Saves the editor buffer to disk as a new file, as is or
converting the EOL format. Boolean ConvertEOL: If true changes the EOL to
DOS/UNIX according to the current OS. Note that memory buffers are always
in the native format.

  Return:
  True on success

***************************************************************************/

Boolean TCEditor::saveFile(Boolean ConvertEOL, Boolean AvoidAutoConvert,
                           Boolean noChangeTime)
{
 if (NoNativeEOL && (editorFlags & efSaveEOLasis) && !AvoidAutoConvert)
    ConvertEOL=True;

 int actionUsed=bkpNone;
 if ((editorFlags & efBackupFiles) &&
     edTestForFile(fileName) && // Forget about it if that's a new file
     (MakeBkpForIt==0 || MakeBkpForIt(fileName)))
   {
    char backupName[PATH_MAX];
    strcpy(backupName,fileName);

    if (editorFlags & efUNIXBkpStyle)
       AddToNameOfFile(backupName,"~");
    else
       ReplaceExtension(backupName,(char *)backupExt);

    if (!FileCopyOrMove(fileName,backupName,actionUsed))
      {
       if (editorDialog(edCantBkp)!=cmYes)
          return False;
      }
    else
      {
       // Now set the original attributes, but only if we used a copy
       if (actionUsed==bkpCopied)
          CLY_SetFileAttributes(&ModeOfFile,backupName);
       if (editorFlags & efHiddenBkps)
          MakeFileHidden(backupName);
      }
    AddToFilesToKill(backupName);
   }
 else
   {// We are not going to back-up the file.
    // Check if it was a RO and the user asked not to warn.
    if ((editorFlags & efDoNotWarnRO) && CLY_FileAttrIsRO(&ModeOfFile))
      {// Ok, we will most probably fail to replace the file
       if (editorDialog(edIsReadOnly,fileName)==cmYes)
         {
          CLY_FileAttrReadWrite(&ModeOfFile);
          if (!CLY_SetFileAttributes(&ModeOfFile,fileName))
            {// Bad luck
             editorDialog(edStillReadOnly);
             // Undo the change to reflect reality
             CLY_FileAttrReadOnly(&ModeOfFile);
             // We will go on, the user could be root and finally
             // overwrite the file.
            }
         }
      }
   }

 TGZFileWrite *f=new TGZFileWrite(fileName,IsaCompressedFile);

 if (!f->ok)
   {
    editorDialog(edCreateError,fileName);
    delete f;
    return False;
   }
 else
   {
    flushLine();

    if (!ConvertEOL)
       f->write(buffer,bufLen);
    else
      {
       unsigned y;
       int Limit=limit.x;
       // A little bit more for a CR and negative offsets
       char *s=(char *)malloc(Limit+4+4);
       if (!s)
         {
          editorDialog(edOutOfMemory);
          return False;
         }
       char *cur=buffer;
       int l;
       s[0]=s[1]=s[2]=s[3]=0;
       s+=4; // To allow negative offsets

       for (y=0; y<=totalLines; y++)
          {
           l=lenLines[y];
           if (l>Limit)
             {
              Limit=limit.x=l;
              s-=4; // Back to the allocated pointer
              s=(char *)realloc(s,Limit+4+4);
              if (!s)
                {
                 editorDialog(edOutOfMemory);
                 return False;
                }
              s[0]=s[1]=s[2]=s[3]=0;
              s+=4;
             }
           memcpy(s,cur,l);
           cur+=l;
           #ifdef CLY_UseCrLf
           if (s[l-1]=='\n' && s[l-2]=='\r')
             {
              s[l-2]='\n';
              l--;
             }
           #else
           // In UNIX is save as DOS
           if (s[l-1]=='\n' && s[l-2]!='\r')
             {
              s[l-1]='\r';
              s[l]='\n';
              l++;
             }
           #endif
           f->write(s,l);
          }
       s-=4; // Back to the allocated pointer
       free(s);
      }

    if (!f->ok)
      {
       editorDialog( edWriteError, fileName );
       delete f;
       return False;
      }
    else
      {
       struct stat s;
       char aux[PATH_MAX+30],*auxIntl;

       modified=False;
       update(ufUpdate);
       RevertModifFlagInUndo();
       delete f;
       if (NoNativeEOL!=ConvertEOL)
          updateCommands(1);
       // The buffer *is* native, if we converted the file is no native.
       NoNativeEOL=ConvertEOL;
       // If we moved the file to do the backup we created a new file and hence it have
       // the umask default attributes and not the right ones.
       if (actionUsed==bkpMoved)
          CLY_SetFileAttributes(&ModeOfFile,fileName);
       if (stat(fileName,&s)==0)
         {
          auxIntl=TVIntl::getTextNew(__("Saved: %s (%ld bytes %d lines)."));
          CLY_snprintf(aux,PATH_MAX+30,auxIntl,fileName,(long)s.st_size,totalLines+1);
          FillEditorId(&EditorId,0,&s);
          // Update it we don't know if all went OK
          GetFileMode(&ModeOfFile,&s,fileName);
          if (!noChangeTime)
             DiskTime=s.st_mtime;
         }
       else
         {
          auxIntl=TVIntl::getTextNew(__("Stat failed."));
          strcpy(aux,auxIntl);
         }
       DeleteArray(auxIntl);
       setStatusLine(aux);
       if (noChangeTime)
         { // Set the modif. and access time to the previous
          struct utimbuf utm;
          utm.actime=utm.modtime=DiskTime;
          utime(fileName,&utm);
         }
       time(&lastTimeCheck);
      }
   }
 return True;
}


Boolean TCEditor::valid( ushort command )
{
 if (command==cmValid)
    return isValid;
 else
   {// If we are quiting or the user closed the window check if we must save
    if (command==cmQuit || command==cmClose)
      {
       if (modified==True)
         {
          int d;
          if (*fileName==EOS)
             d=edSaveUntitled;
          else
             d=edSaveModify;
    
          switch( editorDialog( d, fileName ) )
            {
             case cmYes:
                  return save();
             case cmNo:
                  modified=False;
                  return True;
             case cmCancel:
                  return False;
            }
         }
      }
   }
 return True;
}

/**[txh]********************************************************************

  Description:
  Computes the length of the column markers array.
  
***************************************************************************/

int TCEditor::LenColMarkers(uint32 *markers)
{
 if (!markers) return 0;
 int i=0;
 while (markers[i]!=lastColMarker)
   i++;
 return i;
}

/**[txh]********************************************************************

  Description:
  Creates a copy of the column markers array.
  
***************************************************************************/

uint32 *TCEditor::CopyColMarkers(uint32 *markers)
{
 int l=LenColMarkers(markers);
 if (!l) return 0;
 l++;
 uint32 *ret=new uint32[l];
 memcpy(ret,markers,l*sizeof(uint32));
 return ret;
}

/**[txh]********************************************************************

  Description:
  Reads the columns markers array from disk.
  
***************************************************************************/

uint32 *TCEditor::LoadColMarkers(ipstream& is)
{
 uchar aux;
 is >> aux; // Version
 is >> aux; // How many?

 if (!aux) return 0;

 int c=aux;
 uint32 *ret=new uint32[c+1];
 is.readBytes(ret,c*sizeof(uint32));
 ret[c]=lastColMarker;

 return ret;
}

/**[txh]********************************************************************

  Description:
  Writes the columns markers array to disk.
  
***************************************************************************/

void TCEditor::SaveColMarkers(opstream& os, uint32 *markers)
{
 const uchar version=1;
 os << version;
 uchar c=(uchar)LenColMarkers(markers);
 os << c;

 if (c)
    os.writeBytes(markers,c*sizeof(uint32));
}

void TCEditor::ColMarkers2Str(uint32 *markers, char *str, unsigned maxLen)
{
 char b[64];
 int l=LenColMarkers(markers),i;
 unsigned lenThis,len=0;

 str[0]=0;
 for (i=0; i<l; i++)
    {
     itoa(markers[i],b,10);
     lenThis=strlen(b);
     if (len+lenThis+2<maxLen)
       {
        strcat(str,b);
        len+=lenThis;
        if (i+1<l)
          {
           strcat(str," ");
           len++;
          }
       }
    }
}

static
int compareUint32(const void *k1, const void *k2)
{
 return *((uint32 *)k1)-*((uint32 *)k2);
}

uint32 *TCEditor::Str2ColMarkers(char *str)
{
 char *s=str,*end;
 int count=0,i;

 do
   {
    strtoul(s,&end,10);
    if (s!=end)
      {
       s=end;
       if (*s) s++;
       count++;
      }
    else
       break;
   }
 while (1);

 if (!count)
    return 0;

 uint32 *markers=new uint32[count+1];
 for (i=0, s=str; i<count; i++)
    {
     markers[i]=(uint32)strtoul(s,&end,10);
     s=end;
     if (*s) s++;
    }
 markers[count]=lastColMarker;
 qsort(markers,count,sizeof(uint32),compareUint32);

 return markers;
}

#define DEBUG_SPLINES_UPDATE 0

/**[txh]********************************************************************

  Description:
  Changes the list of special lines for a new one. The object will keep a
copy of the list. The lines that changed are updated if they are visible.
  
***************************************************************************/

void TCEditor::SetSpecialLines(TSpCollection *nLines)
{
 // Move the drawPtr to the first visible line
 AdjustDrawPtr();

 int y1=delta.y, y2=delta.y+size.y, y, i, j, type;
 TSpCollection *oLines;

 if (DEBUG_SPLINES_UPDATE)
    printf("\n\nTCEditor::SetSpecialLines\n");

 oLines=SpecialLines;
 SpecialLines=NULL;

 // We are painting individual lines without calling doUpdate, so we have to
 // clean some things before.
 if (IsHLCOn)
    update(ufClHLCh);

 if (oLines)
   {// We already have them redraw the affected lines
    int cnt=oLines->getCount();
    if (DEBUG_SPLINES_UPDATE)
       printf("Processing %d old splines\n",cnt);
    for (i=0; i<cnt; i++)
       {
        stSpLine *st=oLines->At(i);
        y=st->nline;
        type=st->id;
        if (DEBUG_SPLINES_UPDATE)
           printf("%d) y=%d type=%d\n",i+1,y,type);
        if (y>=y1 && y<y2 && (type==idsplBreak || type==idsplRunLine))
          {// This line is visible
           // Check if it will change
           int found=0;
           if (nLines)
             {
              int cnt=nLines->getCount();
              for (j=0; !found && j<cnt; j++)
                 {
                  stSpLine *st=nLines->At(j);
                  if (st->nline==y && st->id!=idsplError)
                     // It was painted
                     found=1;
                 }
             }
           if (!found)
             {// This line is no longer special or changed its type
              if (DEBUG_SPLINES_UPDATE)
                 printf("%d) Painting %d as normal\n",i+1,y);
              unsigned p=drawPtr;
              int ya;
              for (ya=y1; ya<y; ya++)
                  p+=lenLines.safeLen(ya);
              drawLines(y,1,p);
             }
           else
             {
              if (DEBUG_SPLINES_UPDATE)
                 printf("%d) Found %d skipping\n",i+1,y);
             }
          }
       }
   }

 SpecialLines=nLines;
 if (nLines)
   {// We got a new set, draw the affected lines
    int cnt=nLines->getCount();
    if (DEBUG_SPLINES_UPDATE)
       printf("Processing %d new splines\n",cnt);
    for (i=0; i<cnt; i++)
       {
        stSpLine *st=nLines->At(i);
        y=st->nline;
        type=st->id;
        if (DEBUG_SPLINES_UPDATE)
           printf("%d) y=%d type=%d\n",i+1,y,type);
        if (y>=y1 && y<y2 && (type==idsplBreak || type==idsplRunLine))
          {// This line is visible
           int found=0;
           // 1) If another will be painted over it just skip
           for (j=i-1; j>=0; j--)
              {
               stSpLine *st=nLines->At(j);
               if (st->nline==y)
                 {
                  found=1;
                  if (DEBUG_SPLINES_UPDATE)
                     printf("%d) Skip because %d is over (y=%d)\n",i+1,j+1,y);
                  break;
                 }
              }
           // 2) Check if it's changing
           if (!found && oLines)
             {
              int cnt=oLines->getCount();
              // From the last, that's the one visible
              for (j=0; j<cnt; j++)
                 {
                  stSpLine *st=oLines->At(j);
                  if (st->nline==y && st->id!=idsplError)
                    {// We found an spline there
                     if (st->id==type)
                       {
                        if (DEBUG_SPLINES_UPDATE)
                           printf("%d) Skip because old %d was the same (y=%d)\n",i+1,j+1,y);
                        // Same type, skip
                        found=1;
                       }
                     break;
                    }
                 }
             }
           if (!found)
             {// This line is no longer special or changed its type
              if (DEBUG_SPLINES_UPDATE)
                 printf("%d) Painting %d as special\n",i+1,y);
              unsigned p=drawPtr;
              int ya;
              for (ya=y1; ya<y; ya++)
                  p+=lenLines.safeLen(ya);
              drawLines(y,1,p);
             }
           else
             {
              if (DEBUG_SPLINES_UPDATE)
                 printf("%d) Found %d skipping\n",i+1,y);
             }
          }
       }
   }

 if (DEBUG_SPLINES_UPDATE)
    printf("Now we have %d splines\n\n",SpecialLines ? SpecialLines->getCount() : 0);
}

/**[txh]********************************************************************

  Description: 
  Is the default function to handle the dialogs, the editor must define a
real one.
  
***************************************************************************/

unsigned defEditorDialog( int, ... )
{
 return cmCancel;
}

int FillEditorId(stEditorId *id, const char *name, struct stat *st)
{
 #if defined(SEOS_Win32) && !defined(SECompf_Cygwin)
 // This function must fail on WIN32 systems.
 // Inode number reported by stat is always 0.
 if (st) st=0;
 if (name) name=0;
 #else
 if (st && (st->st_dev!=0 || st->st_ino!=0))
   {
    id->dev=st->st_dev;
    id->inode=st->st_ino;
    return 1;
   }
 struct stat s;
 if (name && stat(name,&s)==0)
   {
    id->dev=s.st_dev;
    id->inode=s.st_ino;
    return 1;
   }
 #endif

 id->dev=0;
 id->inode=0;
 return 0;
}

int CompareEditorId(stEditorId *id1, stEditorId *id2)
{
 return id1->dev==id2->dev && id1->inode==id2->inode;
}

int IsEmptyEditorId(stEditorId *id)
{
 return id->dev==0 && id->inode==0;
}

Boolean TCEditor::checkDiskCopyChanged(Boolean force)
{
 /* Read Only editors are like snap-shots, don't reload */
 /* Don't be fooled by new files, they aren't on disk! */
 if (isReadOnly || !DiskTime)
    return False;
 time_t now;
 time(&now);
 if (!force && !forceNextTimeCheck &&
     difftime(now,lastTimeCheck)<minDifModCheck)
    return False;
 lastTimeCheck=now;
 forceNextTimeCheck=False;
 struct stat st;
 if (stat(fileName,&st)==0)
   {
    if (difftime(st.st_mtime,DiskTime)>0)
      {// To avoid problems we assume the user will reload the file or doesn't
       // care about it. So, in order to avoid a storm of questions and dialogs
       // we set the time of the buffer in memory to the time of the file on disk.
       DiskTime=st.st_mtime;
       return True;
      }
   }
 return False;
}

TEditorDialog
         TCEditor::editorDialog=defEditorDialog;
unsigned TCEditor::editorFlags=efBackupFiles | efPromptOnReplace;
char     TCEditor::findStr[maxFindStrLen]="";
char     TCEditor::replaceStr[maxReplaceStrLen]="";
TCEditor *
         TCEditor::clipboard=0;
int      TCEditor::colorsCached=0;
uint32   TCEditor::minDifModCheck=8;
unsigned TCEditor::staticTabSize=8;
unsigned TCEditor::staticIndentSize=4;
unsigned TCEditor::LoadingVersion;
Boolean  TCEditor::staticUseTabs=False;
Boolean  TCEditor::staticAutoIndent=True;
Boolean  TCEditor::staticIntelIndent=False;
Boolean  TCEditor::staticPersistentBlocks=False;
Boolean  TCEditor::staticCrossCursorInRow=False;
Boolean  TCEditor::staticCrossCursorInCol=False;
Boolean  TCEditor::staticShowMatchPair=True;
Boolean  TCEditor::staticShowMatchPairFly=True;
Boolean  TCEditor::staticShowMatchPairNow=True;
Boolean  TCEditor::staticTransparentSel=True;
Boolean  TCEditor::staticOptimalFill=False;
Boolean  TCEditor::staticDontPurgeSpaces=False;
Boolean  TCEditor::staticSeeTabs=True;
Boolean  TCEditor::staticNoInsideTabs=True;
Boolean  TCEditor::staticTabIndents=False;
Boolean  TCEditor::staticUseIndentSize=False;
Boolean  TCEditor::staticWrapLine=False;
Boolean  TCEditor::staticBackSpUnindents=False;
Boolean  TCEditor::staticNoMoveToEndPaste=False;
Boolean  TCEditor::staticColumnMarkers=False;
uint32  *TCEditor::staticColMarkers=0;
char     TCEditor::oTabChar=0xB1;
char     TCEditor::TabChar =0xB1;
ushort   TCEditor::SearchInSel=0;
ushort   TCEditor::FromWhere=0;
ushort   TCEditor::RegExStyle=0;
ushort   TCEditor::ReplaceStyle=0;
ushort   TCEditor::CanOptimizeRegEx=0;
char     TCEditor::StatusLine[setMaxScreenX*2];
strSHL  *TCEditor::SHLArray=NULL;
strSHL   TCEditor::strC;
int      TCEditor::strCid=-1;
shlState TCEditor::strCtype=shlNoSyntax;
int      TCEditor::SHLCant=0;
Boolean  TCEditor::Recording=False;
int      TCEditor::MacroCount=0;
int      TCEditor::staticWrapCol=78;
int      TCEditor::DontPurge=0;
int      TCEditor::DontLoadFile=0;
TCEditor *TCEditor::showMatchPairFlyCache=NULL;
TSubMenu *TCEditor::RightClickMenu=0;
TPMCollection *TCEditor::PMColl=NULL;
TStringCollection *TCEditor::SHLGenList=NULL;
TSArray<unsigned int> TCEditor::MacroArray(28,32);
TCommandSet TCEditor::cmdsAux;
const char *TCEditor::backupExt=".bkp";
unsigned char TCEditor::SHLTableUse[4]={0,0,0,0};
dflOptions TCEditor::dflOps={0,0xFFFFFFFF,0,0};
int (*TCEditor::MakeBkpForIt)(const char *name)=0;
