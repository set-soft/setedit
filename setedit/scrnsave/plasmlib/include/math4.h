/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifdef __cplusplus
extern "C" {
#endif
extern int    XSin256t[];
extern int    YSin256t[];
extern double DSin256t[];

// 320 values and not 512 ;-)
#define XCos256t (XSin256t+64)
#define YCos256t (YSin256t+64)
#define DCos256t (DSin256t+64)

#define MA4NoScale  1
#define MA4Scale    0
#define MA4XSin     2
#define MA4YSin     0

// Only 16 bits used but 16 bits access is slow in 32 bits modes
typedef int fix8_8;

#define MA4_FromRad(a)     ((a)*256/(2*PI))
#define MA4_FromDeg(a)     ((a)*256/360.0)

extern inline fix8_8 MA4_f2fix(double x)
{ 
 return (fix8_8)(x*256.0+0.5);
}

extern inline double MA4_fix2f(fix8_8 x)
{ 
 return x/256.0;
}

extern inline fix8_8 MA4_i2fix(int x)
{ 
 return (fix8_8)(x*256);
}

extern inline int MA4_fix2i(fix8_8 x)
{ 
 return x/256;
}

//#define USE_EAX

// Fast, not the best but supports decimal increments
extern inline int MA4_XSin(fix8_8 x)
{
#ifdef USE_EAX
 int result;
 __asm__ __volatile__ (
" movzbl %%ah,%%eax              \n"
" movl _XSin256t(,%%eax,4),%k0   \n"
 : "=r" (result)
 : "a" (x));
 return result;
#else
 return XSin256t[(x>>8) & 0xFF];
#endif
}

// With round
extern inline int MA4_XSinR(fix8_8 x)
{
#ifdef USE_EAX
 int result;
 __asm__ __volatile__ (
" addl $0x80,%%eax               \n"
" movzbl %%ah,%%eax              \n"
" movl _XSin256t(,%%eax,4),%k0   \n"
 : "=r" (result)
 : "a" (x));
 return result;
#else
 return XSin256t[((x+0x80)>>8) & 0xFF];
#endif
}

extern inline int MA4_YSin(fix8_8 x)
{
 return YSin256t[(x>>8) & 0xFF];
}

// With round
extern inline int MA4_YSinR(fix8_8 x)
{
 return YSin256t[((x+0x80)>>8) & 0xFF];
}

extern inline int MA4_XCos(fix8_8 x)
{
 return XCos256t[(x>>8) & 0xFF];
}

// With round
extern inline int MA4_XCosR(fix8_8 x)
{
 return XCos256t[((x+0x80)>>8) & 0xFF];
}

extern inline int MA4_YCos(fix8_8 x)
{
 return YCos256t[(x>>8) & 0xFF];
}

// With round
extern inline int MA4_YCosR(fix8_8 x)
{
 return YCos256t[((x+0x80)>>8) & 0xFF];
}

void MA4_FillTable(double Amp, double Off, double Scale, int Flags);
void MA4_FillTableI(int Amp, int Off, int Scale, int Flags);
void MA4_FillTableIW(int Amp, int Off, int Scale, int Flags);

#ifdef __cplusplus
}
#endif

