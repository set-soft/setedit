/**[txh]********************************************************************

  Description:
  Mixer rutines for Linux OSS.
  Copyright (c) 2000 by Salvador E. Tropea.
  This code is under the GPL license.

***************************************************************************/

#include <configed.h>

#if defined(SEOSf_Linux) && defined(HAVE_MIXER)

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include "mixer.h"

#define COMPOSE_VOLUME(left,right) ((right<<8)|left)
#define DECOMPOSE_VOLUME(vol,dev) dev.right=(vol>>8)&0xFF; dev.left=vol&0xFF

static int fdMixer=-1;
static BOARD_MIXER elements[SOUND_MIXER_NRDEVICES];
static int lastElement;
static const char *longNames[SOUND_MIXER_NRDEVICES]=
{
 lMxVol,lMxBass,lMxTrebl,lMxSynth,lMxWave,lMxSpkr,lMxLine,
 lMxMic,lMxCD,lMxMix,lMxWave2,lMxRec,lMxIGain,lMxOGain,
 lMxLine1,lMxLine2,lMxLine3
};
static const char *shortNames[SOUND_MIXER_NRDEVICES]=
{
 sMxVol,sMxBass,sMxTrebl,sMxSynth,sMxWave,sMxSpkr,sMxLine,
 sMxMic,sMxCD,sMxMix,sMxWave2,sMxRec,sMxIGain,sMxOGain,
 sMxLine1,sMxLine2,sMxLine3
};

int LinuxMixerInit()
{
 // Already initialized
 if (fdMixer!=-1) return 1;
 fdMixer=open("/dev/mixer",O_RDWR);
 if (fdMixer!=-1)
   {// Initialize the array
    unsigned devices,stereo,mask,val;
    int i;
    if (ioctl(fdMixer,SOUND_MIXER_READ_DEVMASK,&devices)==-1 ||
        ioctl(fdMixer,SOUND_MIXER_READ_STEREODEVS,&stereo)==-1) return 0;
    lastElement=0;
    memset(elements,0,sizeof(elements));
    for (i=0,mask=1; i<SOUND_MIXER_NRDEVICES; mask<<=1,i++)
       {
        if (devices & mask)
          {
           elements[lastElement].id=i;
           elements[lastElement].name=longNames[i];
           elements[lastElement].s_name=shortNames[i];
           elements[lastElement].flags=stereo & mask ?
             BOARD_MIXER_ELEM_STEREO : BOARD_MIXER_ELEM_MONO;
           ioctl(fdMixer,MIXER_READ(i),&val);
           DECOMPOSE_VOLUME(val,elements[lastElement]);
           lastElement++;
          }
       }
   }
 return fdMixer!=-1;
}

void LinuxMixerDeInit()
{
 if (fdMixer==-1) return;
 close(fdMixer);
 fdMixer=-1;
}

const BOARD_MIXER *LinuxGetElements(int *cant)
{
 if (fdMixer==-1)
   {
    *cant=0;
    return NULL;
   }
 *cant=lastElement;
 return elements;
}

static
int FindElement(int id)
{
 int i;
 for (i=0; i<lastElement; i++)
     if (elements[i].id==id) break;
 return i<lastElement ? i : -1;
}

static
int LinuxUpdateMixerValue(int id, unsigned vol)
{
 int elem;

 elem=FindElement(id);
 if (elem<0) return 0;
 //if (ioctl(fdMixer,MIXER_READ(id),&vol)==-1) return 0;
 DECOMPOSE_VOLUME(vol,elements[elem]);
 return 1;
}

int LinuxSetMixerValue(int id, int left, int right)
{// ID is mapped one to one
 unsigned vol;

 // Avoid reading outside the bounds
 if (fdMixer==-1 || id<0 || id>=SOUND_MIXER_NRDEVICES) return 0;
 vol=COMPOSE_VOLUME(left,right);
 if (ioctl(fdMixer,MIXER_WRITE(id),&vol)==-1) return 0;
 // The ioctl updates the value!
 LinuxUpdateMixerValue(id,vol);
 return 1;
}

int LinuxReadMixerValue(int id, int *left, int *right)
{
 int elem;

 if (fdMixer==-1) return 0;
 elem=FindElement(id);
 if (elem<0) return 0;
 *left=elements[elem].left;
 *right=elements[elem].right;
 return 1;
}

void LinuxMixerGetName(char *buffer, int size)
{
 struct mixer_info mi;
 int available;

 *buffer=0;
 if (fdMixer==-1) return;
 strncpy(buffer,"Open Sound System",size);
 buffer[size-1]=0;
 available=size-1-strlen(buffer)-3;
 if (available>0 && ioctl(fdMixer,SOUND_MIXER_INFO,&mi)!=-1)
   {
    strcat(buffer," (");
    strncat(buffer,mi.name,available);
    strcat(buffer,")");
   }
}

//#define TEST
#ifdef TEST
#include <stdio.h>

int main(int argc, char *argv[])
{
 int left,right,cant,i;
 const BOARD_MIXER *e;

 if (!LinuxMixerInit())
   {
    printf("Can't initialize mixer\n");
    return 1;
   }

 if (!(e=LinuxGetElements(&cant)))
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

 if (!LinuxReadMixerValue(BOARD_MIXER_ID_MIXER_VOLUME,&left,&right))
   {
    printf("Can't read master volume\n");
    return 2;
   }
 printf("Master: %d,%d\n",left,right);

 if (!LinuxSetMixerValue(BOARD_MIXER_ID_MIXER_VOLUME,90,90))
   {
    printf("Can't set master volume\n");
    return 3;
   }

 LinuxMixerDeInit();
 return 0;
}
#endif

#endif // SEOSf_Linux && HAVE_MIXER
