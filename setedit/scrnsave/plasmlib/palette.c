/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]**********************************************************************

  Module: Palette
  Comments:
  This is used to handle palettes.
@p
  The functions named RPF_* uses the RAW Palette, that's an array of 768
bytes.

*****************************************************************************/

#include "palette.h"
#include <math.h>
#include "math1.h"

/**[txh]**********************************************************************

  Include: palette.h
  Description:
  This function creates a Raw Palette with red, green, blue, yellow and
cyan bars. The color 0 is black to avoid a border around the screen.
@p
  by SET

*****************************************************************************/

void RPF_MakeMultiPal(unsigned char *Pal)
{
 int blue,red,green,mz,i,k;
 for (i=0,k=0,blue=0,mz=0; i<8; i++,blue++,mz+=5)
    {
     Pal[k++] = mz;
     Pal[k++] = mz;//i;
     Pal[k++] = blue;
    }
 for (i=0; i<24; i++,blue++,mz++)
    {
     Pal[k++] = mz;
     Pal[k++] = mz;//i;
     Pal[k++] = blue;
    }
 for (i=0,mz-=2; i<32; i++,blue++,mz-=2)
    {
     Pal[k++] = mz;
     Pal[k++] = mz;//32;
     Pal[k++] = blue;
    }
 for (blue--, green=0, i=0; i<32; i++,blue--,green++)
    {
     Pal[k++] = 0;
     Pal[k++] = green;//32;
     Pal[k++] = blue;
    }
 for (i=0; i<32; i++, blue--, green++)
    {
     Pal[k++] = 0;
     Pal[k++] = green;//31-i;
     Pal[k++] = blue;
    }

 for (i=0, red=0, green--; i<32; i++, red++, green--)
    {
     Pal[k++] = red;
     Pal[k++] = green;//i;
     Pal[k++] = 0;
    }
 for (i=0; i<32; i++, red++, green--)
    {
     Pal[k++] = red;
     Pal[k++] = green;//32;
     Pal[k++] = 0;
    }
 for (red--,i=0,mz=0; i<32; red--,i++,mz+=2)
    {
     Pal[k++] = red;
     Pal[k++] = mz;//32;
     Pal[k++] = mz;
    }
 for (i=0,mz-=2; i<24; i++,red--,mz--)
    {
     Pal[k++] = red;
     Pal[k++] = mz;//31-i;
     Pal[k++] = mz;
    }
 for (i=0; i<8; i++,red--,mz-=5)
    {
     Pal[k++] = red;
     Pal[k++] = mz;//31-i;
     Pal[k++] = mz;
    }
}

/**[txh]**********************************************************************

  Description:
  Creates a red palette with some blue. Red grows from 0 to full in the
first 64 colors, is sustained by 128 and then falls in the last 64. Blue
grows and falls in the middle 128 colors.

*****************************************************************************/

void RPF_MakeRedBlueBar(unsigned char *Pal)
{
 int i, k;
 for (i=0, k=0; i<64; i++)
    {
     Pal[k++] = i;
     Pal[k++] = 0;
     Pal[k++] = 0;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 63;
     Pal[k++] = 0;
     Pal[k++] = i;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 63;
     Pal[k++] = 0;
     Pal[k++] = 63-i;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 63-i;
     Pal[k++] = 0;
     Pal[k++] = 0;
    }
}

/**[txh]**********************************************************************

  Description:
  Creates a blue palette with some green. Blue grows from 0 to full in the
first 64 colors, is sustained by 128 and then falls in the last 64. Green
grows and falls in the middle 128 colors.

*****************************************************************************/

void RPF_MakeBlueGreenBar(unsigned char *Pal)
{
 int i, k;
 for (i=0, k=0; i<64; i++)
    {
     Pal[k++] = 0;
     Pal[k++] = 0;
     Pal[k++] = i;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 0;
     Pal[k++] = i;
     Pal[k++] = 63;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 0;
     Pal[k++] = 63-i;
     Pal[k++] = 63;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 0;
     Pal[k++] = 0;
     Pal[k++] = 63-i;
    }
}

/**[txh]**********************************************************************

  Description:
  Creates a green palette with some red. Green grows from 0 to full in the
first 64 colors, is sustained by 128 and then falls in the last 64. Red
grows and falls in the middle 128 colors.

*****************************************************************************/

void RPF_MakeGreenRedBar(unsigned char *Pal)
{
 int i, k;
 for (i=0, k=0; i<64; i++)
    {
     Pal[k++] = 0;
     Pal[k++] = i;
     Pal[k++] = 0;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = i;
     Pal[k++] = 63;
     Pal[k++] = 0;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 63-i;
     Pal[k++] = 63;
     Pal[k++] = 0;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 0;
     Pal[k++] = 63-i;
     Pal[k++] = 0;
    }
}

/**[txh]**********************************************************************

  Description:
  Creates a green palette with some red. The bar starts going to green and
then goes to yellow before disapear. Green grows from 0 to full in the
first 64 colors, is sustained by 64 and then falls in the last 128. Red
grows from 64 to 128 and then falls like the green.

*****************************************************************************/

void RPF_MakeGreenRedBar2(unsigned char *Pal)
{
 int i,k;
 for (i=0, k=0; i<64; i++)
    {
     Pal[k++] = 0;
     Pal[k++] = i;
     Pal[k++] = 0;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = i;
     Pal[k++] = 63;
     Pal[k++] = 0;
    }
 for (i=0; i<128; i++)
    {
     Pal[k++] = 63-(i/2);
     Pal[k++] = 63-(i/2);
     Pal[k++] = 0;
    }
}

/**[txh]**********************************************************************

  Description:
  Creates 2 bars in the palette. One blue and the other red. Blue grows from
0 to 63 in the first 64 colors, mantains the value in the middle 128 and falls
in the last 64. Green is used to enhance the blue luminance and grows and
falls in the middle 128 colors. Red is used to create a bar with a maximun in
0 (the border is red). Falls in the first 64 and grows in the last 64.

*****************************************************************************/

void RPF_MakeBlueGreen_RedBars(unsigned char *Pal)
{
 int i, k;
 
 for (i=0, k=0; i<64; i++)
    {
     Pal[k++] = 63-i;
     Pal[k++] = 0;
     Pal[k++] = i;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 0;
     Pal[k++] = i;
     Pal[k++] = 63;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 0;
     Pal[k++] = 63-i;
     Pal[k++] = 63;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = i;
     Pal[k++] = 0;
     Pal[k++] = 63-i;
    }
}

/**[txh]********************************************************************

  Description:
  This generates a light blue with white palette.

***************************************************************************/

void RPF_Argentina(unsigned char *Pal)
{
 int i, k;
 int r,v,a;
 
 for (i=0, k=0; i<256; i++)
    {
     if (i<64)
        r=cos(i*PI/128)*63;
     else
        if (i>=192)
           r=sin((i-192)*PI/128)*63;
        else
           r=0;
     if (i>32 && i<224)
        r+=sin((i-32)*PI/192)*55;

     if (i<64)
        //v=63-i;
        v=cos(i*PI/128)*63;
     else
        if (i>=192)
           v=sin((i-192)*PI/128)*63;
           //v=i-192;
        else
           v=0;
     if (i>32 && i<224)
        v+=sin((i-32)*PI/192)*60;

     if (i<64)
        a=cos(i*PI/128)*63;
     else
        if (i>=192)
           a=sin((i-192)*PI/128)*63;
        else
           a=0;
     if (i>32 && i<224)
        a+=sin((i-32)*PI/192)*63;

     Pal[k++] = r;
     Pal[k++] = v;
     Pal[k++] = a;
    }
}

/**[txh]********************************************************************

  Description:
  Generates a gray scale palette, 0 is black, 255 is white.

***************************************************************************/

void RPF_GrayScale(unsigned char *Pal)
{
 int i,j;
 for (i=0,j=0; i<63; i++)
    {
     Pal[j]=Pal[j+1]=Pal[j+2]=Pal[j+4]=Pal[j+5]=Pal[j+8]=Pal[j+9]=i;
     Pal[j+3]=Pal[j+6]=Pal[j+7]=Pal[j+10]=Pal[j+11]=i+1;
     j+=12;
    }
 Pal[j]=Pal[j+1]=Pal[j+2]=Pal[j+4]=Pal[j+5]=Pal[j+8]=Pal[j+9]=
 Pal[j+3]=Pal[j+6]=Pal[j+7]=Pal[j+10]=Pal[j+11]=i;
}

/**[txh]**********************************************************************

  Description:
  Draws 256 vertical lines with the 256 colors in the memory provided by s.

*****************************************************************************/

void RPF_DrawPalette(int w, int h, unsigned char *s)
{
 int x,y;

 for (y=0; y<h; y++)
     for (x=0; x<w; s++,x++)
         if (x<256)
            *s=x;
         else
            *s=0;
}

/**[txh]**********************************************************************

  Description:
  Fades from black to the dest palette. The result is stored in out. The step
can range from 0 to 63.

*****************************************************************************/

void RPF_FromBlack(unsigned char *dest,unsigned char *out,int step)
{
 int j;
 for (j=767; j>=0; j--)
     out[j]=dest[j]*step/63;
}

/**[txh]**********************************************************************

  Description:
  Creates the negative palette of orig.

*****************************************************************************/

void RPF_Negative(unsigned char *orig,unsigned char *out)
{
 int i;
 for (i=767; i>=0; i--)
     out[i]=(63-orig[i]);
}

/**[txh]**********************************************************************

  Description:
  This function adds the inc palette to the orig palette and puts the result
in out. The process is made for the step provided [0-63].

*****************************************************************************/

void RPF_AddPaletteStep(unsigned char *orig,unsigned char *inc,
                        unsigned char *out,int step)
{
 int j;
 for (j=767; j>=0; j--)
     out[j]=orig[j]+(inc[j]*step)/63;
}

/**[txh]**********************************************************************

  Description:
  This function goes from white to a dest palette. The steps are from 63 to
0 in this order.

*****************************************************************************/

void RPF_FromWhite(unsigned char *dest,unsigned char *out,int step)
{
 int j;
 for (j=767; j>=0; j--)
    {
     int aux=dest[j]+step;
     if (aux>63)
        aux=63;
     out[j]=aux;
    }
}

/**[txh]**********************************************************************

  Description:
  This function generates 3 bars in the palette one red the other green and
one blue. The bars overlaps and generates other colors. Is good to generate
a beauty palette animation.
@p
  Uses the math1 module to calculate the cos.

*****************************************************************************/

void RPF_RGBBarsWithCos(unsigned char *Pal)
#if 1
{
 // That's almost 10 times faster than the floating point version
 #define mycol(u,a) ((cos_table[u+a]+256)>>3)
 static int r=MA1_FromDegrees(30),g=MA1_FromDegrees(90),
            b=MA1_FromDegrees(150);
 int i=0,j=0;
 int u;
 while(i<256)
   {
    u=TrigTableSize/256.0*i;
    Pal[j]  =mycol(u,r);
    Pal[j+1]=mycol(u,g);
    Pal[j+2]=mycol(u,b);
    j+=3;
    i++;
   }
 r+=MA1_FromDegrees(3);
 MA1_WrapAngleG(r);
 g-=MA1_FromDegrees(3);
 MA1_WrapAngleB(g);
 b+=MA1_FromDegrees(6);
 MA1_WrapAngleG(b);
}
#else
{
	static double r=1.0/6.0*PI,g=3.0/6.0*PI,b=5.0/6.0*PI;
	int i=0,j=0;
	double u;
	while(i<256)
	{
		u=2*PI/256*i;
//#define mycol(u,a) (max(0.0,cos((u)+(a))))*63 // try this line instead
#define mycol(u,a) (cos((u)+(a))+1)*31
                Pal[j]  =mycol(u,r);
                Pal[j+1]=mycol(u,g);
                Pal[j+2]=mycol(u,b);
                j+=3;
		i++;
	}
	r+=0.05;
	g-=0.05;
	b+=0.1;
}
#endif

/**[txh]********************************************************************

  Description:
  Sets the colors [color,color+cant) in the palette from the _pal_ptr
raw palette.

  Example:
  RPF_SetPalRange(palette,1,255); // All but color 0

***************************************************************************/

void RPF_SetPalRange(unsigned char *_pal_ptr, int color, int cant)
{
 int dummy1,dummy2,dummy3,dummy4;
__asm__ __volatile__("		\n\
     outb %%al,%%dx		\n\
     incl %%edx			\n\
     cli			\n\
     rep			\n\
     outsb			\n\
     sti"
: "=a" (dummy1), "=d" (dummy2), "=S" (dummy3), "=c" (dummy4)
: "c" (cant*3), "S" (_pal_ptr), "a" (color), "d" (0x3C8)
);
}

void RPF_GetAllegroPalette(char *pal)
{
 int dummy1,dummy2;
 __asm__ __volatile__ ("	\n\
         outb %%al,%%dx         \n\
         incl %%edx             \n\
         cli                    \n\
 1:                             \n\
         insb                   \n\
         insb                   \n\
         insb                   \n\
         incl %%edi             \n\
         decl %%ecx             \n\
         jnz  1b                \n\
         sti                    \n\
         "
 : "=a" (dummy1), "=d" (dummy2)
 : "d" (0x3C8), "a" (0), "c" (256), "D" (pal)
 );
}
