/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*------------------------------------------------------------*/
/* filename -       sinf.cc                                   */
/*                                                            */
/* Registration objects for the following classes:            */
/*                      TInfViewer                            */
/*                      TInfWindow                            */
/*                      THisCollection                        */
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
/*                                                            */
/*  Needed to write the objects to a stream.                  */
/*                                                            */
/*------------------------------------------------------------*/

#include <tvsetuti.h>
#define Uses_TStreamableClass
// InfView requests
#include <infr.h>
#include <settvuti.h>
#include <inf.h>

__link( RView )
__link( RWindow )
__link( RScrollBar )
__link( RScroller )
__link( RNoCaseStringCollection )

s(InfViewer);
s(InfWindow);
s(HisCollection);

