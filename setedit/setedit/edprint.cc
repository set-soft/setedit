/*****************************************************************************

  Printing Module. Copyright 1995-2003 by Salvador E. Tropea.

  This module can be used by Robert Hoehne in RHIDE. Any other needs the
permission of the author.
  I put this limitation because I want to track any derivative work.

  This module is an adaptation of an old program I did in 1995 to print a
program for my university.

*****************************************************************************/

#include <ceditint.h>
#define Uses_stdio
#define Uses_string
#include <time.h>
#define Uses_stdlib
#define Uses_unistd
#define Uses_intl_fprintf

#define Uses_MsgBox
#define Uses_TDialog
#define Uses_TRect
#define Uses_TInputLine
#define Uses_TLabel
#define Uses_TCheckBoxes
#define Uses_TSItem
#define Uses_TButton
#define Uses_fpstream
#define Uses_TEvent

#define Uses_TSHzGroup
#define Uses_TSInputLine
#define Uses_TSLabel
#define Uses_TSCheckBoxes
#define Uses_TSButton
#define Uses_TSRadioButtons
// First include creates the dependencies
#include <easydia1.h>
#include <tv.h>
// Second request the headers
#include <easydiag.h>

#define Uses_SETAppConst // For the help context
#include <setapp.h>

#include <edmsg.h>

extern ushort execDialog( TDialog *d, void *data );

#define MAX_COLS 256
#define MAX_CCFG 80

const int sizeNames=MAX_CCFG,
          sizeComma=MAX_CCFG/2;

static int iLineas,iLineasActu;
static char TimeFormat[12];
static char DateFormat[12];
static char Dia[40];
static char Hora[40];
static int  iCols,iColsActu;
static int  NumeLineas;
static int  iNumLin;
static int  iMargen;
static int  iOutType;
static char Autor[sizeNames+1];
static char Titulo[sizeNames+1];
static char Output[sizeNames+1];
static char *FileName;
static char *PiePagina=__("Page: %d Lines: %d to %d  Date: %s  Time: %s%s");
static char *Cabecera=__("* Source: %s * Title: %s * Author: %s\n\n");

typedef struct
{
 uchar comm[sizeComma];
 int   size;
} PrnComm;

typedef struct
{
 char *inic;
 char *an_cab;
 char *de_cab;
 char *an_pie;
 char *de_pie;
} PrnDef;

// Reset and font, Empha., No empha., Italic, No italic
static PrnDef Epson={ "\x1B@\x1B!\x1", "\x1B""E", "\x1B""F", "\x1B""4", "\x1B""5" };
// Reset + CR/LF mode + PC437 code page + 12 CPI + TMS Roman
static PrnDef HP={ "\x1B""E\x1B&k2G\x1B(10U\x1B(s12H\x1B(s5T",
                   "\x1B(s3B", "\x1B(s0B", "\x1B(s1S", "\x1B(s0S" };

/* Inicializaci¢n */
static PrnComm ImpreInic;
/* Cabecera */
static PrnComm ImpAnCab;
static PrnComm ImpDeCab;
/* Pie */
static PrnComm ImpAnPie;
static PrnComm ImpDePie;

static
void FillComm(PrnComm &s, char *c)
{
 strcpy((char *)s.comm,c);
 s.size=strlen(c);
}

static
void CopyDefault(PrnDef &pr)
{
 FillComm(ImpreInic,pr.inic);
 FillComm(ImpAnCab,pr.an_cab);
 FillComm(ImpDeCab,pr.de_cab);
 FillComm(ImpAnPie,pr.an_pie);
 FillComm(ImpDePie,pr.de_pie);
}

opstream& operator << (opstream& ps, PrnComm &p)
{
 ps << p.size;
 ps.writeBytes(p.comm,p.size);
 return ps;
}

ipstream& operator >> (ipstream& ps, PrnComm &p)
{
 ps >> p.size;
 ps.readBytes(p.comm,p.size);
 return ps;
}

/**[txh]********************************************************************

  Description:
  Converts the string pointed by Buf to a sequence of chars. Buf contains
a string with values separated by commas. The sequence is stored in the
array pointed by Com and the length is stored in Largo.
  Is used to convert the printer commands from numbers to real commands.

***************************************************************************/

static
void LeeComImpr(char *Buf, PrnComm &p)
{
 int Indice=0;
 char *Pun;

 p.size=0;
 Pun=strtok(Buf,",");
 while(Pun!=NULL)
   {
    p.comm[Indice++]=atoi(Pun);
    p.size++;
    Pun=strtok(NULL,",");
   }
}

/********************
That's no longer needed because now is stored/retreived different
int LeerConfiguracion(void)
{
 FILE *fCon;
 char Buffer[MAX_CCFG+1];
 int Cont=0;

 fCon=fopen("pf.cfg","rt");
 if (fCon==NULL)
   {
    printf("Error no figura el archivo de configuraci¢n\n");
    return 1;
   }

 // Cantidad de lineas
 fgets(Buffer,MAX_CCFG,fCon);
 fgets(Buffer,MAX_CCFG,fCon);
 iLineas=atoi(Buffer)-4;
 if (iLineas<3)
   {
    printf("Error al menos 3 lineas\n");
    return 1;
   }
 printf("Cortar cada: %d\n",iLineas);


 // Cantidad de columnas
 fgets(Buffer,MAX_CCFG,fCon);
 fgets(Buffer,MAX_CCFG,fCon);
 iCols=atoi(Buffer);
 if (iCols<40 || iCols>MAX_COLS)
   {
    printf("Error el n£mero de columnas debe ser entre 40 y %d\n",MAX_COLS);
    return 1;
   }
 printf("Columnas: %d\n",iCols);

 // Margen
 fgets(Buffer,MAX_CCFG,fCon);
 fgets(Buffer,MAX_CCFG,fCon);
 iMargen=atoi(Buffer);
 if (iMargen<0)
   {
    printf("Error el margen debe ser positivo\n");
    return 1;
   }

 // Inversi¢n de fecha
 fgets(Buffer,MAX_CCFG,fCon);
 fgets(Buffer,MAX_CCFG,fCon);
 InvertirDia=atoi(Buffer);
 if (InvertirDia)
    printf("Invirtiendo d¡a/mes\n");


 // Numerar l¡neas
 fgets(Buffer,MAX_CCFG,fCon);
 fgets(Buffer,MAX_CCFG,fCon);
 NumeLineas=atoi(Buffer);
 if (NumeLineas)
    printf("Numerando l¡neas\n");

 // T¡tulo
 fgets(Buffer,MAX_CCFG,fCon);
 Lee(Titulo,fCon);

 // Autor
 fgets(Buffer,MAX_CCFG,fCon);
 Lee(Autor,fCon);

 // Comando de inicializaci¢n de la impresora
 fgets(Buffer,MAX_CCFG,fCon);
 fgets(Buffer,MAX_CCFG,fCon);
 LeeComImpr(Buffer,ImpreInic,&LargoImpreInic);

 // Comando antes de imprimir la cabecera
 fgets(Buffer,MAX_CCFG,fCon);
 fgets(Buffer,MAX_CCFG,fCon);
 LeeComImpr(Buffer,ImpAnCab,&LargoImpAnCab);

 // Comando despu‚s de imprimir la cabecera
 fgets(Buffer,MAX_CCFG,fCon);
 fgets(Buffer,MAX_CCFG,fCon);
 LeeComImpr(Buffer,ImpDeCab,&LargoImpDeCab);

 // Comando antes de imprimir el pie de p gina
 fgets(Buffer,MAX_CCFG,fCon);
 fgets(Buffer,MAX_CCFG,fCon);
 LeeComImpr(Buffer,ImpAnPie,&LargoImpAnPie);

 // Comando despu‚s de imprimir el pie de p gina
 fgets(Buffer,MAX_CCFG,fCon);
 fgets(Buffer,MAX_CCFG,fCon);
 LeeComImpr(Buffer,ImpDePie,&LargoImpDePie);

 fclose(fCon);
 return 0;
}
******************/

/**[txh]********************************************************************

  Description:
  Sends a sequence of commands to the printer. The stream f is the printer.

***************************************************************************/

static
void MandaImpre(PrnComm &p, FILE *f)
{
 int i;

 for (i=0;i<p.size;i++)
     fputc(p.comm[i],f);
}


/**[txh]********************************************************************

  Description:
  Generates the margin just sending spaces.

***************************************************************************/

static
void PoneMargen(FILE *f)
{
 int i;

 for (i=0;i<iMargen;i++)
     fputc(32,f);
}

static
void GetTimeStrings()
{
 time_t t_Actu=time(0);
 struct tm *tActu=localtime(&t_Actu);
 strftime(Dia,40,DateFormat,tActu);
 strftime(Hora,40,TimeFormat,tActu);
}

static
void PrintHeader(FILE *fSal)
{
 MandaImpre(ImpAnCab,fSal);
 PoneMargen(fSal);
 TVIntl::fprintf(fSal,Cabecera,FileName,Titulo,Autor);
 MandaImpre(ImpDeCab,fSal);
}

static
void PrintFooter(FILE *fSal, int iPagina, int i, int iNumLin)
{
 GetTimeStrings();
 MandaImpre(ImpAnPie,fSal);
 fprintf(fSal,"\n");
 PoneMargen(fSal);
 TVIntl::fprintf(fSal,PiePagina,iPagina,i,iNumLin,Dia,Hora,"\r\f");
 MandaImpre(ImpDePie,fSal);
}

/**[txh]********************************************************************

  Description:
  Prints the buffer Buf to the printer (f). iCR indicates if a CR must be
used after the line.

***************************************************************************/

static
void MandarLinea(char *Buf,FILE *f,int *iLin,int *iPagina)
{
 ++iNumLin;
 PoneMargen(f);
 if (NumeLineas)
    fprintf(f,"%4u: %s\n",iNumLin,Buf);
 else
    fprintf(f,"%s\n",Buf);

 if (*iLin==iLineasActu)
   {
    PrintFooter(f,*iPagina,(*iPagina-1)*iLineasActu+1,*iPagina*iLineasActu);
    PrintHeader(f);
    ++*iPagina;
    *iLin=1;
   }
 else
    (*iLin)++;
}

/**[txh]********************************************************************

  Description:
  Fills all the structures with default values. Must be called at start-up
to be sure there are data in the structures.

***************************************************************************/

void PrintSetDefaults(void)
{
 // Lines per page. That's the total including header and footer
 iLineas=64;
 // Columns of text, not including the margin
 iCols=90;
 // Left margin in characters
 iMargen=7;
 // Time and date formats
 strcpy(TimeFormat,"%k:%M");
 strcpy(DateFormat,"%d/%m/%Y");
 // Print line numbers
 NumeLineas=1;
 // Title
 strcpy(Titulo,"SET's editor ;-)");
 // Autor
 strcpy(Autor,"Salvador E. Tropea (SET)");
 CopyDefault(Epson);
 // Output file.
 #ifdef TVOS_UNIX
 strcpy(Output,"lpr");
 iOutType=1;
 #else
 strcpy(Output,"prn");
 iOutType=0;
 #endif
}

static char *FileToScan;

static inline
void InitFileScan(char *b)
{
 FileToScan=b;
}

static
int FGetS(char *Buffer, int size)
{
 int i;

 if (*FileToScan==0)
    return 0;

 size--;
 for (i=0; i<size && FileToScan[i]!='\n' && FileToScan[i]; i++)
     Buffer[i]=FileToScan[i];

 if (i && Buffer[i-1]=='\r')
    Buffer[i-1]=0;
 else
    Buffer[i]=0;

 FileToScan+=i;
 if (*FileToScan)
    FileToScan++;

 return 1;
}

static
char *ExpandTabs(const char *b, unsigned tabSize)
{
 unsigned x,count;
 const char *s;
 char *ret;

 // First meassure it:
 for (x=0, s=b; *s; s++)
     if (*s=='\t')
        x+=tabSize-(x % tabSize);
     else
        x++;
 x++;
 ret=new char[x];
 // Expand it:
 for (x=0, s=b; *s; s++)
     if (*s=='\t')
       {
        count=tabSize-(x % tabSize);
        while (count--)
          ret[x++]=' ';
       }
     else
        ret[x++]=*s;
 ret[x]=0;
 return ret;
}

int PrintSource(char *b, char *fileName, unsigned tabSize)
{
 FILE *fSal;
 char Buffer[1024],BufLin[MAX_COLS+1];
 int  iLin,iPagina,iLargo,i,iPos,iLOr=0,isPipe=0;

 InitFileScan(b);
 EdShowMessageI(__("Starting printing module"),True);

 if (iOutType==1)
   {
    isPipe=1;
    fSal=popen(Output,"w");
   }
 else
    fSal=fopen(Output,"wb");

 if (fSal==NULL)
   {
    messageBox(__("Can't open output file for printing"),mfError | mfOKButton);
    return 1;
   }

 FileName=fileName;

 if (TVIntl::snprintf(Buffer,1024,Cabecera,FileName,Titulo,Autor)>(iCols+2))
   {
    messageBox(__("The header is too large, reduce the title and/or author lengths"),
               mfError | mfOKButton);
    EdShowMessage(Buffer);
    sprintf(Buffer,__("Header length: %d Columns: %d"),(int)strlen(Buffer),iCols);
    EdShowMessage(Buffer);
    if (isPipe)
       pclose(fSal);
    else
       fclose(fSal);
    return 1;
   }

 iColsActu=iCols;
 if (NumeLineas)
    iColsActu-=6;

 iLin=1;
 iPagina=1;
 iNumLin=0;
 iLineasActu=iLineas-4;

 MandaImpre(ImpreInic,fSal);
 PrintHeader(fSal);

 //printf("\nProcesando:\n\tP gina 1");
 while (FGetS(Buffer,1024))
   {
    char *BufAux=ExpandTabs(Buffer,tabSize);
    iLargo=strlen(BufAux);
    iLOr++;
    if (iLargo<(iColsActu+1))
       MandarLinea(BufAux,fSal,&iLin,&iPagina);
    else
      {
       BufLin[iColsActu]=0;
       for (i=0,iPos=0;i<(iLargo/iColsActu);i++,iPos+=iColsActu)
         {
          strncpy(BufLin,BufAux+iPos,iColsActu);
          MandarLinea(BufLin,fSal,&iLin,&iPagina);
         }
       strncpy(BufLin,BufAux+iPos,iColsActu);
       MandarLinea(BufLin,fSal,&iLin,&iPagina);
      }
    delete[] BufAux;
   }
 for (i=iLin;i<=iLineasActu;i++)
     fprintf(fSal,"\n");
 i=(iPagina-1)*iLineasActu+1;

 PrintFooter(fSal,iPagina,i,iNumLin);

 TVIntl::snprintf(Buffer,1024,__("Processed: %d lines, total printed: %d lines"),iLOr,iNumLin);
 EdShowMessage(Buffer);
 if (isPipe)
    pclose(fSal);
 else
    fclose(fSal);

 return 0;
}

const int cmEpson=0x2250, cmHP=0x2251;

// packed because TVision will count the lengths
#pragma pack(1)
typedef struct
{
 char title[sizeNames];
 char author[sizeNames];
 char lines[5];
 char cols[5];
 char margin[5];
 uint32 lineNum;
 char tformat[12];
 char dformat[12];
 char init[sizeNames];
 char b_head[sizeNames];
 char a_head[sizeNames];
 char b_foot[sizeNames];
 char a_foot[sizeNames];
 char out[sizeNames];
 uint32 o_type;
} RectDiag;
#pragma pack()

static
void ExpandComm(char *dest, PrnComm &p)
{
 int i;
 char buf[8];

 *dest=0;
 for (i=0; i<p.size; i++)
    {
     if (i!=p.size-1)
        sprintf(buf,"%d,",p.comm[i]);
     else
        sprintf(buf,"%d",p.comm[i]);
     strcat(dest,buf);
    }
}

static
void ExpandComm(char *dest, char *s)
{
 char buf[8];

 *dest=0;
 for (; *s; s++)
    {
     if (*(s+1)!=0)
        sprintf(buf,"%d,",(uchar)(*s));
     else
        sprintf(buf,"%d",uchar(*s));
     strcat(dest,buf);
    }
}

static
void FillRectDiag(RectDiag &r)
{
 sprintf(r.lines,"%d",iLineas);
 sprintf(r.cols,"%d",iCols);
 sprintf(r.margin,"%d",iMargen);
 strcpy(r.tformat,TimeFormat);
 strcpy(r.dformat,DateFormat);
 r.lineNum=NumeLineas;
 strcpy(r.title,Titulo);
 strcpy(r.author,Autor);
 strcpy(r.out,Output);
 ExpandComm(r.init  ,ImpreInic);
 ExpandComm(r.b_head,ImpAnCab);
 ExpandComm(r.a_head,ImpDeCab);
 ExpandComm(r.b_foot,ImpAnPie);
 ExpandComm(r.a_foot,ImpDePie);
 r.o_type=iOutType;
}

static
void ExpandRectDiag(RectDiag &r)
{
 iLineas=atoi(r.lines);
 iCols=atoi(r.cols);
 iMargen=atoi(r.margin);
 strcpy(TimeFormat,r.tformat);
 strcpy(DateFormat,r.dformat);
 NumeLineas=r.lineNum;
 strcpy(Titulo,r.title);
 strcpy(Autor,r.author);
 strcpy(Output,r.out);
 LeeComImpr(r.init,  ImpreInic);
 LeeComImpr(r.b_head,ImpAnCab);
 LeeComImpr(r.a_head,ImpDeCab);
 LeeComImpr(r.b_foot,ImpAnPie);
 LeeComImpr(r.a_foot,ImpDePie);
 iOutType=r.o_type;
}

class TDiaPrint : public TDialog
{
public:
 TDiaPrint( TRect r, const char *name, RectDiag *dat ) :
      TWindowInit( &TDiaPrint::initFrame ),
      TDialog(r,name),
      rd(dat) {};
 virtual void handleEvent(TEvent& event);
 RectDiag *rd;
 void SetDefault(PrnDef &def);
};

void TDiaPrint::handleEvent(TEvent& event)
{
 TDialog::handleEvent(event);
 if (event.what==evCommand)
   {
    switch (event.message.command)
      {
       case cmEpson:
            SetDefault(Epson);
            break;
       case cmHP:
            SetDefault(HP);
            break;
       default:
           return;
      }
    clearEvent(event);
   }
}

void TDiaPrint::SetDefault(PrnDef &def)
{
 getData(rd);
 ExpandComm(rd->init  ,def.inic);
 ExpandComm(rd->b_head,def.an_cab);
 ExpandComm(rd->a_head,def.de_cab);
 ExpandComm(rd->b_foot,def.an_pie);
 ExpandComm(rd->a_foot,def.de_pie);
 setData(rd);
}

void PrintSetup(void)
{ //ABCDEFHILMOPRSTUVW
 RectDiag r;
 TSViewCol *col=new TSViewCol(new TDiaPrint(TRect(1,1,1,1),__("Printer Setup"),&r));

 TSView::yDefSep=0;
 TSHzLabel *tLines=new TSHzLabel(__("Total ~l~ines per page:"),new
                                 TSInputLine(5));
 TSHzLabel *cols  =new TSHzLabel(__("Columns ~w~/o margin:"),new
                                 TSInputLine(5));
 TSHzLabel *margin=new TSHzLabel(__("Left ~m~argin:"),new TSInputLine(5));
 TSCheckBoxes *pLines=new TSCheckBoxes(new TSItem(__("Print line numbers"),0));
 TSHzLabel *time  =new TSHzLabel(__("~T~ime format:"),new TSInputLine(12));
 TSHzLabel *date  =new TSHzLabel(__("~D~ate format:"),new TSInputLine(12));
 #define C(var,str) TSLabel *var=new TSLabel(str,new TSInputLine(sizeNames,28))
 C(title,__("T~i~tle"));
 C(author,__("~A~uthor"));
 C(bH,__("Before ~h~eading"));
 C(aH,__("A~f~ter heading"));
 C(bF,__("~B~efore footer"));
 C(aF,__("After foote~r~"));
 TSView::yDefSep=1;
 C(init,__("~P~rinter initialization"));
 C(output,__("O~u~tput file"));
 TSRadioButtons *outType=new TSRadioButtons(new TSItem(__("Output is a de~v~ice/file"),
                                            new TSItem(__("Output i~s~ a program"),0)));
 #undef C
 TSHzGroup *buttons=new TSHzGroup(new TSButton(__("~O~k"),cmOK,bfDefault),
                    new TSHzGroup(new TSButton(__("~C~ancel"),cmCancel),
                    new TSHzGroup(new TSButton("~E~pson",cmEpson),
                                  new TSButton("HP",cmHP))));
 buttons->Flags|=wSpan;

 // Left side
 EDForceSameWidth(tLines,margin,time,title,output,bH,bF,0);
 // Right side
 EDForceSameWidth(cols,pLines,date,author,outType,aH,aF,0);
 init->setWidth(tLines->w+1+cols->w);

 #define C(left,right,prev) col->insert(2,yTSUnder,left,0,prev);\
         col->insert(xTSRightOf,yTSUnder,right,left,prev)
 #define S(left,right) col->insert(2,1,left);\
         col->insert(xTSRightOf,1,right,left);
 S(title,author);
 C(tLines,cols,title);
 C(margin,pLines,tLines);
 C(time,date,margin);
 col->insert(2,yTSUnder,init,0,time);
 C(bH,aH,init);
 C(bF,aF,bH);
 C(output,outType,bF);
 #undef C
 #undef S
 col->insert(xTSCenter,yTSUnder,buttons,0,output);

 TDialog *d=col->doIt();
 delete col;
 d->options|=ofCentered;
 d->helpCtx=cmeSetUpPrinter;

 FillRectDiag(r);

 if (execDialog(d,&r)!=cmCancel)
    ExpandRectDiag(r);
}

const int Version=3;

void SavePrintSetUp(opstream* s)
{
 *s << Version;
 *s << iLineas << iCols << iMargen << NumeLineas;
 s->writeString(TimeFormat);
 s->writeString(DateFormat);
 s->writeString(Titulo);
 s->writeString(Autor);
 s->writeString(Output);
 *s << ImpreInic << ImpAnCab << ImpDeCab << ImpAnPie << ImpDePie;
 *s << iOutType;
}

void LoadPrintSetUp(ipstream* s)
{
 int Version;
 *s >> Version;
 if (Version==1)
    return;
 *s >> iLineas >> iCols >> iMargen >> NumeLineas;
 s->readString(TimeFormat,12);
 s->readString(DateFormat,12);
 s->readString(Titulo,sizeNames);
 s->readString(Autor,sizeNames);
 s->readString(Output,sizeNames);
 *s >> ImpreInic >> ImpAnCab >> ImpDeCab >> ImpAnPie >> ImpDePie;
 if (Version>=3)
    *s >> iOutType;
 else
    iOutType=strcmp(Output,"lpr")==0 ? 1 : 0;
}
