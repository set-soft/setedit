/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Class: TProgressBar
  Description:
  Progresive bar. Based on code of Jay Perez and Barnaby W. Falls. Looks
like Jay based it on a C++ example of some library adapting it to TVision.

***************************************************************************/
#include <stdio.h>       // sprintf()
#define Uses_string      // memset

#define Uses_TProgressBar
#define Uses_TView
#define Uses_TRect
#define Uses_TGroup
#define Uses_TDrawBuffer
#define Uses_TStreamableClass
#define Uses_fpstream
#define Uses_TPalette
#include <settvuti.h>

#define cpProgressBar "\x04"
/*                       ³
                         ÀÄÄ Progress Bar Attrib Pair
     cpProgressBar maps to TProgram::appPalette's index 4 which is, by
default, used for the ScrollBar Page. cpProgressBar represents the normal
progress bar color. In the constructor the foreground and background
attributes are swapped to form the highlight color. Thus the highlight will
always be the inverse of the bar color.
*/

TProgressBar::TProgressBar(const TRect& bounds, unsigned long aTotal, char abackChar) :
   TView(bounds)
{
 backChar = abackChar;
 total = aTotal;
 numOffset = (size.x/2)-3;
 bar = new char[size.x+1];
 memset(bar,backChar,size.x);
 bar[size.x] = '\0';
 charValue = 100.0/size.x;
 progress =
 curPercent =
 curWidth = 0;
}

TProgressBar::~TProgressBar()
{
 delete bar;
}

void TProgressBar::draw()
{
 char string[4];

 // A value greater than 999 can damage the stack so avoid it
 if (curPercent>100)
    curPercent=100;
 // That's slower but easier than the original
 sprintf(string,"%3d",curPercent);
 /* Original code, itoa isn't supported in glibc, hey man! no itoX, no tell,
    what a hell ...
 itoa(curPercent,string,10);
 string[3] = '\0';
 if (curPercent<10)
   {
    string[2] = string[0];
    string[1] = string[0] = ' ';
   }
 else
   if (curPercent<100 && curPercent>9)
     {
      string[2] = string[1];
      string[1] = string[0];
      string[0] = ' ';
     }*/
 TDrawBuffer nbuf;
 uchar colorNormal, colorHiLite;
 colorNormal = getColor(1);
 uchar fore  = colorNormal>>4;
 colorHiLite = fore+((colorNormal-(fore<<4))<<4);
 nbuf.moveChar(0,backChar,colorNormal,size.x);
 nbuf.moveStr(numOffset,string,colorNormal);
 nbuf.moveStr(numOffset+3," %",colorNormal);
 for (unsigned i=0;i<curWidth;i++)
     nbuf.putAttribute(i,colorHiLite);
 writeLine(0, 0, size.x, 1, nbuf);
}


TPalette& TProgressBar::getPalette() const
{
 static TPalette palette( cpProgressBar, sizeof( cpProgressBar )-1 );
 return palette;
}


void TProgressBar::update(unsigned long aProgress)
{
 progress = aProgress;
 if (calcPercent())
    drawView();
}

int TProgressBar::calcPercent()
{
 unsigned int percent;
 unsigned int width;

 // calculate the new percentage
 percent = (int) ( ((double)progress/(double)total) * 100 );

 // percentage change?
 if (percent!=curPercent)
   {
    curPercent = percent;          // save new percentage
    width = (int)((double)curPercent/charValue);// calculate percentage bar width

    // width change?
    if (width!=curWidth)
       curWidth = width;          // save new width
    return 1;
   }
 return 0;
}

// return the maximum iteration
unsigned long TProgressBar::getTotal()
{
 return total;
}

// return the current iteration
unsigned long TProgressBar::getProgress()
{
 return progress;
}

// set a new maximum iteration & update display
void TProgressBar::setTotal(unsigned long newTotal)
{
 unsigned long tmp = total;
 total = newTotal;
 memset(bar,backChar,size.x);
 curWidth   = 0;                    // current width of percentage bar
 progress   = 0;                    // current iteration
 curPercent = 0;                    // current percentage
 if (tmp)                // since it starts with 0, only update if changing
    drawView();                       // update the thermometer bar display
}

// set a new current iteration & update display
void TProgressBar::setProgress(unsigned long newProgress)
{
 progress = newProgress;
 if (calcPercent())
    drawView();                       // paint the thermometer bar
}

void TProgressBar::write( opstream& os )
{
 TView::write( os );
 os.writeString( bar );
 os << backChar << total << progress << dispLen <<
       curPercent << curWidth << numOffset << charValue;
}

void *TProgressBar::read( ipstream& is )
{
 TView::read( is );
 bar = is.readString();
 is >> backChar >> total >> progress >> dispLen >>
       curPercent >> curWidth >> numOffset >> charValue;
 return this;
}


