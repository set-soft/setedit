/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#include <stdio.h>
#include <stdlib.h>
#define Uses_string
#define Uses_unistd
class TRect;
#define Uses_TDialog
#define Uses_TPoint
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TScroller
#define Uses_TNSCollection
#define Uses_MsgBox

#include <tv.h>

#include <dskwin.h>
#include <dskman.h>

#define Uses_TManWindow
#define Uses_TManPageView
#define Uses_TEnhancedText
#include <manview.h>

#define Uses_SETAppDialogs
#define Uses_SETAppVarious
#include <setapp.h>

#include <rhutils.h>

extern TView *setFocusTo;
extern Boolean focusChanged;

TStreamable *TDskWinMan::build()
{
 return new TDskWinMan(streamableInit);
}

void TDskWinMan::write(opstream& os)
{
 TManWindow *mw=(TManWindow *)view;
 os << mw << (int)(TProgram::deskTop->indexOf(mw));
}

void *TDskWinMan::read(ipstream& is)
{
 TManWindow *mw;
 is >> mw >> ZOrder;
 view=mw;

 return this;
}

char *TDskWinMan::GetText(char *dest, short maxLen)
{
 TManWindow *mw=(TManWindow *)view;
 TVIntl::snprintf(dest,maxLen,__("   Man page: %s"),mw->getFileName());
 return dest;
}

TDskWinMan::TDskWinMan(const char *file, const char *sections, const char *extraOps)
{
 type=dktMan;
 CanBeSaved=1;
 CanBeDeletedFromDisk=0;
 view=CreateManWindow(file,sections,extraOps,CopyHelp2Clip);
}

TDskWinMan::~TDskWinMan()
{
}

TDskWinMan *ManPageView(const char *name)
{
 ManPageOptions *op;
 TDialog *d=ManPageViewSelect(name,&op);
 TDskWinMan *ret=0;
 if (execDialog(d,op)==cmOK)
   {
    ret=new TDskWinMan(op->program,op->section,op->options);
   }
 return ret;
}

