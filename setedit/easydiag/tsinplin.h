/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSInputLine) && !defined(__TSInputLine_Defined__)
#define __TSInputLine_Defined__

typedef TInputLine *(* tMakeInputLine)(const TRect& bounds, int aMaxLen);
TInputLine *TSInputLineMake(const TRect& bounds, int aMaxLen);

class TSInputLine : public TSView
{
public:
 TSInputLine(int MaxLen, int haveID, ushort ID, int wForce,
             tMakeInputLine makeIt=TSInputLineMake)
   {fill(MaxLen,haveID,ID,wForce,makeIt); };
 TSInputLine(int MaxLen, int wForce);
 TSInputLine(int MaxLen, ushort ID);
 TSInputLine(int MaxLen);
 void fill(int MaxLen, int haveID, ushort ID, int wForce,
           tMakeInputLine makeIt);
 virtual void insert(TDialog *d);

 THistory *hist;
};

// The class must be called TSxxxx, implementing a Txxxx input line with a
// TSxxxxMake plug-in

#define InputLineSpecialize(className) \
TInputLine *TS##className##Make(const TRect& bounds, int aMaxLen);\
class TS##className : public TSInputLine \
{ \
public: \
 TS##className(int MaxLen, int haveID, ushort ID, int wForce) : \
   TSInputLine(MaxLen,haveID,ID,wForce,TS##className##Make) {}; \
 TS##className(int MaxLen, int wForce) : \
   TSInputLine(MaxLen,0,0,wForce,TS##className##Make) {}; \
 TS##className(int MaxLen, ushort ID) : \
   TSInputLine(MaxLen,1,ID,-1,TS##className##Make) {}; \
 TS##className(int MaxLen) : \
   TSInputLine(MaxLen,0,0,-1,TS##className##Make) {}; \
};

#define InputLineImplement(className) \
TInputLine *TS##className##Make(const TRect& bounds, int aMaxLen)\
{ \
 return new T##className(bounds,aMaxLen);\
}

#endif

