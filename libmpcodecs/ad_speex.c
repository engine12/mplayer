/*
 * Speex decoder by Reimar Döffinger <Reimar.Doeffinger@stud.uni-karlsruhe.de>
 *
 * This code may be be relicensed under the terms of the GNU LGPL when it
 * becomes part of the FFmpeg project (ffmpeg.org)
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "config.h"
#include <stdlib.h>
#include <speex/speex.h>
#include <speex/speex_stereo.h>
#include <speex/speex_header.h>
#include "ad_internal.h"

static const ad_info_t info = {
  "Speex audio decoder",
  "speex",
  "Reimar Döffinger",
  "",
  ""
};

LIBAD_EXTERN(speex)

typedef struct {
  SpeexBits bits;
  void *dec_context;
  int fd;
	
} context_t;


#define FRAMES_PER_RTP_PACKET 21
#define SPEEX_FRAME_SIZE 320

#define MAX_FRAMES_PER_PACKET 100

static int preinit(sh_audio_t *sh) {
  sh->audio_out_minsize = 2 * 320 * MAX_FRAMES_PER_PACKET * 2 * sizeof(short);
  return 1;
}

static int read_le32(const uint8_t **src) {
    const uint8_t *p = *src;
    *src += 4;
    return p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
}

static int init(sh_audio_t *sh) {
  context_t *ctx = calloc(1, sizeof(context_t));

  mp_msg(MSGT_DECAUDIO, MSGL_WARN, "sh->wf->cbSize %d \n", sh->wf->cbSize);

	//	char* packet = (char *)(&sh->wf[1]);
	/*	for (unsigned i = 0; i < sh->wf->cbSize; ++i) {
			mp_msg(MSGT_DECAUDIO, MSGL_WARN, "0x%X ", packet[i]);
			
		}	
*/

/*
		if((ctx->fd = open("/tmp/test_rec.spx", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		//    OPRINT("could not open the file \n");
			printf("could not open the file \n");
			return;
		}
	*/	
		
  ctx->dec_context = speex_decoder_init(&speex_wb_mode);
  speex_bits_init(&ctx->bits);
 
  sh->channels = 1; //ctx->hdr->nb_channels;
  sh->samplerate = 16000;//ctx->hdr->rate;
  sh->samplesize = 2;
  sh->sample_format = AF_FORMAT_S16_NE;
  sh->context = ctx;
  return 1;

err_out:
  free(ctx);
  return 0;
}

static void uninit(sh_audio_t *sh) {
  context_t *ctx = sh->context;
  if (ctx) {
    speex_bits_destroy(&ctx->bits);
    speex_decoder_destroy(ctx->dec_context);
 //   close(ctx->fd);
	
    free(ctx);
  }
  
     
   	
	
  ctx = NULL;
}


static int decode_audio(sh_audio_t *sh, unsigned char *buf,
                        int minlen, int maxlen) {
  double pts;
  context_t *ctx = sh->context;
  int len, framelen, framesamples;
  char *packet;
  int i, err;
  
   // framelen = SPEEX_FRAME_SIZE;
  
  speex_decoder_ctl(ctx->dec_context, SPEEX_GET_FRAME_SIZE, &framesamples);
  
  framelen = framesamples *  sizeof(short);
  if (maxlen <  FRAMES_PER_RTP_PACKET * framelen) {
    mp_msg(MSGT_DECAUDIO, MSGL_V, "maxlen too small in decode_audio\n");
    return -1;
  }
  
  
  len = ds_get_packet_pts(sh->ds, (unsigned char **)&packet, &pts);
 
  if (len <= 0) return -1;
  if (sh->pts == MP_NOPTS_VALUE)
    sh->pts = 0;
  if (pts != MP_NOPTS_VALUE) {
    sh->pts = pts;
    sh->pts_bytes = 0;
  }
  
  /*
  		if(write(ctx->fd, packet, len) < 0) 
				perror("write() error");
							
	*/						
							
		char* fPtr = packet;
		
		int nloops =1;
		while(nloops <= 3){
			
			int nbBytes;// = *((int*)(fPtr));
			memcpy(&nbBytes,fPtr, sizeof(int));

	//		printf("nbBytes=%d\n", nbBytes);
	//		printf("len=%d\n", len);
	//		printf("minlen=%d\n", minlen);
			
			fPtr+=sizeof(int);	
			
			speex_bits_read_from(&ctx->bits, fPtr, nbBytes);
		//	speex_bits_read_from(&ctx->bits, fPtr, len);
			fPtr+=nbBytes;
			
	/*		
	  i = FRAMES_PER_RTP_PACKET;
	  do {
		err = speex_decode_int(ctx->dec_context, &ctx->bits, (short *)buf);
		if (err == -2)
		  mp_msg(MSGT_DECAUDIO, MSGL_ERR, "Error decoding file.\n");
		
		buf = &buf[framelen];
	  } while (--i > 0);
	  */
#if 1
			int nFrames = 1;
			while(nFrames<= FRAMES_PER_RTP_PACKET){
				/*Decode the data*/
				speex_decode_int(ctx->dec_context, &ctx->bits, (short *)buf);
		//		speex_decode(state, &bits, output);

				/*Copy from float to short (16 bits) for output*/
			//	for (i=0;i<frame_size;i++)
			//		out[i]=output[i];

				/*Write the decoded audio */
				buf += framelen;
				
				nFrames++;
			}

	  
			sh->pts_bytes += FRAMES_PER_RTP_PACKET * framelen;

//			printf("framelen=%d\n", framelen);
			
			nloops++;
		}						
						
	return FRAMES_PER_RTP_PACKET * framelen*3;
#else
	return 0;
#endif

}

static int control(sh_audio_t *sh, int cmd, void *arg, ...) {
  return CONTROL_UNKNOWN;
}
