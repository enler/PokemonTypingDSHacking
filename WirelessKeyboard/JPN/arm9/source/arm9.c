#include "nds/ndstypes.h"
#include "common.h"
#include <dswifi9.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

void orig_HandleTouchData(void*);

typedef struct _TPPacket {
	int ver;
	int type;
	int seqNo;
	int reserved;
	u8 data[16];
} TPPacket;

typedef enum _PACKET_TYPE {
	REQ_TOUCHPANEL,
	REQ_ACK,
	REQ_ASCII
} PACKET_TYPE;

void* malloc(u32 size) {
	return FndAllocFromExpHeapEx(*heapHeaderRef, size, 0x10);
}

void free(void * ptr) {
	return FndFreeToExpHeap(*heapHeaderRef, ptr);
}

int TouchState = 0;
int TouchXY = 0;

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

u16 inputCharacter = 0;
int inputState = 0;

u16 hook_GetInput(void * ctx) {
	if (inputState == 1) {
		inputState = 0;
		return inputCharacter;
	}
	return 0;
}

void HandlePacket(void * arg) {
	int sock = *(int*)arg;
	*(u32*)handleTouchDataOffset = 0x46C04778;
	*(u32*)(handleTouchDataOffset + 4) = MAKE_BRANCH(handleTouchDataOffset + 4, hook_HandleTouchData);
	*(u32*)GetInputOffset = 0x46C04778;
	*(u32*)(GetInputOffset + 4) = MAKE_BRANCH(GetInputOffset + 4, hook_GetInput);
	int curSeqNo, addr_len;
	curSeqNo = -1;
	TPPacket packet;
	struct sockaddr_in sock_in;
	memset(&packet, 0, sizeof(TPPacket));
	memset(&sock_in, 0, sizeof(struct sockaddr_in));
	while(1) {
		if (recvfrom(sock, &packet, sizeof(TPPacket), 0, (struct sockaddr*)&sock_in, &addr_len) >= 0) {
			if (packet.seqNo != curSeqNo && REQ_TOUCHPANEL == packet.type) {
				if (TouchState == 0) {
					*(u8*)0x20CD31F = 0;
					TouchXY = *(int*)&packet.data[0];
					TouchState = 1;
				}
				curSeqNo = packet.seqNo;
				packet.type = REQ_ACK;
				sendto(sock, &packet, sizeof(TPPacket), 0, (const struct sockaddr*)&sock_in, addr_len);
			}
			else if (packet.seqNo != curSeqNo && REQ_ASCII == packet.type) {
				if (inputState == 0) {
					*(u8*)0x20CD31F = 1;
					inputCharacter = *(u16*)&packet.data[0];
					inputState = 1;
				}
				curSeqNo = packet.seqNo;
				packet.type = REQ_ACK;
				sendto(sock, &packet, sizeof(TPPacket), 0, (const struct sockaddr*)&sock_in, addr_len);
			}
			else if (packet.seqNo == curSeqNo) {
				packet.type = REQ_ACK;//only send ack packet
				sendto(sock, &packet, sizeof(TPPacket), 0, (const struct sockaddr*)&sock_in, addr_len);
			}
		}
	}
	OS_ExitThread();
}

void startServer() {
	if (Wifi_InitDefault(1)) {
		int sock = socket(AF_INET ,SOCK_DGRAM ,0);
		if (sock >= 0) {
			static struct sockaddr_in serv_addr;
			static OSThread backboardThread;
			memset(&serv_addr, 0, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_addr.s_addr = INADDR_ANY;
			serv_addr.sin_port = htons(818);
			if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
				static u8 stack[0x1000];
				static int args[1];
				args[0] = sock;
				OS_CreateThread(&backboardThread, HandlePacket, (void*)&args, stack + 0x1000, 0x1000, 8);
				OS_WakeupThreadDirect(&backboardThread);
			}
		}
	}
	else while(1);
}

int main() {
	initArm7Payload();
	initBackup();
	startServer();
	return 0;
}