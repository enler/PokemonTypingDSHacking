#include "nds/ndstypes.h"
#include "common.h"


#ifdef JPN

void ** heapHeaderRef = (void**)0x20F4CE4;
u32 heapTop = 0x2130C20;

u32 PxiRtcCallback7 = 0x023E02A4;

const u8** backupCtxRef = (u8**)0x20C7D3C;
const int readBackupHookOffset = 0x2004CDC;
const int writeBackupHookOffset = 0x201953C;

const int handleTouchDataOffset = 0x2004724;
const int GetInputOffset = 0x205A1B8;
const u8 * wirelessKeyboardEnableFlag = 0x20CD31F;

#endif

#ifdef EUR

void ** heapHeaderRef = (void**)0x20F01B0;
u32 heapTop = 0x212C260;

u32 PxiRtcCallback7 = 0x023E02A4;

const u8** backupCtxRef = (u8**)0x20C30DC;
const int readBackupHookOffset = 0x2004D30;
const int writeBackupHookOffset = 0x2019658;

const int handleTouchDataOffset = 0x2004778;
const int GetInputOffset = 0x205AEB8;
u8 * wirelessKeyboardEnableFlag = (u8*)0x20C87D7;

#endif