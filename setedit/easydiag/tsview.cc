/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
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

int TSView::xDefSep=1;
int TSView::yDefSep=1;
