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

#if (defined EUR) || (defined FRA) || (defined ITA) || (defined SPA) || (defined GER)

#include "typingMap.h"

typedef struct _charKeyIDPair {
	u32 character:16;
	u32 keyID:16;
} charKeyIDPair;

u8 findKeyID(u16 character) {
	charKeyIDPair * dic = (charKeyIDPair*)&typingMap;
	int counter = sizeof(typingMap) / sizeof(charKeyIDPair);
	int max, min, mid;
	min = 0;
	max = counter - 1;
	while (min <= max) {
		mid = (min + max) / 2;
		if (dic[mid].character < character) min = mid + 1;
		else if (dic[mid].character > character) max = mid - 1;
		else return dic[mid].keyID;
	}
	return 0xFF;
}

#endif

u32 hook_GetInput(void * ctx) {
	if (inputState == 1) {
		inputState = 0;
#ifdef JPN
		return inputCharacter;
#endif

#if (defined EUR) || (defined FRA) || (defined ITA) || (defined SPA) || (defined GER)
		return (findKeyID(inputCharacter) << 16) | inputCharacter;
#endif
	}
	return 0;
}

int main() {
	initBackup();
	initArm7Payload();
#ifndef NO_NETWORKING
	*(u32*)handleTouchDataOffset = 0x46C04778;
	*(u32*)(handleTouchDataOffset + 4) = MAKE_BRANCH(handleTouchDataOffset + 4, hook_HandleTouchData);
	*(u32*)GetInputOffset = 0x46C04778;
	*(u32*)(GetInputOffset + 4) = MAKE_BRANCH(GetInputOffset + 4, hook_GetInput);
	startServer();
#endif
	return 0;
}