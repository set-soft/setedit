/* Copyright (C) 1997,1998,1999 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]********************************************************************

 Module: Lissajous
 Comments:
 That's a module to calculate smooth trajectories. By now only generates
tables and draw the trajectory.

***************************************************************************/

#include <math.h>
#include <string.h>
#include <allegro.h>
#include "lissa.h"
#include "math1.h"

static Lissajous lstemp;

#define LS_CopyStruct(a,b) memcpy(a,b,sizeof(Lissajous))

/**[txh]********************************************************************

  Description:
  Draws the trajectory to the specified BITMAP with the specified color.@p
  Uses tables for co/sine.

***************************************************************************/

void LST_DrawLissajous(BITMAP *b,Lissajous *l,int color)
{
 int i;

 LS_CopyStruct(&lstemp,l);
 for (i=lstemp.len; i; --i)
    {
     putpixel(b,lstemp.Xc+(int)(lstemp.w*t255sin(lstemp.alpha)),
                lstemp.Yc+(int)(lstemp.h*t255sin(lstemp.beta)), color);
     MA1_IncAng(lstemp.alpha,lstemp.Dalpha);
     MA1_IncAng(lstemp.beta,lstemp.Dbeta);
    }
}

/**[txh]********************************************************************

  Description:
  Draws the trajectory to the specified BITMAP with the specified color.@p
  Uses FPU for co/sine.

***************************************************************************/

void LS_DrawLissajous(BITMAP *b,Lissajous *l,int color)
{
 int i;

 LS_CopyStruct(&lstemp,l);
 for (i=lstemp.len; i; --i)
    {
     putpixel(b,lstemp.Xc+(int)(lstemp.w*sin(lstemp.alpha)),
                lstemp.Yc+(int)(lstemp.h*sin(lstemp.beta)), color);
     lstemp.alpha+=lstemp.Dalpha;
     lstemp.beta+=lstemp.Dbeta;
    }
}

/**[txh]**********************************************************************

  Description:
  This function fills a Lissajous structure with the needed data helping
with the convertions. The parameter len is the number of points to generate.
(x1,y1) (x2,y2) is the rectangle to contain the trajectory. a0 and b0 are
the phases of the angles in degrees, for example suplying 0 and 90 you'll
get sine v.s. cosine. a and b are the relations between the angles, where
eta=b/a. Example a=1 and b=2 => eta=0.5 => alpha goes from 0 to 2*PI and
b from 0 to 4*PI.
@p
  For example a0=0, b0=90, eta=1 is a circle; a0=0, b0=0, eta=0.5 looks like
an eight and a0=0, b0=0, eta=2 as an infinite symbol. Good and complex
trajectories can be achived with 1:3, 1:4 and 1:5.

*****************************************************************************/

void LST_FillDegrees(Lissajous *l, int len, double a0, double b0, int a,
                     int b, int x1, int y1, int x2, int y2)
{
 double eta=(double)b/a;

 l->Xc=(x2+x1)/2;
 l->Yc=(y2+y1)/2;
 l->w=((x2-x1-1)/2)/255.0;
 l->h=((y2-y1-1)/2)/255.0;
 l->alpha=MA1_FromDegrees(a0);
 l->beta=MA1_FromDegrees(b0);
 if (eta>=1)
   {
    l->Dalpha=a*TrigTableSize/(double)len;
    l->Dbeta=l->Dalpha*eta;
   }
 else
   {
    l->Dbeta=b*TrigTableSize/(double)len;
    l->Dalpha=l->Dbeta/eta;
   }
 if (a!=1)
   {
    if (b!=1)
       l->flags=LS_AlphaWrap;
    else
       l->flags=LS_BothWrap;
   }
 else
   {
    if (b!=1)
       l->flags=LS_BetaWrap;
    else
       l->flags=0;
   }

 l->len=len;
}

/**[txh]**********************************************************************

  Description:
  That's the equivalent of @x{LST_FillDegrees} but to use the FPU sin.

*****************************************************************************/

void LS_FillDegrees(Lissajous *l, int len, double a0, double b0, int a, int b,
                    int x1, int y1, int x2, int y2)
{
 double eta=(double)b/a;

 l->Xc=(x2+x1)/2;
 l->Yc=(y2+y1)/2;
 l->w=(x2-x1-1)/2;
 l->h=(y2-y1-1)/2;
 l->alpha=MA1_DegToRad(a0);
 l->beta=MA1_DegToRad(b0);
 l->flags=0;
 if (eta>=1)
   {
    l->Dalpha=a*2*PI/len;
    l->Dbeta=l->Dalpha*eta;
   }
 else
   {
    l->Dbeta=b*2*PI/len;
    l->Dalpha=l->Dbeta/eta;
   }
 l->len=len;
}

void LS_FillRadInc(Lissajous *l, int len, double a0, double b0, double a,
                   double b, int x1, int y1, int x2, int y2)
{
 l->Xc=(x2+x1)/2;
 l->Yc=(y2+y1)/2;
 l->w=(x2-x1-1)/2;
 l->h=(y2-y1-1)/2;
 l->alpha=a0;
 l->beta=b0;
 l->flags=0;
 l->Dalpha=a;
 l->Dbeta=b;
 l->len=len;
}

/**[txh]**********************************************************************

  Description:
  This function fills the array off with the offsets calculated for the
trajectory. The offsets are x+y*w and can be used to move an object in a
linear addressing space. interleave is the value to increment the index in
the array, 1 is the normal, but if you want to store 2 trajectories in the
same array you can use interleave=2.
@p
  That's the look-up table version. Is less accurate than the floating point
but is 58% faster in my machine. Isn't too optimal because is very generic,
a function where the Dalpha and Dbeta values are integer can be MUCH more
faster.
@p
  The idea for this function is to pre-generate the trajectory so speed isn't
the most important. Anyways, I got 340600 points per second, isn't so bad no?

*****************************************************************************/

// 340600 pps with -O2 -fast-math
void LST_FillOffsets(Lissajous *l, int *off, int interleave, int w)
{
 int i,k;

 LS_CopyStruct(&lstemp,l);
 for (k=0,i=lstemp.len; i; k+=interleave,--i)
    {
     off[k]=   lstemp.Xc+(int)(lstemp.w*t255sin(lstemp.alpha))+
            w*(lstemp.Yc+(int)(lstemp.h*t255sin(lstemp.beta)));
     MA1_IncAng(lstemp.alpha,lstemp.Dalpha);
     MA1_IncAng(lstemp.beta,lstemp.Dbeta);
    }
}

/**[txh]********************************************************************

  Description:
  The FPU version of @x{LST_FillOffsets}.

***************************************************************************/
// 192000 points per second -O2
// 215000 pps with -O2 -fast-math
void LS_FillOffsets(Lissajous *l, int *off, int interleave, int w)
{
 int i;

 LS_CopyStruct(&lstemp,l);
 for (i=lstemp.len; i; off+=interleave,--i)
    {
     *off=  lstemp.Xc+(int)(lstemp.w*sin(lstemp.alpha))+
          w*(lstemp.Yc+(int)(lstemp.h*sin(lstemp.beta)));
     lstemp.alpha+=lstemp.Dalpha;
     lstemp.beta+=lstemp.Dbeta;
    }
}

void LS_FillXY(Lissajous *l, int *off, int interleave)
{
 int i;

 interleave<<=1;
 LS_CopyStruct(&lstemp,l);
 for (i=lstemp.len; i; off+=interleave,--i)
    {
     *off=lstemp.Xc+(int)(lstemp.w*sin(lstemp.alpha));
     *(off+1)=lstemp.Yc+(int)(lstemp.h*sin(lstemp.beta));
     lstemp.alpha+=lstemp.Dalpha;
     lstemp.beta+=lstemp.Dbeta;
    }
}

/**[txh]**********************************************************************

  Description:
  This function calculates the next point in a trajectory. Can be used to
calculate trajectories in real time but I think isn't enough fast for it.

*****************************************************************************/

void LST_GetPoint(Lissajous *l, int *x, int *y)
{
 *x=l->Xc+(int)(l->w*t255sin(l->alpha));
 *y=l->Yc+(int)(l->h*t255sin(l->beta));
 MA1_IncAng(l->alpha,l->Dalpha);
 MA1_IncAng(l->beta,l->Dbeta);
}

/**[txh]********************************************************************

  Description:
  The FPU version of @x{LST_GetPoint}.

***************************************************************************/

void LS_GetPoint(Lissajous *l, int *x, int *y)
{
 *x=l->Xc+(int)(l->w*sin(l->alpha));
 *y=l->Yc+(int)(l->h*sin(l->beta));
 l->alpha+=l->Dalpha;
 l->beta+=l->Dbeta;
}
/*
 Code to profile LS?_FillOffsets

 int i=0;
 uclock_t t2,t1,t3,t4;
 #define ITER 500
 off=(int *)malloc(1600);
 LST_FillDegrees(&t,1600,0,0,2,5,0,0,w,h);
 t3=uclock();
 for (i=ITER; i; --i);
 t4=uclock();
 t1=uclock();
 for (i=ITER; i; --i)
     LST_FillOffsets(&t,off,1,w);
 t2=uclock();
 for (i=0; i<1600; i++)
     putpixel(screen,off[i]%w,off[i]/w,11);
 readkey();
 allegro_exit();
 printf("%f\n",1600*ITER/((t2-t1-t4+t3)/(double)UCLOCKS_PER_SEC));
*/
/*
 Code to test LS?_FillDegrees and LS?_DrawLissajous

 int i;
 for (i=0; i<360; i++)
    {
     LST_FillDegrees(&t,1600,i,0,2,5,0,0,w,h);
     LST_DrawLissajous(screen,&t,10);
     vsync();
     vsync();
     LST_DrawLissajous(screen,&t,0);
    }
*/
