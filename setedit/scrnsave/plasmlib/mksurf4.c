/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]********************************************************************

 Module: Plasma 4
 Comments:
 That's a ramdom fractal. Uses the surface mixer of @x{Plasma 3}.@p
 The generated surface is very interesting.

***************************************************************************/

#ifdef FAKE_ALLEGRO
#include <fakealle.h>
#else
#include <allegro.h>
#endif
#include "math2.h"

/* I did 2 versions, one optimized and the other generic.
   If you ask for a plasma of width Xopt the optimized routine is used. If
 not a generic one is used.
*/
#define Xopt 576
#define SetPixel(x,y,val)  pl4Scr[x+y*Xopt]=val
#define GetPixel(x,y)      pl4Scr[x+y*Xopt]
#define SetPixelG(x,y,val) _putpixel(Bmp,x,y,val)
#define GetPixelG(x,y)     _getpixel(Bmp,x,y)
static unsigned char *pl4Scr;
static unsigned char  recadj=255;
static BITMAP        *Bmp;
static void (*CB)();
static int cont;
#define CB_CONT 500

/**[txh]**********************************************************************

  Include: mksurf4.h
  Description:
  This fucnction adjust the value of the (xm,ym) pixel. The value to put is
the average of the value of (x1,y1) and (x2,y2) plus a ramdom value. The
ramdom value can be + or -.
@p
  The amount of the random value is limited by recadj. This value depends on
the starting value and the level of recusivity. When the points are closer
the recadj is lower creating a continuous surface.

*****************************************************************************/

static int Adjust(int x1,int y1,int x2,int y2,int xm,int ym)
{
 int val;

 // Pixels average
 val=(GetPixel(x1,y1)+GetPixel(x2,y2))>>1;
 // +/- random value
 val+=(MA2_GetRand8() & recadj)-(recadj>>1);

 if (val<=0)
    val=1;
 else
    if (val>255)
       val=255;

 SetPixel(xm,ym,val);

 return val;
}

/**[txh]********************************************************************

  Description:
  The generic version of @x{Adjust}.

***************************************************************************/
static int AdjustG(int x1,int y1,int x2,int y2,int xm,int ym)
{
 int val;

 // Pixels average
 val=(GetPixelG(x1,y1)+GetPixelG(x2,y2))>>1;
 // +/- random value
 val+=(MA2_GetRand8() & recadj)-(recadj>>1);

 if (val<=0)
    val=1;
 else
    if (val>255)
       val=255;

 SetPixelG(xm,ym,val);

 return val;
}

/**[txh]**********************************************************************

  Description:
  That's the recursive function. It calculates the rectangle (x1,y1)-(x2,y2).
For it the function divides the rect. in four rectangles, calculates one
corner for each and the central point of the rectangle (as an average color
of the four corners) and finally calls to itself to solve each rectangle.

*****************************************************************************/

static void SubDivide(int x1,int y1,int x2,int y2)
{
 int x,y,val,acum;
 signed char oldrecadj;

 // First, check for recursion halts.
 x=(x1+x2)>>1; // X = X center
 y=(y1+y2)>>1; // Y = Y center
 if (x1==x && y1==y)  // That means we reached a central point
    return;

 //  Next, check each of the four sides for empty
 //  and call 'Adjust' if so.  Meanwhile, keep
 //  a running total of all the four sides so we
 //  can do a quick average of them in the next
 //  step.

 // TOP
 val=GetPixel(x,y1);
 if (val==0)
    val=Adjust(x1,y1,x2,y1,x,y1);
 acum=val;

 // RIGHT
 val=GetPixel(x2,y);
 if (val==0)
    val=Adjust(x2,y1,x2,y2,x2,y);
 acum+=val;

 // BOTTOM
 val=GetPixel(x,y2);
 if (val==0)
    val=Adjust(x1,y2,x2,y2,x,y2);
 acum+=val;

 // LEFT
 val=GetPixel(x1,y);
 if (val==0)
    val=Adjust(x1,y1,x1,y2,x1,y);
 acum+=val;

 //  Okay.  All four side pixels have been computed
 //  and plotted on the screen and in memory.  AX
 //  holds the sum of all four pixels.  Divide by four
 //  and set x,y to the resulting value.
 acum>>=2;
 SetPixel(x,y,acum);

 if (!--cont)
   {
    cont=CB_CONT;
    CB();
   }
 // Finally.  All the real work is done.  Now, decrease
 // the recurse-level adjustment and recurse down
 // to the next level for each of the four quads
 oldrecadj=recadj;
 recadj>>=1;
 SubDivide(x1,y1,x,y);
 SubDivide(x,y1,x2,y);
 SubDivide(x,y,x2,y2);
 SubDivide(x1,y,x,y2);
 recadj=oldrecadj;
}

/**[txh]********************************************************************

  Description:
  The generic version of @x{SubDivide}.

***************************************************************************/
static void SubDivideG(int x1,int y1,int x2,int y2)
{
 int x,y,val,acum;
 signed char oldrecadj;

 x=(x1+x2)>>1; // X = X center
 y=(y1+y2)>>1; // Y = Y center
 if (x1==x && y1==y)  // That means we reached a central point
    return;
 val=GetPixelG(x,y1);
 if (val==0)
    val=AdjustG(x1,y1,x2,y1,x,y1);
 acum=val;
 val=GetPixelG(x2,y);
 if (val==0)
    val=AdjustG(x2,y1,x2,y2,x2,y);
 acum+=val;
 val=GetPixelG(x,y2);
 if (val==0)
    val=AdjustG(x1,y2,x2,y2,x,y2);
 acum+=val;
 val=GetPixelG(x1,y);
 if (val==0)
    val=AdjustG(x1,y1,x1,y2,x1,y);
 acum+=val;
 acum>>=2;
 SetPixelG(x,y,acum);
 oldrecadj=recadj;
 recadj>>=1;
 if (!--cont)
   {
    cont=CB_CONT;
    CB();
   }
 SubDivideG(x1,y1,x,y);
 SubDivideG(x,y1,x2,y);
 SubDivideG(x,y,x2,y2);
 SubDivideG(x1,y,x,y2);
 recadj=oldrecadj;
}

/**[txh]********************************************************************

  Description:
  Generates the ramdom surface in the provided BITMAP.@p
  ADJ is the recursion amplitud bit pattern. Use 0xFF, 0x7F or 0x3F for
rougth, medium and smooth surfaces.

***************************************************************************/

int MKS_RamdomFrac1(BITMAP *bmp, unsigned char adj, void (*CallBack)())
{
 int w,h;

 if (!is_linear_bitmap(bmp))
    return 1;
 clear(bmp);
 pl4Scr=bmp->line[0];
 w=bmp->w-1; h=bmp->h-1;
 recadj=adj;
 Bmp=bmp;
 CB=CallBack;
 cont=CB_CONT;

 // Initialize Random table
 MA2_InitRTable();
 if (w==Xopt-1)
   {
    // Set the first corners
    SetPixel(0,0,MA2_GetRand8());
    SetPixel(w,h,MA2_GetRand8());
    SetPixel(0,h,MA2_GetRand8());
    SetPixel(w,0,MA2_GetRand8());
    // Start the recursion
    SubDivide(0,0,w,h);
   }
 else
   {
    // Set the first corners
    SetPixelG(0,0,MA2_GetRand8());
    SetPixelG(w,h,MA2_GetRand8());
    SetPixelG(0,h,MA2_GetRand8());
    SetPixelG(w,0,MA2_GetRand8());
    // Start the recursion
    SubDivideG(0,0,w,h);
   }

 return 0;
}


