/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSLabel) && !defined(__TSLabel_Defined__)
#define __TSLabel_Defined__

class TSLabel : public TSView
{
public:
 TSLabel(const char *aText, TSView *link);
 virtual ~TSLabel() { delete linked; };
 virtual void insert(TDialog *);
 virtual void setWidth(int aW);
 virtual void setGrowMode(unsigned val);
 TSView *linked;
};

class TSHzLabel : public TSView
{
public:
 TSHzLabel(const char *aText, TSView *link, int separation=1);
 virtual ~TSHzLabel() { delete linked; };
 virtual void insert(TDialog *);
 virtual void setWidth(int aW);
 virtual void setGrowMode(unsigned val);
 TSView *linked;
 int xSecond;
};

#endif
