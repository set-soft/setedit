/* Copyright (C) 1997-2001 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]********************************************************************

  Description:
  That's an example on how to use the plasma 1 module. @x{Plasma 1}.
  It test the 320x200 resolution version. Use -s to force a vsync.
  Isn't faster than the generic. At least the fps isn't greater.

***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <time.h>

#include <fakealle.h>
#include "palette.h"
#include "math1.h"
#include "plasa1.h"

RawPal red_palette,blue_palette,temp,temp2,green_palette,BlueP1,BlueP2;
unsigned char *screen_buffer;
BITMAP *BMPAuxScreen;
int Frames=0;
unsigned long t1,t2;
int WaitSync=1;

void setup_palettes(void)
{
 RPF_MakeMultiPal(BlueP2);
 RPF_MakeRedBlueBar(red_palette);
 RPF_MakeBlueGreenBar(blue_palette);
 RPF_MakeGreenRedBar2(green_palette);
 RPF_MakeBlueGreen_RedBars(BlueP1);
}

void setup_lookup_tables(void)
{
 MA1_CreateLookUp();
}

void terminate(void)
{//Sets text mode and ends program.
 double t;

 allegro_exit();
 t2=rawclock();
 t=(t2-t1)/18.2;
 printf("Time: %f Frames: %d => %f fps\n",t,Frames,Frames/t);
 printf(
"Designed and Programmed By Salvador Eduardo Tropea SET. Copyright 1997-1999.\n"
"Original plasma idea By Richard A. Nutman, a.k.a Nutty. Copyright 1997.\n"
"\n"
"There is NO warranty.  You may redistribute this software\n"
"under the terms of the GNU General Public License.\n"
"For more information about these matters, see the files named COPYING.\n"
);
 exit(100);
}

void end_if_user_ready(void)
{
 if (poll_input())
    terminate();
}

#define Blit320() if (WaitSync) \
                     vsync(); \
                  blit(BMPAuxScreen); \
                  Frames++

void PassFromOneToOther(void (*f1)(unsigned char *),
                        void (*f2)(unsigned char *),
                        unsigned char *p1,unsigned char *p2,int len)
{
 int i,f;

 RPF_Negative(p1,temp);
 for (f=0, i=3; f<15; f++,i+=4)
    {
     end_if_user_ready();
     RPF_AddPaletteStep(p1,temp,temp2,i);
     f1(screen_buffer);
     Blit320();
     RPF_SetAllPal(temp2);
    }
 for (f=0, i=63-8; f<7; f++,i-=8)
    {
     end_if_user_ready();
     RPF_FromWhite(p2,temp,i);
     f2(screen_buffer);
     Blit320();
     RPF_SetAllPal(temp);
    }
 RPF_SetAllPal(p2);
 for (i=0; i<len; i++)
    {
     end_if_user_ready();
     f2(screen_buffer);
     Blit320();
    }
}

void PrintHelp(void)
{
 printf(
"\n"
"Plasma 1 test (c) 1997-1999 by Salvador E. Tropea (SET)\n"
"e-mail: salvador@inti.gov.ar\n"
"\n"
"Use: ps1 [-s] [-h] [-i] [-n]\n"
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
"Name:        Plasma 1\n"
"Copyright:   GPL, (c) 1997-1999 by Salvador E. Tropea (SET)\n"
"Author:      Designed and Programmed By Salvador Eduardo Tropea SET.\n"
"             Original plasma idea By Richard A. Nutman, a.k.a Nutty.\n"
"             Copyright 1997.\n"
"Description: A sequence of nice plasma effects based in sin/cos look-up\n"
"             tables.\n"
);
}

int main(int argc, char *argv[])
{
 int i,f;
 int ShowPar=0;

 for (i=1; !ShowPar && i<argc; i++)
    {
     if (argv[i][0]=='-')
       {
        switch (argv[i][1])
          {
           case 's':
                WaitSync=0;
                break;
           case 'h':
                ShowPar=1;
                break;
           case 'i':
                ShowInfo();
                return 2;
           case 'n':
                printf("Plasma sin/cos\n");
                return 3;
          }
       }
    }
 if (ShowPar)
   {
    PrintHelp();
    return 1;
   }

 allegro_init();

 // Virtual screen
 BMPAuxScreen=create_bitmap(320,200);
 screen_buffer=BMPAuxScreen->line[0];

 memset(temp,0,768);
 set_gfx_mode(GFX_VGA,320,200,0,0);
 RPF_SetAllPal(temp);
 setup_palettes();
 setup_lookup_tables();
 t1=rawclock();
 while (1)
   {
    for (i=0; i<64; i++)
       {
        end_if_user_ready();
        RPF_FromBlack(blue_palette,temp,i);
        PLA1_Step320x200_1(screen_buffer);
        Blit320();
        RPF_SetAllPal(temp);
       }
    RPF_SetAllPal(blue_palette);
    for (i=0; i<550; i++)
       {
        end_if_user_ready();
        PLA1_Step320x200_1(screen_buffer);
        Blit320();
       }
    PassFromOneToOther(PLA1_Step320x200_1,PLA1_Step320x200_2,blue_palette,
                       red_palette,500);
    PassFromOneToOther(PLA1_Step320x200_2,PLA1_Step320x200_3,red_palette,
                       green_palette,500);
    PassFromOneToOther(PLA1_Step320x200_3,PLA1_Step320x200_1b,green_palette,
                       blue_palette,500);
    PassFromOneToOther(PLA1_Step320x200_1b,PLA1_Step320x200_2b,blue_palette,
                       red_palette,500);
    PassFromOneToOther(PLA1_Step320x200_2b,PLA1_Step320x200_4,red_palette,
                       green_palette,500);
    PassFromOneToOther(PLA1_Step320x200_4,PLA1_Step320x200_6,green_palette,
                       BlueP2,500);
    PassFromOneToOther(PLA1_Step320x200_6,PLA1_Step320x200_7,BlueP2,BlueP1,500);
    for (i=0,f=63; i<128; i++)
       {
        end_if_user_ready();
        RPF_FromBlack(BlueP1,temp,f);
        if (i & 1)
           f--;
        PLA1_Step320x200_7(screen_buffer);
        Blit320();
        RPF_SetAllPal(temp);
       }
   }
 allegro_exit();

 return 100;
}

