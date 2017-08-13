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
#define DATA_BODY_SIZE 384
#define SCREENSHOT_WIDTH 256
#define SCREENSHOT_HEIGHT 192

typedef enum _TP_PACKET_TYPE {
	TYPE_CMD = 0x43415054, //'TPAC'
	TYPE_DATA = 0x44415054 //'TPAD'
} TP_PACKET_TYPE;

typedef struct _TPPacketHeader{
	TP_PACKET_TYPE type;
	int seqNo;
	union _common {
		int cmd;
		int index;
	} common;
	int bodySize;
} TPPacketHeader;

typedef struct _TPPacketCMD {
	TPPacketHeader header;
	union _data {
		u32 req[4];
		u32 resp[4];
	} data;
} TPPacketCMD;

typedef enum _PACKET_TYPE {
	REQ_TOUCHPANEL,
	REQ_ACK,
	REQ_ASCII,
	REQ_CAPTURE,
	REQ_RETRY,
	REQ_CAPTURE_DONE,
	REQ_START_SEND
} PACKET_TYPE;



static inline void sendBufferByIndex(int sock, u8* buffer, int sz, struct sockaddr_in * sock_in, int addr_len, TPPacketHeader * header) {
	int index = header->common.index;
	u8 tmp_buffer[sizeof(TPPacketHeader) + DATA_BODY_SIZE];
	memcpy(&tmp_buffer, header, sizeof(TPPacketHeader));
	u8 * body = tmp_buffer + sizeof(TPPacketHeader);
	memcpy(body, buffer + index * DATA_BODY_SIZE, sz - DATA_BODY_SIZE * index < DATA_BODY_SIZE ? sz - DATA_BODY_SIZE * index : DATA_BODY_SIZE);
	sendto(sock, tmp_buffer, sizeof(tmp_buffer), 0, (const struct sockaddr*)sock_in, addr_len);
}

void sendBuffer(int sock, u8* buffer, int sz, struct sockaddr_in * sock_in, int addr_len, int seqNo) {
	TPPacketHeader header;
	header.type = TYPE_DATA;
	header.seqNo = seqNo;
	header.bodySize = DATA_BODY_SIZE;
	int packetCount = (sz + DATA_BODY_SIZE - 1) / DATA_BODY_SIZE;
	for (int i = 0; i < packetCount; i++) {
		header.common.index = i;
		sendBufferByIndex(sock, buffer, sz, sock_in, addr_len, &header);
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
	int sock, curSeqNo, addr_len, dumpSize, index;
	u8 * snapshot,* offset;
	TPPacketCMD packet;
	struct sockaddr_in serv_addr, sock_in;
	
	if (!Wifi_InitDefault(1)) OS_ExitThread();
	
	sock = socket(AF_INET ,SOCK_DGRAM ,0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(818);
	
	if (sock >= 0 && bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
		curSeqNo = -1;
		addr_len = dumpSize = 0;
		snapshot = offset = (u8*)0;
	
		memset(&packet, 0, sizeof(TPPacketCMD));
		memset(&sock_in, 0, sizeof(struct sockaddr_in));
	
		while(1) {
			if (recvfrom(sock, &packet, sizeof(TPPacketCMD), 0, (struct sockaddr*)&sock_in, &addr_len) >= 0) {
				if (packet.header.type != TYPE_CMD || packet.header.bodySize != sizeof(packet.data)) continue;//invalid packet
				if (packet.header.seqNo != curSeqNo) {
					curSeqNo = packet.header.seqNo;
					switch(packet.header.common.cmd) {
					case REQ_TOUCHPANEL:
						if (TouchState == 0) {
							*wirelessKeyboardEnableFlag = 0;
							TouchXY = (int)packet.data.req[0];
							TouchState = 1;
						}
						packet.header.common.cmd = REQ_ACK;
						sendto(sock, &packet, sizeof(TPPacketCMD), 0, (const struct sockaddr*)&sock_in, addr_len);
						break;
					case REQ_ASCII:
						if (inputState == 0) {
							*wirelessKeyboardEnableFlag = 1;
							inputCharacter = (u16)packet.data.req[0];
							inputState = 1;
						}
						packet.header.common.cmd = REQ_ACK;
						sendto(sock, &packet, sizeof(TPPacketCMD), 0, (const struct sockaddr*)&sock_in, addr_len);
						break;
					case REQ_CAPTURE:
						if (snapshot) free(snapshot);
						dumpSize = SCREENSHOT_WIDTH * SCREENSHOT_HEIGHT * 2;
						snapshot = (u8*)FndAllocFromExpHeapEx(*heapHeaderRef, dumpSize, -0x10);
						if (snapshot) {
							screenshot(snapshot);
							packet.header.common.cmd = REQ_ACK;
							packet.data.resp[0] = dumpSize;
							packet.data.resp[1] = dumpSize / BLOCK_SIZE;
							packet.data.resp[2] = BLOCK_SIZE;
							sendto(sock, &packet, sizeof(TPPacketCMD), 0, (const struct sockaddr*)&sock_in, addr_len);
						}
						else {
							packet.header.common.cmd = REQ_ACK;
							memset(&packet.data, 0, sizeof(packet.data));
							sendto(sock, &packet, sizeof(TPPacketCMD), 0, (const struct sockaddr*)&sock_in, addr_len);
						}
						break;
					case REQ_START_SEND:
						if (snapshot && dumpSize) {
							sendBuffer(sock, snapshot, dumpSize, &sock_in, addr_len, curSeqNo);
						}
						break;
					case REQ_RETRY:
						if (snapshot && dumpSize) {
							index = (int)packet.data.req[0];
							packet.header.type = TYPE_DATA;
							packet.header.common.index = index;
							packet.header.bodySize = DATA_BODY_SIZE;
							sendBufferByIndex(sock, snapshot, dumpSize, &sock_in, addr_len, &packet.header);
						}
						break;
					case REQ_CAPTURE_DONE:
						if (snapshot) {
							free(snapshot);
							snapshot = (u8*)0;
						}
						if (dumpSize) dumpSize = 0;
						packet.header.common.cmd = REQ_ACK;
						sendto(sock, &packet, sizeof(TPPacketCMD), 0, (const struct sockaddr*)&sock_in, addr_len);
						break;
					}
				}
				else {
					packet.header.common.cmd = REQ_ACK;//only send ack packet
					sendto(sock, &packet, sizeof(TPPacketCMD), 0, (const struct sockaddr*)&sock_in, addr_len);
				}
			}
		}
	}
	OS_ExitThread();
}

void startServer() {
	static OSThread backboardThread;
	static u8 stack[0x1000];
	OS_CreateThread(&backboardThread, HandlePacket, (void*)0, stack + 0x1000, 0x1000, 8);
	OS_WakeupThreadDirect(&backboardThread);
}