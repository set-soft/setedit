/* Copyright (C) 1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
/**[txh]********************************************************************

  Module: Mix Surface
  Comments:
  This module is used to add bitmaps.

***************************************************************************/

#include <stdio.h>
#ifdef FAKE_ALLEGRO
#include <fakealle.h>
#else
#include <allegro.h>
#endif
#include "mixsurf.h"

MS_4struct MS_4;

/**[txh]********************************************************************

  Description:
  That's a very generic and very optimized four surfaces mixer.

***************************************************************************/

/* That's the C version used to generate the ASM version.
   The ASM is 9% faster because I removed 1 silly instruction in the inner
loop. Is a clear bug in the GCC optimizer.
   Some interesting numbers:
   Each mix is 80*200 dwords = 16000 passes. The inner loop have 9
instructions (144000), the outther 10 more (144800), suppose the rest is
40 instructions (144840) and I got 182 fps (to memory) => 26360880. Puaj! */
void MS_4addC(void)
{
 static long i;
 long j;
 unsigned *s;
 // Create the pointers to 4 surfaces
 unsigned *yy1=MS_4.s1;
 unsigned *yy2=MS_4.s2;
 unsigned *yy3=MS_4.s3;
 unsigned *yy4=MS_4.s4;
 s=(unsigned *)MS_4.dest;

 for (i=MS_4.sH; i; --i)
   {
    for (j=0; j<MS_4.sWw; s++,j++)
        *s=yy1[j]+yy2[j]+yy3[j]+yy4[j];
    yy1+=MS_4.mWw;
    yy2+=MS_4.mWw;
    yy3+=MS_4.mWw;
    yy4+=MS_4.mWw;
   }
}

void MS_4add(void)
{
 static int i;
 asm("					\n\
	pushl %%ebp                     \n\
                                        \n\
   // s1,s2,s3,s4                       \n\
	movl _MS_4,%%ebp                \n\
	movl _MS_4+4,%%edi              \n\
	movl _MS_4+8,%%esi              \n\
	movl _MS_4+12,%%ebx             \n\
                                        \n\
   // dest                              \n\
	movl _MS_4+16,%%ecx             \n\
   // sH, fuck just 1 register more demm Intel   \n\
	movl _MS_4+24,%%eax             \n\
	movl %%eax,%k0                  \n\
                                        \n\
LoopI:                                  \n\
   // j=0                               \n\
	xorl %%edx,%%edx                \n\
	.align 2,0x90                   \n\
                                        \n\
LoopJ:                                  \n\
	movl (%%ebp,%%edx,4),%%eax      \n\
	addl (%%edi,%%edx,4),%%eax      \n\
	addl (%%esi,%%edx,4),%%eax      \n\
	addl (%%ebx,%%edx,4),%%eax      \n\
	movl %%eax,(%%ecx)              \n\
        // I moved it here hoping the P5 will pair the 4 instructions \n\
        incl %%edx   			\n\
	addl $4,%%ecx                   \n\
	cmpl %%edx,_MS_4+20             \n\
	ja LoopJ                        \n\
                                        \n\
	movl _MS_4+32,%%eax             \n\
	addl %%eax,%%ebp                \n\
	addl %%eax,%%edi                \n\
	addl %%eax,%%esi                \n\
	addl %%eax,%%ebx                \n\
	decl %k0                        \n\
	jne  LoopI                      \n\
                                        \n\
	popl %%ebp                      \n\
   ": "=m" (i) : : "%eax","%ebx","%ecx","%edx","%esi","%edi");
}

/*void MS_4addCyrix(void)
{
 static int i;
 asm("                                  \n\
	pushl %%ebp                     \n\
                                        \n\
   // s1,s2,s3,s4                       \n\
	movl _MS_4,%%ebp                \n\
	movl _MS_4+4,%%ecx              \n\
	movl _MS_4+8,%%esi              \n\
	movl _MS_4+12,%%ebx             \n\
                                        \n\
   // dest                              \n\
	movl _MS_4+16,%%edi             \n\
   // sH, fuck just 1 register more demm Intel \n\
	movl _MS_4+24,%%eax             \n\
	movl %%eax,%k0                  \n\
                                        \n\
0:                                      \n\
   // j=0                               \n\
	xorl %%edx,%%edx                \n\
	.align 2,0x90                   \n\
                                        \n\
1:                                      \n\
	movl (%%ebp,%%edx,4),%%eax      \n\
	addl (%%ecx,%%edx,4),%%eax      \n\
	addl (%%esi,%%edx,4),%%eax      \n\
	addl (%%ebx,%%edx,4),%%eax      \n\
   stosl                                \n\
	incl %%edx                      \n\
	cmpl %%edx,_MS_4+20             \n\
	ja   1b                         \n\
                                        \n\
	movl _MS_4+32,%%eax             \n\
	addl %%eax,%%ebp                \n\
	addl %%eax,%%ecx                \n\
	addl %%eax,%%esi                \n\
	addl %%eax,%%ebx                \n\
	decl %k0                        \n\
	jne  0b                         \n\
                                        \n\
	popl %%ebp                      \n\
   ": "=m" (i) : : "%eax","%ebx","%ecx","%edx","%esi","%edi");
}*/


// That's as fast as the assembler version. To be honest is a little slower
// but you can't meassure the difference. I didn't tested the it in a P5.
// I think the ASM is faster there.
void MS_4addW(void)
{
 long i=0,j=0;
 unsigned *s;
 // Create the pointers to 4 surfaces
 unsigned *yy1=MS_4.s1;
 unsigned *yy2=MS_4.s2;
 unsigned *yy3=MS_4.s3;
 unsigned *yy4=MS_4.s4;
 s=(unsigned *)MS_4.dest;

 for (i=MS_4.sH; i; --i)
   {
    for (j=MS_4.sWw; j; s++,--j)
       *s=yy1[j]+yy2[j]+yy3[j]+yy4[j];
    yy1+=MS_4.mWw;
    yy2+=MS_4.mWw;
    yy3+=MS_4.mWw;
    yy4+=MS_4.mWw;
   }
}

void MS_3addC(void)
{
 long i,j;
 unsigned *s;
 unsigned *yy1=MS_4.s1;
 unsigned *yy2=MS_4.s2;
 unsigned *yy3=MS_4.s3;
 s=(unsigned *)MS_4.dest;

 for (i=MS_4.sH; i; --i)
   {
    for (j=0; j<MS_4.sWw; s++,j++)
        *s=yy1[j]+yy2[j]+yy3[j];
    yy1+=MS_4.mWw;
    yy2+=MS_4.mWw;
    yy3+=MS_4.mWw;
   }
}

void MS_3add(void)
{
 asm("                                   \n\
	pushl %%ebp                      \n\
                                         \n\
   // s1,s2,s3                           \n\
	movl _MS_4,%%ebp                 \n\
	movl _MS_4+4,%%edi               \n\
	movl _MS_4+8,%%esi               \n\
                                         \n\
   // dest                               \n\
	movl _MS_4+16,%%ecx              \n\
   // sH                                 \n\
	movl _MS_4+24,%%ebx              \n\
                                         \n\
LoopI2:                                  \n\
   // j=0                                \n\
	xorl %%edx,%%edx                 \n\
                                         \n\
LoopJ2:                                  \n\
	movl (%%ebp,%%edx,4),%%eax       \n\
	addl (%%edi,%%edx,4),%%eax       \n\
	addl (%%esi,%%edx,4),%%eax       \n\
	movl %%eax,(%%ecx)               \n\
        // I moved it here hoping the P5 will pair the 4 instruction\n\
        incl %%edx                       \n\
	addl $4,%%ecx                    \n\
	cmpl %%edx,_MS_4+20              \n\
	ja LoopJ2                        \n\
                                         \n\
	movl _MS_4+32,%%eax              \n\
	addl %%eax,%%ebp                 \n\
	addl %%eax,%%edi                 \n\
	addl %%eax,%%esi                 \n\
	decl %%ebx                       \n\
	jne  LoopI2                      \n\
                                         \n\
	popl %%ebp                       \n\
   ": : : "%eax","%ebx","%ecx","%edx","%esi","%edi");
}

// Here my meassure indicates C faster than ASM ?!
void MS_2addC(void)
{
 long i,j,k;
 unsigned *s;
 unsigned *yy1=MS_4.s1;
 unsigned *yy2=MS_4.s2;
 s=(unsigned *)MS_4.dest;

 for (i=MS_4.sH; i; --i)
   {
    for (j=0,k=MS_4.sWw; k; s++,j++,--k)
        *s=yy1[j]+yy2[j];
    yy1+=MS_4.mWw;
    yy2+=MS_4.mWw;
   }
}

void MS_2add(void)
{
 asm("                                          \n\
	pushl %%ebp                             \n\
                                                \n\
   // s1,s2,s3                                  \n\
	movl _MS_4,%%esi                        \n\
	movl _MS_4+4,%%edi                      \n\
                                                \n\
   // dest                                      \n\
	movl _MS_4+16,%%ecx                     \n\
   // sH                                        \n\
	movl _MS_4+24,%%edx                     \n\
                                                \n\
LoopI3:                                         \n\
   // j=0                                       \n\
	xorl %%eax,%%eax                        \n\
   movl _MS_4+20,%%ebp                          \n\
	.align 2,0x90                           \n\
                                                \n\
LoopJ3:                                         \n\
	movl (%%esi,%%eax,4),%%ebx              \n\
	addl (%%edi,%%eax,4),%%ebx              \n\
	movl %%ebx,(%%ecx)                      \n\
	incl %%eax                              \n\
	addl $4,%%ecx                           \n\
   decl %%ebp                                   \n\
	jne  LoopJ3                             \n\
                                                \n\
	movl _MS_4+32,%%eax                     \n\
	addl %%eax,%%esi                        \n\
	addl %%eax,%%edi                        \n\
	decl %%edx                              \n\
	jne  LoopI3                             \n\
                                                \n\
	popl %%ebp                              \n\
   ": : : "%eax","%ebx","%ecx","%edx","%edi","%esi");
}

/**[txh]********************************************************************

  Description:
  This function is used to invert a an image generated with a for loop that
increments and then blitted with a loop that decrements.@p
  I used it to speed up the routines but isn't much faster and needs
multiple of four X coordinates, so isn't smooth.

***************************************************************************/

void MS_InvertDWordScreen(int w, int h, unsigned char *b)
{
 int x,y,s1,s2,w2;
 unsigned *s=(unsigned *)b;
 unsigned aux;

 w2=w/4;
 w/=8;

 for (y=h; y; s+=w2,y--)
     for (s1=0,s2=w2-1,x=w; x; s1++,s2--,x--)
        {
         aux=s[s1];
         s[s1]=s[s2];
         s[s2]=aux;
        }
}

void MS_AddBmpsMem(BITMAP *source, BITMAP *dest, int source_x,
                   int source_y, int dest_x, int dest_y, int width,
                   int height)
{
 int y,x;
 char *src=source->line[source_y]+source_x;
 int wS=source->w-width;
 char *dst=dest->line[dest_y]+dest_x;
 int wD=dest->w-width;

 for (y=height; y; src+=wS,dst+=wD,--y)
     for (x=width; x; src++,dst++,--x)
         *dst+=*src;
}

void MS_CutBMPColorsAt(BITMAP *bmp, unsigned char cut, unsigned char val)
{
 int y,x;
 unsigned char *s=bmp->line[0];

 for (y=bmp->h; y; --y)
     for (x=bmp->w; x; s++,--x)
         *s=*s<cut ? 0 : val;
}

void MS_CutBMPColorsAtInv(BITMAP *bmp, unsigned char cut, unsigned char val)
{
 int y,x;
 unsigned char *s=bmp->line[0];

 for (y=bmp->h; y; --y)
     for (x=bmp->w; x; s++,--x)
         *s=*s<cut ? val : 0;
}

void MS_AddWithMaskMem(BITMAP *source, BITMAP *dest, int source_x,
                   int source_y, int dest_x, int dest_y, int width,
                   int height,int val)
{
 int y,x;
 char *src=source->line[source_y]+source_x;
 int wS=source->w-width;
 char *dst=dest->line[dest_y]+dest_x;
 int wD=dest->w-width;

 for (y=height; y; src+=wS,dst+=wD,--y)
     for (x=width; x; src++,dst++,--x)
         if (*src)
            *dst+=val;
}

inline void Flush(unsigned char **d,int count,int type)
{
 while (count>0)
   {
    if (count>127)
      {
       *((*d)++)=type | 127;
       count-=127;
      }
    else
      {
       *((*d)++)=type | count;
       count=0;
      }
   }
}

void MS_CompactMask(BITMAP *bmp)
{
 int x,y;
 int w=bmp->w,h=bmp->h,count=1;
 unsigned char *s=bmp->line[0],*d;
 int is_1;
 d=s;

 for (y=h; y; --y)
    {
     is_1=*s!=0;
     count=1;
     for (s++,x=1; x<w; s++,x++)
        {
         if (is_1)
           {
            if (*s)
              {
               count++;
              }
            else
              {
               Flush(&d,count,0x80);
               is_1=0;
               count=1;
              }
           }
         else
           {
            if (!*s)
               count++;
            else
              {
               Flush(&d,count,0);
               is_1=1;
               count=1;
              }
           }
        }
     if (is_1)
        Flush(&d,count,0x80);
     else
        Flush(&d,count,0);
     *(d++)=0;
    }
}

void MS_AddWithMaskMemC(char *source, BITMAP *dest, int dest_x,
                        int dest_y, int width, int height, int val)
{
 int y;
 char *src=source;
 char *dst=dest->line[dest_y]+dest_x;
 int wD=dest->w-width;
 unsigned char count;

 for (y=height; y; dst+=wD,--y)
    {
     while (*src)
       {
        if (*src & 0x80)
          {
#if 0
           count=*src & 0x7F;
           while (count--)
             *(dst++)+=val;
#else
           count=*src & 0x7F;
           do
             {
              *(dst++)+=val;
             }
           while (--count);
#endif
          }
        else
          dst+=*src;
        src++;
       }
     src++;
    }
}

void MS_DumpCompacted(BITMAP *bmp, char *name, char *file)
{
 int y;
 int h=bmp->h;
 FILE *f;
 unsigned char *src=bmp->line[0];

 f=fopen(file,"wt");
 if (!f) return;
 fprintf(f,"int %sW=%d,%sH=%d;\nunsigned char %sP[]={\n",name,bmp->w,name,bmp->h,name);

 for (y=0; y<h; y++)
    {
     while (*src)
       {
        fprintf(f,"%3d, ",*src);
        src++;
       }
     fprintf(f,"  0%c /* %d */\n",y==h-1 ? ' ' : ',',y);
     src++;
    }
 fprintf(f,"}; // Size: %ld Compression: %5.2f %%\n",src-bmp->line[0],(1.0-(src-bmp->line[0])/(double)(h*bmp->w))*100.0);
 fclose(f);
}

/* Menos del 3% de diferencia y no funciona! ver por que
void MS_AddWithMaskMem(BITMAP *source, BITMAP *dest, int source_x,
                   int source_y, int dest_x, int dest_y, int width,
                   int height,int val)
{
 char *src=source->line[source_y]+source_x;
 int wS=source->w-width;
 char *dst=dest->line[dest_y]+dest_x;
 int wD=dest->w-width;

 asm ("
   movl  %k1,%%ebp
	testl %%ecx,%%ecx
	je    5f
	testl %%edi,%%edi
	je    5f
	.align 2,0x90

1:
   movl  %%edi,%%eax
2:
	cmpb $0,(%%esi)
	je   3f
	addb %%bl,(%%edx)
3:
	incl %%esi
	incl %%edx
	decl %%eax
	jne  2b

4:
	addl %%ebp,%%esi
	addl %k0,%%edx
	decl %%ecx
	jne  1b
5:
 " : "=m" (wD)
   : "q" (wS), "c" (height), "D" (width), "S" (src), "b" (val), "d" (dst)
   : "%eax", "%ebp");
}*/

