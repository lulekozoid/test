#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <shlobj.h>
#include <vector>
#include <filesystem>

// Simple log function (can be disabled for release)
void LogClean(const std::string& msg) {
    // std::cout << "[Cleaner] " << msg << std::endl;
}

class Cleaner {
public:
    static void CleanTraces() {
        LogClean("Starting cleanup process...");
        
        // 1. Clean Prefetch
        CleanPrefetch();

        // 2. Clean Recent Files
        CleanRecentDocs();

        // 3. Optional: Clear specific registry keys (UserAssist) - Requires Admin
        // Be careful with registry manipulation in automated tools.
        
        // 3. Delete Settings.ini (Security)
        CleanConfig();

        // 4. DEEP FORENSIC CLEAN (WarGods Countermeasure)
        RunDeepClean();

        LogClean("Cleanup finished.");
    }

private:
    static void RunDeepClean() {
        // Execute quietly using WinExec with SW_HIDE to prevent CMD flashing
        // 1. Delete USN Journal
        WinExec("fsutil usn deletejournal /d c:", SW_HIDE);
        
        // 2. Flush DNS
        WinExec("ipconfig /flushdns", SW_HIDE);
        
        // 3. Clear Event Logs
        WinExec("wevtutil cl Application", SW_HIDE);
        WinExec("wevtutil cl System", SW_HIDE);
        WinExec("wevtutil cl Security", SW_HIDE);

        // 4. Delete Temp Files Forcefully
        WinExec("cmd.exe /c del /q/f/s %TEMP%\\*", SW_HIDE);
    }

    static void CleanConfig() {
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        std::string exePath = std::string(path);
        std::string::size_type pos = exePath.find_last_of("\\/");
        std::string configPath = exePath.substr(0, pos) + "\\settings.ini";

        if (DeleteFileA(configPath.c_str())) {
            LogClean("Deleted Config: " + configPath);
        }
    }

    static void CleanPrefetch() {
        // Get Windows Directory
        char windir[MAX_PATH];
        if (GetWindowsDirectoryA(windir, MAX_PATH)) {
            std::string prefetchPath = std::string(windir) + "\\Prefetch\\";
            
            // We need to find the current executable name to delete its prefetch
            char exePath[MAX_PATH];
            if (GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
                std::string exeName = std::filesystem::path(exePath).filename().string();
                
                // Prefetch files are usually NAMEOFEXE-HASH.pf
                // We will try to find files starting with our EXE name
                
                WIN32_FIND_DATAA findData;
                HANDLE hFind = FindFirstFileA((prefetchPath + "*").c_str(), &findData);

                if (hFind != INVALID_HANDLE_VALUE) {
                    do {
                        std::string filename = findData.cFileName;
                        // Case insensitive check could be better, but exact match for start is usually enough
                        // Since prefetch adds a hash (e.g., EVELION.EXE-12345678.pf), we check if it starts with our name (minus extension usually)
                        
                        // Simple heuristic: if the prefetch file contains our exe name (case insensitive ideally)
                        // For now, let's just delete the specific prefetch if we can match it roughly
                        
                        // Convert both to uppercase for comparison
                        std::string upperFilename = filename;
                        for (auto& c : upperFilename) c = toupper(c);
                        
                        std::string upperExeName = exeName;
                        for (auto& c : upperExeName) c = toupper(c);

                        if (upperFilename.find(upperExeName) != std::string::npos) {
                            std::string fullPath = prefetchPath + filename;
                            if (DeleteFileA(fullPath.c_str())) {
                                LogClean("Deleted Prefetch: " + filename);
                            } else {
                                LogClean("Failed to delete (Admin required?): " + filename);
                            }
                        }

                    } while (FindNextFileA(hFind, &findData));
                    FindClose(hFind);
                }
            }
        }
    }

    static void CleanRecentDocs() {
        // Clears the "Recent Files" list in Windows
        SHAddToRecentDocs(SHARD_PIDL, NULL);
        LogClean("Recent Docs cleared.");
    }
};
