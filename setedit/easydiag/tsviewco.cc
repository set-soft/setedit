/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define NDEBUG
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#define Uses_TSViewCol
#define Uses_TApplication
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

TSViewCol::TSViewCol(const char *name) :
  TNSCollection(5,5)
{
 d=new TDialog(TRect(1,1,1,1),name);
}

void TSViewCol::insert(int x, int y, TSView *view, TSView *refX, TSView *refY)
{
 SViewNode *node=new SViewNode;
 node->view=view;
 node->refX=refX;
 node->refY=refY;
 view->x=x;
 view->y=y;
 TNSCollection::insert(node);
}

void TSViewCol::freeItem(void *item)
{
 SViewNode *node=(SViewNode *)item;
 delete node->view;
 delete node;
}

const int xTSMargin=2,yTSMargin=1;


TDialog *TSViewCol::doIt(TDeskTop *dkt)
{
 // First step: figure out the size of the dialog
 int w,h,wNeed=0,hNeed=0,wThis,xThis,xTemp;
 // hThis=0, egcs is crazy and thinks it could be used uninitialized!
 int hThis=0,yThis,yTemp;
 TSView *This;
 TRect r=d->getBounds();
 w=max(r.b.x-r.a.x,2*xTSMargin);
 w=max(w,(int)strlen(d->getTitle(0))+14);
 h=max(r.b.y-r.a.y,2*yTSMargin);
 int i;
 SViewNode *node;

 for (i=0; i<count; i++)
    {
     node=(SViewNode *)at(i);
     This=node->view;

     // Solve needed width and x in the following cases:
     // xTSLeft Left of the dialog
     // xTSLeftOf Left of an object
     // xTSRightOf Right of an object
     wThis=This->w;
     xThis=This->x;
     if (xThis>=0)
       { // Ok, the position is fixed
        wNeed=xThis+wThis;
       }
     else
       {
        switch (xThis)
          {
           case xTSRight:
                //wNeed=w-xTSMargin+wThis+This->xSep;
                wNeed=xTSMargin+wThis;
                break;
           case xTSCenter:
                wNeed=xTSMargin+wThis;
                break;
           case xTSLeft:
                This->x=xTSMargin;
                wNeed=xTSMargin+wThis;//+This->xSep;
                break;
           case xTSLeftOf:
                wNeed=0;
                assert(node->refX);
                xTemp=node->refX->x;//+node->refX->w;
                assert(xTemp>=0);
                This->x=max(0,xTemp-This->xSep-wThis);
                break;
           case xTSRightOf:
                assert(node->refX);
                xTemp=node->refX->x+node->refX->w;
                assert(xTemp>=0);
                This->x=xTemp+This->xSep;
                wNeed=This->x+wThis;
                break;
           default:
                assert(0);
          }
       }
     wNeed+=xTSMargin;
     if (wNeed>w)
        w=wNeed;

     // Solve needed height and y in the following cases:
     hThis=This->h;
     yThis=This->y;
     if (yThis>=0)
       { // Ok, the position is fixed
        hNeed=yThis+hThis;
       }
     else
       {
        switch (yThis)
          {
           case yTSDown:
                hNeed=h-yTSMargin+hThis+This->ySep;
                break;
           case yTSCenter:
                hNeed=yTSMargin+hThis;
                break;
           case yTSUp:
                This->y=yTSMargin;
                hNeed=yTSMargin+hThis;
                break;
           case yTSUpSep:
                This->y=yTSMargin+1;
                hNeed=yTSMargin+1+hThis;
                break;
           case yTSOver:
                hNeed=0;
                assert(node->refY);
                yTemp=node->refY->y;
                assert(yTemp>=0);
                This->y=max(0,yTemp-This->ySep-hThis);
                break;
           case yTSUnder:
                assert(node->refY);
                yTemp=node->refY->y+node->refY->h;
                assert(yTemp>=0);
                This->y=yTemp+This->ySep;
                hNeed=This->y+hThis;
                break;
           default:
                assert(0);
          }
       }
     hNeed+=yTSMargin;
     if (hNeed>h)
        h=hNeed;
    }
 // Hopefully here we have the w/h of the dialog, some x/y remains unsolved
 d->growTo(w,h);
 // Now is safe to insert the views because there is enough room in the buffer

 /**** Second step: Is time to insert the views ****/
 for (i=0; i<count; i++)
    {
     node=(SViewNode *)at(i);
     This=node->view;

     if (This->Flags & wSpan)
       {
        int wAux=w-xTSMargin;
        if (This->x<0)
           wAux-=xTSMargin;
        else
           wAux-=This->x;
        This->setWidth(wAux);
       }
     // Solve X if needed
     xThis=This->x;
     wThis=This->w;
     if (xThis<0)
       {
        switch (xThis)
           {
            case xTSRight:
                 xThis=This->x=max(0,w-xTSMargin-wThis);
                 break;
            case xTSCenter:
                 assert(wThis<w);
                 xThis=This->x=(w-wThis)/2;
                 break;
            default:
                 assert(0);
           }
       }
     // Solve Y if needed
     yThis=This->y;
     hThis=This->h;
     if (yThis<0)
       {
        switch (yThis)
           {
            case yTSDown:
                 yThis=This->y=max(0,h-yTSMargin-hThis);
                 break;
            case yTSCenter:
                 assert(hThis<h);
                 yThis=This->y=(h-hThis)/2;
                 break;
            default:
                 assert(0);
           }
       }
     // So now we can resize it
     /*r.a.x=xThis;
     r.a.y=yThis;
     r.b.x=xThis+wThis;
     r.b.y=yThis+hThis;
     This->view->locate(r);
     d->insert(This->view);*/
     This->insert(d);
     //d->insert(This);
    }
 d->selectNext(False);
 if (dkt)
    dkt->insert(d);
 return d;
}

TDialog *TSViewCol::doItCenter(int context)
{
 TDialog *d=doIt();
 d->options|=ofCentered;
 d->helpCtx=context;
 return d;
}

ushort TSViewCol::exec(void *data, int center)
{
 TDialog *d=doIt();
 if (center)
    d->options|=ofCentered;

 TView *p=TProgram::application->validView(d);
 if (p==0)
     return cmCancel;
 else
  {
   if (data!=0)
      p->setData(data);
   ushort result=TProgram::deskTop->execView(p);
   if (result!=cmCancel && data!=0)
      p->getData(data);
   TObject::CLY_destroy(p);
   return result;
  }
}

int EDMaxWidth(TSView *first, ...)
{
 va_list arg;
 va_start(arg,first);

 TSView *s;
 int max=first->w;

 while ((s=va_arg(arg,TSView *))!=0)
   {
    if (s->w>max)
       max=s->w;
   }
 va_end(arg);

 return max;
}

void EDForceSameWidth(TSView *first, ...)
{
 va_list arg;
 va_start(arg,first);

 TSView *s;
 int max=first->w;

 while ((s=va_arg(arg,TSView *))!=0)
   {
    if (s->w>max)
       max=s->w;
   }
 va_end(arg);

 va_start(arg,first);
 first->setWidth(max);
 while ((s=va_arg(arg,TSView *))!=0)
       s->setWidth(max);
 va_end(arg);
}

