/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSHScrollBar
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

TSHScrollBar::TSHScrollBar(int width, int max) :
   TSView()
{
 w=width+2;
 h=1;
 sb=new TScrollBar(TRect(0,0,w,1));
 view=sb;
 setMax(max);
}

void TSHScrollBar::setMax(int max)
{
 sb->setParams(0,0,max,(int)(max/(double)(w-2)+0.5),1);
}
