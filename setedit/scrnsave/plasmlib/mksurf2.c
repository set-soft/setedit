/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
#include <math.h>
#include "math1.h"

void MKS_C2ySySiCx(int w, int h, unsigned char *s, void (*CallBack)())
{
 int temp;
 int i,j,X,Y;

 temp=30;
 for (Y=0,i=h; i; --i)
    {
     int Yv=cos_table[Y*2]+sin_table[Y]+sin_table[temp];
     for (X=0,j=w; j; s++,--j)
        {
         *s=(Yv+cos_table[X]+1020)>>4;
         MA1_IncAng(X,8);
        }
     MA1_IncAng(Y,6);
     MA1_IncAng(temp,5);
     CallBack();
    }
}

void MKS_SinDisCentSf(int w, int h, unsigned char *s, int sw, int sh,
                      double Bsize, void (*CallBack)())
{
 int temp,i,j,t2;

 Bsize=MA1_FromRad(1/Bsize);
 for (i=0; i<h; i++)
    {
     t2=(sh-i)*(sh-i);
     for (j=w; j; s++,--j)
      {
       temp=Bsize*sqrt(t2+(sw-j)*(sw-j));
       *s=(t255sin(temp%TrigTableSize)+255)>>3;
      }
     CallBack();
    }
}

/**[txh]**********************************************************************

  Description:
  This function calculates the crazy surface for this plasma. The equation
is TOO complex that's because the plasma uses 2 times the same surface so
it MUST be very complex.
@p
  The original program calculated the surface outside the demo. The equation
is: z=64+10*[sin(x/30)+cos(y/46)+cos(x/37)+sin(y/70)+sin((x+y)/43)+
cos(hypot(256-x,150-y)/20)]. Where hypot is sqrt(x^2,y^2).

*****************************************************************************/
// Using tcos and tsin is 30% faster
// Using integer x,y 5.7% more
// Moving the sin(x...)+cos(x...) outside 28% more
// Moving the sin(y...)+cos(y...) outside 25% more
//   With some little things that takes the half of the original, I tried
// to use even more integers but that doesn't help because the lack of
// registers.
/* That's the original code (3.08 s for the total init)

   for (k=0,y=0;y<pla3h;y++)
   for (x=0;x<pla3w;k++,x++)
      {
       PLA3_Map[k]=64+10*( sin(x/30) + cos(y/46) +
         cos(x/37) + sin(y/70) +
         sin((x+y)/43) +
         cos(hypot(256-x,150-y)/20)
         );
      }
*/
void MKS_SxCxSyCySxyCh(int w, int h, unsigned char *s,void (*CallBack)(void))
{
 double x1,y1,x2,y2,xy,xy2;
 double aux1,aux2;
 double *cos_sin_x_t;
 int k;
 int x,y,w2,h2;

 w2=w>>1; h2=h>>1;
 // Precalculate the sin(x/37)+cos(x/30) values (ones and not pla3h times)
 cos_sin_x_t=(double *)alloca((w+1)*sizeof(double));
 if (!cos_sin_x_t)
    return;
 x1=x2=0;
 for (x=w; x; --x)
    {
     cos_sin_x_t[x]=(t255cos(x2)+t255sin(x1))*tsinConst;
     MA1_IncAngRad(x1,1/30.0);
     MA1_IncAngRad(x2,1/37.0);
    }

 CallBack();
 y1=y2=xy=0;
 for (k=0,y=0;y<h;y++)
    {
     xy2=xy;
     aux2=(t255cos(y1)+t255sin(y2))*tsinConst;
     for (x=w; x; k++,--x)
        {
         aux1=cos_sin_x_t[x];
         s[k]=64+(int)(10*(aux1+aux2+tsin(xy2)+cos(hypot(w2-x,h2-y)/20)));
         MA1_IncAngRad(xy2,1/43.0);
        }
     MA1_IncAngRad(y1,1/46.0);
     MA1_IncAngRad(y2,1/70.0);
     MA1_IncAngRad(xy,1/43.0);
     CallBack();
    }
}

/**[txh]********************************************************************

  Description:
  That's an alternative surface.

***************************************************************************/

void MKS_SxCxSyCySxy(int w, int h, unsigned char *s,void (*CallBack)(void))
{
 unsigned x1,y1,x2,y2,xy,xy2;
 double aux1,aux2;
 double *cos_sin_x_t;
 int k;
 int x,y;

 cos_sin_x_t=(double *)alloca((w+1)*sizeof(double));
 if (!cos_sin_x_t)
    return;
 x1=x2=0;
 for (x=w; x; --x)
    {
     cos_sin_x_t[x]=(t255cos(x2)+t255sin(x1))*tsinConst;
     if (!(x&3)) x1++;
     MA1_IncAng(x1,4);
     MA1_IncAng(x2,3);
    }
 CallBack();

 y1=y2=xy=0;
 for (k=0,y=0;y<h;y++)
    {
     xy2=xy;
     aux2=(t255cos(y1)+t255sin(y2))*tsinConst;
     for (x=w; x; k++,--x)
        {
         aux1=cos_sin_x_t[x];
         s[k]=64+(int)(10*(aux1+aux2+tsin(xy2)));
         if (x&1) xy2++;
         MA1_IncAng(xy2,3);
        }
     if (y&1) { y1++; y2++; xy++; }
     MA1_IncAng(y1,2);
     MA1_IncAng(y2,2);
     MA1_IncAng(xy,3);
     CallBack();
    }
}

