/**[txh]********************************************************************

  Description:
  Mixer definitions shared by all modules.
  Copyright (c) 2000-2001 by Salvador E. Tropea.
  This code is under the GPL license.

***************************************************************************/

#ifndef MIXER_BOARD_INCLUDED
#define MIXER_BOARD_INCLUDED

// Flags
#define BOARD_MIXER_ELEM_STEREO 1
#define BOARD_MIXER_ELEM_MONO   0
#define IS_BOARD_MIXER_ELEM_STEREO(a) (a.flags & BOARD_MIXER_ELEM_STEREO)

//
// Commonly found ids, is not a coincidence they are the same used by Linux kernel ;-)
// Any value greater than 0x80000000 is board specific.
//
#define BOARD_MIXER_ID_MIXER_VOLUME	0
#define BOARD_MIXER_ID_MIXER_BASS	1
#define BOARD_MIXER_ID_MIXER_TREBLE	2
#define BOARD_MIXER_ID_MIXER_SYNTH	3  /* FM */
#define BOARD_MIXER_ID_MIXER_PCM		4  /* Wav */
#define BOARD_MIXER_ID_MIXER_SPEAKER	5
#define BOARD_MIXER_ID_MIXER_LINE	6
#define BOARD_MIXER_ID_MIXER_MIC		7
#define BOARD_MIXER_ID_MIXER_CD		8
#define BOARD_MIXER_ID_MIXER_IMIX	9	/*  Recording monitor  */
#define BOARD_MIXER_ID_MIXER_ALTPCM	10
#define BOARD_MIXER_ID_MIXER_RECLEV	11	/* Recording level */
#define BOARD_MIXER_ID_MIXER_IGAIN	12	/* Input gain */
#define BOARD_MIXER_ID_MIXER_OGAIN	13	/* Output gain */
#define BOARD_MIXER_ID_MIXER_LINE1	14	/* Input source 1  (aux1) */
#define BOARD_MIXER_ID_MIXER_LINE2	15	/* Input source 2  (aux2) */
#define BOARD_MIXER_ID_MIXER_LINE3	16	/* Input source 3  (line) */

extern const char sMxVol[],sMxBass[],sMxTrebl[],sMxSynth[],sMxWave[],sMxSpkr[],
 sMxLine[],sMxMic[],sMxCD[],sMxMix[],sMxWave2[],sMxRec[],sMxIGain[],sMxOGain[],
 sMxLine1[],sMxLine2[],sMxLine3[],lMxVol[],lMxBass[],lMxTrebl[],lMxSynth[],lMxWave[],
 lMxSpkr[],lMxLine[],lMxMic[],lMxCD[],lMxMix[],lMxWave2[],lMxRec[],lMxIGain[],
 lMxOGain[],lMxLine1[],lMxLine2[],lMxLine3[];

typedef struct BOARD_MIXER
{
 unsigned id;
 const char *name;
 const char *s_name;
 unsigned flags;
 int right;           // 0-100
 int left;            // 0-100
} BOARD_MIXER;

#define MaxMixerVol 100

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SECompf_djgpp
int  SBMixerInit();
void SBMixerDeInit();
int  SBSetMixerValue(int id, int left, int right);
int  SBReadMixerValue(int id, int *left, int *right);
const BOARD_MIXER *SBGetElements(int *cant);
void SBGetName(char *buffer, int size);
#endif

#ifdef SEOSf_Linux
int  LinuxMixerInit();
void LinuxMixerDeInit();
int  LinuxSetMixerValue(int id, int left, int right);
int  LinuxReadMixerValue(int id, int *left, int *right);
const BOARD_MIXER *LinuxGetElements(int *cant);
void LinuxMixerGetName(char *buffer, int size);
#endif

int  MixerInit();
void MixerDeInit();
int  SetMixerValue(int id, int left, int right);
int  ReadMixerValue(int id, int *left, int *right);
const BOARD_MIXER *GetElements(int *cant);
const char *GetMixerName();

#ifdef __cplusplus
}

class fpstream;

extern void BoardMixerDialog();
extern void BoardMixerInit();
extern void BoardMixerDisable();
extern void BoardMixerSave(fpstream& s);
extern void BoardMixerLoad(fpstream& s);

#endif

#endif // MIXER_BOARD_INCLUDED
