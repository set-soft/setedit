/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */

#define Uses_stdio
#define Uses_string
#define Uses_fpstream
#define Uses_TScreen
#include <tv.h>
#include <tpaltext.h>

PalCol TTextPalette::ActualPalette[16];
char   TTextPalette::Changed=0;

/*
  In TV 2.0.0 I moved it to Turbo Vision, so that's a simple wrapper over
  the TV code. It also acts as a cache
*/

void TTextPalette::SetOne(int color, int R, int G, int B)
{
 ActualPalette[color].R=R;
 ActualPalette[color].G=G;
 ActualPalette[color].B=B;
 TScreen::setPaletteColors(color,1,&ActualPalette[color]);
 // Re-compute the changed value
 const TScreenColor *orig=TDisplay::getDefaultPalette()+color;
 if (orig->R==R && orig->G==G && orig->B==B)
   {// Restoring this color
    if (Changed)
       ComputeChangedStatus();
   }
 else
    Changed=1;
}

void TTextPalette::ComputeChanged()
{
 const TScreenColor *orig=TDisplay::getDefaultPalette();
 int i;
 for (i=0; i<16; i++)
     if (ActualPalette[i].R!=orig[i].R || ActualPalette[i].G!=orig[i].G ||
         ActualPalette[i].B!=orig[i].B)
       {
        Changed=1;
        return;
       }
 Changed=0;
}

void TTextPalette::GetOne(int color, int &R, int &G, int &B)
{
 R=ActualPalette[color].R;
 G=ActualPalette[color].G;
 B=ActualPalette[color].B;
}

void TTextPalette::BackToDefault(void)
{
 TScreen::resetPalette();
 Changed=0;
 Cache();
}

// This funtion exists only for compatibility with all desktop files.
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
   {
    memcpy(dest,TDisplay::getDefaultPalette(),sizeof(ActualPalette));
    Changed=0;
   }
 else
    if (!pal)
      {
       ComputeChanged();
       Restore();
      }
}

void TTextPalette::Cache()
{
 TScreen::getPaletteColors(0,16,ActualPalette);
 ComputeChanged();
}

void TTextPalette::GetArray(unsigned *pal)
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
 TScreen::setPaletteColors(0,16,pal);
 memcpy(ActualPalette,pal,sizeof(ActualPalette));
 ComputeChanged();
}

void TTextPalette::Restore()
{
 TScreen::setPaletteColors(0,16,ActualPalette);
}

/**[txh]********************************************************************
  Description:
  Used when Restore will be called soon and we just want to set a value for
it.
***************************************************************************/

void TTextPalette::PreparePal(PalCol *pal)
{
 memcpy(ActualPalette,pal,sizeof(ActualPalette));
 ComputeChanged();
}

void TTextPalette::Copy(PalCol *dest)
{
 memcpy(dest,ActualPalette,sizeof(ActualPalette));
}

