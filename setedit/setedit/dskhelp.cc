/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#define Uses_string
#define Uses_TApplication
#define Uses_TStreamable
#define Uses_TWindow
#define Uses_TCEditWindow
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TVCodePage
#define Uses_TScreen
// InfView requests
#include <infr.h>
#include <ceditor.h>
#define Uses_SETAppVarious
#define Uses_TSetEditorApp
#include <setapp.h>
#include <dskwin.h>
#include <dskhelp.h>
#include <stdio.h>
#include <edcollec.h>

inline void SetInfManager(TDskWinHelp *p)
{
 editorApp->InfManager=p;
}

inline TDskWinHelp *GetInfManager(void)
{
 return editorApp->InfManager;
}

TStreamable *TDskWinHelp::build()
{
 return new TDskWinHelp( streamableInit );
}

void TDskWinHelp::Goto(char *File, char *Node, char *word)
{
 char aux[MAX_NODE_NAME*2];
 aux[0]='(';
 aux[1]=0;
 strcat(aux,File);
 strcat(aux,")");
 strcat(aux,Node);
 window->viewer->switchToTopic(aux);
 if (word)
    window->viewer->findInTopic(word);
}

void TDskWinHelp::write( opstream& os )
{
 os << window << (int)(TProgram::deskTop->indexOf(window));
}

void *TDskWinHelp::read( ipstream& is )
{
 // In v0.4.25 I changed the name of the editor's help from editor to setedit. That's
 // needed to avoid problems under Linux (future ones, not currently). As a draw back
 // if you load an old desktop file it will scream about 'editor' help file not found
 // to avoid it I convert the name for old desktop files.
 if (editorApp->deskTopVersion<0x425)
    TInfWindow::SetMagicEditorInfoTranslation();

 if (editorApp->deskTopVersion<=0x215)
   {
    char File[MAX_NODE_NAME];
    char Node[MAX_NODE_NAME];
   
    is.readString(File,MAX_NODE_NAME);
    is.readString(Node,MAX_NODE_NAME);
   
    Create(File,Node);
   
    TPoint aux;
   
    is >> aux;
    window->moveTo(aux.x,aux.y);
    is >> aux;
    window->growTo(aux.x,aux.y);
    int vis;
    is >> vis;
    if (vis)
       window->show();
   
    is >> ZOrder;
   
    int aux2;
    is >> aux2;
    window->isTheOne=aux2 ? True : False;
    window->options&=~(ofCenterX | ofCenterY);
   
    if (aux2)
       SetInfManager(this);
   }
 else
   {
    is >> window >> ZOrder;
   
    // Set up the alias pointers
    file=window->viewer->hFile;
    view=window;
   
    // Set-Up the paste
    window->viewer->InsertRoutine=CopyHelp2Clip;
   
    if (window->isTheOne)
       SetInfManager(this);
   }

 if (editorApp->deskTopVersion<0x425)
    TInfWindow::ResetMagicEditorInfoTranslation();
 return this;
}

char *TDskWinHelp::GetText(char *dest, short maxLen)
{
 if (window->isTheOne)
    TVIntl::snprintf(dest,maxLen,__("   Main InfView (%s)%s"),file->NameOfFile,
                     window->viewer->topic->Node);
 else
    TVIntl::snprintf(dest,maxLen,__("   InfView (%s)%s"),file->NameOfFile,
                     window->viewer->topic->Node);
 return dest;
}

void TDskWinHelp::Create(char *File, char *Node, Boolean TheOne)
{
 file = new TInfFile(File);
 window = new TInfWindow(file,Node,NULL,CopyHelp2Clip,TheOne);
 if ((window=(TInfWindow *)editorApp->validView(window))!=0)
    window->hide();
 view=window;
}

void TDskWinHelp::CreateModal(char *File, char *Node)
{
 TInfFile *aFile = new TInfFile(File);
 TInfWindow *aWindow = new TInfWindow(aFile,Node,NULL,CopyHelp2Clip,False);
 if ((aWindow=(TInfWindow *)editorApp->validView(aWindow))!=0)
    TProgram::deskTop->execView(aWindow);
 CLY_destroy(aWindow);
}

TDskWinHelp::TDskWinHelp(char *File, char *Node, Boolean TheOne)
{
 Create(File,Node,TheOne);
 type=dktHelp;
 CanBeSaved=1;
 CanBeDeletedFromDisk=0;
}

TDskWinHelp::~TDskWinHelp()
{
 //if (file)
 //   delete file; Wrong! that's part of the view too I realized under Linux
 if (GetInfManager()==this)
   {
    SetInfManager(0);
    // The main InfView is never closed, just hided, now is time to kill it
    //destroy(window);
    window->close();
   }
 file=0;
 window=0;
}

int TDskWinHelp::DeleteAction(ccIndex, Boolean)
{
 if (window->isTheOne)
    window->hide();
 else
    closeView(window,NULL);

 return 0;
}

void TDskWinHelp::MakeVisible(void)
{
 window->select();
 window->show();
}


