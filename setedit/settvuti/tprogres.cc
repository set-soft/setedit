/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Class: TProgress
  Description:
  That's a very simply progress indicator used when we don't know how much
items will be processed and we don't know the user could think the program
hanged.

***************************************************************************/
#include <stdio.h>       // sprintf()
#define Uses_string
#define Uses_AllocLocal
#define Uses_TProgress
#define Uses_TView
#define Uses_TRect
#define Uses_TGroup
#define Uses_TDrawBuffer
#define Uses_TStreamableClass
#define Uses_fpstream
#define Uses_TPalette
#include <settvuti.h>

// Same as static text, after all we are almost this
#define cpProgress "\x06"

TProgress::TProgress(const TRect& bounds, const char *aMessage) :
   TView(bounds),
   message(TVIntl::getTextNew(aMessage)),
   state(0)
{
 len=strlen(message)+3;
}

TProgress::~TProgress()
{
 DeleteArray(message);
}

void TProgress::draw()
{
 AllocLocalStr(string,len);
 sprintf(string,"%s %c",message,states[state]);

 TDrawBuffer nbuf;
 nbuf.moveStr(0,string,getColor(1));
 writeLine(0,0,size.x,1,nbuf);
}


TPalette& TProgress::getPalette() const
{
 static TPalette palette(cpProgress,sizeof(cpProgress)-1);
 return palette;
}


void TProgress::update()
{
 state++;
 if (state>3)
    state=0;
 drawView();
}

void TProgress::write(opstream& os)
{
 TView::write(os);
 os.writeString(message);
}

void *TProgress::read(ipstream& is)
{
 TView::read(is);
 message=is.readString();
 return this;
}

char TProgress::states[4]={'-','\\','|','/'};
