/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#include <stdlib.h>
#define Uses_TLispVar
#define Uses_TMLIArraySimple
#include <mli.h>

TMLIArraySimple::TMLIArraySimple(int start, int delt)
{
 array=0;
 delta=delt;
 size=0;
 nextsize=start;
}

TMLIArraySimple::~TMLIArraySimple()
{
 int i;
 for (i=0; i<firstfree; i++)
     destroyVar(array[i]);
 free(array);
 array=0;
}

void TMLIArraySimple::Push(TLispVar *v)
{
 if (firstfree>=size)
   {
    array=(TLispVar **)realloc(array,nextsize*sizeof(TLispVar *));
    size=nextsize;
    nextsize+=delta;
   }
 if (array)
    array[firstfree++]=v;
}

TLispVar *TMLIArraySimple::Get(int pos)
{
 if (pos>=firstfree || pos<0)
    return NULL;
 return array[pos];
}

TLispVar *TMLIArraySimple::FreeItems(int cant)
{
 int h,i;

 if (cant>firstfree-1)
    cant=firstfree-1;
 h=firstfree-cant;
 for (i=h; i<firstfree; i++)
     destroyVar(array[i]);
 firstfree=h-1;
 if (array[firstfree])
    array[firstfree]->type|=1;
 return array[firstfree];
}

void TMLIArraySimple::ReplaceItem(int pos, TLispVar *o)
{
 if (pos<firstfree)
   {
    destroyVar(array[pos]);
    array[pos]=o;
   }
}
