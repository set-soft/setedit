/**[txh]********************************************************************

  Description:
  Mixer rutines for Sound Blaster Pro and 16.
  Copyright (c) 2000 by Salvador E. Tropea.
  Part of this code (board initialization) were taked from Allegro.
  This code is under the GPL license except Allegro part.

***************************************************************************/

#include <configed.h>

#if defined(SECompf_djgpp) && defined(HAVE_MIXER)
#include <stdlib.h>
#include <string.h>
#include <pc.h>
#include "mixer.h"

/*****************************************************************************

 As I don't know if this code will be integrated to Allego I'm taking the
needed routines and putting them in this module.

*****************************************************************************/

static int sb_dsp_ver = -1;               /* SB DSP version */
static int _sb_port = -1;
static int sb_hw_dsp_ver = -1;            /* as reported by autodetect */

/* sb_write_dsp:
 *  Writes a byte to the SB DSP chip. Returns -1 if it times out.
 */
static inline volatile int sb_write_dsp(unsigned char byte)
{
   int x;

   for (x=0; x<0xffff; x++) {
      if (!(inportb(0x0C+_sb_port) & 0x80)) {
	 outportb(0x0C+_sb_port, byte);
	 return 0;
      }
   }
   return -1; 
}

/* sb_read_dsp:
 *  Reads a byte from the SB DSP chip. Returns -1 if it times out.
 */
static inline volatile int sb_read_dsp()
{
   int x;

   for (x=0; x<0xffff; x++)
      if (inportb(0x0E + _sb_port) & 0x80)
	 return inportb(0x0A+_sb_port);

   return -1; 
}


/* _sb_reset_dsp:
 *  Resets the SB DSP chip, returning -1 on error.
 */
static
int _sb_reset_dsp(int data)
{
   int x;

   outportb(0x06+_sb_port, data);

   for (x=0; x<8; x++)
      inportb(0x06+_sb_port);

   outportb(0x06+_sb_port, 0);

   if (sb_read_dsp() != 0xAA)
      return -1;

   return 0;
}

/* _sb_read_dsp_version:
 *  Reads the version number of the SB DSP chip, returning -1 on error.
 */
static
int _sb_read_dsp_version()
{
   int x, y;

   if (sb_hw_dsp_ver > 0)
      return sb_hw_dsp_ver;

   if (_sb_port <= 0)
      _sb_port = 0x220;

   if (_sb_reset_dsp(1) != 0) {
      sb_hw_dsp_ver = -1;
   }
   else {
      sb_write_dsp(0xE1);
      x = sb_read_dsp();
      y = sb_read_dsp();
      sb_hw_dsp_ver = ((x << 8) | y);
   }

   return sb_hw_dsp_ver;
}


/* sb_detect:
 *  SB detection routine. Uses the BLASTER environment variable,
 *  or 'sensible' guesses if that doesn't exist.
 */
static int sb_detect(void)
{
   char *blaster = getenv("BLASTER");

   /* parse BLASTER env */
   if (blaster) { 
      while (*blaster) {
	 while ((*blaster == ' ') || (*blaster == '\t'))
	    blaster++;

	 if (*blaster) {
	    switch (*blaster) {

	       case 'a': case 'A':
		  if (_sb_port < 0)
		     _sb_port = strtol(blaster+1, NULL, 16);
		  break;

	       case 'i': case 'I':
	       case 'd': case 'D':
	       case 'h': case 'H':
		     strtol(blaster+1, NULL, 10);
		  break;
	    }

	    while ((*blaster) && (*blaster != ' ') && (*blaster != '\t'))
	       blaster++;
	 }
      }
   }

   if (_sb_port < 0)
      _sb_port = 0x220;

   /* make sure we got a good port address */
   if (_sb_reset_dsp(1) != 0) { 
      static int bases[] = { 0x210, 0x220, 0x230, 0x240, 0x250, 0x260, 0 };
      int i;

      for (i=0; bases[i]; i++) {
	 _sb_port = bases[i];
	 if (_sb_reset_dsp(1) == 0)
	    break;
      }
   }

   /* check if the card really exists */
   _sb_read_dsp_version();
   if (sb_hw_dsp_ver < 0) {
      return 1;
   }
   sb_dsp_ver = sb_hw_dsp_ver;

   /* what breed of SB? */
   // sb_dsp_ver >= 0x400) "SB 16"
   // sb_dsp_ver >= 0x300) "SB Pro"
   // sb_dsp_ver >= 0x201) "SB 2.0"
   // sb_dsp_ver >= 0x200) "SB 1.5"
   // else                 "SB 1.0";
   return 0;
}


/*****************************************************************************

 End of Allegro code.

*****************************************************************************/

// 0-255 to 0-100
#define SB16ValToMixer(a)    (((a|7)*100+100)/255)
#define SBProLeftToMixer(a)  ((a & 0xF)*100/15)
#define SBProRightToMixer(a) ((a>>4)*100/15)
#define SBProMicToMixer(a)   (((a & 7)>>1)*100/3)
#define MixerToSB16Val(a)    (a*255/100)
#define MixerToSBPro(l,r)    (((l*15/100) & 0xF) | ((r*15/100) << 4))
#define MixerToSBProMic(a)   ((a*3/100)<<1)

static BOARD_MIXER *elements=0;
static int isSB16;
static int lastElement=0;

static void SBReadRealMixerValue(int id, int pos);

#include <stdio.h>

static inline
void WriteMixer(int index, unsigned char value)
{
 outportb(_sb_port+4,index);
 outportb(_sb_port+5,value);
}

static inline
unsigned char ReadMixer(int index)
{
 outportb(_sb_port+4,index);
 return inportb(_sb_port+5);
}

static BOARD_MIXER SBProMixer[]=
{
 {
  BOARD_MIXER_ID_MIXER_VOLUME,
  lMxVol,sMxVol,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }, // reg 02h (Only SBPro) and 22h
 {
  BOARD_MIXER_ID_MIXER_SYNTH,
  lMxSynth,sMxSynth,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }, // reg 26h
 {
  BOARD_MIXER_ID_MIXER_PCM,
  lMxWave,sMxWave,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }, // reg 04h
 {
  BOARD_MIXER_ID_MIXER_LINE,
  lMxLine,sMxLine,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }, // reg 2Eh
 {
  BOARD_MIXER_ID_MIXER_MIC,
  lMxMic,sMxMic,
  BOARD_MIXER_ELEM_MONO,-1,-1
 }, // reg 0Ah
 {
  BOARD_MIXER_ID_MIXER_CD,
  lMxCD,sMxCD,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }  // reg 28h
};

static unsigned char SBProMixerReg[]=
{
 0x22,0x26,0x04,0x2E,0x0A,0x28
};

static BOARD_MIXER SB16Mixer[]=
{
 {
  BOARD_MIXER_ID_MIXER_VOLUME,
  lMxVol,sMxVol,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }, // reg 30/1h
 {
  BOARD_MIXER_ID_MIXER_BASS,
  lMxBass,sMxBass,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }, // reg 46/7h
 {
  BOARD_MIXER_ID_MIXER_TREBLE,
  lMxTrebl,sMxTrebl,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }, // reg 44/5h
 {
  BOARD_MIXER_ID_MIXER_SYNTH,
  lMxSynth,sMxSynth,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }, // reg 34/5h
 {
  BOARD_MIXER_ID_MIXER_PCM,
  lMxWave,sMxWave,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }, // reg 32/3h
 {
  BOARD_MIXER_ID_MIXER_SPEAKER,
  lMxSpkr,sMxSpkr,
  BOARD_MIXER_ELEM_MONO,-1,-1
 }, // reg 3Bh
 {
  BOARD_MIXER_ID_MIXER_LINE,
  lMxLine,sMxLine,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }, // reg 38/9h
 {
  BOARD_MIXER_ID_MIXER_MIC,
  lMxMic,sMxMic,
  BOARD_MIXER_ELEM_MONO,-1,-1
 }, // reg 3Ah
 {
  BOARD_MIXER_ID_MIXER_CD,
  lMxCD,sMxCD,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 },  // reg 36/7h
 {
  BOARD_MIXER_ID_MIXER_IGAIN,
  lMxIGain,sMxIGain,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 },  // reg 3F/40h
 {
  BOARD_MIXER_ID_MIXER_OGAIN,
  lMxOGain,sMxOGain,
  BOARD_MIXER_ELEM_STEREO,-1,-1
 }  // reg 41/2h
};

static unsigned char SB16MixerReg[]=
{
 0x30,0x46,0x44,0x34,0x32,0x3B,0x38,0x3A,0x36,0x3F,0x41
};

int SBMixerInit()
{
 int i;

 if (sb_dsp_ver!=-1) return 1;
 if (sb_detect() ||    // !SB found or
     sb_dsp_ver<0x300) // too old, only SB Pro and newers have mixer
    return 0;
 if (sb_dsp_ver>=0x400)
   {
    elements=SB16Mixer;
    isSB16=1;
    lastElement=sizeof(SB16Mixer)/sizeof(BOARD_MIXER);
   }
 else
   {
    elements=SBProMixer;
    isSB16=0;
    lastElement=sizeof(SBProMixer)/sizeof(BOARD_MIXER);
   }
 // Read current values
 for (i=0; i<lastElement; i++)
     SBReadRealMixerValue(elements[i].id,i);
 return 1;
}

static
int FindElement(int id)
{
 int i;
 if (!elements) return -1;
 for (i=0; i<lastElement; i++)
     if (elements[i].id==id) break;
 return i<lastElement ? i : -1;
}

static inline
void ReadSB16MixerStereo(int pos)
{
 elements[pos].left =SB16ValToMixer(ReadMixer(SB16MixerReg[pos]));
 elements[pos].right=SB16ValToMixer(ReadMixer(SB16MixerReg[pos]+1));
}

static inline
void ReadSB16MixerMono(int pos)
{
 elements[pos].right=
 elements[pos].left =SB16ValToMixer(ReadMixer(SB16MixerReg[pos]));
}

static inline
void ReadSBProMixerStereo(int pos)
{
 int vol=ReadMixer(SBProMixerReg[pos]);
 elements[pos].left =SBProLeftToMixer(vol);
 elements[pos].right=SBProRightToMixer(vol);
}

static inline
void ReadSBProMixerMono(int pos)
{
 elements[pos].right=
 elements[pos].left =SBProMicToMixer(ReadMixer(SBProMixerReg[pos]));
}

static
void SBReadRealMixerValue(int id, int pos)
{
 if (!elements) return;
 if (pos<0) pos=FindElement(id);
 if (pos<0) return;

 if (isSB16)
   {
    if (IS_BOARD_MIXER_ELEM_STEREO(elements[pos]))
       ReadSB16MixerStereo(pos);
    else
       ReadSB16MixerMono(pos);
   }
 else
   {
    if (IS_BOARD_MIXER_ELEM_STEREO(elements[pos]))
       ReadSBProMixerStereo(pos);
    else
       ReadSBProMixerMono(pos);
   }
}

void SBMixerDeInit()
{
}

const BOARD_MIXER *SBGetElements(int *cant)
{
 *cant=lastElement;
 return elements;
}

static inline
void WriteSB16MixerStereo(int pos, int left, int right)
{
 WriteMixer(SB16MixerReg[pos],MixerToSB16Val(left));
 WriteMixer(SB16MixerReg[pos]+1,MixerToSB16Val(right));
}

static inline
void WriteSB16MixerMono(int pos, int vol)
{
 WriteMixer(SB16MixerReg[pos],MixerToSB16Val(vol));
}

static inline
void WriteSBProMixerStereo(int pos, int left, int right)
{
 WriteMixer(SBProMixerReg[pos],MixerToSBPro(left,right));
}

static inline
void WriteSBProMixerMono(int pos, int vol)
{
 WriteMixer(SBProMixerReg[pos],MixerToSBProMic(vol));
}

int SBSetMixerValue(int id, int left, int right)
{
 int pos;

 if (!elements) return 0;
 pos=FindElement(id);
 if (isSB16)
   {
    if (IS_BOARD_MIXER_ELEM_STEREO(elements[pos]))
       WriteSB16MixerStereo(pos,left,right);
    else
       WriteSB16MixerMono(pos,left);
   }
 else
   {
    if (IS_BOARD_MIXER_ELEM_STEREO(elements[pos]))
       WriteSBProMixerStereo(pos,left,right);
    else
       WriteSBProMixerMono(pos,left);
   }
 SBReadRealMixerValue(id,pos);
 return 1;
}

int SBReadMixerValue(int id, int *left, int *right)
{
 int elem;

 if (!elements) return 0;
 elem=FindElement(id);
 if (elem<0) return 0;
 *left=elements[elem].left;
 *right=elements[elem].right;
 return 1;
}

void SBGetName(char *buffer, int size)
{
 if (isSB16)
    strncpy(buffer,"Sound Blaster 16",size);
 else
    strncpy(buffer,"Sound Blaster Pro",size);
 buffer[size-1]=0;
}

//#define TEST
#ifdef TEST
#include <stdio.h>

int main(int argc, char *argv[])
{
 int left,right,cant,i;
 const BOARD_MIXER *e;

 if (!SBMixerInit())
   {
    printf("Can't initialize mixer\n");
    return 1;
   }

 if (!(e=SBGetElements(&cant)))
   {
    printf("Can't get list of elements\n");
    return 4;
   }
 printf("%d of supported elements:\n",cant);
 for (i=0; i<cant; i++)
    {
     if (IS_BOARD_MIXER_ELEM_STEREO(e[i]))
        printf("%s (%s) [%d,%d]\n",e[i].name,e[i].s_name,e[i].left,e[i].right);
     else
        printf("%s (%s) [%d]\n",e[i].name,e[i].s_name,e[i].left);
    }

 if (!SBReadMixerValue(BOARD_MIXER_ID_MIXER_VOLUME,&left,&right))
   {
    printf("Can't read master volume\n");
    return 2;
   }
 printf("Master: %d,%d\n",left,right);

 if (!SBSetMixerValue(BOARD_MIXER_ID_MIXER_VOLUME,50,50))
   {
    printf("Can't set master volume\n");
    return 3;
   }

 SBMixerDeInit();
 return 0;
}
#endif

#endif // djgpp && HAVE_MIXER
