/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSCheckBoxesArray
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

TSCheckBoxesArray::TSCheckBoxesArray(int xNum, int yNum)
{
 fill(xNum,yNum);
}

TSCheckBoxesArray::TSCheckBoxesArray(int xNum, int yNum, int (*pressCB)(int))
{
 TCheckBoxesArray *p=fill(xNum,yNum);
 p->pressCallBack=pressCB;
}

TCheckBoxesArray *TSCheckBoxesArray::fill(int xNum, int yNum)
{
 w=xNum*4+1;
 h=yNum;
 TCheckBoxesArray *p=new TCheckBoxesArray(TRect(0,0,w,h),xNum*yNum);
 view=p;
 return p;
}


