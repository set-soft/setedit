/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSButton
#define Uses_TSViewCol
#define Uses_TSHzGroup
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

TSButton::TSButton(const char *aTitle, ushort aCommand, ushort aFlags, int wForced)
{
 fill(aTitle,aCommand,aFlags,wForced,0);
}

TSButton::TSButton(const char *aTitle, ushort aCommand, ushort aFlags)
{
 fill(aTitle,aCommand,aFlags,-1,0);
}

TSButton::TSButton(const char *aTitle, ushort aCommand, ushort aFlags,
                   int wForced, TButtonCallBack cb)
{
 fill(aTitle,aCommand,aFlags,wForced,cb);
}

TSButton::TSButton(const char *aTitle, ushort aCommand, ushort aFlags,
                   TButtonCallBack cb)
{
 fill(aTitle,aCommand,aFlags,-1,cb);
}

void TSButton::fill(const char *aTitle, ushort aCommand, ushort aFlags,
                    int wForced, TButtonCallBack cb)
{
 if (wForced>=0)
    w=wForced;
 else
    w=cstrlen(aTitle)+3;
 h=2;
 TButton *b=new TButton(TRect(0,0,w+1,h),aTitle,aCommand,aFlags);
 if (cb!=0)
    b->setCallBack(cb);
 view=b;
}

void TSButton::insert(TDialog *d)
{
 // Why in the hell buttons are inserted in x+1?
 TRect r(x-1,y,x+w,y+h);
 view->locate(r);
 d->insert(view);
}

void EasyInsertOKCancelSp(TSViewCol *col, int sep)
{
 TSHzGroup *but12=new TSHzGroup(new TSButton(_("O~K~"),cmOK,bfDefault),
                  new TSButton(_("Cancel"),cmCancel),sep);
 col->insert(xTSCenter,yTSDown,but12);
 but12->Flags=wSpan;
}

void EasyInsertOKCancel(TSViewCol *col, int sep)
{
 TSButton *ok=new TSButton(_("O~K~"),cmOK,bfDefault);
 ok->view->growMode=gfGrowAll;
 TSButton *cancel=new TSButton(_("Cancel"),cmCancel);
 cancel->view->growMode=gfGrowAll;
 TSHzGroup *but12=new TSHzGroup(ok,cancel,sep);
 col->insert(xTSCenter,yTSDown,but12);
}
