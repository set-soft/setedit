/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSRadioButtons
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

TSRadioButtons::TSRadioButtons(TSItem *aStrings, int wForced, int Columns)
{
 TSItem *itemAux=aStrings;
 int items=0;

 while (itemAux)
   {
    items++;
    itemAux=itemAux->next;
   }
 h=items/Columns;
 if (items%Columns) h++;

 #ifdef Clusters32Bits
 TRadioButtons *rb=new TRadioButtons32(TRect(0,0,w,h),aStrings);
 #else
 TRadioButtons *rb=new TRadioButtons(TRect(0,0,w,h),aStrings);
 #endif
 view=rb;
 if (wForced>=0)
   {
    w=wForced;
   }
 else
   {
    int wMax=0,l,item,itY;

    w=0;
    for (item=itY=0; item<items; item++)
       {
        l=cstrlen(rb->getItemText(item))+6;
        if (l>wMax) wMax=l;
        itY++;
        if (itY==h)
          {
           itY=0;
           w+=wMax;
           wMax=0;
          }
       }
    if (wMax) w+=wMax;
   }
}

void TSRadioButtons::setEnableMask(uint32 *masks, TSView **views, int cViews)
{
 int i;
 TView **p=(TView **)views;
 for (i=0; i<cViews; i++)
     p[i]=views[i]->view;
 ((TRadioButtons *)view)->setEnableMask(masks,p,cViews);
}

