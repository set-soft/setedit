/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
#include "math4.h"
#define NO_BITMAP
#include "mixsurf.h"
#include "movsurf1.h"

// I use structures to avoid name conflicts but they are static, so then
// there are no penalty.

MVS_4struct MVS_4;

void MVS_4SurfSC(void)
{
 MVS_4.c1+=MVS_4.i1;
 MVS_4.c2+=MVS_4.i2;
 MS_4.s1=(unsigned *)(MVS_4.s1+MA4_XSin(MVS_4.c1)+MA4_YCos(MVS_4.c2));
 MVS_4.c3+=MVS_4.i3;
 MVS_4.c4+=MVS_4.i4;
 MS_4.s2=(unsigned *)(MVS_4.s2+MA4_XSin(MVS_4.c3)+MA4_YCos(MVS_4.c4));
 MVS_4.c5+=MVS_4.i5;
 MVS_4.c6+=MVS_4.i6;
 MS_4.s3=(unsigned *)(MVS_4.s3+MA4_XSin(MVS_4.c5)+MA4_YCos(MVS_4.c6));
 MVS_4.c7+=MVS_4.i7;
 MVS_4.c8+=MVS_4.i8;
 MS_4.s4=(unsigned *)(MVS_4.s4+MA4_XSin(MVS_4.c7)+MA4_YCos(MVS_4.c8));
 MS_4add();
}


