/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned char RawPal[768];

/* Nutty's set_entire_palette */
/* Our inline set palette function. Inline for maximum speed. */
extern __inline__ void RPF_SetAllPal(unsigned char *_pal_ptr)
{//Sets all 768 6bit color component entries on the VGA.
 int dummy;
 __asm__ __volatile__(
"     movl $0x3c8, %%edx   \n"
"     xorl %%eax, %%eax    \n"
"     outb %%al, %%dx      \n"
"     incl %%edx           \n"
"     movl $768, %%ecx     \n"
"     cli                  \n"
"     rep                  \n"
"     outsb                \n"
"     sti                  \n"
 : "=S" (dummy) : "S" (_pal_ptr) : "%eax", "%ecx", "%edx");
}

void RPF_MakeMultiPal(unsigned char *Pal);          // SET
void RPF_MakeRedBlueBar(unsigned char *Pal);        // Nutty
void RPF_MakeBlueGreenBar(unsigned char *Pal);      // Nutty
void RPF_MakeGreenRedBar(unsigned char *Pal);       // SET
void RPF_MakeGreenRedBar2(unsigned char *Pal);      // Nutty
void RPF_MakeBlueGreen_RedBars(unsigned char *Pal); // SET
void RPF_DrawPalette(int w, int h, unsigned char *s); // SET
void RPF_FromBlack(unsigned char *dest,unsigned char *out,int step);
void RPF_Negative(unsigned char *orig,unsigned char *out);
void RPF_AddPaletteStep(unsigned char *orig,unsigned char *inc,unsigned char *out,int step);
void RPF_FromWhite(unsigned char *dest,unsigned char *out,int step);
void RPF_RGBBarsWithCos(unsigned char *Pal);
void RPF_SetPalRange(unsigned char *_pal_ptr, int color, int cant);
void RPF_Argentina(unsigned char *Pal);
void RPF_GetAllegroPalette(char *pal);
void RPF_GrayScale(unsigned char *Pal);
#ifdef __cplusplus
}
#endif

