/* Copyright (C) 1997-2001 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]********************************************************************

  Description:
  That's an example on how to use the plasma 3 module. @x{Plasma 3}.
  It test the generic resolution version. Use -s to force a vsync.

***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

#include "fakealle.h"
#include "palette.h"
#include "math1.h"
#include "plasa1.h"
#include "movsurf2.h"
#include "varios.h"

#include <time.h>

RawPal temp,temp2;
unsigned char *screen_buffer;
BITMAP *BMPAuxScreen;
int c,w=320,h=200;

void terminate(void)
{//Sets text mode and ends program.
 printf("Designed and Programmed By Salvador Eduardo Tropea SET. Copyright 1997-1999.\n"
        "Original plasma idea By JCL-software (Jezza). Copyright 1994.\n\n"
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
"Plasma 3 test (c) 1997-1999 by Salvador E. Tropea (SET)\n"
"e-mail: salvador@inti.gov.ar\n"
"\n"
"Use: ps3 [-s] [-h] [-i] [-n]\n"
"\n"
"s: Don't wait for vertical retrace, smooth but you can't meassure the\n"
"   frame rate.\n"
"h: Shows this help.\n"
"i: Shows information.\n"
"n: Descriptive name.\n"
);
}

void ShowInfo(void)
{
printf(
"\n"
"Name:        Plasma 3\n"
"Copyright:   GPL, (c) 1997-1999 by Salvador E. Tropea (SET)\n"
"Author:      Designed and Programmed By Salvador Eduardo Tropea SET.\n"
"             Original plasma idea By JCL-software (Jezza). (c) 1994.\n"
"Description: A sequence of nice plasma effects based the addition of\n"
"             surfaces.\n"
);
}

#define Blit() blit(BMPAuxScreen)

int main(int argc, char *argv[])
{
 unsigned long t1a,t2a;
 int i;
 int ErrorPar=0;
 unsigned long t1,t2;
 int WaitSync=1,Frames=0;
 double t;

 for (i=1; !ErrorPar && i<argc; i++)
    {
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
                printf("Plasma mixsurf 2\n");
                return 3;
          }
       }
    }

 AllegroInit();
 CalcuInit();

 // Virtual screen
 BMPAuxScreen=create_bitmap(w,h);
 screen_buffer=BMPAuxScreen->line[0];

 // 1.3Mb of table for 800x600!
 t1a=rawclock();
 PLA3_InitPlasmaTables(1,w,h,screen_buffer,(int)(w*1.8) & 0xFFFFFFFC,
                       (int)(h*1.5) & 0xFFFFFFFC,NULL,CalcuCallBack);
 t2a=rawclock();

 AllegroSetMode();

 t1=rawclock();
 while (!keypressed())
   {
    MVS_2SurfA();
    if (WaitSync)
       vsync();
    PLA3_StepPal();
    Blit();
    Frames++;
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

