/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSViewCol) && !defined(__TSViewCol_Defined__)
#define __TSViewCol_Defined__

typedef struct
{
 TSView *view;
 TSView *refX;
 TSView *refY;
} SViewNode;


class TSViewCol : public TNSCollection
{
public:
 TSViewCol(TDialog *aDialog) : TNSCollection(5,5), d(aDialog) {};
 TSViewCol(const char *name);
 ~TSViewCol() { freeAll(); };
 void insert(int x, int y, TSView *view, TSView *refX=0, TSView *refY=0);
 TDialog *doIt(TDeskTop *dkt=0);
 TDialog *doItCenter(int context=0);
 ushort   exec(void *data, int center=1);
 virtual void freeItem(void *item);

 TDialog *d;
};

#endif
