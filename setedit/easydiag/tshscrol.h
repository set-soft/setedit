/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSHScrollBar) && !defined(__TSHScrollBar_Defined__)
#define __TSHScrollBar_Defined__

class TSHScrollBar : public TSView
{
public:
 TSHScrollBar(int width, int max=100);
 void setValue(int value) { sb->setValue(value); };
 void setMax(int max);

 TScrollBar *sb;
};

#endif
