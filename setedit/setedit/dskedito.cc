/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#include <stdio.h>
#define Uses_unistd

#define Uses_TApplication
#define Uses_TStreamable
#define Uses_TWindow
#define Uses_TCEditWindow
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TListBox
#include <ceditor.h>
#define Uses_SETAppVarious
#define Uses_TSetEditorApp
#include <setapp.h>
#include <dskwin.h>
#include <dskedito.h>
#include <edcollec.h>

extern TEditorCollection *edHelper;
void RemoveFromEditorsHelper(TCEditWindow *p);

TStreamable *TDskWinEditor::build()
{
 return new TDskWinEditor( streamableInit );
}

void TDskWinEditor::write( opstream& os )
{
 os << number << edw << (int)(TProgram::deskTop->indexOf(edw));
}

void *TDskWinEditor::read( ipstream& is )
{
 is >> number >> edw >> ZOrder;
 edw=(TCEditWindow *)editorApp->validView(edw);
 if (edw)
   {
    edw->number=number;
    edw->show();
   }
 // If the file is empty don't put it in the desktop and let the TEditorCollection
 // close it.
 view=(edw->editor->FailedToLoad) ? 0 : edw;

 return this;
}

char *TDskWinEditor::GetText(char *dest, short maxLen)
{
 TCEditor *ed=edw->editor;
 sprintf(dest,"%2d%c ",number,ed->isReadOnly ? 'R' : (ed->modified ? '*' : ' '));
 return TDskWin::GetText(dest,ed->fileName,maxLen);
}


int TDskWinEditor::DeleteAction(ccIndex , Boolean fromDiskToo)
{
 // It calls to close, close removes the object from the editor helper
 // and calls to destroy
 if (fromDiskToo)
   {
    unlink(edw->editor->fileName);
    // We will be autodestroyed soon, for this reason we can't use the
    // this pointer anymore. That's why we need a copy (efence reported).
    TCEditWindow *oldEdw=edw;
    edHelper->removeEditor(edw,True);
    destroy(oldEdw);
   }
 else
    edw->close();

 return 0;
}

TDskWinEditor::~TDskWinEditor()
{
 /*destroy(edw);
 edw=0;*/
}

