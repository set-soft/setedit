/*
	Amp Library interface
	 (audio.c + guicontrol.c)
	Adapted by Ove Kaaven <ovek@arcticnet.no>
 */


#include "amp.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifndef __DJGPP__
#ifndef __BEOS__
#include <sys/uio.h>
#endif
#endif

#ifndef __DJGPP__
#include <sys/socket.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define AUDIO
#include "audio.h"
#include "formats.h"
#include "getbits.h"
#include "huffman.h"
#include "layer2.h"
#include "layer3.h"
#include "position.h"
#include "rtbuf.h"
#include "transform.h"
#include "controldata.h"
#include "guicontrol.h"

#include "libamp.h"

#ifndef __BEOS__
typedef int bool;
#endif

void calculate_t43(void);

int install_amp(void)
{
 static bool is_installed=FALSE;

 if (!is_installed) {
/* initialise_decoder */
  premultiply();
  imdct_init();
  calculate_t43();

/* args */
  AUDIO_BUFFER_SIZE=300*1024;
  A_DUMP_BINARY=FALSE;
  A_QUIET=TRUE;
  A_FORMAT_WAVE=FALSE;
  A_SHOW_CNT=FALSE;
  A_SET_VOLUME=-1;
  A_SHOW_TIME=0;
  A_AUDIO_PLAY=TRUE;
  A_WRITE_TO_FILE=FALSE;
  A_MSG_STDOUT=FALSE;
  A_DOWNMIX=FALSE;

  is_installed=TRUE;
 }

 return(is_installed);
}

int amp_bitrate,amp_samprat;
int amp_mpg_ver,amp_layer,amp_stereo,amp_pollsize;
int amp_playing,amp_draining,amp_loaded=FALSE,amp_reverse_phase=FALSE;
int amp_looping;
int amp_dec_frame,amp_dec_time;
int amp_frame,amp_time;
struct AUDIO_HEADER amp_cur_header;

void get_amp_info(struct AUDIO_HEADER*header)
{
 /* show_header from dump.c */
 amp_bitrate=t_bitrate[header->ID][3-header->layer][header->bitrate_index];
 amp_samprat=t_sampling_frequency[header->ID][header->sampling_frequency];

 amp_layer=4-header->layer;
 if (header->ID==1) amp_mpg_ver=1; else amp_mpg_ver=2;

 if (header->mode==3) amp_stereo=FALSE; else amp_stereo=TRUE;

 amp_pollsize=2*576;
 if (amp_layer==3)
  if (!header->ID) amp_pollsize=576;
}

int read_amp_frame(struct AUDIO_HEADER*header)
{
 if (header->layer==1) {
  if (layer3_frame(header,amp_dec_frame)) {
   unload_amp();
   warn(" error. blip.\n");
   return(0);
  }
 } else if (header->layer==2)
  if (layer2_frame(header,amp_dec_frame)) {
   unload_amp();
   warn(" error. blip.\n");
   return(0);
  }
 return(1);
}


int load_amp(char*filename,int loop)
{
 int g;

 if (amp_loaded)
 if (!unload_amp()) return(0);

 if (!install_amp()) return(0);

 if ((in_file=fopen(filename,"rb"))==NULL) {
  warn("Could not open file: %s\n",filename);
  return(0);
 }

/* initialise_globals */
  append=data=nch=0;
  f_bdirty=TRUE;
  bclean_bytes=0;
  memset(s,0,sizeof s);
  memset(res,0,sizeof res);

/* load MPEG header */
 if ((g=gethdr(&amp_cur_header))!=0) {
  report_header_error(g);
  fclose(in_file);
  return(0);
 }
 if (amp_cur_header.protection_bit==0) getcrc();

 get_amp_info(&amp_cur_header);

/* setup_audio */
  if (AUDIO_BUFFER_SIZE==0)
   audioOpen(amp_samprat,(amp_stereo && !A_DOWNMIX),A_SET_VOLUME);
  else
   audioBufferOpen(amp_samprat,(amp_stereo && !A_DOWNMIX),A_SET_VOLUME);

 amp_looping=loop;
 amp_dec_time=amp_dec_frame=0;
 amp_time=amp_frame=0;

 return(amp_playing=amp_loaded=TRUE);
}

extern int audioUsedSpace(void);
extern int audioSpaceRemaining(void);
extern int audioReady(void);
extern int audioStop(void);
extern int audioStart(void);
extern int audioBufferPad(void);
extern unsigned audioBufferGetFrame(void);
extern int audioIsPlaying(void);

static void update_track_pos(void)
{
 int new_frame=audioBufferGetFrame();

 if (new_frame>=0) {
  amp_frame=new_frame;
 }
 amp_time=amp_frame*1152/amp_samprat;
}

static int init_stop(void)
{
 amp_playing=FALSE;
 /* check for any data left to play in buffer */
 if (audioIsPlaying()) {
  amp_draining=TRUE;
  return(0);
 }
 /* nothing left to play, stop */
 audioStop();
 return(-1);
}

static int fetch_header(struct AUDIO_HEADER*header)
{
 int g;

 if ((g=gethdr(header))!=0) {
  report_header_error(g);
  if (g==GETHDR_EOF && A_FORMAT_WAVE) wav_end(header);
  A_WRITE_TO_FILE=FALSE; A_FORMAT_WAVE=FALSE;
  return(-1);
 }

 if (header->protection_bit==0) getcrc();

 return(0);
}

static int init_rewind(struct AUDIO_HEADER*header)
{
 fseek(in_file,0,SEEK_SET);
 amp_dec_time=amp_dec_frame=0;

 return(fetch_header(header));
}

static int exec_replay(void)
{
 if (!(amp_playing||amp_draining)) {
  amp_frame=amp_dec_frame;
  audioStart();
  update_track_pos();
 }

 amp_draining=FALSE;

 return(amp_playing=TRUE);
}

static int init_replay(void)
{
 if (init_rewind(&amp_cur_header)<0) {
  return(init_stop());
 }

 return(exec_replay());
}

static int end_of_track(void)
{
 if (amp_looping) {
  return(init_replay());
 } else {
  return(init_stop());
 }
}

int poll_amp(void)
{
 int poll_size;

 if (!amp_loaded)
    return(-1);
 if (!amp_playing)
   {
    if (amp_draining)
      {
       if (audioUsedSpace())
         {
          audioBufferPad();
          /* try to move some data from buffer to sound card */
          audioBufferWrite(NULL,0);
         }
       update_track_pos();
       /* check for any data left to play in buffer */
       if (audioIsPlaying())
          return(0);
       /* nothing left to play, stop */
       amp_draining=FALSE; audioStop();
       return(-1);
      }
    else
       return(-1);
   }

 poll_size=2*576;
 if (amp_cur_header.layer==1)
  if (!amp_cur_header.ID) poll_size=576;

/* make sure we have enough buffer space left, to avoid blocking */
 if (audioSpaceRemaining()<poll_size) {
  /* try to move some data from buffer to sound card */
  audioBufferWrite(NULL,0);
  update_track_pos();
  /* failing that, return */
  if (audioSpaceRemaining()<poll_size) return(0);
 }

 amp_dec_frame++;
 amp_dec_time=amp_dec_frame*1152/amp_samprat;

 if (!read_amp_frame(&amp_cur_header)) {
  update_track_pos();
  return(end_of_track());
 }

 update_track_pos();

 /* get next header */
 if (fetch_header(&amp_cur_header)<0) {
  return(end_of_track());
 }

 return(1);
}

int run_amp(void)
{
 int ret=1;

 if (!amp_loaded) return(-1);
 if (!(amp_playing||amp_draining)) return(-1);
 while ((ret=poll_amp())>0);
 return(ret);
}

int replay_amp(void)
{
 if (!amp_loaded) return(-1);
 if (amp_playing) {
  audioStop(); amp_playing=amp_draining=FALSE;
 }
 return(init_replay());
}

int stop_amp(void)
{
 if (!amp_loaded)
    return(-1);
 if (amp_playing)
   {
    audioStop();
    amp_playing=amp_draining=FALSE;
   }
 return 0;
}

int seek_amp_abs(int frame)
{
 if (!amp_loaded) return(-1);
 if (amp_playing) {
  audioStop(); amp_playing=amp_draining=FALSE;
 }

 if (frame>amp_dec_frame) {
  amp_dec_frame+=ffwd(&amp_cur_header,frame-amp_dec_frame);
  exec_replay();
  return(0);
 } else
 if (frame<amp_dec_frame) {
  if (frame<(amp_dec_frame/2)) {
   if (init_rewind(&amp_cur_header)<0) return(-1);
   amp_dec_frame+=ffwd(&amp_cur_header,frame);
  } else {
   amp_dec_frame-=rew(&amp_cur_header,amp_dec_frame-frame);
  }
  exec_replay();
  return(0);
 } else {
  exec_replay();
  return(0);
 }
}

int seek_amp_rel(int framecnt)
{
 int frame;

 frame=amp_frame+framecnt;
 if (frame<0) frame=0;
 return(seek_amp_abs(frame));
}

int unload_amp(void)
{
 if (!amp_loaded) return(0);
 amp_loaded=amp_playing=amp_draining=FALSE;
 if (AUDIO_BUFFER_SIZE!=0)
  audioBufferClose();
 else
  audioClose();
 fclose(in_file);
 return(1);
}

void report_header_error(int err)
{
	switch (err) {
		case GETHDR_ERR: die("error reading mpeg bitstream. exiting.\n");
					break;
		case GETHDR_NS : warn("this is a file in MPEG 2.5 format, which is not defined\n");
				 warn("by ISO/MPEG. It is \"a special Fraunhofer format\".\n");
				 warn("amp does not support this format. sorry.\n");
					break;
		case GETHDR_FL1: warn("ISO/MPEG layer 1 is not supported by amp (yet).\n");
					break;
		case GETHDR_FF : warn("free format bitstreams are not supported. sorry.\n");
					break;  
		case GETHDR_SYN: warn("oops, we're out of sync.\n");
					break;
		case GETHDR_EOF: 
		default:                ; /* some stupid compilers need the semicolon */
	}       
}

