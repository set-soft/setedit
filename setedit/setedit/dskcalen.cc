/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#include <stdio.h>
#define Uses_string
#define Uses_TView
#define Uses_TPoint
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TWindow
#define Uses_AllocLocal
#include <ceditor.h>
#include <dskwin.h>
#include <dskcalen.h>

#define Uses_SETAppVarious
#include <setapp.h>

#include <calendar.h>

char *strncpyZ(char *dest, const char *orig, int size);

static TDskWinCalendar *helper;

TStreamable *TDskWinCalendar::build()
{
 return new TDskWinCalendar(streamableInit);
}

TDskWinCalendar::TDskWinCalendar()
{
 view=new TCalendarWindow();
 TProgram::deskTop->insert(view);
 type=dktCalendar;
 CanBeDeletedFromDisk=0;
 CanBeSaved=1;
 helper=this;
}

char *TDskWinCalendar::GetText(char *dest, short max)
{
 char *calendar=TVIntl::getTextNew(__("   Calendar "));
 int lCal=strlen(calendar);
 AllocLocalStr(buffer,lCal+40+1);
 strcpy(buffer,calendar);
 ((TCalendarWindow *)view)->view->getMonthStr(buffer+lCal,40,0);
 strncpyZ(dest,buffer,max);
 DeleteArray(calendar);
 return dest;
}

TDskWinCalendar::~TDskWinCalendar()
{
 helper=0;
}

void TDskWinCalendar::write(opstream& os)
{
 os << (uchar)1 << view->origin << (int)(TProgram::deskTop->indexOf(view));
}

void *TDskWinCalendar::read(ipstream& is)
{
 uchar version;
 TPoint pos;
 is >> version >> pos >> ZOrder;
 view=new TCalendarWindow();
 view->moveTo(pos.x,pos.y);
 helper=this;
 return this;
}

void CalendarWindow()
{
 if (helper)
    helper->view->select();
 else
    AddNonEditorToHelper(new TDskWinCalendar());
}
