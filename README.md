# Cromekks Inspector

A lightweight, high-performance process diagnostics tool built in C++ utilizing Direct3D 11 and ImGui. This tool features a borderless, hardware-accelerated user interface designed to analyze virtual memory, active process modules, and system object handles in user space.

## Features

* **Clean Borderless UI:** Streamlined window interface with a custom draggable header context, completely bypassing standard operating system window styling.
* **Module & PE Parsing:** Real-time snapshotting of all mapped `.dll` and `.exe` binaries inside a target runtime, including automated portable executable header parsing to trace module entry points.
* **Memory Mapping:** Virtual memory space diagnostics that map out allocation states (`COMMIT`, `RESERVE`, `FREE`), memory types, and human-readable page protection access flags (`RWX`, `RX`, `R`).
* **System Handle Monitoring:** Basic architectural slots configured to display targeted resource path handles including system files, synchronization objects, and thread handles.

## Project Structure

* `main.cpp`: Initializes high-DPI scaling configurations, registers the transparent Win32 overlay interface window layer, boots the Direct3D 11 backend engine, and coordinates the primary user interface loop.
* `inspector.hpp`: Implements low-level Windows API interaction routines, handles target memory reading pipelines, and structures the primary process diagnostics logic.

## Technical Details

* **Graphics Backend:** Direct3D 11 Hardware Rendering
* **UI Framework:** Dear ImGui (Win32 & DX11 implementations)
* **Target Architecture:** Windows x64 (Native Win32 Subsystem)
* **SDK Requirements:** Windows SDK (DwmApi, TlHelp32, DXGI)

## Compilation & Building

To compile the executable locally via MSBuild or Microsoft Visual Studio:

1. Ensure the Microsoft Visual C++ compiler toolset is configured for x64 targets.
2. Link the following internal system library dependencies within your project properties or source file headers:
   * `d3d11.lib`
   * `dwmapi.lib`
3. Configure the source include paths to reference your local copy of the Dear ImGui library repository files.
4. Set compiler flag optimization configurations to C++17 or newer for structural binding compatibility.

## Usage

1. Launch `Cromekks Inspector` with administrative execution rights to ensure proper system process handle privileges.
2. Enter the target process numeric Identifier (`PID`) into the Target Control control box.
3. Click **Attach & Scan Suite** to let the tool process the operational modules and parse memory diagnostic tables.
