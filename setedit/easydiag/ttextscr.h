/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TTextScroller) && !defined(__TTextScroller_Defined__)
#define __TTextScroller_Defined__

class TTextScroller : public TScroller
{
public:
 TTextScroller(const TRect& bounds, TScrollBar *aHScrollBar,
               TScrollBar *aVScrollBar, TNSCollection *str);
 virtual void draw();
 virtual TPalette& getPalette() const;
 virtual void handleEvent(TEvent& event);

private:
 TNSCollection *text;
};

#endif
