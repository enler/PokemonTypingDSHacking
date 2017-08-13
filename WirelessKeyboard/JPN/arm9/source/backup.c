#include "nds/ndstypes.h"
#include "common.h"

#define MAKE_BRANCH_T_L(src, dest) (((((s32)(dest) - 4 - (s32)(src)) >> 12) & 0x7FF) | 0xF000)
#define MAKE_BRANCH_T_H(src, dest) (((((s32)(dest) - 4 - (s32)(src)) >> 1) & 0x7FF) | 0xF800)

void writeBackup();
u32 readBackup();

u32 backupSharedMem[96 / 4] = {0, 0x1101, 0, 0, 0, 0, 0x20000, 0x100, 0, 0x100, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFFFF, 0};

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
	*(u16*)readBackupHookOffset = (u16)MAKE_BRANCH_T_L(readBackupHookOffset, readBackup);
	*(u16*)(readBackupHookOffset + 2) = (u16)MAKE_BRANCH_T_H(readBackupHookOffset, readBackup);
	*(u16*)writeBackupHookOffset = (u16)MAKE_BRANCH_T_L(writeBackupHookOffset, jmp_writeBackup);
	*(u16*)(writeBackupHookOffset + 2) = (u16)MAKE_BRANCH_T_H(writeBackupHookOffset, jmp_writeBackup);
	PXI_SetFifoRecvCallback(11, onFifoRecv);
	requestFlag = 0;
	while(!sendBackupFifo(0));
	DC_FlushRange(&backupSharedMem, sizeof(backupSharedMem));
	while(!sendBackupFifo((u32)&backupSharedMem));
	do OS_Sleep(100); while(!requestFlag);
	requestFlag = 0;
	while(!sendBackupFifo(2));
	do OS_Sleep(100); while(!requestFlag);
}

u32 readBackup() {
	u32 ret = foo();
	const u8 * backupCtx = *backupCtxRef;
	for (int i = 0; i < 4; i++) {
		void * buf = backupGetBuffer(backupCtx + i * 120 + 12, 0);
		backupSharedMem[3] = i << 12;
		backupSharedMem[4] = (int)buf;
		backupSharedMem[5] = backupLength;
		DC_FlushRange(&backupSharedMem, sizeof(backupSharedMem));
		requestFlag = 0;
		while(!sendBackupFifo(6));
		do OS_Sleep(100); while(!requestFlag);
		DC_InvalidateRange(buf, backupLength);
	}
	return ret;
}

void writeBackup() {
	const u8 * backupCtx = *backupCtxRef;
	int slot = backupCtx[480];
	const u8 * slotCtx = backupCtx + slot * 120;
	void * buf = backupGetBuffer(slotCtx + 12, 0);
	DC_FlushRange(buf, backupLength);
	backupSharedMem[3] = (int)buf;
	backupSharedMem[4] = slot << 12;
	backupSharedMem[5] = backupLength;
	DC_FlushRange(&backupSharedMem, sizeof(backupSharedMem));
	requestFlag = 0;
	while(!sendBackupFifo(7));
	do OS_Sleep(100); while(!requestFlag);
}