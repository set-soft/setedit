/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSLabelRadio
#define Uses_TSRadioButtons
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>
#include <stdarg.h>

TSLabel *TSLabelRadio(const char *name, ...)
{
 const char *s;
 va_list arg;
 va_start(arg,name);

 TSItem *first=0,*last=0,*aux;
 while ((s=va_arg(arg,const char *))!=0)
   {
    aux=new TSItem(s,0);
    if (!first)
       first=aux;
    if (last)
       last->next=aux;
    last=aux;
   }
 va_end(arg);
 return new TSLabel(name,new TSRadioButtons(first));
}

TSLabel *TSLabelRadio(int columns, const char *name, ...)
{
 const char *s;
 va_list arg;
 va_start(arg,name);

 TSItem *first=0,*last=0,*aux;
 while ((s=va_arg(arg,const char *))!=0)
   {
    aux=new TSItem(s,0);
    if (!first)
       first=aux;
    if (last)
       last->next=aux;
    last=aux;
   }
 va_end(arg);
 return new TSLabel(name,new TSRadioButtons(first,-1,columns));
}

