/* Copyright (C) 2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSSlider
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>


/**[txh]********************************************************************

  Description:
  This callback is quite tricky. The idea is to move both scrollbars at the
same time when some value (pointed by join) is != 0. The methode is tricky
and I don't know if I was cleaver or a completly waco *;-)

***************************************************************************/

static
void CallBack(int value, TScrollBarCB *obj, void *data)
{
 stTSSliderInfo *st=(stTSSliderInfo *)data;
 TSSlider *sl=st->obj;
 if (st->side) // Right side
   { // Call right callback
    if (sl->rcb) sl->rcb(value,obj,sl->rdata);
    // And copy the value to the other
    if (sl->join && *(sl->join) && sl->left) sl->left->setValue(value);
   }
 else // Left side
   { // Call left callback
    if (sl->lcb) sl->lcb(value,obj,sl->ldata);
    // And copy the value to the other
    if (sl->join && *(sl->join) && sl->right) sl->right->setValue(value);
   }
}

TSSlider::TSSlider(int height, Boolean stereo, const char *aLabel, TScrollBarCBfunc aLcb,
                   void *aLdata, TScrollBarCBfunc aRcb, void *aRdata, int *Join,
                   int aMax) :
  TSView()
{
 h=height--;
 w=5;
 stLeft.side=0;  stLeft.obj=this;
 stRight.side=1; stRight.obj=this;
 join=Join;
 if (stereo)
   {
    left =new TScrollBarCB(TRect(1,0,2,height),CallBack,&stLeft);
    right=new TScrollBarCB(TRect(3,0,4,height),CallBack,&stRight);
   }
 else
   {
    left =new TScrollBarCB(TRect(2,0,3,height),CallBack,&stLeft);
    right=0;
   }
 label=aLabel ? new TStaticText(TRect(0,height,5,height+1),aLabel) : 0;
 view=label;
 max=aMax;
 ldata=aLdata; rdata=aRdata;
 lcb=aLcb; rcb=aRcb;
}

void TSSlider::Set(int valueLeft, int valueRight)
{
 left->setParams(valueLeft,0,max,max,max/10);
 if (right)
    right->setParams(valueRight,0,max,max,max/10);
}

void TSSlider::insert(TDialog *d)
{
 TRect r=left->getBounds();
 left->moveTo(x+r.a.x,y+r.a.y);
 d->insert(left);
 if (right)
   {
    r=right->getBounds();
    right->moveTo(x+r.a.x,y+r.a.y);
    d->insert(right);
   }
 if (label)
   {
    r=label->getBounds();
    label->moveTo(x+r.a.x,y+r.a.y);
    d->insert(label);
   }
}

void TScrollBarCB::scrollDraw()
{
 TScrollBar::scrollDraw();
 if (callBack)
    callBack(value,this,data);
}
