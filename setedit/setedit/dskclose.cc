/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#include <stdio.h>
#define Uses_string
#define Uses_unistd

#define Uses_TApplication
#define Uses_TStreamable
#define Uses_TWindow
#define Uses_TCEditWindow
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TListBox
#define Uses_TVCodePage
#define Uses_TScreen
#include <ceditor.h>
#define Uses_SETAppVarious
#define Uses_TSetEditorApp
#include <setapp.h>
#include <dskwin.h>
#include <dskclose.h>
#include <edcollec.h>

TStreamable *TDskWinClosed::build()
{
 return new TDskWinClosed( streamableInit );
}

void TDskWinClosed::write( opstream& os )
{
 os.writeString(Name);
 TCEditWindow::SaveResume(resume,os);
}

void *TDskWinClosed::read( ipstream& is )
{
 char fileName[PATH_MAX];
 is.readString(fileName,PATH_MAX);
 Name=newStr(fileName);
 if (editorApp->deskTopVersion>=0x306)
    TCEditWindow::ReadResume(resume,is);
 else
   {
    TPoint origin,size,cursor;
    is >> origin >> size >> cursor;
    TCEditWindow::FillResumeWith(resume,origin,size,cursor);
   }
 ZOrder=-1;
 view=0;

 return this;
}

char *TDskWinClosed::GetText(char *dest, short maxLen)
{
 TVIntl::snprintf(dest,maxLen,__("   Closed: "));
 return TDskWin::GetText(dest,Name,maxLen);
}

TDskWinClosed::TDskWinClosed(TCEditWindow *edw)
{
 Name=newStr(edw->editor->fileName);
 edw->FillResume(resume);
 type=dktClosed;
 CanBeSaved=1;
 CanBeDeletedFromDisk=1;
 view=0;
}

TDskWinClosed::~TDskWinClosed()
{
 delete[] Name;
}

int TDskWinClosed::GoAction(ccIndex i)
{
 TCEditWindow *ed;

 // Remove from the closed list
 TSetEditorApp::edHelper->atRemove(i);
 TSetEditorApp::edHelper->Closed--;
 // Lock the desktop to avoid lotz redraws
 TProgram::deskTop->lock();
 // Reopen the file
 ed=editorApp->openEditor(Name,True,&resume);
 // Indicate the new Top editor
 setFocusTo=(TView *)ed;
 focusChanged=True;

 return 1;
}

int TDskWinClosed::DeleteAction(ccIndex i, Boolean fromDiskToo)
{
 if (fromDiskToo)
    unlink(Name);
 TSetEditorApp::edHelper->atRemove(i);
 TSetEditorApp::edHelper->Closed--;

 return 1;
}

