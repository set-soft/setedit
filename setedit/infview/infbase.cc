/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/***************************************************************

 Filename -       infbase.cc

 Functions
 Member functions of following classes
                      TInfTopic
                      TInfIndex
                      TInfFile

***************************************************************/

// That's the first include because is used to configure the editor.
#include <ceditint.h>

#define CUT_NOTES

#include <stdio.h>
#define Uses_stdlib
#define Uses_string
#define Uses_unistd
#define Uses_TStreamableClass
#define Uses_TPoint
#define Uses_TStreamable
#define Uses_TRect
#define Uses_MsgBox
#define Uses_TStringCollection
#define Uses_TSortedListBox
#define Uses_sys_stat
#define Uses_alloca
#define Uses_ctype
#define Uses_limits
#define Uses_filelength
#define Uses_TVCodePage
// InfView requests
#include <infbaser.h>
#include <settvuti.h>

#include <fcntl.h> // open
#include <gzfiles.h>
#include <dyncat.h>

#ifdef TVCompf_djgpp
#include <crt0.h>
#endif
#ifndef TVOS_UNIX
# include <io.h>
#endif

#include <tprogdia.h>
#include <infbase.h>
#include <rhutils.h> // unique_name

//#define DEBUG
#ifdef DEBUG
# ifndef FOR_EDITOR
# include "nointl.h"
# endif
# ifndef Linux
#  include <conio.h>
#  define Clava(a) { printf(a); getch(); exit(1); }
# else
#  define Clava(a) { fprintf(stderr,a); exit(1); }
# endif
#else
#define Clava(a) exit(1)
#endif

#define CheckClassError(a) {if(a) { Status=True; return; }}

const int TabSize=8;

/*
void *operator new(size_t size)
{
 void * p;

 size = size ? size : 1;
 p = malloc(size);
 if (p==NULL)
   {
    printf(_("Error memory exhausted!!!\n"));
    getch();
    exit(1);
   }
 memset(p,1,size);

 return p;
}

void operator delete(void *p)
{
 if (p==NULL)
   {
    printf(_("Error: pointer is NULL!!!\n"));
    getch();
    long pp=((long *)p)[0];
    printf("%lX\n",pp);
   }
 else
 free(p);
}
*/
// TInfTopic

void *TInfTopic::Read(TInfFile &File, int offset, int &y)
{
 int hadOffset=offset;
 offset-=ReadNodeInfo(File);
 if (!Status)
   {
    ReadText(File,hadOffset ? offset : 0,y);
    ReadCrossRefs();
   }
 width = 0;
 lastLine = INT_MAX;
 return this;
}


static void CopiarDatosDe(char *Tipo,char *Dest,char *Ori,int maxlen)
{
 char *pos,*s;
 int largo;

 if ((pos=strstr(Ori,Tipo))!=NULL)
   {
    pos+=strlen(Tipo)+2;
    if (pos-Ori>=maxlen)
      {
       *Dest=0;
       return;
      }
    for (s=pos,largo=0; *s && *s!='\n' && *s!='\r' && *s!=',' && *s!='\t'; s++,largo++);
    *s=0;
    if (largo>MAX_NODE_NAME)
       Dest[0]=0;
    else
       strcpy(Dest,pos);
    *s=',';
   }
 else
   Dest[0]=0;
}

static inline
int isWord(int a)
{
 return TVCodePage::isAlNum(a) || a=='_';
}

#ifdef InfV_UseBinaryFile
void TInfFile::GetLine(void)
{
 if (fgets(Buffer,SizeOfReadBuf,stream))
   {
    int l = strlen(Buffer);
    if (l > 1 && Buffer[l-2] == '\r' && Buffer[l-1] == '\n')
      {
       Buffer[l-2] = '\n';
       Buffer[l-1] = 0;
      }
   }
}
#endif

/* Added by Robert to seek for a Node with brute-force */
int TInfFile::seekToNode(const char *Name, int fromStart)
{
 long Pos;
 int found = 0;
 int lName = strlen(Name);

 if (fromStart)
    fseek(stream,0,SEEK_SET);

 do
  {
   Pos = ftell(stream);
   GetLine();
   if (feof(stream))
      return 0;
   if (strncmp(Buffer,"File",4)==0)
     {
      char *node=strstr(Buffer,"Node:");
      if (node)
        {
         if (strncmp(node+6,Name,lName)==0 && !isWord(node[6+lName])) // ispunct ?
            found=1;
        }
     }
  }
 while (!found);
 fseek(stream,Pos,SEEK_SET);
 return 1;
}

int TInfTopic::ReadNodeInfo(TInfFile &File)
{
 char *Buf;
 FILE *f=File.stream;
 long Pos=ftell(f);
 int  isFile, isDOS=0, ret=0;

 Buf=File.Buffer;
 do
   { File.GetLine(); }
 while(!feof(f) && (Buf[0]==31 || ucisspace(Buf[0])));

 // Make it a little robust
 isFile= strncmp(Buf,"File",4)==0;
 if (!isFile)
   {
    isDOS= Buf[strlen(Buf)-2]=='\r';
    if (!isDOS)
      {
       // Try around the position
       Pos-=100;
       if (Pos<0) Pos=0;
       fseek(f,Pos,SEEK_SET);
       do
         { File.GetLine(); }
       while(Buf[0]!=31 && !feof(f));
       if (Buf[0]==31)
         {
          do
            { File.GetLine(); }
          while(Buf[0]==31 || ucisspace(Buf[0]));
         }
       isFile= strncmp(Buf,"File",4)==0;
      }
   }

 if (isFile)
   {
    ret=strlen(Buf);
    CopiarDatosDe("Node",Node,Buf,File.SizeOfReadBuf);
    CopiarDatosDe("Next",Next,Buf,File.SizeOfReadBuf);
    CopiarDatosDe("Prev",Prev,Buf,File.SizeOfReadBuf);
    CopiarDatosDe("Up",Up,Buf,File.SizeOfReadBuf);
   }
 else
   {
    if (isDOS)
      {
       if (File.IsCompressed)
         {
          //messageBox(__("Attention !! this file is in DOS format,"
          //              "and is compressed, uncompress the file,"
          //              " reopen it with the Help and then recompress it."),
          //              mfError | mfOKButton);
          if (!File.ConvertIt(Pos))
            {
             Status=True;
             return ret;
            }
          ReadNodeInfo(File);
          return ret;
         }
       else
         {
          if (messageBox(__("Attention !! This file is in DOS format,"
              " Do you want to convert it?,"
              " But maybe make a backup first !!"), mfError | mfYesButton | mfNoButton)
              ==cmYes)
            {
             if (!File.ConvertIt(Pos))
               {
                Status=True;
                return ret;
               }
             ReadNodeInfo(File);
             return ret;
            }
         }
       Status=True;
       return ret;
      }
    messageBox(__("Error: can't find the Node info"),mfError | mfOKButton);
    Status=True;
    return ret;
   }

 return ret;
}

TInfTopic::TInfTopic( int mode )
{
 numRefs = 0;
 crossRefs = 0;
 width = 0;
 lastOffset = 0;
 lastLine = INT_MAX;
 Text = NULL;
 Node[0]=Next[0]=Prev[0]=Up[0]=0;
 Status=False; // OK
 modeFlags=mode;
};

void TInfTopic::ReadText(TInfFile &File, int offset, int &y)
{
 char *s,*d;
 int i,x,t,yFound=0;
 int lWidth;
 FILE *f=File.stream;
 char *Buf=File.Buffer;

 // Primera pasada: mide
 long Pos=ftell(f);
 lSize=0;
 iLines=0;
 File.GetLine();
 maxLineWidth=0;
 y=0;
 do
  {
   for (lWidth=0, s=Buf; *s; s++)
      {
       if (*s=='\t')
          lWidth+=TabSize;
       else
          lWidth++;
      }
   int len=s-Buf;
   if (!yFound)
     {
      if (offset<len)
         yFound=1;
      else
        {
         offset-=len;
         y++;
        }
     }
   if (lWidth>maxLineWidth)
      maxLineWidth=lWidth;
   lSize+=lWidth;
   iLines++;
   File.GetLine();
  }
 while (Buf[0]!=31 && !feof(f));

 // Arriba
 fseek(f,Pos,SEEK_SET);
 Text = new char[lSize+1];
 for (i=iLines, d=Text; i; --i)
    {
     File.GetLine();
     for (s=Buf,x=0;*s;s++)
         if (*s=='\t')
           {
            t=TabSize-(x%TabSize);
            x+=t; 
            for (int j=t;j;--j) *d++=' ';
           } 
         else
           {
            x++;
            if (*s!='\r')
               *d++=*s;
           }
    }
 *d=0;
}


static int IsDelimiterDeXX( int Val )
{
 return Val==' ' || Val=='\r' || Val=='\t' || Val=='\n' || Val=='.' ||
        Val==0  || Val==',' || Val==';' || Val==')'  || Val=='(' || Val<0;
}


//   Esta es la versi¢n m s conservadora, la mantengo para zafar en caso de que la
// otra falle.
static void TakeName(char *Buf,char *Nom,int &ini,int &largo,int &Linea, int &linebreak)
{
 char *s,*ori,*s_pos,*ori_pos;
 int CruzaLin,OffEnProxLin=0,EnParentesis,warnpos=0,ini_pos;

 ini+=1;
 largo=0;
 CruzaLin=0;
 // Quitar espacios indeceables
 for (ori=Buf+1; ucisspace(*ori) && *ori; ori++)
    {
     if (*ori=='\n')
       {
        // Opss se cay¢
        Linea++;
        ini=0;
       }
     else
        ini++;
    }
 // El primer : puesto ac  no cuenta
 if (*ori==':')
   {
    ori++;
    ini++;
   }
 s_pos=Nom;
 ori_pos=ori;
 ini_pos=ini;
 for (s=Nom; *ori; s++,ori++)
    {
     if (*ori==':')
       {
        if ((!TVCodePage::isAlpha(ori[1]) && ori[1]!=':') || ( ori[1]==':' && IsDelimiterDeXX(ori[2]) ) )
           break;
        if (warnpos)
           break;
        warnpos=1;
        *s++=*ori++;
        largo++;
       }
     else
     if (*ori=='.' || *ori==',' || *ori=='*')
       { // Ups! the file isn't OK.
        if (warnpos>0)
          { // Perhaps the idiot forgot to put an space after ::
           // un-roll all
           largo=1;
           CruzaLin=OffEnProxLin=0;
           s=s_pos;
           ori=ori_pos;
           ini=ini_pos;
           continue;
          }
        //break;
       }
     if (*ori=='\n')
       {
        *s=' ';
        for (OffEnProxLin=0;*ori && ucisspace(*ori); ori++,OffEnProxLin++,largo++);
        ori--;
        CruzaLin=1;
        //largo++;
       }
     else
       {
        if (CruzaLin)
           OffEnProxLin++;
        largo++;
        *s=*ori;
       }
    }
 *s=0;
 ori++;
 linebreak=OffEnProxLin;
 if (*ori==':') return;
 if (CruzaLin)
   {
    CruzaLin=0;
    Linea++;
    ini=++OffEnProxLin;
    OffEnProxLin=0;
   }
 else
    ini+=largo;
 largo=0;
 for (ini++;*ori && ucisspace(*ori); ori++,ini++)
    {
     if (*ori=='\n')
       {
        Linea++;
        ini=-1;
       }
    }
 EnParentesis=0;
 for (s=Nom; *ori && *ori!=':' && (*ori!='.' || EnParentesis || !IsDelimiterDeXX(*(ori+1))) && *ori!=','; s++,ori++)
    {
     if (*ori=='\n')
       {
        *s++=' ';
        ori++;
        for (OffEnProxLin=0;*ori && ucisspace(*ori); ori++,OffEnProxLin++,largo++);
        CruzaLin=1;
        largo++;
       }
     else
       {
        if (CruzaLin)
           OffEnProxLin++;
        largo++;
        if (*ori=='(')
           EnParentesis=1;
        else
          if (EnParentesis && *ori==')')
             EnParentesis=0;
       }
     *s=*ori;
    }
 *s=0;
 linebreak=OffEnProxLin;
}


// Esta es un poco loca, pero el resultado es mucho mejor
static void TakeName(char *Buf,char *Nom,char *Nom2,int &ini,int &largo,int &Linea,
                     int &linebreak,char *Origin, long &Size, char *&Fin)
{
 char *cutFrom;
 char *s,*ori,*s_pos,*ori_pos;
 int CruzaLin,OffEnProxLin=0,EnParentesis,warnpos=0,ini_pos,cutCross=0;


 ini+=1;
 largo=0;
 CruzaLin=0;
 // Quitar espacios indeseables
 for (ori=Buf+1; ucisspace(*ori) && *ori; ori++)
    {
     if (*ori=='\n')
       {
        // Opss se cay¢
        Linea++;
        ini=0;
       }
     else
        ini++;
    }
 // El primer : puesto ac  no cuenta
 if (*ori==':')
   {
    ori++;
    ini++;
   }
 s_pos=Nom;
 ori_pos=ori;
 ini_pos=ini;
 for (s=Nom; *ori; s++,ori++)
    {
     if (*ori==':')
       {
        if ((!TVCodePage::isAlpha(ori[1]) && ori[1]!=':') || ( ori[1]==':' && IsDelimiterDeXX(ori[2]) ) )
           break;
        if (warnpos)
           break;
        warnpos=1;
        *s++=*ori++;
        largo++;
       }
     else
     if (*ori=='.' || *ori==',' || *ori=='*')
       { // Ups! the file isn't OK.
        if (warnpos>0)
          { // Perhaps the idiot forgot to put an space after ::
           // un-roll all
           largo=1;
           CruzaLin=OffEnProxLin=0;
           s=s_pos;
           ori=ori_pos;
           ini=ini_pos;
           continue;
          }
        //break;
       }
     if (*ori=='\n')
       {
        *s=' ';
        for (OffEnProxLin=0;*ori && ucisspace(*ori); ori++,OffEnProxLin++,largo++);
        ori--;
        CruzaLin=1;
        //largo++;
       }
     else
       {
        if (CruzaLin)
           OffEnProxLin++;
        largo++;
        *s=*ori;
       }
    }
 *s=0;
 ori++;
 linebreak=OffEnProxLin;
 strncpy(Nom2,Nom,MAX_NODE_NAME-1);
 if (*ori==':')
   {
    ori--;
    memmove(ori,ori+2,size_t(Size-(ori-Origin)-2+1));
    Size-=2;
    Fin-=2;
    return;
   }
 cutFrom=ori-1;
 for (;*ori && ucisspace(*ori); ori++)
     if (*ori=='\n')
        cutCross++;
 EnParentesis=0;
 for (s=Nom; *ori && *ori!=':' && (*ori!='.' || EnParentesis || !IsDelimiterDeXX(*(ori+1))) && *ori!=','; s++,ori++)
    {
     if (*ori=='\n')
       {
        *s++=' ';
        ori++;
        for (;*ori && ucisspace(*ori); ori++);
        cutCross++;
       }
     else
       {
        if (*ori=='(')
           EnParentesis=1;
        else
          if (EnParentesis && *ori==')')
             EnParentesis=0;
       }
     *s=*ori;
    }
 *s=0;
 linebreak=OffEnProxLin;
 ori++;
 // Sanity check coz that's too complex.
 // Solo le corto el nombre si estoy seguro de que no voy a reventar todo
 if (!((ori-Origin)<1 || ori<=cutFrom || Size<=(ori-Origin)))
   {
    if (cutCross)
      {
       *cutFrom='\n';
       cutFrom++;
      }
    memcpy(cutFrom,ori,size_t(Size-(ori-Origin)+1));
    Size-=ori-cutFrom;
    Fin-=ori-cutFrom;
   }
}

/* First approach, hmmmm ... I think that this won't work with
   2 notes in the same line, one crossing the end
void CutNoteStr(char *s)
{
 char *end;
 for (end=s; *end!='\n'; end++);
 int cant;
 if (s[5]==' ')
    cant=6;
 else
    cant=5;
 memcpy(s,s+cant,end-s-cant);
 s=end-cant;
 int i;
 for (i=cant; i; i--,s++)
     *s=' ';
}
*/

void CutNoteStr(char *s, char *ori, long &size, char *&fin)
{
 int cant;
 
 if (s[5]==' ')
    cant=6;
 else
    cant=5;
 //char eos=*fin;
 CLY_memcpy(s,s+cant,size_t(size-(s-ori)-cant+1));
 size-=cant;
 fin-=cant;
 //if (!eos) *fin=0;
}

void TInfTopic::ReadCrossRefs( void )
{
 CrossRef *crossRefPtr;
 char Buf[BUF_SIZE],BufLine[BUF_SIZE];
 int Linea,LineaMenu=0,iMenues;
 int OffInLine,Length,OffInThisLine,LineOfNote;
 // Para los notes
 char *cpPos,*cpFin,*cpMenu;
 int iNotes,iNAdd=0,iRAdd,iMAdd;

 // Barrer el texto buscando todos los *?ote y * Menu
 iNotes=0;
 cpMenu=cpFin=Text+(int)lSize;
 cpPos=Text;
 Linea=1;
 while (cpPos<cpFin)
    {
     if (*cpPos=='*' && cpPos[2]=='o' && cpPos[3]=='t'
         && cpPos[4]=='e' && ucisspace(cpPos[5]) )
       {
        iNotes++;
        cpPos+=4;
       }
     else
       if (!LineaMenu && *cpPos=='*' && cpPos[1]==' ' && cpPos[2]=='M'
           && cpPos[3]=='e' && cpPos[4]=='n' && cpPos[5]=='u')
         {
          cpMenu=cpPos;
          cpPos+=4;
          LineaMenu=Linea;
         }
       else
         if (*cpPos=='\n')
            Linea++;
     cpPos++;
    }

 // Medir el men£
 Linea=LineaMenu+1;
 iMenues=0;
 do
  {
   getLine(Linea++,BufLine);
   if (*BufLine=='*' && BufLine[1]==' ')
      iMenues++;
  }
 while(Linea<=iLines /*&& (*BufLine=='*' || ucisspace(*BufLine) || *BufLine==0)*/);

 numRefs=iMenues+iNotes;

 if (numRefs)
   {
    crossRefs = new CrossRef[numRefs];
    // Agregar todas las Note antes del men£
    cpPos=Text;
    Linea=1;
    iRAdd=0;
    OffInThisLine=0;
    if (iNotes)
      {
       for (iNAdd=0; iNAdd<iNotes && cpPos<cpMenu; )
          {
           if (*cpPos=='*' && cpPos[2]=='o' && cpPos[3]=='t'
               && cpPos[4]=='e' && ucisspace(cpPos[5]))
             {
              if (modeFlags & moinCutNodeWord)
                {
                 CutNoteStr(cpPos,Text,lSize,cpMenu);
                 cpPos--;
                 OffInThisLine--;
                }
              else
                {
                 cpPos+=4;
                 OffInThisLine+=4;
                }
              crossRefPtr = (CrossRef *)crossRefs + iRAdd;
              LineOfNote=Linea;
              OffInLine=OffInThisLine-1;
              if (modeFlags & moinHideNodeLink)
                 TakeName(cpPos,crossRefPtr->Name,crossRefPtr->Name2,OffInLine,Length,LineOfNote,crossRefPtr->linebreak,Text,lSize,cpMenu);
              else
                {
                 TakeName(cpPos,Buf,OffInLine,Length,LineOfNote,crossRefPtr->linebreak);
                 strcpy(crossRefPtr->Name,Buf);
                }
              crossRefPtr->offset=OffInLine;
              crossRefPtr->line=LineOfNote;
              crossRefPtr->length=Length;
              iNAdd++;
              iRAdd++;
             }
           else
             if (*cpPos=='\n')
               {
                Linea++;
                OffInThisLine=0;
               }
           cpPos++;
           OffInThisLine++;
          }
      }
    cpFin=Text+(int)lSize-6;
    if (iMenues)
      {
       Linea=LineaMenu+1;
       int l=getLine(Linea,BufLine);
       cpPos=Text+lastOffset-l-1;
       iMAdd=0;
       do
        {
         if (*cpPos=='*' && cpPos[1]==' ')
           {
            crossRefPtr = (CrossRef *)crossRefs + iRAdd;
            LineOfNote=Linea;
            OffInLine=0;
            if (modeFlags & moinHideNodeLink)
              {
               //long olSize=lSize;
               TakeName(cpPos,crossRefPtr->Name,crossRefPtr->Name2,OffInLine,Length,LineOfNote,crossRefPtr->linebreak,Text,lSize,cpFin);
               //lastOffset+=lSize-olSize; // Porque al cortarlo cambi¢
              }
            else
              {
               TakeName(cpPos,Buf,OffInLine,Length,LineOfNote,crossRefPtr->linebreak);
               strcpy(crossRefPtr->Name,Buf);
              }
            crossRefPtr->offset=OffInLine;
            crossRefPtr->line=LineOfNote;
            crossRefPtr->length=Length;
            iMAdd++;
            iRAdd++;
           }
         // Ahora ir a la pr¢xima l¡nea
         if (iMAdd<iMenues)
           {
            if (cpPos-Text>=lSize)
               break;
            for (; *cpPos && *cpPos!='\n'; cpPos++);
            cpPos++;
           }
         Linea++;
        }
       while(iMAdd<iMenues);
      }
    // Agregar las que est‚n atr s del men£
    if (iNotes && iNAdd<iNotes)
      {
       cpPos=cpMenu;
       Linea=LineaMenu;
       OffInThisLine=0;
       for (; iNAdd<iNotes && cpPos<cpFin; )
          {
           if (*cpPos=='*' && cpPos[2]=='o' && cpPos[3]=='t'
               && cpPos[4]=='e' && ucisspace(cpPos[5]))
             {
              if (modeFlags & moinCutNodeWord)
                {
                 CutNoteStr(cpPos,Text,lSize,cpFin);
                 cpPos--;
                 OffInThisLine--;
                }
              else
                {
                 cpPos+=4;
                 OffInThisLine+=4;
                }
              crossRefPtr = (CrossRef *)crossRefs + iRAdd;
              LineOfNote=Linea;
              OffInLine=OffInThisLine-1;
              if (modeFlags & moinHideNodeLink)
                {
                 TakeName(cpPos,crossRefPtr->Name,crossRefPtr->Name2,OffInLine,Length,LineOfNote,crossRefPtr->linebreak,Text,lSize,cpFin);
                }
              else
                {
                 TakeName(cpPos,Buf,OffInLine,Length,LineOfNote,crossRefPtr->linebreak);
                 strcpy(crossRefPtr->Name,Buf);
                }
              crossRefPtr->offset=OffInLine;
              crossRefPtr->line=LineOfNote;
              crossRefPtr->length=Length;
              iNAdd++;
              iRAdd++;
             }
           else
             if (*cpPos=='\n')
               {
                Linea++;
                OffInThisLine=0;
               }
           cpPos++;
           OffInThisLine++;
          }
      }
   }
}

TInfTopic::~TInfTopic()
{
 if (crossRefs != 0)
    delete[] crossRefs;
 if (Text!=NULL)
    delete[] Text;
}

void TInfTopic::getCrossRef( int i, TPoint& loc, uchar& length, char *& ref,
                             int &pl )
{
 CrossRef *crossRefPtr;

 crossRefPtr = crossRefs + i;
 loc.x = crossRefPtr->offset;
 loc.y = crossRefPtr->line;
 length = crossRefPtr->length;
 ref = crossRefPtr->Name;
 pl = crossRefPtr->linebreak;
}

int TInfTopic::getLine( int line, char *buffer )
{
 int offset, i, l;
 char *s;

 if (lastLine < line)
     {
     i = line;
     line -= lastLine;
     lastLine = i;
     offset = lastOffset;
     }
 else
     {
     offset = 0;
     lastLine = line;
     }
 buffer[0] = 0;
 while (offset < lSize)
 {
  --line;
  if (line == 0)
    {
     char *dest=buffer;
     // Barrer buscando el fin de l¡nea y copiando
     for (s=Text+offset, l=0; *s && *s!='\n'; s++,offset++,dest++,l++) *dest=*s;
     if (*s=='\n')
       {
        offset++;
        *dest++=0;
       }
     lastOffset = offset;
     return l;
    }
  else
    {
     // Barrer buscando el fin de l¡nea
     for (s=Text+offset; *s && *s!='\n'; s++,offset++);
     if (*s=='\n') offset++;
    }
 }
 buffer[0] = 0;
 return 0;
}


static int MatchCross(char *s,char *match,int l,int &vislen)
{
 for (vislen=l;*s && !TVCodePage::isAlpha(*s);s++,vislen++);
 return strncasecmp(s,match,l)==0;
}

#define VisibleNameOf(i) \
        modeFlags & moinHideNodeLink ? crossRefs[i].Name2 : crossRefs[i].Name

int TInfTopic::SearchNextWith(char *str,int len,int &selected,int &vislen)
{
 int i,j;

 if (selected<numRefs)
   {
    if (selected<1) // It was a bug and was there for ages
        selected=1;
    for (i=selected-1; i<numRefs; i++)
       {
        if (MatchCross(VisibleNameOf(i),str,len,vislen))
          {
           selected=i+1;
           return 1;
          }
       }
   }
 if (numRefs)
   {
    for (j=selected-1,i=0; i<j; i++)
       {
        if (MatchCross(VisibleNameOf(i),str,len,vislen))
          {
           selected=i+1;
           return 1;
          }
       }
   }
 return 0;
}


int TInfTopic::selBestMatch(char *match, int &PerfectMatch, unsigned opts)
{
 PerfectMatch=0;
 if (match==0 || numRefs==0) return 0;

 int i,j,BestPoints=0,BestPos=-1;
 char Buf[MAX_NODE_NAME];
 char *s1,*s2;
 strupr(match);
 size_t lenMatch=strlen(match);
 for (i=numRefs-1; i>=0; --i)
    {
     // Copy & struppr
     s1=opts & bestMVisibleName ? crossRefs[i].Name2 : crossRefs[i].Name;
     for (s2=Buf; *s1; s1++,s2++) *s2=TVCodePage::toUpper(*s1); *s2=0;
     // Compare
     for (s1=Buf, s2=match, j=0; *s1==*s2 && *s1; s1++,s2++,j++);
     if (j>BestPoints ||
         (j==BestPoints && j==(int)lenMatch)) // Give priority to the perfect over
                                         // a partial match.
       {
        BestPoints=j;
        BestPos=i;
       }
    }
 PerfectMatch=(lenMatch==(size_t)BestPoints);
 return BestPos+1;
}


// ******************** TInfIndexCollection ************************

// Un insert adecuado.
stkHandler TInfIndexCollection::insert(long pos, stkHandler node, char *s,
                                       int len)
{
 struct TIIC *p;
 stkHandler h=stkL->alloc(sizeof(pos)+sizeof(stkHandler)+1+len);
 p=(struct TIIC *)stkL->GetPointerOf(h);
 p->pos=pos;
 p->node=node;
 strncpy(p->name,s,len);
 p->name[len]=0;
 TNoCaseSOSStringCollection::insert(h);
 return h;
}

// Devuelve el string salteando a la posici¢n
char *TInfIndexCollection::GetString( void *h )
{
 return ((TIIC *)stkL->GetPointerOf((stkHandler)h))->name;
}


// ******************** TInfIndex ********************

TInfIndex::~TInfIndex()
{
 if (stk!=NULL)
    delete stk;
 if (coll!=NULL)
    delete coll;
 if (IndOffsets!=NULL)
    delete[] IndOffsets;
 IndOffsets=NULL;
}


TInfIndex::TInfIndex( TInfFile& o, int Indirect )
{
 indirects=0;
 IndOffsets=NULL;
 stk=new SOStack;
 coll=new TInfIndexCollection(100,100,stk);
 coll->duplicates = True;

 CheckClassError(!stk || !coll);

 FILE *f=o.stream;
 Status = False; // OK

#ifdef DEBSTD
 fprintf(stderr,"Loading the Index, indirect: %d\n",Indirect);
#endif
 // Indirecta => varios archivos
 long PosStream = ftell(f);
 if (Indirect)
   {
    // Medir
    do
     {
      o.GetLine();
      if (o.Buffer[0]==31) break;
      indirects++;
     }
    while (!feof(f));
    fseek(f,PosStream,SEEK_SET);
#ifdef DEBSTD
    fprintf(stderr,"Found %d indirects\n",indirects);
#endif
    // Cargar
    IndOffsets=new IndirectIndex[indirects];
    indirects=0;
    do
     {
      char *s,*ori;

      o.GetLine();
      if (o.Buffer[0]==31) break;
      s=IndOffsets[indirects].Name;
      ori=o.Buffer;
      for (;*ori && *ori!=':'; s++,ori++) *s=*ori;
      *s=0;
      ori++;
      IndOffsets[indirects].lPos=atol(ori);
      indirects++;
     }
    while (!feof(f));
#ifdef DEBSTD
    fprintf(stderr,"Loaded %d indirects\n",indirects);
#endif

    // Ir al ¡ndice
    do
     {
      o.GetLine();
      if (strncmp(o.Buffer,"Tag Table",9)==0) break;
      o.SkipNode();
     }
    while(!feof(f));
    if (feof(f))
      {
       #ifdef DEBSTD
       fprintf(stderr,"Tag table not found in indirect table\n");
       #endif
       Status=True;
       return;
      }
    o.GetLine();
   }

 // Una pasada lee y almacena
 char *start,*end;
 // The table is ever at the end of the 1st file so that's valid:
 long ltPos=o.fTell();
 long lTable=o.fileLength-ltPos;
 int  enableProg=0;
 int  nodesRead=0;
 if (lTable>40000)
   {
    enableProg=1;
    char *ReadingMessage=TVIntl::getTextNew(__("Reading index table for: "));
    char *s=new char[strlen(ReadingMessage)+strlen(o.NameOfFile)+1];
    strcpy(s,ReadingMessage);
    strcat(s,o.NameOfFile);
    ProgBar_Init(__("InfView"),lTable,s);
    DeleteArray(s);
    DeleteArray(ReadingMessage);
   }
 stkHandler prevNode=0,curNode;
 unsigned isReference=0;
 do
  {
   o.GetLine();
   if (enableProg)
     {
      if ((++nodesRead & 0x7)==0)
         ProgBar_UpDate(o.fTell()-ltPos);
     }
   if (o.Buffer[0]==31)
      break;
   int skip=0;
   if (strncmp(o.Buffer,"Node:",5)==0) { skip=6; isReference=0; }
   // New thing introduced in texinfo 4.0
   if (strncmp(o.Buffer,"Ref:" ,4)==0) { skip=5; isReference=1; }
   if (!skip)
     {
      Status=True;
      break;
     }
   for (start=o.Buffer+skip, end=start; *end && *end!=127 && *end!='\n'; end++);
   if (*end!=127)
     {
      Status=True;
      break;
     }
   if (isReference)
      curNode=coll->insert(atol(end+1),prevNode,start,end-start);
   else
     {
      curNode=coll->insert(atol(end+1),stkNULL,start,end-start);
      prevNode=curNode;
     }
  }
 while (!feof(f));
 if (enableProg)
   {
    ProgBar_UpDate(o.fTell()-ltPos);
    ProgBar_DeInit();
   }
 if (Status)
    return;

 size = coll->getCount();

 CheckClassError(!size || !stk->Buffer);

 #ifdef DEBSTD
 fprintf(stderr,"Tag Table loaded\n");
 #endif
}


// For files without Tag Table (not real .info, just texts with 0x1F as delimiter)
TInfIndex::TInfIndex( TInfFile& o )
{
 long PosNode;
 char *Pos;

 indirects=0;
 IndOffsets=NULL;
 stk=new SOStack;
 coll=new TInfIndexCollection(100,100,stk);
 coll->duplicates = True;

 CheckClassError(!stk || !coll);

 FILE *f=o.stream;
 Status = False; // OK

 // Al ppio.
 rewind(f);

 // Asumo que lo primero es el t¡tulo y no un nodo.

 // Una pasada lee y almacena
 do
  {
   o.SkipNode();
   while (!feof(f))
     {
      PosNode=ftell(f);
      o.GetLine();
      if (strncmp(o.Buffer,"File:",5)==0)
        {
         Pos=strstr(o.Buffer+5,"Node:");
         CheckClassError(Pos==NULL);
         Pos+=6;
         char *s;
         int largo;
         for (s=Pos,largo=0; *s && *s!='\n' && *s!='\r' && *s!=',' && *s!='\t'; s++,largo++);
         coll->insert(PosNode,stkNULL,Pos,largo);
         break;
        }
     }
  }
 while (!feof(f));

 size = coll->getCount();

 CheckClassError(!size);
}

long TInfIndex::position(int i, char *&name, int &difference)
{
 if (i<size && !Status)
   {
    TIIC *p=(TIIC *)stk->GetPointerOf((stkHandler)coll->at(i));
    if (p->node!=stkNULL)
      {
       long oldPos=p->pos;
       p=(TIIC *)stk->GetPointerOf(p->node);
       name=p->name;
       difference=oldPos-p->pos;
      }
    return p->pos;
   }
 return -1;
}

int TInfIndex::WhatIndex(char *Nom)
{
 ccIndex index;

 if (coll->SearchCase(Nom,index))
    return index;
 return size+1;
}

int TInfIndex::SearchFirstWith(int Key,int &selected)
{
 int i;

 for (i=0; i<size; i++)
    {
     if (coll->atStr(i)[0]==Key)
       {
        selected=i+1;
        return 1;
       }
    }
 return 0;
}



// TInfFile

int TInfFile::SizeOfReadBuf=BUF_SIZE;

void TInfFile::DoAll( char *Nombre, int Verbose )
{
 int Indirect=0;
 index=NULL;
 strcpy(NameOfFile,Nombre);
 // Volarle la extensi¢n
// char *s;
// if ((s=strrchr(NameOfFile,'.'))!=NULL) *s=0;
 Status=False; // OK

 stream=fOpen(Nombre);
 if (stream==NULL)
   {
    if (Verbose)
       messageBox(mfError|mfOKButton,__("Attention: can't open the help file '%s'."), Nombre);
    Status=True;
    return;
   }

 // Buscar el Index
 do
  {
   GetLine();
   if (strncmp(Buffer,"Tag Table",9)==0) break;
   if (strncmp(Buffer,"Indirect:",9)==0) { Indirect=1; break; }
   SkipNode();
  }
 while(!feof(stream));

 //   Si fall¢ es que no posee Tag Table, antes yo pensaba que era imposible,
 // pero el Debian crea el dir automaticamente y los idiotas en lugar de
 // crear un verdadero dir.info crean un archivo de texto plano con un solo 0x1F
 // para delimitar el t¡tulo del £nico nodo.
 if (feof(stream))
   {
    // Levantarlo a RAM, pero con un constructor !=
    index=new TInfIndex(*this);
   }
 else
   {
    // Levantarlo a RAM
    index=new TInfIndex(*this,Indirect);
   }
 Status=index->Status;

 if (Indirect && !Status)
   { // Abrir la primera
    fClose(stream);
    iFile=0;
    stream=fOpen(index->IndOffsets[0].Name);
    if (stream==NULL)
      {
       Status=True;
       return;
      }
   }
}

/********************************************************************************

  Function: int TestForFile(char *s)

  Objetive: Test if some file exists and in this case if is a normal file.

  Parameter:
  char *s The name of the file

  Returns:
  -1 Fail
  0 OK

********************************************************************************/

int TestForFile(char *s)
{
 struct stat a;
 if (stat(s,&a)!=0)
    return -1;
 return S_ISREG(a.st_mode) ? 0 : -1;
}

/********************************************************************************

  Function: int TryWithName(char *s, int have_ext, int iExt, int &is_compressed)

  Type: static.

  Objetive: Try to find a file with the specified name but with variants in
  the extention.

  Parameter:
  char *s: a pointer to the name of the file.
  int have_ext: 1 if the original name already have an extention.
  int iExt: 1 try to add an extention.
  int &is_compressed: returns 1 if the name belongs to a compressed file.

  Returns:
  1 if the file doesn't exists.
  0 if all OK, in this case the string holds the valid name.

********************************************************************************/

static int TryWithName(char *s, char *ext, int iExt, Boolean &is_compressed)
{
#define TestForFileRetZ(s) {if(TestForFile(s)==0) { is_compressed=True; return 0; }}
#define TestForFileRet(s)  {if(TestForFile(s)==0) { return 0; }}
#define TestForFileRetAddZ(s,e) { \
        strcat(s,e); \
        if (TestForFile(s)==0) \
           { is_compressed=True; return 0; } \
        s[pos]=0; \
            }
#define TestForFileRetAdd(s,e) { \
        strcat(s,e); \
        if (TestForFile(s)==0) \
           return 0;  \
        s[pos]=0; \
            }

 int pos;

 pos=strlen(s);
 is_compressed=(ext!=0 && strchr(ext,'z')!=0) ? True : False;
 // Try the normal name
 TestForFileRet(s);
 if (iExt)
   {
    // If the normal fails try with .inf, etc
    pos=strlen(s);
    if (ext==0)
      {
       TestForFileRetAdd(s,".inf");
       TestForFileRetAdd(s,".info");
       TestForFileRetAddZ(s,".igz");
       TestForFileRetAddZ(s,".gz");
       TestForFileRetAddZ(s,".info.gz");
       TestForFileRetAddZ(s,".inf.gz");
       TestForFileRetAddZ(s,".inz");
       #ifdef HAVE_BZIP2
       TestForFileRetAddZ(s,".bz2");
       TestForFileRetAddZ(s,".info.bz2");
       #endif
      }
    else
      {
       TestForFileRetAddZ(s,".gz");
       #ifdef HAVE_BZIP2
       TestForFileRetAddZ(s,".bz2");
       #endif
       // Change the extention
       char *dot=strrchr(s,'.')+1;
       int ext_l=strlen(dot);
       // People that used SFNs and then switchs to LFNs can experiment it.
       if (ext_l==3 && strncmp(dot,"inf",3)==0)
         {
          dot[3]='o'; dot[4]=0;
          if (TestForFile(s)==0)
             return 0;
          dot[3]=0;
         }
       is_compressed=True;
       switch (ext_l)
         {
          case 1: dot[1]='z';
                  dot[2]=0;
                  TestForFileRet(s);
                  dot[1]='g';
                  dot[2]='z';
                  dot[3]=0;
                  TestForFileRet(s);
                  break;

          case 2: dot[2]='z';
                  dot[3]=0;
                  TestForFileRet(s);
                  break;

          case 3: dot[2]='z';
                  TestForFileRet(s);
                  break;
         }
      }
   }

 return 1;
}

#ifdef SECompf_djgpp
void LoadInfoEnviroment(void)
{
 static int info_env_loaded = 0;
 if (!info_env_loaded)
   {
    __crt0_load_environment_file("info");
    info_env_loaded = 1;
   }
}

static
void GuessInfoPath(DynStrCatStruct *InfoPath)
{
 char *djgpp=getenv("DJDIR");
 if (djgpp)
   {
    DynStrCatInit(InfoPath,djgpp);
    DynStrCat(InfoPath,"/info",5);
    return;
   }
 DynStrCatInit(InfoPath,".",1);
}
#else
inline
void LoadInfoEnviroment(void)
{
}

static inline
void GuessInfoPath(DynStrCatStruct *InfoPath)
{
 DynStrCatInit(InfoPath,"/usr/local/info:/usr/info:/usr/share/info:/usr/local/share/info");
}
#endif

static DynStrCatStruct InfoPath={0,0};

static
void RemovePath(void)
{
 free(InfoPath.str);
}

char *InfViewGetInfoDir(void)
{
 if (InfoPath.str)
    return InfoPath.str;
    
 // Try at first the directories in $INFOPATH, if there are some
 // DOS: Get the $INFOPATH like info.exe sees it
 LoadInfoEnviroment();
 char *infodir=getenv("INFOPATH");
 // That's for all the lazy people, who don't set the INFOPATH
 if (infodir)
    DynStrCatInit(&InfoPath,infodir);
 else
    GuessInfoPath(&InfoPath);
 atexit(RemovePath);

 return InfoPath.str;
}

void InfViewAddInfoDir(char *dir)
{
 if (InfoPath.str)
   {
    char b[2]={PATHSEPARATOR,0};
    DynStrCat(&InfoPath,b,1);
    DynStrCat(&InfoPath,dir);
   }
 else
    DynStrCatInit(&InfoPath,dir);
}

static
char *strrpbrk(char *string, char *search)
{
 char *scanp,*str;
 int c, sc;

 for (str=string; *str; str++);
 for (str--; str>=string; str--)
     for (c=*str, scanp=search; (sc=*scanp++)!=0;)
         if (sc==c)
            return str;
 return 0;
}


int TInfFile::ExpandName(char *Buf, char *Nombre, int iExt)
{
 int a;
 int HavePath;
 char *ext;

 strcpy(Buf,Nombre);

 // Check if it have a path (relative or absolute)
 ext=strrpbrk(Buf,"/\\:");
 if (ext)
    HavePath=1;
 else
   {
    HavePath=0;
    ext=Buf;
   }

 // Check if it have extention, I assume the extention is the last one in LFN systems
 ext=strrchr(ext,'.');

 if (HavePath)
   {
    // Try with the name as supplied, that's without adding a path
    a=TryWithName(Buf,ext,iExt,IsCompressed);
    if (!a)
      {
       Status=False;
       return 0;
      }
   }

 char *infodir=InfViewGetInfoDir();
 if (infodir)
 {
   char *idir=(char *)alloca(strlen(infodir) + 1);
   char *end=idir,*start=idir;
   strcpy(idir,infodir);
   while (1)
   {
     char c;
     while (*end && *end != PATHSEPARATOR) end++;
     c = *end;
     *end = 0;
     strcpy(Buf,start);
     strcat(Buf,"/");
     strcat(Buf,Nombre);
     a = TryWithName(Buf,ext,iExt,IsCompressed);
     if (!a)
     {
       Status = False;
       return 0;
     }
     if (!c) break;
     start=end=end+1;
   }
 }

 char *s=getenv("DJGPP");
 if (s==NULL)
   {
    Status=True;
    return 1;
   }
 strcpy(Buf,s);
 strupr(Buf);
 for (s=Buf; *s; s++) if (*s=='/') *s='\\';
 s=strstr(Buf,"DJGPP.ENV");
 if (s==NULL)
   {
    Status=True;
    return 1;
   }
 s[0]='I';s[1]='N';s[2]='F';s[3]='O';s[4]='\\';
 char *s2=Nombre;
 for (s+=5; *s2; s++,s2++) *s=*s2; *s=0;
 a = TryWithName(Buf,ext,iExt,IsCompressed);
 if (a)
   {
    Status=True;
    return 1;
   }
 return 0;
}

FILE *TInfFile::fOpen(char *Nombre)
{
 char Buf[PATH_MAX];
 char *name=Buf;
 int  freeName=0;

 if (ExpandName(Buf,Nombre,1))
   {
    // There are an special case: when the NameOfFile have a path and we are looking for
    // the component files of an splitted .info then we must try in the directory where
    // NameOfFile is located. The fix is relative slow but simple.
    char *endOfPath=strrpbrk(NameOfFile,"/\\:");
    if (endOfPath)
      {
       char BufNomb[PATH_MAX];
       int l=endOfPath-NameOfFile+1;
       strncpy(BufNomb,NameOfFile,l);
       BufNomb[l]=0;
       strcat(BufNomb,Nombre);
       if (ExpandName(Buf,BufNomb,1))
          return NULL;
      }
    else
       return NULL;
   }
 if (IsCompressed)
   {
    name=unique_name("gz",NameOfTemporal);
    if (!name)
       name="__infc__";
    else
       freeName=1;
    if (GZFiles_ExpandHL(name,Buf))
      {
       if (freeName) free(name);
       return NULL;
      }
   }

 #ifdef InfV_UseBinaryFile
 FILE *ret=fopen(name,"rb");
 #else
 FILE *ret=fopen(name,"rt");
 #endif

 #ifdef SEOS_UNIX
 if (IsCompressed)
   {// In UNIX we can remove an open file and still using it
    // That's better if we crash
    unlink(name);
    DontRemoveCompressed=True;
   }
 #else
 DontRemoveCompressed=False;
 #endif
 fileLength=ret ? filelength(fileno(ret)) : 0;
 if (freeName) free(name);
 return ret;
}

long TInfFile::fTell(void)
{
 return ftell(stream);
}

int TInfFile::fClose(FILE *f)
{
 int ret=0;

 if (f!=NULL)
   {
    ret=fclose(f);
    if (IsCompressed)
      {
       if (!DontRemoveCompressed)
          unlink(NameOfTemporal);
       IsCompressed=False;
      }
   }
 return ret;
}

TInfFile::~TInfFile(void)
{
 if (index!=NULL)
    delete index;
 fClose(stream);
}

void TInfFile::SkipNode(void)
{
 do
  {
   GetLine();
  }
 while(Buffer[0]!=31 && !feof(stream));
}

int TInfFile::fSeek(long Pos)
{
 #ifdef DEBSTD
 fprintf(stderr,"Making a seek to %d\n",Pos);
 #endif
 if (index->indirects)
   {
    int i;
    for (i=0; i<index->indirects && Pos>=index->IndOffsets[i].lPos; i++);
    #ifdef DEBSTD
    fprintf(stderr,"Using the indirect table, entry %d actual %d\n",i-1,iFile);
    #endif
    if (--i!=iFile)
      {
       if (i==-1) return 1;
       iFile=i;
       fClose(stream);
       #ifdef DEBSTD
       fprintf(stderr,"Opening %s\n",index->IndOffsets[i].Name);
       #endif
       stream=fOpen(index->IndOffsets[i].Name);
       #ifdef DEBSTD
       fprintf(stderr,"Result: %s\n",stream==NULL ? "fail" : "ok");
       #endif       
       if (stream==NULL)
          return 1;
      }
    Pos=Pos-index->IndOffsets[i].lPos+index->IndOffsets[0].lPos;
   }
 #ifdef DEBSTD
 fprintf(stderr,"Seeking to %d\n",Pos);
 #endif
 return fseek(stream,Pos,SEEK_SET);
}


TInfTopic *TInfFile::getTopic(char *NameOri, int Verbose, int modeForTopic,
                              int &suggestedY)
{
 long pos;
 char cFile[MAX_NODE_NAME];
 char *Name;
 int  WatchDog=MAX_NODE_NAME-1;

 Name=NameOri;
 // Ver de que archivo es
 if (Name[0]=='(')
   {
    char *s=Name+1,*s2=cFile;

    for (;*s && *s!=')' && WatchDog;s++,s2++,WatchDog--) *s2=*s; *s2=0;
    if (!WatchDog)
      {
       messageBox("Guau ... guau!!", mfInformation | mfOKButton);
      }
    Name=s+1;

    if (strcmp(NameOfFile,cFile)!=0)
      { // Se pudri¢
       if (index!=NULL)
          delete index;
       fClose(stream);
       DoAll(cFile,Verbose);
       if (index==NULL)
          return(invalidTopic());
      }
   }
 if (Status)
    return(invalidTopic());
 if (Name[0]==0)
   {
    strcpy(NameOri,"(");
    strcat(NameOri,NameOfFile);
    strcat(NameOri,")Top");
    Name=strchr(NameOri,')')+1;
   }
 int nodeNum=index->WhatIndex(Name);
 int difference=0;
 pos = index->position(nodeNum,Name,difference);
 if (pos>0)
     {
      fSeek(pos);
      TInfTopic *topic = new TInfTopic(modeForTopic);
      if (!seekToNode(Name))
        {// Try in all the file
         if (!seekToNode(Name,1))
           {
            delete topic;
            return(invalidTopic());
           }
        }
      topic->Read(*this,difference,suggestedY);
      if (topic->Status)
        {
         delete topic;
         return(invalidTopic());
        }
      return topic;
     }
 else return(invalidTopic());
}

TInfTopic *TInfFile::invalidTopic()
{
 TInfTopic *topic;
 char *invalidText=TVIntl::getTextNew(__("\n Sorry, I can't find this link (press ALT-F1).\n"));

 topic =  new TInfTopic(0);
 topic->Text = invalidText;
 topic->lSize = strlen(invalidText);
 topic->iLines = 2;
 topic->Status = True;

 return topic;
}

static void Rename(char *oldname, char *newname)
{
 void *buf;
 buf = malloc(16*1024); // Just the transfer buffer
 int count;
 FILE *f = fopen(oldname,"rb");
 FILE *stream = fopen(newname,"w+b");
 while ((count = fread(buf,1,16*1024,f)) > 0)
 {
   fwrite(buf,1,count,stream);
 }
 fclose(stream);
 fclose(f);
 remove(oldname);
 free(buf);
}

int TInfFile::ConvertIt(long Pos)
{
 int ret=0,i;
 char *s;
 FILE *f;
 char *Tempo,Ori[PATH_MAX];
 Boolean wasCompressed;

 wasCompressed=IsCompressed;
 if (IsCompressed)
    rewind(stream);
 else
    fClose(stream);
 i=0;
 do
   {
    if (IsCompressed)
      {
       //tmpnam(NameOfTemporal);
       strcpy(Ori,NameOfTemporal);
      }
    else
      {
       if (i==index->indirects)
          ExpandName(Ori,NameOfFile,1);
       else
          ExpandName(Ori,index->IndOffsets[i].Name,1);
       stream=fopen(Ori,"rb");
      }
    Tempo=unique_name("iz");
    f=fopen(Tempo,"wb");
    if (stream==NULL || f==NULL) return ret;
    do
     {
      GetLine();
      if (!feof(stream))
        {
         for (s=Buffer; *s; s++);
         if (*(s-2)=='\r')
           {
            *(s-2)='\n';
            *(s-1)=0;
           }
         fputs(Buffer,f);
        }
     }
    while (!feof(stream));
    fclose(f);
    fclose(stream);
    Rename(Tempo,Ori);
    string_free(Tempo);
    i++;
   }
 while (i<=index->indirects && !wasCompressed);

 // Test if convertion succesfull
 if (wasCompressed)
   {
    stream=fopen(Ori,"rb");
    IsCompressed=True;
    #ifdef SEOS_UNIX
    DontRemoveCompressed=True;
    unlink(Ori);
    #else
    DontRemoveCompressed=False;
    #endif
   }
 else
   {
    stream=fOpen(NameOfFile);
    iFile++; // force reopen or we won't find the node
   }
 if (stream==NULL) return ret;
 fSeek(Pos);
 return 1;
}


