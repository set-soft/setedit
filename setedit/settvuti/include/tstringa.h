/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TStringable) && !defined(TSTRINGA_H_INCLUDED)
#define TSTRINGA_H_INCLUDED
class TStringable
{
public:
 TStringable() {};
 virtual ~TStringable() {};

 virtual void getText(char *dest, unsigned item, int maxLen)=0;
 virtual unsigned GetCount(void) { return Count; };

protected:
 unsigned Count;
};
#endif // TSTRINGA_H_INCLUDED
