/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TMLIArraySimple) && !defined(__TMLIArraySimple__)
#define __TMLIArraySimple__

class TMLIArraySimple : public TMLIArrayBase
{
public:
 TMLIArraySimple(int start, int delt);
 virtual ~TMLIArraySimple();
 virtual void Push(TLispVar *v);
 virtual TLispVar *Get(int pos);
 virtual TLispVar *FreeItems(int cant);
 virtual void ReplaceItem(int pos, TLispVar *o);

private:
 int size;
 int delta;
 int nextsize;
 TLispVar **array;
};
#endif
