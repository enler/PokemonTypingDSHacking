#define STUB_FUNC_THUMB(name, _name)\
.global name;\
.type name, %function;\
.code 32;\
name:\
ldr pc, =_name;\


.section .text.start
.global _start;
_start:
.arm
push {lr}
ldr r0, =__bss_start
mov r1, #0
ldr r2, =_end
sub r2, r2, r0
bl memset
bl changePU
pop {lr}
b	main

.global changePU;
.type changePU, %function;
.arm
changePU:
ldr r0, =0x240002B
mcr p15, 0, r0, c6, c2, 0
mrc p15, 0, r0, c5, c0, 2
bic r0, r0, #0xF00
orr r0, r0, #0x300
mcr p15, 0, r0, c5, c0, 2
mrc p15, 0, r0, c5, c0, 3
bic r0, r0, #0xF00
orr r0, r0, #0x300
mcr p15, 0, r0, c5, c0, 3
bx lr

STUB_FUNC_THUMB(FS_InitFile, _FS_InitFile)
STUB_FUNC_THUMB(FS_OpenFile, _FS_OpenFile)
STUB_FUNC_THUMB(FS_ReadFile, _FS_ReadFile)
STUB_FUNC_THUMB(FS_CloseFile, _FS_CloseFile)
STUB_FUNC_THUMB(OS_EnableIrqMask, _OS_EnableIrqMask)
STUB_FUNC_THUMB(OS_SetIrqFunction, _OS_SetIrqFunction)
STUB_FUNC_THUMB(PXI_SendWordByFifo, _PXI_SendWordByFifo)
STUB_FUNC_THUMB(PXI_SetFifoRecvCallback, _PXI_SetFifoRecvCallback)
STUB_FUNC_THUMB(PXI_IsCallbackReady, _PXI_IsCallbackReady)
STUB_FUNC_THUMB(OS_CreateThread, _OS_CreateThread)
STUB_FUNC_THUMB(OS_ExitThread, _OS_ExitThread)
STUB_FUNC_THUMB(OS_WakeupThreadDirect, _OS_WakeupThreadDirect)
STUB_FUNC_THUMB(OS_Sleep, _OS_Sleep)
STUB_FUNC_THUMB(FndAllocFromExpHeapEx, _FndAllocFromExpHeapEx)
STUB_FUNC_THUMB(FndFreeToExpHeap, _FndFreeToExpHeap)
STUB_FUNC_THUMB(backupGetBuffer, _backupGetBuffer)


.global orig_HandleTouchData;
.type orig_HandleTouchData, %function;
orig_HandleTouchData:
.arm
PUSH {R4-R7,LR}
SUB SP, SP, #0xC
MOVS R5, R0
MOVS R6, #0
ldr pc, =_orig_HandleTouchData_8

.global jmp_writeBackup
.type jmp_writeBackup, %function;
jmp_writeBackup:
.thumb
push {r3}
bl writeBackup
pop {pc}

