/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*****************************************************************************

  Class TGrowDialog

*****************************************************************************/

#if defined(Uses_TGrowDialog) && !defined(__TGrowDialog__)
#define __TGrowDialog__

class TGrowDialog : public TDialog
{
 public:

 TGrowDialog( TRect r, const char *name, int extraOptions=0 ) :
      TWindowInit( &TGrowDialog::initFrame ),
      TDialog(r,name)
 {
  growMode = gfGrowAll;
  flags   |= wfGrow | wfZoom;
  options |= extraOptions;
  minSize = r.b-r.a;
 };
 void sizeLimits(TPoint& min, TPoint& max);

 TPoint minSize;
};

const int gfMoveBottomCorner = gfGrowHiX | gfGrowHiY;
const int gfMoveAccording = gfGrowAll;

#endif // defined(Uses_TGrowDialog) && !defined(__TGrowDialog__)

