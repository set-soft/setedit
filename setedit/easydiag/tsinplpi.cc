/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSInputLinePiped
#include <easydia1.h>
#include <settvuti.h>
#include <easydiag.h>

TSInputLinePiped::TSInputLinePiped(int MaxLen, unsigned flags)
{
 fill(MaxLen,0,0,-1,flags);
}

TSInputLinePiped::TSInputLinePiped(int MaxLen, ushort ID, unsigned flags)
{
 fill(MaxLen,1,ID,-1,flags);
}

TSInputLinePiped::TSInputLinePiped(int MaxLen, int wForce, unsigned flags)
{
 fill(MaxLen,0,0,wForce,flags);
}
 
void TSInputLinePiped::fill(int MaxLen, int haveID, ushort ID, int wForce, unsigned flags)
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
    w=MaxLen+2;
   }
 h=1;
 TInputLine *p;
 p=new TInputLinePiped(TRect(0,0,w,h),MaxLen,flags);
 view=p;
 if (haveID)
   {
    w+=3;
    hist=new THistory(TRect(0,0,3,1),p,ID);
   }
 else
   hist=0;
}

void TSInputLinePiped::insert(TDialog *d)
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

