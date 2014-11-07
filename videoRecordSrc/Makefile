FFMPEG_LIB = `pkg-config --libs libavformat libavcodec libswscale libavutil`
FFMPEG_CFLAGS = `pkg-config --cflags libavformat libavcodec libswscale libavutil`
 
all: gentech_record yaao_record

yaao_record: yaao_record.o gentech_log.o
	g++ -o ../videoRecordBin/yaao_record $^ $(FFMPEG_LIB)
yaao_record.o: yaao_record.cpp gentech_log.h gentech_log.cpp
	g++ -o $@ -c $< $(FFMPEG_CFLAGS)
gentech_record: gentech_record.cpp gentech_videoRecord.o gentech_log.o
	g++ -o ../videoRecordBin/gentech_record $^ $(FFMPEG_LIB) 
gentech_videoRecord.o: gentech_videoRecord.cpp gentech_videoRecord.h gentech_utility.h
	g++ -o $@ -c $< $(FFMPEG_CFLAGS)
gentech_log.o: gentech_log.cpp gentech_log.h
	g++ -o $@ -c $<
.PHONY: clean
clean:
	rm *.o gentech_record
