/* Copyright (C) 2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TSSlider) && !defined(TSSlider_Defined)
#define TSSlider_Defined

class TScrollBarCB;

typedef void (*TScrollBarCBfunc)(int value, TScrollBarCB *obj, void *data);

class TScrollBarCB : public TScrollBar
{
public:
 TScrollBarCB(const TRect &rect, TScrollBarCBfunc cb, void *aData) :
   TScrollBar(rect), callBack(cb), data(aData) {};

 virtual void scrollDraw();
 TScrollBarCBfunc callBack;
 void *data;
};

class TSSlider;

typedef struct
{
 int side; // 0=left, 1=right
 TSSlider *obj;
} stTSSliderInfo;

class TSSlider : public TSView
{
public:
 TSSlider(int height, Boolean stereo, const char *label, TScrollBarCBfunc aLcb=0,
          void *aLdata=0, TScrollBarCBfunc aRcb=0, void *aRdata=0,
          int *Join=0, int max=100);
 void Set(int valueLeft, int valueRight=0);
 virtual void insert(TDialog *d);

 TScrollBarCB *left,*right;
 TStaticText *label;
 void *ldata,*rdata;
 TScrollBarCBfunc lcb,rcb;
 stTSSliderInfo stLeft,stRight;
 int *join;
 int max;
};

#endif
