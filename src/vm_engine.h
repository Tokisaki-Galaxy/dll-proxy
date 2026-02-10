#include <windows.h>

// 随机化的指令码
#define OP_PUSH  0x71
#define OP_ADD   0x32
#define OP_XOR   0xA5
#define OP_STORE 0xE4
#define OP_HALT  0xFF

// 字节码：通过数学运算动态生成 "calc.exe"
static unsigned char vm_bytecode[] = {
	OP_PUSH, 0x30, OP_PUSH, 0x33, OP_ADD, OP_STORE,
	OP_PUSH, 0x30, OP_PUSH, 0x31, OP_ADD, OP_STORE,
	OP_PUSH, 0x36, OP_PUSH, 0x36, OP_ADD, OP_STORE,
	OP_PUSH, 0x30, OP_PUSH, 0x33, OP_ADD, OP_STORE,
	OP_PUSH, 0x17, OP_PUSH, 0x17, OP_ADD, OP_STORE,
	OP_PUSH, 0x32, OP_PUSH, 0x33, OP_ADD, OP_STORE,
	OP_PUSH, 0x3C, OP_PUSH, 0x3C, OP_ADD, OP_STORE,
	OP_PUSH, 0x32, OP_PUSH, 0x33, OP_ADD, OP_STORE,
	OP_HALT
};

// 解释器
static void ExecuteVM(WCHAR* output) {
	int stack[16];
	int sp = 0;
	int ip = 0;
	int dp = 0;

	while (1) {
		unsigned char opcode = vm_bytecode[ip++];
		if (opcode == OP_HALT) break;

		switch (opcode) {
			case OP_PUSH:
				stack[sp++] = vm_bytecode[ip++];
				break;
			case OP_ADD:
				stack[sp - 2] = stack[sp - 2] + stack[sp - 1];
				sp--;
				break;
			case OP_XOR:
				stack[sp - 2] = stack[sp - 2] ^ stack[sp - 1];
				sp--;
				break;
			case OP_STORE:
				output[dp++] = (WCHAR)stack[--sp];
				break;
		}
	}
	output[dp] = L'\0';
}
