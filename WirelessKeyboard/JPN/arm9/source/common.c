#include "nds/ndstypes.h"
#include "common.h"

u32 PxiRtcCallback7 = 0x023E02A4;

#if (defined JPN)

void ** heapHeaderRef = (void**)0x20F4CE4;
u32 heapTop = 0x2130C20;

const u8** backupCtxRef = (const u8**)0x20C7D3C;
const int readBackupHookOffset = 0x2004CDC;
const int writeBackupHookOffset = 0x201953C;

const int handleTouchDataOffset = 0x2004724;
const int GetInputOffset = 0x205A1B8;
u8 * wirelessKeyboardEnableFlag = (u8*)0x20CD31F;

const char * arm7BinPath = "/worldmap/sprite/arm7.bin";

#elif (defined EUR)

void ** heapHeaderRef = (void**)0x20F01B0;
u32 heapTop = 0x212C260;
const int GetInputOffset = 0x205AEB8;

const char * arm7BinPath = "dataUK/worldmap/sprite/arm7.bin";
 
#elif (defined ITA)

const int GetInputOffset=0x205AED4;
void ** heapHeaderRef = (void**)0x20F01D0;
u32 heapTop = 0x212C280;

const char * arm7BinPath = "dataIT/worldmap/sprite/arm7.bin";

#elif (defined SPA)

const int GetInputOffset=0x205AEE0;
void ** heapHeaderRef = (void**)0x20F01D0;
u32 heapTop = 0x212C280;

const char * arm7BinPath = "dataSP/worldmap/sprite/arm7.bin";

#elif (defined FRA)

const int GetInputOffset=0x205AEC4;
void ** heapHeaderRef = (void**)0x20F01D0;
u32 heapTop = 0x212C260;

const char * arm7BinPath = "dataFR/worldmap/sprite/arm7.bin";

#elif (defined GER)

const int GetInputOffset=0x205AF4C;
void ** heapHeaderRef = (void**)0x20F0270;
u32 heapTop = 0x212C320;

const char * arm7BinPath = "dataGE/worldmap/sprite/arm7.bin";

#endif 

#if (defined EUR) || (defined ITA) || (defined SPA) || (defined FRA) || (defined GER)

const int readBackupHookOffset = 0x2004D30;
const int writeBackupHookOffset = 0x2019658;
const int handleTouchDataOffset = 0x2004778;

#endif

