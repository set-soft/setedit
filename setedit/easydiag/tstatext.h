/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSStaticText) && !defined(__TSStaticText_Defined__)
#define __TSStaticText_Defined__

class TSStaticText : public TSView
{
public:
 TSStaticText(const char *aText);
 TSStaticText(const char *aText, int wrapCol);
};

#endif
