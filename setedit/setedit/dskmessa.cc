/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#include <stdio.h>
class TRect;
class TSOSListBox;
#define Uses_TDialog
#define Uses_TPoint
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TApplication
#define Uses_TVCodePage
#define Uses_TScreen
#include <ceditor.h>
#include <edmsg.h>
#include <dskwin.h>
#include <dskmessa.h>
#define Uses_TSetEditorApp
#include <setapp.h>

extern TView *setFocusTo;
extern Boolean focusChanged;

TStreamable *TDskWinMessage::build()
{
 return new TDskWinMessage( streamableInit );
}

void TDskWinMessage::write( opstream& os )
{
 TEdMsgDialog *edw=(TEdMsgDialog *)view;
 os << edw->origin << edw->size << (int)(edw->state & sfVisible)
    << edw->zoomRect
    << (int)(TProgram::deskTop->indexOf(edw));
}

void *TDskWinMessage::read( ipstream& is )
{
 TPoint aux;

 TEdMsgDialog *edw=EdMessageWindowInit(0);

 // Now restore the last settings
 is >> aux;
 TRect r=TProgram::deskTop->getExtent();
 // Don't let it outside the screen
 if (TSetEditorApp::geFlags & geVertWindows)
   {
    if (TSetEditorApp::geFlags & geRightSide)
      {
       if (aux.x>=r.b.x)
          aux.x=r.b.x-TSetEditorApp::widthVertWindows;
      }
    else
      {
       if (aux.x>=r.b.x)
          aux.x=0;
      }
    if (aux.y>=r.b.y)
       aux.y=0;
   }
 else
   {
    if (aux.x>=r.b.x)
       aux.x=0;
    if (aux.y>=r.b.y)
       aux.y=r.b.y-7;
   }
 edw->moveTo(aux.x,aux.y);
 is >> aux;
 edw->growTo(aux.x,aux.y);
 int vis;
 is >> vis;
/* if (vis)
    edw->show();
 else // It doesn't have any sense, if the window is visible and without messages...*/
    edw->hide();
 is >> edw->zoomRect >> ZOrder;
 view=edw;

 return this;
}

char *TDskWinMessage::GetText(char *dest, short maxLen)
{
 TVIntl::snprintf(dest,maxLen,__("   Message Window (%d lines)"),EdMessageCantMessages());
 return dest;
}

TDskWinMessage::TDskWinMessage(TEdMsgDialog *w)
{
 view=w;
 type=dktMessage;
 CanBeSaved=1;
 CanBeDeletedFromDisk=0;
}

int TDskWinMessage::DeleteAction(ccIndex, Boolean)
{
 view->hide();
 return 0;
}

TDskWinMessage::~TDskWinMessage()
{
}

