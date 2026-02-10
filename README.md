

<p align="center">
    <img src="https://img.shields.io/badge/Language-Japanese%2FEnglish%2FChinese-blue.svg" alt="Language">
    <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License">
    <img src="https://github.com/tokisaki-galaxy/dll-proxy/actions/workflows/build.yml/badge.svg" alt="Build Status">
    <img src="https://img.shields.io/badge/Platform-Windows%207%2F8%2F10%2F11-lightgrey.svg" alt="Platform">
    <img src="https://img.shields.io/badge/Type-DLL%20Proxy-orange.svg" alt="Type">
</p>

<p align="center">
    <a href="#dll-forwarder-design-document">
        <img src="https://img.shields.io/badge/Jump%20to-English%20Version-blue?style=for-the-badge" alt="Jump to English Version">
    </a>
</p>

---

# DLLフォワーダー設計書

**目的**: 純正のシステム関数への呼び出しを効率的に転送（フォワード）し、かつペイロードの隠密な起動ツールとして機能する、互換性の高い `version.dll` ハイジャック用動的リンクライブラリを開発すること。

---

## 1. プロジェクト概要 (Overview)

本プロジェクトの目的は、`version.dll` の「スケルトン（空の）」プロキシを作成することである。このDLLは、WindowsのDLL検索順序の仕様（DLLサイドローディング）を利用し、ホストプログラムによってロードされることを想定している。
本モジュールの中核となるタスクは以下の2点である。

1.  **シームレスな転送**: 標準的な17個のバージョンチェック関数への呼び出しを、透過的に正規の `C:\Windows\System32\version.dll` へと中継する。
2.  **サイレント・トリガー**: ロードされた瞬間に独立した検証用スレッドを立ち上げ、復号されたペイロード（電卓の起動）を実行する。

---

## 2. 機能要件 (Functional Requirements)

### 2.1 コア転送機能 (FR-Skeleton)
*   **要件**: x64アーキテクチャにおける `version.dll` の全17個の標準関数をエクスポートすること。
*   **実装方式**: **絶対パス転送 (Absolute Path Forwarding)** を採用する。
*   **エクスポート一覧**:
        *   `GetFileVersionInfoA`, `GetFileVersionInfoByHandle`, `GetFileVersionInfoExA`, `GetFileVersionInfoExW`, `GetFileVersionInfoSizeA`, `GetFileVersionInfoSizeExA`, `GetFileVersionInfoSizeExW`, `GetFileVersionInfoSizeW`, `GetFileVersionInfoW`, `VerFindFileA`, `VerFindFileW`, `VerInstallFileA`, `VerInstallFileW`, `VerLanguageNameA`, `VerLanguageNameW`, `VerQueryValueA`, `VerQueryValueW`。

### 2.2 文字列の難読化 (FR-Obfuscation)
*   **要件**: ターゲットとなる実行ファイル名 `"calc.exe"` を、平文（プレーンテキスト）の状態でデータセグメントに保持することを禁止する。
*   **実装方式**:
        *   **暗号化**: 単純な XOR アルゴリズムを使用する。
        *   **実行時復号**: `CreateProcess` が呼び出される直前にのみ、スタック上で復元を行うこと。
*   **難読化キー**: 任意の1バイトまたはマルチバイトのキー（例: `0xAB`）を使用する。

### 2.3 非同期実行エンジン (FR-Loader)
*   **要件**: ペイロードの実行によって `DllMain` をブロックしてはならない。これはホストプログラムのフリーズや、ローダーロック（Loader Lock）の発生を防ぐためである。
*   **実装方式**:
        1.  `DllMain` が `DLL_PROCESS_ATTACH` 通知を受け取る。
        2.  直ちに `std.Thread.spawn` または Win32 API の `CreateThread` を呼び出し、新規スレッドを作成する。
        3.  `DllMain` は速やかに `TRUE` を返す。

### 2.4 ペイロードロジック (FR-Payload)
*   **要件**: システムの電卓を起動すること。
*   **実装方式**:
        *   `CreateProcessW` を呼び出す。
        *   **パラメータ設定**:
                *   `lpApplicationName`: `null` に設定。
                *   `lpCommandLine`: 復号された `calc.exe` のパスを指定。
                *   `dwCreationFlags`: デフォルト値。
*   **安全性**: 復号された文字列は、使用後直ちに `SecureZeroMemory` を用いて消去すること。

---

## 3. 非機能要件 (Non-Functional Requirements)

### 3.1 互換性 (Compatibility)
*   **ターゲットシステム**: Windows 7, 8, 10, 11 (x64)。
*   **安定性**: 転送遅延は1ms未満とし、ホストプログラムが遅延を感知しないようにすること。

### 3.2 隠密性 (Stealth)
*   **サイズ**: 最終的な成果物である `version.dll` のファイルサイズは 50KB 以内に抑えること。
*   **シンボルの削除**: デバッグシンボルはすべて削除（ストリップ）すること。
*   **リソース偽装**: バージョン情報リソース（VersionInfo）を埋め込み、オリジナルの `version.dll` の記述（"Microsoft Version Checking Library"）を複製すること。

---
---

# DLL Forwarder Design Document

**Objective**: To develop a highly compatible `version.dll` hijacking dynamic link library that efficiently forwards calls to the original system functions and serves as a covert launcher for payloads.

---

## 1. Project Overview

The aim of this project is to create a "skeleton" proxy for `version.dll`. This DLL is intended to be loaded by a host program utilizing the Windows DLL Search Order vulnerability (Sideloading).
The core tasks of this module are the following two points:

1.  **Seamless Forwarding**: Transparently relay calls for the 17 standard version check functions to the legitimate `C:\Windows\System32\version.dll`.
2.  **Silent Trigger**: Launch an independent verification thread at the moment of loading to execute the decrypted payload (launching Calculator).

---

## 2. Functional Requirements

### 2.1 Core Forwarding Function (FR-Skeleton)
*   **Requirement**: Export all 17 standard functions of `version.dll` for x64 architecture.
*   **Implementation Method**: Adopt **Absolute Path Forwarding**.
*   **Export List**:
    *   `GetFileVersionInfoA`, `GetFileVersionInfoByHandle`, `GetFileVersionInfoExA`, `GetFileVersionInfoExW`, `GetFileVersionInfoSizeA`, `GetFileVersionInfoSizeExA`, `GetFileVersionInfoSizeExW`, `GetFileVersionInfoSizeW`, `GetFileVersionInfoW`, `VerFindFileA`, `VerFindFileW`, `VerInstallFileA`, `VerInstallFileW`, `VerLanguageNameA`, `VerLanguageNameW`, `VerQueryValueA`, `VerQueryValueW`.

### 2.2 String Obfuscation (FR-Obfuscation)
*   **Requirement**: It is prohibited to hold the target executable name `"calc.exe"` in the data segment in plain text.
*   **Implementation Method**:
    *   **Encryption**: Use a simple XOR algorithm.
    *   **Runtime Decryption**: Restore on the stack only immediately before `CreateProcess` is called.
*   **Obfuscation Key**: Use an arbitrary single-byte or multi-byte key (e.g., `0xAB`).

### 2.3 Asynchronous Execution Engine (FR-Loader)
*   **Requirement**: The execution of the payload must not block `DllMain`. This is to prevent the host program from freezing or triggering a Loader Lock.
*   **Implementation Method**:
    1.  `DllMain` receives the `DLL_PROCESS_ATTACH` notification.
    2.  Immediately call `std.Thread.spawn` or the Win32 API `CreateThread` to create a new thread.
    3.  `DllMain` returns `TRUE` promptly.

### 2.4 Payload Logic (FR-Payload)
*   **Requirement**: Launch the system calculator.
*   **Implementation Method**:
    *   Call `CreateProcessW`.
    *   **Parameter Configuration**:
        *   `lpApplicationName`: Set to `null`.
        *   `lpCommandLine`: Specify the path of the decrypted `calc.exe`.
        *   `dwCreationFlags`: Default value.
*   **Safety**: The decrypted string must be erased using `SecureZeroMemory` immediately after use.

---

## 3. Non-Functional Requirements

### 3.1 Compatibility
*   **Target Systems**: Windows 7, 8, 10, 11 (x64).
*   **Stability**: Forwarding latency shall be less than 1ms to ensure the host program does not perceive any delay.

### 3.2 Stealth
*   **Size**: The file size of the final artifact `version.dll` should be kept within 50KB.
*   **Symbol Stripping**: All debug symbols must be stripped.
*   **Resource Masquerading**: Embed VersionInfo resources and clone the description of the original `version.dll` ("Microsoft Version Checking Library").
