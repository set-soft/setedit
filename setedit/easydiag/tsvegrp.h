/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSVeGroup) && !defined(__TSVeGroup_Defined__)
#define __TSVeGroup_Defined__

class TSVeGroup : public TSView
{
public:
 TSVeGroup(TSView *este, TSView *ant,int sep=1);
 virtual ~TSVeGroup();
 virtual void insert(TDialog *);
 virtual void setWidth(int aW);
 void makeSameW() { setWidth(w); };
 virtual void setGrowMode(unsigned val);
 int sepa;
 TSView *Este,*Ant;
};

const int tsveMakeSameW=0x8000;
TSVeGroup *MakeVeGroup(TSView *este, TSView *ant, ...);
TSVeGroup *MakeVeGroup(int sepa, TSView *este, TSView *ant, ...);
#endif
