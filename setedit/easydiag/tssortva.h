/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSSortedListBox_Var) && !defined(__TSSortedListBox_Var_Defined__)
#define __TSSortedListBox_Var_Defined__

const int tsslbNo=0,tsslbVertical=1,tsslbHorizontal=2;

#define ListBoxSpecialize(newClass) \
TView *newClass##Make(const TRect& bounds, unsigned aNumCols, \
                      TScrollBar *aHScrollBar, TScrollBar *aVScrollBar); \
class newClass : public TSListBox \
{ \
public: \
 newClass(int width, int height, int scrolls=tsslbNo, int cols=1, int aHSRange=0) : \
   TSListBox(width,height,scrolls,cols,aHSRange,newClass##Make) {}; \
}

#define ListBoxImplement(className) \
TView *TS##className##Make(const TRect& bounds, unsigned aNumCols,\
                           TScrollBar *aHScrollBar, TScrollBar *aVScrollBar)\
{\
 return new T##className(bounds,aNumCols,aHScrollBar,aVScrollBar,True);\
}

#endif
