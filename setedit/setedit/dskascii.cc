/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#include <stdio.h>
#define Uses_TView
#define Uses_TPoint
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TWindow
#include <ceditor.h>
#include <dskwin.h>
#include <dskascii.h>

#define Uses_SETAppVarious
#include <setapp.h>

#include <ascii.h>

static TDskWinASCII *helper;

TStreamable *TDskWinASCII::build()
{
 return new TDskWinASCII(streamableInit);
}

TDskWinASCII::TDskWinASCII()
{
 view=new TAsciiChart();
 TProgram::deskTop->insert(view);
 type=dktASCII;
 CanBeDeletedFromDisk=0;
 CanBeSaved=1;
 helper=this;
}

char *TDskWinASCII::GetText(char *dest, short maxLen)
{
 TVIntl::snprintf(dest,maxLen,__("   ASCII chart"));
 return dest;
}

TDskWinASCII::~TDskWinASCII()
{
 helper=0;
}

void TDskWinASCII::write(opstream& os)
{
 os << (uchar)1 << view->origin << (int)(TProgram::deskTop->indexOf(view));
}

void *TDskWinASCII::read(ipstream& is)
{
 uchar version;
 TPoint pos;
 is >> version >> pos >> ZOrder;
 view=new TAsciiChart();
 view->moveTo(pos.x,pos.y);
 helper=this;
 return this;
}

void ASCIIWindow()
{
 if (helper)
    helper->view->select();
 else
    AddNonEditorToHelper(new TDskWinASCII());
}
