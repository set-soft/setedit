/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSHzGroup) && !defined(__TSHzGroup_Defined__)
#define __TSHzGroup_Defined__

class TSHzGroup : public TSView
{
public:
 TSHzGroup(TSView *este, TSView *ant,int sep=1);
 virtual ~TSHzGroup();
 virtual void insert(TDialog *);
 virtual void setWidth(int aW);
 virtual int  howManyHz();
 virtual void setGrowMode(unsigned val);
 int sepa;
 TSView *Este,*Ant;
};

TSHzGroup *MakeHzGroup(TSView *este, TSView *ant, ...);
TSHzGroup *MakeHzGroup(int sepa, TSView *este, TSView *ant, ...);
#endif
