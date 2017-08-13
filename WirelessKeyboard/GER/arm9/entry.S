#define STUB_FUNC_THUMB(name, _name)\
.global name;\
.type name, %function;\
.code 32;\
name:\
ldr pc, =_name;\

.section .text.start
.global _start;
.arm
_start:
push {lr}
ldr r0, =__bss_start
mov r1, #0
ldr r2, =_end
sub r2, r2, r0
bl memset
bl changePU
pop {lr}
adr r0, AP_Patch
ldr r1, =0x2000A00
sub r0, r0, r1
mvn r2, #0xFF000000
and r0,r2, r0, asr #2
orr r0, #0xEA000000
str r0, [r1, #-0x8]
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
STUB_FUNC_THUMB(foo, _foo)


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

.align 4
.arm
AP0:
ldr r0, value0
bx lr
value0:
.word 0xB3CF
AP1:
ldr r0, value1
bx lr
value1:
.word 0xC127

AP_Patch:
push {r0-r2, lr}
ldr r0,=0x6A0F72A8
ldr r1,=0x20F85AC
ldr r2, [r1]
cmp r2, r0
ldreq r0, patchValue
streq r0, [r1]
addeq r0, #0xC
streq r0, [r1, #0xAC]
pop {r0-r2, pc}

patchValue:
.word AP0 + 0x60000000