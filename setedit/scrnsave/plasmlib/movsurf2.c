/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
#define NO_BITMAP
#include "movsurf2.h"
#include "mixsurf.h"

MVS_2Astruct MVS_2A;

void MVS_2SurfA(void)
{
 MS_4.s1=(unsigned *)(MVS_2A.s1+MVS_2A.Tray[MVS_2A.cont*2]);
 MS_4.s2=(unsigned *)(MVS_2A.s2+MVS_2A.Tray[MVS_2A.cont*2+1]);
 MS_2addC();
 if (++MVS_2A.cont==MVS_2A.cant)
    MVS_2A.cont=0;
}
