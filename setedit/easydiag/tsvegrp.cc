/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSVeGroup
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>
#include <stdarg.h>

TSVeGroup::~TSVeGroup()
{
 delete Este;
 delete Ant;
};

TSVeGroup::TSVeGroup(TSView *este, TSView *ant, int sep)
{
 sepa=sep;
 h=este->h+ant->h+sep;
 w=max(este->w,ant->w);
 Este=este;
 Ant=ant;
}

void TSVeGroup::insert(TDialog *d)
{
 Este->x=x;
 Este->y=y;
 Este->insert(d);
 Ant->y=y+Este->h+sepa;
 Ant->x=x;
 Ant->insert(d);
}

void TSVeGroup::setWidth(int aW)
{
 w=aW;
 Este->setWidth(aW);
 Ant->setWidth(aW);
}

void TSVeGroup::setGrowMode(unsigned val)
{
 Este->setGrowMode(val);
 Ant->setGrowMode(val);
}

TSVeGroup *MakeVeGroup(TSView *este, TSView *ant, ...)
{
 TSVeGroup *ret;
 TSView *cur;
 va_list arg;
 va_start(arg,ant);

 ret=new TSVeGroup(este,ant);
 while ((cur=va_arg(arg,TSView *))!=0)
   {
    ret=new TSVeGroup(ret,cur);
   }
 va_end(arg);
 return ret;
}

TSVeGroup *MakeVeGroup(int sepa, TSView *este, TSView *ant, ...)
{
 TSVeGroup *ret;
 TSView *cur;
 va_list arg;
 va_start(arg,ant);

 int sep=sepa & (~tsveMakeSameW);
 ret=new TSVeGroup(este,ant,sep);
 while ((cur=va_arg(arg,TSView *))!=0)
   {
    ret=new TSVeGroup(ret,cur,sep);
   }
 va_end(arg);
 if (sepa & tsveMakeSameW)
    ret->makeSameW();
 return ret;
}


