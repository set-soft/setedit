/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSCheckBoxes
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

TSCheckBoxes::TSCheckBoxes(TSItem *aStrings, int wForced, int Columns)
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
 TCheckBoxes *cb=new TCheckBoxes32(TRect(0,0,w,h),aStrings);
 #else
 TCheckBoxes *cb=new TCheckBoxes(TRect(0,0,w,h),aStrings);
 #endif
 view=cb;
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
        l=cstrlen(cb->getItemText(item))+6;
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


