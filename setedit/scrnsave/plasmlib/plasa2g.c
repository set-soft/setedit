/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]********************************************************************

 Module: Plasma 2G
 Comments:
 That's the generic version of the plasma 2 module. @x{Plasma 2}.@p
 Supports any resolution and is very optimized. Perhaps no so fast as the
PLA2_Step3 (@x{PLA2_Step3}), but almost as fast and more accuracy.@p

***************************************************************************/

#include <stdlib.h>
#include <math.h>
#define NO_BITMAP
#include "math1.h"
#include "math4.h"
#include "mixsurf.h"
#include "mksurf.h"
#include "movsurf1.h"

#define uchar unsigned char

static unsigned char *tab1,*tab2;

/**[txh]********************************************************************

  Description:
  Calculates the surfaces for the plasma.@p
  SURF1 selects the surface. Use 1 to get the same surfaces used in the
320x200 version. @x{PLA2G_InitPlasmaTables}. Use 0 for surfaces that are
calculated much more faster.@p
  BSIZE sets the size of the sine period. Use values from 2 to 80 with
decimals.@p
  W and H sets the size of the screen, the surfaces are twice greater be
careful in SVGA modes (various Mb).

  Return:
  0 if OK, 1 if no memory.

***************************************************************************/

int PLA2G_InitPlasmaTables(int sW, int sH, unsigned char *buf, double Bsize,
                           int Surf1, int mW, int mH, void (*CallBack)(void))
{
 unsigned char *s;
 int tX,tY;

 tX=sW/2; tY=sH/2;
 MS_4.sWw=sW/4; MS_4.sH=sH; MS_4.mWw=mW/4; MS_4.mW=mW;
 MS_4.dest=buf;

 // be carefull, 1024*768 => 2048*1536=3.14 Mb => 4Mb so you'll need
 // 8 Mb for the map and 1 Mb for the virtual screen. That's HUGE and
 // the map calculation will take a LOT of time, so don't exagerate.
 tab1=(unsigned char *)malloc(mW*mH);
 tab2=(unsigned char *)malloc(mW*mH);
 if (!tab1 || !tab2)
    return 1;

 // The table is 2x the screen to move the circles
 s=tab1;
 switch (Surf1)
   {
    case 0:
         MKS_C2ySySiCx(mW,mH,s,CallBack);
         break;
    default:
         MKS_DistCentS(mW,mH,s,sW,sH,CallBack);
   }

 s=tab2;
 switch (Surf1)
   {
    case 0:
         MKS_SinDisCentSf(mW,mH,s,sW,sH,Bsize,CallBack);
         break;
    default:
         MKS_SinDisCentS(mW,mH,s,sW,sH,Bsize,CallBack);
         break;
   }
#ifdef DWORD_ALIGN
 MS_InvertDWordScreen(mW,mH,tab1);
 MS_InvertDWordScreen(mW,mH,tab2);
 MA4_FillTableIW(tX-1,tX,0,MA4NoScale | MA4XSin);
#else
 MA4_FillTableI(tX-1,tX,0,MA4NoScale | MA4XSin);
#endif
 MA4_FillTableI(tY-1,tY,mW,MA4YSin);

 /* Initialize the trajectory parameters
    Don't worry about the function call, they are solved in compile time */
 MVS_4.c1=0; MVS_4.c2=0;
 MVS_4.i1=MA4_f2fix(MA4_FromRad(0.085/6));
 MVS_4.i2=MA4_f2fix(MA4_FromRad(0.1/6));
 MVS_4.s1=tab1;

 // Invert the sin and cos
 MVS_4.c3=MA4_i2fix(MA4_FromDeg(90)); MVS_4.c4=MA4_i2fix(MA4_FromDeg(270));
 MVS_4.i3=MA4_f2fix(MA4_FromRad(0.3/6));
 MVS_4.i4=-MA4_f2fix(MA4_FromRad(0.2/6));
 MVS_4.s2=tab2;

 MVS_4.c5=MA4_i2fix(MA4_FromDeg(90)); MVS_4.c6=MA4_i2fix(MA4_FromDeg(270));
 MVS_4.i5=MA4_f2fix(MA4_FromRad(0.4/6));
 MVS_4.i6=-MA4_f2fix(MA4_FromRad(0.15/6));
 MVS_4.s3=tab2;

 MVS_4.c7=MA4_i2fix(MA4_FromDeg(90)); MVS_4.c8=MA4_i2fix(MA4_FromDeg(270));
 MVS_4.i7=MA4_f2fix(MA4_FromRad(0.35/6));
 MVS_4.i8=-MA4_f2fix(MA4_FromRad(0.05/6));
 MVS_4.s4=tab2;

 return 0;
}


/**[txh]********************************************************************

  Description:
  Deallocates the memory used for the tables.

***************************************************************************/

void PLA2G_DeInit(void)
{
 free(tab1);
 free(tab2);
}




