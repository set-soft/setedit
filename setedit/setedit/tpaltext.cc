/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */

#define Uses_stdio
#define Uses_string
#define Uses_fpstream
#include <tv.h>
#include <tpaltext.h>
#if TV_MAJOR_VERSION==2
#define Uses_TScreen
#include <termios.h>
#include <term.h>
#include <sys/ioctl.h>
#include <tv/screen.h>
#include <tv/linux/screen.h>
#endif

PalCol TTextPalette::OriginalPalette[16];
PalCol TTextPalette::ActualPalette[16];

/********************** DJGPP Working version ************************/
/* Implemented with BIOS to increase compatibility                   */
#ifdef TVCompf_djgpp
#include <dpmi.h>
#include <go32.h>
#include <sys/movedata.h>

__dpmi_regs r;
#define AX r.x.ax
#define BL r.h.bl
#define BH r.h.bh
#define CL r.h.cl
#define CH r.h.ch
#define DL r.h.dl
#define DH r.h.dh
#define ES r.x.es
#define DX r.x.dx
#define BX r.x.bx
#define Int(a) __dpmi_int(a,&r)

/* Not used but very interesting to understand how it works
void TXTSetOneColor(int t_col, int v_col)
{
 AX=0x1000;
 BL=t_col;
 BH=v_col;
 Int(0x10);
}

int TXTGetOneColor(int t_col)
{
 AX=0x1007;
 BL=t_col;
 Int(0x10);
 return BH;
}
*/

static char ColorsMap[17];

static
void TXTGetAllColors(char *map)
{
 AX=0x1009;
 ES=__tb>>4;
 DX=__tb & 0xF;
 Int(0x10);
 dosmemget(__tb,17,map);
}

static
void TXTInitialize()
{// Get the colors mapping table so we know what color in the 256 colors
 // palette must use.
 TXTGetAllColors(ColorsMap);
}

static
void TXTSetOneIndex(int index, PalCol *col)
{
 AX=0x1010;
 BX=ColorsMap[index];
 DH=col->R>>2; CH=col->G>>2; CL=col->B>>2;
 Int(0x10);
}

static
void TXTGetOneIndex(int index, PalCol *col)
{
 AX=0x1015;
 BL=ColorsMap[index];
 Int(0x10);
 col->R=DH<<2;
 col->G=CH<<2;
 col->B=CL<<2;
}
/**************** End of  DJGPP Working version **********************/
#define PAL_DEFINED
#endif // DJGPP



/********************** Linux version ************************/
#ifdef TVOSf_Linux
#define Uses_TScreen
#include <tv.h>
#include <stdlib.h>

const int NotTerm=0,LinuxTerm=1,XTerm=2,UnknownTerm=3;
static int TerminalType=NotTerm;

// I assume we start with the BIOS palette. It could be wrong but I
// don't know how to get the palette.
PalCol BIOSPalette[16]={
{ 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0xA8 },
{ 0x00, 0xA8, 0x00 },
{ 0x00, 0xA8, 0xA8 },
{ 0xA8, 0x00, 0x00 },
{ 0xA8, 0x00, 0xA8 },
{ 0xA8, 0x54, 0x00 },
{ 0xA8, 0xA8, 0xA8 },
{ 0x54, 0x54, 0x54 },
{ 0x54, 0x54, 0xFC },
{ 0x54, 0xFC, 0x54 },
{ 0x54, 0xFC, 0xFC },
{ 0xFC, 0x54, 0x54 },
{ 0xFC, 0x54, 0xFC },
{ 0xFC, 0xFC, 0x54 },
{ 0xFC, 0xFC, 0xFC }};

static
void TXTInitialize()
{
 if (TerminalType!=NotTerm) return;
 char * term;
 term=getenv("TERM");
 if (!strncmp(term,"xterm",5))
    TerminalType=XTerm;
 else if (!strcmp(term,"console") || !strcmp(term,"linux"))
    TerminalType=LinuxTerm;
 else
    TerminalType=UnknownTerm;
}

// Escape sequences from ctheme, a nice program.
static
void TXTSetOneIndex(int index, PalCol *col)
{
 char b[40];
 int  changed;
 index&=0xF;

 /* Adapt to Linux style */
 static char map[]={0,4,2,6,1,5,3,7,8,12,10,14,9,13,11,15,16};
 int Index=map[index];
 int R=col->R;
 int G=col->G;
 int B=col->B;

 switch (TerminalType)
   {
    case LinuxTerm:
         sprintf(b,"\e]P%1.1X%2.2X%2.2X%2.2X",Index,R,G,B);
         #if TV_MAJOR_VERSION==2
         TScreenUNIX::SendToTerminal(b);
         #else
         TScreen::SendToTerminal(b);
         #endif
         changed=1;
         break;
    case XTerm:
         sprintf(b,"\e]4;%d;#%2.2x%2.2x%2.2x\e\\",Index,R,G,B);
         #if TV_MAJOR_VERSION==2
         TScreenUNIX::SendToTerminal(b);
         #else
         TScreen::SendToTerminal(b);
         #endif
         if (Index==0)
           { /* If setting 'black', also set the background */
            sprintf(b,"\e]11;#%2.2x%2.2x%2.2x\e\\",R,G,B);
            #if TV_MAJOR_VERSION==2
            TScreenUNIX::SendToTerminal(b);
            #else
            TScreen::SendToTerminal(b);
            #endif
           }
         else if (Index==7)
           { /* if setting 'white', also set the foreground */
            sprintf(b,"\e]10;#%2.2x%2.2x%2.2x\e\\",R,G,B);
            #if TV_MAJOR_VERSION==2
            TScreenUNIX::SendToTerminal(b);
            #else
            TScreen::SendToTerminal(b);
            #endif
           }
         changed=1;
         break;
    default:
         changed=0;
   }
 if (changed)
    BIOSPalette[index]=*col;
}

static
void TXTGetOneIndex(int index, PalCol *col)
{
 *col=BIOSPalette[index & 0xF];
}
/**************** End of Linux Dummy version ************************/
#define PAL_DEFINED
#endif // Linux



/********************** Dummy version ************************/
#ifndef PAL_DEFINED
static
void TXTSetOneIndex(int /*index*/, PalCol */*col*/)
{
}

PalCol BIOSPalette[16]={
{ 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0xA8 },
{ 0x00, 0xA8, 0x00 },
{ 0x00, 0xA8, 0xA8 },
{ 0xA8, 0x00, 0x00 },
{ 0xA8, 0x00, 0xA8 },
{ 0xA8, 0x54, 0x00 },
{ 0xA8, 0xA8, 0xA8 },
{ 0x54, 0x54, 0x54 },
{ 0x54, 0x54, 0xFC },
{ 0x54, 0xFC, 0x54 },
{ 0x54, 0xFC, 0xFC },
{ 0xFC, 0x54, 0x54 },
{ 0xFC, 0x54, 0xFC },
{ 0xFC, 0xFC, 0x54 },
{ 0xFC, 0xFC, 0xFC }};

static
void TXTGetOneIndex(int index, PalCol *col)
{
 *col=BIOSPalette[index & 0xF];
}

static
void TXTInitialize()
{
}
/**************** End of Dummy version ************************/
#endif // !PAL_DEFINED


/**************** Common code shared by all versions ************************/
TTextPalette::TTextPalette()
{
 int i;
 TXTInitialize();
 for (i=0; i<16; i++)
     TXTGetOneIndex(i,&OriginalPalette[i]);
 memcpy(ActualPalette,OriginalPalette,sizeof(ActualPalette));
}

void TTextPalette::SetPalette(PalCol *cols)
{
 int i;
 for (i=0; i<16; i++)
     TXTSetOneIndex(i,&cols[i]);
}

void TTextPalette::suspend()
{
 SetPalette(OriginalPalette);
}

void TTextPalette::resume()
{
 SetPalette(ActualPalette);
}

TTextPalette::~TTextPalette()
{
 suspend();
}

void TTextPalette::SetOne(int color, int R, int G, int B)
{
 PalCol *col=&ActualPalette[color];
 col->R=R; col->G=G; col->B=B;
 TXTSetOneIndex(color,col);
}

void TTextPalette::GetOne(int color, int &R, int &G, int &B)
{
 PalCol *col=&ActualPalette[color];
 R=col->R; G=col->G; B=col->B;
}

void TTextPalette::BackToDefault(void)
{
 suspend();
 memcpy(ActualPalette,OriginalPalette,sizeof(ActualPalette));
}

void TTextPalette::Save(fpstream &s)
{
 // version
 s << (char)2;
 s.writeBytes(ActualPalette,sizeof(ActualPalette));
}

void TTextPalette::Load(fpstream &s)
{
 char version;
 s >> version;
 s.readBytes(ActualPalette,sizeof(ActualPalette));
 if (version==1)
   { // v1 was stored with 0-63 values, 0-255 is much more generic.
    int i;
    for (i=0; i<16; i++)
       {
        ActualPalette[i].R<<=2;
        ActualPalette[i].G<<=2;
        ActualPalette[i].B<<=2;
       }
   }
 // Old versions for Linux saved black palettes, we must avoid using them
 int i,acum=0;
 for (i=0; i<16 && !acum; i++)
     acum+=ActualPalette[i].R+ActualPalette[i].G+ActualPalette[i].B;
 if (!acum)
    memcpy(ActualPalette,OriginalPalette,sizeof(ActualPalette));
 //memcpy(ActualPalette,OriginalPalette,sizeof(ActualPalette));
 //resume(); Not needed, the next video mode setting will do it
}

void TTextPalette::getArray(unsigned *pal)
{
 int i;

 for (i=0; i<16; i++)
     pal[i]=(ActualPalette[i].R<<16) | (ActualPalette[i].G<<8) | ActualPalette[i].B;
}

PalCol *TTextPalette::GetAllPal()
{
 PalCol *ret=new PalCol[16];
 memcpy(ret,ActualPalette,sizeof(ActualPalette));
 return ret;
}

void TTextPalette::SetAllPal(PalCol *pal)
{
 memcpy(ActualPalette,pal,sizeof(ActualPalette));
 resume();
}

#ifdef TEST
int main(int argc, char *argv[])
{
 int i;

 intensevideo();
 for (i=0; i<16; i++)
    {
     textattr(0xF | (i<<4));
     cprintf(" %d ",i);
    }
 getch();

 TTextPalette *p=new TTextPalette();

 textattr(0x1F);
 p->SetOne(6,30,60,10);
 getch();
 p->suspend();
 getch();
 p->resume();
 getch();

 delete p;
 return 0;
}
#endif // TEST
/* for (i=0; i<16; i++)
     printf("%d,",TXTGetOneColor(i));
 TXTGetAllColors(ColorsMap);
 printf("\n");
 for (i=0; i<16; i++)
     printf("%d,",ColorsMap[i]);
*/

