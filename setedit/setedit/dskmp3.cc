/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#include <stdio.h>
class TRect;
#define Uses_TView
#define Uses_TPoint
#define Uses_TProgram
#define Uses_TDeskTop
#include <ceditor.h>
#include <dskwin.h>
#include <dskmp3.h>

//  setFocusTo and focusChanged
#define Uses_SETAppVarious
#include <setapp.h>
#include <intermp3.h>
#include <mixer.h>

TStreamable *TDskWinMP3::build()
{
 return new TDskWinMP3(streamableInit);
}

TDskWinMP3::TDskWinMP3(TView *w)
{
 view=w;
 type=dktMP3;
 CanBeDeletedFromDisk=0;
 // Before the MP3 engine
 BoardMixerInit();
#ifdef SUP_MP3
 CanBeSaved=1;
#else
 CanBeSaved=0;
#endif
}

char *TDskWinMP3::GetText(char *dest, short maxLen)
{
 TVIntl::snprintf(dest,maxLen,__("   MP3 Control panel"));
 return dest;
}

int TDskWinMP3::GoAction(ccIndex pos)
{
 TDskWin::GoAction(pos);
 CanBeSaved=1;
 return 0;
}

int TDskWinMP3::DeleteAction(ccIndex, Boolean)
{
 view->hide();
 CanBeSaved=0;
 return 0;
}

TDskWinMP3::~TDskWinMP3()
{
}

#ifdef SUP_MP3
void TDskWinMP3::write(opstream& os)
{
 if (!CanBeSaved)
    return;

 MP3WriteInfo(os,view);
}

void *TDskWinMP3::read(ipstream& is)
{
 // Before the MP3 engine
 BoardMixerInit();
 view=MP3ReadInfo(is,ZOrder,this);
 return this;
}
#else
void TDskWinMP3::write(opstream& )
{
}

void *TDskWinMP3::read(ipstream& is)
{
 view=MP3ReadInfo(is,ZOrder,this);
 return this;
}
#endif
