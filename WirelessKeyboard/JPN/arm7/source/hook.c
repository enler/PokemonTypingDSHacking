#include "nds/ndstypes.h"
#include "arm7.h"
#include "hook.h"

#define MAKE_BRANCH(src, dest) (((((s32)(dest) - 8 - (s32)(src)) >> 2) & 0xFFFFFF) | 0xEA000000);

void HookFunction(HookDataEntry * entry) {
	entry->oldInstruction = *(u32*)entry->functionAddr;
	*(u32*)entry->functionAddr = MAKE_BRANCH(entry->functionAddr, entry->hookFunction);
	entry->jumpInstruction = MAKE_BRANCH(&entry->jumpInstruction, (u32)entry->functionAddr + 4);
	if (entry->origFunctionRef)
		*entry->origFunctionRef = (void*)&entry->oldInstruction;
}