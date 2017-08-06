#include "nds/ndstypes.h"
#include "nds/dma.h"
#include "nds/arm9/video.h"
#include "common.h"
#include <dswifi9.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define BLOCK_SIZE 384
#define SCREENSHOT_WIDTH 256
#define SCREENSHOT_HEIGHT 192

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
	REQ_ASCII,
	REQ_CAPTURE,
	REQ_RETRY,
	REQ_CAPTURE_DONE,
	REQ_START_SEND
} PACKET_TYPE;

void retryBuffer(int sock, u8* buffer, int sz, struct sockaddr_in * sock_in, int addr_len, int index) {
	u8 tmp_buffer[16 + BLOCK_SIZE];
	memset(tmp_buffer, 0, 16 + BLOCK_SIZE);
	*(u32*)&tmp_buffer[0] = 0x89ABCDEF;
	*(u32*)&tmp_buffer[4] = 0x01234567;
	*(int*)&tmp_buffer[8] = index;
	memcpy(&tmp_buffer[16], buffer + index * BLOCK_SIZE, sz - BLOCK_SIZE * index < BLOCK_SIZE ? sz - BLOCK_SIZE * index : BLOCK_SIZE);
	sendto(sock, tmp_buffer, sizeof(tmp_buffer), 0, (const struct sockaddr*)sock_in, addr_len);
}

void sendBuffer(int sock, u8* buffer, int sz, struct sockaddr_in * sock_in, int addr_len) {
	u8 tmp_buffer[16 + BLOCK_SIZE];
	memset(tmp_buffer, 0, 16 + BLOCK_SIZE);
	*(u32*)&tmp_buffer[0] = 0x89ABCDEF;
	*(u32*)&tmp_buffer[4] = 0x01234567;
	int packetCount = (sz + BLOCK_SIZE - 1) / BLOCK_SIZE;
	for (int i = 0; i < packetCount; i++) {
		*(int*)&tmp_buffer[8] = i;
		memcpy(&tmp_buffer[16], buffer + i * BLOCK_SIZE, sz - BLOCK_SIZE * i < BLOCK_SIZE ? sz - BLOCK_SIZE * i : BLOCK_SIZE);
		sendto(sock, tmp_buffer, sizeof(tmp_buffer), 0, (const struct sockaddr*)sock_in, addr_len);
		OS_Sleep(10);
		if (i != 0 && ((i & 0xFF) == 0))
			OS_Sleep(1000);
	}
}

void screenshot(u8* buffer) {
	u8* vram_temp=(u8*)FndAllocFromExpHeapEx(*heapHeaderRef, 128*1024, -0x10);
	
	if (vram_temp) {
	
		u8 vram_cr_temp=VRAM_D_CR;
		VRAM_D_CR=VRAM_D_LCD | 0x80;
	
		dmaCopy(VRAM_D, vram_temp, 128*1024);

		REG_DISPCAPCNT=DCAP_BANK(3)|DCAP_ENABLE|DCAP_SIZE(3);
		while(REG_DISPCAPCNT & DCAP_ENABLE);

		dmaCopy(VRAM_D, buffer, 256*192*2);
		dmaCopy(vram_temp, VRAM_D, 128*1024);
	
		VRAM_D_CR=vram_cr_temp;
	
		free(vram_temp);
	}
}

void HandlePacket(void * arg) {
	int sock = *(int*)arg;
	int curSeqNo, addr_len, dumpSize, index;
	u8 * snapshot,* offset;
	TPPacket packet;
	struct sockaddr_in sock_in;
	
	curSeqNo = -1;
	addr_len = dumpSize = 0;
	snapshot = offset = (u8*)0;
	
	memset(&packet, 0, sizeof(TPPacket));
	memset(&sock_in, 0, sizeof(struct sockaddr_in));
	
	while(1) {
		if (recvfrom(sock, &packet, sizeof(TPPacket), 0, (struct sockaddr*)&sock_in, &addr_len) >= 0) {
			if (packet.seqNo != curSeqNo)
			{
				switch(packet.type) {
					case REQ_TOUCHPANEL:
						if (TouchState == 0) {
							*(u8*)0x20CD31F = 0;
							TouchXY = *(int*)&packet.data[0];
							TouchState = 1;
						}
						curSeqNo = packet.seqNo;
						packet.type = REQ_ACK;
						sendto(sock, &packet, sizeof(TPPacket), 0, (const struct sockaddr*)&sock_in, addr_len);
						break;
					case REQ_ASCII:
						if (inputState == 0) {
							*(u8*)0x20CD31F = 1;
							inputCharacter = *(u16*)&packet.data[0];
							inputState = 1;
						}
						curSeqNo = packet.seqNo;
						packet.type = REQ_ACK;
						sendto(sock, &packet, sizeof(TPPacket), 0, (const struct sockaddr*)&sock_in, addr_len);
						break;
					case REQ_CAPTURE:
						if (snapshot) free(snapshot);
						dumpSize = SCREENSHOT_WIDTH * SCREENSHOT_HEIGHT * 2;
						snapshot = (u8*)FndAllocFromExpHeapEx(*heapHeaderRef, dumpSize, -0x10);
						if (snapshot) {
							screenshot(snapshot);
							packet.type = REQ_ACK;
							*(int*)&packet.data[0] = dumpSize;
							*(int*)&packet.data[4] = dumpSize / BLOCK_SIZE;
							*(int*)&packet.data[8] = BLOCK_SIZE;
							sendto(sock, &packet, sizeof(TPPacket), 0, (const struct sockaddr*)&sock_in, addr_len);
						}
						else {
							packet.type = REQ_ACK;
							memset(&packet.data, 0, sizeof(packet.data));
							sendto(sock, &packet, sizeof(TPPacket), 0, (const struct sockaddr*)&sock_in, addr_len);
						}
						curSeqNo = packet.seqNo;
						break;
					case REQ_START_SEND:
						if (snapshot && dumpSize) {
							sendBuffer(sock, snapshot, dumpSize, &sock_in, addr_len);
						}
						break;
					case REQ_RETRY:
						if (snapshot && dumpSize) {
							index = *(int*)&packet.data[0];
							retryBuffer(sock, snapshot, dumpSize, &sock_in, addr_len, index);
						}
						break;
					case REQ_CAPTURE_DONE:
						if (snapshot) {
							free(snapshot);
							snapshot = (u8*)0;
						}
						if (dumpSize) dumpSize = 0;
						packet.type = REQ_ACK;
						sendto(sock, &packet, sizeof(TPPacket), 0, (const struct sockaddr*)&sock_in, addr_len);
						break;
				}
			}
			else {
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