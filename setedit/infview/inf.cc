/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*****************************************************************************

   Filename -       inf.cc

   Functions:
                    Member functions of following classes
                        TInfViewer
                        TInfWindow
                        THelpControlDialog
                        TStrListBox

History: (for Inf.Cpp and InfBase.Cpp)
5/5/96 v0.0.7
Added: BestMatch.
       Robert modifications for cursor visibility and use.
       Local help in ^H.
       Support for a StatusLine.
Modifyed: ALT-F10 and ALT-F1 now are commands.
          Double line links with speed optimization.
7/5/96
Modified: getLine of TInfTopic from char *getLine(int) to void getLine(int,
          char *) for 2 reasons: 1) It's more safe 2) supress an extra
          strcpy in TInfViewer::draw().

12/6/96
Modified: selWord=0 in draw() to avoid warnings. (Robert)
Added:    drawView(); switchToTopic because some times the window isn't
          updated (when there is no cursor movement between topics). (Robert)
Modified: TInfIndicator was replaced by a change of title. (Robert)
Modified: An error in MakeVisible (-1).

??/10/96
Added:    Support for compressed files.
Added:    International support.

04/11/96 v0.0.8
Modified: Parser of menus and * aaaa: aaa.XXXXXX.

12/11/96 v0.0.9
Modified: to hide the *Note string.

13/11/96
Modified: the methode to hide *Notes.
Added: A little selection and paste to the clipboard feature.

1/12/96
Added: isTheOne to manage the info from the standalone editor.
Modified: Now getTopic is safer when the file doesn't exist.

9/12/96
Added: Match of topic while writing the initials. (requested by NIK).

10/12/96
Bug Killed: '0' didn't check if the number of refs. was 0. (reported by Artur).
Added: A help context to each dialog. (requested by Artur).

4/1/97
Added: Now the selection disapears when we change of topic.
Added: Selection in reverse direction.
Added: New behaviour of End.
Added: Selection with End,Home,Page Up and Page Down.
Added: Mouse selection.

9/1/97
Added: Support for files without Tag Table (just plain text files with delimiters).

10/1/97
Added: Better support for Tabs.
Added: Ported to Linux (no shift arrows, double click emulated).
Fixed: Now the items in the first line below the Menu entry is taked in count.
Added: A dialog to choose any node of the file.
v0.1.0

14/1/97
Added: Code to show only the true link and not the node, is much better.
Added: Better support for links like "TInfView::read".
Modified: In ^O now I'm using a non-owner collection because that can produce a disaster.
          Now I'm creating the collection with the final size to speed-up.
Added: Two modes to control the Node-cut and the link hiding.
Added: A dialog box to control the modes.

15/1/97
Added: Now the TInfViewer and TInfWindow are stremable, saves:
       1) The TWindow data.
       2) The TScroller data.
       3) The search options.
       4) Node & topic position.
       5) The configuration flags.
       6) The history.
Modified: The history is now a THisCollection (new).

21/1/97
Modified: Some things in the parser.
v0.1.1

26/1/97
Modified: Now TInfIndex uses a collection for the nodes and the collection
uses a SOStack for the names of the nodes:
Advantages: The nodes and positions are parsed and stored in 1 pass. (not 2).
            The wasted memory is less.
            The search in the node list is faster.
            The ^O doesn't need to create a temporal collection.
Disadvantages: The SOStack can need some realloc calls forcing copies depending on
               how fragmented is the heap.
               Some simple tasks (but used only a few times) are slower because
               needs to call some virtual members. (atStr for example to get the name
               of the node).

1/2/97
Added: A command to paste the #include of the topic in the clipboard.
v0.2.0

2/1/97
Added: Now when we type first letters and the real name of the link is hided the
       program makes match with the text that we see on the screen not the real
       name of the link.
v0.2.1

8/4/97
Added: Now ExpandName checks for the name of the file first so if the name is
       full path qualified or exists in the current directory is opened.

23/6/97
Modified: Now the Bookmark is static
v0.2.2

2/9/97
Added: Open Info dialog, buttons in the Control dialog to call the rest of the
       dialogs.
v0.2.3

13/11/97
Fixed: Underscore was ignored in incremental searchs.

21/04/98
Fixed: Now when we load a file splitted in the current dir it doesn't fail.
v0.2.4

16/06/98
Fixed: Node names in seek where tested using strncmp, that's ok, but if both
names start with the same letters they match even when the node isn't the
desired one.

ToDo:
  When pressing the initials accumulate the keystrokes, works but needs some
adjustments, for example I didn't take care about names crossing lines.
  Enhance the selection.

*****************************************************************************/

// That's the first include because is used to configure the editor.
#include "ceditint.h"

#define Uses_TStreamableClass
#define Uses_TPoint
#define Uses_TStreamable
#define Uses_TRect
#define Uses_TScrollBar
#define Uses_TScroller
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_TWindow
#define Uses_TKeys
#define Uses_TKeys_Extended
#define Uses_TPalette
#define Uses_TButton
#define Uses_TListBox
#define Uses_TLabel
#define Uses_TDialog
#define Uses_MsgBox
#define Uses_TFindDialogRec
#define Uses_TInputLine
#define Uses_TCheckBoxes
#define Uses_TRadioButtons
#define Uses_THistory
#define Uses_TSItem
#define Uses_TStatusLine
#define Uses_TFrame
#define Uses_TStringCollection
#define Uses_TSortedListBox
#define Uses_TFileDialog
#define Uses_TCommandSet
#define Uses_TVOSClipboard
#define Uses_TVCodePage

#define Uses_TCEditor_External
#define Uses_TCEditor_Commands
#define Uses_FileOpenAid
#define Uses_Progress
#define Uses_string
#define Uses_ctype

// InfView stuff
#define Uses_TGrowDialog
#define Uses_TSOSSortedListBox
#include <infr.h>

#include <ceditor.h>
#include <diaghelp.h>
#include <dyncat.h>
#include <inf.h>

#ifdef TVOS_UNIX
# include <stdlib.h>
# define _USE_LFN 1
#else
# include <io.h>
# include <fcntl.h>
#endif


//#define DEBUG
#ifdef DEBUG
# ifndef FOR_EDITOR
# include <nointl.h>
# endif
#endif

extern char *strncpyZ(char *dest, const char *orig, int size);
extern ushort execDialog( TDialog *d, void *data );
TDialog *createNodeChooseDialog();
TDialog *createConfigDialog();

#define getshiftstate()  event.keyDown.shiftState


#define kbFlagsShift (kbRightShift | kbLeftShift)
#define kbCtrlFlg    kbCtrlShift
#define kbAltFlg     kbAltShift
#define kbShiftFlg   kbFlagsShift

#ifdef STANDALONE
void RemoveFromEditorsHelper(TInfWindow *p);
#endif

/***************************************************************
 class TInfViewer
***************************************************************/

TInfViewer::TInfViewer( const TRect& bounds, TScrollBar* aHScrollBar,
    TScrollBar* aVScrollBar, TInfFile *aInfFile, char *context,
    char *match )
    : TScroller( bounds, aHScrollBar, aVScrollBar )
{
 char NameCopy[MAX_NODE_NAME];

 InitCommandSet();
 modeFlags=moinCutNodeWord | moinHideNodeLink;
 // pumm!!! bug killed, fucking strings in code by GCC, i need a buffer not
 // a simple string (my fault).
 strcpy(NameCopy,context);

 lockCount=0;
 mustBeRedrawed=False;

 options |= ofSelectable;
 growMode = gfGrowHiX | gfGrowHiY;
 hFile = aInfFile;
 int suggY=0;
 topic = aInfFile->getTopic(NameCopy,1,modeFlags,suggY);
 topic->setWidth(size.x);
 setLimit(topic->maxWidth() + size.x, topic->numLines() + size.y);
 QuickLen=0;

 int LaPego;
 if ((selected=topic->selBestMatch(match,LaPego))!=0)
//    selected = 1;
// else
   {
    cursor.x=topic->crossRefs[selected-1].offset;
    int y=topic->crossRefs[selected-1].line-1;
    if (y>(size.y>>1))
      {
       delta.y=y-(size.y>>1);
       cursor.y=size.y>>1;
      }
    else
      {
       delta.y=y;
       cursor.y=1;
      }
    delta.x=0;
    scrollTo(cursor.x,y);
   }
 else
    if (suggY)
      {
       cursor.y=suggY;
       scrollTo(cursor.x,suggY);
      }

 TextSelected = False;
 findStr[0]=0;
 findType=0;
 findWhere=fitInTopic;
 SearchArmed=0;
 lastSearch=0;
 OpenVerbose=1;

 // History
 HistPoint=0;
 History= new THisCollection(10,10);
 stHisto *h = new stHisto;
 strcpy(h->Name,"(");
 strcat(h->Name,hFile->NameOfFile);
 strcat(h->Name,")");
 strcat(h->Name,context);
 h->HisPos=cursor+delta;
 h->HisSel=selected;
 History->insert((void *)h);

 state |= sfCursorVis;

 if (LaPego)
   {
    switchToTopic(topic->crossRefs[selected-1].Name);
    cursor.x=delta.x=cursor.y=delta.y=0;
   }

 SetTitle( hFile->NameOfFile, topic->Node );

 selRowStart=selRowEnd=selColStart=selColEnd=0;
 selecting=False;

 // BookMark
 if (!BookMark)
    BookMark=new TNoCaseStringCollection(10,10);
}


TInfViewer::~TInfViewer()
{
 delete hFile;
 delete topic;
 CLY_destroy(History);
}

static int PointInCrossRef(TPoint point,TInfTopic *topic,int & i)
{
 if (!topic)
    return -1;
 int crossCount = topic->numRefs;
 i = 0;
 if (crossCount == 0) return -1;
 point.y++;
 for (i=0;i<crossCount;i++)
    {
     CrossRef & ref = topic->crossRefs[i];
     if (ref.line > point.y) return -1;
     // Support for crossRefs that cross lines, slow
     if (ref.line == (point.y-1))
       {
        if (ref.linebreak && point.x<=ref.linebreak)
           return i;
        continue;
       }
     if (ref.line < point.y) continue;
     if (ref.offset > point.x) return -1;
     if (ref.offset+ref.length <= point.x) continue;
     return i;
    }
 i = crossCount;
 return -1;
}

void TInfViewer::resetCursor()
{
  int before;
  TScroller::resetCursor();
  selected = PointInCrossRef(cursor+delta,topic,before)+1;
}

void TInfViewer::scrollDraw()
{
 TPoint  d;

 if( hScrollBar != 0 )
     d.x = hScrollBar->value;
 else
     d.x = 0;

 if( vScrollBar != 0 )
     d.y = vScrollBar->value;
 else
     d.y = 0;

 if ( (d.x - delta.x) != cursor.x || (d.y - delta.y) != cursor.y)
 {
   if (d.x - delta.x >= size.x)
   {
     delta.x = d.x - size.x + 1;
   }
   else if (d.x - delta.x < 0)
   {
     delta.x = d.x;
   }
   if (d.y - delta.y >= size.y)
   {
     delta.y = d.y - size.y + 1;
   }
   else if (d.y - delta.y < 0)
   {
     delta.y = d.y;
   }
   setCursor(d.x-delta.x,d.y-delta.y);
   if( drawLock != 0 )
       drawFlag = True;
   else
       drawView();
 }
}

void TInfViewer::changeBounds( const TRect& bounds )
{
 TScroller::changeBounds(bounds);
 topic->setWidth(size.x);
 setLimit(topic->maxWidth() + size.x, topic->numLines() + size.y);
}

static void moveChar(int indent, char c, char attr, int count, ushort *b)
{
 ushort val;
 uchar *p=(uchar *)&val;
 p[0]=c; p[1]=attr;

 for (b+=indent; count; count--, b++)
     *b=val;
}

static void moveStr(int indent, char *buffer, char attr, ushort *b)
{
 char *s=(char *)(b+indent);

 while (*buffer)
   {
    *s=*buffer;
    *(s+1)=attr;
    s+=2;
    buffer++;
   }
}

void TInfViewer::setState(uint16 aState, Boolean enable)
{
 TScroller::setState(aState,enable);
 if (aState==sfActive)
    updateCommands(1);
}

void TInfViewer::setCmdState(uint16 command, Boolean enable)
{
 if (enable && (state & sfActive))
    enableCommand(command);
 else
    disableCommand(command);
}

void TInfViewer::InitCommandSet()
{
 if (ts)
    return;
 ts=new TCommandSet();

 ts->enableCmd(cmcFind);
 ts->enableCmd(cmcCopyClipWin);
 ts->enableCmd(cmcCopy);
 ts->enableCmd(cmcSearchAgain);

 ts->enableCmd(cmInfHelp);
 ts->enableCmd(cmInfControl);
 ts->enableCmd(cmInfBack);
 ts->enableCmd(cmInfPasteIn);
 ts->enableCmd(cmInfBookM);
 ts->enableCmd(cmInfNodes);
 ts->enableCmd(cmInfGoto);
 ts->enableCmd(cmInfOpen);
 ts->enableCmd(cmInfDir);
 ts->enableCmd(cmInfTop);

 ts->enableCmd(cmhNext);
 ts->enableCmd(cmhPrev);
 ts->enableCmd(cmhUp);
 ts->enableCmd(cmhPrevH);
 ts->enableCmd(cmhHide);
 ts->enableCmd(cmhNodeList);
 ts->enableCmd(cmhBookMarks);
 ts->enableCmd(cmhConfigDia);
 ts->enableCmd(cmhOpenInfo);
 ts->enableCmd(cmhHistSel);
}

void TInfViewer::DisableAllCommands()
{
 if (!ts)
    InitCommandSet();
 if (ts)
    disableCommands(*ts);
}

void TInfViewer::updateCommands(int full)
{
 if (!(state & sfActive))
   { // We lost the focus, disable all
    DisableAllCommands();
    return;
   }

 // Enable in block then go for the particular ones
 if (full && ts)
    enableCommands(*ts);

 int deltaY=selRowEnd-selRowStart+1;
 int haveSel=deltaY>1 || (deltaY==1 && selColEnd>selColStart);
 setCmdState(cmcCopyClipWin,(TVOSClipboard::isAvailable() && haveSel) ? True : False);
 setCmdState(cmcCopy,(InsertRoutine && haveSel) ? True : False);
 setCmdState(cmInfPasteIn,InsertRoutine ? True : False);
 setCmdState(cmcSearchAgain,Boolean(SearchArmed));
}


void TInfViewer::draw()
{
 if (lockCount)
   {
    mustBeRedrawed=True;
    return;
   }
 mustBeRedrawed=False;

 ushort b[256];
 char linea[256],*line;
 char buffer[256];
 char *bufPtr;
 unsigned char aux;
 int i, j, l, k;
 int keyCount;
 ushort normal, keyword, selKeyword, c,selWord=0,colSelArea;
 TPoint keyPoint;
 uchar keyLength;
 char *keyRef;
 int NumCrossRefs=topic->getNumCrossRefs();
 int LineaReal;
 int Exceso;
 char *bf=(char *)b;

 normal = getColor(1);
 keyword = getColor(2);
 selKeyword = getColor(3);
 if (TextSelected)
    selWord = getColor(4);
 colSelArea=0x70;
 keyCount = 0;
 keyPoint.x = 0;
 keyPoint.y = 0;
 topic->setWidth(size.x);

 // Saltear las crossrefs que est n antes en la pantalla
 if (NumCrossRefs > 0)
     {
      do
       {
        topic->getCrossRef(keyCount++, keyPoint, keyLength, keyRef, Exceso);
       }
      while ( (keyCount < NumCrossRefs) && (keyPoint.y <= delta.y));
     }

 for (i = 1, LineaReal=delta.y+1; i <= size.y; ++i,LineaReal++)
     {
     topic->getLine(LineaReal,linea);
     if (linea[0]==30)
       {
        aux=(unsigned char)linea[1];
        if (aux<='9')
           c=(aux-'0')<<4;
        else
           c=(aux-(unsigned char)'A'+10)<<4;
        aux=(unsigned char)linea[2];
        if (aux<='9')
           c+=aux-'0';
        else
           c+=aux-(unsigned char)'A'+10;
        line=linea+3;
       }
     else
       {
        c=normal;
        line=linea;
       }
     moveChar(0, ' ', c, size.x, b); // Attr pedido
       
     if ((int)strlen(line) > delta.x)
         {
         bufPtr = line + delta.x;
         strncpy(buffer, bufPtr, size.x);
         buffer[size.x] = 0;
         moveStr(0, buffer, c, b);
         }
     else
         moveStr(0, "", c, b);


     while (LineaReal == keyPoint.y)
         {
         if (Exceso>0)
            l = keyLength-Exceso;
         else
            l = keyLength;
         if (keyPoint.x < delta.x )
           {
            l -= (delta.x - keyPoint.x);
            keyPoint.x = delta.x;
           }
         if (keyCount == selected)
            c = selKeyword;
         else
            c = keyword;
         for (j = 0; j < l; ++j)
             bf[(keyPoint.x - delta.x + j)*2+1]=c;
         if (Exceso>0)
           {
            keyPoint.x=0;
            keyPoint.y++;
            keyLength=Exceso+1;
            Exceso=0;
           }
         else
           {
            ++keyCount;
            if (keyCount <= NumCrossRefs)
                topic->getCrossRef(keyCount-1, keyPoint, keyLength, keyRef, Exceso);
            else
                keyPoint.y = 0;
           }
         }

     // Paint the selection
     if (LineaReal>=selRowStart && LineaReal<=selRowEnd)
       {
        int start;
        if (selRowStart==selRowEnd)
          {
           l=selColEnd-selColStart;
           if ( selColStart<delta.x )
             {
              l -= (delta.x - selColEnd);
              start = 0;
             }
           else
              start = selColStart - delta.x;
          }
        else
          {
           if (LineaReal==selRowStart)
             {
              start=selColStart-delta.x;
              l=size.x-start;
             }
           else
             if (LineaReal==selRowEnd)
               {
                start=0;
                l=selColEnd-delta.x;
               }
             else
               {
                start=0;
                l=size.x;
               }
          }
        for(j = 0, k=start*2+1; j < l; ++j, k+=2)
            bf[k] = (bf[k] & 0x0F) | colSelArea;
       }

     // Paint the founded word
     if (TextSelected && (LineaReal>=LineStartSelect && LineaReal<=LineEndSelect))
       {
        int start;
        l = ColEndSelect-ColStartSelect;
        if ( ColStartSelect<delta.x )
          {
           l -= (delta.x - ColStartSelect);
           start = 0;
          }
        else
           start = ColStartSelect - delta.x;
        for(j = 0, k=start*2+1; j < l; ++j, k+=2)
            bf[k]=selWord;
       }
     writeLine(0, i-1, size.x, 1, b);
     }
 if (state & sfActive)
    updateCommands();
}

void TInfViewer::SetTitle(char *File, char *Node)
{
 if (owner) /* is a window ? */
   {
    char *intlFmt=TVIntl::getTextNew(__("InfView - File: %s - Node: %s"));
    char *newTitle=new char[strlen(intlFmt)+strlen(File)+strlen(Node)-3];

    sprintf(newTitle,intlFmt,File,Node);
    DeleteArray(intlFmt);
    
    delete (char *)((TWindow *)owner)->title;
    ((TWindow *)owner)->title = (const char *)newTitle;
    ((TWindow *)owner)->frame->drawView();
   }
}


TPalette& TInfViewer::getPalette() const
{
    static TPalette palette(cInfViewer, sizeof( cInfViewer)-1);
    return palette;
}

void TInfViewer::makeSelectVisible()
{
 int ProxLine;
 TPoint keyPoint;
 uchar keyLength;
 char *keyRef;

 topic->getCrossRef(selected-1, keyPoint, keyLength, keyRef, ProxLine);
 if (QuickLen)
    keyPoint.x+=QuickVisPos-1;
 makeVisible(keyPoint,keyLength);
}


void TInfViewer::makeVisible( TPoint& keyPoint, int largo )
{
 if ((keyPoint.x+largo)>(delta.x+size.x) && (size.x>=largo))
    scrollTo(keyPoint.x+largo-1,keyPoint.y-1);
 else
    scrollTo(keyPoint.x,keyPoint.y-1);
}

void TInfViewer::switchToTopic( char *keyRef, TPoint Pos )
{
 char NameCopy[MAX_NODE_NAME];

 // Kill the selection
 selRowStart=selRowEnd=selColStart=selColEnd=0;
 selecting=False;

 // Because i can destroy the original bellow, keyRef is from topic and i go
 // to detroy it.
 strcpy(NameCopy,keyRef);

 if (topic != 0)
   {
    delete topic;
    topic=NULL;
   }
 int suggY;
 topic=hFile->getTopic(NameCopy,OpenVerbose,modeFlags,suggY);
 if (Pos.y<0) Pos.y=suggY;

 SetTitle(hFile->NameOfFile,topic->Node);

 topic->setWidth(size.x);
 setLimit(topic->maxWidth() + size.x, topic->numLines() + size.y);
 scrollTo(Pos.x,Pos.y);
 // No search
 TextSelected=False;
 lastSearch=topic->Text;
 drawView();
}

void TInfViewer::switchToTopic(char *keyRef)
{
 TPoint Pos;

 AddToHistory(keyRef);
 Pos.x=0; Pos.y=-1;
 selected=0;
 switchToTopic( keyRef,Pos );
}

void TInfViewer::switchToTopic(stHisto *h)
{
 AddToHistory(h->Name);
 selected=h->HisSel;
 switchToTopic(h->Name,h->HisPos);
}

/**[txh]********************************************************************

  Description:
  Jumps to an indicated cross reference. The name can match partially.
Passing jmpXRSubStr in opts jumps only if the name is a substring, normally
jumps just to the name that have more coincident letters at the start (can
be just one). The search isn't case sensitive. Passing the bestMVisibleName
flag the search is done using the visible names of the cross references, no
the node names.

  Return:
  !=0 if a jump was done

***************************************************************************/

int TInfViewer::jumpXRefPartial(char *name, unsigned opts)
{
 int LaPego,cual;
 cual=topic->selBestMatch(name,LaPego,opts);
 if (cual)
   {
    cual--;
    if (opts & jmpXRSubStr)
      {
       char *node=opts & bestMVisibleName ?
                  topic->crossRefs[cual].Name2 :
                  topic->crossRefs[cual].Name;
       if (strncasecmp(name,node,strlen(name)))
          return 0;
      }
    switchToTopic(topic->crossRefs[cual].Name);
    return 1;
   }
 return 0;
}

static void AddFileToNode(char *dest, char *source, char *file)
{
 if (source[0]=='(')
    strncpy(dest,source,MAX_NODE_NAME-1);
 else
   {
    dest[0]='(';dest[1]=0;
    strncat(dest,file,MAX_NODE_NAME-4);
    strcat(dest,")");
    if (strlen(source)+strlen(dest)+1<MAX_NODE_NAME)
       strcat(dest,source);
   }
}

void TInfViewer::AddToHistory(char *context)
{
 stHisto *h = (stHisto *)History->at(HistPoint);

 h->HisPos = cursor + delta;
 h->HisSel=selected;

 h = new stHisto;

 AddFileToNode(h->Name,context,hFile->NameOfFile);

 History->insert(h);
 HistPoint++;

 if (HistPoint>=MAX_HIST_DEEP)
   {
    History->atRemove(0);
    HistPoint--;
   }
}

char *TInfViewer::TakeFromHistory(TPoint& Pos)
{
 stHisto *h;
 if (HistPoint>0)
   {
    History->atRemove(HistPoint--);
    h=(stHisto *)History->at(HistPoint);
   }
 else
    h=(stHisto *)History->at(0);

 selected=h->HisSel;
 Pos = h->HisPos;
 return h->Name;
}

void TInfViewer::OSInsertRoutine(int clip, char *b, long l)
{
 if (!TVOSClipboard::copy(clip,b,l))
   {
    messageBox(mfError | mfOKButton,__("Error copying to clipboard: %s"),
               TVOSClipboard::getError());
    return;
   }
}

void TInfViewer::OSInsertRoutine0(char *b, long l)
{
 OSInsertRoutine(0,b,l);
}

void TInfViewer::OSInsertRoutine1(char *b, long l)
{
 OSInsertRoutine(1,b,l);
}

void TInfViewer::PasteToClipboard(void (*ir)(char *b, long l))
{
 char b[256];
 int deltaY=selRowEnd-selRowStart;

 if (deltaY<0 || (deltaY==0 && selColEnd<=selColStart))
    return;

 char *d=new char[(deltaY+1)*256];
 char *s=d;

 if (d==NULL)
    return;

 int i;
 int l=0,lTotal=0,lLine;
 int Xs=selColStart,Xe=selColEnd;
 for (i=selRowStart; i<=selRowEnd; i++)
    {
     if (i==selRowStart || i==selRowEnd)
       {
        lLine=topic->getLine(i,b);
        if (selRowStart==selRowEnd)
          {
           if (Xs>lLine) return;
           if (Xe>lLine) Xe=lLine;
           lTotal=Xe-Xs;
           memcpy(s,b+Xs,lTotal);
          }
        else
          if (i==selRowStart)
            {
             if (Xs<lLine)
               {
                lTotal=lLine-Xs;
                memcpy(s,b+Xs,lTotal);
                s+=lTotal;
                const char *p=CLY_crlf;
                for (int i=0; i<CLY_LenEOL; i++)
                  {
                   s[0]=p[i];
                   s++;
                   lTotal++;
                  }
               }
            }
          else
            if (i==selRowEnd)
              {
               if (Xe>lLine)
                  Xe=lLine;
               l=Xe;
               memcpy(s,b,l);
               lTotal+=l;
              }
       }
     else
       {
        lLine=topic->getLine(i,s);
        s+=lLine;
        const char *p=CLY_crlf;
        for (int i=0; i<CLY_LenEOL; i++)
          {
           s[0]=p[i];
           s++;
           lTotal++;
          }
        lTotal+=lLine;
       }
    }

 if (lTotal)
    ir(d,lTotal);

 delete d;
}


void TInfViewer::PasteInclude(void)
{
 char *s,*f;
 int lines=topic->iLines,line=0,l,inserted=0;
 char b[256];
 DynStrCatStruct cat;

 DynStrCatInit(&cat,"",0);
 do
   {
    topic->getLine(line+1,b);
    s=strstr(b,"#include");
    if (s)
      {
       for (f=s, l=1; *f && *f!='>'; f++,l++);
       if (inserted)
         {
          DynStrCat(&cat,(char *)CLY_crlf,CLY_LenEOL);
          DynStrCat(&cat,s,l);
         }
       else
          DynStrCatInit(&cat,s,l);
       inserted++;
      }
    line++;
   }
 while (line<lines);
 if (inserted)
   {
    InsertRoutine(cat.str,cat.len);
    ::free(cat.str);
   }
}

void TInfViewer::GoEndOfLine(void)
{
 char linea[256];
 int Xp=0,x=0;

 topic->getLine(delta.y+cursor.y+1,linea);
 while (linea[x])
   {
    if (!ucisspace(linea[x])) Xp=x;
    x++;
   }
 Xp++;
 scrollTo(Xp,delta.y+cursor.y);
}

void TInfViewer::UpdateSelection(int X, int Y)
{
 if (Y>selRowStartPoint || (Y==selRowStartPoint && X>=selColStartPoint))
   {
    selRowEnd=Y;
    selColEnd=X;
    selRowStart=selRowStartPoint;
    selColStart=selColStartPoint;
   }
 else
   {
    selRowStart=Y;
    selColStart=X;
    selRowEnd=selRowStartPoint;
    selColEnd=selColStartPoint;
   }
}

void TInfViewer::MoveToMouse(TPoint m, uchar selMode)
{
 TPoint mouse = makeLocal(m);

 scrollTo(mouse.x+delta.x,mouse.y+delta.y);
 if (selMode & smExtend)
   { // Extends the selection
    UpdateSelection(cursor.x+delta.x,cursor.y+delta.y+1);
   }
 else
   { // Starts a selection
    if (selMode & smDouble)
      { // Jump to the topic
       if (selected > 0)
          switchToTopic(topic->getCrossRef(selected-1));
      }
    else
      {
       selRowStartPoint=cursor.y+delta.y+1;
       selColStartPoint=cursor.x+delta.x;
      }
   }
}

void TInfViewer::unlock()
{
 if (lockCount>0)
   {
    lockCount--;
    if (lockCount==0 && mustBeRedrawed)
       drawView();
   }
}

void TInfViewer::gotoInteractive()
{
 char gotoStr[MAX_NODE_NAME];

 gotoStr[0]=0;
 if (execDialog(createGoToDialog(),gotoStr)!=cmCancel)
    switchToTopic(gotoStr);
}

void TInfViewer::NextWord(int selectMode, int x, int y)
{
 char b[256],*s;
 int ox=x, oy=y;
 topic->getLine(y+1,b);
 s=b+x;
 if (!*s)
   {
    if (y>=limit.y) return;
    y++; x=0;
    topic->getLine(y+1,b);
    s=b;
    vScrollBar->setValue(y);
   }
 if (TVCodePage::isAlpha(*s))
    for (; *s && TVCodePage::isAlpha(*s); s++, x++);
 for (; *s && !TVCodePage::isAlpha(*s); s++, x++);
 hScrollBar->setValue(x);
 if (selectMode && (ox!=x || oy!=y))
   {
    UpdateSelection(x,y+1);
    drawView();
   }
}

void TInfViewer::PrevWord(int selectMode, int x, int y)
{
 char b[256],*s;
 if (x==0)
   {
    if (y==0) return;
    y--;
    x=topic->getLine(y+1,b);
    scrollTo(x,y);
    if (selectMode)
      {
       UpdateSelection(x,y+1);
       drawView();
      }
    return;
   }
 topic->getLine(y+1,b);
 s=b+x;
 s--; x--;
 if (!TVCodePage::isAlpha(*s))
    for (; s!=b && !TVCodePage::isAlpha(*s); s--, x--);
 for (; s!=b && TVCodePage::isAlpha(*s); s--, x--);
 if (s!=b)
   {
    s++;
    x++;
   }
 hScrollBar->setValue(x);
 if (selectMode)
   {
    UpdateSelection(x,y+1);
    drawView();
   }
}

void TInfViewer::handleEvent( TEvent& event )
{
 uchar selectMode = 0;
 int S;
 int key;
 int X,Y;

 if (event.what==evKeyDown)
   {
    switch (event.keyDown.keyCode)
      {
       #define C(a) case kbSh##a: event.keyDown.keyCode=kb##a; break;
       C(Down)
       C(Up)
       C(Left)
       C(Right)
       C(End)
       C(Home)
       C(PgUp)
       C(PgDn)
       C(CtLeft)
       C(CtRight)
       #undef C
      }
   }

 TScroller::handleEvent(event);
 switch (event.what)
     {
     case evCommand:
          switch (event.message.command)
             {
              case cmInfHelp:
                   switchToTopic("(infview)");
                   break;

              case cmcFind:
                   find();
                   break;

              case cmcSearchAgain:
                   makeSearch();
                   break;

              case cmcCopy:
                   if (InsertRoutine)
                      PasteToClipboard(InsertRoutine);
                   break;

              case cmcCopyClipWin:
                   if (TVOSClipboard::isAvailable())
                      PasteToClipboard(OSInsertRoutine0);
                   break;

              case cmInfPasteIn:
                   if (InsertRoutine)
                      PasteInclude();
                   break;

              case cmInfBookM:
                   BookMarksDialog();
                   break;

              case cmInfNodes:
                   ChooseNode();
                   break;

              case cmInfGoto:
                   gotoInteractive();
                   break;

              case cmInfOpen:
                   OpenInfo();
                   break;

              case cmhNext:
                   switchToTopic(topic->Next);
                   break;

              case cmhPrev:
                   switchToTopic(topic->Prev);
                   break;

              case cmhUp:
                   switchToTopic(topic->Up);
                   break;

              case cmhConfigDia:
                   ConfigDialog();
                   break;

              case cmInfDir:
                   switchToTopic("(dir)");
                   break;

              case cmInfTop:
                   switchToTopic("Top");
                   break;

              case cmInfControl:
                   {
                    int Comando =
                           execDialog( new THelpControlDialog(History), NULL );
                    if (Comando>=cmhHistSel)
                      {
                       switchToTopic((stHisto *)History->at(Comando-cmhHistSel));
                      }
                    else
                       switch (Comando)
                         {
                          case cmhNext:
                               switchToTopic(topic->Next);
                               break;
                          case cmhPrev:
                               switchToTopic(topic->Prev);
                               break;
                          case cmhUp:
                               switchToTopic(topic->Up);
                               break;
                          case cmhPrevH:
                               {
                                TPoint Pos;
                                char *contexto=TakeFromHistory(Pos);
                                switchToTopic(contexto,Pos);
                               }
                               break;
                          case cmhNodeList:
                               ChooseNode();
                               break;
                          case cmhBookMarks:
                               BookMarksDialog();
                               break;
                          case cmhConfigDia:
                               ConfigDialog();
                               break;
                          case cmhOpenInfo:
                               OpenInfo();
                               break;
                         }
                   }
                   break;

              case cmInfBack:
                   {
                    TPoint Pos;
                    char *contexto=TakeFromHistory(Pos);
                    switchToTopic(contexto,Pos);
                   }
                   break;

              case cmInfLastLink:
                   if (topic->numRefs)
                      switchToTopic(topic->crossRefs[topic->numRefs-1].Name);
                   break;

              case cmInfLink1:
              case cmInfLink2:
              case cmInfLink3:
              case cmInfLink4:
              case cmInfLink5:
              case cmInfLink6:
              case cmInfLink7:
              case cmInfLink8:
              case cmInfLink9:
                   key=event.message.command-cmInfLink1;
                   if (key<topic->numRefs)
                     {
                      switchToTopic(topic->crossRefs[key].Name);
                      QuickLen=0;
                     }
                   break;

              case cmClose:
              case cmCancel:
                   // When the help is called from a modal window we use a modal
                   // help. In this case we must release the focus with endModal.
                   if (owner->state & sfModal)
                      endModal(event.message.command);
                   else
                   // For the rest the default is applied, it means close() is called.
                      return;
                   break;
                  
              default:
                   return;
             }
          clearEvent(event);
          break;

     case evKeyDown:
         S=event.keyDown.shiftState;

         if (selecting)
           {
            if ((S & kbFlagsShift) != 0)
               selectMode = smExtend;
            else
              {
               selectMode = 0;
               selecting = False;
              }
           }
         else
           {
            if ((S & kbFlagsShift) != 0)
            selectMode = smStartSel;
           }

         key=event.keyDown.keyCode;
         X=delta.x+cursor.x;
         Y=delta.y+cursor.y;
         if (selectMode==smStartSel && (key==kbDown || key==kbUp ||
             key==kbLeft || key==kbRight || key==kbEnd ||
             key==kbHome || key==kbPgUp || key==kbPgDn ||
             key==kbCtLeft || key==kbCtRight) )
           {
            selRowStartPoint=Y+1;
            selColStartPoint=X;
            selecting=True;
           }

         switch (key)
             {
             case kbTab:
                  if (PointInCrossRef(cursor+delta,topic,selected) >= 0)
                     selected++;
                  selected++;
                  if (selected > topic->getNumCrossRefs())
                      selected = 1;
                  if ( topic->getNumCrossRefs() != 0 )
                      makeSelectVisible();
                  QuickLen=0;
                  break;

             case kbShTab:
                  PointInCrossRef(cursor+delta,topic,selected);
                  if (selected == 0)
                      selected = topic->getNumCrossRefs();
                  if ( topic->getNumCrossRefs() != 0 )
                      makeSelectVisible();
                  QuickLen=0;
                  break;

             case kbEnter:
                  if (selected > 0)
                  if (selected <= topic->getNumCrossRefs())
                     switchToTopic(topic->getCrossRef(selected-1));
                  QuickLen=0;
                  break;

             case kbEsc:
                  if (owner->state & sfModal)
                     endModal(cmCancel);
                  else
                    {
                     event.what=evCommand;
                     event.message.command=cmClose;
                     event.message.infoPtr=NULL;
                     putEvent(event);
                    }
                 QuickLen=0;
                 break;

             case kbDown:
             case kbShDown:
                  if (selecting)
                    {
                     UpdateSelection(X,Y+2);
                     drawView();
                    }
                  QuickLen=0;
                  return;

             case kbUp:
             case kbShUp:
                  if (selectMode)
                    {
                     UpdateSelection(X,Y);
                     drawView();
                    }
                  QuickLen=0;
                  return;

             case kbLeft:
             case kbShLeft:
                  if (selectMode)
                    {
                     UpdateSelection(X-1,Y+1);
                     drawView();
                    }
                  QuickLen=0;
                  return;

             case kbRight:
             case kbShRight:
                  if (selectMode)
                    {
                     UpdateSelection(X+1,Y+1);
                     drawView();
                    }
                  QuickLen=0;
                  return;

             case kbCtInsert:
                  if (InsertRoutine)
                     PasteToClipboard(InsertRoutine);
                  break;

             case kbEnd:
             case kbShEnd:
                  GoEndOfLine();
                  X=delta.x+cursor.x;
                  if (selectMode)
                     UpdateSelection(X,Y+1);
                  hScrollBar->setValue(X);
                  QuickLen=0;
                  break;

             case kbHome:
             case kbShHome:
                  if (selectMode)
                    {
                     UpdateSelection(0,Y+1);
                     drawView();
                    }
                  QuickLen=0;
                  return;

             case kbPgUp:
             case kbShPgUp:
                  scrollTo(X,Y-(size.y-1));
                  Y=cursor.y+delta.y;
                  if (selectMode)
                     UpdateSelection(cursor.x+delta.x+1,Y+1);
                  vScrollBar->setValue(Y);
                  QuickLen=0;
                  break;

             case kbPgDn:
             case kbShPgDn:
                  scrollTo(X,Y+size.y-1);
                  Y=cursor.y+delta.y;
                  if (selectMode)
                     UpdateSelection(cursor.x+delta.x+1,Y+1);
                  vScrollBar->setValue(Y);
                  QuickLen=0;
                  break;


             case kbShCtLeft:
             case kbCtLeft:
                  PrevWord(selectMode,X,Y);
                  break;

             case kbShCtRight:
             case kbCtRight:
                  NextWord(selectMode,X,Y);
                  QuickLen=0;
                  break;

             default:
                 int Key=TVCodePage::toUpper(event.keyDown.charScan.charCode);
                 if ((key>=kbA && key<=kbZ) || key==kbSpace || Key=='_')
                   {
                    if (QuickLen<MAX_NODE_NAME)
                      {
                       QuickSearch[QuickLen++]=Key;
                       if (topic->SearchNextWith(QuickSearch,QuickLen,selected,QuickVisPos))
                          makeSelectVisible();
                       else
                          QuickLen--;
                      }
                   }
                 else if (key>=kb1 && key<=kb9)
                   {
                    Key-='1';
                    if (Key<topic->numRefs)
                      {
                       switchToTopic(topic->crossRefs[Key].Name);
                       QuickLen=0;
                      }
                    else
                       return;
                   }
                 else
                   {
                    QuickLen=0;
                    switch (key)
                      {
                       case kbCtL:
                            makeSearch();
                            break;

                       case kbCtS:
                            find();
                            break;

                       case kbCtN:
                            switchToTopic(topic->Next);
                            break;

                       case kbCtP:
                            switchToTopic(topic->Prev);
                            break;

                       case kbCtU:
                            switchToTopic(topic->Up);
                            break;

                       case kbCtD:
                            switchToTopic("(dir)");
                            break;

                       case kbCtT:
                            switchToTopic("Top");
                            break;

                       case kb0:
                            if (topic->numRefs)
                               switchToTopic(topic->crossRefs[topic->numRefs-1].Name);
                            break;

                       case kbCtG:
                            gotoInteractive();
                            break;
                            
                       case kbCtH:
                            switchToTopic("(infview)");
                            break;

                       case kbCtO:
                            ChooseNode();
                            break;

                       case kbCtF:
                            ConfigDialog();
                            break;

                       default:
                            return;
                      }
                   }
             }
         drawView();
         clearEvent(event);
         break;

     case evMouseDown:
          if (event.mouse.doubleClick)
             selectMode|=smDouble;
          do
            {
             lock();
             MoveToMouse(event.mouse.where,selectMode);
             selectMode|=smExtend;
             unlock();
            }
          while (mouseEvent(event,evMouseMove+evMouseAuto));
          if (TVOSClipboard::isAvailable()>1)
             PasteToClipboard(OSInsertRoutine1);
          break;
     }
}

extern char *InfViewGetInfoDir(void);

void TInfViewer::OpenInfo(void)
{
 char file[PATH_MAX];
 int l;

 AsoID *aso=GetFileIDDirBuffer(hID_OpenInfo);
 char *path=0;
 if (aso && aso->dir)
    path=aso->dir;

 if (path && path[0]==0)
   { // If the directory isn't configured
    strcpy(file,InfViewGetInfoDir());
    char *start=file;
    char *end=start;
   
    while (*end && *end!=PATHSEPARATOR) end++;
    *end = 0;
    SetFileIDDirValue(hID_OpenInfo,start);
   }

 strcpy(file,"*.in[fz]*");

 if (GenericFileDialog(__("Open Info"),file,0,hID_OpenInfo)!=cmCancel)
   {
    l=strlen(file);
    memmove(file+1,file,l);
    file[0]='(';
    file[l+1]=')';
    file[l+2]=0;
    switchToTopic(file);
   }
}


void TInfViewer::ConfigDialog(void)
{
 uint32 ops=modeFlags;
 if (execDialog(createConfigDialog(),&ops)!=cmCancel)
    topic->modeFlags=modeFlags=ops;
}

void TInfViewer::ChooseNode(void)
{
 int cant=hFile->index->size;

 if (cant)
   {
    TListBoxRec br;
   
    br.items=hFile->index->coll;
    br.selection=0;
   
    if ( execDialog( createNodeChooseDialog(), &br )!=cmCancel )
       switchToTopic(((TInfIndexCollection *)br.items)->atStr(br.selection));
   }
}

void TInfViewer::BookMarksDialog(void)
{
 char aux[256],*s;
 TListBoxRec br;
 br.items=BookMark;
 br.selection=0;
 int Comando;

 // For some unknown reason we can reach this point without a valid BookMark
 // so we must avoid a crash
 if (!BookMark)
    BookMark=new TNoCaseStringCollection(10,10);
 do
   {
    Comando = execDialog( new TBookMarkDialog(BookMark), &br );
    enableCommand(cmbmGoTo);
    enableCommand(cmbmDeleteFromBookMark);

    switch (Comando)
      {
       case cmbmAddToBookMark:
            aux[0]='(';
            aux[1]=0;
            strcat(aux,hFile->NameOfFile);
            strcat(aux,")");
            strcat(aux,topic->Node);
            s=strdup(aux);
            BookMark->insert(s);
            break;
   
       case cmbmDeleteFromBookMark:
            BookMark->atRemove(br.selection);
            break;
   
       case cmbmGoTo:
            switchToTopic((char *)BookMark->at(br.selection));
            break;
      }
   }
 while(Comando==cmbmDeleteFromBookMark);
}

int StrNCmp(const char *s1, const char *s2, size_t n)
{
  if (n == 0) return 0;
  do
   {
    if (TVCodePage::toUpper(*s1) != *s2++) return 1;
    if (*s1++ == 0)
      break;
   }
  while (--n != 0);
  return 0;
}


char *ScanStr(char *s, char *find, int &Linea, int &Col)
{
  char c, sc;
  size_t len;
  char Buf[256];

  strcpy(Buf,find);
  strupr(Buf);
  find=Buf;

  if ((c = *find++) != 0)
  {
    len = strlen(find);
    do
      {
      do
        {
         if ((sc = *s++) == 0) return 0;
         if (sc=='\n')
           {
            Col=0;
            Linea++;
           }
         else
            Col++;
        }
      while (TVCodePage::toUpper(sc) != c);
      }
    while (StrNCmp(s, find, len) != 0);
    s--;
  }
  Col--;
  return s;
}

char *ScanStrCase(char *s, char *find, int &Linea, int &Col)
{
  char c, sc;
  size_t len;

  if ((c = *find++) != 0)
  {
    len = strlen(find);
    do
      {
      do
        {
	 if ((sc = *s++) == 0) return 0;
         if (sc=='\n')
           {
            Col=0;
            Linea++;
           }
         else
            Col++;
        }
      while (sc != c);
      }
    while (strncmp(s, find, len) != 0);
    s--;
  }
  Col--;
  return s;
}



TDialog *createFindDialog();
TDialog *createInfFindDialog();

int TInfViewer::searchInCurrentTopic(int Linea,int Col,int largo)
{
 TPoint Punto;

 do
  {
   lastSearch=findType & fitCaseSens ?
              ScanStrCase(lastSearch,findStr,Linea,Col) :
              ScanStr(lastSearch,findStr,Linea,Col);
   if (lastSearch!=0) // hit and
     {
      if ( !(findType & fitWWord) ||  // (!Whole word or
         // is a whole word)
         (!TVCodePage::isAlpha(*(lastSearch-1)) && !TVCodePage::isAlpha(*(lastSearch+largo))) )
        {
         LineEndSelect=LineStartSelect=Linea+1;
         ColStartSelect=Col;
         ColEndSelect=Col+largo;
         Punto.x=Col;
         Punto.y=Linea+1;
         TextSelected=True;
         lastSearch+=largo;
         makeVisible(Punto,largo);
         return 1;
        }
      lastSearch+=largo;
      Col+=largo;
     }
  }
 while (lastSearch);

 return 0;
}


int TInfViewer::searchInCurrentFile(int Linea,int Col,int largo)
{
 char Buf[256];
 char findUp[256];
 FILE *f;

 // first in the current topic
 if (TopicInSearch!=fitNotInTopic && searchInCurrentTopic(Linea,Col,largo))
    return 1;
 // now across topics
 long PosTopic;
 strcpy(findUp,findStr);
 strupr(findUp);
 char *s;
 int LastTopic=TopicInSearch;
 Progress_Init(__("Please wait"),__("Searching ..."));
 char *newName=0; int diff;
 while ((PosTopic=hFile->index->position(++TopicInSearch,newName,diff))!=-1)
   {
    Progress_UpDate();
    // Skip anchor references (not really nodes)
    if (newName) { newName=0; continue; }
    hFile->fSeek(PosTopic);
    topic->ReadNodeInfo(*hFile);
    if (topic->Status)
      {
       switchToTopic("Top");
       break;
      }
    f=hFile->stream;
    do
     {
      if (fgets(Buf,256,f)==NULL) break;
      strupr(Buf);
      s=strstr(Buf,findUp);
     }
    while (s==NULL && Buf[0]!=31);

    if (Buf[0]!=31 && !feof(f))
      {
       switchToTopic(hFile->index->coll->atStr(TopicInSearch));

       lastSearch=topic->Text;
       if (searchInCurrentTopic(0,0,largo))
         {
          Progress_DeInit();
          return 1;
         }
      }
   };
 if (LastTopic!=fitNotInTopic)
    switchToTopic(hFile->index->coll->atStr(LastTopic));
 Progress_DeInit();
 return 0;
}

void TInfViewer::makeSearch(int beVerbose)
{
 int Linea,Col;
 int largo,Eureka=0;

 if (!SearchArmed) return;

 largo=strlen(findStr);
 if (largo==0) return;

 // Si finaliz¢ una b£squeda antes y ahora presion¢ Ctrl+L que vuelva al ppio.
 if (lastSearch==NULL)
    lastSearch=topic->Text;

 if (lastSearch!=topic->Text)
   {
    Linea=LineEndSelect-1;
    Col=ColEndSelect;
   }
 else
   {
    Linea=0;
    Col=0;
   }

 OpenVerbose=0;
 switch (findWhere)
  {
   case fitInTopic:
        // Search in the current topic, is only in a large buffer
        Eureka=searchInCurrentTopic(Linea,Col,largo);
        break;

   case fitInFile:
        // It's complex; and calls searchInCurrentTopic
        Eureka=searchInCurrentFile(Linea,Col,largo);
        break;

   case fitInDir:
        // It's Waco, and infected (i kill thousands of bugs here).
        if (FileInSearch==fitNotInTopic)
          { // It's the first time
           // Keep track were start
           AddFileToNode(OriginalPlace.Name,topic->Node,hFile->NameOfFile);
           OriginalPlace.HisPos = cursor + delta;
           OriginalPlace.HisSel=selected;
          }
        else
          { // Not the first time, continue in this file
           if (searchInCurrentFile(Linea,Col,largo))
             {
              OpenVerbose=1;
              return;
             }
          }
        // now across files
        // Go to Top of Top
        switchToTopic("(dir)Top");
        while ((++FileInSearch)<topic->numRefs)
          {
           switchToTopic(topic->crossRefs[FileInSearch].Name);
           if (!hFile->Status)
             {
              TopicInSearch=fitNotInTopic;
              if (searchInCurrentFile(Linea,Col,largo))
                {
                 OpenVerbose=1;
                 return;
                }
             }
           switchToTopic("(dir)Top");
          }
        switchToTopic(&OriginalPlace);
        break;
  }
 if (!Eureka && beVerbose)
   {
    messageBox(__("Sorry can't find the string"),mfError | mfOKButton);
    SearchArmed=0;
   }
 OpenVerbose=1;
}


void TInfViewer::find()
{
 TFindInfDialogRec findRec( findStr, findType, findWhere );
 lastSearch=topic->Text;
 FileInSearch=TopicInSearch=fitNotInTopic;
 if ( execDialog( createInfFindDialog(), &findRec ) != cmCancel )
   {
    strcpy( findStr, findRec.find );
    findType = findRec.options;
    findWhere =  findRec.ops2;
    SearchArmed=1;
    makeSearch();
   }
}

void TInfViewer::findInTopic(char *s)
{
 strncpyZ(findStr,s,MAX_NODE_NAME);
 findType=findWhere=0;
 SearchArmed=1;
 makeSearch(0);
}

/****** Stream functions for TInfViewer ******/

void TInfViewer::write( opstream& os )
{
 TScroller::write( os );
 os.writeString(hFile->NameOfFile);
 os.writeString(topic->Node);
 os << modeFlags;
 // Find variables
 os << findType << findWhere;
 os.writeString(findStr);
 // History
 os << HistPoint << History;
 // Selection, I'm just crazy, no?
 os << selRowStart << selRowEnd << selColStart << -0x022 << selColEnd;
}

void *TInfViewer::read( ipstream& is )
{
 // Base class
 TScroller::read( is );

 // Read stuff for this class
 char File[MAX_NODE_NAME];
 char Node[MAX_NODE_NAME];

 is.readString(File,MAX_NODE_NAME);
 // As I changed the name for the editor's help here I provide a mechanism to
 // convert the name at load time. It will save old users to get messages about
 // 'editor' not found
 if (TranslateName==1 && strncasecmp(File,"editor",6)==0)
    strcpy(File,"setedit");

 is.readString(Node,MAX_NODE_NAME);
 is >> modeFlags;
 // Find variables
 is >> findType >> findWhere;
 is.readString(findStr,MAX_NODE_NAME);
 // History
 is >> HistPoint >> History;
 // Selection, I'm just crazy, no?
 is >> selRowStart >> selRowEnd >> selColStart >> selColEnd;

 if (selColEnd<0)
   { // Sorry for this crap, I forgot to save the version
    TNoCaseStringCollection *p;
    version=-selColEnd;
    is >> selColEnd;
    if (version<0x022)
      {
       is >> p;
       if (!BookMark)
          BookMark=p;
       else
          destroy0(p);
      }
   }
 else
   {
    version=0;
    if (!BookMark)
       BookMark=new TNoCaseStringCollection(10,10);
   }

 // Initialize all
 lockCount=0;
 mustBeRedrawed=False;
 options |= ofSelectable;
 growMode = gfGrowHiX | gfGrowHiY;
 QuickLen=0;
 TextSelected = False;
 SearchArmed=0;
 lastSearch=0;
 OpenVerbose=1;
 selecting=False;

 hFile=new TInfFile(File);
 int suggY;
 topic=hFile->getTopic(Node,1,modeFlags,suggY);
 topic->setWidth(size.x);
 setLimit(topic->maxWidth() + size.x, topic->numLines() + size.y);

 state |= sfCursorVis;
 return this;
}

TNoCaseStringCollection *TInfViewer::BookMark=NULL;
int TInfViewer::version=0x022;
TCommandSet *TInfViewer::ts=0;
int TInfViewer::TranslateName=0;

TStreamable *TInfViewer::build()
{
 InitCommandSet();
 return new TInfViewer( streamableInit );
}

TInfViewer::TInfViewer( StreamableInit ) : TScroller( streamableInit )
{
}

void (*TInfViewer::InsertRoutine)(char *b, long l)=NULL;


static TRect defaultSizeWindow;
static
TRect &getDefaultSizeWindow()
{
 defaultSizeWindow.a.x=0;
 defaultSizeWindow.a.y=0;
 defaultSizeWindow.b.x=GetDeskTopCols()-4;
 defaultSizeWindow.b.y=GetDeskTopRows()-4;
 return defaultSizeWindow;
}

/***************************************************************

 class TInfWindow

 Just like THelpWindow.

***************************************************************/

TInfWindow::TInfWindow(TInfFile *hFile, char *context, char *match,
                       void (*ir)(char *b, long l), Boolean IsTheOne) :
       TWindowInit( &TInfWindow::initFrame),
       TWindow( getDefaultSizeWindow(), "InfView", wnNoNumber )
{
 TRect r=getDefaultSizeWindow();
 options = (options | ofCentered);
 r.grow(-1,-1);

 viewer=new TInfViewer (r,standardScrollBar(sbHorizontal | sbHandleKeyboard),
   standardScrollBar(sbVertical | sbHandleKeyboard), hFile, context,
   match);
 viewer->InsertRoutine=ir;
 insert(viewer);
 helpCtx = hcInfView;
 isTheOne=IsTheOne;
}

TInfWindow::~TInfWindow()
{
 // Only the main InfView can do it or the rest will lose the BookMark
 if (isTheOne)
   {
    destroy0(TInfViewer::BookMark); // Avoid releasing it twice
    if (TInfViewer::ts)
      {
       delete TInfViewer::ts;
       TInfViewer::ts=0;
      }
   }
}

TPalette& TInfWindow::getPalette() const
{
 static TPalette palette(cInfWindow, sizeof( cInfWindow)-1);
 return palette;
}

void TInfWindow::close()
{
 TInfViewer::DisableAllCommands();
 if (isTheOne)
    hide();
 else
    TWindow::close();
}

/****** Stream functions for TInfWindow ******/

void TInfWindow::write( opstream& os )
{
 TWindow::write( os );
 os << viewer << (int)isTheOne;
 if (isTheOne)
    os << viewer->BookMark;
}

void *TInfWindow::read( ipstream& is )
{
 int aux;

 TWindow::read( is );

 is >> viewer >> aux;
 isTheOne=aux ? True : False;
 if (isTheOne && viewer->version>=0x022)
   {
    destroy0(viewer->BookMark);
    is >> viewer->BookMark;
   }
 options&=~(ofCenterX | ofCenterY);

 return this;
}

TStreamable *TInfWindow::build()
{
 return new TInfWindow( streamableInit );
}

TInfWindow::TInfWindow( StreamableInit ) :
    TWindowInit( NULL ),
    TWindow( streamableInit )
{
}

/***************************************************************

 class TStrListBox

 Una ListBox que responde a Enter y Doble click.
 A List Box with Enter+Double click response.

 La respuesta es un mensaje cmStrChoose y en infoPtr el ¡ndice
seleccionado.

***************************************************************/

void TStrListBox::handleEvent(TEvent& event)
{
 short a=focused;
 if ((event.what==evKeyDown && event.keyDown.keyCode==kbEnter) ||
     (event.what==evMouseDown && event.mouse.doubleClick))
   {
    message( owner, evBroadcast, cmStrChoose, (void *)&focused );
    clearEvent(event);
    return;
   }
 TListBox::handleEvent(event);
 if (focused!=a)
    draw();
}

/***************************************************************

 class THistCollection members

***************************************************************/

void *THisCollection::readItem( ipstream& is )
{
 stHisto *h=new stHisto;
 is.readBytes( h, sizeof(stHisto) );
 return (void *)h;
}

void THisCollection::writeItem( void *obj, opstream& os )
{
 os.writeBytes( (const char *)obj, sizeof(stHisto) );
}

/***************************************************************

 class THelpControlDialog

 Una caja de di logo para controlar el help.
 A Dialog to control the help.

***************************************************************/

void THelpControlDialog::handleEvent(TEvent& event)
{
 TDialog::handleEvent(event);
 if( event.what == evCommand || event.what == evBroadcast)
  {
   //ushort comm=event.message.command;
   switch( event.message.command )
           {
            // a button
            case cmhNext:
            case cmhPrev:
            case cmhUp:
            case cmhPrevH:
            case cmhHide:
            case cmhNodeList:
            case cmhBookMarks:
            case cmhConfigDia:
            case cmhOpenInfo:
                {
                endModal(event.message.command);
                clearEvent(event);
                }
                break;

            // from the list
            case cmStrChoose:
                {
                 endModal(cmhHistSel+*((ushort *)event.message.infoPtr));
                 clearEvent(event);
                }
                break;
            default:
                break;
           }
  }
}


THelpControlDialog::THelpControlDialog(THisCollection *oCol) :
    TWindowInit( &THelpControlDialog::initFrame ),
    TDialog( TRect( 1, 1, 56, 20 ), __("Help Control") )
{
 TRect r(1,2,18,4);
 insert( new TButton(r,__("~N~ext"),cmhNext,bfDefault) );
 r.a.y+=2;
 r.b.y+=2;
 insert( new TButton(r,__("~P~revious"),cmhPrev,bfNormal) );
 r.a.y+=2;
 r.b.y+=2;
 insert( new TButton(r,__("~U~p"),cmhUp,bfNormal) );
 r.a.y+=2;
 r.b.y+=2;
 insert( new TButton(r,__("Prev. ~H~ist."),cmhPrevH,bfNormal) );
 r.a.y+=2;
 r.b.y+=2;
 insert( new TButton(r,__("Nodes ~L~ist"),cmhNodeList,bfNormal) );
 r.a.y+=2;
 r.b.y+=2;
 insert( new TButton(r,__("~B~ookmarks"),cmhBookMarks,bfNormal) );
 r.a.y+=2;
 r.b.y+=2;
 insert( new TButton(r,__("~C~onfiguration"),cmhConfigDia,bfNormal) );
 r.a.y+=2;
 r.b.y+=2;
 insert( new TButton(r,__("~O~pen Info"),cmhOpenInfo,bfNormal) );
 r.a.x+=19;
 r.b.x+=19;
 insert( new TButton(r,__("H~i~de"),cmhHide,bfNormal) );

 TScrollBar *sb = new TScrollBar( TRect(52,3,53,15) );
 insert(sb);
 TStrListBox *ListaH = new TStrListBox( TRect(21,3,52,15),(ushort)1, sb );
 insert(ListaH);
 ListaH->newList(oCol);
 insert (new TLabel(TRect(21,2,52,3),__("Hi~s~tory"), ListaH));

 options|=ofTopSelect;
 selectNext( False );
 helpCtx=hcInfControl;
}


TDialog *createInfFindDialog()
{
 TDialog *d = new TDialog( TRect( 0, 0, 38, 15 ), __("Inf Find") );

 d->options |= ofCentered;

 TInputLine *control = new TInputLine( TRect( 3, 3, 32, 4 ), MAX_NODE_NAME );
 d->insert( control );
 d->insert(
     new TLabel( TRect( 2, 2, 30, 3 ), __("~T~ext to find"), control ) );
 d->insert(
     new THistory( TRect( 32, 3, 35, 4 ), control, hID_TextSearchInf ) );

 d->insert( new TCheckBoxes32( TRect( 3, 5, 35, 7 ),
     new TSItem( __("~C~ase sensitive"),
     new TSItem( __("~W~hole words only"), 0 ))));

 d->insert( new TRadioButtons32( TRect( 3, 8, 35, 11 ),
     new TSItem( __("In current ~t~opic"),
     new TSItem( __("In the ~f~ile"),
     new TSItem( __("In ~a~ll files"), 0 )))));

 d->insert(
     new TButton( TRect( 14, 12, 24, 14 ), __("~O~K"), cmOK, bfDefault ) );
 d->insert(
     new TButton( TRect( 26, 12, 36, 14 ), __("Cancel"), cmCancel, bfNormal ) );

 d->selectNext( False );
 d->helpCtx=hcInfSearch;
 return d;
}

TDialog *createGoToDialog()
{
 TDialog *d = new TDialog( TRect( 0, 0, 38, 9 ), __("Go To") );

 d->options |= ofCentered;

 TInputLine *control = new TInputLine( TRect( 3, 3, 32, 4 ), MAX_NODE_NAME );
 d->insert( control );
 d->insert(
     new TLabel( TRect( 2, 2, 30, 3 ), __("~N~ame of Node"), control ) );
 d->insert(
     new THistory( TRect( 32, 3, 35, 4 ), control, hID_GotoInf ) );

 d->insert(
     new TButton( TRect( 14, 6, 24, 8 ), __("~O~K"), cmOK, bfDefault ) );
 d->insert(
     new TButton( TRect( 26, 6, 36, 8 ), __("Cancel"), cmCancel, bfNormal ) );

 d->selectNext( False );
 d->helpCtx=hcInfGoto;
 return d;
}


TDialog *createNodeChooseDialog()
{
 TRect r( 0, 0, 60, 20 );
 // Make it growable
 TGrowDialog *d = new TGrowDialog( r, __("Tag Table"), ofCentered );

 TScrollBar *sb = new TScrollBar( TRect(r.b.x-3,2,r.b.x-2,r.b.y-4) );
 d->insert(sb);
 TSOSSortedListBox *ListaN = new TSOSSortedListBox( TRect(2,2,r.b.x-3,r.b.y-4),(ushort)1, sb );
 ListaN->growMode = gfMoveBottomCorner;
 d->insert(ListaN);
 d->insert(new TLabel(TRect(2,1,r.b.x-3,2),__("~N~odes"), ListaN));

 TButton *tb1=new TButton( TRect( 2, r.b.y-3, 16, r.b.y-1 ), __("~O~K"), cmOK, bfDefault );
 tb1->growMode = gfMoveAccording;
 d->insert(tb1);

 TButton *tb2=new TButton( TRect( 20, r.b.y-3, 34, r.b.y-1 ), __("Cancel"), cmCancel, bfNormal );
 tb2->growMode = gfMoveAccording;
 d->insert(tb2);

 d->selectNext( False );
 d->helpCtx=hcInfChoose;

 return d;
}

TDialog *createConfigDialog()
{
 TRect r( 0, 0, 36, 8);

 TDialog *d = new TDialog( r, __("InfView Configuration") );

 d->options |= ofCentered;

 d->insert( new TCheckBoxes32( TRect( 3, 2, 33, 4 ),
     new TSItem( __("Hide '*~N~ote' word"),
     new TSItem( __("Hide real ~l~ink name"), 0 ))));
 d->insert(
     new TButton( TRect( 2, 5, 14, 7 ), __("~O~K"), cmOK, bfDefault ) );
 d->insert(
     new TButton( TRect( 16, 5, 28, 7 ), __("Cancel"), cmCancel, bfNormal ) );

 d->selectNext( False );
 d->helpCtx=hcInfConfig;

 return d;
}

/***************************************************************

 class TBookMarkDialog

 Una caja de di logo para controlar los book marks.
 A Dialog to control the book marks.

***************************************************************/

void TBookMarkDialog::handleEvent(TEvent& event)
{
 TDialog::handleEvent(event);
 if ( event.what == evCommand || event.what == evBroadcast)
   {
    switch ( event.message.command )
      {
       // a button
       case cmbmAddToBookMark:
       case cmbmDeleteFromBookMark:
       case cmbmGoTo:
            endModal(event.message.command);
            clearEvent(event);
            break;
       default:
           break;
      }
   }
}

TBookMarkDialog::TBookMarkDialog(TNoCaseStringCollection *oCol) :
    TWindowInit( &TBookMarkDialog::initFrame ),
    TDialog( TRect( 0, 0, 60, 15 ), __("InfView Bookmarks") )
{
 int Cant=oCol->getCount();

 TRect r(0,0,60,15);
 options|=ofCentered | ofTopSelect;

 TRect bt(1,2,13,4);
 TButton *tb=new TButton( bt, __("~A~dd"),cmbmAddToBookMark,bfNormal);
 insert(tb);

 bt.move(0,2);
 tb=new TButton( bt, __("~D~elete"),cmbmDeleteFromBookMark,bfNormal);
 insert(tb);

 bt.move(0,2);
 insert( new TButton( bt, __("Cancel"),cmCancel,bfNormal) );

 bt.move(0,2);
 tb=new TButton( bt, __("~G~o to"),cmbmGoTo,bfDefault);
 insert(tb);
 if (!Cant)
   {
    disableCommand(cmbmDeleteFromBookMark);
    disableCommand(cmbmGoTo);
   }

 TScrollBar *sb = new TScrollBar( TRect(r.b.x-3,2,r.b.x-2,r.b.y-2) );
 insert(sb);
 TSortedListBox *Lista=new TSortedListBox( TRect(bt.b.x+1,2,r.b.x-3,r.b.y-2),(ushort)1, sb );
 insert(Lista);
 insert(new TLabel(TRect(bt.b.x+1,1,r.b.x-3,2),__("~M~arks"), Lista));
 //Lista->newList(oCol);

 selectNext( False );
 helpCtx=hcInfBookMark;
}



