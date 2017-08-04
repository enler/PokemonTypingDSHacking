//file interface

typedef struct _FSFile {
	char data[0x3c];
} FSFile;


void FS_InitFile(FSFile *p_file);
bool FS_OpenFile(FSFile *p_file, const char *path);

static inline u32 FS_GetLength(const FSFile *p_file)
{
    return *(u32*)&p_file->data[0x28] - *(u32*)&p_file->data[0x24];
}

s32 FS_ReadFile(FSFile *p_file, void *dst, s32 len);
bool FS_CloseFile(FSFile *p_file);

//cache interface

void DC_InvalidateRange(void *startAddr, u32 nBytes);
void DC_StoreRange(const void *startAddr, u32 nBytes);
void DC_FlushRange(const void *startAddr, u32 nBytes);

//memory interface

void* FndAllocFromExpHeapEx(void* heap, u32 size, s32 align);
void FndFreeToExpHeap(void* heap, void* ptr);

//pxi interface
typedef void (*PXIFifoCallback) (int tag, u32 data, bool err);

int PXI_SendWordByFifo(int fifotag, u32 data, bool err);
bool    PXI_IsCallbackReady(int fifotag, int proc);
void PXI_SetFifoRecvCallback(int fifotag, PXIFifoCallback callback);

//thread interface
typedef struct _OSThread {
	u8 context[512];
} OSThread;

void OS_CreateThread(OSThread *thread, void (*func) (void *), void *arg, void *stack, u32 stackSize, u32 prio);
void OS_WakeupThreadDirect(OSThread *thread);
void OS_ExitThread(void);
void OS_Sleep(u32 msec);

void initArm7Payload();

// backup
void jmp_writeBackup();
void initBackup();
void* backupGetBuffer(void* ctx, int a1);

extern void ** heapHeaderRef;
extern u32 heapTop;

extern u32 PxiRtcCallback7;

extern const u8** backupCtxRef;
extern const int readBackupHookOffset;
extern const int writeBackupHookOffset;

extern const int handleTouchDataOffset;
extern const int GetInputOffset;

#define MAKE_BRANCH(src, dest) (((((s32)(dest) - 8 - (s32)(src)) >> 2) & 0xFFFFFF) | 0xEA000000);