/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSTextScroller
#include <string.h>
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

TSTextScroller::TSTextScroller(int width, int height, TNSCollection *str,
                               int haveHoriz, int haveVert, int XLimit) :
  TSView()
{
 w=width;
 h=height;
 horiz=haveHoriz ? 1 : 0;
 vert=haveVert ? 1 : 0;

 if (horiz)
    hS=new TScrollBar(TRect(0,0,width-vert,1));
 else
    hS=0;
 if (vert)
    vS=new TScrollBar(TRect(0,0,1,height-horiz));
 else
    vS=0;

 if (XLimit<0)
   {
    int c=str->getCount(),i,max=0,l;
    for (i=0; i<c; i++)
       {
        l=strlen((char *)str->at(i));
        if (l>max)
           max=l;
       }
    XLimit=max;
   }

 TTextScroller *scroller=new TTextScroller(TRect(0,0,width-vert,height-horiz),hS,vS,str);
 view=scroller;
 scroller->setLimit(XLimit,str->getCount());
}

void TSTextScroller::insert(TDialog *d)
{
 TRect r(x,y,x+w-vert,y+h-horiz);
 view->locate(r);
 d->insert(view);

 if (horiz)
   {
    TRect r(x,y+h-1,x+w-vert,y+h);
    hS->locate(r);
    d->insert(hS);
   }
 if (vert)
   {
    TRect r(x+w-1,y,x+w,y+h-horiz);
    vS->locate(r);
    d->insert(vS);
   }
}

