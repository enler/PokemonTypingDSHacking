.section .text.start
.global _start;
_start:
push {r0-r12, lr}
ldr r0, =__bss_start
mov r1, #0
ldr r2, =_end
loop:
cmp r0, r2
bge entry
strb r1, [r0], #1
b loop
entry:
bl	main
pop {r0-r12, lr}
ldr r12, =RtcPxiCallback
add r12, r12, #1
bx r12

.global PXI_SendWordByFifo
.type PXI_SendWordByFifo, %function;
PXI_SendWordByFifo:
ldr r12, =_PXI_SendWordByFifo
bx r12

.global PXI_SetFifoRecvCallback
.type PXI_SetFifoRecvCallback, %function;
PXI_SetFifoRecvCallback:
ldr r12, =_PXI_SetFifoRecvCallback
bx r12

