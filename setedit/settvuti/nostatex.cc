/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_string

#define Uses_TNoStaticText
#include <settvuti.h>

TNoStaticText::TNoStaticText(const TRect& bounds, const char *aText) :
    TStaticText(bounds,aText)
{
 startLen=strlen(getText());
}

TNoStaticText::TNoStaticText(const TRect& bounds, const char *aText,
                             stTVIntl *cache) :
    TStaticText(bounds,aText,cache)
{
 startLen=strlen(getText());
}

void TNoStaticText::setText(const char *s)
{
 int l=strlen(s);
 int copy=min(startLen,l);
 memcpy((char *)text,s,copy);
 int rest=startLen-copy;
 if (rest)
    memset((char *)text+copy,' ',rest);
 TVIntl::freeSt(intlText); // Invalidate i18n cache
 draw();
}
