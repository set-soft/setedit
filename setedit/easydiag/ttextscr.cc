/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_string
#define Uses_AllocLocal
#define Uses_TTextScroller
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

TTextScroller::TTextScroller(const TRect& bounds, TScrollBar *aHScrollBar,
                             TScrollBar *aVScrollBar, TNSCollection *str) :
                             TScroller(bounds,aHScrollBar,aVScrollBar)
{
 text=str;
}

void TTextScroller::draw()
{
 char *p;
 
 ushort c=getColor(1);
 for (int i=0; i<size.y; i++)
    {
     TDrawBuffer b;
     b.moveChar(0,' ',c,size.x);
 
     if (delta.y+i<text->getCount())
       {
        AllocLocalStr(s,size.x+1);
        p=(char *)text->at(delta.y+i);
        if (!p || strlen(p)<(unsigned)delta.x)
           s[0] = EOS;
        else
          {
           strncpy(s,p+delta.x,size.x);
           s[size.x]=EOS;
          }
        b.moveStr( 0, s, c );
       }
     writeBuf( 0, i, size.x, 1, b );
    }
}

TPalette& TTextScroller::getPalette() const
{
 static TPalette palette("\x10",1);
 return palette;
}

void TTextScroller::handleEvent(TEvent& event)
{
 TScroller::handleEvent(event);
 // As I have the focus they don't get the events
 if (event.what==evKeyDown && hScrollBar)
    hScrollBar->handleEvent(event);
 if (event.what==evKeyDown && vScrollBar)
    vScrollBar->handleEvent(event);
}
