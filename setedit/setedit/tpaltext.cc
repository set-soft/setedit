/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */

#define Uses_stdio
#define Uses_string
#define Uses_fpstream
#define Uses_TScreen
#include <tv.h>
#include <tpaltext.h>

PalCol TTextPalette::OriginalPalette[16];
PalCol TTextPalette::ActualPalette[16];

/*
  In TV 2.0.0 I moved it to Turbo Vision, so that's a simple wrapper over
  the TV code.
*/

TTextPalette::TTextPalette()
{
}

void TTextPalette::SetPalette(PalCol *cols)
{
 TScreen::setPaletteColors(0,16,cols);
}

void TTextPalette::suspend()
{
}

void TTextPalette::resume()
{
}

TTextPalette::~TTextPalette()
{
}

void TTextPalette::SetOne(int color, int R, int G, int B)
{
 TScreenColor col={R,G,B,0xFF};
 TScreen::setPaletteColors(color,1,&col);
}

void TTextPalette::GetOne(int color, int &R, int &G, int &B)
{
 TScreenColor col;
 TScreen::getPaletteColors(color,1,&col);
 R=col.R;
 G=col.G;
 B=col.B;
}

void TTextPalette::BackToDefault(void)
{
 TScreen::resetPalette();
}

void TTextPalette::Save(fpstream &s)
{
 // version
 s << (char)2;
 TScreen::getPaletteColors(0,16,ActualPalette);
 int i;
 for (i=0; i<16; i++)
     s << ActualPalette[i].R << ActualPalette[i].G << ActualPalette[i].B;
}

void TTextPalette::Load(fpstream &s, PalCol *pal)
{
 PalCol *dest=pal ? pal : ActualPalette;
 char version;
 s >> version;
 int i;
 for (i=0; i<16; i++)
     s >> dest[i].R >> dest[i].G >> dest[i].B;
 if (version==1)
   { // v1 was stored with 0-63 values, 0-255 is much more generic.
    int i;
    for (i=0; i<16; i++)
       {
        dest[i].R<<=2;
        dest[i].G<<=2;
        dest[i].B<<=2;
       }
   }
 // Old versions for Linux saved black palettes, we must avoid using them
 int acum=0;
 for (i=0; i<16 && !acum; i++)
     acum+=dest[i].R+dest[i].G+dest[i].B;
 if (!acum)
    memcpy(dest,OriginalPalette,sizeof(ActualPalette));
 if (!pal)
    TScreen::setPaletteColors(0,16,ActualPalette);
}

void TTextPalette::getArray(unsigned *pal)
{
 int i;

 TScreen::getPaletteColors(0,16,ActualPalette);
 for (i=0; i<16; i++)
     pal[i]=(ActualPalette[i].R<<16) | (ActualPalette[i].G<<8) | ActualPalette[i].B;
}

PalCol *TTextPalette::GetAllPal()
{
 PalCol *ret=new PalCol[16];
 TScreen::getPaletteColors(0,16,ret);
 return ret;
}

void TTextPalette::SetAllPal(PalCol *pal)
{
 TScreen::setPaletteColors(0,16,pal);
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

