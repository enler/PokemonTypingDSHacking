typedef void (*PXIFifoCallback) (int tag, u32 data, bool err);

int PXI_SendWordByFifo(int fifotag, u32 data, bool err);
void PXI_SetFifoRecvCallback(int fifotag, PXIFifoCallback callback);

void* backupGetBuffer(void* ctx, int a1);
void OS_Sleep(u32 msec);
void jmp_writeBackup();