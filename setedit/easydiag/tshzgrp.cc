/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSHzGroup
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>
#include <stdarg.h>


TSHzGroup::~TSHzGroup()
{
 delete Este;
 delete Ant;
};

TSHzGroup::TSHzGroup(TSView *este, TSView *ant, int sep)
{
 sepa=sep;
 w=este->w+ant->w+sep;
 h=max(este->h,ant->h);
 Este=este;
 Ant=ant;
 view=Este->view; // Just in case somebody tries it.
}

void TSHzGroup::insert(TDialog *d)
{
 Este->x=x;
 Este->y=y;
 Este->insert(d);
 Ant->x=x+Este->w+sepa;
 Ant->y=y;
 Ant->insert(d);
}

void TSHzGroup::setWidth(int aW)
{
 int cant=howManyHz();
 int extra=aW-w;
 if (extra<=0)
    return;
 w=aW;
 int forFirst=(int)(((double)extra/cant)*Este->howManyHz()+0.5);
 Este->setWidth(Este->w+forFirst);
 Ant->setWidth(Ant->w+extra-forFirst);
}

int TSHzGroup::howManyHz()
{
 return Este->howManyHz()+Ant->howManyHz();
}

void TSHzGroup::setGrowMode(unsigned val)
{
 Este->setGrowMode(val);
 Ant->setGrowMode(val);
}


TSHzGroup *MakeHzGroup(TSView *este, TSView *ant, ...)
{
 TSHzGroup *ret;
 TSView *cur;
 va_list arg;
 va_start(arg,ant);

 ret=new TSHzGroup(este,ant);
 while ((cur=va_arg(arg,TSView *))!=0)
   {
    ret=new TSHzGroup(ret,cur);
   }
 va_end(arg);
 return ret;
}

TSHzGroup *MakeHzGroup(int sepa, TSView *este, TSView *ant, ...)
{
 TSHzGroup *ret;
 TSView *cur;
 va_list arg;
 va_start(arg,ant);

 ret=new TSHzGroup(este,ant,sepa);
 while ((cur=va_arg(arg,TSView *))!=0)
   {
    ret=new TSHzGroup(ret,cur,sepa);
   }
 va_end(arg);
 return ret;
}
