/* Copyright (C) 1997-2001 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]********************************************************************

  Description:
  That's an example on how to use the plasma 2 module. @x{Plasma 2}.
  It test the generic resolution version. Use -s to force a vsync.

***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

#include "fakealle.h"
#include "palette.h"
#include "math1.h"
#include "math4.h"
#include "plasa1.h"
#include "movsurf1.h"
#include "varios.h"
#include "putscree.h"

#include <time.h>

RawPal temp,temp2;
unsigned char *screen_buffer;
BITMAP *BMPAuxScreen;
int c,w=320,h=200;
int WaitSync=1;

void terminate(void)
{//Sets text mode and ends program.
 printf("Designed and Programmed By Salvador Eduardo Tropea SET. Copyright 1997-1999.\n"
        "Original plasma idea By Jan MÅller & Erik Hansen.\n\n"
        "There is NO warranty.  You may redistribute this software\n"
        "under the terms of the GNU General Public License.\n"
        "For more information about these matters, see the files named COPYING.\n");
 exit(100);
}

int end_if_user_ready(void)
{
 if (poll_input())
    terminate();
 return 0;
}


void PrintHelp(void)
{
 printf(
"\n"
"Plasma 2 test (c) 1997 by Salvador E. Tropea (SET)\n"
"e-mail: salvador@inti.gov.ar\n"
"\n"
"Use: ps2 [-s] [-h] [-g] [-k float] [-i] [-n]\n"
"\n"
"s: Don't wait for vertical retrace, smooth but you can't meassure the\n"
"   frame rate.\n"
"g: Use the slow algorithm to calculate the surfaces (needed for -k).\n"
"k: Constant for the plasma sine period. Default: 18.334.\n"
"h: Shows this help.\n"
"i: Shows information.\n"
"n: Descriptive name.\n"
);
}


void ShowInfo(void)
{
printf(
"\n"
"Name:        Plasma 2\n"
"Copyright:   GPL, (c) 1997-1999 by Salvador E. Tropea (SET)\n"
"Author:      Designed and Programmed By Salvador Eduardo Tropea SET.\n"
"             Original plasma idea By Jan MÅller & Erik Hansen.\n"
"Description: A sequence of nice plasma effects based the addition of\n"
"             surfaces.\n"
);
}

#define Blit() if (WaitSync) \
                  vsync(); \
               blit(BMPAuxScreen); \
               Frames++

//#define PROFILE
int main(int argc, char *argv[])
{
 int i,Frames=0;
 unsigned long t1=0,t2,t3,t4;
 float Time;
 double k=18.334;
 int ForceGeneric=0;
 int Surface=1,UnoMas;

 for (i=1; i<argc; i++)
    {
     UnoMas=i<argc-1;
     if (argv[i][0]=='-')
       {
        switch (argv[i][1])
          {
           case 's':
                WaitSync=0;
                break;
           case 'g':
                ForceGeneric=1;
                break;
           case 'h':
                PrintHelp();
                return 1;
           case 'i':
                ShowInfo();
                return 2;
           case 'n':
                printf("Plasma mixsurf 1\n");
                return 3;
           case 'F':
                Surface=0;
                break;
           case 'k':
                if (UnoMas)
                   k=atof(argv[++i]);
                break;
          }
       }
    }

 AllegroInit();
 CalcuInit();

 // Virtual screen
 BMPAuxScreen=create_bitmap(w,h);
 screen_buffer=BMPAuxScreen->line[0];

 RPF_MakeBlueGreen_RedBars(temp2);

 if (!ForceGeneric)
   {
    t3=rawclock();
    PLA2_InitPlasmaTables();
    t4=rawclock();
    AllegroSetMode();

    RPF_RGBBarsWithCos(temp);
    RPF_SetAllPal(temp);
    t1=rawclock();
    while (!poll_input())
      {
       for (i=0; i<1000 && !poll_input(); i++)
          {
           PLA2_Step2(screen_buffer);
           RPF_RGBBarsWithCos(temp);
           if (WaitSync)
              vsync();
           PS_FullBlitLinear(BMPAuxScreen,screen);
           Frames++;
           RPF_SetAllPal(temp);
          }
       if (i<1000) break;
       RPF_SetAllPal(temp2);
       for (i=0; i<1000 && !poll_input(); i++)
          {
           PLA2_Step2(screen_buffer);
           if (WaitSync)
              vsync();
           PS_FullBlitLinear(BMPAuxScreen,screen);
           Frames++;
          }
       if (i<1000) break;
      }
   }
 else
   {
    t3=rawclock();
    PLA2G_InitPlasmaTables(w,h,screen_buffer,k,Surface,w*2,h*2,CalcuCallBack);
    t4=rawclock();
    AllegroSetMode();
    RPF_RGBBarsWithCos(temp);
    RPF_SetAllPal(temp);
    t1=rawclock();
    while (!poll_input())
      {
       for (i=0; i<1000 && !poll_input(); i++)
          {
           MVS_4SurfSC();
           RPF_RGBBarsWithCos(temp);
           Blit();
           RPF_SetAllPal(temp);
          }
       if (i<1000) break;
       RPF_SetAllPal(temp2);
       for (i=0; i<1000 && !poll_input(); i++)
          {
           MVS_4SurfSC();
           Blit();
          }
       if (i<1000) break;
      }
   }
 t2=rawclock();
 PLA2G_DeInit();
 allegro_exit();

 Time=(t2-t1)/18.2;
 printf("Time: %f Frames: %d => %f fps.\n",Time,Frames,Frames/Time);
 Time=(t4-t3)/18.2;
 printf("Initialization time: %f.\n",Time);

 terminate();
 return 0;
}

