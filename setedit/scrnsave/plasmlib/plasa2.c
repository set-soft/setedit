/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]********************************************************************

 Module: Plasma 2
 Comments:
 That's a plasma that uses 4 "circles" (3D cones). Each circle is moved
without syncro with the others, the addition of the 4 circles gives the
plasma.@p
 The original code was by Jan M›ller & Erik Hansen.@p
 Look in the plasma 2g module for a generic version. @x{Plasma 2G}.

***************************************************************************/

#include <stdlib.h>
#include <math.h>
#include "math1.h"

#define pla2w 320
#define pla2h 200
#define pla2w2 (pla2w*2)
#define pla2h2 (pla2h*2)
#define uchar unsigned char

static unsigned char *tab1,*tab2;

/**[txh]********************************************************************

  Include: plasa1.h
  Description:
  It creates the two surfaces used for the plasma. They have w=2*screen
width and h=2*screen height.@p
  The equations used are:@*
{sqrt[16.0+(Y-Ycenter)^2+(X-Xcenter)^2]-4}*5@*
90*(sin{sqrt[16.0+(Y-Ycenter)^2+(X-Xcenter)^2]-4}+1)@p
  That's the 320x200 version. @x{PLA2G_InitPlasmaTables}.

  Return:
  0 if all OK, 1 if not memory.

***************************************************************************/

int PLA2_InitPlasmaTables(void)
{
 int i=0,j=0;
 float temp;
 unsigned char *s;

 tab1=(unsigned char *)malloc(pla2w2*pla2h*2);
 tab2=(unsigned char *)malloc(pla2w2*pla2h*2);
 if (!tab1 || !tab2)
    return 1;

 // The table is 2x the screen to move the circles
 s=tab1;
 for (i=0; i<pla2h2; i++)
     for (j=pla2w2; j; s++,--j)
         *s=(uchar)((sqrt( 16.0+(pla2h-i)*(pla2h-i)+(pla2w-j)*(pla2w-j) )-4) *5 );

 s=tab2;
 for (i=0; i<pla2h2; i++)
     for (j=pla2w2; j; s++,--j)
      {
       temp=sqrt(16.0+(pla2h-i)*(pla2h-i)+(pla2w-j)*(pla2w-j))-4;
       *s=(sin(temp/9.5)+1)*90;
       //tab2[(i*pla2w2)+j]=(sin(sqrt((pla2h-i)*(pla2h-i)+(pla2w-j)*(pla2w-j))/9.5)+1)*90;
      }
 return 0;
}


/**[txh]********************************************************************

  Description:
  Deallocates the memory used for the tables.

***************************************************************************/

void PLA2_DeInit(void)
{
 free(tab1);
 free(tab2);
}

static float circle1=0,circle2=0,circle3=0,circle4=0,circle5=0,circle6=0,circle7=0,circle8=0;
static int PLA2_x1,PLA2_y1,PLA2_x2,PLA2_y2,PLA2_x3,PLA2_y3,PLA2_x4,PLA2_y4;
//static int roll=0;

/*void PLA2_StepBody(unsigned char *buf)
{
 long i=0,j=0;
 char *s;
 // Create the pointers to 4 surfaces
 char *yy1=tab1+pla2w2*PLA2_y1+PLA2_x1;
 char *yy2=tab2+pla2w2*PLA2_y2+PLA2_x2;
 char *yy3=tab2+pla2w2*PLA2_y3+PLA2_x3;
 char *yy4=tab2+pla2w2*PLA2_y4+PLA2_x4;
 s=buf;

 for (i=pla2h; i; --i)
   {
    for (j=pla2w; j; s++,--j)
      {
       // Here we add the surfaces
       *s=yy1[j]+yy2[j]+yy3[j]+yy4[j];
      }
    yy1+=pla2w2;
    yy2+=pla2w2;
    yy3+=pla2w2;
    yy4+=pla2w2;
   }
}*/
/**[txh]********************************************************************

  Description:
  That's a four surfaces mixer for 320x200. Isn't generic because uses
320x200 and the surfaces are in fact two, one is used three times with
different trajectories. @x{PLA2G_StepBody}.@p
  BUF is the destination buffer.

***************************************************************************/

void PLA2_StepBody(unsigned char *buf)
{
 long i=0,j=0;
 unsigned *s;
 // Create the pointers to 4 surfaces
 unsigned *yy1=(unsigned *)(tab1+pla2w2*PLA2_y1+PLA2_x1);
 unsigned *yy2=(unsigned *)(tab2+pla2w2*PLA2_y2+PLA2_x2);
 unsigned *yy3=(unsigned *)(tab2+pla2w2*PLA2_y3+PLA2_x3);
 unsigned *yy4=(unsigned *)(tab2+pla2w2*PLA2_y4+PLA2_x4);
 s=(unsigned *)buf;

 for (i=pla2h; i; --i)
   {
    for (j=0; j<pla2w/4; s++,++j)
      {
       // Here we add the surfaces, but with 32 bits ;-))
       *s=yy1[j]+yy2[j]+yy3[j]+yy4[j];
      }
    yy1+=pla2w2/4;
    yy2+=pla2w2/4;
    yy3+=pla2w2/4;
    yy4+=pla2w2/4;
   }
}

/* That's the GCC code plus a little optimization. GCC generates a silly
   reload inside the inner loop (bug).
   Addtionally it shows a second bug: EBP isn't restored by GCC even when
   it's in the clobber list (was reported).
void PLA2_StepBody(unsigned char *buf)
{
 int i;

 asm("  pushl %%ebp

	movl _PLA2_y1,%%eax
	leal (%%eax,%%eax,4),%%eax
	sall $7,%%eax
	addl _tab1,%%eax
	addl _PLA2_x1,%%eax
	movl %%eax,%%edi

	movl _PLA2_y2,%%eax
	leal (%%eax,%%eax,4),%%eax
	sall $7,%%eax
	movl _tab2,%%ecx
	addl %%ecx,%%eax
	movl %%eax,%%ebp
	addl _PLA2_x2,%%ebp

	movl _PLA2_y3,%%eax
	leal (%%eax,%%eax,4),%%eax
	sall $7,%%eax
	addl %%ecx,%%eax
	movl %%eax,%%esi
	addl _PLA2_x3,%%esi

	movl _PLA2_y4,%%eax
	leal (%%eax,%%eax,4),%%eax
	sall $7,%%eax
	addl %%ecx,%%eax
	movl %%eax,%%ebx
	addl _PLA2_x4,%%ebx

	movl $200,%k0
	.align 2,0x90
LoopY:
	movl $320,%%ecx
	.align 2,0x90
LoopX:
	movb (%%ecx,%%edi),%%al
	addb (%%ecx,%%ebp),%%al
	addb (%%ecx,%%esi),%%al
	addb (%%ecx,%%ebx),%%al
	movb %%al,(%%edx)

	incl %%edx
	decl %%ecx
	jne LoopX

	addl $640,%%edi
	addl $640,%%ebp
	addl $640,%%esi
	addl $640,%%ebx
	decl %k0
	jne LoopY

        popl %%ebp
        " : "=m" (i)
          : "d" (buf)
          : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", "%ebp");
}*/

/**[txh]********************************************************************

  Description:
  That's one of the functions you can call to step in this plasma. This
version uses floating point for the tajectories.

***************************************************************************/
// That's slow, about 48750 trajectory points per second in my machine
void PLA2_Step1(unsigned char *buf)
{
 circle1+=0.085/6;
 circle2-=0.1/6;
 circle3+=.3/6;
 circle4-=.2/6;
 circle5+=.4/6;
 circle6-=.15/6;
 circle7+=.35/6;
 circle8-=.05/6;
 PLA2_x2=(pla2w/2)+(pla2w/2)*sin(circle1);
 PLA2_y2=(pla2h/2)+(pla2h/2)*cos(circle2);

 PLA2_x1=(pla2w/2)+(pla2w/2)*cos(circle3);
 PLA2_y1=(pla2h/2)+(pla2h/2)*sin(circle4);

 PLA2_x3=(pla2w/2)+(pla2w/2)*cos(circle5);
 PLA2_y3=(pla2h/2)+(pla2h/2)*sin(circle6);

 PLA2_x4=(pla2w/2)+(pla2w/2)*cos(circle7);
 PLA2_y4=(pla2h/2)+(pla2h/2)*sin(circle8);
 PLA2_StepBody(buf);
}


/**[txh]********************************************************************

  Description:
  That's one of the functions you can call to step in this plasma. This
version uses floating point increments and the Math 1 module for the co/sine.
@x{Math 1}.

***************************************************************************/
// That's a little faster 79130 per second
// I guess the error is less than 0.5% (1 pixel?)
double d1=0,d2=0,d3=0,d4=0,d5=0,d6=0,d7=0,d8=0;

void PLA2_Step2(unsigned char *buf)
{
 MA1_IncAng(d1,MA1_FromRad(0.085/6));
 MA1_DecAng(d2,MA1_FromRad(0.1/6));
 MA1_IncAng(d3,MA1_FromRad(.3/6));
 MA1_DecAng(d4,MA1_FromRad(.2/6));
 MA1_IncAng(d5,MA1_FromRad(.4/6));
 MA1_DecAng(d6,MA1_FromRad(.15/6));
 MA1_IncAng(d7,MA1_FromRad(.35/6));
 MA1_DecAng(d8,MA1_FromRad(.05/6));

 PLA2_x2=(pla2w/2)+(pla2w/2)*tsin(d1);
 PLA2_y2=(pla2h/2)+(pla2h/2)*tcos(d2);

 PLA2_x1=(pla2w/2)+(pla2w/2)*tcos(d3);
 PLA2_y1=(pla2h/2)+(pla2h/2)*tsin(d4);

 PLA2_x3=(pla2w/2)+(pla2w/2)*tcos(d5);
 PLA2_y3=(pla2h/2)+(pla2h/2)*tsin(d6);

 PLA2_x4=(pla2w/2)+(pla2w/2)*tcos(d7);
 PLA2_y4=(pla2h/2)+(pla2h/2)*tsin(d8);

 PLA2_StepBody(buf);
}

/**[txh]********************************************************************

  Description:
  That's one of the functions you can call to step in this plasma. This
version uses integer increments and the Math 1 module for the co/sine.
@x{Math 1}.

***************************************************************************/
// That's really faster 160600 per second, but is a little different.
// If you need more speed you can:
// 1) Use a sin/cos table with pla2w/2 as amplitude (other for h).
// 2) Just precalculate the trajectories with the Lissajous lib.
static int c1=0,c2=0,c3=0,c4=0,c5=0,c6=0,c7=0,c8=0;

void PLA2_Step3(unsigned char *buf)
{
 MA1_IncAng(c1,1);
 MA1_DecAng(c2,1);
 MA1_IncAng(c3,3);
 MA1_DecAng(c4,2);
 MA1_IncAng(c5,3);
 MA1_DecAng(c6,1);
 MA1_IncAng(c7,3);
 MA1_DecAng(c8,1);

 PLA2_x2=(pla2w/2)+(int)((pla2w/2)*tsin(c1));
 PLA2_y2=(pla2h/2)+(int)((pla2h/2)*tcos(c2));

 PLA2_x1=(pla2w/2)+(int)((pla2w/2)*tcos(c3));
 PLA2_y1=(pla2h/2)+(int)((pla2h/2)*tsin(c4));

 PLA2_x3=(pla2w/2)+(int)((pla2w/2)*tcos(c5));
 PLA2_y3=(pla2h/2)+(int)((pla2h/2)*tsin(c6));

 PLA2_x4=(pla2w/2)+(int)((pla2w/2)*tcos(c7));
 PLA2_y4=(pla2h/2)+(int)((pla2h/2)*tsin(c8));

 PLA2_StepBody(buf);
}


