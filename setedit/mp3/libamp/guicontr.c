/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
/* guicontrol.c
 *
 * Edouard Lafargue, 12 May 1997
 */ 
#ifndef __DJGPP__
#ifndef OS_SunOS
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

#ifndef __BEOS__
typedef int bool;
#endif


typedef struct __nextsong
{
      bool avail;
      int fd;
      int type;
} TNextSong, *PNextSong;

TNextSong nextSong;

void send_msg(PControlMsg msg, bool safe)
{
  if(send_fd != -1)
    {
      if(safe)
	while(write(send_fd, msg, sizeof(TControlMsg)) < 0 &&
	      errno == EAGAIN)
	  ;
      else
	write(send_fd, msg, sizeof(TControlMsg));
    }
}
  
int get_msg(PControlMsg msg)
{
  return read(receive_fd, msg, sizeof(TControlMsg));
}



void seek_rewind(int pos)
{
     fprintf(stderr,"Seek rewind\n");
}


/* This routine sends the current frame,
 * and where we are in the file (in percent)
 */

void GUIstatusDisplay(int frameno)
{
  TControlMsg message;

  message.type = MSG_FRAMES;
  message.data = frameno;
  send_msg(&message,TRUE);

  message.type = MSG_POSITION;
  message.data = frameno*419;
  send_msg(&message,TRUE);
  
}

void next_song(int fd, int type)
{
  
   nextSong.avail = TRUE;
   nextSong.fd = fd;
   nextSong.type = type;
   
}


/* parse_msg returns the current "cnt" (it may have been modified
 * if we did "ffwd" or "rew")
 * Large chunks of this routine were taken from "sajberplay",
 * the mpeg part of the original sajber jukebox.
 * The Sajber Jukebox was written by
 * Joel Lindholm <wizball@kewl.campus.luth.se>
 */

int parse_msg(PControlMsg msg, struct AUDIO_HEADER *header, int cnt)
{
  switch(msg->type)   {
  case MSG_BUFFER:
/*     fprintf(stderr, "MSG_BUFFER %d\n", msg->data);     */
    break;
  case MSG_BUFAHEAD:
/*     fprintf(stderr, "MSG_PLAYAHEAD %d\n", msg->data); */
    break;
  case MSG_SEEK:
    break;
  case MSG_RELEASE:
    break;
  case MSG_PRIORITY:
    break;
  case MSG_QUERY:
/*     fprintf(stderr,"MSG_QUERY\n"); */
    {
      TControlMsg rmsg;
      rmsg.type = MSG_RESPONSE;
      switch(msg->data)
	{
	case QUERY_PLAYING:
	  rmsg.data = GUI_PLAYING; /* No need to synch. */
/*        fprintf(stderr, "Reply to query playing\n"); */
	  send_msg(&rmsg, TRUE);
	  break;
	case QUERY_PAUSED:
/*        fprintf(stderr, "Reply to QUERY_PAUSED\n"); */
	  rmsg.data = GUI_PAUSE;
	  send_msg(&rmsg, TRUE);
	  break;
	}
    }
  break;
  case MSG_CTRL:
    switch(msg->data)
      {
      case PLAY_PAUSE:
/*      fprintf(stderr, "MSG_CTRL - PLAY_PAUSE\n"); */
	if(GUI_PAUSE)
	  GUI_PAUSE= FALSE;
	else {
	  GUI_PAUSE=TRUE ;
	  /*
	  if (GUI_PLAYING) audioBufferFlush();
	  */
	}
	break;
      case PLAY_STOP:
/*      fprintf(stderr, "MSG_CTRL - PLAY_STOP\n"); */
	GUI_STOP = TRUE;
	if (GUI_PLAYING) audioBufferFlush();
	break;
      case FORWARD_BEGIN:
/*      fprintf(stderr, "FORWARD_BEGIN\n"); */
	/*      forward(8); */
	{
	  TControlMsg rmsg;
	  
	  rmsg.type = MSG_RESPONSE;
	  rmsg.data = FORWARD_BEGIN;
	  
	  send_msg(&rmsg, TRUE);
	}
	break;
      case FORWARD_STEP:
	{
/*        fprintf(stderr, "FORWARD_STEP\n"); */
	  if(ffwd(header, 10)==10) cnt+=10;
	  {
	    TControlMsg rmsg;
	    
	    rmsg.type = MSG_RESPONSE;
	    rmsg.data = FORWARD_STEP;
	    
	    send_msg(&rmsg, TRUE);
	  }

	  if (GUI_PLAYING) audioBufferFlush();
	}
	break;
      case FORWARD_END:
/*      fprintf(stderr, "FORWARD_END\n"); */
	break;
      case REWIND_BEGIN:
/*      fprintf(stderr, "REWIND_BEGIN\n"); */
	{
	  TControlMsg rmsg;
	  
	  rmsg.type = MSG_RESPONSE;
	  rmsg.data = REWIND_BEGIN;
	  
	  send_msg(&rmsg, TRUE);
	}
	break;
      case REWIND_STEP:
	{
	  int result;
	  /*      fprintf(stderr, "REWIND_STEP\n"); */
	  if (cnt>10){
	    if ((result = rew(header, 10))!= -1) {
	      cnt-=result;
	    }
	  }
	  {
	    TControlMsg rmsg;
	    
	    rmsg.type = MSG_RESPONSE;
	    rmsg.data = REWIND_STEP;
	    
	    send_msg(&rmsg, TRUE);
	  }

	  if (GUI_PLAYING) audioBufferFlush();
	}
	break;
      case REWIND_END:
/*      fprintf(stderr, "REWIND_END\n"); */
	break;
      }
     break;
  case MSG_QUIT:
    fprintf(stderr, "MSG_QUIT\n");
    if (GUI_PLAYING) audioBufferFlush();
    break;
  case MSG_SONG:
      {
	 int fd;
	 struct msghdr hdr;
	 struct m_cmsghdr fdhdr;
	 struct iovec iov[1];
	 char data[2];

	 iov[0].iov_base = data;
	 iov[0].iov_len = 2;

/*          fprintf(stderr, "MSG_SONG\n"); */

	 hdr.msg_iov = iov;
	 hdr.msg_iovlen = 1; 
	 hdr.msg_name = NULL;
	 hdr.msg_namelen = 0;

	 fdhdr.cmsg_len = sizeof(struct m_cmsghdr);
	 fdhdr.cmsg_level = SOL_SOCKET;
	 fdhdr.cmsg_type = SCM_RIGHTS;
	 fdhdr.fd = 0;

	 hdr.msg_control = &fdhdr;
	 hdr.msg_controllen = sizeof(struct m_cmsghdr);

	 if(recvmsg(receive_fd, &hdr, 0) < 0)
	   perror("recvmsg");
	 
	 fd = fdhdr.fd;
	 
/*          fprintf(stderr, "FILEFD READ=%d\n", fd); */
	 
	 GUI_FD_TO_PLAY = fd;
	if (GUI_PLAYING) audioBufferFlush();
      }

      break;
  }
  return cnt;
}

void gui_control(void)
{
  int flags,dummy;
  TControlMsg msg;
  struct AUDIO_HEADER head;

/*   fprintf(stderr,"Amp est contrôlé par le jukebox...\n"); */
  
  if((flags = fcntl(STDOUT_FILENO, F_GETFL, 0)) < 0)
    perror("fcntl");
  flags |= O_NONBLOCK;
  
  if(fcntl(STDOUT_FILENO, F_SETFL, flags) < 0)
    perror("fcntl");

  send_fd = STDOUT_FILENO;
  receive_fd = STDIN_FILENO;  

  GUI_FD_TO_PLAY = -1;

  while(1)
    {
      GUI_PAUSE = FALSE;
      GUI_STOP = FALSE;
      GUI_STOPPED = TRUE;
      GUI_PLAY = FALSE;
      GUI_PLAYING = FALSE;
      
      if(get_msg(&msg) <= 0)
	quit_flag = TRUE;
      else
	dummy = parse_msg(&msg, &head,0);

      if (quit_flag)
	{
/*        fprintf(stderr, "quit_flag is true\n"); */
	  exit(0);
	}
      while(GUI_FD_TO_PLAY != -1)
	{
	  GUI_PAUSE = FALSE;
	  GUI_STOP = FALSE;
	  GUI_STOPPED = TRUE;
	  GUI_PLAY = FALSE;
	  GUI_PLAYING = FALSE;

	 /* Set stdin to non-blocking */
	 if((flags = fcntl(STDIN_FILENO, F_GETFL, 0)) < 0)
	   perror("fcntl");
	 flags |= O_NONBLOCK;
	 
	 if(fcntl(STDIN_FILENO, F_SETFL, flags) < 0)
	   perror("fcntl");
	 
	 decodeMPEG_2(GUI_FD_TO_PLAY);
	 
	 if((flags = fcntl(STDIN_FILENO, F_GETFL, 0)) < 0)
	   perror("fcntl");
	 flags ^= O_NONBLOCK;
	 
	 if(fcntl(STDIN_FILENO, F_SETFL, flags) < 0)
	   perror("fcntl");
	}
    }   
}


int decodeMPEG_2(int inFilefd)
{
  struct AUDIO_HEADER header;
  int cnt,g,err=0;
  TControlMsg message;


  if ((in_file=fdopen(inFilefd,"r"))==NULL) {
    return(1);
  }

  append=data=nch=0; /* initialize globals */

  GUI_STOPPED = FALSE;
  GUI_PLAYING = TRUE;
  GUI_FD_TO_PLAY = -1;
  
  for (cnt=0;;cnt++) {
    if ((g=gethdr(&header))!=0) {
      switch (g) {
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
      default: 
      }
      break;
    }
    
    if (!(cnt%10)){
    GUIstatusDisplay(cnt);
    }

    if(get_msg(&message) >= 0)
      {
	int pflag = 0;
	cnt = parse_msg(&message,&header,cnt);
	if (GUI_PAUSE) {
		int flags;
		pflag = 1;
		/* Set stdin to blocking */
		if((flags = fcntl(STDIN_FILENO, F_GETFL, 0)) < 0)
			perror("fcntl");
		flags ^= O_NONBLOCK;
		if(fcntl(STDIN_FILENO, F_SETFL, flags) < 0)
			perror("fcntl");
	}
	while(GUI_PAUSE){
	  if(get_msg(&message) >= 0)
	    cnt = parse_msg(&message,&header,cnt);
	}
	if (pflag) {
		int flags;
		/* Set stdin to non-blocking */
		if((flags = fcntl(STDIN_FILENO, F_GETFL, 0)) < 0)
			perror("fcntl");
		flags |= O_NONBLOCK;
		if(fcntl(STDIN_FILENO, F_SETFL, flags) < 0)
			perror("fcntl");
	}
	if (GUI_STOP || (GUI_FD_TO_PLAY != -1)){
	  break;
	}
      }

    /* crc is read just to get out of the way.
     */
    if (header.protection_bit==0) getcrc();
    
    if (!cnt && A_AUDIO_PLAY) { /* setup the audio when we have the frame info */

      if (AUDIO_BUFFER_SIZE==0)
	audioOpen(t_sampling_frequency[header.ID][header.sampling_frequency],
		  (header.mode!=3),
		  A_SET_VOLUME);
      else
	audioBufferOpen(t_sampling_frequency[header.ID][header.sampling_frequency],(header.mode!=3),A_SET_VOLUME);
    }
    
    if (layer3_frame(&header,cnt)) {
      warn(" error. blip.\n");
      err=1;
      break;
    } 
    
  }
  fclose(in_file);
  if (A_AUDIO_PLAY)
    if (AUDIO_BUFFER_SIZE!=0)
      audioBufferClose();
    else
      audioClose();
  else
    fclose(out_file);
  if (!(GUI_STOP) && (GUI_FD_TO_PLAY == -1)) {
    /* We've reached the end of the track, notify the jukebox...
     */
    TControlMsg rmsg;
    
    rmsg.type = MSG_NEXT;
    rmsg.data = 0;    
    send_msg(&rmsg, TRUE);
  }

  GUI_STOPPED = TRUE;
  GUI_PLAYING = FALSE;
  return(err);
}
#endif /*OS_SunOS*/
#endif /* DJGPP */

