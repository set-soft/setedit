/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#include <stdio.h>
#define Uses_unistd
#define Uses_string
#define Uses_stdlib
#define Uses_ctype
#define Uses_fcntl
#define Uses_sys_stat

#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_TEvent
#define Uses_TEventQueue
#define Uses_TCEditWindow
#define Uses_TScreen
#define Uses_TStringCollection
#define Uses_TVCodePage

#define Uses_TGKey
#include <ceditor.h>
#define Uses_TSetEditorApp
#define Uses_SETAppVarious
#include <setapp.h>
#include <intermp3.h>
#include <runprog.h>
#include <pathtool.h>
#include <dyncat.h>
#include <rhutils.h>

#define InlineAsm __asm__ __volatile__

const unsigned scrsvNeedsResetVideoMode=1;

typedef struct
{
 char Name[24];
 void (*update)();
 void (*start)();
 void (*shutDown)();
 unsigned flags;
} ScrSaver;

static TStringCollection *ScrSavers=0;

static void CreateScrSaversList(void);
static void KillAllocatedMemory(void);
static const char *ExternalScrSavListFile="extrscsv.txt";

#define MaxRow()  TDisplay::getRows()
#define MaxCol()  TDisplay::getCols()

class TAllScreen : public TView
{
public:
 TAllScreen();
 virtual void draw();
 void printAt(int x, int y, const char *s);
 void printAt(int x, int y, const char *s, char color);
 void scrollUp(void);
 char *buffer;
 static uchar curColor;
 int numRows,numCols;
};

uchar TAllScreen::curColor=7;

void TAllScreen::draw()
{
 writeBuf(0,0,numCols,numRows,buffer);
}

TAllScreen::TAllScreen() : TView(TRect(0,0,TScreen::screenWidth,TScreen::screenHeight))
{
 numRows=TScreen::screenHeight;
 numCols=TScreen::screenWidth;
 buffer=new char[numRows*numCols*2];
 memset(buffer,0,numRows*numCols*2);
}

void TAllScreen::printAt(int x, int y, const char *s)
{
 if (y>numRows-1)
    return;
 int off=(x+y*numCols)*2;
 while (*s && x<numCols)
   {
    buffer[off]=*s;
    buffer[off+1]=curColor;
    off+=2;
    s++;
    x++;
   }
}

void TAllScreen::printAt(int x, int y, const char *s, char color)
{
 char aux=curColor;
 curColor=color;
 printAt(x,y,s);
 curColor=aux;
}


void TAllScreen::scrollUp(void)
{
 memcpy(buffer,buffer+numCols*2,numCols*(numRows-1)*2);
 memset(buffer+numCols*(numRows-1)*2,0,numCols*2);
}

TAllScreen *scrn;

//#define HIDE_MOUSE
#ifdef TVCompf_djgpp
extern int emulate_mouse;
#define EmulateMouse(a) emulate_mouse=a
#else
#define EmulateMouse(a)
#endif

void TSetEditorApp::screenSaver()
{
 TEvent event;

 if (!MaxRow() || !MaxCol())
    return;

 int type;
 void *scrSav=WichSaverIs(WhichScrSaver,type);
 if (!scrSav)
    return;

 if (type==scsvInternal)
   {
    ScrSaver *ss=(ScrSaver *)scrSav;
    scrn=new TAllScreen();
    int scrMode=TScreen::screenMode;
   
    TProgram::deskTop->setState(sfVisible,False);
    #ifndef HIDE_MOUSE
    // I can't hide the mouse I don't know why so I preffer to disable it
    TMouse::suspend();
    #endif
   
    ss->start();
   
    #ifdef HIDE_MOUSE
    TMouse::hide();
    EmulateMouse(1);
    TMouse::hide();
    #endif
    insert(scrn);
    MP3Buffered;
    ProcessMP3Idle;
    do
      {
       ss->update();
       #ifdef HIDE_MOUSE
       TMouse::hide();
       #endif
       ProcessMP3Idle;
       CLY_YieldProcessor(-1);
       clearEvent(event);
       event.getKeyEvent();
       #ifdef HIDE_MOUSE
       if (!(event.what & evKeyDown))
          event.getMouseEvent();
       #endif
      }
    while ((event.what & (evKeyDown | evMouseDown))==0);
    #ifdef HIDE_MOUSE
    TMouse::show();
    EmulateMouse(0);
    TMouse::show();
    #endif
    ss->shutDown();
   
    remove(scrn);
    CLY_destroy(scrn);
    ProcessMP3Idle;
    MP3UnBuffered;
    if (ss->flags & scrsvNeedsResetVideoMode)
       ResetVideoMode(scrMode);

    #ifndef HIDE_MOUSE
    TMouse::resume();
    #endif
    TProgram::deskTop->setState(sfVisible,True);
    TProgram::resetIdleTime();
   
    TProgram::deskTop->redraw();
    TProgram::application->redraw();
   }
 else
   {
    // Create a file containing the screen contents
    unsigned len=MaxRow()*MaxCol()*2;
    char *buf;
    buf=new char[len<80 ? 80 : len];

    TScreen::getCharacters(0,(ushort *)buf,len/2);
    char *name=unique_name("sc");
    int h=open(name,O_RDWR | O_CREAT | O_TRUNC | O_BINARY,0);
    if (h>=0)
      {
       ::write(h,buf,len);
       #ifdef TVOS_UNIX
       // In UNIX we can delete the file so nobody will know about it.
       unlink(name);
       #endif
      }

    // Create the command line
    sprintf(buf," 0x%04X 0x%04X 0x%08X %d ",TScreen::screenWidth,TScreen::screenHeight,1,h);
    DynStrCatStruct st;
    DynStrCatInit(&st,ExpandHome((char *)scrSav));
    DynStrCat(&st,buf);
    DynStrCat(&st,ExtScrSaverOpts);
    RunExternalProgram(st.str,repDontShowDialog | repDontShowAsMessage |
                       repRestoreScreen);
    free(st.str);
    if (h>=0)
      {
       close(h);
       unlink(name);
      }
    delete[] buf;
    string_free(name);
    RunExternalProgramKillFile();
   }
}

static
void Dummy(void) {}

static
void UpdateSilly(void)
{
 static int i=0;
 int k;

 if (!(i%6))
   {
    k=i%60;
    char *s=TVIntl::getTextNew(__("Screen saver. Press a key"));
    scrn->printAt(k,scrn->numRows-1,s);
    DeleteArray(s);
   }
 scrn->scrollUp();
 scrn->draw();
 i++;
 CLY_YieldProcessor(55000);
 CLY_YieldProcessor(55000);
}

ScrSaver Silly=
{
 "A very silly one",
 UpdateSilly,Dummy,Dummy,
 0
};

ScrSaver Blanker=
{
 "Blanker",
 Dummy,Dummy,Dummy,
 0
};


#if defined(TVCompf_djgpp) || defined(HAVE_AA)

/***************************** Common plasma code ***********************************/
// 0.5 degrees of resolution, 256==360 is faster but the resolution is too bad :-(
#define TrigTableSize  (360*2)
#define TamReal (TrigTableSize*2+TrigTableSize/4) // 2*360+90 degrees
#define cos_table (sin_table+TrigTableSize/4)
static int *sin_table;
//static uchar *screen_buffer; to the stack
#define IncWithWrap(a)   if (++a==TrigTableSize) a=0

static int dsI7=0,dsX7=0,dsY7=0,direction7=1;

typedef unsigned char RawPal[768];

static
void RPF_MakeBlueGreen_RedBars(unsigned char *Pal)
{
 int i, k;
 
 for (i=0, k=0; i<64; i++)
    {
     Pal[k++] = 63-i;
     Pal[k++] = 0;
     Pal[k++] = i;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 0;
     Pal[k++] = i;
     Pal[k++] = 63;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = 0;
     Pal[k++] = 63-i;
     Pal[k++] = 63;
    }
 for (i=0; i<64; i++)
    {
     Pal[k++] = i;
     Pal[k++] = 0;
     Pal[k++] = 63-i;
    }
}

#endif // defined(TVCompf_djgpp) || defined(HAVE_AA)



#ifdef TVCompf_djgpp

/***************************** Red and Blue plasma ***********************************/
// This code is relative compact under DOS
#include <dpmi.h>
#include <sys/movedata.h>
#include <sys/segments.h>
#include <math.h>
#include <pc.h>
#include <go32.h>

extern __inline__ void RPF_SetAllPal(unsigned char *_pal_ptr)
{//Sets all 768 6bit color component entries on the VGA.
 int dummy;
 InlineAsm (
"     movl $0x3c8, %%edx         \n"
"     xorl %%eax, %%eax          \n"
"     outb %%al, %%dx            \n"
"     incl %%edx                 \n"
"     movl $768, %%ecx           \n"
"     cli                        \n"
"     rep                        \n"
"     outsb                      \n"
"     sti                        \n"
 : "=S" (dummy) : "S" (_pal_ptr) : "%eax", "%ecx", "%edx");
}

static
void StartPlasmaRB1(void)
{
 __dpmi_regs r;
 r.x.ax=0x13;
 __dpmi_int(0x10,&r);
 dsI7=0;
 dsX7=0;
 dsY7=0;
 direction7=1;
 sin_table=new int[TamReal];
 //screen_buffer=new uchar[64000];

 unsigned int i;
 double temp;

 for (i=0; i<TamReal; i++)
    {
     temp = sin((double)(i*PI/(180*(TrigTableSize/360)) ));
     temp *= 255;
     sin_table[i] = (int)temp;
    }

 RawPal BlueP1;
 RPF_MakeBlueGreen_RedBars(BlueP1);
 RPF_SetAllPal(BlueP1);
}

static
void ShutDownPlasmaRB1(void)
{
 __dpmi_regs r;
 r.x.ax=0x3;
 __dpmi_int(0x10,&r);
 delete sin_table;
// delete screen_buffer;
}


static
void owr_vsync()
{
 while (inportb(0x3DA) & 1);
 while (!(inportb(0x3DA) & 8));
}

static
void PLA1_Step320x200_7(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=sin_table[dsI7]+256;

 for (Y=200; Y; Y--)
    {
     int temp=dsIl;
     for (X=320; X; s++,X--)
        {
         IncWithWrap(temp);
         *s=cos_table[X+dsX7]+sin_table[Y+dsY7]+sin_table[temp];
        }
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI7);
 if (dsY7==(TrigTableSize-200))
    direction7=-1;
 else
   if (dsY7==0 && direction7==-1)
      direction7=1;
 dsY7+=direction7;
 dsX7=(cos_table[dsI7]+256)>>1;
}

static
void UpdatePlasmaRB1(void)
{
 uchar screen_buffer[64000]; // I think isn't so dangerous here
 PLA1_Step320x200_7(screen_buffer);
 owr_vsync();
 _movedatal(_my_ds(),(unsigned)screen_buffer,_dos_ds,0xA0000,16000);
}


ScrSaver PlasmaRB1=
{
 "Plasma (Red/Blue)",
 UpdatePlasmaRB1,StartPlasmaRB1,ShutDownPlasmaRB1,
 scrsvNeedsResetVideoMode
};


/****************************** Inferno 1 **************************************/
/***************************************************************************

  Description:
  I got it from the djgpp list posted by Rylan. Then I modified the code to
really work and added some special touchs to make it really cool.

***************************************************************************/

/* 4 more because the bottom lines are too noisy */
#define SIZE_INFERNO (320*104)

static uchar *Inferno1VScreen;

static
void RPF_MakeRedPalette(unsigned char *Pal)
{
 int i,j;
 uchar *s=Pal;

 for (i=0; i<255; i++)
    {
     if (i<64)
        j=0;
     else
        if (i<128)
           j=i-64;
        else
           if (i<192)
              j=191-i;
           else
              j=0;
     *s=j;
     *(s+1)=*(s+2)=0;
     s+=3;
    }
}

static
void RPF_MakeGrayPalette(unsigned char *Pal)
{
 int i,j;
 uchar *s=Pal;

 for (i=0; i<255; i++)
    {
     if (i<64)
        j=0;
     else
        if (i<128)
           j=i-64;
        else
           if (i<192)
              j=191-i;
           else
              j=0;
     *s=*(s+1)=*(s+2)=j;
     s+=3;
    }
}

static
void StartInferno1(void)
{
 __dpmi_regs r;
 r.x.ax=0x13;
 __dpmi_int(0x10,&r);

 RawPal Red;
 RPF_MakeRedPalette(Red);
 RPF_SetAllPal(Red);

 Inferno1VScreen=(uchar *)calloc(1,SIZE_INFERNO+2*320);
}

static
void StartSmoker1(void)
{
 __dpmi_regs r;
 r.x.ax=0x13;
 __dpmi_int(0x10,&r);

 RawPal Gray;
 RPF_MakeGrayPalette(Gray);
 RPF_SetAllPal(Gray);

 Inferno1VScreen=(uchar *)calloc(1,SIZE_INFERNO+2*320);
}

static
void ShutDownInferno1(void)
{
 __dpmi_regs r;
 r.x.ax=0x3;
 __dpmi_int(0x10,&r);
 delete Inferno1VScreen;
}


void DibujarInferno1(uchar *screen)
{
 InlineAsm
 (
"   movl $320,%%esi         \n"
"   addl %0,%%esi           \n"
"   movl %1,%%ecx           \n"

"   movl $0,%%ebx           \n"

// Smooth
"0:                         \n"
"   movl $0,%%eax           \n"
   // Current byte and increment
"   lodsb                   \n"
   // Next
"   movb (%%esi),%%bl       \n"
"   addl %%ebx,%%eax        \n"
   // Y+1
"   movb 319(%%esi),%%bl    \n"
"   addl %%ebx,%%eax        \n"
   // Previous
"   movb -2(%%esi),%%bl     \n"
"   addl %%ebx,%%eax        \n"
   // Average
"   shrl $2,%%eax           \n"
   // Put it in Y-1
"   movb %%al,-321(%%esi)   \n"
"   loop 0b                 \n"

"   movl %1,%%edi           \n"
"   addl %0,%%edi           \n"
"   movl $320,%%ecx         \n"

// Randline
"1:                         \n"
"   mull (%%edi)            \n"
"   incl %%eax              \n"
"   stosw                   \n"
"   decl %%edi              \n"
"   loop 1b                 \n"
 :
 : "qm" (screen), "i" (SIZE_INFERNO)
 : "edi","esi","eax","ebx","ecx","edx","memory"
 );
}

static
void UpdateInferno1(void)
{
 int d1,d2,d3;
 DibujarInferno1(Inferno1VScreen);
 owr_vsync();
 InlineAsm(
" pushl %%es                      \n"
 
" movw  %%ax,%%es                 \n"
" movl  $100,%%edx                \n"
"0:                               \n"
" movl  $80,%%ecx                 \n"
"1:                               \n"
" movl  (%%esi),%%eax             \n"
" movl  %%eax,%%es:(%%edi)        \n"
" movl  %%eax,%%es:320(%%edi)     \n"
" addl  $4,%%esi                  \n"
" addl  $4,%%edi                  \n"
" loop  1b                        \n"
" addl  $320,%%edi                \n"
" decl  %%edx                     \n"
" jnz   0b                        \n"

" popl  %%es                      \n"
   : "=S" (d1), "=D" (d2), "=a" (d3)
   : "a" (_dos_ds), "S" (Inferno1VScreen), "D" (0xA0000)
   : "ecx", "edx" );
}

ScrSaver Inferno1=
{
 "Inferno 1",
 UpdateInferno1,StartInferno1,ShutDownInferno1,
 scrsvNeedsResetVideoMode
};

ScrSaver Smoker1=
{
 "Smoker 1",
 UpdateInferno1,StartSmoker1,ShutDownInferno1,
 scrsvNeedsResetVideoMode
};
#endif // TVCompf_djgpp


#ifdef HAVE_AA

/************************ Red and Blue plasma adapted for AA-lib **************************/
#include <aalib.h>
#include <math.h>

static aa_context *context=NULL;
static unsigned aaW,aaH;
static aa_palette pal;

static
void RPF_SetAllPalAA(unsigned char *pal_ptr)
{
 int i,j;
 for (i=0,j=0; i<256; i++,j+=3)
     aa_setpalette(pal,i,pal_ptr[j]*4,pal_ptr[j+1]*4,pal_ptr[j+2]*4);
}

static
void StartPlasmaRB1AA(void)
{
 aa_defparams.supported|=AA_EXTENDED;
 context=aa_autoinit(&aa_defparams);
 if (!context)
    return;
 aaW=aa_imgwidth(context);
 aaH=aa_imgheight(context);
   
 dsI7=0;
 dsX7=0;
 dsY7=0;
 direction7=1;
 sin_table=new int[TamReal];
 //screen_buffer=new uchar[64000];

 unsigned int i;
 double temp;

 for (i=0; i<TamReal; i++)
    {
     temp = sin((double)(i*M_PI/(180*(TrigTableSize/360)) ));
     temp *= 255;
     sin_table[i] = (int)temp;
    }

 RawPal BlueP1;
 RPF_MakeBlueGreen_RedBars(BlueP1);
 RPF_SetAllPalAA(BlueP1);
}

static
void ShutDownPlasmaRB1AA(void)
{
 delete sin_table;
 if (context)
    aa_close(context);
 TScreen::setCursorType(TScreen::cursorLines);
}


static
void PLA1_Step_7(unsigned char *screen_buffer)
{
 int X,Y;
 unsigned char *s=screen_buffer;
 int dsIl=sin_table[dsI7]+256;

 for (Y=aaH; Y; Y--)
    {
     int temp=dsIl;
     for (X=aaW; X; s++,X--)
        {
         IncWithWrap(temp);
         *s=cos_table[X+dsX7]+sin_table[Y+dsY7]+sin_table[temp];
        }
     IncWithWrap(dsIl);
    }
 IncWithWrap(dsI7);
 if (dsY7==(TrigTableSize-200))
    direction7=-1;
 else
   if (dsY7==0 && direction7==-1)
      direction7=1;
 dsY7+=direction7;
 dsX7=(cos_table[dsI7]+256)>>1;
}

static
void UpdatePlasmaRB1AA()
{
 if (!context) return;
 PLA1_Step_7(aa_image(context));
 //aa_fastrender(context,0,0,aa_scrwidth(context),aa_scrheight(context));
 aa_renderpalette(context,pal,&aa_defrenderparams,0,0,aa_scrwidth(context),aa_scrheight(context));
 aa_flush(context);
}


ScrSaver PlasmaRB1AA=
{
 "AA-Plasma (Red/Blue)",
 UpdatePlasmaRB1AA,StartPlasmaRB1AA,ShutDownPlasmaRB1AA,
 scrsvNeedsResetVideoMode
};
#endif // HAVE_AA


/************************** Text Stars saver *****************************/
static char *cCartel1=__("Press a key to continue");

static char s1[2]="ú";
static char s2[2]="ù";
static char s3[2]="þ";
static char s4[2]="\xF";
static char p1[2]="ú";
static char p2[2]="ù";
static char p3[2]="þ";
static char p4[2]="\xF";
char *cFormaScreenSaverStars[]={s1,s2,s3,s4};
char *coFormaScreenSaverStars[]={p1,p2,p3,p4};
static char  cColor[]={0xB,0xB,0x7,0xF};
static char anX[16],anY[16],anGrado[16];
static char *cBorra;
static int nFilRel,nColRel,nStepX,nRow;
static const char *cCartel;

static
void InitTextStars(void)
{
 int nI,nJ;
 cCartel=TVIntl::getTextNew(cCartel1);

 nFilRel=MaxRow()/2;
 nColRel=MaxCol()/2;

 for (nI=0; nI<4; nI++)
     for (nJ=0; nJ<4; nJ++)
         anGrado[nJ*4+nI]=-nI;
 for (nI=0; nI<16; nI++)
     anX[nI]=anY[nI]=0;

 int nLen=strlen(cCartel);
 cBorra=new char[nLen+1];
 memset(cBorra,' ',nLen);
 cBorra[nLen]=0;
 nStepX=0;
 nRow=MaxRow()/2;
 srand(0);
}

static
void DeInitTextStars(void)
{
 DeleteArray(cBorra);
 DeleteArray(cCartel);
}

static
void UpdateTextStars(void)
{
 int nTot;
 signed char nGrado,nJ;

 int nLen=strlen(cCartel);
 nTot=MaxCol()+nLen;

 if (nStepX<nLen)
    scrn->printAt(0,nRow,&cCartel[nLen-nStepX]);
 else
    scrn->printAt(nStepX-nLen,nRow,cCartel);
 for (nJ=0; nJ<16; nJ++)
    {
     nGrado=anGrado[nJ];
     if (nGrado==0)
       {
        scrn->printAt(anX[nJ],anY[nJ]," ");
        anX[nJ]=rand()%MaxCol();
        anY[nJ]=rand()%MaxRow();
       }
     if (nGrado>-1)
        scrn->printAt(anX[nJ],anY[nJ],cFormaScreenSaverStars[nGrado],cColor[nGrado]);
     if (nGrado==3)
        anGrado[nJ]=0;
     else
        anGrado[nJ]=nGrado+1;
    }
 scrn->draw();
 CLY_YieldProcessor(55000);
 CLY_YieldProcessor(55000);
 if (nStepX<nLen)
    scrn->printAt(0,nRow,cBorra);
 else
    scrn->printAt(nStepX-nLen,nRow,cBorra);
 nStepX++;
 if (nStepX==nTot)
   {
    nStepX=0;
    nRow=rand()%MaxRow();
   }
}

ScrSaver TextStars=
{
 "Text Stars",
 UpdateTextStars,InitTextStars,DeInitTextStars,
 0
};
/************************** End Text Stars saver *****************************/

#ifdef TVCompf_djgpp
/************************** Safe saver *****************************/
static char *cCartel2=__("That's a 'safe' screen saver. The editor has a beautiful "
                         "plasma screen saver but currently it isn't the default "
                         "because some (really buggy) Winshit 9x video drivers "
                         "do strange things when a windowed DOS program switches "
                         "to video mode. I hate it ....");

// Shared with Text Stars
//#define MaxRow()  TDisplay::getRows()
//#define MaxCol()  TDisplay::getCols()
//static char *cCartel;

static
void InitSafeSaver(void)
{
 cCartel=TVIntl::getTextNew(cCartel2);

 int nLen=strlen(cCartel);
 cBorra=new char[nLen+1];
 memset(cBorra,' ',nLen);
 cBorra[nLen]=0;
 nStepX=0;
 nRow=MaxRow()/2;
}

static
void DeInitSafeSaver(void)
{
 DeleteArray(cBorra);
 DeleteArray(cCartel);
}

static
void UpdateSafeSaver(void)
{
 int nTot,nW;
 int nLen=strlen(cCartel);
 nTot=MaxCol()+nLen;
 nW=MaxCol()-1;

 if (nStepX>nW)
    scrn->printAt(0,nRow,cCartel+nStepX-nW);
 else
    scrn->printAt(nW-nStepX,nRow,cCartel);
 scrn->draw();

 CLY_YieldProcessor(55000);
 CLY_YieldProcessor(55000);
 CLY_YieldProcessor(55000);

 scrn->printAt(0,nRow,cBorra);
 nStepX++;
 if (nStepX==nTot)
   {
    nStepX=0;
    nRow=rand()%MaxRow();
   }
}

ScrSaver SafeSaver=
{
 "'Safe' saver",
 UpdateSafeSaver,InitSafeSaver,DeInitSafeSaver,
 0
};
/************************** End of Safe saver *****************************/
#endif


#ifdef TVCompf_djgpp
#include <dos.h> // get_dos_version
static
void CreateScrSaversList(void)
{
 ScrSavers=new TStringCollection(3,1);
 ScrSavers->insert(&PlasmaRB1);
 ScrSavers->insert(&Silly);
 ScrSavers->insert(&TextStars);
 ScrSavers->insert(&SafeSaver);
 ScrSavers->insert(&Inferno1);
 ScrSavers->insert(&Smoker1);
 ScrSavers->insert(&Blanker);
 ScrSavers->setOwnerShip(False);
 atexit(KillAllocatedMemory);
}

char *GetDefaultScreenSaver(void)
{
 // Use the 'safe' saver for crap 9x
 unsigned short true_dos_version=_get_dos_version(1);
 if (true_dos_version>=0x700)
    return newStr(SafeSaver.Name);
 // Fuck you Bill
 return newStr(PlasmaRB1.Name);
}
#else
static
void CreateScrSaversList(void)
{
 ScrSavers=new TStringCollection(3,1);
 #ifdef HAVE_AA
 ScrSavers->insert(&PlasmaRB1AA);
 #endif
 ScrSavers->insert(&Silly);
 ScrSavers->insert(&TextStars);
 ScrSavers->setOwnerShip(False);
 atexit(KillAllocatedMemory);
}

char *GetDefaultScreenSaver(void)
{
 #ifdef HAVE_AA
 return newStr(PlasmaRB1AA.Name);
 #else
 return newStr(Silly.Name);
 #endif
}
#endif

static
TStringCollection *GetScreenSaverListInt(void)
{
 if (!ScrSavers)
    CreateScrSaversList();
 return ScrSavers;
}

/************************ External Screen Savers ***************************/

typedef struct
{
 char *name;
 char *program;
} ExtrScrSav;

class TExtrScrSavList : public TStringCollection
{
public:
 TExtrScrSavList();
 virtual void freeItem(void *item);
 virtual void *keyOf(void *item);
 void insert(char *name, char *program);
 char *atProgram(ccIndex pos);
 CLY_DummyTStreamRW(TStringCollection)
};

TExtrScrSavList::TExtrScrSavList() :
  TStringCollection(4,4)
{
}

void TExtrScrSavList::freeItem(void *item)
{
 ExtrScrSav *p=(ExtrScrSav *)item;
 delete[] p->name;
 delete[] p->program;
 delete p;
}

void *TExtrScrSavList::keyOf(void *item)
{
 return ((ExtrScrSav *)item)->name;
}

void TExtrScrSavList::insert(char *name, char *program)
{
 ExtrScrSav *p=new ExtrScrSav;
 p->name=name;
 p->program=program;
 TStringCollection::insert(p);
}

char *TExtrScrSavList::atProgram(ccIndex pos)
{
 ExtrScrSav *p=(ExtrScrSav *)at(pos);
 return p->program;
}

static void ReplaceCRby0(char *s)
{
 for (; *s && *s!='\n' && *s!='\r'; s++);
 *s=0;
}

static TExtrScrSavList *ExtrScrSavList=0;

static
void CreateExtrScrSavList(int forceReRead)
{
 if (ExtrScrSavList)
   {
    if (forceReRead)
       CLY_destroy(ExtrScrSavList);
    else
       return;
   }
 char *fileName=ExpandHome(ExternalScrSavListFile);
 FILE *f=fopen(fileName,"rt");
 if (!f)
    return;
 ExtrScrSavList=new TExtrScrSavList();
 char buffer[PATH_MAX],*name;
 while (fgets(buffer,PATH_MAX,f))
   {// Skip blank or commented lines
    if (*buffer=='#' || ucisspace(*buffer)) continue;
    ReplaceCRby0(buffer);
    name=newStr(buffer);
    if (!fgets(buffer,PATH_MAX,f))
      {
       delete[] name;
       break;
      }
    ReplaceCRby0(buffer);
    ExtrScrSavList->insert(name,newStr(buffer));
   }
 fclose(f);
}

static
TExtrScrSavList *GetScreenSaverListExt(int forceReRead)
{
 CreateExtrScrSavList(forceReRead);
 return ExtrScrSavList;
}

/**[txh]********************************************************************

  Description:
  That's what the outside world sees.

  Return:
  A string collection with the list of all available screen saver. It must
be destroyed by the caller.

***************************************************************************/

TStringCollection *GetScreenSaverList(int forceReRead)
{
 int cInt=0,cExt=0,i;
 TStringCollection *internal=GetScreenSaverListInt();
 TStringCollection *external=GetScreenSaverListExt(forceReRead);
 if (internal)
    cInt=internal->getCount();
 if (external)
    cExt=external->getCount();
 TStringCollection *ret=new TStringCollection(cInt+cExt,4);

 for (i=0; i<cInt; i++)
     ret->insert(newStr((char *)internal->keyOf(internal->at(i))));
 for (i=0; i<cExt; i++)
     ret->insert(newStr((char *)external->keyOf(external->at(i))));

 return ret;
}

void *WichSaverIs(char *name, int &type)
{
 TStringCollection *c=GetScreenSaverListInt();
 if (name)
   {
    // Check if that's an internal
    ccIndex pos;
    if (c && c->search(name,pos))
      {
       type=scsvInternal;
       return c->at(pos);
      }
    // Check if that's an external
    TExtrScrSavList *ex=GetScreenSaverListExt(0);
    if (ex && ex->search(name,pos))
      {
       type=scsvExternal;
       return ex->atProgram(pos);
      }
   }

 return c ? c->at(0) : 0;
}

static
char *GetScrSaverOut(char *name, char *option)
{
 int type;
 char *namePrg=(char *)WichSaverIs(name,type);
 if (type!=scsvExternal)
    return 0;
 char *fullName=ExpandHome(namePrg);
 if (!edTestForFile(fullName))
    return 0;
 strcat(fullName,option);
 RunExternalProgram(fullName,repDontShowAsMessage);
 type=2048;
 return RunExternalProgramGetFile(type);
}

char *GetScrSaverInfo(char *name)
{
 return GetScrSaverOut(name," -i");
}

char *GetScrSaverHelp(char *name)
{
 return GetScrSaverOut(name," -h");
}

static
void KillAllocatedMemory(void)
{
 destroy0(ScrSavers);
 destroy0(ExtrScrSavList);
}




