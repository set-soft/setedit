/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSListBox
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

TSListBox::TSListBox(int width, int height, int scrolls, int cols, int aHSRange,
                     tMakeListBox MakeListBox) :
  TSView()
{
 w=width;
 h=height;
 scrollType=scrolls;

 vScrollBar=hScrollBar=0;
 int hScrl=(scrolls & tsslbHorizontal) ? 1 : 0;
 int vScrl=(scrolls & tsslbVertical)   ? 1 : 0;
 if (vScrl)
    vScrollBar=new TScrollBar(TRect(0,0,1,height-hScrl));
 if (hScrl)
   {
    hScrollBar=new TScrollBar(TRect(0,0,width-vScrl,1));
    if (aHSRange>0)
      hScrollBar->setRange(0,aHSRange);
   }
 view=MakeListBox(TRect(0,0,width-vScrl,height-hScrl),cols,hScrollBar,vScrollBar);
}


void TSListBox::insert(TDialog *d)
{
 if (scrollType==tsslbNo)
   {
    TSView::insert(d);
    return;
   }
 int hScrl=(scrollType & tsslbHorizontal) ? 1 : 0;
 int vScrl=(scrollType & tsslbVertical)   ? 1 : 0;

 TRect r(x,y,x+w-vScrl,y+h-hScrl);
 view->locate(r);
 d->insert(view);
 if (vScrollBar)
   {
    vScrollBar->moveTo(x+w-1,y);
    d->insert(vScrollBar);
   }
 if (hScrollBar)
   {
    TRect r2(x,y+h-1,x+w-vScrl,y+h);
    hScrollBar->locate(r2);
    d->insert(hScrollBar);
   }
}

// Just a TListBox object
TView *TSListBoxMakeListBox(const TRect& bounds, unsigned aNumCols,
                            TScrollBar *aHScrollBar, TScrollBar *aVScrollBar)
{
 return new TListBox(bounds,aNumCols,aHScrollBar,aVScrollBar,True);
}
