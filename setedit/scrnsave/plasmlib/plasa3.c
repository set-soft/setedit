/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]********************************************************************

 Module: Plasma 3
 Comments:
 Plasma 3 is a 2 surfaces mixer. Is based on a plasma by JCL-software
(Jezza).@p
 The plasma uses two times the same surfaces so is suitable only for very
complex surfaces.

***************************************************************************/

#include <stdlib.h>
#include <math.h>
#define NO_BITMAP
#include <string.h>
#include "math1.h"
#include "palette.h"
#include "lissa.h"
#include "mixsurf.h"
#include "mksurf.h"
#include "movsurf2.h"

// Trajectory points
#define pla3Tp 10000
// Colors
#define pla3Co (pla3Tp+240)

static unsigned char  *PLA3_Map =NULL;
static unsigned       *PLA3_Tray=NULL;
static unsigned char  *PLA3_Cols=NULL;

/**[txh]********************************************************************

  Description:
  This function creates the precalculated trajectories.@p
  Uses the Lissajous module. @x{Lissajous}.

***************************************************************************/

void PLA3_MakeTrajectory(int mw, int mh, int sw, int sh)
{
 Lissajous t;
 int w=mw-sw;
 int h=mh-sh;

 // 1:2 is like the infinite symbol
 LS_FillRadInc(&t,pla3Tp,MA1_DegToRad(90),0,1/32.0,1/16.0,0,0,w,h);
 LS_FillOffsets(&t,PLA3_Tray,2,mw);
 // That's a very complex: 24:21
 LS_FillRadInc(&t,pla3Tp,0,MA1_DegToRad(90),1/21.0,1/24.0,0,0,w,h);
 LS_FillOffsets(&t,PLA3_Tray+1,2,mw);
}

// That uses a lot of FPU
void PLA3_CreateSSPal(unsigned char *s)
{
 double y1=0,y2=0,y3=0,y4=0,y5=0,y6=0;
 int i,k;

 memset(s,0,768);
 for (k=768,i=0; i<256*39; i++,k+=3)
    {
     s[k]  =31+(int)(tsin(y1)*tsin(y2)*31);
     s[k+1]=31+(int)(tsin(y3)*tsin(y4)*31);
     s[k+2]=31+(int)(tsin(y5)*tsin(y6)*31);
     MA1_IncAngRad(y1,1/20.0);
     MA1_IncAngRad(y2,1/15.0);
     MA1_IncAngRad(y3,1/35.0);
     MA1_IncAngRad(y4,1/22.0);
     MA1_IncAngRad(y5,1/13.0);
     MA1_IncAngRad(y6,1/30.0);
    }
}

static int deleteSurf;

/**[txh]********************************************************************

  Description:
  This function initialize all the tables, that's: the surface, the
trajectories (2) and the colors.@p
  W,H is the size of the screen.@p
  W2,H2 is the size of the surface.@p
  extSurface can be used to provide an external surface. @x{Plasma 4}.
  Surface selects the surface 1 is the original, 0 one similar.

  Return:
  0 if OK.

***************************************************************************/

int PLA3_InitPlasmaTables(int Surface,int w,int h,unsigned char *buf,
                          int w2,int h2,unsigned char *extSurface,
                          void (*CallBack)(void))
{
 // Parameters for the surface maker
 MS_4.sWw=w>>2;  MS_4.sH=h;
 MS_4.mWw=w2>>2; MS_4.mW=w2;
 MS_4.dest=buf;

 // Allocate memory for surface, tajectory and colors
 if (!extSurface)
   {
    PLA3_Map=(unsigned char *)malloc(w2*h2);
    deleteSurf=1;
   }
 else
   {
    PLA3_Map=extSurface;
    deleteSurf=0;
   }
 PLA3_Tray=(unsigned *)malloc(pla3Tp*sizeof(unsigned)*2);
 PLA3_Cols=(unsigned char *)malloc(pla3Co*3);
 if (!PLA3_Map || !PLA3_Tray || !PLA3_Cols)
    return 1;

 // Create the surface
 if (!extSurface)
    switch (Surface)
      {
       case 0: // Similar to the original
            MKS_SxCxSyCySxy(w2,h2,PLA3_Map,CallBack);
            break;
       default: // Almost the original, with less decimals
            MKS_SxCxSyCySxyCh(w2,h2,PLA3_Map,CallBack);
      }

 // Create the trajectory
 PLA3_MakeTrajectory(w2,h2,w,h);
 // Create the palette
 PLA3_CreateSSPal(PLA3_Cols);

 // Initialize the surface mixer parameters
 MVS_2A.cont=0; MVS_2A.cant=pla3Tp;
 MVS_2A.Tray=PLA3_Tray;
 MVS_2A.s1=MVS_2A.s2=PLA3_Map;

 return 0;
}


/**[txh]********************************************************************

  Description:
  Deallocates the memory used for the tables.

***************************************************************************/

void PLA3_DeInit(void)
{
 if (deleteSurf)
    free(PLA3_Map);
 free(PLA3_Tray);
 free(PLA3_Cols);
 PLA3_Map=NULL;
 PLA3_Tray=NULL;
 PLA3_Cols=NULL;
}


/**[txh]********************************************************************

  Description:
  It animates the palette using the precalculated table.

***************************************************************************/

void PLA3_StepPal(void)
{
 RPF_SetPalRange(PLA3_Cols+MVS_2A.cont*3,1,255);
}