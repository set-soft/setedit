/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSListBox) && !defined(__TSListBox_Defined__)
#define __TSListBox_Defined__

typedef TView *(* tMakeListBox)(const TRect& bounds, unsigned aNumCols,
                TScrollBar *aHScrollBar, TScrollBar *aVScrollBar);
TView *TSListBoxMakeListBox(const TRect& bounds, unsigned aNumCols,
                            TScrollBar *aHScrollBar, TScrollBar *aVScrollBar);

/*****************************************************************************

  Important note: The last parameter helps to specialize this class. In this
  way creating a variant of TSListBox is a 2 lines job. That's what
  TSSortedListBox, TSSOSSortedListBox and TSStringableListBox does in fact.
    The solution is really *nasty*. I don't like it at all, but after trying
  to do it using virtual members I gave up. Looks like that's impossible
  because I need to override the behavior of part of the constructor and
  inside the constructor we can't call virtual members of a child class, the
  this variable points to our virtual table and not the child's one.

*****************************************************************************/

class TSListBox : public TSView
{
public:
 TSListBox(int width, int height, int scrolls=tsslbNo, int cols=1, int aHSRange=0,
           tMakeListBox MakeListBox=TSListBoxMakeListBox);
 virtual void insert(TDialog *);

 int scrollType;
 TScrollBar *vScrollBar;
 TScrollBar *hScrollBar;
};

#endif
