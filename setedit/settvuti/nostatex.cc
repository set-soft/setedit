/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_string

#define Uses_TNoStaticText
#include <settvuti.h>

TNoStaticText::TNoStaticText(const TRect& bounds, const char *aText) :
    TStaticText(bounds,aText)
{
 startLen=strlen(aText);
}

void TNoStaticText::setText(char *s)
{
 int l=strlen(s);
 int copy=min(startLen,l);
 memcpy((char *)text,s,copy);
 int rest=startLen-copy;
 if (rest)
    memset((char *)text+copy,' ',rest);
 draw();
}
