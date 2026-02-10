# 工程需求文档：version.dll 转发器

**版本**: 1.0
**目标**: 开发一个高度兼容的 `version.dll` 劫持动态链接库，实现对原版系统函数的高效转发，并作为 Payload 载荷的隐蔽启动器。

---

## 1. 项目概述 (Overview)

本项目旨在创建一个 `version.dll` 的“空壳”代理。该 DLL 将利用 Windows 的 DLL 搜索顺序漏洞（Sideloading）被宿主程序加载。
其核心任务有两个：
1.  **无缝转发**: 将 17 个标准的版本检查函数调用透明地传递给真实的 `C:\Windows\System32\version.dll`。
2.  **静默触发**: 在加载瞬间启动一个独立的验证线程，执行解密后的 Payload（弹出计算器）。

---

## 2. 功能性需求 (Functional Requirements)

### 2.1 核心转发 (FR-Skeleton)
*   **需求**: 必须导出 `version.dll` 在 x64 架构下的所有 17 个标准函数。
*   **实现方式**: 采用 **绝对路径转发 (Absolute Path Forwarding)**。
*   **导出清单**:
    *   `GetFileVersionInfoA`, `GetFileVersionInfoByHandle`, `GetFileVersionInfoExA`, `GetFileVersionInfoExW`, `GetFileVersionInfoSizeA`, `GetFileVersionInfoSizeExA`, `GetFileVersionInfoSizeExW`, `GetFileVersionInfoSizeW`, `GetFileVersionInfoW`, `VerFindFileA`, `VerFindFileW`, `VerInstallFileA`, `VerInstallFileW`, `VerLanguageNameA`, `VerLanguageNameW`, `VerQueryValueA`, `VerQueryValueW`。

### 2.2 字符串混淆 (FR-Obfuscation)
*   **需求**: 目标执行程序名 `"calc.exe"` 禁止以明文形式存储在数据段中。
*   **实现方式**:
    *   **加密**: 使用简单的 XOR 算法。
    *   **运行时解密**: 仅在 `CreateProcess` 调用前的瞬间在栈上完成还原。
*   **混淆 Key**: 自定义单字节或多字节 Key（例如 `0xAB`）。

### 2.3 异步执行引擎 (FR-Loader)
*   **需求**: Payload 的执行不得阻塞 `DllMain`，防止宿主程序卡死或触发 Loader Lock。
*   **实现方式**:
    1.  在 `DllMain` 接收到 `DLL_PROCESS_ATTACH` 通知时。
    2.  立即调用 `std.Thread.spawn` 或 Win32 `CreateThread` 创建新线程。
    3.  `DllMain` 迅速返回 `TRUE`。

### 2.4 Payload 逻辑 (FR-Payload)
*   **需求**: 启动系统计算器。
*   **实现方式**:
    *   调用 `CreateProcessW`。
    *   **参数配置**: 
        *   `lpApplicationName`: 设为 `null`。
        *   `lpCommandLine`: 指向解密后的 `calc.exe` 路径。
        *   `dwCreationFlags`: 默认值。
*   **安全性**: 确保解密后的字符串在使用后立即通过 `SecureZeroMemory` 擦除。

---

## 3. 非功能性需求 (Non-Functional Requirements)

### 3.1 兼容性 (Compatibility)
*   **目标系统**: Windows 7, 8, 10, 11 (x64)。
*   **稳定性**: 转发延迟应低于 1ms，确保宿主无感知。

### 3.2 隐蔽性 (Stealth)
*   **体积**: 最终产物 `version.dll` 大小应控制在 50KB 以内。
*   **符号剥离**: 必须剥离所有调试符号。
*   **资源伪装**: 嵌入版本信息资源（VersionInfo），克隆原版 `version.dll` 的描述（"Microsoft Version Checking Library"）。

---

## 4. 测试建议 (Testing)

1.  **静态检查**: 使用 `Strings.exe` 验证是否存在 "calc.exe" 字符串。
2.  **功能检查**: 将 DLL 放入测试程序目录，验证是否弹出计算器且程序功能正常。
3.  **兼容性检查**: 在不同的虚拟机环境下（Win7 vs Win11）验证转发是否有效。
