typedef struct _HookDataEntry {
	void * functionAddr;
	void * hookFunction;
	void ** origFunctionRef;
	u32 oldInstruction;
	u32 jumpInstruction;
} HookDataEntry;

void HookFunction(HookDataEntry * entry);