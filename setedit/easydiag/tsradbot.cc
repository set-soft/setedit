/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
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
 
 if (wForced>=0)
   {
    w=wForced;
   }
 else
   {
    int wMax=0,l,item=0;

    w=0;
    itemAux=aStrings;
    while (itemAux)
      {
       l=cstrlen(itemAux->value)+6;
       if (l>wMax) wMax=l;
       itemAux=itemAux->next;
       item++;
       if (item==h)
         {
          item=0;
          w+=wMax;
          wMax=0;
         }
      }
    if (wMax) w+=wMax;
   }
 #ifdef Clusters32Bits
 view=new TRadioButtons32(TRect(0,0,w,h),aStrings);
 #else
 view=new TRadioButtons(TRect(0,0,w,h),aStrings);
 #endif
}
