/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
typedef TScreenColor PalCol;
class fpstream;

class TTextPalette
{
public:
 TTextPalette() {};

 static void SetOne(int color, int R, int G, int B);
 static void GetOne(int color, int &R, int &G, int &B);
 static void BackToDefault(void);
 static void Load(fpstream &s, PalCol *pal=NULL);
 static void GetArray(unsigned *pal);
 static PalCol *GetAllPal();
 static void Copy(PalCol *dest);
 static void SetAllPal(PalCol *);
 static void Restore();
 static void Cache();
 static void ComputeChanged();
 static void PreparePal(PalCol *pal);
 static char IsChanged() { return Changed; };

protected:
 static PalCol ActualPalette[16];
 static char   Changed;
};

extern TTextPalette *EditorPalette;
#define DeclarePalette                TTextPalette *EditorPalette
#define InitPaletteSystem()           EditorPalette=new TTextPalette()
#define ShutDownPaletteSystem()       delete EditorPalette
#define LoadPaletteSystem(s)          EditorPalette->Load(s)
#define LoadPaletteSystemDontSet(s,p) EditorPalette->Load(s,p)
#define GetRGBArrayPaletteSystem(a)   EditorPalette->GetArray(a)
#define RestorePaletteSystem()        EditorPalette->Restore()
#define CacheActualPalette()          EditorPalette->Cache()
#define ComputeChangedStatus()        EditorPalette->ComputeChanged()
#define PreparePalForRestore(p)       EditorPalette->PreparePal(p)
#define PaletteWasChanged()           EditorPalette->IsChanged()
#define PaletteCopy(p)                EditorPalette->Copy(p)

