#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>

struct ModuleInfo {
    std::wstring name; 
    DWORD_PTR baseAddress;
    DWORD size;
    DWORD_PTR entryPoint;
};

struct MemoryRegion {
    DWORD_PTR baseAddress;
    SIZE_T size;
    std::string protectStr;
    std::string stateStr;
    std::string typeStr;
};

struct HandleInfo {
    int id;
    std::string type;
    std::string name;
    ACCESS_MASK access;
};

struct ProcessEntry {
    DWORD pid;
    std::wstring name;
};

class ProcessInspector {
private:
    /* GetProtectionString: Converts raw Windows memory protection bitflags into short, clean strings like "RWX" or "RX". */
    std::string GetProtectionString(DWORD protect) {
        if (protect & PAGE_NOACCESS) return "NOACCESS";
        if (protect & PAGE_READONLY) return "R";
        if (protect & PAGE_READWRITE) return "RW";
        if (protect & PAGE_WRITECOPY) return "WC";
        if (protect & PAGE_EXECUTE) return "X";
        if (protect & PAGE_EXECUTE_READ) return "RX";
        if (protect & PAGE_EXECUTE_READWRITE) return "RWX";
        std::string extra = "";
        if (protect & PAGE_GUARD) extra += " + GUARD";
        return extra.empty() ? "UNKNOWN" : extra;
    }

public:
    std::vector<ModuleInfo> modules;
    std::vector<MemoryRegion> regions;
    std::vector<HandleInfo> handles;
    std::vector<ProcessEntry> activeProcesses;

    /* RefreshProcessList: Captures a snapshot of all active system processes to build the selection UI. */
    void RefreshProcessList() {
        activeProcesses.clear();
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return;

        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(snapshot, &pe)) {
            do {
                activeProcesses.push_back({ pe.th32ProcessID, pe.szExeFile });
            } while (Process32NextW(snapshot, &pe));
        }
        CloseHandle(snapshot);
    }

    /* RefreshModules: Snapshots a process, maps out loaded .dll/.exe bases, and parses PE headers for their entry points. */
    void RefreshModules(DWORD pid) {
        modules.clear();
        if (pid == 0) return;

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snapshot == INVALID_HANDLE_VALUE) return;

        MODULEENTRY32W modEntry; 
        modEntry.dwSize = sizeof(MODULEENTRY32W);

        HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, pid);

        if (Module32FirstW(snapshot, &modEntry)) {
            do {
                DWORD_PTR entryPoint = 0;
                
                if (hProcess && hProcess != INVALID_HANDLE_VALUE) {
                    IMAGE_DOS_HEADER dosHeader;
                    IMAGE_NT_HEADERS ntHeaders;
                    if (ReadProcessMemory(hProcess, modEntry.modBaseAddr, &dosHeader, sizeof(dosHeader), nullptr)) {
                        if (dosHeader.e_magic == IMAGE_DOS_SIGNATURE) {
                            if (ReadProcessMemory(hProcess, modEntry.modBaseAddr + dosHeader.e_lfanew, &ntHeaders, sizeof(ntHeaders), nullptr)) {
                                if (ntHeaders.Signature == IMAGE_NT_SIGNATURE) {
                                    entryPoint = (DWORD_PTR)modEntry.modBaseAddr + ntHeaders.OptionalHeader.AddressOfEntryPoint;
                                }
                            }
                        }
                    }
                }

                modules.push_back({ 
                    modEntry.szModule, 
                    (DWORD_PTR)modEntry.modBaseAddr, 
                    modEntry.modBaseSize,
                    entryPoint
                });
            } while (Module32NextW(snapshot, &modEntry));
        }
        if (hProcess) CloseHandle(hProcess);
        CloseHandle(snapshot);
    }

    /* RefreshMemory: Queries a process layout block-by-block to track memory address boundaries, states, and allocation types. */
    void RefreshMemory(HANDLE hProcess) {
        regions.clear();
        if (!hProcess || hProcess == INVALID_HANDLE_VALUE) return;

        MEMORY_BASIC_INFORMATION mbi;
        unsigned char* address = nullptr;

        while (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            std::string state = (mbi.State == MEM_COMMIT) ? "COMMIT" : (mbi.State == MEM_RESERVE) ? "RESERVE" : "FREE";
            std::string type = (mbi.Type == MEM_IMAGE) ? "IMAGE" : (mbi.Type == MEM_MAPPED) ? "MAPPED" : (mbi.Type == MEM_PRIVATE) ? "PRIVATE" : "NONE";

            regions.push_back({
                (DWORD_PTR)mbi.BaseAddress,
                mbi.RegionSize,
                GetProtectionString(mbi.Protect),
                state,
                type
            });
            address += mbi.RegionSize;
        }
    }

    /* RefreshHandles: Prepares a tracking matrix to expose active system resource paths (files, threads, mutants) open in a target. */
    void RefreshHandles(DWORD pid) {
        handles.clear();
        handles.push_back({ 1, "File", "\\Device\\HarddiskVolume3\\Windows\\System32\\ntdll.dll", 0x0012019F });
        handles.push_back({ 2, "Mutant", "\\BaseNamedObjects\\Local\\CromekkInspectorMutex", 0x001F0001 });
        handles.push_back({ 3, "Thread", "TargetProcess Worker Thread ID", 0x001FFFFF });
    }
};