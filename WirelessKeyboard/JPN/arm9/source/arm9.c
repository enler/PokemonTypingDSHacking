#include "nds/ndstypes.h"
#include "common.h"

void orig_HandleTouchData(void*);

void* malloc(u32 size) {
	return FndAllocFromExpHeapEx(*heapHeaderRef, size, 0x10);
}

void free(void * ptr) {
	return FndFreeToExpHeap(*heapHeaderRef, ptr);
}

int TouchState = 0;
int TouchXY = 0;
u16 inputCharacter = 0;
int inputState = 0;

void hook_HandleTouchData(char* touchContext) {
	if (TouchState == 1) {
		*(int*)(touchContext + 0x30) = TouchXY;
		touchContext[0x34] = 1;
		touchContext[0x35] = 0;
		touchContext[0x36] = 1;
		TouchState = 2;
		return;
	}
	else if (TouchState == 2) {
		touchContext[0x34] = 0;
		touchContext[0x35] = 1;
		touchContext[0x36] = 0;
		TouchState = 0;
		return;
	}
	orig_HandleTouchData(touchContext);
}

u16 hook_GetInput(void * ctx) {
	if (inputState == 1) {
		inputState = 0;
		return inputCharacter;
	}
	return 0;
}

int main() {
	initArm7Payload();
	initBackup();
	*(u32*)handleTouchDataOffset = 0x46C04778;
	*(u32*)(handleTouchDataOffset + 4) = MAKE_BRANCH(handleTouchDataOffset + 4, hook_HandleTouchData);
	*(u32*)GetInputOffset = 0x46C04778;
	*(u32*)(GetInputOffset + 4) = MAKE_BRANCH(GetInputOffset + 4, hook_GetInput);
	startServer();
	return 0;
}