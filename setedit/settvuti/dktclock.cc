/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#include <stdio.h>
#define Uses_TView
#define Uses_TDeskTopClock
#include <settvuti.h>

TDeskTopClock:: TDeskTopClock(const TRect & r) :
  TView(r), curTime(0)
{
 growMode=gfGrowLoX | gfGrowHiX;
 putSeparator=0;
};

void TDeskTopClock::update()
{
 time_t curtime;
 time(&curtime);
 if (curtime!=curTime)
   {
    curTime=curtime;
    drawView();
   }
}

void TDeskTopClock::draw()
{
 TDrawBuffer b;
 uchar c = getColor(2);

 if (curTime==0)
    time(&curTime);
 struct tm *timeStruct=localtime(&curTime);
 char buffer[12];
 // Not full international but digital clocks have only it ;-)
 if (mode)
    strftime(buffer,11,CLK24_FORMAT,timeStruct);
 else
    strftime(buffer,11,CLKAMPM_FORMAT,timeStruct);
 if (putSeparator)
    putSeparator=0;
 else
   {
    putSeparator=1;
    buffer[2]=' ';
   }
 b.moveStr(0,buffer,c);

 writeLine(0,0,size.x,1,b);
}

int TDeskTopClock::mode=1; // 1 => 24hs 0 => AM/PM


