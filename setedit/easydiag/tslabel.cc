/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSLabel
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

TSLabel::TSLabel(const char *aText, TSView *link) :
   TSView()
{
 stTVIntl *cache=NULL;
 const char *str=TVIntl::getText(aText,cache);
 w=max(cstrlen(str)+1,link->w);
 h=1+link->h;
 view=new TLabel(TRect(0,0,w,1),aText,link->view,cache);
 linked=link;
}

void TSLabel::insert(TDialog *d)
{
 // Why in the hell labels are inserted in x+1?
 TRect r(x-1,y,x-1+w,y+1);
 view->locate(r);
 d->insert(view);
 linked->x=x;
 linked->y=y+1;
 linked->insert(d);
}

void TSLabel::setWidth(int aW)
{
 w=aW;
 linked->setWidth(aW);
}

void TSLabel::setGrowMode(unsigned val)
{
 view->growMode=val;
 linked->setGrowMode(val);
}

TSHzLabel::TSHzLabel(const char *aText, TSView *link, int separation) :
   TSView()
{
 stTVIntl *cache=NULL;
 int lenStr=cstrlen(TVIntl::getText(aText,cache));

 xSecond=lenStr+separation;
 w=xSecond+link->w;
 h=1;
 view=new TLabel(TRect(0,0,lenStr+1,1),aText,link->view,cache);
 linked=link;
}

void TSHzLabel::insert(TDialog *d)
{
 // Why in the hell labels are inserted in x+1?
 TRect r(x-1,y,x-1+xSecond,y+1);
 view->locate(r);
 view->moveTo(x-1,y);
 d->insert(view);
 linked->x=x+xSecond;
 linked->y=y;
 linked->insert(d);
}

void TSHzLabel::setWidth(int aW)
{
 w=aW;
 linked->setWidth(aW-xSecond);
}

void TSHzLabel::setGrowMode(unsigned val)
{
 view->growMode=val;
 linked->setGrowMode(val);
}

