.thumb
expandMemory:
ldr r0, heapTopRef
ldrh r1, [r0, #2]
add r1, #1
strh r1, [r0, #2]
ldr r0, =0x0205b2e0
ldr r1, =0x2100
strh r1, [r0]
bx lr

loadPayload:
push {r4-r7, lr}
sub sp, #0x3C
bl 0x20010A0
mov r0, sp
bl 0x2094F24
ldr r1, =0x20F2D64
ldr r2, =11120
bl 0x2094FE0
ldr r1, [sp, #0x24]
ldr r2, [sp, #0x28]
sub r2, r1
ldr r1, payloadOffset
mov r0, sp
bl 0x209510C
mov r0, sp
bl 0x209505C
blx 0x209A204
ldr r0, payloadOffset
blx r0
add sp, #0x3C
pop {r4-r7, pc}

.align 2

heapTopRef:
.word 0x209A524

payloadOffset:
.word 0x2130400