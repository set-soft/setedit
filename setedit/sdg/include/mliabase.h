/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TMLIArrayBase) && !defined(__TMLIArrayBase__)
#define __TMLIArrayBase__
class TLispVar;

class TMLIArrayBase
{
public:
 TMLIArrayBase() {firstfree=0;};
 virtual ~TMLIArrayBase() {};
 virtual void Push(TLispVar *) = 0;
 virtual TLispVar *Get(int ) = 0;
 virtual TLispVar *FreeItems(int ) = 0;
 virtual void ReplaceItem(int , TLispVar *) = 0;
 int GetCount() { return firstfree;};

 int firstfree;
};
#endif
