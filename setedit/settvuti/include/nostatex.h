/* Copyright (C) 1996-2005 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TNoStaticText) && !defined(__TNoStaticText_Defined__)
#define __TNoStaticText_Defined__

class TNoStaticText : public TStaticText
{
public:
 TNoStaticText(const TRect& bounds, const char *aText);
 TNoStaticText(const TRect& bounds, const char *aText, stTVIntl *cache);
 void setText(const char *s);
 int  getStartLen() { return startLen; };

protected:
 int startLen;
};

#endif
