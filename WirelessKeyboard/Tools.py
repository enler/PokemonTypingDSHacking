import sys
import struct

def searchFunc(mem, sig, isThumb):
	addr = mem.find(sig)
	if addr == -1:
		return -1;
	else:
		addr = addr + 0x2000000
		if isThumb:
			return addr + 1;
		else:
			return addr

def searchFunctions(mem):
	sigDicT = {'FS_InitFile':'\x23\x21\x00\x22\x09\x02\x11\x43\x82\x60\x42\x60\x02\x60\xC2\x61',
					'FS_OpenFileFast':'\x0F\xB4\x08\xB5\x82\xB0\x05\x99\x00\x23\x06\x9A\x00\x29\x09\xD0',
					'FS_ReadFile':'\x70\xB5\x82\xB0\x0E\x1C\x00\xA9\x05\x1C\x14\x1C\x00\xF0\xD2\xFF',
					'OS_EnableIrqMask':'\x18\xB4\x07\x4C\x00\x21\x23\x88\x22\x1C\x08\x32\x21\x80\x11\x68',
					'OS_SetIrqFunction':'\xF0\xB4\x0D\x1C\x00\x23\x06\x1C\x12\x49\x13\x4A\x18\x1C\x01\x24',
					'PXI_SendWordByFifo':'\x78\xB5\x81\xB0\x00\x9C\x1F\x23\x9C\x43\x1F\x23\x18\x40\x23\x1C',
					'PXI_SetFifoRecvCallback':'\x38\xB5\x05\x1C\x0C\x1C\xFF\xF7\x12\xEE\x02\x1C\xA9\x00\x0D\x48',
					'PXI_IsCallbackReady':'\x18\xB4\x01\x22\x82\x40\x06\x4C\x88\x00\x21\x18\xE2\x20\x80\x00',
					'OS_CreateThread':'\xF0\xB5\x83\xB0\x05\x1C\x00\x91\x01\x92\x1E\x1C\x01\xF0\x8E\xEB',
					'OS_ExitThread':'\x08\xB5\x01\xF0\x30\xEB\x03\x48\x00\x21\x00\x6A\x00\xF0\x04\xF8',
					'OS_WakeupThreadDirect':'\x38\xB5\x05\x1C\x01\xF0\x54\xEA\x04\x1C\x01\x20\x68\x66\xFF\xF7',
					'OS_Sleep':'\x78\xB5\x8D\xB0\x02\xAE\x05\x1C\x30\x1C\x00\xF0\x8D\xFF\x15\x48',
					'FndAllocFromExpHeapEx':'\x08\xB5\x00\x29\x00\xD1\x01\x21\x03\x23\xC9\x1C\x99\x43\x00\x2A',
					'FndFreeToExpHeap':'\x70\xB5\x82\xB0\x0D\x1C\x04\x1C\x10\x3D\x00\xAE\x24\x34\x30\x1C'}
	symbolDic = {}
	for key, v in sigDicT.items():
		addr = searchFunc(mem, v, True)
		symbolDic['_' + key] = addr
	sigDicArm = {'DC_InvalidateRange':'\x00\x10\x81\xE0\x1F\x00\xC0\xE3\x36\x0F\x07\xEE\x20\x00\x80\xE2',
					'DC_StoreRange':'\x00\x10\x81\xE0\x1F\x00\xC0\xE3\x3A\x0F\x07\xEE\x20\x00\x80\xE2',
					'DC_FlushRange':'\x00\xC0\xA0\xE3\x00\x10\x81\xE0\x1F\x00\xC0\xE3\x9A\xCF\x07\xEE',
					'DC_FlushAll':'\x00\xC0\xA0\xE3\x00\x10\xA0\xE3\x00\x00\xA0\xE3\x00\x20\x81\xE1'
		}
	for key, v in sigDicArm.items():
		addr = searchFunc(mem, v, False)
		symbolDic[key] = addr
	sigDicT2 = {'_FS_OpenFile':('\xF8\xB5\xCA\xB0\x05\x1C\x16\x1C\x00\x24\x08\x1C\x00\xA9\x04\xAA', '\x01\x4B\x01\x22\x18\x47\xC0\x46'),
					'_FS_CloseFile': ('\xF0\xB5\x83\xB0\x05\x1C\xE8\x68\x00\x91\x01\x21\x17\x1C\x00\x26','\x01\x4B\x08\x21\x01\x22\x18\x47')}
	for key, v in sigDicT2.items():
		addr = searchFunc(mem, v[0], True)
		sig2 = v[1] + struct.pack('i', addr)
		addr = searchFunc(mem, sig2, True)
		symbolDic[key] = addr
	# for EUR FRA GER ITA SPA these addresses are fixed
	symbolDic['_backupGetBuffer'] = 0x201A5B9
	symbolDic['_orig_HandleTouchData_8'] = 0x2004781
	symbolDic['backupCtxRef'] = 0x2002B80
	symbolDic['wirelessKeyboardEnableFlag'] = 0x200897C
	instructionH = struct.unpack('H', mem[0x4D30: 0x4D32])[0]
	instructionL = struct.unpack('H', mem[0x4D32: 0x4D34])[0]
	immH = instructionH & 0x3FF
	immL = instructionL & 0x7FF
	imm = (immH << 12) | (immL << 1)
	symbolDic['_foo']  = 0x2004D34 + imm + 1
	return symbolDic

def searchValue(mem, value):
	results = []
	offset = 0
	while True:
		result = mem.find(value, offset)
		if result == -1:
			break
		results = results + [result +  0x2000000]
		offset = result + len(value)
	return results

def searchHeap(mem):
	HPXEOffsets = searchValue(mem, 'HPXE')
	heapTopOffsetRefs = searchValue(mem, struct.pack('i', HPXEOffsets[1]))
	heapHeaderRefs = searchValue(mem, struct.pack('i', HPXEOffsets[2]))
	return [HPXEOffsets[1], heapTopOffsetRefs[0], heapHeaderRefs[2]]

def searchSymbols(mem, heapdata):
	results = []
	symDic = {'const int GetInputOffset':'\x02\x6C\x41\x6C\x8A\x42\x01\xD1\x00\x20\x70\x47\x91\x00\x43\x58'}
	for key, v in symDic.items():
		result = searchFunc(mem, v, False)
		results = results + [key + '=' + '0x%X' % result + ';']
	results = results + ['void ** heapHeaderRef = (void**)' + '0x%X' % heapdata[2] + ';']
	results = results + ['u32 heapTop = ' + '0x%X' % heapdata[0] + ';']
	return results

def listAddressOfLoadPossible(mem):
	securityArea = mem[0 : 2048]
	subAreas = securityArea.split('\xDF\x70\x47')
	offset = 0
	max = 0
	maxOffset = 0
	for area in subAreas:
		if max < len(area) - 1:
			maxOffset = offset
			max = len(area) - 1
		offset = offset + len(area) + 3
	maxOffset = maxOffset + 0x2000000
	print 'address = 0x%X , length = 0x%X' % (maxOffset, max)
	ins = 0xFA000000 | ((maxOffset - 0x2000900 - 8) >> 2) & 0xFFFFFF
	print '0x2000900 0x%X' % ins
	imm = (maxOffset + 0x10 - 0x2000BB2 - 4) >> 1;
	insH = ((imm >> 11) & 0x7FF) | 0xF000
	insL = (imm & 0x7FF) | 0xF800
	print '0x2000BB2 %X, %X' % (insH, insL)

def tryGetTypingMap(mem):
	ref = struct.unpack('i', mem[0x1094 : 0x1094 + 4])[0] & 0xFFFFFF
	ref = struct.unpack('i', mem[ref : ref + 4])[0] & 0xFFFFFF
	ref = struct.unpack('i', mem[ref + 4 : ref + 8])[0] & 0xFFFFFF
	if ref == 0:
		return ''
	dic = {}
	print '0x%X' % ref
	for i in range(0, 0x40):
		typingDataOffset = struct.unpack('i', mem[ref + 8 + i * 4 : ref + 0xc + i * 4])[0] & 0xFFFFFF
		for j in range(0, 4):
			char = struct.unpack('H', mem[typingDataOffset + 0x10 + j * 2 : typingDataOffset + 0x12 + j * 2])[0]
			if char == 0:
				continue
			elif dic.has_key(char) == False:
				dic[char] = struct.unpack('H', mem[typingDataOffset + 0x18 : typingDataOffset + 0x1A])[0]
	items = dic.items()
	items.sort()
	typingMap = ''
	for k, v in items:
		typingMap = typingMap + struct.pack('HH',k, v)
	return typingMap

def main():
	if len(sys.argv) == 2:
		f = open(sys.argv[1], 'rb')
		mem = f.read()
		f.close()
		functions = searchFunctions(mem)
		heapdata = searchHeap(mem)
		symbols = searchSymbols(mem, heapdata)
		print 'arm7payload entrypoint = 0x%x' % (heapdata[0] + 0x400000)
		print 'arm9payload entrypoint = 0x%x' % ((heapdata[0] + 0x4000 + 0x3FF) & ~0x3FF)
		print 'oldHeapRef = 0x%X' % heapdata[1]
		archiveObjs = searchValue(mem, 'rom\x00')
		print 'archiveObj = 0x%X' % archiveObjs[4]
		listAddressOfLoadPossible(mem)
		f = open('symbol.sym', 'w')
		for k, v in functions.items():
			f.write(k + ' = ' + '0x%X' % v + ';\n')
		f.write('__entry = ' + '0x%X' % ((heapdata[0] + 0x4000 + 0x3FF) & ~0x3FF) + ';\n')
		f.close()
		f = open('common.c', 'w')
		for sym in symbols:
			f.write(sym + '\n')
		f.close()
		typingMap = tryGetTypingMap(mem)
		if len(typingMap) != 0:
			f = open('typingMap.bin', 'wb')
			f.write(typingMap)
			f.close()

if __name__ == "__main__":
	main()