/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSInputLinePiped) && !defined(__TSInputLinePiped_Defined__)
#define __TSInputLinePiped_Defined__

class TSInputLinePiped : public TSView
{
public:
 TSInputLinePiped(int MaxLen, int haveID, ushort ID, int wForce, unsigned flags=0)
   {fill(MaxLen,haveID,ID,wForce,flags); };
 TSInputLinePiped(int MaxLen, int wForce, unsigned flags=0);
 TSInputLinePiped(int MaxLen, ushort ID, unsigned flags=0);
 TSInputLinePiped(int MaxLen, unsigned flags=0);
 void fill(int MaxLen, int haveID, ushort ID, int wForce, unsigned flags);
 virtual void insert(TDialog *d);

 THistory *hist;
};

#endif
