/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
#include <math.h>

#define uchar unsigned char

void MKS_DistCentS(int w, int h, unsigned char *s, int sw, int sh, void (*CallBack)())
{
 int d1,i,j;

 for (i=0; i<h; i++)
    {
     d1=16+(sh-i)*(sh-i);
     for (j=w; j; s++,--j)
         *s=(uchar)((sqrt( d1+(sw-j)*(sw-j) )-4) *5 );
     CallBack();
    }
}

void MKS_SinDisCentS(int w, int h, unsigned char *s, int sw, int sh,
                     double Bsize, void (*CallBack)())
{
 double t2;
 int t1,i,j;

 for (i=0; i<h; i++)
    {
     t1=16+(sh-i)*(sh-i);
     for (j=w; j; s++,--j)
        {
         t2=sqrt(t1+(sw-j)*(sw-j))-4;
         *s=(sin(t2/Bsize)+1)*90;
        }
     CallBack();
    }
}
