/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSView
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

/**[txh]********************************************************************

  Description:
  That's the default insert routine for TSView objects. It inserts the
object in the provided TDialog moving the object to the x,y coordinates
solved by the collection and then inserts the object.

***************************************************************************/

void TSView::insert(TDialog *d)
{
 TRect r(x,y,x+w,y+h);
 view->locate(r);
 d->insert(view);
}

/**[txh]********************************************************************

  Description:
  When 2 objects doesn't belong to the same vertical group but they are in
column (special cases of the flow inside the dialog) you make then of the
same width using it.
  
***************************************************************************/

void TSView::makeSameW(TSView *o1, TSView *o2)
{
 if (o1->w>o2->w)
    o2->setWidth(o1->w);
 else if (o1->w<o2->w)
    o1->setWidth(o2->w);
}

int TSView::xDefSep=1;
int TSView::yDefSep=1;

