import json
import datetime
import os
import time
import random

videoFilePath = "/Recorder/24-record/"
logFilePath = "/Recorder/logs/"
localIP = "168.168.11.52"
record_time = [8,12,14,16]

cameras = {
	"ALK":{
		"1121+800N":"rtsp://168.168.15.178/HD",
		"1115+700N":"rtsp://168.168.17.35/HD",
		"1103+000S":"rtsp://168.168.17.55/HD",
		"158+659S":"rtsp://168.168.18.22/HD",
		"217+450S":"rtsp://168.168.19.22/HD",
		"234+800N":"rtsp://168.168.19.98/HD",
		"172+833S":"rtsp://168.168.18.6/HD",
		"202+850S":"rtsp://168.168.19.6/HD"
	},
	"YFT":{
		"1135+050N":"udp://224.2.15.142:11112"
	},
	"YaAo":{
		"1140+500S":["231.169.165.7",9000,6],
		"267+800N":["227.169.163.70",9000,2],
		"1120+900S":["227.169.163.35",9000,2]
	}
}
def date_time():
	year = datetime.datetime.now().year
	month = datetime.datetime.now().month
	day = datetime.datetime.now().day
	hour = datetime.datetime.now().hour
	minute = datetime.datetime.now().minute
	second = datetime.datetime.now().second
	time = "%d-%d-%d-%d-%d"%(year,month,day,hour,minute)
	times = [time,hour,minute,second]
	return times

def alk_camera():
	c = cameras["ALK"]
	t = c.keys()
	camera = random.choice(t)
	streamPath = c[camera]
	fileName = "%s_%s.avi"%(date_time()[0],camera)	
	print "./gentech_record %s %s %s"%(streamPath,videoFilePath + fileName,logFilePath)
def yft_camera():
	c = cameras["YFT"]
	t = c.keys()
	camera = random.choice(t)
	streamPath = c[camera]
	fileName = "%s_%s.avi"%(date_time()[0],camera)	
	print "./gentech_record %s %s %s"%(streamPath,videoFilePath + fileName,logFilePath)
def ya_camera():
	c = cameras["YaAo"]
	t = c.keys()
	camera = random.choice(t)
	multiIP = c[camera][0]
	multiPort = c[camera][1]
	devicePort = c[camera][2]
	fileName = "%s_%s.avi"%(date_time()[0],camera)
	print "./yaao_record %s %s %s %s %s %s"%(localIP,multiIP,multiPort,devicePort,videoFilePath + fileName,logFilePath)

if __name__ == '__main__':
	while True:
		if date_time()[1] in record_time and date_time()[2] == 10 and date_time()[3] in range(0,5):
			alk_camera()
			yft_camera()
			ya_camera()
		elif date_time()[1] == 14 and date_time()[2] == 10 and date_time()[3] in range(0,5):
			print "reboot"
		print date_time()[0],date_time()[3]
		time.sleep(5)

		




