#include <configed.h>
#include "mixer.h"

#ifdef HAVE_MIXER

#define drvNone   -1
#define drvFailed -2

static int Driver=drvNone,DriverOld=drvNone;

const char sMxVol[]  =" Vol ";
const char sMxBass[] ="Bass ";
const char sMxTrebl[]="Trebl";
const char sMxSynth[]="Synth";
const char sMxWave[] ="Wave ";
const char sMxSpkr[] ="Spkr ";
const char sMxLine[] ="Line ";
const char sMxMic[]  =" Mic ";
const char sMxCD[]   =" CD  ";
const char sMxMix[]  =" Mix ";
const char sMxWave2[]="Wave2";
const char sMxRec[]  =" Rec ";
const char sMxIGain[]="IGain";
const char sMxOGain[]="OGain";
const char sMxLine1[]="Line1";
const char sMxLine2[]="Line2";
const char sMxLine3[]="Line3";

const char lMxVol[]  ="master vol";
const char lMxBass[] ="bass";
const char lMxTrebl[]="treble";
const char lMxSynth[]="synthetized";
const char lMxWave[] ="wave";
const char lMxSpkr[] ="speaker";
const char lMxLine[] ="line";
const char lMxMic[]  ="mic";
const char lMxCD[]   ="cd";
const char lMxMix[]  ="mixer";
const char lMxWave2[]="wave 2";
const char lMxRec[]  ="record";
const char lMxIGain[]="input gain";
const char lMxOGain[]="output gain";
const char lMxLine1[]="line 1";
const char lMxLine2[]="line 2";
const char lMxLine3[]="line 3";

typedef struct
{
 int  (*Init)();
 void (*DeInit)();
 int  (*Set)(int id, int left, int right);
 int  (*Read)(int id, int *left, int *right);
 const BOARD_MIXER *(*GetElements)(int *cant);
 void (*GetName)(char *buffer, int size);
} stDriver;

#define MaxNameLen 80
static char DrvName[MaxNameLen];

stDriver Drivers[]=
{
#ifdef SECompf_djgpp
 { SBMixerInit,  SBMixerDeInit,  SBSetMixerValue,
   SBReadMixerValue, SBGetElements, SBGetName }
#endif

#ifdef SEOSf_Linux
 { LinuxMixerInit, LinuxMixerDeInit, LinuxSetMixerValue,
   LinuxReadMixerValue, LinuxGetElements, LinuxMixerGetName }
#endif
};

#define NumDrivers (sizeof(Drivers)/sizeof(stDriver))

int MixerInit()
{
 int i;
 if (Driver>=0) return 1;
 if (Driver!=drvFailed && DriverOld>=0)
   {
    if (Drivers[DriverOld].Init())
      {
       Driver=DriverOld;
       return 1;
      }
    DriverOld=drvFailed;
   }
 for (i=0; i<NumDrivers; i++)
     if (Drivers[i].Init())
       {
        Driver=i;
        return 1;
       }
 Driver=drvFailed;
 return 0;
}

void MixerDeInit()
{
 if (Driver<0) return;
 Drivers[Driver].DeInit();
 DriverOld=Driver;
 Driver=drvNone;
}

int SetMixerValue(int id, int left, int right)
{
 if (Driver<0) return 0;
 return Drivers[Driver].Set(id,left,right);
}

int ReadMixerValue(int id, int *left, int *right)
{
 if (Driver<0) return 0;
 return Drivers[Driver].Read(id,left,right);
}

const BOARD_MIXER *GetElements(int *cant)
{
 if (Driver<0) return 0;
 return Drivers[Driver].GetElements(cant);
}

const char *GetMixerName()
{
 if (Driver<0) return 0;
 Drivers[Driver].GetName(DrvName,MaxNameLen);
 return DrvName;
}
#else  // HAVE_MIXER
int  MixerInit() { return 0; }
int  SetMixerValue(int id, int left, int right)
{ if (id | left | right) id=0; return 0; }
void MixerDeInit() {}
const BOARD_MIXER *GetElements(int *cant)
{
 *cant=0;
 return 0;
}
#endif // HAVE_MIXER
