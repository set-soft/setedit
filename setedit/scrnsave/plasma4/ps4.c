/* Copyright (C) 1997-2001 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]********************************************************************

  Description:
  That's an example on how to use the plasma 4 module. @x{Plasma 4}.
  It test the generic resolution version. Use -s to force a vsync.

***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <time.h>

#include "fakealle.h"
#include "palette.h"
#include "math1.h"
#include "mksurf4.h"
#include "plasa1.h"
#include "lissa.h"
#include "varios.h"
#include "movsurf2.h"

#include <time.h>

RawPal temp,temp2;
unsigned char *screen_buffer;
BITMAP *BMPAuxScreen,*BMPMap;
int c,w=320,h=200;

void terminate(void)
{//Sets text mode and ends program.
 printf("Designed and Programmed By Salvador Eduardo Tropea SET. Copyright 1997-1999.\n"
        "Original plasma idea By Tom Dibble. Copyright 1994.\n\n"
        "There is NO warranty.  You may redistribute this software\n"
        "under the terms of the GNU General Public License.\n"
        "For more information about these matters, see the files named COPYING.\n");
 exit(100);
}

void end_if_user_ready(void)
{
 if (keypressed())
    terminate();
}

void PrintHelp(void)
{
 printf(
"\n"
"Plasma 4 test (c) 1997 by Salvador E. Tropea (SET)\n"
"e-mail: salvador@inti.gov.ar\n"
"\n"
"Use: ps4 [-s] [-h] [-t 0|1|3] [-i] [-n]\n"
"\n"
"s: Don't wait for vertical retrace, smooth but you can't meassure the\n"
"   frame rate.\n"
"h: Shows this help.\n"
"t: Sets the amplitud of the start recursion. 0 is smooth and 3 rought.\n"
"   Default: 3.\n"
"i: Shows information.\n"
"n: Descriptive name.\n"
"\n"
);
}

void ShowInfo(void)
{
printf(
"\n"
"Name:        Plasma 4\n"
"Copyright:   GPL, (c) 1997-1999 by Salvador E. Tropea (SET)\n"
"Author:      Designed and Programmed By Salvador Eduardo Tropea SET.\n"
"             Original plasma idea By Tom Dibble. Copyright 1994.\n"
"Description: A sequence of nice plasma effects based the addition of\n"
"             surfaces. The surfaces are randomly generated.\n"
);
}



#define Blit() blit(BMPAuxScreen)

int main(int argc, char *argv[])
{
 int w2,h2;
 unsigned long t1,t2,t1a,t2a;
 int i;
 int UnoMas,MaskInit=255;
 int WaitSync=1,Frames=0,tipo,count;
 double t;

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
           case 'h':
                PrintHelp();
                return 1;
           case 'i':
                ShowInfo();
                return 2;
           case 'n':
                printf("Plasma mixsurf random\n");
                return 3;
           case 't':
                if (UnoMas)
                  {
                   tipo=atoi(argv[++i]);
                   if (!(tipo!=0 && tipo!=1 && tipo!=3))
                      MaskInit=(tipo<<6) | 0x3F;
                  }
                break;
          }
       }
    }

 AllegroInit();
 CalcuInit();

 // Multiplo de 4!
 w2=(int)(w*1.8) & 0xFFFFFFFC; h2=(int)(h*1.5) & 0xFFFFFFFC;
 // Virtual screen
 BMPAuxScreen=create_bitmap(w,h);
 screen_buffer=BMPAuxScreen->line[0];
 // Plasma Map
 BMPMap=create_bitmap(w2,h2);
 // Generate the plasma random surface
 t1a=rawclock();
 MKS_RamdomFrac1(BMPMap,MaskInit,CalcuCallBack);
 t2a=rawclock();

 AllegroSetMode();

 // Pass it to the plasma 3 engine
 PLA3_InitPlasmaTables(1,w,h,screen_buffer,w2,h2,BMPMap->line[0],CalcuCallBack);

 t1=rawclock();
 while (!keypressed())
   {
    count=0;
    while (!keypressed() && count++<1400)
      {
       MVS_2SurfA();
       if (WaitSync)
          vsync();
       PLA3_StepPal();
       Blit();
       Frames++;
      }
    if (count<1400) break;
    //readkey();
    RPF_MakeBlueGreen_RedBars(temp);
    RPF_SetAllPal(temp);
    count=0;
    while (!keypressed() && count++<1400)
      {
       MVS_2SurfA();
       if (WaitSync)
          vsync();
       Blit();
       Frames++;
      }
    if (count<1400) break;
   }
 t2=rawclock();

 allegro_exit();
 PLA3_DeInit();

 t=(t2-t1)/18.2;
 printf("Time: %f Frames: %d => %f fps\n",t,Frames,Frames/t);
 t=(t2a-t1a)/18.2;
 printf("Initialization: %f s\n",t);
 terminate();
 return 0;
}

