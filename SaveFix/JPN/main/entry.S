#define STUB_FUNC_THUMB(name, _name)\
.global name;\
.type name, %function;\
.code 32;\
name:\
ldr pc, =_name;\


.section .text.start
.global _start;
_start:
.thumb
push {lr}
ldr r0, =__bss_start
mov r1, #0
ldr r2, =_end
sub r2, r2, r0
blx memset
# blx changePU
bl main
pop {pc}

/*
.global changePU;
.type changePU, %function;
.code 32;
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
*/

STUB_FUNC_THUMB(PXI_SendWordByFifo, _PXI_SendWordByFifo)
STUB_FUNC_THUMB(PXI_SetFifoRecvCallback, _PXI_SetFifoRecvCallback)
STUB_FUNC_THUMB(OS_Sleep, _OS_Sleep)
STUB_FUNC_THUMB(backupGetBuffer, _backupGetBuffer)

.global jmp_writeBackup
.type jmp_writeBackup, %function;
jmp_writeBackup:
.thumb
push {r3}
bl writeBackup
pop {pc}

