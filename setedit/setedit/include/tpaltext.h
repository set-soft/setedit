/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
typedef struct
{
 uchar R,G,B;
} PalCol;

class fpstream;

class TTextPalette
{
public:
 TTextPalette();
 ~TTextPalette();

 void suspend();
 void resume();
 void restore() { resume(); }
 void SetOne(int color, int R, int G, int B);
 void GetOne(int color, int &R, int &G, int &B);
 void BackToDefault(void);
 void Save(fpstream &s);
 void Load(fpstream &s);
 void getArray(unsigned *pal);
 PalCol *GetAllPal();
 void SetAllPal(PalCol *);

protected:
 static PalCol OriginalPalette[16];
 static PalCol ActualPalette[16];

 void SetPalette(PalCol *cols);
};

extern TTextPalette *EditorPalette;
#define DeclarePalette TTextPalette *EditorPalette
#define InitPaletteSystem() EditorPalette=new TTextPalette()
#define ShutDownPaletteSystem() delete EditorPalette
#define SuspendPaletteSystem() EditorPalette->suspend()
#define ResumePaletteSystem() EditorPalette->resume()
#define SavePaletteSystem(s) EditorPalette->Save(s)
#define LoadPaletteSystem(s) EditorPalette->Load(s)
#define RestorePaletteSystem() EditorPalette->restore()
#define GetRGBArrayPaletteSystem(a) EditorPalette->getArray(a)
