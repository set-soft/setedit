/**[txh]********************************************************************

  Copyright (c) 2002 by Salvador E. Tropea
  Covered by the GPL License

  Description:
  This program was used to generate .sft files from .pcf font files (XFree86
uses this format).@*
  The code is a hack, I don't even bothered looking for the PCF format, I
just figured out where is located what I need. It works for the fonts I have
and isn't really generic. In fact I solve some missing symbols using
knowledge about the fonts it converts.@*
  If you successfully convert another fonts to the SFT format please
consider contributing them to the Turbo Vision and SETEdit projects.@*
  This code compiles and runs under Linux and most probably is ok for other
POSIX systems. I used gcc specific extensions. The code assumes gzip is
available and uses it to uncompress .gz files, at least in Debian all X
fonts are compressed using gzip.

***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Linux stdlib defines LITTLE_ENDIAN
#define MY_LITTLE_ENDIAN
//#define MY_BIG_ENDIAN
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned u32;

#include "intcode.h"

void SaveChar(int v, FILE *d);
void DrawChar(int v);

const u32 PCF1Magic=0x70636601,typeShapes=8,typeUnicodes=0x20,typeInfo=1;
static FILE *f;
u32 offShapes=0,offUnicode=0,numUnicodes,numShapes,fontW=0,fontH=0,fontWb=0;
u16 *unicodeTable;
int verboseLevel=1,buggyHalfs=0,useForUnknown=-1;
unsigned sizeChar;
typedef struct
{
 int first, last, lines;
 int width;
} Font;

char *Signature="SET's editor font\x1A";
char  Name[512];

#ifdef MY_BIG_ENDIAN
inline
void Swap(u32 *val)
{
 u8 *p=(u8 *)val;
 unsigned aux;
 aux=p[3];
 p[3]=p[0];
 p[0]=aux;
 aux=p[2];
 p[2]=p[1];
 p[1]=aux;
}
#else
inline
void Swap(u32 *)
{
}
#endif

u32 ReadU32()
{
 u32 val;
 fread(&val,4,1,f);
 //printf("0x%08X\n",val);
 Swap(&val);
 return val;
}

void GetName(char *dest, u32 offset)
{
 fseek(f,offset,SEEK_SET);
 int i=0;
 while (i<255)
   {
    dest[i]=fgetc(f);
    if (dest[i]==0) return;
    i++;
   }
 dest[i]=0;
}

void WriteBlank(int l, FILE *d)
{
 int i,c=l*fontWb;
 for (i=0; i<c; i++)
     fputc(0,d);
}

void WriteFull(FILE *d)
{
 int c=fontW;
 while (c>8)
   {
    fputc(0xFF,d);
    c-=8;
   }
 unsigned mask=0x80,acc=0;
 while (c)
   {
    acc|=mask;
    mask>>=1;
    c--;
   }
 fputc(acc,d);
}

void CopyLines(unsigned h, FILE *d)
{
 unsigned i,j,mask,mask2,val,vres;
 u8 c[4];

 for (j=0; j<h; j++)
    {
     fread(c,4,1,f);
     for (i=0; i<fontWb; i++)
        {
         val=c[i];
         vres=0;
         for (mask=1,mask2=0x80; mask2; mask<<=1, mask2>>=1)
             if (val & mask)
                vres|=mask2;
         fputc(vres,d);
        }
    }
}

void CopyLinesLeft(unsigned h, unsigned cut, FILE *d)
{
 unsigned i,j,mask,mask2,vres,k,val;
 u8 c[4];
 //printf("CopyLinesLeft(h=%d,cut=%d)\n",h,cut);

 for (j=0; j<h; j++)
    {
     fread(c,4,1,f);
     if (!c[0])
       {
        fwrite(c,fontWb,1,d);
        //fputc(j,d);
        //fputc(j,d);
        //printf("%d) Writing %d %d fontWb=%d",j,c[0],c[1],fontWb);
       }
     else
        for (k=0, i=0; i<fontWb; i++)
           {
            val=c[i];
            vres=0;
            for (mask=1,mask2=0x80; mask2; k++, mask<<=1, mask2>>=1)
                if (k<cut && (val & mask))
                   vres|=mask2;
            fputc(vres,d);
            //printf("%d) Wrinting 0x%02X ",j,vres);
           }
     //printf("\n");
    }
}

void CopyLinesRight(unsigned h, unsigned cut, FILE *d)
{
 unsigned i,j,mask,mask2,vres,k,val;
 u8 c[4];

 for (j=0; j<h; j++)
    {
     fread(c,4,1,f);
     if (!c[0])
        fwrite(c,fontWb,1,d);
     else
        for (k=0, i=0; i<fontWb; i++)
           {
            val=c[i];
            vres=0;
            for (mask=1,mask2=0x80; mask2; k++, mask<<=1, mask2>>=1)
                if (k>=cut && (val & mask))
                   vres|=mask2;
            fputc(vres,d);
           }
    }
}

u32 VerifyAvailable(int uni, int replace, FILE *d)
{
 if (uni>=numUnicodes)
   {
    printf("Missing U%04X 1\n",replace);
    SaveChar(0,d);
    return 0;
   }
 u32 val=unicodeTable[uni];
 if (val==0xFFFF)
   {
    printf("Missing U%04X 2\n",replace);
    SaveChar(0,d);
    return 0;
   }
 return val;
}

void SeekChar(unsigned val)
{
 fseek(f,offShapes+val*sizeChar,SEEK_SET);
}

void GenerateF801(FILE *d)
{
 unsigned val;
 if (!(val=VerifyAvailable(0x2500,0xF801,d))) return;
 int prev=fontH/4,rest=fontH-prev;
 fseek(f,offShapes+val*sizeChar+prev*4,SEEK_SET);
 printf("Generating a replacement for UF801\n");
 CopyLines(rest,d);
 u8 c[4];
 memset(c,0,4);
 int i;
 for (i=0; i<prev; i++)
     fwrite(c,fontWb,1,d);
}

void GenerateF803(FILE *d)
{
 unsigned val;
 if (!(val=VerifyAvailable(0x2500,0xF803,d))) return;
 int prev=fontH/4,rest=fontH-prev;
 fseek(f,offShapes+val*sizeChar,SEEK_SET);
 printf("Generating a replacement for UF803\n");
 int i;
 u8 c[4];
 memset(c,0,4);
 for (i=0; i<prev; i++)
     fwrite(c,fontWb,1,d);
 CopyLines(rest,d);
}

// BOX DRAWINGS LIGHT LEFT
void Generate2574(FILE *d)
{
 unsigned val;
 if (!(val=VerifyAvailable(0x2500,0x2574,d))) return;
 fseek(f,offShapes+val*sizeChar,SEEK_SET);
 printf("Generating a replacement for U2574\n");
 CopyLinesLeft(fontH,fontW/2,d);
}

// BOX DRAWINGS LIGHT RIGHT
void Generate2576(FILE *d)
{
 unsigned val;
 if (!(val=VerifyAvailable(0x2500,0x2576,d))) return;
 fseek(f,offShapes+val*sizeChar,SEEK_SET);
 printf("Generating a replacement for U2576\n");
 CopyLinesRight(fontH,fontW/2,d);
}

// BOX DRAWINGS LIGHT UP
void Generate2575(FILE *d)
{
 unsigned val;
 if (!(val=VerifyAvailable(0x2502,0x2575,d))) return;
 int prev=fontH/2,rest=fontH-prev;
 fseek(f,offShapes+val*sizeChar,SEEK_SET);
 printf("Generating a replacement for U2575\n");
 CopyLines(prev,d);
 int i;
 u8 c[4];
 memset(c,0,4);
 for (i=0; i<rest; i++)
     fwrite(c,fontWb,1,d);
}

// BOX DRAWINGS LIGHT DOWN
void Generate2577(FILE *d)
{
 unsigned val;
 if (!(val=VerifyAvailable(0x2502,0x2577,d))) return;
 int prev=fontH/2,rest=fontH-prev;
 fseek(f,offShapes+val*sizeChar,SEEK_SET);
 printf("Generating a replacement for U2577\n");
 int i;
 u8 c[4];
 memset(c,0,4);
 for (i=0; i<rest; i++)
     fwrite(c,fontWb,1,d);
 CopyLines(prev,d);
}

int MeassureVert(unsigned val)
{
 SeekChar(val);
 u8 c[4];
 fread(c,4,1,f);
 unsigned i,j=0,mask,mask2;
 int start=-1,end=-1;
 for (i=0; i<fontWb; i++)
    {
     val=c[i];
     for (mask=1,mask2=0x80; mask2; j++, mask<<=1, mask2>>=1)
         if (val & mask)
           {
            if (start==-1) start=j;
           }
         else
           {
            if (start!=-1 && end==-1) end=j;
           }
    }
 return end-start;
}

// [s,e)
void MeassureHoriz2(unsigned val, int &s, int &e)
{
 fseek(f,offShapes+val*sizeChar,SEEK_SET);
 u8 c[4];
 s=0;
 fread(c,4,1,f);
 while (!c[0])
   {
    s++;
    fread(c,4,1,f);
   }
 while (c[0])
   {
    s++;
    fread(c,4,1,f);
   }
 e=s;
 while (!c[0])
   {
    e++;
    fread(c,4,1,f);
   }
}

// [start;end)
void CreateV(u8 *c, int start, int end)
{
 unsigned i,mask2,vres,k;
 memset(c,0,4);

 for (k=0, i=0; i<fontWb; i++)
    {
     vres=0;
     for (mask2=0x80; mask2; k++, mask2>>=1)
         if (k>=start && k<end)
            vres|=mask2;
     c[i]=vres;
    }
}

// BOX DRAWINGS HEAVY LEFT, but Linux uses *DOUBLE*
void GenerateFake2578(FILE *d)
{
 unsigned valV,valH2;
 if (!(valH2=VerifyAvailable(0x2550,0x2578,d))) return;
 if (!(valV=VerifyAvailable(0x2502,0x2578,d))) return;
 int w=MeassureVert(valV);
 int s,e;
 MeassureHoriz2(valH2,s,e);
 printf("Generating a replacement for U2578 Width: %d [%d;%d)\n",w,s,e);
 SeekChar(valH2);
 u8 c[4];
 u32 half=fontW/2,i;
 CreateV(c,half-w,half);
 CopyLinesLeft(s,half,d);
 for (i=s; i<e; i++)
    {
     //printf("%d) Writing %d %d fontWb=%d\n",i,c[0],c[1],fontWb);
     fwrite(c,fontWb,1,d);
    }
 fseek(f,(e-s)*4,SEEK_CUR);
 CopyLinesLeft(fontH-e,half,d);
}

// BOX DRAWINGS HEAVY RIGHT, but Linux uses *DOUBLE*
void GenerateFake257A(FILE *d)
{
 unsigned valV,valH2;
 if (!(valH2=VerifyAvailable(0x2550,0x257A,d))) return;
 if (!(valV=VerifyAvailable(0x2502,0x257A,d))) return;
 int w=MeassureVert(valV);
 int s,e;
 MeassureHoriz2(valH2,s,e);
 printf("Generating a replacement for U257A Width: %d [%d;%d)\n",w,s,e);
 SeekChar(valH2);
 u8 c[4];
 u32 half=fontW/2,i;
 CreateV(c,half,half+w);
 CopyLinesRight(s,half,d);
 for (i=s; i<e; i++)
     fwrite(c,fontWb,1,d);
 fseek(f,(e-s)*4,SEEK_CUR);
 CopyLinesRight(fontH-e,half,d);
}

int MeassureHoriz(unsigned val, FILE *d)
{
 SeekChar(val);
 u8 c[4];
 fread(c,4,1,f);
 while (!c[0]) fread(c,4,1,f);
 int h=0;
 while (c[0])
   {
    h++;
    fread(c,4,1,f);
   }
 return h;
}

void MeassureVert2(unsigned val, int &s, int &e)
{
 SeekChar(val);
 u8 c[4];
 fread(c,4,1,f);

 unsigned i,mask2,k,v,end=0;
 s=e=-1;
 for (k=0, i=0; i<fontWb; i++)
    {
     v=c[i];
     for (mask2=1; mask2; k++, mask2<<=1)
         if (mask2 & v)
           {
            if (s==-1) s=k;
            if (e!=-1) end=e=k;
           }
         else
           {
            if (s!=-1 && e==-1) e=k;
            if (end)
              {
               e=k;
               return;
              }
           }
    }
}

// BOX DRAWINGS HEAVY UP, but Linux uses *DOUBLE*
void GenerateFake2579(FILE *d)
{
 unsigned valV2,valH;
 if (!(valV2=VerifyAvailable(0x2551,0x2579,d))) return;
 if (!(valH=VerifyAvailable(0x2500,0x2579,d))) return;
 int h=MeassureHoriz(valH,d);
 int s,e;
 MeassureVert2(valV2,s,e);
 printf("Generating a replacement for U2579 Height: %d [%d;%d)\n",h,s,e);
 SeekChar(valV2);
 u8 c[4];
 u32 half=fontH/2,rest=fontH-half,i;
 CreateV(c,s,e);
 CopyLines(half-h,d);
 for (i=0; i<h; i++)
     fwrite(c,fontWb,1,d);
 memset(c,0,4);
 for (i=0; i<rest; i++)
     fwrite(c,fontWb,1,d);
}

// BOX DRAWINGS HEAVY DOWN, but Linux uses *DOUBLE*
void GenerateFake257B(FILE *d)
{
 unsigned valV2,valH;
 if (!(valV2=VerifyAvailable(0x2551,0x257B,d))) return;
 if (!(valH=VerifyAvailable(0x2500,0x257B,d))) return;
 int h=MeassureHoriz(valH,d);
 int s,e;
 MeassureVert2(valV2,s,e);
 printf("Generating a replacement for U257B Height: %d [%d;%d)\n",h,s,e);
 SeekChar(valV2);
 u8 c[4];
 u32 half=fontH/2,rest=fontH-half-h,i;
 memset(c,0,4);
 for (i=0; i<half; i++)
     fwrite(c,fontWb,1,d);
 CreateV(c,s,e);
 for (i=0; i<h; i++)
     fwrite(c,fontWb,1,d);
 CopyLines(rest,d);
}

u8 *LoadChar(int v)
{
 SeekChar(v);
 u8 *ch=new u8[fontH*fontW];
 u8 c[4],*d;
 unsigned i,j,mask,mask2,val,vres,k;

 for (d=ch, j=0; j<fontH; j++)
   {
    fread(c,4,1,f);
    for (k=0, i=0; i<4 && k<fontW; i++)
       {
        val=c[i];
        for (mask=1; mask<0x100 && k<fontW; d++, k++, mask<<=1)
           {
            if (val & mask)
               *d=1;
            else
               *d=0;
           }
       }
   }
 return ch;
}

void FillRect(unsigned x1, unsigned y1, unsigned x2, unsigned y2, u8 val, u8 *dest)
{
 unsigned x,y;
 u8 *d=dest+y1*fontW;
 for (y=y1; y<=y2; d+=fontW, y++)
     for (x=x1; x<=x2; x++)
         d[x]=val;
}

void SaveChar(u8 *c, FILE *d)
{
 unsigned h,w,mask,vres,x;

 for (h=0; h<fontH; c+=fontW, h++)
     for (x=0, w=0; w<fontWb; w++)
        {
         for (vres=0, mask=0x80; mask && x<fontW; x++, mask>>=1)
             if (c[x]) vres|=mask;
         fputc(vres,d);
        }
}

void Generate21B5(FILE *df)
{
 unsigned val,valV;
 if (!(val=VerifyAvailable(0x2190,0x21B5,df))) return;
 if (!(valV=VerifyAvailable(0x2502,0x21B5,df))) return;
 int w=MeassureVert(valV);
 unsigned halfW=fontW*3/4-1,qH=fontH/4;
 u8 *c=LoadChar(val);
 int h;
 u8 *d=c;
 for (h=0; h<fontH; d+=fontW, h++)
     if (d[halfW]) break;
 for (; h<fontH; d+=fontW, h++)
     if (!d[halfW]) break;
 printf("Generating a replacement for U21B5 Width: %d [%d;%d)\n",w,qH,h);

 FillRect(halfW,0,fontW-1,fontH-1,0,c);
 FillRect(halfW,qH,halfW+w-1,h-1,1,c);
 SaveChar(c,df);
}

void SaveChar(int v, FILE *d)
{
 if (v==0xF801)
   {
    GenerateF801(d);
    return;
   }
 if (v==0xF803)
   {
    GenerateF803(d);
    return;
   }
 // Terminus have wrong 0x2574-0x2577, BTW: it also have wrong 0x2578-0x257B
 // they aren't the Unicode shapes nor the Linux ones.
 if (buggyHalfs && v==unicodeTable[0x2574])
   {
    Generate2574(d);
    return;
   }
 if (buggyHalfs && v==unicodeTable[0x2575])
   {
    Generate2575(d);
    return;
   }
 if (buggyHalfs && v==unicodeTable[0x2576])
   {
    Generate2576(d);
    return;
   }
 if (buggyHalfs && v==unicodeTable[0x2577])
   {
    Generate2577(d);
    return;
   }
 // What we call 0x2578-0x257B isn't really unicode, just a crazy idea used
 // in Linux console fonts.
 if (v && v==unicodeTable[0x2578])
   {
    GenerateFake2578(d);
    return;
   }
 if (v && v==unicodeTable[0x2579])
   {
    GenerateFake2579(d);
    return;
   }
 if (v && v==unicodeTable[0x257A])
   {
    GenerateFake257A(d);
    return;
   }
 if (v && v==unicodeTable[0x257B])
   {
    GenerateFake257B(d);
    return;
   }

 fseek(f,offShapes+v*sizeChar,SEEK_SET);
 CopyLines(fontH,d);
}

void GenSFT(const char *name, bool append)
{
 FILE *d;
 if (append)
   {
    d=fopen(name,"r+b");
    if (!d)
      {
       printf("Unable to open %s\n",name);
       return;
      }
    fseek(d,strlen(Signature)+4,SEEK_SET);
    u32 cant=1;
    fread(&cant,4,1,d);
    cant++;
    fseek(d,strlen(Signature)+4,SEEK_SET);
    fwrite(&cant,4,1,d);
    fseek(d,0,SEEK_END);
   }
 else
   {
    d=fopen(name,"wb");
    if (!d)
      {
       printf("Unable to create %s\n",name);
       return;
      }
    fwrite(Signature,strlen(Signature),1,d);
    u32 version=2;
    fwrite(&version,4,1,d);
    u32 cant=1;
    fwrite(&cant,4,1,d);
    u16 len=strlen(Name)+1;
    fwrite(&len,2,1,d);
    fputs(Name,d);
   }

 Font fnt={0,InternalMap[providedUnicodes-1].code,fontH,fontW};
 fwrite(&fnt,sizeof(fnt),1,d);

 int ant=-1,uni,code,i,tryNext=0;
 for (i=0; i<providedUnicodes; i++)
    {
     code=InternalMap[i].code;
     if (code==ant && !tryNext) continue;
     if (code!=ant+1 && !tryNext) printf("%d\n",code);
     tryNext=0;
     ant=code;
     uni=InternalMap[i].unicode;
     if (uni>=numUnicodes)
       {
        if (uni==0xF801 || uni==0xF803)
           SaveChar(uni,d);
        else
          {
           if (useForUnknown==-1)
              SaveChar(0,d);
           else
              SaveChar(useForUnknown,d);
           printf("Missing U%04X\n",uni);
          }
       }
     else
       {
        int val=unicodeTable[uni];
        if (val==0xFFFF)
          {
           if (code==InternalMap[i+1].code)
              tryNext=1;
           else
             {
              if (uni==0xF801 || uni==0xF803)
                 SaveChar(uni,d);
              else if (uni==0x21B5)
                {
                 Generate21B5(d);
                }
              else
                {
                 SaveChar(0,d);
                 printf("Missing U%04X\n",uni);
                }
             }
          }
        else
           SaveChar(val,d);
       }
    }
 fclose(d);
 #if 0
 // Terminus font bug
 printf("U2502 %d\n",unicodeTable[0x2502]);
 DrawChar(unicodeTable[0x2502]);
 printf("U2574 %d\n",unicodeTable[0x2574]);
 DrawChar(unicodeTable[0x2574]);
 printf("U2575 %d\n",unicodeTable[0x2575]);
 DrawChar(unicodeTable[0x2575]);
 printf("U2576 %d\n",unicodeTable[0x2576]);
 DrawChar(unicodeTable[0x2576]);
 printf("U2577 %d\n",unicodeTable[0x2577]);
 DrawChar(unicodeTable[0x2577]);
 #endif
}

void DrawChar(int v)
{
 SeekChar(v);
 u8 c[4];
 unsigned i,j,mask,mask2,val,vres,k;

 for (j=0; j<fontH; j++)
   {
    fread(c,4,1,f);
    for (k=0, i=0; i<4; i++)
       {
        val=c[i];
        for (mask=1; mask<0x100; k++, mask<<=1)
           {
            if (val & mask)
               printf("X");
            else
               if (k<fontW)
                  printf(".");
           }
       }
    printf("\n");
   }
}


const char *tempName="/tmp/.pcf1tempo.pcf";

int ReadPCF(const char *s, const char *sft, bool append)
{
 if (strstr(s,".pcf.gz"))
   {
    char b[64+strlen(s)];
    sprintf(b,"gzip -dc %s > %s",s,tempName);
    system(b);
    s=tempName;
   }
 f=fopen(s,"rb");
 if (!f)
   {
    printf("Error opening %d\n",s);
    return -1;
   }
 u32 magic=ReadU32();
 if (magic!=PCF1Magic)
   {
    printf("Not a PCF1 file (0x%08X)\n",magic);
    return 2;
   }
 u32 sections=ReadU32();
 if (verboseLevel>1)
    printf("Contains %d sections\n");
 u32 i,type,flags,size,offset;
 u32 offInfo;
 for (i=0; i<sections; i++)
    {
     type=ReadU32();
     flags=ReadU32();
     size=ReadU32();
     offset=ReadU32();
     if (verboseLevel>1)
        printf("%d) type 0x%08X flags 0x%08X size 0x%08X offset 0x%08X\n",i,type,flags,size,offset);
     if (type==typeShapes)
        offShapes=offset;
     if (type==typeUnicodes)
       {
        offUnicode=offset;
        numUnicodes=size/2;
       }
     if (type==typeInfo)
        offInfo=offset;
    }
 // Information about the font
 fseek(f,offInfo,SEEK_SET);
 flags=ReadU32();
 u32 numDefs=ReadU32();
 // Each definition is 9 bytes, 4  for the flags, 4 for the number of defs and
 // 4 for the lenght of the strings. Then pad it to be in a 32 bits pos.
 u32 offNames=(offInfo+numDefs*9+4+8+3)&(~3);
 if (verboseLevel>1)
    printf("%u Attributes (0x%08X)\n",numDefs,offNames);
 char buf[256];
 Name[0]=0;
 int copy,name;
 for (i=0; i<numDefs; i++)
    {
     u32 offName,Val,pos;
     u8  Type;

     offName=ReadU32();
     Type=fgetc(f);
     Val=ReadU32();
     pos=ftell(f);

     GetName(buf,offNames+offName);
     //printf("0x%08X\n",offNames+offName);
     if (verboseLevel>1) printf("%s: ",buf);
     if (Type)
       {// Is another string
        copy=!strcmp(buf,"COPYRIGHT");
        name=!strncmp(buf,"FAMILY",6) || !strcmp(buf,"WEIGHT_NAME");
        GetName(buf,offNames+Val);
        if (verboseLevel>1)
           printf("%s\n",buf);
        else
          if (copy && verboseLevel)
             printf("Copyright: %s\n",buf);
        if (name)
          {
           if (*Name)
             {
              strcat(Name," ");
              strcat(Name,buf);
             }
           else
              strcpy(Name,buf);
          }
       }
     else
       {// Is an integer
        if (verboseLevel>1) printf("%d\n",Val);
        if (!strcmp(buf,"PIXEL_SIZE")) fontH=Val;
        if (!strcmp(buf,"QUAD_WIDTH"))
          {
           fontW=Val;
           fontWb=(fontW+7)/8;
          }
       }
     fseek(f,pos,SEEK_SET);
     //printf("%d) %d %u(%x) %u(%x)\n",i+1,v,v1,v1,v2,v2);
    }
 if (!fontH || !fontW)
   {
    printf("Unable to find font size\n");
    return 6;
   }
 if (verboseLevel)
    printf("Font size %ux%u\n",fontW,fontH);
 sizeChar=4*fontH;
 // Find the number of shapes:
 if (!offShapes)
   {
    printf("Unable to find the shapes\n");
    return 3;
   }
 fseek(f,offShapes,SEEK_SET);
 flags=ReadU32();
 numShapes=ReadU32();
 offShapes+=8+numShapes*4+16;
 if (verboseLevel)
    printf("Found %u shapes\n",numShapes);
 // Load the unicode table
 if (!offUnicode)
   {
    printf("Unable to find unicode table\n");
    return 4;
   }
 unicodeTable=new u16[numUnicodes];
 fseek(f,offUnicode+4+10,SEEK_SET);
 if (fread(unicodeTable,2,numUnicodes,f)!=numUnicodes)
   {
    printf("Error reading unicode table\n");
    return 5;
   }
 if (verboseLevel)
    printf("Loaded unicode table with %u values\n",numUnicodes);
 // Print them
 if (verboseLevel>1)
   {
    DrawChar(unicodeTable['a']);
    DrawChar(unicodeTable['A']);
   }
 GenSFT(sft,append);
 fclose(f);
 delete[] unicodeTable;
 unlink(tempName);
}


int main(int argc, char *argv[])
{
 #if 1
 useForUnknown=4251;
 buggyHalfs=0;
 ReadPCF("10x20.pcf.gz","XFont.sft",false);
 ReadPCF("9x18.pcf.gz","XFont.sft",true);
 ReadPCF("9x15.pcf.gz","XFont.sft",true);
 ReadPCF("7x13.pcf.gz","XFont.sft",true);
 ReadPCF("6x13.pcf.gz","XFont.sft",true);
 ReadPCF("6x10.pcf.gz","XFont.sft",true);
 ReadPCF("5x8.pcf.gz","XFont.sft",true);
 ReadPCF("5x7.pcf.gz","XFont.sft",true);
 #endif

 #if 1
 // Terminus Bold
 useForUnknown=-1;
 buggyHalfs=1;
 ReadPCF("ter-u14b.pcf.gz","BTerminus.sft",false);
 ReadPCF("ter-u16b.pcf.gz","BTerminus.sft",true);
 ReadPCF("ter-u20b.pcf.gz","BTerminus.sft",true);
 // Terminus Regular
 ReadPCF("ter-u14n.pcf.gz","Terminus.sft",false);
 ReadPCF("ter-u16n.pcf.gz","Terminus.sft",true);
 ReadPCF("ter-u20n.pcf.gz","Terminus.sft",true);
 #endif
 return 0;
}
