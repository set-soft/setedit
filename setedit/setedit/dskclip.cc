/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#include <stdio.h>

#define Uses_TApplication
#define Uses_TStreamable
#define Uses_TWindow
#define Uses_TCEditWindow
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TVCodePage
#define Uses_TScreen
#include <ceditor.h>
#define Uses_SETAppVarious
#define Uses_TSetEditorApp
#include <setapp.h>
#include <dskwin.h>
#include <dskclip.h>

TStreamable *TDskWinClipboard::build()
{
 return new TDskWinClipboard( streamableInit );
}

void TDskWinClipboard::write( opstream& os )
{
 os << edw->origin << edw->size << (int)(edw->state & sfVisible)
    << (int)(TProgram::deskTop->indexOf(edw));
}

void *TDskWinClipboard::read( ipstream& is )
{
 TPoint aux;

 // Create a default clipboard
 TRect r = TProgram::deskTop->getExtent();
 TCEditWindow *p;

 p = (TCEditWindow *)editorApp->validView( new TCEditWindow( r, 0, wnNoNumber ) );
 p->hide();

 edw=TSetEditorApp::clipWindow=p;
 if (TSetEditorApp::clipWindow)
   {
    TCEditor::clipboard=TSetEditorApp::clipWindow->editor;
    TCEditor::clipboard->canUndo=False;
   }

 // Now restore the last settings
 is >> aux;
 TSetEditorApp::clipWindow->moveTo(aux.x,aux.y);
 is >> aux;
 TSetEditorApp::clipWindow->growTo(aux.x,aux.y);
 int vis;
 is >> vis;
 if (vis)
    edw->show();
 is >> ZOrder;
 view=edw;

 return this;
}

char *TDskWinClipboard::GetText(char *dest, short maxLen)
{
 TVIntl::snprintf(dest,maxLen,__("   Clipboard (%d bytes)"),edw->editor->bufLen);
 return dest;
}

TDskWinClipboard::TDskWinClipboard(TCEditWindow *EdW)
{
 view=edw=EdW;
 type=dktClipboard;
 CanBeSaved=1;
 CanBeDeletedFromDisk=0;
}

int TDskWinClipboard::DeleteAction(ccIndex, Boolean)
{
 edw->hide();

 return 0;
}

TDskWinClipboard::~TDskWinClipboard()
{
 //destroy(edw);
 edw->close();
 if (TSetEditorApp::clipWindow==edw)
    TSetEditorApp::clipWindow=0;
 edw=0;
}

