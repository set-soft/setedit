/*****************************************************************************/

/*
 *      rtbuf.c  --  Linux realtime audio output.
 *
 *      Copyright (C) 1996  Thomas Sailer (sailer@ife.ee.ethz.ch)
 *        Swiss Federal Institute of Technology (ETH), Electronics Lab
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *  This is the Linux realtime sound output driver
 */

/*****************************************************************************/

#include "amp.h"
#include "transform.h"
#include "audio.h"
#include "getbits.h"
#include "layer3.h"

#define RTBUF
#include "rtbuf.h"

#ifdef LINUX_REALTIME
 
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/soundcard.h>
#include <sys/time.h>
#include <sched.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/user.h>
      

static int audio_fd;
static struct audio_buf_info abinfo;
static caddr_t abuf;
static unsigned int abuf_wptr;
static unsigned int abuf_sz;

static struct count_info cinfo;

#define IBUFSZ 65520

typedef volatile int atomic_t;

static struct ibuf {
	atomic_t rdptr;
	atomic_t wrptr;
	unsigned char buf[IBUFSZ];
} *ibuf;

#define atomic_set(x,y) x = y
#define atomic_read(x) (x)

static pid_t bufproc;


/* --------------------------------------------------------------------- */

int prefetch_get_input(unsigned char *bp, int bytes)
{
	unsigned int rd, wr, rem, u;
	int stat;
	pid_t ret;

	rd = atomic_read(ibuf->rdptr);
	wr = atomic_read(ibuf->wrptr);
	rem = (IBUFSZ+wr-rd) % IBUFSZ;
	if (bytes > rem) {
		/* TODO: figure out the best way to find out whether it was an
		 * error or an eof. bufproc exit ststus?
		 */
		ret = wait4(bufproc, &stat, WNOHANG, NULL);
		/*
		if (ret != (pid_t)-1 && ret == bufproc) {
			bufproc = (pid_t) -1;
#if 0
			printf("prefetch_get_input(): Terminating");
#endif
		} else 
			fprintf(stderr, "get_input: Underrun\n");
		snd_eof = 1; 
		*/
		if (ret!=0) 
			bufproc = (pid_t) -1;
		else 
			fprintf(stderr, "get_input: Underrun\n");

		memset(bp, 0, bytes);
		return -1;
	}
	u = IBUFSZ-rd;
	if (bytes >= u) {
		memcpy(bp, ibuf->buf+rd, u);
		bp += u;
		rd = 0;
		bytes -= u;
	}
	if (bytes > 0) {
		memcpy(bp, ibuf->buf+rd, bytes);
		rd += bytes;
	}
	atomic_set(ibuf->rdptr, rd);
	return 0;
}

/* --------------------------------------------------------------------- */

static void child(char *file)
{
	int fd, i;
	unsigned int rd, wr, free;

	/*
	 * release superuser priority
	 */
	setreuid(getuid(), getuid());	
	/*
	 * open file
	 */
	if ((fd = open(file, O_RDONLY, 0)) < 0) {
		fprintf(stderr, "Cannot open file \"%s\"\n", file);
		return;
	}
	for (;;) {
		rd = atomic_read(ibuf->rdptr);
		wr = atomic_read(ibuf->wrptr);
		free = (IBUFSZ + rd - wr - 1) % IBUFSZ;
		if (free <= 0) {
			usleep(100000L);
		} else {
			if (free > 4096)
				free = 4096;
			i = read(fd, ibuf->buf+wr, 
				 (wr + free > IBUFSZ) ? IBUFSZ-wr : free);
			if (i == -1) {
				perror("read");
				close(fd);
				return;
			}
			if (i == 0) 
				break;
			wr += i;
			if (wr >= IBUFSZ)
				wr = 0;
			atomic_set(ibuf->wrptr, wr);
#if 0
			fprintf(stderr, "buffer fill: wr: %d rd: %d\n", wr, rd);
			fflush(stderr);
#endif
		}
	}
	close(fd);
}


/* --------------------------------------------------------------------- */

int rt_play(char *file)
{
	pid_t playproc;
	int i;

	/*
	 * this fork enables us to run both the buffering
	 * process and the decoding/play process to run at
	 * user privileges while retaining superuser privileges
	 * for subsequent files. This should minimize security
	 * problems
	 */
	switch (playproc = fork()) {
	case -1:
		perror("fork");
		return -1;

	case 0:
		break;

	default:
#if 0
		fprintf(stderr, "Play task PID: %d\n", playproc);
		fflush(stderr);
#endif
		wait4(playproc, &i, 0, NULL);
		return i;
	}
	/*
	 * allocate the communication buffer between the
	 * prefetch and the decode processes
	 */
#if 0
	ibuf = (struct ibuf *)mmap(NULL, sizeof(struct ibuf), PROT_READ|PROT_WRITE,
	                           MAP_SHARED|MAP_ANON, -1, 0);
	if (ibuf == (struct ibuf *)-1) {
		perror("mmap: shared anonymous memory");
		exit(-1);
	}
#else
        if ((i = shmget(IPC_PRIVATE, /*SHMLBA*/sizeof(struct ibuf), 
			IPC_CREAT | IPC_EXCL | 0600)) < 0) {
		perror("shmget");
		exit(-1);
	}
        if ((ibuf = (struct ibuf *)shmat(i, NULL, 0)) == (struct ibuf *)-1) {
		perror("shmat");
		exit(-1);
	}
        if (shmctl(i, IPC_RMID, NULL)) {
		perror("shmctl");
		exit(-1);
	}
#endif
	atomic_set(ibuf->rdptr, 0);
	atomic_set(ibuf->wrptr, 0);
	/*
	 * lock memory down
	 */
	if (mlockall(MCL_CURRENT) == -1) 
		perror("mlockall");
	/*
	 * fork the prefetch process
	 */
	switch (bufproc = fork()) {
	case 0: 
		child(file);
		exit(0);
	
	case -1:
		perror("fork");
		if (munlockall() == -1)
			perror("munlockall");
		exit(-1);
		
	default:
#if 0
		fprintf(stderr, "Buffering task PID: %d\n", bufproc);
		fflush(stderr);
#endif
		audio_fd = -1;
		//decoder_process();
		decodeMPEG();
		if (audio_fd != -1)
			close(audio_fd);
		if (bufproc != (pid_t)-1) {
			/* TODO: think of something nicer here. This is really a quick
			 * workaround. The reason is that if gethdr() returns GETHDR_NS
			 * we just don't know if bufproc exited.
			 */
			if (kill(bufproc, SIGTERM) == -1)
				/*perror("kill")*/;
			if (wait4(bufproc, NULL, 0, NULL) == (pid_t)-1)
				/*perror("wait4")*/;
		}
		break;
	}
	if (munlockall() == -1)
		/*perror("munlockall")*/;
	exit(0);
}

/* --------------------------------------------------------------------- */

void rt_printout(short *sbuf, int ln)
{
	int rem = abuf_sz - abuf_wptr;
	
	if (ln >= rem) {
		memcpy((void *)(abuf + abuf_wptr), sbuf, rem);
		(unsigned char *)sbuf += rem;
		abuf_wptr = 0;
		ln -= rem;
	}
	if (ln > 0) {
		memcpy((void *)(abuf + abuf_wptr), sbuf, ln);
		abuf_wptr += ln;
	}
}
	
/* --------------------------------------------------------------------- */

int setup_fancy_audio(struct AUDIO_HEADER *mpegheader)
{
int apar;

	/*
	 * open the audio device
	 * Don't use O_WRONLY, as the following mmap will fail with permission error!
	 */
	if ((audio_fd = open("/dev/dsp", O_RDWR, 0)) < 0) {
		perror("open /dev/dsp");
		return -1;
	}

	/*
	 * configure audio
	 */
	apar = AFMT_S16_LE;
	if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &apar) == -1) {
		perror("ioctl: SNDCTL_DSP_SETFMT");
		return -1;
	}
	if (apar != AFMT_S16_LE) {
		fprintf(stderr, "16 bit format not supported\n");
		return -1;
	}
	apar = ((mpegheader->mode != 3) && !A_DOWNMIX);
	if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &apar) == -1) {
		perror("ioctl: SNDCTL_DSP_STEREO");
		return -1;
	}
	if (apar != ((mpegheader->mode != 3)&& !A_DOWNMIX)) {	
		fprintf(stderr, "%s not supported\n", 
			(mpegheader->mode != 3) ? "stereo" : "mono");
		return -1;
	}
	apar = t_sampling_frequency[mpegheader->ID][mpegheader->sampling_frequency];
	if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &apar) == -1) {
		perror("ioctl: SNDCTL_DSP_SPEED");
		return -1;
	}
	if (apar != t_sampling_frequency[mpegheader->ID][mpegheader->sampling_frequency]) {
		fprintf(stderr, "Warning: requested sampling rate %d does not "
			"match effective %d\n", 
			t_sampling_frequency[mpegheader->ID][mpegheader->sampling_frequency],
			apar);
	}

	/* volume A_SET_VOLUME */
	/*
	 * determine if the driver is capable to support our access method
	 */
	if (ioctl(audio_fd, SNDCTL_DSP_GETCAPS, &apar) == -1) {
		perror("ioctl: SNDCTL_DSP_GETCAPS");
		return -1;
	}
	if (!(apar & DSP_CAP_TRIGGER) || !(apar & DSP_CAP_MMAP)) {
		fprintf(stderr, "Sound driver does not support mmap and/or trigger\n");
		return -1;
	}

	/*
	 * set fragment sizes
	 */
	apar = 0xffff000d;
	if (ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &apar) == -1) {
		perror("ioctl: SNDCTL_DSP_SETFRAGMENT");
		return -1;
	}
	if (ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &abinfo) == -1) {
		perror("ioctl: SNDCTL_DSP_GETOSPACE");
		return -1;
	}
	abuf_sz = abinfo.fragstotal * abinfo.fragsize; 
#if 0
	fprintf(stderr, "OSS: # fragments: %d  fragment size: %d  total buffer size: %d\n",
		abinfo.fragstotal, abinfo.fragsize, abuf_sz);
#endif

	/*
	 * mmap buffer
	 * BSD people attention: you may need to uncomment the PROT_READ
	 * feedback welcome: sailer@ife.ee.ethz.ch
	 */
	if ((abuf = mmap(NULL, abuf_sz, PROT_WRITE /* | PROT_READ*/, MAP_FILE | MAP_SHARED, 
	                 audio_fd, 0)) == (caddr_t)-1) {
		perror("mmap");
		return -1;
	}

	memset(abuf, 0, abinfo.fragstotal * abinfo.fragsize);
	abuf_wptr = 0;
	return 0;
}

int start_fancy_audio(struct AUDIO_HEADER *mpegheader)
{
int apar;
	/*
	 * start audio device
	 */
	/* cinfo.ptr = 0;  is this one needed? */
	apar = 0;
	if (ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &apar) == -1) {
		perror("ioctl: SNDCTL_DSP_SETTRIGGER");
		return -1;
	}
	apar = PCM_ENABLE_OUTPUT;
	if (ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &apar) == -1) {
		perror("ioctl: SNDCTL_DSP_SETTRIGGER");
		return -1;
	}
	return 0;
}

int stop_fancy_audio(void)
{
int apar;
	apar = 0;
	if (ioctl(audio_fd, SNDCTL_DSP_RESET, &apar) == -1) {
		perror("ioctl: SNDCTL_DSP_RESET");
		return -1;
	}

	return 0;
}

int block_fancy_audio(int snd_eof)
{
int i;
struct fd_set wmask;
struct timeval tm;

	do {
		FD_ZERO(&wmask);
		FD_SET(audio_fd, &wmask);
		tm.tv_sec = 10;
		tm.tv_usec = 0;
		i = select(audio_fd+1, NULL, &wmask, NULL, &tm);
		if (i < 0) {
			perror("select");
			return -1;
		}
		if (i == 0) {
			fprintf(stderr, "Sound output: timeout\n");
			return -1;
		}
		if (ioctl(audio_fd, SNDCTL_DSP_GETOPTR, &cinfo) == -1) {
			perror("ioctl: SNDCTL_DSP_GETOPTR");
			return -1;
		}
	} while (snd_eof && cinfo.ptr/abinfo.fragsize != abuf_wptr/abinfo.fragsize);

	return 0;
}

int ready_fancy_audio(void)
{
int i = (abuf_sz + cinfo.ptr - abuf_wptr - 1) % abuf_sz;

	if (i >= 2*sizeof sample_buffer)
		return 1;
	else 
		return 0;
}

void cleanup_fancy_audio(void)
{
	memset(abuf+abuf_wptr,0,abinfo.fragsize-(abuf_wptr%abinfo.fragsize));
}

int set_realtime_priority(void)
{
struct sched_param schp;
	/*
	 * set the process to realtime privs
	 */
        memset(&schp, 0, sizeof(schp));
	schp.sched_priority = sched_get_priority_min(SCHED_RR);

	if (sched_setscheduler(0, SCHED_RR, &schp) != 0) {
		perror("sched_setscheduler");
		return -1;
	}

	return 0;

#if 0
/*
        Try to set the priority of this process to a value which
        allows us to play without buffering, thus saving memory
        and avoiding cache-misses.
        If we cannot get any priority high enough to allow for
        undisturbed replay (because we don't have sufficient
        priviledges), return a zero, otherwise, return a one.
*/
        
        /* try to lock process in physical memory, just ignore if this
	 * fails 
	 */
        plock(PROCSHLIBLOCK);
        
        /* try to set a realtime-priority of 64 */
        if (-1 != rtprio(0, 64)) {
                DB(audio, msg("using real-time priority\n"); )
                return 0; 
        }
        
        /* try to set a nice-level of -20 */
        if (-1 != nice(-20)) {
                DB(audio, msg("using nice-level -20\n"); )
                return 0; 
        }
        
        DB(audio, msg("using buffered output\n"); )

	return -1;
#endif
}

void prefetch_initial_fill(void)
{
int i;
	while (atomic_read(ibuf->wrptr) < IBUFSZ/2) {
		if (wait4(bufproc, &i, WNOHANG, NULL) == bufproc) {
			break;
		}
		usleep(100000L);
	}
}

#endif /* LINUX_REALTIME */
