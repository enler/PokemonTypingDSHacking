
# -*- coding: utf-8 -*-
 
import pythoncom
import pyHook
import socket
import struct
import time

seqNo = 0
 
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
    elif spKeyDic.has_key(event.KeyID):
        keyValue = spKeyDic[event.KeyID]
        msg = struct.pack("4iH14x", 0, 2, seqNo, 0, keyValue)
        # print repr(msg)
        s.sendto(msg, address)
        seqNo = seqNo + 1
    return True
 
def main():
    ndsIP = raw_input("enter IP of NDS\n")
    global address
    global s
    
    address = (ndsIP, 818)
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    hm = pyHook.HookManager()
 
    hm.KeyDown = onKeyboardEvent

    hm.HookKeyboard()
    
    pythoncom.PumpMessages()
 
if __name__ == "__main__":
    main()
