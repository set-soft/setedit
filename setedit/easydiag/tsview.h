/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSView) && !defined(__TSView_Defined__)
#define __TSView_Defined__

const int xTSCenter=-1,xTSLeft=-2,xTSRight=-3,xTSLeftOf=-4,xTSRightOf=-5;
const int yTSCenter=-1,yTSUp=-2,yTSDown=-3,yTSOver=-4,yTSUnder=-5,yTSUpSep=-6;
const int wSpan=1;


class TSView
{
public:
 TSView() { xSep=xDefSep; ySep=yDefSep; Flags=0; };
 virtual ~TSView() {};
 int w,h,x,y;
 int xSep,ySep;
 unsigned Flags;
 TView *view;
 virtual void insert(TDialog *);
 virtual void setWidth(int aW) { w=aW; };
 virtual int  howManyHz() { return 1; };
 virtual void setGrowMode(unsigned val) { view->growMode=val; };
 static int xDefSep,yDefSep;
};

int  EDMaxWidth(TSView *first, ...);
void EDForceSameWidth(TSView *first, ...);
#endif
