/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSInputLine
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

TSInputLine::TSInputLine(int MaxLen)
{
 fill(MaxLen,0,0,-1,TSInputLineMake);
}

TSInputLine::TSInputLine(int MaxLen, ushort ID)
{
 fill(MaxLen,1,ID,-1,TSInputLineMake);
}

TSInputLine::TSInputLine(int MaxLen, int wForce)
{
 fill(MaxLen,0,0,wForce,TSInputLineMake);
}
 
void TSInputLine::fill(int MaxLen, int haveID, ushort ID, int wForce,
                       tMakeInputLine makeIt)
{
 if (wForce>=0)
   {
    w=wForce;
    if (haveID)
       w-=3;
   }
 else
   {
    // We need 2 extra chars to feet MaxLen-1 chars + 0
    w=MaxLen+1;
   }
 h=1;
 TInputLine *p;
 p=makeIt(TRect(0,0,w,h),MaxLen);
 view=p;
 if (haveID)
   {
    w+=3;
    hist=new THistory(TRect(0,0,3,1),p,ID);
   }
 else
   hist=0;
}

void TSInputLine::insert(TDialog *d)
{
 TRect r(x,y,x+w-(hist ? 3 : 0),y+h);
 view->locate(r);
 d->insert(view);
 if (hist)
   {
    hist->moveTo(x+w-3,y);
    d->insert(hist);
   }
}

InputLineImplement(InputLine);

