/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TLispSDGstring) && !defined(__TLispSDGstring__)
#define __TLispSDGstring__

class TLispSDGstring : public TLispVar
{
public:
 TLispSDGstring(char *st,char *en,int flgs)
 { start=st; end=en; flags=flgs; type=MLITypeSDGvar; };
 virtual ~TLispSDGstring();
 virtual int print(FILE *s);
 virtual char *toStr();

 int  flags;
 char *start,*end;
};
#endif

