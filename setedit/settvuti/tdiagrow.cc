/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]**********************************************************************

  Class: TGrowDialog
  Comments:

  This is a TDialog that can grow, not shrink, simply grow. You can Zoom
the dialog too.
@p
I defined 2 gfXXXXX constants:
@p
gfMoveBottomCorner: Used in TListBoxes, the uper-left corner still in
your place and the bottom-right is moved according to the size of the dialog.@p

gfMoveAccording: Used in the TButtons, the buttons are moved according
to the new size of the dialog.@p

The TScrollBars have grow attributes by default.@p

*****************************************************************************/

#define Uses_TGrowDialog
#include <settvuti.h>

/**[txh]**********************************************************************

 Include: tdiagrow
 Module: SET TV Utils
 Description:
 It overwrites the original sizeLimits to return the original size of the
dialog instead of the minimun size. In this way the dialog can't be shrinked.

*****************************************************************************/

void TGrowDialog::sizeLimits(TPoint& min, TPoint& max)
{
 TDialog::sizeLimits(min,max);
 min=minSize;
}

/**[txh]**********************************************************************

 Function: TGrowDialog
 Prototype: TGrowDialog( TRect r, const char *name, int extraOptions=0 )
 Description:
 Creates a dialog that can be enlarged. "r" is the initial and minimun size,
"name" is the caption name and "extraOptions" is ored with the options of
the dialog object.

 It calls to the dialog constructor and sets the flags to allow the growing
facility.

*****************************************************************************/

