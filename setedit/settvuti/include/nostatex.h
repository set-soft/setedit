/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TNoStaticText) && !defined(__TNoStaticText_Defined__)
#define __TNoStaticText_Defined__

class TNoStaticText : public TStaticText
{
public:
 TNoStaticText(const TRect& bounds, const char *aText);
 void setText(char *s);

protected:
 int startLen;
};

#endif
