/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSButton
#define Uses_TSViewCol
#define Uses_TSHzGroup
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

const int tpTButton=0, tpTButtonRef=1;

TSButton::TSButton(const char *aTitle, ushort aCommand, ushort aFlags, int wForced)
{
 fill(tpTButton,aTitle,aCommand,aFlags,wForced,0,NULL);
}

TSButton::TSButton(const char *aTitle, ushort aCommand, ushort aFlags)
{
 fill(tpTButton,aTitle,aCommand,aFlags,-1,0,NULL);
}

TSButton::TSButton(const char *aTitle, ushort aCommand, ushort aFlags,
                           int wForced, TButtonCallBack cb, void *cbData)
{
 fill(tpTButton,aTitle,aCommand,aFlags,wForced,cb,cbData);
}

TSButton::TSButton(const char *aTitle, ushort aCommand, ushort aFlags,
                           TButtonCallBack cb, void *cbData)
{
 fill(tpTButton,aTitle,aCommand,aFlags,-1,cb,cbData);
}

TSButtonRef::TSButtonRef(const char *aTitle, ushort aCommand, ushort aFlags, int wForced)
{
 fill(tpTButtonRef,aTitle,aCommand,aFlags,wForced,0,NULL);
}

TSButtonRef::TSButtonRef(const char *aTitle, ushort aCommand, ushort aFlags)
{
 fill(tpTButtonRef,aTitle,aCommand,aFlags,-1,0,NULL);
}

TSButtonRef::TSButtonRef(const char *aTitle, ushort aCommand, ushort aFlags,
                           int wForced, TButtonCallBack cb, void *cbData)
{
 fill(tpTButtonRef,aTitle,aCommand,aFlags,wForced,cb,cbData);
}

TSButtonRef::TSButtonRef(const char *aTitle, ushort aCommand, ushort aFlags,
                           TButtonCallBack cb, void *cbData)
{
 fill(tpTButtonRef,aTitle,aCommand,aFlags,-1,cb,cbData);
}

void TSButtonBase::fill(int type, const char *aTitle, ushort aCommand, ushort aFlags,
                        int wForced, TButtonCallBack cb, void *cbData)
{
 if (wForced>=0)
    w=wForced;
 else
   {
    stTVIntl *cache=NULL;
    const char *t=TVIntl::getText(aTitle,cache);
    w=cstrlen(t)+3;
    TVIntl::freeSt(cache);
   }
 h=2;
 TButton *b=NULL;
 if (type==tpTButton)
    b=new TButton(TRect(0,0,w+1,h),aTitle,aCommand,aFlags);
 else if (type==tpTButtonRef)
    b=new TButtonRef(TRect(0,0,w+1,h),aTitle,aCommand,aFlags);
 if (cb!=0)
    b->setCallBack(cb,cbData);
 view=b;
 // That's the more common ... I guess
 b->growMode=gfGrowAll;
}

void TSButtonBase::insert(TDialog *d)
{
 // Why in the hell buttons are inserted in x+1?
 TRect r(x-1,y,x+w,y+h);
 view->locate(r);
 d->insert(view);
}

void EasyInsertOKCancelSp(TSViewCol *col, int sep)
{
 TSHzGroup *but12=new TSHzGroup(new TSButton(__("O~K~"),cmOK,bfDefault),
                  new TSButton(__("Cancel"),cmCancel),sep);
 col->insert(xTSCenter,yTSDown,but12);
 but12->Flags=wSpan;
}

void EasyInsertOKCancel(TSViewCol *col, int sep)
{
 TSButton *ok=new TSButton(__("O~K~"),cmOK,bfDefault);
 //ok->view->growMode=gfGrowAll;
 TSButton *cancel=new TSButton(__("Cancel"),cmCancel);
 //cancel->view->growMode=gfGrowAll;
 TSHzGroup *but12=new TSHzGroup(ok,cancel,sep);
 col->insert(xTSCenter,yTSDown,but12);
}
