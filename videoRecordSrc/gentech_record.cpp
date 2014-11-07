#include "gentech_videoRecord.h"
#include "gentech_utility.h"
#include "gentech_log.h"
#include <iostream>

int main(int argc, char **argv) 
{
	if (argc != 4) {
		std::cerr<<"./gentech_record streamPath videoFilePath logFilePath"<<std::endl;
		exit(1);
	}
	// open log file
	const char *pLogFilePath = argv[3];
	if (!gentech::logInit(pLogFilePath)) {
		std::cerr<<"log module init failed."<<std::endl;
		exit(1);
	}
	// open video stream
	int ret = 0;
	const char *pStreamPath = argv[1];
	gentech::CVideoRecord record;
	ret = record.open(pStreamPath);
	if (ret == GENTECH_FUNC_FAILED) {
		gentech::logWrite("open the video stream failed.\n");
		exit(1);
	}
	// open video file
	const char *pVideoSavePath = argv[2];
	FILE *pVideoFile = fopen(pVideoSavePath, "wb");
	if (!pVideoFile) {
		gentech::logWrite("open the video file for saveing failed.\n");
		exit(1);
	}
	// write 25*60*20(20 minutes) frames to video file
	int nFrames = 25 * 60 * 20;
	for (int i = 0; i < nFrames; ++i) {
		ret = record.write(pVideoFile);
		if (ret == GENTECH_FUNC_FAILED) {
			char log[200] = { 0 };
			snprintf(log, sizeof(log), "write %d frame failed.\n", i + 1);
			gentech::logWrite(log);
		}
	}
	// write succeed, clean up
	fclose(pVideoFile);
	gentech::logClose();
}

