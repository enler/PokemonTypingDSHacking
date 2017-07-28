.thumb
expandMemory:
ldr r0, heapTopHandler
ldr r1, [r0]
mov r2, #0x40
lsl r2, #4
add r1, r2
str r1, [r0]
bx lr

loadPayload:
push {r4-r7, lr}
sub sp, #0x3C
bl 0x20010A0
mov r0, sp
bl 0x2096880
ldr r1, =0x20F77E4
ldr r2, =11138
bl 0x209693C
ldr r1, [sp, #0x24]
ldr r2, [sp, #0x28]
sub r2, r1
ldr r1, payloadOffset
mov r0, sp
bl 0x2096A68
mov r0, sp
bl 0x20969B8
ldr r0, payloadOffset
add r0, #1
blx r0
add sp, #0x3C
pop {r4-r7, pc}

.align 2

heapTopHandler:
.word 0x209BE80

payloadOffset:
.word 0x2130C20