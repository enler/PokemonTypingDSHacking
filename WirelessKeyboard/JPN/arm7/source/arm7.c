#include "nds/ndstypes.h"
#include "arm7.h"
#include "hook.h"
#include <dswifi7.h>

void (*orig_VBlank)() = (void*)0;

void hook_VBlank() {
	if (orig_VBlank) orig_VBlank();
	Wifi_Update();
}

int main()
{
	static HookDataEntry entry = {(void*)0, hook_VBlank, (void**)&orig_VBlank};
	entry.functionAddr = OSi_IrqVBlank;
	HookFunction(&entry);
	u32* restoreOffset = (u32*)RtcPxiCallback;
	*restoreOffset = 0x2A00B510;
	*(restoreOffset + 1) = 0x207FD122;
	installWifiFIFO();
}