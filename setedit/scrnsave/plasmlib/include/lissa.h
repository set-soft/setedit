/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifdef __cplusplus
extern "C" {
#endif
typedef struct
{
 int Xc,Yc; // Center
 double w,h;   // Size
 double Dalpha; // X angle increment
 double Dbeta;  // Y angle increment
 double alpha,beta; // Actual angle
 int len;       // Number of points to calculate
 int flags;     // By now just what angle needs wrap
} Lissajous;

#define LS_BetaWrap  1
#define LS_AlphaWrap 2
#define LS_BothWrap  4

void LS_GetPoint(Lissajous *l, int *x, int *y);
void LST_GetPoint(Lissajous *l, int *x, int *y);
void LS_FillOffsets(Lissajous *l, int *off, int interleave, int w);
void LST_FillOffsets(Lissajous *l, int *off, int interleave, int w);
void LS_FillDegrees(Lissajous *l, int len, double a0, double b0, int a, int b,
                    int x1, int y1, int x2, int y2);
void LST_FillDegrees(Lissajous *l, int len, double a0, double b0, int a,
                     int b, int x1, int y1, int x2, int y2);
#ifndef NO_BITMAP
void LS_DrawLissajous(BITMAP *b,Lissajous *l,int color);
void LST_DrawLissajous(BITMAP *b,Lissajous *l,int color);
#endif
void LS_FillRadInc(Lissajous *l, int len, double a0, double b0, double a,
                   double b, int x1, int y1, int x2, int y2);
void LS_FillXY(Lissajous *l, int *off, int interleave);
#ifdef __cplusplus
}
#endif

