#coding=utf-8

import socket
import struct

"""
对于沪宁标清视频，沪宁处提供一个编码服务器，将标清摄像头模拟信号转成数字信号，模转数编码协议采用亚奥协议。
获取视频数据流流程为：
1）客户端向编码服务器发送登录请求
2）发送视频传输启动命令
3）发送获取设备信息命令
4）根据接收的信息解析出组播地址
此组播地址在整个连接期间保持不变，但如果再次重复上述4个流程，可能收到不同的组播地址。
因此，对于亚奥协议的摄像头视频流获取，对某一路固定的相机，不能用固定的组播地址获取其数据流。
"""
def getMultiIP(forward_ip, forward_port, dev_port):
	encodeSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	encodeSock.settimeout(5)
	encodeSock.connect((forward_ip, forward_port))
	#发送登录信息
	info = struct.pack(">BBHIBBBBBBBBIIIIIHHBBBBIBBBB",0x03,0x00,0x00,0x34,\
							   0x00,0x00,0x00,0x00,\
							   0x00,0x00,0x00,0x00,\
							   0x00,0x0E,0xFFFF,0x00,0x00,0x3455,0x00,\
							   0xFE,0x00,0x00,0x00,0x186A5,0x00,0x00,0x00,0x00)
	encodeSock.send(info)            
	recvinfo = encodeSock.recv(4096)
	#发送视频传输启动命令
	info = struct.pack(">BBHIBBBBBBBBIIIIIHHIBBBBII",0x03,0x00,0x00,0x38,\
						         0x00,0x00,0x00,0x00,\
						         0x00,0x00,0x00,0x00,\
							 0x00,0x01,0x640368,dev_port,0x00,0x3455,0x00,\
						         0x640368,0x00,0x14,0x19,0x00,0x0600,0x0200)
	encodeSock.send(info)            
	#发送获取设备信息命令
	info = struct.pack(">BBHIBBBBBBBBIIIIIHHIIIIIIII",0x03,0x00,0x0000,0x48,\
							  0x00,0x00,0x00,0x00,\
							  0x00,0x00,0x00,0x00,\
							  0x00,0x81,0xFFFF,dev_port,\
							  0x00,0x3455,0x00,\
							  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00)
						
	encodeSock.send(info)             
	#根据接收的信息分析出对应的组播地址
	recvinfo = encodeSock.recv(4096)
	b,c,d = struct.unpack( ">BBB", recvinfo[59:62] )
	multiIP = "%d.%d.%d.%d" % (225 + dev_port % 8, b, c, d)
	return multiIP

def multiVideoStream(multiIP, multiPort, dev_port):
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
	sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	sock.bind((multiIP, multiPort))
	sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 255)
	sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP,
			socket.inet_aton(multiIP) + socket.inet_aton(localip))

	videoFile = open("./yaao_forward_test.h264", "wb")
	#while True:
	for i in range(1, 1000):
		try:
			info = sock.recv(4096)
			# 解析包信息过滤对应dev_port
			a,b,c,d,e,f,g,h,i,j,k,l,m = struct.unpack( "<BBHIIIIIIIIHH", info[0:40] )
			if a == 16 and j == dev_port:
				videoFile.write(info[40:])
		except:
			print "exception"
			continue
	videoFile.close()


if __name__ == "__main__":
	# 转发ip对应 摄像机IP.csv 文件中 IPADDR 列
	forward_ip = "168.168.5.44"
	# 转发port固定是 9000，同时组播的端口号也固定是9000
	forward_port = 9000 
	# 设备端口号对应 摄像机IP.csv 文件中 PORT 列
	dev_port = 2
	multiIP = getMultiIP(forward_ip, forward_port, dev_port) 
	print multiIP
	#multiVideoStream(multiIP, forward_port, port)

