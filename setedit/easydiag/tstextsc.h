/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSTextScroller) && !defined(__TSTextScroller_Defined__)
#define __TSTextScroller_Defined__

class TSTextScroller : public TSView
{
public:
 TSTextScroller(int width, int height, TNSCollection *str,
                int haveHoriz=0, int haveVert=0, int XLimit=-1);
 virtual void insert(TDialog *);

 int horiz,vert;
 TScrollBar *hS,*vS;
};

#endif
