#include "nds/ndstypes.h"
#include "arm9.h"

#define MAKE_BRANCH_T_L(src, dest) (((((s32)(dest) - 4 - (s32)(src)) >> 12) & 0x7FF) | 0xF000)
#define MAKE_BRANCH_T_H(src, dest) (((((s32)(dest) - 4 - (s32)(src)) >> 1) & 0x7FF) | 0xF800)

u32 backupSharedMem[96 / 4] = {0, 0x1101, 0, 0,
								0, 0,  0x20000, 0x100,
								0, 0x100, 3, 5,
								0, 0, 0, 0,
								0, 0, 0, 0,
								0, 0, 0xFFFF, 0};

int backupCtxHandler = 0x20C7D3C;
int backupLength = 0xFD8;

int requestFlag = 0;

void onFifoRecv(int tag, u32 data, bool err)
{
    if ((tag == 11) && err) {
    	if (1 == data) {
    		requestFlag = 1;
    	}
    }
}

static inline bool sendBackupFifo(u32 data) {
	return PXI_SendWordByFifo(11, data, 1) >= 0;
}

void initBackup() {
	requestFlag = 0;
	while(!sendBackupFifo(0));
	while(!sendBackupFifo((u32)&backupSharedMem));
	do OS_Sleep(100); while(!requestFlag);
	requestFlag = 0;
	while(!sendBackupFifo(2));
	do OS_Sleep(100); while(!requestFlag);
}

u32 readBackup() {
	u32 (*foo)(void) = (void*)0x2106941;
	u32 ret = foo();
	u8 * backupCtx = (u8*)(*(u32*)backupCtxHandler);
	for (int i = 0; i < 4; i++) {
		void * buf = backupGetBuffer(backupCtx + i * 120 + 12, 0);
		backupSharedMem[3] = i << 12;
		backupSharedMem[4] = (int)buf;
		backupSharedMem[5] = backupLength;
		requestFlag = 0;
		while(!sendBackupFifo(6));
		do OS_Sleep(100); while(!requestFlag);
	}
	return ret;
}

void writeBackup() {
	u8 * backupCtx = (u8*)(*(u32*)backupCtxHandler);
	int slot = backupCtx[480];
	u8 * slotCtx = backupCtx + slot * 120;
	void * buf = backupGetBuffer(slotCtx + 12, 0);
	backupSharedMem[3] = (int)buf;
	backupSharedMem[4] = slot << 12;
	backupSharedMem[5] = backupLength;
	requestFlag = 0;
	while(!sendBackupFifo(7));
	do OS_Sleep(100); while(!requestFlag);
}

int main() {
	*(u16*)0x2004CDC = (u16)MAKE_BRANCH_T_L(0x2004CDC, readBackup);
	*(u16*)0x2004CDE = (u16)MAKE_BRANCH_T_H(0x2004CDC, readBackup);
	*(u16*)0x201953C = (u16)MAKE_BRANCH_T_L(0x201953C, jmp_writeBackup);
	*(u16*)0x201953E = (u16)MAKE_BRANCH_T_H(0x201953C, jmp_writeBackup);
	PXI_SetFifoRecvCallback(11, onFifoRecv);
	initBackup();
	return 0;
}