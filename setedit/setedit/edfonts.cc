/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_stdio
#define Uses_string
#define Uses_stdlib
#define Uses_alloca
#define Uses_limits
#define Uses_dirent
#define Uses_TScreen
#define Uses_TNSCollection
#define Uses_fpstream
#define Uses_TStringCollection
#define Uses_TCEditor_Internal // For isWordChar
#include <ceditor.h>

#include <codepage.h>

typedef struct
{
 int first, last, lines;
 unsigned char *fontFull; // 354 chars
 unsigned char *font;     // 256 according to the code page
} Font;

class TFontColl : public TNSCollection
{
public:
 TFontColl(ccIndex aLimit, ccIndex aDelta) : TNSCollection(aLimit,aDelta) {};

 virtual void freeItem(void *item);
};

void TFontColl::freeItem(void *item)
{
 Font *p=(Font *)item;
 if (p)
   {
    delete[] p->font;
    delete[] p->fontFull;
    delete p;
   }
}

Font *Search(int height);
static TFontColl *curPrimF=0;
static TFontColl *curSecoF=0;

/*class TFontEditor : public TFont
{
public:
 TFontEditor(TFontColl *primary, TFontColl *secondary) :
    FontPrimary(primary), FontSecondary(secondary), lastFont(0)
    {
     curPrimF=primary;
     curSecoF=secondary;
    }
 ~TFontEditor();
 virtual int SelectFont(int height, int width=8, int bank=0, int noForce=0, int modeRecalculate=1);
 virtual void RestoreDefaultFont(void);
 void SetFont(TFontColl *fCol, int height, int bank, int noForce, int modeRecalculate);

protected:
 TFontColl *FontPrimary;
 TFontColl *FontSecondary;
 int lastFont;
};

TFontEditor::~TFontEditor()
{
 destroy(FontPrimary);
 destroy(FontSecondary);
 curPrimF=curSecoF=0;
}*/

#if 0
static
Boolean LookFor(void *item, void *arg)
{
 Font *p=(Font *)item;
 if (p && p->lines==*((int *)arg))
    return True;
 return False;
}

static
void ReduceFontInOne(uchar *dest, uchar *ori, int num, int height)
{
 int i;
 // height is the dest size, ori is one more

 for (i=0; i<num; i++)
    {
     if (isWordChar(i) && ori[0])
       { // first pixel is used
        memcpy(dest,ori,height);
       }
     else
       { // first free
        memcpy(dest,ori+1,height);
       }

     ori+=height+1;
     dest+=height;
    }
}

static
void EnlargeFontInOne(uchar *dest, uchar *ori, int num, int height)
{
 int i;
 // height is the dest size, ori is one less

 for (i=0; i<num; i++)
    {
     memcpy(dest,ori,height);
     dest[height]=0;

     ori+=height-1;
     dest+=height;
    }
}
#endif


#if 0
#include <dpmi.h>
#include <sys/movedata.h>
#include <go32.h>
#include <pc.h>

/**[txh]********************************************************************

  Description:
  That selects what fonts are used for no/intense foreground colors.@p
  The bits of the call are a little tweaked because it's EGA compatible:@*
---VGA---@*
 0,1,4 block selected by characters with attribute bit 3 clear@*
 2,3,5 block selected by characters with attribute bit 3 set@*

***************************************************************************/

static
void SetDualCharacter(int b1, int b2)
{
 int value;
 // No intense foreground font
 value=b1 & 0x3;
 if (b1 & 0x4)
    value|=0x10;
 // intense foreground font
 value|=(b2 & 0x3)<<2;
 if (b2 & 0x4)
    value|=0x20;

 __dpmi_regs regs;
 regs.x.ax = 0x1103;
 regs.h.bl = value;
 __dpmi_int(0x10, &regs);
}

void TFontEditor::SetFont(TFontColl *fCol, int height, int bank, int noForce, int modeRecalculate)
{
 int oneMore=0,oneLess=0;

 Font *p=(Font *)(fCol->firstThat(LookFor,(void *)&height));
 // If we can find a font of the right size look for 1 more and one less
 if (!p)
   {
    height++;
    p=(Font *)(fCol->firstThat(LookFor,(void *)&height));
    if (p)
       oneMore=1;
    else
      {
       height-=2;
       p=(Font *)(fCol->firstThat(LookFor,(void *)&height));
       if (p)
          oneLess=1;
      }
   }

 if (p)
   {
    __dpmi_regs regs;

    uchar *fontShape=p->font;
    if (oneMore)
      {
       height--;
       fontShape=(uchar *)alloca(256*height);
       ReduceFontInOne(fontShape,p->font,256,height);
      }
    else
       if (oneLess)
         {
          height++;
          fontShape=(uchar *)alloca(256*height);
          EnlargeFontInOne(fontShape,p->font,256,height);
         }

    dosmemput(fontShape,256*height,__tb);
    regs.x.es = __tb>>4;    /* pass pointer to our font in ES:BP */
    regs.x.bp = __tb & 0xF;
    regs.x.dx = p->first;   /* 1st char: ASCII 0 */
    regs.x.cx = 256;        /* 256 chars */
    regs.h.bh = height;     /* points per char */
    regs.h.bl = bank;       /* block */
    regs.x.ax = 0x1110;
    if (!modeRecalculate)
       regs.h.al&=0xF;
    __dpmi_int(0x10, &regs);
    //outportw(0x3c4,0x101);
    lastFont=height;
   }
 else
   {
    // Not found
    TFont::SelectFont(height,8,bank,noForce,modeRecalculate);
   }
}
#endif
#if 0 // DJGPP
static
void SetDualCharacter(int , int ) {}

void TFontEditor::SetFont(TFontColl *fCol, int height, int bank, int noForce,
                          int modeRecalculate)
{
 int oneMore=0,oneLess=0;

 Font *p=(Font *)(fCol->firstThat(LookFor,(void *)&height));
 // If we can find a font of the right size look for 1 more and one less
 if (!p)
   {
    height++;
    p=(Font *)(fCol->firstThat(LookFor,(void *)&height));
    if (p)
       oneMore=1;
    else
      {
       height-=2;
       p=(Font *)(fCol->firstThat(LookFor,(void *)&height));
       if (p)
          oneLess=1;
      }
   }

 if (p)
   {
    uchar *fontShape=p->font;
    if (oneMore)
      {
       height--;
       fontShape=(uchar *)alloca(256*height);
       ReduceFontInOne(fontShape,p->font,256,height);
      }
    else
       if (oneLess)
         {
          height++;
          fontShape=(uchar *)alloca(256*height);
          EnlargeFontInOne(fontShape,p->font,256,height);
         }

    /* Set the font here like in DOS */
    lastFont=height;
   }
 else
   {
    // Not found
    TFont::SelectFont(height,8,bank,noForce,modeRecalculate);
   }
}
#endif

#if 0
int TFontEditor::SelectFont(int height, int width, int , int noForce, int modeRecalculate)
{
 lastFont=0;
 if (UseDefaultFontsNextTime)
   {
    // Disable it ;-)
    SetDualCharacter(0,0);
    return TFont::SelectFont(height,width);
   }

 if (width!=8)
    return 1;

 SetFont(FontPrimary,height,0,noForce,modeRecalculate);
 if (FontSecondary)
   {
    SetFont(FontSecondary,height,1,noForce,modeRecalculate);
    SetDualCharacter(1,0);
   }
 else
    SetDualCharacter(0,0);
 return 0;
}

void TFontEditor::RestoreDefaultFont(void)
{
 if (lastFont)
   {
    TFont::SelectFont(lastFont);
    lastFont=0;
   }
}
#endif

static char Signature[]="SET's editor font\x1A";

static
int CheckSignature(FILE *f)
{
 char buf[sizeof(Signature)];
 fread(buf,sizeof(Signature)-1,1,f);
 buf[sizeof(Signature)-1]=0;
 return strcmp(Signature,buf)==0;
}

static
char *ReadName(FILE *f)
{
 short strLen;
 fread(&strLen,2,1,f);
 char *aux=new char[strLen];
 strLen--;
 fread(aux,strLen,1,f);
 aux[strLen]=0;
 return aux;
}

const int SizeInDisk=sizeof(Font)-2*sizeof(unsigned char *);
static char *PrimaryFontFile=0;
static char *PrimaryFontName=0;
static char *SecondaryFontFile=0;
static char *SecondaryFontName=0;
static int PageFont1=-1,PageFont2=-1;

static
void createFont(void *item, void *arg)
{
 Font *f=(Font *)item;
 unsigned short *map=(unsigned short *)arg;
 delete f->font;
 f->font=new unsigned char[256*f->lines];
 int i;
 unsigned char *dest=f->font;
 for (i=0; i<256; i++,dest+=f->lines)
     memcpy(dest,&f->fontFull[map[i]*f->lines],f->lines);
 #if 0
 // This code stores the generated font to disk, is used for debug
 // purposes
 FILE *F;
 char b[PATH_MAX],*t;
 t=getenv("TMP");
 sprintf(b,"%s/font.%03d",t,f->lines);
 F=fopen(b,"wb");
 fwrite(f->font,256*f->lines,1,F);
 fclose(F);
 #endif
}

static
void SetCodePageOf(TFontColl *fcol, unsigned short *map)
{
 fcol->forEach(createFont,map);
}

void ChangeEncodingOfFont(int font, int newEncode)
{
 if (font==1)
   {
    if (curPrimF)
       SetCodePageOf(curPrimF,GetCodePage128Translate(newEncode));
    RemapCharactersFor(newEncode);
    PageFont1=newEncode;
   }
 else
 if (font==2)
   {
    if (curSecoF)
       SetCodePageOf(curSecoF,GetCodePage128Translate(newEncode));
    PageFont2=newEncode;
   }
}

static
int LoadEditorFont(char *name, char *&storeFile, char *&storeName,
                   TFontColl *&FontsLoaded, int CodePage)
{
 if (!name)
   { // Back to BIOS fonts
    delete storeFile;
    delete storeName;
    storeFile=storeName=0;
    /*TFont *font=new TFont();
    TDisplay::SetFontHandler(font);*/
    return 0;
   }

 FILE *f=fopen(name,"rb");
 if (!f)
    return 2;

 if (!CheckSignature(f))
   {
    fclose(f);
    return 3;
   }

 int version;
 fread(&version,sizeof(version),1,f);
 int numfonts;
 fread(&numfonts,sizeof(numfonts),1,f);
 FontsLoaded=new TFontColl(numfonts,1);
 if (!FontsLoaded)
    return 1;
 unsigned short *FontMap=GetCodePage128Translate(CodePage);
 if (!FontMap)
    FontMap=GetCodePage128Translate(437);

 int i;
 Font *p;
 unsigned char *fData;
 unsigned size;

 // Replace the name
 delete storeName;
 storeName=ReadName(f);
 delete storeFile;
 storeFile=newStr(name);

 for (i=0; i<numfonts; i++)
    {
     p=new Font;
     fread(p,SizeInDisk,1,f);
     size=(p->last-p->first+1)*p->lines;
     fData=new unsigned char[size];
     fread(fData,size,1,f);
     p->fontFull=fData;
     p->font=0;
     FontsLoaded->insert(p);
    }
 fclose(f);
 SetCodePageOf(FontsLoaded,FontMap);

 return 0;
}

int LoadEditorFonts(char *primary, char *secondary, int cp1, int cp2)
{
 TFontColl *primF,*secoF;

 // Load the first font
 if (LoadEditorFont(primary,PrimaryFontFile,PrimaryFontName,primF,cp1)==0)
   {
    PageFont1=cp1;
    PageFont2=cp2;
    RemapCharactersFor(cp1);
    if (!primary)
       return 0;
    // Now the second if it's specified
    if (secondary)
      {
       if (LoadEditorFont(secondary,SecondaryFontFile,SecondaryFontName,secoF,cp2)!=0)
         { // Return with error
          destroy(primF);
          return 1;
         }
      }
    else
      {
       secoF=0;
       delete SecondaryFontFile;
       delete SecondaryFontName;
       SecondaryFontFile=SecondaryFontName=0;
      }
    // Create a new object for these fonts
    //TFontEditor *font=new TFontEditor(primF,secoF);
    // Replace the default, the changes will take effect in the next screen mode
    // setting
    //TDisplay::SetFontHandler(font);
   }
 return 0;
}

void UnLoadEditorFonts(void)
{
 delete[] PrimaryFontFile;
 delete[] PrimaryFontName;
 delete SecondaryFontFile;
 delete SecondaryFontName;
 PrimaryFontFile=0;
 PrimaryFontName=0;
 SecondaryFontFile=0;
 SecondaryFontName=0;
}

void SaveFontLoadedInfo(fpstream& s)
{
 // version
 s << (char)4;
 // The following could happend if the editor wasn't able to load the desktop file
 // for example if there is no one in SET_FILES and current dir (first time)
 if (PageFont1<0)
    PageFont1=GetCurrentOSCodePage();
 if (PageFont2<0)
    PageFont2=PageFont1;
 s << PageFont1 << PageFont2;
 if (PrimaryFontFile)
   {
    s << (char)1;
    s.writeString(PrimaryFontFile);
    if (SecondaryFontFile)
      {
       s << (char)1;
       s.writeString(SecondaryFontFile);
      }
    else
       s << (char)0;
   }
 else
   s << (char)0;
}

void LoadFontLoadedInfo(fpstream& s)
{
 char version,aux;
 s >> version;
 int cp1,cp2;

 if (version && version<=3)
   {
    char *namePrim=s.readString();
    if (version>=3)
       s >> cp1;
    else
       cp2=cp1=GetCurrentOSCodePage();
    char *nameSeco=0;

    if (version>=2)
      {
       s >> aux;
       if (aux)
         {
          nameSeco=s.readString();
          if (version>=3)
             s >> cp2;
         }
      }
    LoadEditorFonts(namePrim,nameSeco,cp1,cp2);
    delete[] namePrim;
    delete[] nameSeco;
   }
 else
 if (version>=4)
   {
    char *namePrim=0;
    char *nameSeco=0;
    s >> cp1 >> cp2;
    s >> aux;
    if (aux)
      {
       namePrim=s.readString();
       s >> aux;
       if (aux)
          nameSeco=s.readString();
      }
    LoadEditorFonts(namePrim,nameSeco,cp1,cp2);
    delete[] namePrim;
    delete[] nameSeco;
   }
 else
   {
    cp2=cp1=GetCurrentOSCodePage();
    LoadEditorFonts(0,0,cp1,cp2);
   }
}

TStringCollection *CreateListOfFonts(char *dir, uint32 &IsPrimOn, ccIndex &indexPrim,
                                     uint32 &IsSecoOn, ccIndex &indexSeco)
{
 char FullName[PATH_MAX];
 TStringCollection *col=new TStringCollection(8,4);

 if (!dir)
    dir=".";
 DIR *d=opendir(dir);
 struct dirent *de;
 if (d)
   {
    while ((de=readdir(d))!=0)
      {
       if (strstr(de->d_name,".sft"))
         {
          strcpy(FullName,dir);
          strcat(FullName,"/");
          strcat(FullName,de->d_name);
          FILE *f=fopen(FullName,"rb");
          if (f)
            {
             if (CheckSignature(f))
               {
                fseek(f,2*sizeof(int),SEEK_CUR);
                char *name=ReadName(f);
                int len=strlen(name)+1;
                name=(char *)realloc(name,len+strlen(FullName)+1);
                strcpy(name+len,FullName);
                col->insert(name);
               }
             fclose(f);
            }
         }
      }
    closedir(d);
   }
 IsPrimOn=PrimaryFontName ? 1 : 0;
 indexPrim=0;
 indexSeco=0;
 if (PrimaryFontName)
    if (col->search(PrimaryFontName,indexPrim)==False)
       indexPrim=0;
 if (IsPrimOn)
   {
    IsSecoOn=SecondaryFontName ? 1 : 0;
    indexSeco=0;
    if (SecondaryFontName)
       if (col->search(SecondaryFontName,indexSeco)==False)
          indexSeco=0;
   }
 else
    IsSecoOn=0;
 return col;
}

int GetCodePageFont(int font)
{
 int ret=-1;
 if (font==1)
    ret=PageFont1;
 else
   if (font==2)
      ret=PageFont2;
 if (ret==-1)
    ret=GetCurrentOSCodePage();
 return ret;
}
/*#else
#define Uses_fpstream
#include <ceditor.h>

void SaveFontLoadedInfo(fpstream& s)
{
 s << (char)4;
 s << 88791 << 88791;
 s << (char)1;
 s.writeString("ROM BIOS");
 s << (char)0;
}

void LoadFontLoadedInfo(fpstream& s)
{
 char version,aux;
 s >> version;
 int cp1,cp2;

 if (version && version<=3)
   {
    char *namePrim=s.readString();
    if (version>=3)
       s >> cp1;
    else
       cp2=cp1=88791;
    char *nameSeco=0;

    if (version>=2)
      {
       s >> aux;
       if (aux)
         {
          nameSeco=s.readString();
          if (version>=3)
             s >> cp2;
         }
      }
    delete namePrim;
    delete nameSeco;
   }
 else
 if (version>=4)
   {
    char *namePrim=0;
    char *nameSeco=0;
    s >> cp1 >> cp2;
    s >> aux;
    if (aux)
      {
       namePrim=s.readString();
       s >> aux;
       if (aux)
          nameSeco=s.readString();
      }
    delete namePrim;
    delete nameSeco;
   }
 else
   {
    cp2=cp1=88791;
   }
}
#endif*/
