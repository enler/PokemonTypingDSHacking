
# -*- coding: utf-8 -*-
 
import pythoncom
import pyHook
import socket
import struct
import time
import png

seqNo = 0

def recvBuffer(sz, packetsz, packectr):
	s.settimeout(1.5)
	packets = [False] * packectr
	packetfmt = "Qi4x" + str(packetsz) + "s"
	while True:
		try:
			data, addr = s.recvfrom(16 + packetsz)
			magic, tmp_seqNo, buf = struct.unpack(packetfmt, data)
			# print "magic", magic
			# print "seq no", tmp_seqNo
			packets[tmp_seqNo] = buf
			# print repr(buf)
		except socket.timeout, e:
			print e
			break
	print "retry"
	for index in range(0, packectr):
		if packets[index] == False:
			msg = struct.pack("5i12x", 0, 4, seqNo, 0, index)
			print "missing packet id", index
			s.sendto(msg, address)
			for i in range(0, 4):
				try:
					data, addr = s.recvfrom(16 + packetsz)
					magic, tmp_seqNo, buf = struct.unpack(packetfmt, data)
					packets[tmp_seqNo] = buf
					# print repr(buf)
					break
				except socket.timeout, e:
					print e
					continue
	msg = struct.pack("4i16x", 0, 5, 0, 0)
	s.settimeout(0.1)
	for i in range(0, 4):
		s.sendto(msg, address)
		try:
			msg, addr = s.recvfrom(32)
			break
		except socket.timeout, e:
			print "ack packet lose"
			continue
	outbuf = ""
	missingbuf = "missing\x00" * (packetsz / 8)
	for subPacket in packets:
		if subPacket == False:
			outbuf = outbuf + missingbuf
		else:
			outbuf = outbuf + subPacket
	return outbuf[0:sz]

def savePng(width, height, buffer, fname):
	rows = [0] * height
	for j in range(0, height):
		row = []
		for i in range(0, width):
			value = struct.unpack("H", buffer[(j * width + i) * 2 : (j * width + i) * 2 + 2])[0]
			r = (value & 0x1F) << 3
			g = (value & 0x3E0) >> 2
			b = (value & 0x7C00) >> 7
			#print r, g, b
			row = row + [r, g, b]
		rows[j] = row
	f = open(fname, 'wb')
	w = png.Writer(width, height)
	w.write(f, rows)
	f.close()
 
def onKeyboardEvent(event):
	"""
	print "Ascii:", event.Ascii, chr(event.Ascii)
	print "Key:", event.Key
	print "KeyID:", event.KeyID
	special keys:
	right shift 0xFF00
	left shift 0xFF01
	control 0xFF02
	right alt 0xFF04
	caps lock 0xFF06
	function 0xFF07
	home 0xFF08
	half/full 0xFF09
	yan mark 0xFF0A
	left 0xE04B
	up 0xE048
	down 0xE050
	right 0xE04D
	"""
	spKeyDic = {20:0xFF06,  37: 0xE04B, 38: 0xE048, 39: 0xE04D, 40: 0xE050, 160: 0xFF00,161:0xFF01, 162:0xFF02, 36: 0xFF08, 27: 0xFF09, 164:0xFF04}
	global seqNo
	if chr(event.Ascii)[0] != '\x00' and spKeyDic.has_key(event.KeyID) == False:
		msg = struct.pack("4ic15x", 0, 2, seqNo, 0, chr(event.Ascii)[0])
		# print repr(msg)
		s.sendto(msg, address)
		seqNo = seqNo + 1
		try:
			msg, addr = s.recvfrom(32)
		except socket.timeout, e:
			print "ack packet lose"
			msg = struct.pack("4i16x", 0, 5, 0, 0)
			s.sendto(msg, address)
	elif spKeyDic.has_key(event.KeyID):
		keyValue = spKeyDic[event.KeyID]
		msg = struct.pack("4iH14x", 0, 2, seqNo, 0, keyValue)
		# print repr(msg)
		s.sendto(msg, address)
		seqNo = seqNo + 1
		try:
			msg, addr = s.recvfrom(32)
		except socket.timeout, e:
			print "ack packet lose"
			msg = struct.pack("4i16x", 0, 5, 0, 0)
			s.sendto(msg, address)
	elif event.KeyID == 44:
		msg = struct.pack("4i16x", 0, 3, seqNo, 0)
		# print repr(msg)
		s.sendto(msg, address)
		seqNo = seqNo + 1
		try:
			msg, addr = s.recvfrom(32)
			sz, packectr, packetsz = struct.unpack("16x3i4x", msg)
			print "size = ", sz
			print "packet counter = ", packectr
			if sz != 0:
				msg = struct.pack("4i16x", 0, 6, seqNo, 0)
				s.sendto(msg, address)
				seqNo = seqNo + 1
				buf = recvBuffer(sz, packetsz, packectr)
				fname = time.strftime('%Y%m%d%H%M%S',time.localtime(time.time()))
				savePng(256, 192, buf, fname + ".png")
				print "save screenshot"
		except socket.timeout, e:
			print "ack packet lose"
	return True
 
def main():
	ndsIP = raw_input("enter IP of NDS\n")
	global address
	global s

	address = (ndsIP, 818)
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	s.settimeout(0.1)
	hm = pyHook.HookManager()
 
	hm.KeyDown = onKeyboardEvent

	hm.HookKeyboard()
	
	pythoncom.PumpMessages()
 
if __name__ == "__main__":
	main()
