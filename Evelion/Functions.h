#pragma once
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <iostream>
#include <dwmapi.h>
#include <d3d9.h>
#include <chrono>
#include <thread>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    const int bufferSize = 256;
    char buffer[bufferSize];

    if (GetWindowTextA(hwnd, buffer, bufferSize) > 0) {
        if (strstr(buffer, "Counter") != nullptr && strstr(buffer, "Strike") != nullptr) {

            *reinterpret_cast<HWND*>(lParam) = hwnd;
            return FALSE;
        }
    }

    return TRUE;
}

HWND GetProcessHwnd() {

    HWND hwnd = nullptr;

    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&hwnd));

    if (hwnd != nullptr) {
        return hwnd;
    }
    else {
        MessageBox(nullptr, "Get ProcessHwnd error.", "Evelion", MB_OK);
        exit(0);
    }
}

bool IsProcessAlive(const char* name) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) {
        do {
            if (strcmp(pe.szExeFile, name) == 0) {
                CloseHandle(hSnap);
                return true;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return false;
}

bool ProcessAlive() {
    while (1) {

        if (!IsProcessAlive(xorstr_("hl.exe"))) {
            MessageBox(nullptr, xorstr_("Game is closed. Please restart HACK"), xorstr_("HACK"), MB_OK);
            exit(0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}                                                                                                            

std::string RandomString(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}