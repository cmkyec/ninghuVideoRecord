#define __STDC_CONSTANT_MACROS
extern "C"
{
#include <libavcodec/avcodec.h> 
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include <sys/types.h>    
#include <sys/socket.h>    
#include <netinet/in.h>    
#include <arpa/inet.h>    
#include <string.h>    
#include <stdio.h>    
#include <unistd.h>    
#include <stdlib.h>    
#include "gentech_log.h"

static int codecInit(AVCodecContext *&pCodecCtx, AVCodec *&pCodec, AVCodecParserContext *&pParserCtx)
{
	av_register_all();
	avformat_network_init();
	pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!pCodec) return -1;

	pCodecCtx = avcodec_alloc_context3(NULL);
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;
	pCodecCtx->bit_rate = 0;
	pCodecCtx->ticks_per_frame = 2;
	pCodecCtx->frame_number = 1;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->width = 720;
	pCodecCtx->height = 576;
	pCodecCtx->coded_width = 720;
	pCodecCtx->coded_height = 576;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	pCodecCtx->flags2 |=  CODEC_FLAG2_CHUNKS;
	if (avcodec_open2(pCodecCtx, pCodec, NULL) != 0) {
		printf("avcodec_open2 failed.\n");
		return -1;
	}

	pParserCtx = av_parser_init(AV_CODEC_ID_H264);
	if (!pParserCtx) {
		printf("av_parser_init failed.\n");
		return -1;
	}
	return 0;	
}

static int decodeFrame(AVCodecContext *&pCodecCtx, AVFrame *&pFrame, uint8_t *pdata, int size)
{
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = pdata;
	packet.size = size;
	int got_picture = 0;
	int iret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
	if (iret >= 0 && got_picture) {
		return 1;
	}
	return 0;
}

int main(int argc, char ** argv)
{
	if (argc != 7) {
		printf("./yaao_record localIP multiIP multiPort devicePort videoFielPath logFilePath\n");
		exit(1);
	}
	const char *pLocalIP = argv[1];
	const char *pMultiIP = argv[2];
	int multiPort = atoi(argv[3]);
	int devicePort = atoi(argv[4]);
	const char *pVideoFilePath = argv[5];
	const char *pLogFilePath = argv[6];

	// open log file
	if (!gentech::logInit(pLogFilePath)) {
		printf("can not open the log file.\n");
		exit(1);
	}
	char logInfo[200] = { 0 };
	// open video file
	FILE *pVideoFile = fopen(pVideoFilePath, "wb");
	if (!pVideoFile) {
		snprintf(logInfo, sizeof(logInfo), "open video file for saving failed.\n");
		exit(1);
	}

	av_register_all();
	avformat_network_init();
	int sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0) {
		snprintf(logInfo, sizeof(logInfo), "open datagram socket failed.\n");
		gentech::logWrite(logInfo);
		exit(1);
	}

	int reuse = 1;
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
		snprintf(logInfo, sizeof(logInfo), "multicast set reuse address failed.\n");
		gentech::logWrite(logInfo);
		exit(1);
	}

	struct sockaddr_in localsock;
	memset(&localsock, 0, sizeof(localsock));
	localsock.sin_family = AF_INET;
	localsock.sin_port = htons(multiPort);
	localsock.sin_addr.s_addr = inet_addr(pMultiIP);
	if (bind(sd, (struct sockaddr *)&localsock, sizeof(localsock)) < 0) {
		snprintf(logInfo, sizeof(logInfo), "multicast bind socket failed.\n");
		gentech::logWrite(logInfo);
		exit(1);
	}

	struct ip_mreq group;
	group.imr_multiaddr.s_addr = inet_addr(pMultiIP);
	group.imr_interface.s_addr = inet_addr(pLocalIP);
	if (setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0) {
		snprintf(logInfo, sizeof(logInfo), "add multicast group failed.\n");
		gentech::logWrite(logInfo);
		exit(1);
	}

	AVCodecParserContext *pParserCtx = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec = NULL;
	if (codecInit(pCodecCtx, pCodec, pParserCtx) < 0) {
		snprintf(logInfo, sizeof(logInfo), "codecInit failed.\n");
		gentech::logWrite(logInfo);
		return 1;
	}	
	
	char *info = (char *)malloc(sizeof(char) * 3000);
	if (!info) {
		snprintf(logInfo, sizeof(logInfo), "malloc info failed.\n");
		gentech::logWrite(logInfo);
		return 1;
	}
	struct sockaddr_in sendaddr;
	int sendaddrsize = sizeof(sendaddr);

  	int64_t pts = AV_NOPTS_VALUE;
	int64_t dts = AV_NOPTS_VALUE;	
	int saveFrames = 25 * 60 * 20, nFrames = 1;
	while (nFrames < saveFrames) {
		memset(info, 0, sizeof(char) * 3000);
		size_t nbyte = recvfrom(sd, info, 3000, 0, (struct sockaddr *)&sendaddr, (socklen_t *)&sendaddrsize);
		if (nbyte < 0) {
			snprintf(logInfo, sizeof(logInfo), "multicast recvfrom error.\n");
			gentech::logWrite(logInfo);
			break;
		}
		else if (nbyte == 0) {
			snprintf(logInfo, sizeof(logInfo), "multicast source close.\n");
			gentech::logWrite(logInfo);
			break;
		}
		else if (info[0] != 16 && info[28] != devicePort && info[29] != 0 && info[30] != 0 && info[31] != 0) {
			continue;
		}
		else {
			int parsePos = 0;
			char *actualInfo = info + 40;
			int actualByte = nbyte - 40;
			while (actualByte > 0) {
				uint8_t *poutbuf = NULL;
				int poutbuf_size = 0;
				int len = av_parser_parse2(pParserCtx, pCodecCtx, &poutbuf, &poutbuf_size,
						           (uint8_t *)(actualInfo + parsePos), actualByte, pts, dts, AV_NOPTS_VALUE);
				if (poutbuf_size) {
					fwrite(poutbuf, sizeof(uint8_t), poutbuf_size, pVideoFile);
					nFrames++;
				}
				parsePos += len;
				actualByte -= len;
			}	
		}
		continue;
	}
	free(info);
	close(sd);
	avcodec_free_context(&pCodecCtx);
	av_parser_close(pParserCtx);
	fclose(pVideoFile);
	gentech::logClose();
	return 0;
}
