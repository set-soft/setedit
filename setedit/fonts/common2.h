/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
void SaveSFT(Font &fnt,FILE *f)
{
 fwrite(&fnt,12,1,f);
 fwrite(fnt.font,(fnt.last-fnt.first+1)*fnt.lines,1,f);
}

void SaveFont(Font &fnt)
{
 int i=0,part=1;
 char name[PATH_MAX];
 char bTMP[256*fnt.lines];

 while (i<fnt.last)
   {
    sprintf(name,NAME_PART,part,fnt.lines);
    FILE *f=fopen(name,"wb");
    if (f!=NULL)
      {
       int cant=fnt.last-i+1;
       if (cant>256)
          cant=256;
       memset(bTMP,0,256*fnt.lines);
       memcpy(bTMP,fnt.font+i*fnt.lines,fnt.lines*cant);
       fwrite(bTMP,256*fnt.lines,1,f);
       fclose(f);
      }
    i+=256;
    part++;
   }
}

void SaveFontFull(Font &fnt)
{
 int i=0;
 char name[PATH_MAX];

 sprintf(name,NAME_PART,0,fnt.lines);
 FILE *f=fopen(name,"wb");
 if (f!=NULL)
   {
    fwrite(fnt.font,NUM_FONTS*fnt.lines,1,f);
    fclose(f);
   }
}

void GenSFT(void)
{
 FILE *f=fopen(NAME_FONT,"wb");
 fwrite(Signature,strlen(Signature),1,f);
 int version=1;
 fwrite(&version,4,1,f);
 int cant=2;
 fwrite(&cant,4,1,f);
 short len=strlen(Name)+1;
 fwrite(&len,2,1,f);
 fputs(Name,f);
 SaveSFT(ARRAY1,f);
 SaveSFT(ARRAY2,f);
 fclose(f);
}

void Dump1(int lines, char *buffer,FILE *d)
{
 int i,j;
 unsigned char *s=(unsigned char *)buffer;
 for (i=0; i<256; i++)
    {
     for (j=0; j<lines; j++,s++)
         fprintf(d,"0x%02X,",*s);
     switch (i)
       {
        case 0:
             fprintf(d," // 0\n");
             break;
        case 9:
             fprintf(d," // \\t\n");
             break;
        case 10:
             fprintf(d," // \\n\n");
             break;
        case 13:
             fprintf(d," // \\r\n");
             break;
        case 26:
             fprintf(d," // \\x1A\n");
             break;
        case '\\':
             fprintf(d," // \\.\n");
             break;
        case 255:
             fprintf(d," // 255\n");
             break;
        default:
             fprintf(d," // %c\n",i);
       }
    }
}

void Dump2(int lines, char *buffer, int cant, int base,FILE *d)
{
 int i,j;
 unsigned char *s=(unsigned char *)buffer;
 for (i=0; i<cant; i++)
    {
     for (j=0; j<lines; j++,s++)
         fprintf(d,"0x%02X,",*s);
     fprintf(d," // %d\n",i+base);
    }
}

void GenH1(Font &fnt, FILE *d)
{
 int i=0,part=1;
 char name[PATH_MAX];
 char bTMP[256*fnt.lines];

 fprintf(d,"unsigned char %s%d[]={\n",PREFIXS,fnt.lines);
 while (i<fnt.last)
   {
    sprintf(name,NAME_PART,part,fnt.lines);
    FILE *f=fopen(name,"rb");
    if (f!=NULL)
      {
       int cant=fnt.last-i+1;
       if (cant>256)
          cant=256;
       fread(bTMP,cant*fnt.lines,1,f);
       fclose(f);
       if (i==0)
          Dump1(fnt.lines,bTMP,d);
       else
          Dump2(fnt.lines,bTMP,cant,i,d);
      }
    i+=256;
    part++;
   }
 fprintf(d,"};\n\n");
}

void GenH(void)
{
 char name[PATH_MAX];
 sprintf(name,NAME_PART,0,0);
 FILE *f=fopen(name,"wt");
 if (f)
   {
    GenH1(ARRAY1,f);
    GenH1(ARRAY2,f);
   }
 fclose(f);
}

#if 0
unsigned Added[]=
{'B','E','K','M','H','O','P','C','T','X','a','e','o','p','c','x',
 137,280,'s','i',139,'j',281,'J','S',0};
 // 611 => U+0408 (74==J)
 // 612 => U+0405 (83==S)

void CPatch(Font *f)
{
 unsigned lines=f->lines;
 unsigned char *nfont=new unsigned char [lines*(NUM_FONTS+1)];
 memcpy(nfont,f->font,lines*588);
 int i=0;
 while (Added[i])
   {
    memcpy(nfont+lines*(588+i),nfont+lines*Added[i],lines);
    i++;
   }
 f->font=nfont;
 printf("Copied upto: %d\n",588+i-1);
}

void CyrillicPatch()
{
 CPatch(&ARRAY1);
 CPatch(&ARRAY2);
}
#else
 #define CyrillicPatch()
#endif

#if 1
unsigned Added[]=
{0xE1,0x185,0xE6,'o','A','B','E','Z','H','I','K','M','N','O','P','T','Y','X',
 0x119,0x1C2,0};

void CPatch(Font *f)
{
 unsigned lines=f->lines;
 unsigned char *nfont=new unsigned char [lines*(NUM_FONTS+1)];
 memcpy(nfont,f->font,lines*613);
 int i=0;
 while (Added[i])
   {
    memcpy(nfont+lines*(613+i),nfont+lines*Added[i],lines);
    i++;
   }
 f->font=nfont;
 printf("Copied upto: %d\n",613+i-1);
}

void GreekPatch()
{
 CPatch(&ARRAY1);
 CPatch(&ARRAY2);
}
#else
 #define GreekPatch()
#endif

int main(void)
{
 CyrillicPatch();
 GreekPatch();
 printf("
Choose an action:

1 - Generate .sft from data in the .EXE file (for editor)
2 - Generate .0xx files from data in the .EXE file (for font editor)
3 - Generate .h file to recompile the .EXE
4 - Generate .0xx file with all the fonts in the .EXE file (for tools)

");
 int c=getchar()-'0';

 switch (c)
   {
    case 1:
         GenSFT();
         break;
    case 2:
         SaveFont(ARRAY1);
         SaveFont(ARRAY2);
         break;
    case 3:
         GenH();
         break;
    case 4:
         SaveFontFull(ARRAY1);
         SaveFontFull(ARRAY2);
         break;
    default:
         printf("wrong option!\n");
   }
 return 0;
}
