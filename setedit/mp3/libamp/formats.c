/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
 
/* formats.c  put functions for different audio formats in here. currently
 *	      only .wav, .au would be fine  
 * Created by: tomislav uzelac  May 1996
 */
#include "audio.h"

#define FORMATS
#include "formats.h"

/* leave room for .wav header
*/
void wav_begin(void)
{
	fwrite("",1,44,out_file);
}

/* this is not proper really, but works!
*/
void wav_end(struct AUDIO_HEADER *header)
{
unsigned char ispred[20]={0x52 ,0x49 ,0x46 ,0x46 ,0xfc ,0x59  ,0x4  ,0x0 ,0x57
 ,0x41 ,0x56 ,0x45 ,0x66 ,0x6d ,0x74 ,0x20 ,0x10  ,0x0  ,0x0  ,0x0 };
unsigned char iza[8]={0x64 ,0x61 ,0x74 ,0x61};
int len,fs,i;

	len=ftell(out_file)-44;
	fs=t_sampling_frequency[header->ID][header->sampling_frequency];
	rewind(out_file);
	fwrite(ispred,1,20,out_file);

	/* 'microsoft' PCM */
	fputc(1,out_file);
	fputc(0,out_file);
	
	/* nch */
	fputc(nch,out_file);
	fputc(0,out_file);
	
	/* samples_per_second */
	for (i=0;i<32;i+=8) fputc((fs>>i)&0xff,out_file);
	
	/* average block size */
	fs *= 2*nch;
	for (i=0;i<32;i+=8) fputc((fs>>i)&0xff,out_file);
	
	/* block align */
        fs = 2*nch;
        for (i=0;i<16;i+=8) fputc((fs>>i)&0xff,out_file);
	         
	/* bits per sample */
        fputc(16,out_file);
        fputc(0,out_file);
        
        /* i jope anomalija! */
        fwrite(iza,1,4,out_file);

	/* length */
	for (i=0;i<32;i+=8) fputc((len>>i)&0xff,out_file);
	
/*	exit(0); F*k U man! why?!!!*/
}
