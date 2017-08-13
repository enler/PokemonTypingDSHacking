import pythoncom
import pyHook
import socket
import struct
import time
import png

seqNo = 0

def sendCMDPacket(cmd, data):
    global seqNo
    if len(data) > 0x10:
        data = data[0 : 0x10]
    msg = 'TPAC' + struct.pack('iii%ds%dx' % (len(data), 16 - len(data)), seqNo, cmd, 0x10, data)
    s.sendto(msg, address)
    curSeqNo = seqNo
    seqNo = seqNo + 1
    return curSeqNo

def recvAckPacket(ackSeqNo):
    while True:
        try:
            msg, addr = s.recvfrom(400)
        except:
            return (False, '')
        if len(msg) != 32:
            continue
        packetType, seq, cmd, bodySize, data = struct.unpack('4i16s', msg)
        if packetType == 0x43415054 and seq == ackSeqNo and cmd == 1:
            return (True, data)

def recvDataPacket(ackSeqNo, packetsz):
    while True:
        try:
            msg, addr = s.recvfrom(16 + packetsz)
        except:
            return (False, '', -1)
        if len(msg) != 400:
            continue
        packetType, seq, index, bodySize, data = struct.unpack('4i%ds' % packetsz, msg)
        if packetType == 0x44415054 and seq == ackSeqNo and bodySize == packetsz:
            return (True, data, index)

def recvBuffer(sz, packetsz, packectr):
    global seqNo
    s.settimeout(1.5)
    ackSeqNo = sendCMDPacket(6, '')
    packets = [False] * packectr
    while True:
        result, data, index = recvDataPacket(ackSeqNo, packetsz)
        if result == False:
            break
        else:
            packets[index] = data
    s.settimeout(0.1)
    for i in range(0, packectr):
        if packets[i] == False:
            print 'missing packet id', i
            for j in range(0, 4):
                ackSeqNo = sendCMDPacket(4, struct.pack('i', i))
                result, data, index = recvDataPacket(ackSeqNo, packetsz)
                if result == False:
                    continue
                elif index != i:
                    print 'index is not match'
                else:
                    packets[index] = data
                    break
    ackSeqNo = sendCMDPacket(5, '')
    result, body = recvAckPacket(ackSeqNo)
    if result == False:
        print 'ack packet missing'
    outbuf = ''
    missingbuf = 'missing\x00' * (packetsz / 8 + 1)
    for subPacket in packets:
        if subPacket == False:
            outbuf = outbuf + missingbuf[0 : packetsz]
        else:
            outbuf = outbuf + subPacket
    return outbuf[0 : sz]

def savePng(width, height, buffer, fname):
    rows = [0] * height
    for j in range(0, height):
        row = []
        for i in range(0, width):
            value = struct.unpack("H", buffer[(j * width + i) * 2 : (j * width + i) * 2 + 2])[0]
            r = (value & 0x1F) << 3
            g = (value & 0x3E0) >> 2
            b = (value & 0x7C00) >> 7
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
	half/full 0xFF09 (JPN only)
	yan mark 0xFF0A (JPN only)
	left alt 0xFF0B (EUR only)
	left 0xE04B
	up 0xE048
	down 0xE050
	right 0xE04D
	"""
    spKeyDic = {20:0xFF06, 37:0xE04B, 38:0xE048, 39:0xE04D, 40:0xE050, 160:0xFF00, 161:0xFF01, 162:0xFF02, 36:0xFF08, 27:0xFF09, 164:0xFF04, 165:0xFF0B, 163:0xFF07}
    print 'key down'
    global seqNo
    if chr(event.Ascii)[0] != '\x00' and spKeyDic.has_key(event.KeyID) == False:
        curSeqNo = sendCMDPacket(2, chr(event.Ascii))
        result, body = recvAckPacket(curSeqNo)
        if result == False:
            print 'ack packet missing'
    elif spKeyDic.has_key(event.KeyID):
        keyValue = spKeyDic[event.KeyID]
        curSeqNo = sendCMDPacket(2, struct.pack('H', keyValue))
        result, body = recvAckPacket(curSeqNo)
        if result == False:
            print 'ack packet missing'
    elif event.KeyID == 44:
        curSeqNo = sendCMDPacket(3, '')
        result, body = recvAckPacket(curSeqNo)
        if result == False:
            print 'ack packet missing'
        else:
            sz, packectr, packetsz = struct.unpack('3i4x', body)
            if sz != 0 and packectr != 0 and packetsz != 0:
                buf = recvBuffer(sz, packetsz, packectr)
                fname = time.strftime('%Y%m%d%H%M%S',time.localtime(time.time()))
                savePng(256, 192, buf, fname + ".png")
                print "save screenshot"
    return True



def main():
    ndsIP = raw_input('enter IP of nds:\n')
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
