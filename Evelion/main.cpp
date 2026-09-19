#include "xorstr.hpp"
#include "Functions.h"
#include "memory.h"
#include "struct.h"
#include "w2s.h"
#include "Offsets.h"

#include <TlHelp32.h>
#include <thread>
#include <string>
#include "iostream"
#include <cmath>
#include <math.h>
#include <vector>
#include <array>
#include <utility>
#include <cstring>
#include "Cleaner.h"
#include "JunkCode.h"
#include "Config.h" // Config System
#include <mutex>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <ShlObj.h> // For Desktop path
#include <Psapi.h> // For GetModuleInformation
#pragma comment(lib, "version.lib") // For GetFileVersionInfo
#pragma comment(lib, "psapi.lib") // For GetModuleInformation

// ============================================
// DEBUG LOG SISTEMI - MASAUSTUNE YAZAR
// ============================================
#define DEBUG_LOG_ENABLED true  // false yaparak tamamen kapat

class DebugLogger {
private:
    std::string logPath;
    std::ofstream logFile;
    int stepCounter = 0;
    
    std::string GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::tm tm_buf;
        localtime_s(&tm_buf, &time);
        
        std::ostringstream oss;
        oss << "[" << (tm_buf.tm_hour < 10 ? "0" : "") << tm_buf.tm_hour << ":"
            << (tm_buf.tm_min < 10 ? "0" : "") << tm_buf.tm_min << ":"
            << (tm_buf.tm_sec < 10 ? "0" : "") << tm_buf.tm_sec << "."
            << (ms.count() < 100 ? "0" : "") << (ms.count() < 10 ? "0" : "") << ms.count() << "]";
        return oss.str();
    }
    
    std::string GetDesktopPath() {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, path))) {
            return std::string(path);
        }
        return "C:\\Users\\Public\\Desktop";
    }

public:
    DebugLogger() {
        if (!DEBUG_LOG_ENABLED) return;
        
        logPath = GetDesktopPath() + "\\BEYAZKOLYE_DEBUG_LOG.txt";
        logFile.open(logPath, std::ios::out | std::ios::trunc);
        
        if (logFile.is_open()) {
            logFile << "========================================================\n";
            logFile << "     BEYAZ KOLYE VIP - HATA AYIKLAMA RAPORU\n";
            logFile << "========================================================\n";
            logFile << "Olusturulma: " << __DATE__ << " " << __TIME__ << "\n";
            logFile << "Log Dosyasi: " << logPath << "\n";
            logFile << "========================================================\n\n";
            logFile.flush();
        }
    }
    
    ~DebugLogger() {
        if (logFile.is_open()) {
            logFile << "\n========================================================\n";
            logFile << "LOG SONA ERDI\n";
            logFile << "========================================================\n";
            logFile.close();
        }
    }
    
    void LogSystemInfo() {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << "--- SISTEM BILGILERI ---\n";
        
        // Windows Version
        OSVERSIONINFOEXA osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
        
        logFile << "Windows Surumu: ";
        HMODULE hMod = GetModuleHandleA("ntdll.dll");
        if (hMod) {
            typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
            RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
            if (RtlGetVersion) {
                RTL_OSVERSIONINFOW rovi = { 0 };
                rovi.dwOSVersionInfoSize = sizeof(rovi);
                if (RtlGetVersion(&rovi) == 0) {
                    logFile << rovi.dwMajorVersion << "." << rovi.dwMinorVersion << " (Build " << rovi.dwBuildNumber << ")\n";
                }
            }
        }
        
        // RAM
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            logFile << "Toplam RAM: " << (memInfo.ullTotalPhys / (1024 * 1024)) << " MB\n";
            logFile << "Kullanilabilir RAM: " << (memInfo.ullAvailPhys / (1024 * 1024)) << " MB\n";
        }
        
        // DirectX DLL Check
        HMODULE d3d9 = GetModuleHandleA("d3d9.dll");
        logFile << "DirectX 9 (d3d9.dll): " << (d3d9 ? "YUKLU" : "EKSIK!") << "\n";
        
        // Visual C++ Runtime Check
        HMODULE vcrt = GetModuleHandleA("vcruntime140.dll");
        logFile << "VC++ Runtime (vcruntime140.dll): " << (vcrt ? "YUKLU" : "EKSIK!") << "\n";
        
        logFile << "------------------------\n\n";
        logFile.flush();
    }
    
    void LogStep(const std::string& message, bool success = true) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        stepCounter++;
        logFile << GetTimestamp() << " [ADIM " << stepCounter << "] " 
                << (success ? "[OK] " : "[HATA] ") << message << "\n";
        logFile.flush();
    }
    
    void LogError(const std::string& errorCode, const std::string& description, const std::string& solution) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        logFile << GetTimestamp() << " ***** KRITIK HATA *****\n";
        logFile << "Hata Kodu: " << errorCode << "\n";
        logFile << "Aciklama: " << description << "\n";
        logFile << "Cozum: " << solution << "\n";
        logFile << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n";
        logFile.flush();
    }
    
    void LogValue(const std::string& name, const std::string& value) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << GetTimestamp() << " [DEGER] " << name << " = " << value << "\n";
        logFile.flush();
    }
    
    void LogValue(const std::string& name, int value) {
        LogValue(name, std::to_string(value));
    }
    
    void LogValue(const std::string& name, long value) {
        LogValue(name, std::to_string(value));
    }
    
    void LogValue(const std::string& name, DWORD value) {
        std::ostringstream oss;
        oss << value << " (0x" << std::hex << value << ")";
        LogValue(name, oss.str());
    }
    
    void LogValue(const std::string& name, void* ptr) {
        std::ostringstream oss;
        oss << "0x" << std::hex << reinterpret_cast<uintptr_t>(ptr);
        LogValue(name, oss.str());
    }
    
    void LogModuleInfo(const std::string& moduleName, uintptr_t address) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        std::ostringstream oss;
        oss << "0x" << std::hex << address;
        
        if (address == 0) {
            logFile << GetTimestamp() << " [MODUL] " << moduleName << " = BULUNAMADI!\n";
        } else {
            logFile << GetTimestamp() << " [MODUL] " << moduleName << " = " << oss.str() << " [OK]\n";
        }
        logFile.flush();
    }
    
    void LogGameInfo(HWND hwnd, DWORD pid) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << "\n--- OYUN BILGILERI ---\n";
        
        if (hwnd) {
            char title[256] = {0};
            GetWindowTextA(hwnd, title, 256);
            logFile << "Pencere Basligi: " << title << "\n";
            
            char className[256] = {0};
            GetClassNameA(hwnd, className, 256);
            logFile << "Pencere Sinifi: " << className << "\n";
            
            RECT rect;
            if (GetWindowRect(hwnd, &rect)) {
                logFile << "Pencere Boyutu: " << (rect.right - rect.left) << "x" << (rect.bottom - rect.top) << "\n";
                logFile << "Pencere Konumu: (" << rect.left << ", " << rect.top << ")\n";
            }
            
            // Check if fullscreen
            RECT desktopRect;
            GetWindowRect(GetDesktopWindow(), &desktopRect);
            bool isFullscreen = (rect.left == 0 && rect.top == 0 && 
                                 rect.right == desktopRect.right && 
                                 rect.bottom == desktopRect.bottom);
            logFile << "Tam Ekran: " << (isFullscreen ? "EVET (SORUN!)" : "HAYIR (OK)") << "\n";
        } else {
            logFile << "Pencere: BULUNAMADI!\n";
        }
        
        logFile << "Process ID (PID): " << pid << "\n";
        logFile << "------------------------\n\n";
        logFile.flush();
    }
    
    void LogSuccess(const std::string& message) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << "\n*** BASARI: " << message << " ***\n\n";
        logFile.flush();
    }
    
    void LogMemoryError(const std::string& operation, uintptr_t address, DWORD errorCode) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        std::ostringstream oss;
        oss << "Adres: 0x" << std::hex << address << " - Hata Kodu: " << std::dec << errorCode;
        
        logFile << "\n!!! BELLEK HATASI !!!\n";
        logFile << GetTimestamp() << " Islem: " << operation << "\n";
        logFile << oss.str() << "\n";
        
        // Hata koduna gore cozum onerileri
        switch(errorCode) {
            case 5: // ERROR_ACCESS_DENIED
                logFile << "Cozum: Yonetici olarak calistirin veya oyun hakki yok.\n";
                break;
            case 6: // ERROR_INVALID_HANDLE
                logFile << "Cozum: Oyun kapanmis olabilir. Yeniden baslatin.\n";
                break;
            case 299: // ERROR_PARTIAL_COPY
                logFile << "Cozum: Bellek bolgesi korunmus veya gecersiz.\n";
                break;
            case 998: // ERROR_NOACCESS
                logFile << "Cozum: Bu bellek bolgesine erisim yok.\n";
                break;
            default:
                logFile << "Cozum: Bilinmeyen hata. Oyunu yeniden baslatin.\n";
        }
        logFile << "\n";
        logFile.flush();
    }
    
    void LogThreadInfo(const std::string& threadName, bool started) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << GetTimestamp() << " [THREAD] " << threadName << ": " 
                << (started ? "BASLATILDI" : "DURDURULDU") << "\n";
        logFile.flush();
    }
    
    void LogPerformance(const std::string& operation, int milliseconds) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << GetTimestamp() << " [PERFORMANS] " << operation << ": " << milliseconds << " ms\n";
        logFile.flush();
    }
    
    void LogWarning(const std::string& message) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << GetTimestamp() << " [UYARI] " << message << "\n";
        logFile.flush();
    }
    
    void LogSection(const std::string& sectionName) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << "\n========== " << sectionName << " ==========\n";
        logFile.flush();
    }
    
    void LogHexDump(const std::string& name, void* data, size_t size) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        if (size > 64) size = 64; // Max 64 byte
        
        logFile << GetTimestamp() << " [HEX] " << name << ": ";
        unsigned char* bytes = (unsigned char*)data;
        for (size_t i = 0; i < size; i++) {
            char hex[4];
            sprintf_s(hex, "%02X ", bytes[i]);
            logFile << hex;
        }
        logFile << "\n";
        logFile.flush();
    }
    
    void LogCrash(const std::string& location, DWORD exceptionCode) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << "\n################################################################\n";
        logFile << "#################### COKME TESPIT EDILDI ####################\n";
        logFile << "################################################################\n";
        logFile << GetTimestamp() << "\n";
        logFile << "Konum: " << location << "\n";
        logFile << "Exception Kodu: 0x" << std::hex << exceptionCode << std::dec << "\n";
        
        switch(exceptionCode) {
            case 0xC0000005:
                logFile << "Tur: ACCESS_VIOLATION - Bellek erisim ihlali\n";
                logFile << "Cozum: Oyun guncellenmis olabilir. Offset'leri kontrol edin.\n";
                break;
            case 0xC0000094:
                logFile << "Tur: INTEGER_DIVIDE_BY_ZERO - Sifira bolme hatasi\n";
                break;
            case 0xC00000FD:
                logFile << "Tur: STACK_OVERFLOW - Yigin tasmasi\n";
                break;
            default:
                logFile << "Tur: BILINMEYEN EXCEPTION\n";
        }
        logFile << "################################################################\n\n";
        logFile.flush();
    }
    
    void LogOffsetCheck(const std::string& name, uintptr_t offset, bool valid) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        std::ostringstream oss;
        oss << "0x" << std::hex << offset;
        
        logFile << GetTimestamp() << " [OFFSET] " << name << " = " << oss.str() 
                << " " << (valid ? "[GECERLI]" : "[GECERSIZ!]") << "\n";
        logFile.flush();
    }
    
    void LogCheatStatus(bool espEnabled, bool aimbotEnabled) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << "\n--- HILE DURUMU ---\n";
        logFile << "ESP: " << (espEnabled ? "ACIK" : "KAPALI") << "\n";
        logFile << "Aimbot: " << (aimbotEnabled ? "ACIK" : "KAPALI") << "\n";
        logFile << "-------------------\n\n";
        logFile.flush();
    }
    
    // Steam Beta Sürümü Tespiti
    void LogSteamBetaInfo() {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << "\n--- STEAM / OYUN SURUMU TESPITI ---\n";
        
        // 1. hl.exe dosya yolunu bul
        char hlPath[MAX_PATH] = {0};
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, Game::PID);
        if (hProcess) {
            DWORD pathSize = MAX_PATH;
            QueryFullProcessImageNameA(hProcess, 0, hlPath, &pathSize);
            CloseHandle(hProcess);
            logFile << "hl.exe Yolu: " << hlPath << "\n";
        }
        
        // 2. hl.exe dosya boyutuna bakarak beta tespiti
        if (strlen(hlPath) > 0) {
            HANDLE hFile = CreateFileA(hlPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                LARGE_INTEGER fileSize;
                GetFileSizeEx(hFile, &fileSize);
                CloseHandle(hFile);
                
                logFile << "hl.exe Dosya Boyutu: " << fileSize.QuadPart << " bytes\n";
                
                // Bilinen Steam Beta boyutlari (yaklasik)
                std::string betaType = "BILINMIYOR";
                
                // CS 1.6 versiyonlarini dosya boyutuna gore tahmin et
                if (fileSize.QuadPart > 4000000 && fileSize.QuadPart < 4500000) {
                    betaType = "STEAM (Standart - Pre-25th Anniversary)";
                } else if (fileSize.QuadPart > 4500000 && fileSize.QuadPart < 5000000) {
                    betaType = "STEAM (25th Anniversary Beta)";
                } else if (fileSize.QuadPart > 5000000 && fileSize.QuadPart < 6000000) {
                    betaType = "STEAM (Yeni Beta - Limited Beta)";
                } else if (fileSize.QuadPart > 2000000 && fileSize.QuadPart < 3000000) {
                    betaType = "NON-STEAM (Korsan veya Eski Surum)";
                } else if (fileSize.QuadPart > 1000000 && fileSize.QuadPart < 2000000) {
                    betaType = "NON-STEAM (Warzone veya Revemu)";
                }
                
                logFile << "Tahmin Edilen Surum: " << betaType << "\n";
            }
            
            // 3. Dosya versiyonunu oku
            DWORD verHandle = 0;
            DWORD verSize = GetFileVersionInfoSizeA(hlPath, &verHandle);
            if (verSize > 0) {
                char* verData = new char[verSize];
                if (GetFileVersionInfoA(hlPath, verHandle, verSize, verData)) {
                    VS_FIXEDFILEINFO* fileInfo = nullptr;
                    UINT len = 0;
                    if (VerQueryValueA(verData, "\\", (void**)&fileInfo, &len)) {
                        logFile << "Dosya Versiyonu: " 
                                << HIWORD(fileInfo->dwFileVersionMS) << "."
                                << LOWORD(fileInfo->dwFileVersionMS) << "."
                                << HIWORD(fileInfo->dwFileVersionLS) << "."
                                << LOWORD(fileInfo->dwFileVersionLS) << "\n";
                    }
                }
                delete[] verData;
            }
        }
        
        // 4. Steam Registry kontrolu
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            logFile << "Steam Kurulu: EVET\n";
            
            char steamPath[MAX_PATH] = {0};
            DWORD pathSize = MAX_PATH;
            if (RegQueryValueExA(hKey, "SteamPath", NULL, NULL, (LPBYTE)steamPath, &pathSize) == ERROR_SUCCESS) {
                logFile << "Steam Yolu: " << steamPath << "\n";
            }
            RegCloseKey(hKey);
            
            // CS 1.6 Beta secimini kontrol et
            std::string betaCheckPath = std::string(steamPath) + "\\steamapps\\common\\Half-Life";
            if (GetFileAttributesA(betaCheckPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                logFile << "CS 1.6 Klasoru Mevcut: EVET\n";
            }
        } else {
            logFile << "Steam Kurulu: HAYIR (Non-Steam surum olabilir)\n";
        }
        
        // 5. Modul versiyonlarini kontrol et
        HMODULE hHw = GetModuleHandleA("hw.dll");
        HMODULE hClient = GetModuleHandleA("client.dll");
        HMODULE hVgui = GetModuleHandleA("vgui.dll");
        HMODULE hSteamApi = GetModuleHandleA("steam_api.dll");
        
        logFile << "\n--- MODUL DURUMU ---\n";
        logFile << "hw.dll: " << (hHw ? "YUKLU" : "YOK") << "\n";
        logFile << "client.dll: " << (hClient ? "YUKLU" : "YOK") << "\n";
        logFile << "vgui.dll: " << (hVgui ? "YUKLU" : "YOK") << "\n";
        logFile << "steam_api.dll: " << (hSteamApi ? "YUKLU (Steam)" : "YOK (Non-Steam olabilir)") << "\n";
        
        // 6. Onemli uyari
        if (!hSteamApi) {
            logFile << "\n[UYARI] steam_api.dll bulunamadi!\n";
            logFile << "Bu, Non-Steam (korsan) bir surum oldugunu gosterebilir.\n";
            logFile << "Hile sadece STEAM surumunde test edilmistir.\n";
        }
        
        logFile << "------------------------------------\n\n";
        logFile.flush();
    }
    
    void LogGameVersion(uintptr_t hwBase) {
        if (!DEBUG_LOG_ENABLED || !logFile.is_open()) return;
        
        logFile << "\n--- OYUN VERSIYON ANALIZI ---\n";
        
        // hw.dll boyutu ile versiyon tahmini
        MODULEINFO modInfo;
        HMODULE hHw = GetModuleHandleA("hw.dll");
        if (hHw && GetModuleInformation(GetCurrentProcess(), hHw, &modInfo, sizeof(modInfo))) {
            logFile << "hw.dll Boyutu: " << modInfo.SizeOfImage << " bytes\n";
            
            // Bilinen hw.dll boyutlari
            if (modInfo.SizeOfImage > 0x1A00000) {
                logFile << "Versiyon Tahmini: 25th Anniversary Steam Beta\n";
                logFile << "[UYARI] Bu surum icin offset'ler farkli olabilir!\n";
            } else if (modInfo.SizeOfImage > 0x1800000) {
                logFile << "Versiyon Tahmini: Steam (8684 Build)\n";
            } else if (modInfo.SizeOfImage > 0x1500000) {
                logFile << "Versiyon Tahmini: Steam (Eski Build)\n";
            } else {
                logFile << "Versiyon Tahmini: Non-Steam veya Modlu Surum\n";
            }
        }
        
        logFile << "-------------------------------\n\n";
        logFile.flush();
    }
};

// Global Logger Instance
DebugLogger g_Logger;


// initialization
// ==============
// AUTO-HASH CHANGER: This ensures the EXE Hash changes every time you re-compile!
// WarGods cannot ban the Hash if it changes every minute.
volatile const char* BUILD_T = __DATE__ " " __TIME__; 

// CRITICAL FIX: These were global constructors running before main().
// If hl.exe wasn't running, processHandle=NULL, all reads crash.
// Now they are pointers initialized in WinMain() after process check.
Memory* g_memory = nullptr;
uintptr_t hw = 0;
uintptr_t client = 0;

uintptr_t resolvedViewMatrixAddress = 0;

HWND hwnd1 = NULL;
// CRITICAL FIX: Removed uninitialized hwnd1 usage:
// Old code: int id = GetWindowThreadProcessId(hwnd1, &Game::PID); // CRASHED - hwnd1 was garbage!
int id = 0;
// ==============
// end of initialization

// Loader Mutex Protection (REMOVED FOR GITHUB RELEASE)
// bool IsLoaderRunning() { ... }
// void LoaderCheckThread() { ... }


// not working concept, i'll leave it here for future edit :(
/*
void GetPlayersCount() {

	uintptr_t startAddress = libcef + 0x0B5B8A8;
	int pointer = memory.Read<int>(startAddress);

	std::stringstream temp;
	temp << std::hex << pointer;

	uintptr_t hexValue;
	temp >> hexValue;

	std::cout << "First: 0x" << std::hex << hexValue << std::endl;

	uintptr_t finalAddress = hexValue + 0x548;

	std::cout << "Final: 0x" << std::hex << finalAddress << std::endl;
	playersCount = memory.Read<int>(finalAddress);

	std::cout << "Count: " << playersCount << std::endl;
}
*/

bool IsPlausibleMatrix(const float* matrix) {
	float magnitude = 0.0f;
	for (int i = 0; i < 16; ++i) {
		if (!std::isfinite(matrix[i]) || std::fabs(matrix[i]) > 1000000.0f)
			return false;
		magnitude += std::fabs(matrix[i]);
	}
	return magnitude > 0.01f;
}

bool TryReadViewMatrix(uintptr_t address, float* matrix) {
	return address != 0 &&
		g_memory->ReadHugeMemory(address, matrix, sizeof(float) * 16) &&
		IsPlausibleMatrix(matrix);
}

bool IsPlausiblePosition(const Vector3& position) {
	return std::isfinite(position.x) && std::isfinite(position.y) && std::isfinite(position.z) &&
		std::fabs(position.x) < 131072.0f &&
		std::fabs(position.y) < 131072.0f &&
		std::fabs(position.z) < 131072.0f;
}

bool TryReadEntityPosition(uintptr_t entity, uintptr_t objectData, Vector3& position) {
	if (g_memory->Read(entity + Offsets::Build10210::EntityPosition, position) &&
		IsPlausiblePosition(position)) {
		return true;
	}

	return objectData > 0x10000 &&
		g_memory->Read(objectData + Offsets::Build10210::ObjectPosition, position) &&
		IsPlausiblePosition(position);
}

bool TryReadEntityHealth(uintptr_t entity, uintptr_t objectData, float& health) {
	if (objectData > 0x10000 &&
		g_memory->Read(objectData + Offsets::Build10210::EntityHealth, health) &&
		std::isfinite(health) && health >= 0.0f && health <= 10000.0f) {
		return true;
	}

	return g_memory->Read(entity + Offsets::Build10210::EntityHealth, health) &&
		std::isfinite(health) && health >= 0.0f && health <= 10000.0f;
}

std::string TryReadPlayerName(int index) {
	char buffer[32] = {};
	const uintptr_t address = hw +
		Offsets::Build10210::PlayerInfoList +
		Offsets::Build10210::PlayerInfoStride * static_cast<uintptr_t>(index) +
		Offsets::Build10210::PlayerInfoName;

	if (!g_memory->ReadHugeMemory(address, buffer, sizeof(buffer)))
		return {};

	size_t length = 0;
	while (length < sizeof(buffer) && buffer[length] != '\0') {
		const unsigned char value = static_cast<unsigned char>(buffer[length]);
		if (value < 32 || value == 127)
			return {};
		++length;
	}

	if (length == 0 || length == sizeof(buffer))
		return {};

	return std::string(buffer, length);
}

void MatrixUpdate() {
	float matrix[16] = {};

	while (1) {
		while (esp || aimbot_active) {
			if (!g_memory || hw == 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			const uintptr_t candidates[] = {
			hw + Offsets::Build10210::ViewMatrixHw,
			client ? client + Offsets::Build10210::ViewMatrixClient : 0
			};

			bool found = false;
			for (uintptr_t address : candidates) {
				if (TryReadViewMatrix(address, matrix)) {
					resolvedViewMatrixAddress = address;
					memcpy(gWorldToScreen, matrix, sizeof(gWorldToScreen));
					found = true;
					break;
				}
			}

			if (!found)
				resolvedViewMatrixAddress = 0;

			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void OffsetsUpdate() {
	while (1) {
		JUNK_BLOCK_1;
		while (esp || aimbot_active) {
			if (!g_memory || hw == 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			Offsets::RemotePtr entityList = 0;
			if (!g_memory->Read(hw + Offsets::Build10210::EntityListRoot, entityList) || entityList < 0x10000) {
				std::lock_guard<std::mutex> lock(playerMutex);
				for (auto& player : players)
					player = PlayerPosition{};
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			Offsets::RemotePtr localPlayer = 0;
			int localTeam = 0;
			g_memory->Read(entityList + Offsets::Build10210::LocalPlayerSlot, localPlayer);
			if (localPlayer > 0x10000)
				g_memory->Read(localPlayer + Offsets::Build10210::EntityTeam, localTeam);

			std::array<PlayerPosition, Offsets::MaxPlayers> nextPlayers{};

			for (int i = 0; i < Offsets::MaxPlayers; ++i) {
				const uintptr_t slotAddress = entityList +
					Offsets::Build10210::LocalPlayerSlot +
					Offsets::Build10210::EntityStride * static_cast<uintptr_t>(i + 1);

				Offsets::RemotePtr entity = 0;
				if (!g_memory->Read(slotAddress, entity) || entity < 0x10000 || entity == localPlayer)
					continue;

				int team = 0;
				if (!g_memory->Read(entity + Offsets::Build10210::EntityTeam, team) || team < 1 || team > 2)
					continue;

				if (def_models && localTeam >= 1 && localTeam <= 2 && team == localTeam)
					continue;

				Offsets::RemotePtr objectData = 0;
				g_memory->Read(entity + Offsets::Build10210::EntityObjectData, objectData);

				float health = 0.0f;
				Vector3 position{};
				if (!TryReadEntityHealth(entity, objectData, health) ||
					!TryReadEntityPosition(entity, objectData, position) ||
					health <= 1.0f) {
					continue;
				}

				Vector3 target = { position.x, position.y, position.z + aimbot_bone_z };
				float screenPosition[2] = {};
				if (!WorldToScreen(target, screenPosition))
					continue;

				nextPlayers[i].screenPosition[0] = screenPosition[0];
				nextPlayers[i].screenPosition[1] = screenPosition[1];
				nextPlayers[i].dead = false;
				nextPlayers[i].health = health;
				nextPlayers[i].team = team;
				nextPlayers[i].name = TryReadPlayerName(i);
				if (nextPlayers[i].name.empty())
					nextPlayers[i].name = "Player " + std::to_string(i + 1);
			}

			{
				std::lock_guard<std::mutex> lock(playerMutex);
				for (int i = 0; i < Offsets::MaxPlayers; ++i)
					players[i] = std::move(nextPlayers[i]);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

// Basic Distance function
float GetDistance2D(float x1, float y1, float x2, float y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void AimbotLogic() {
	if (!aimbot_active) return;
	if (!(GetAsyncKeyState(aimbot_key) & 0x8000)) return; // Only work if key held

	std::array<PlayerPosition, Offsets::MaxPlayers> playerSnapshot{};
	{
		std::lock_guard<std::mutex> lock(playerMutex);
		for (int i = 0; i < Offsets::MaxPlayers; ++i)
			playerSnapshot[i] = players[i];
	}

	float bestDist = aimbot_fov;
	int bestIndex = -1;
	float centerX = (float)width / 2.0f;
	float centerY = (float)height / 2.0f;

	// Find Closest Target to Crosshair (FOV)
	for (int i = 0; i < Offsets::MaxPlayers; i++) {
		if (playerSnapshot[i].dead || playerSnapshot[i].screenPosition[0] == 0) continue;

		float x = playerSnapshot[i].screenPosition[0];
		float y = playerSnapshot[i].screenPosition[1];
		
		float dist = GetDistance2D(centerX, centerY, x, y);

		if (dist < bestDist) {
			bestDist = dist;
			bestIndex = i;
		}
	}

	// Aim
	if (bestIndex != -1) {
		float targetX = playerSnapshot[bestIndex].screenPosition[0];
		float targetY = playerSnapshot[bestIndex].screenPosition[1];

		float deltaX = targetX - centerX;
		float deltaY = targetY - centerY;

		// Smooth
		if (aimbot_smooth < 1.0f) aimbot_smooth = 1.0f;
		deltaX /= aimbot_smooth;
		deltaY /= aimbot_smooth;

		mouse_event(MOUSEEVENTF_MOVE, (DWORD)deltaX, (DWORD)deltaY, 0, 0);
	}
}

void LobbyCheck() {

	while (1) {
		// ESP KAPALI: Aimbot yine de çalışsın
		if (!esp) {
			AimbotLogic();
		}
		
		while (esp) {
			// ESP AÇIK: Aimbot burada da çalışsın!
			AimbotLogic();
			
			if (g_memory && hw != 0) {
				Offsets::RemotePtr entityList = 0;
				const bool inGame = g_memory->Read(
					hw + Offsets::Build10210::EntityListRoot,
					entityList) && entityList > 0x10000;

				if (!inGame && !in_lobby) {
					std::lock_guard<std::mutex> lock(playerMutex);
					for (auto& player : players)
						player = PlayerPosition{};
					in_lobby = true;
				}
				else if (inGame) {
					in_lobby = false;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 100Hz - Daha hızlı, ama salak gibi değil
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

void Draw() {
	if (esp) {
		std::lock_guard<std::mutex> lock(playerMutex); // Lock for thread safety during drawing

		if (enemy_box || enemy_name) {
			for (int i = 0; i < 64; i++)
			{
				float x = players[i].screenPosition[0];
				float y = players[i].screenPosition[1];

				if (players[i].dead || y < 5 || x < 5) continue;
				
				// Draw FOV Circle
				if(aimbot_active && i == 0) { // Draw only once
					// ImGui::GetOverlayDrawList()->AddCircle(ImVec2(width/2, height/2), aimbot_fov, IM_COL32(255, 255, 255, 100), 100);
				}

				if (enemy_box) {
					// Draw Rectangle based on "Head" position?
					// Currently 'x' and 'y' are the HEAD position (because we added offset in Loop).
					// So drawing box needs to go DOWN from head.
					
					// Head is at (x, y). Feet is approx at (x, y + height).
					// Height depends on distance... 
					// Since we don't have proper W2S height calculation here, 
					// we can just stick to the requested "DOT" but at HEAD level (which we fixed).
					// User said "ESP shows chest dot". Now it will show Head Dot.
					
					// I will modify this to draw a small corner box if requested?
					// User said "ESP based flawless". A box is better than dot.
					// Let's draw a generic size box around the head dot for now, or just the dot if user prefers.
					// "ESP dot" comment suggests they see a dot. 
					
					DrawCircleFilled(x, y, 3.0, &BoxColor); // Head Dot
				}

				if (enemy_name) DrawNewText(x - 14, y - 32, &NameColor, players[i].name.c_str());

			}
		}
	}
	JUNK_BLOCK_2; // More Junk Code during render
}

void SetStyle() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // White Checkmark
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	
	style.WindowRounding = 4.0f;
	style.FrameRounding = 2.0f;
	style.GrabRounding = 2.0f;
}

void DrawMenu() {
	static bool styleInit = false;
	if(!styleInit) { SetStyle(); styleInit = true; }

	ImGui::SetNextWindowSize(ImVec2(450.f, 300.f));
	
	// Premium "Dark & White" Theme
	ImGui::Begin("EVELION", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
	
	// Header Center
	float winWidth = ImGui::GetWindowWidth();
	float textWidth = ImGui::CalcTextSize(xorstr_("EVELION - AniLLL3734")).x;
	ImGui::SetCursorPosX((winWidth - textWidth) * 0.5f);
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), xorstr_("EVELION - AniLLL3734"));
	
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Columns(2, xorstr_("MainColumns"), false); // 2 Columns
	ImGui::SetColumnWidth(0, 140); // Sidebar Width

	// Column 1: Sidebar / Selection
	ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), xorstr_("[ MENU ]"));
	ImGui::Spacing();
	static int activeTab = 0;
	if(ImGui::Button(t(xorstr_("Visuals"), xorstr_("Goruntu")), ImVec2(120, 25))) activeTab = 0;
	if(ImGui::Button(xorstr_("Aimbot"), ImVec2(120, 25))) activeTab = 1; 
	if(ImGui::Button(t(xorstr_("Colors"), xorstr_("Renkler")), ImVec2(120, 25))) activeTab = 2;
	if(ImGui::Button(t(xorstr_("Settings"), xorstr_("Ayarlar")), ImVec2(120, 25))) activeTab = 3; 
	
	ImGui::Spacing(); ImGui::Spacing();
	ImGui::TextDisabled(xorstr_("v2.2 | Build 10210"));

	ImGui::NextColumn();

	// Column 2: Content
	if(activeTab == 0) {
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), t(xorstr_("> Visual Settings"), xorstr_("> Goruntu Ayarlari")));
		ImGui::Separator();
		ImGui::Spacing();
		
		ImGui::Checkbox(t(xorstr_("Master Switch"), xorstr_("Ana Salter")), &esp);
		if(esp) {
			ImGui::Indent();
			ImGui::Checkbox(t(xorstr_("Draw Box (Head Dot)"), xorstr_("Kutu (Kafa Noktasi)")), &enemy_box);
			ImGui::Checkbox(t(xorstr_("Draw Name"), xorstr_("Isim Goster")), &enemy_name);
			ImGui::Checkbox(t(xorstr_("Enemy Only"), xorstr_("Sadece Dusmanlar")), &def_models);
			ImGui::SliderFloat(t(xorstr_("Head Offset"), xorstr_("Kafa Ayari")), &aimbot_bone_z, 0.f, 50.f, xorstr_("%.1f"));
			ImGui::Unindent();
		}
	}
	else if (activeTab == 1) {
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), xorstr_("> Aimbot Settings"));
		ImGui::Separator();
		ImGui::Spacing();
		
		ImGui::Checkbox(t(xorstr_("Active"), xorstr_("Aktif")), &aimbot_active);
		ImGui::Indent();
		ImGui::SliderFloat(xorstr_("FOV"), &aimbot_fov, 5.0f, 500.0f, xorstr_("%.0f px"));
		ImGui::SliderFloat(xorstr_("Smooth"), &aimbot_smooth, 1.0f, 100.0f, xorstr_("%.1f"));
		
		const char* keys[] = { t(xorstr_("L-Mouse"), xorstr_("Sol-Tik")), t(xorstr_("R-Mouse"), xorstr_("Sag-Tik")), xorstr_("Alt"), xorstr_("Shift") };
		static int keyIdx = 0;
		if(ImGui::Combo(t(xorstr_("Key"), xorstr_("Tus")), &keyIdx, keys, IM_ARRAYSIZE(keys))) {
			switch(keyIdx) {
				case 0: aimbot_key = 0x01; break; // VK_LBUTTON
				case 1: aimbot_key = 0x02; break; // VK_RBUTTON
				case 2: aimbot_key = 0x12; break; // VK_MENU (Alt)
				case 3: aimbot_key = 0x10; break; // VK_SHIFT
			}
		}
		
		if(aimbot_active) {
			ImGui::TextDisabled(t(xorstr_("Hold Key to Aim"), xorstr_("Nisan icin Tuas Bas")));
		}
		ImGui::Unindent();
	}
	else if (activeTab == 2) {
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), t(xorstr_("> Color Configuration"), xorstr_("> Renk Ayarlari")));
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::ColorEdit3(xorstr_("Esp Box"), boxTemp);
		BoxColor.R = static_cast<int>(boxTemp[0] * 255);
		BoxColor.G = static_cast<int>(boxTemp[1] * 255);
		BoxColor.B = static_cast<int>(boxTemp[2] * 255);
		
		ImGui::Spacing();
		
		ImGui::ColorEdit3(xorstr_("Esp Name"), nameTemp);
		NameColor.R = static_cast<int>(nameTemp[0] * 255);
		NameColor.G = static_cast<int>(nameTemp[1] * 255);
		NameColor.B = static_cast<int>(nameTemp[2] * 255);
	}
	else if (activeTab == 3) { // Changed to 3
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), xorstr_("> SYSTEM"));
		ImGui::Separator();
		ImGui::Spacing();
		
		// Language Selector
		const char* languages[] = { "English", "Turkce" };
		ImGui::Combo(t(xorstr_("Language"), xorstr_("Dil")), &language, languages, IM_ARRAYSIZE(languages));
		ImGui::Spacing();

		if (ImGui::Button(xorstr_("SAVE CONFIG"), ImVec2(180, 25))) {
			Config::Save();
		}
		ImGui::Spacing();

		if (ImGui::Button(xorstr_("UNHOOK & PANIC (DELETE)"), ImVec2(180, 30))) {
			Unhook = true;
		}
		ImGui::TextDisabled(t(xorstr_("Press DELETE to \nPanic and Clean Logs."), xorstr_("DELETE basilinca\nLoglar silinir ve kapanir.")));
	}

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::TextDisabled(xorstr_("Status: Secure | User: VIP"));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), t(xorstr_("Aimbot & Visuals added by: github.com/AniLLL3734"), xorstr_("Aimbot ve Visuals ekleyen: github.com/AniLLL3734")));
	ImGui::End();
}

void Render() {
	if (GetAsyncKeyState(VK_DELETE) & 1) Unhook = true;
	if (GetAsyncKeyState(VK_INSERT) & 1) ShowMenu = !ShowMenu;
	//if (GetAsyncKeyState(VK_END) & 1) enemy_box = !enemy_box;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	Draw();

	if (ShowMenu)
		DrawMenu();

	ImGui::EndFrame();

	DirectX9Interface::pDevice->SetRenderState(D3DRS_ZENABLE, false);
	DirectX9Interface::pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	DirectX9Interface::pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	DirectX9Interface::pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	if (DirectX9Interface::pDevice->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		DirectX9Interface::pDevice->EndScene();
	}

	HRESULT result = DirectX9Interface::pDevice->Present(NULL, NULL, NULL, NULL);
	if (result == D3DERR_DEVICELOST && DirectX9Interface::pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
		ImGui_ImplDX9_InvalidateDeviceObjects();
		DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

void MainLoop() {
	static RECT OldRect;
	ZeroMemory(&DirectX9Interface::Message, sizeof(MSG));

	while (DirectX9Interface::Message.message != WM_QUIT) {
		if (Unhook) {
			// Config::Save(); REMOVED FOR SECURITY (WarGods Evidence)
			Cleaner::CleanTraces(); // <--- CLEAN TRACES ON EXIT
			ImGui_ImplDX9_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			DestroyWindow(OverlayWindow::Hwnd);
			UnregisterClass(OverlayWindow::WindowClass.lpszClassName, OverlayWindow::WindowClass.hInstance);
			exit(0);
		}

		if (PeekMessage(&DirectX9Interface::Message, OverlayWindow::Hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&DirectX9Interface::Message);
			DispatchMessage(&DirectX9Interface::Message);
		}
		HWND ForegroundWindow = GetForegroundWindow();
		if (ForegroundWindow == Process::Hwnd) {
			HWND TempProcessHwnd = GetWindow(ForegroundWindow, GW_HWNDPREV);
			SetWindowPos(OverlayWindow::Hwnd, TempProcessHwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		RECT TempRect;
		POINT TempPoint;
		ZeroMemory(&TempRect, sizeof(RECT));
		ZeroMemory(&TempPoint, sizeof(POINT));

		GetClientRect(Process::Hwnd, &TempRect);
		ClientToScreen(Process::Hwnd, &TempPoint);

		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = Process::Hwnd;

		POINT TempPoint2;
		GetCursorPos(&TempPoint2);
		io.MousePos.x = TempPoint2.x - TempPoint.x;
		io.MousePos.y = TempPoint2.y - TempPoint.y;

        // FIX: Only set MouseDown - NEVER set MouseClicked manually!
		// MouseClicked is calculated internally by ImGui. Setting it manually corrupts state = CRASH!
		if (ShowMenu) { // Only force input when Menu is OPEN
			io.MouseDown[0] = (GetAsyncKeyState(0x1) & 0x8000) != 0; // VK_LBUTTON
		}
		else {
			io.MouseDown[0] = false;
		}

		if (TempRect.left != OldRect.left || TempRect.right != OldRect.right || TempRect.top != OldRect.top || TempRect.bottom != OldRect.bottom) {
			OldRect = TempRect;
			Process::WindowWidth = TempRect.right;
			Process::WindowHeight = TempRect.bottom;
			DirectX9Interface::pParams.BackBufferWidth = Process::WindowWidth;
			DirectX9Interface::pParams.BackBufferHeight = Process::WindowHeight;
			SetWindowPos(OverlayWindow::Hwnd, (HWND)0, TempPoint.x, TempPoint.y, Process::WindowWidth, Process::WindowHeight, SWP_NOREDRAW);
			DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		}
		Render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (DirectX9Interface::pDevice != NULL) {
		DirectX9Interface::pDevice->EndScene();
		DirectX9Interface::pDevice->Release();
	}
	if (DirectX9Interface::Direct3D9 != NULL) {
		DirectX9Interface::Direct3D9->Release();
	}
	DestroyWindow(OverlayWindow::Hwnd);
	UnregisterClass(OverlayWindow::WindowClass.lpszClassName, OverlayWindow::WindowClass.hInstance);
}

bool DirectXInit() {
	g_Logger.LogStep("DirectXInit() fonksiyonu basladi");
	
	// ========== DIRECT3D9 INTERFACE OLUSTURMA ==========
	g_Logger.LogStep("Direct3DCreate9Ex cagiriliyor...");
	HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &DirectX9Interface::Direct3D9);
	
	if (FAILED(hr)) {
		std::ostringstream errorMsg;
		errorMsg << "HRESULT: 0x" << std::hex << hr << " - Direct3D9 interface olusturulamadi";
		g_Logger.LogError("D3D9_CREATE_FAILED", 
			errorMsg.str(),
			"1. DirectX 9.0c End-User Runtime kurun: https://www.microsoft.com/download/details.aspx?id=35\n"
			"   2. d3d9.dll dosyasinin C:\\Windows\\System32 klasorunde oldugunu kontrol edin\n"
			"   3. Ekran kartinizin DirectX 9 destekleyip desteklemedigini kontrol edin\n"
			"   4. SFC /SCANNOW komutunu yonetici CMD'de calistirin");
		return false;
	}
	g_Logger.LogStep("Direct3D9 interface olusturuldu");
	g_Logger.LogValue("Direct3D9 Pointer", (void*)DirectX9Interface::Direct3D9);

	// ========== ADAPTER BILGILERINI LOG'LA ==========
	D3DADAPTER_IDENTIFIER9 adapterInfo;
	if (SUCCEEDED(DirectX9Interface::Direct3D9->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &adapterInfo))) {
		g_Logger.LogValue("Ekran Karti", std::string(adapterInfo.Description));
		g_Logger.LogValue("Surucu", std::string(adapterInfo.Driver));
		std::ostringstream driverVersion;
		driverVersion << HIWORD(adapterInfo.DriverVersion.HighPart) << "." 
					  << LOWORD(adapterInfo.DriverVersion.HighPart) << "."
					  << HIWORD(adapterInfo.DriverVersion.LowPart) << "."
					  << LOWORD(adapterInfo.DriverVersion.LowPart);
		g_Logger.LogValue("Surucu Surumu", driverVersion.str());
	}

	// ========== PRESENT PARAMETERS ==========
	g_Logger.LogStep("D3DPRESENT_PARAMETERS ayarlaniyor...");
	D3DPRESENT_PARAMETERS Params = { 0 };
	Params.Windowed = TRUE;
	Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	Params.hDeviceWindow = OverlayWindow::Hwnd;
	Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	Params.BackBufferWidth = 0; // Auto-detect from Window
	Params.BackBufferHeight = 0; // Auto-detect from Window
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.EnableAutoDepthStencil = FALSE; 
	Params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	Params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	
	g_Logger.LogValue("Windowed Mode", Params.Windowed ? "TRUE (Dogru)" : "FALSE (Yanlis!)");
	g_Logger.LogValue("Device Window (HWND)", (void*)Params.hDeviceWindow);

	// ========== DEVICE OLUSTURMA - HARDWARE ==========
	g_Logger.LogStep("Hardware Vertex Processing ile Device olusturuluyor...");
	hr = DirectX9Interface::Direct3D9->CreateDeviceEx(
		D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, 
		OverlayWindow::Hwnd, 
		D3DCREATE_HARDWARE_VERTEXPROCESSING, 
		&Params, 
		0, 
		&DirectX9Interface::pDevice
	);
	
	if (FAILED(hr)) {
		std::ostringstream hwError;
		hwError << "Hardware VP basarisiz - HRESULT: 0x" << std::hex << hr;
		g_Logger.LogStep(hwError.str(), false);
		
		// ========== FALLBACK TO SOFTWARE ==========
		g_Logger.LogStep("Software Vertex Processing deneniyor (Fallback)...");
		hr = DirectX9Interface::Direct3D9->CreateDeviceEx(
			D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL, 
			OverlayWindow::Hwnd, 
			D3DCREATE_SOFTWARE_VERTEXPROCESSING, 
			&Params, 
			0, 
			&DirectX9Interface::pDevice
		);
		
		if (FAILED(hr)) {
			std::ostringstream swError;
			swError << "Software VP de basarisiz - HRESULT: 0x" << std::hex << hr;
			
			// Detayli hata analizi
			std::string solution;
			switch(hr) {
				case D3DERR_INVALIDCALL:
					solution = "Gecersiz parametre. Overlay penceresi dogru olusturulmus mu kontrol edin.";
					break;
				case D3DERR_NOTAVAILABLE:
					solution = "Bu ozellik ekran kartiniz tarafindan desteklenmiyor. Farkli bir PC deneyin.";
					break;
				case D3DERR_OUTOFVIDEOMEMORY:
					solution = "Ekran karti bellegi yetersiz. Diger uygulamalari kapatin.";
					break;
				default:
					solution = "1. Visual C++ 2015-2022 Redistributable (x86) kurun\n"
							   "   2. DirectX End-User Runtime kurun\n"
							   "   3. Ekran karti suruculerini guncelleyin\n"
							   "   4. Windows Updates kontrol edin";
			}
			
			g_Logger.LogError("D3D9_DEVICE_FAILED", swError.str(), solution);
			DirectX9Interface::Direct3D9->Release();
			return false;
		}
		g_Logger.LogStep("Software Vertex Processing ile Device olusturuldu");
	} else {
		g_Logger.LogStep("Hardware Vertex Processing ile Device olusturuldu");
	}
	
	g_Logger.LogValue("Device Pointer", (void*)DirectX9Interface::pDevice);

	// ========== IMGUI CONTEXT ==========
	g_Logger.LogStep("ImGui Context olusturuluyor...");
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	g_Logger.LogStep("ImGui Context olusturuldu");

	// ========== FONT YUKLEME ==========
	g_Logger.LogStep("Varsayilan font yukleniyor...");
	io.Fonts->AddFontDefault();
	g_Logger.LogStep("Font yuklendi");

	// ========== IMGUI INIT ==========
	g_Logger.LogStep("ImGui_ImplWin32_Init cagiriliyor...");
	if (!ImGui_ImplWin32_Init(OverlayWindow::Hwnd)) {
		g_Logger.LogError("IMGUI_WIN32_FAILED", 
			"ImGui_ImplWin32_Init basarisiz oldu.",
			"Overlay penceresi dogru olusturulmus mu kontrol edin.");
	}
	g_Logger.LogStep("ImGui_ImplWin32_Init tamamlandi");
	
	g_Logger.LogStep("ImGui_ImplDX9_Init cagiriliyor...");
	if (!ImGui_ImplDX9_Init(DirectX9Interface::pDevice)) {
		g_Logger.LogError("IMGUI_DX9_FAILED", 
			"ImGui_ImplDX9_Init basarisiz oldu.",
			"DirectX Device dogru olusturulmus mu kontrol edin.");
	}
	g_Logger.LogStep("ImGui_ImplDX9_Init tamamlandi");
	
	DirectX9Interface::Direct3D9->Release();
	DirectX9Interface::Direct3D9 = NULL; // CRITICAL FIX: Prevent double-release in WM_DESTROY
	g_Logger.LogSuccess("DirectXInit() basariyla tamamlandi!");
	return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message) {
	case WM_DESTROY:
		if (DirectX9Interface::pDevice != NULL) {
			DirectX9Interface::pDevice->EndScene();
			DirectX9Interface::pDevice->Release();
		}
		if (DirectX9Interface::Direct3D9 != NULL) {
			DirectX9Interface::Direct3D9->Release();
		}
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (DirectX9Interface::pDevice != NULL && wParam != SIZE_MINIMIZED) {
			ImGui_ImplDX9_InvalidateDeviceObjects();
			DirectX9Interface::pParams.BackBufferWidth = LOWORD(lParam);
			DirectX9Interface::pParams.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
			if (SUCCEEDED(hr)) {
				ImGui_ImplDX9_CreateDeviceObjects();
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void SetupWindow() {
	g_Logger.LogStep("SetupWindow() fonksiyonu basladi");
	
	// Random Class Name Generator for Anti-Detection
	g_Logger.LogStep("Rastgele pencere ismi olusturuluyor...");
	static std::string randomName;
	if (randomName.empty()) {
		const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		for (int i = 0; i < 16; ++i) randomName += charset[rand() % (sizeof(charset) - 1)];
	}
	g_Logger.LogValue("Rastgele Pencere Ismi", randomName);
	
	OverlayWindow::Name = randomName.c_str(); // Set Random Name

	g_Logger.LogStep("WNDCLASSEX yapisi ayarlaniyor...");
	OverlayWindow::WindowClass = {
		sizeof(WNDCLASSEX), 0, WinProc, 0, 0, nullptr, LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, OverlayWindow::Name, LoadIcon(nullptr, IDI_APPLICATION)
	};
	
	g_Logger.LogStep("RegisterClassEx cagiriliyor...");
	ATOM classAtom = RegisterClassEx(&OverlayWindow::WindowClass);
	if (classAtom == 0) {
		DWORD lastError = GetLastError();
		std::ostringstream errMsg;
		errMsg << "RegisterClassEx basarisiz - GetLastError: " << lastError;
		g_Logger.LogError("REGISTER_CLASS_FAILED", errMsg.str(), 
			"Pencere sinifi zaten kayitli olabilir veya sistem kaynaklari yetersiz.");
	} else {
		g_Logger.LogStep("RegisterClassEx basarili");
	}
	
	if (Process::Hwnd){
		g_Logger.LogStep("Oyun penceresi boyutlari aliniyor...");
		static RECT TempRect = { NULL };
		static POINT TempPoint;
		GetClientRect(Process::Hwnd, &TempRect);
		ClientToScreen(Process::Hwnd, &TempPoint);
		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		Process::WindowWidth = TempRect.right;
		Process::WindowHeight = TempRect.bottom;
		
		g_Logger.LogValue("Process Window Width", (int)Process::WindowWidth);
		g_Logger.LogValue("Process Window Height", (int)Process::WindowHeight);
		g_Logger.LogValue("Process Window Left", (int)TempRect.left);
		g_Logger.LogValue("Process Window Top", (int)TempRect.top);
	} else {
		g_Logger.LogError("PROCESS_HWND_NULL", 
			"Process::Hwnd NULL - Oyun penceresi referansi yok.",
			"Oyunun dogru yuklenmis oldugundan emin olun.");
	}

	g_Logger.LogStep("Overlay penceresi olusturuluyor (CreateWindowEx)...");
	OverlayWindow::Hwnd = CreateWindowEx(NULL, OverlayWindow::Name, OverlayWindow::Name, WS_POPUP | WS_VISIBLE, Process::WindowLeft, Process::WindowTop, Process::WindowWidth, Process::WindowHeight, NULL, NULL, 0, NULL);
	
	if (OverlayWindow::Hwnd == NULL) {
		DWORD lastError = GetLastError();
		std::ostringstream errMsg;
		errMsg << "CreateWindowEx basarisiz - GetLastError: " << lastError;
		g_Logger.LogError("CREATE_WINDOW_FAILED", errMsg.str(),
			"1. Yonetici olarak calistirdiginizdan emin olun\n"
			"   2. Antivirus yaziliminizi kontrol edin\n"
			"   3. Sistem kaynaklarini kontrol edin");
	} else {
		g_Logger.LogStep("Overlay penceresi olusturuldu");
		g_Logger.LogValue("Overlay HWND", (void*)OverlayWindow::Hwnd);
	}
	
	g_Logger.LogStep("DwmExtendFrameIntoClientArea cagiriliyor...");
	HRESULT hr = DwmExtendFrameIntoClientArea(OverlayWindow::Hwnd, &DirectX9Interface::Margin);
	if (FAILED(hr)) {
		std::ostringstream errMsg;
		errMsg << "DWM basarisiz - HRESULT: 0x" << std::hex << hr;
		g_Logger.LogStep(errMsg.str(), false);
	} else {
		g_Logger.LogStep("DwmExtendFrameIntoClientArea basarili");
	}
	
	g_Logger.LogStep("SetWindowLong ile stil ayarlaniyor...");
	SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);
	g_Logger.LogStep("Pencere stili ayarlandi (WS_EX_LAYERED | WS_EX_TRANSPARENT)");
	
	ShowWindow(OverlayWindow::Hwnd, SW_SHOW);
	UpdateWindow(OverlayWindow::Hwnd);
	g_Logger.LogSuccess("SetupWindow() basariyla tamamlandi!");
}

void GetWindowRect() {
	g_Logger.LogStep("GetWindowRect() fonksiyonu basladi");
	g_Logger.LogValue("hwnd1 (Oyun Penceresi)", (void*)hwnd1);
	
	RECT clientRect;
	if (GetClientRect(hwnd1, &clientRect)) {
		int clientWidth = clientRect.right - clientRect.left;
		int clientHeight = clientRect.bottom - clientRect.top;

		width = clientWidth;
		height = clientHeight;
		
		g_Logger.LogValue("Client Width", clientWidth);
		g_Logger.LogValue("Client Height", clientHeight);
		g_Logger.LogStep("GetWindowRect() basarili");
	}
	else {
		DWORD lastError = GetLastError();
		std::ostringstream errMsg;
		errMsg << "GetClientRect basarisiz - GetLastError: " << lastError << " - hwnd1: 0x" << std::hex << (uintptr_t)hwnd1;
		g_Logger.LogError("GET_WINDOW_RECT_FAILED", errMsg.str(),
			"1. Oyun penceresi kapanmis veya minimize edilmis olabilir\n"
			"   2. Oyunu yeniden baslatin ve PENCERELI modda acin\n"
			"   3. Hileyi baslattiktan hemen sonra oyunu kapatmayin");
		MessageBox(nullptr, "Pencere boyutu alinamadi!\n\nOyunu PENCERELI modda actiginizdan emin olun.\n\n[HATA KODU: GET_WINDOW_RECT_FAILED]", "BEYAZ KOLYE VIP", MB_ICONERROR);
		exit(0);
	}
}



void ConfigAutoLoad() {
	// Removed continuous config loading so settings can be changed in the menu.
	// We only load once at startup now.
}

// DEBUG MODE: Set to true to see detailed startup messages
// Disable this before release!
#define DEBUG_MODE false

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	RANDOM_OPS; // Execute random junk code at startup
	bool WindowFocus = false;
	setlocale(LC_ALL, "Russian"); 

	// ========== BASLANGIÇ LOGLARI ==========
	g_Logger.LogStep("Uygulama baslatiliyor...");
	g_Logger.LogSystemInfo();
	
	if(DEBUG_MODE) MessageBox(NULL, "STEP 1: Starting...", "DEBUG", MB_OK);

	// ========== LOADER MUTEX KONTROLU (REMOVED) ==========
	// Mutex check bypassed for standalone version



	// ========== OYUN PROCESS KONTROLU ==========
	g_Logger.LogStep("hl.exe aranıyor...");
	if (!IsProcessAlive("hl.exe")) {
		g_Logger.LogError("HL_NOT_FOUND", 
			"hl.exe isimli process bulunamadi.",
			"1. CS 1.6 oyununu PENCERELI modda acin\n   2. Oyun tamamen yuklenene kadar bekleyin\n   3. Sonra hileyi tekrar calistirin");
		MessageBox(nullptr, "Oyun bulunamadi!\n\n1. CS 1.6'yi PENCERELI modda acin\n2. Sonra hileyi tekrar calistirin\n\n[HATA KODU: HL_NOT_FOUND]", "BEYAZ KOLYE VIP", MB_ICONERROR);
		exit(0);
	}
	g_Logger.LogStep("hl.exe bulundu");

	if(DEBUG_MODE) MessageBox(NULL, "STEP 3: hl.exe Found", "DEBUG", MB_OK);

	// ========== PENCERE HANDLE KONTROLU ==========
	g_Logger.LogStep("Oyun penceresi aranıyor...");
	hwnd1 = GetProcessHwnd();
	g_Logger.LogValue("hwnd1", (void*)hwnd1);
	
	if(hwnd1 == NULL) {
		g_Logger.LogError("HWND_NULL", 
			"Oyun penceresi bulunamadi (HWND = NULL).",
			"1. Oyunu TAM EKRAN degil, PENCERELI modda acin\n   2. Video ayarlarindan 'Windowed' secin\n   3. Oyun acildiktan sonra 1-2 saniye bekleyin");
		MessageBox(NULL, "Oyun penceresi bulunamadi!\n\nCS 1.6'yi PENCERELI modda actiginizdan emin olun.\nTam ekran DESTEKLENMEZ!\n\n[HATA KODU: HWND_NULL]", "BEYAZ KOLYE VIP", MB_ICONERROR);
		exit(0);
	}
	g_Logger.LogStep("Oyun penceresi bulundu");

	if(DEBUG_MODE) MessageBox(NULL, "STEP 4: Window Handle OK", "DEBUG", MB_OK);

	// ========== PROCESS ID ==========
	id = GetWindowThreadProcessId(hwnd1, &Game::PID);
	g_Logger.LogValue("Game PID", Game::PID);
	g_Logger.LogGameInfo(hwnd1, Game::PID);

	// ========== MEMORY OBJECT INITIALIZATION ==========
	// CRITICAL FIX: Create Memory object HERE, not at global scope!
	// Global scope runs before main() - game might not be running yet = crash
	g_Logger.LogStep("Memory nesnesi olusturuluyor...");
	g_memory = new Memory(xorstr_("hl.exe"));
	if (!g_memory || !g_memory->IsValid()) {
		g_Logger.LogError("MEMORY_INIT_FAILED", "Memory nesnesi olusturulamadi.", "Yeniden deneyin.");
		MessageBox(NULL, "Bellek hatasi!", "EVELION", MB_ICONERROR);
		exit(0);
	}
	g_Logger.LogStep("Memory nesnesi olusturuldu");
	
	// Get module addresses NOW (after process is confirmed)
	hw = g_memory->GetModuleAddress(xorstr_("hw.dll"));
	client = g_memory->GetModuleAddress(xorstr_("client.dll"));
	
	// ========== MODUL ADRESLERI ==========
	g_Logger.LogStep("Modul adresleri okunuyor...");
	g_Logger.LogModuleInfo("hw.dll", hw);
	g_Logger.LogModuleInfo("client.dll", client);
	
	if (hw == 0) {
		g_Logger.LogError("HW_DLL_NOT_FOUND", 
			"hw.dll modulu bulunamadi. Oyun yanlis yuklenmis olabilir.",
			"1. Oyunu yeniden baslatin\n   2. Farkli bir CS 1.6 surumu deneyin");
	}
	if (client == 0) {
		g_Logger.LogError("CLIENT_DLL_NOT_FOUND", 
			"client.dll modulu bulunamadi.",
			"1. Oyunu yeniden baslatin\n   2. Farkli bir CS 1.6 surumu deneyin");
	}
	
	// ========== STEAM BETA TESPITI ==========
	g_Logger.LogSection("STEAM / OYUN SURUMU ANALIZI");
	g_Logger.LogSteamBetaInfo();

	// ========== PENCERE BOYUTU ==========
	g_Logger.LogStep("Pencere boyutu okunuyor...");
	GetWindowRect();
	g_Logger.LogValue("Pencere Genisligi", width);
	g_Logger.LogValue("Pencere Yuksekligi", height);
	
	if (width == 0 || height == 0) {
		g_Logger.LogError("WINDOW_SIZE_ZERO", 
			"Pencere boyutu 0x0 olarak okundu.",
			"Oyun pencereli modda degil veya minimize edilmis olabilir.");
	}

	// ========== CONFIG YUKLEME (SADECE BIR KERE) ==========
	Config::Load();
	g_Logger.LogStep("Config yuklendi");

	// ========== THREAD'LER BASLATILIYOR ==========
	g_Logger.LogStep("Thread'ler baslatiliyor...");
	std::thread aliveThread(ProcessAlive);
	g_Logger.LogStep("  -> ProcessAlive thread OK");

	std::thread matrixThread(MatrixUpdate);
	g_Logger.LogStep("  -> MatrixUpdate thread OK");
	
	std::thread updateThread(OffsetsUpdate);
	g_Logger.LogStep("  -> OffsetsUpdate thread OK");
	
	std::thread lobbyThread(LobbyCheck);
	g_Logger.LogStep("  -> LobbyCheck thread OK");
	
	// std::thread loaderCheckThread(LoaderCheckThread);
	// g_Logger.LogStep("  -> LoaderCheckThread OK");

	// ========== OYUN ODAKLANMASI BEKLENIYOR ==========
	g_Logger.LogStep("Oyun penceresinin on plana gelmesi bekleniyor...");
	int focusWaitCounter = 0;
	while (WindowFocus == false) {
		DWORD ForegroundWindowProcessID;
		GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
		if (Game::PID == ForegroundWindowProcessID) {
			Process::ID = Game::PID;
			Process::Hwnd = hwnd1;
			// CRITICAL FIX: Actually open process handle (was never set before = crash)
			Process::Handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, Game::PID);

			RECT TempRect;
			GetWindowRect(Process::Hwnd, &TempRect);
			Process::WindowWidth = TempRect.right - TempRect.left;
			Process::WindowHeight = TempRect.bottom - TempRect.top;
			Process::WindowLeft = TempRect.left;
			Process::WindowRight = TempRect.right;
			Process::WindowTop = TempRect.top;
			Process::WindowBottom = TempRect.bottom;

			char TempTitle[MAX_PATH];
			GetWindowText(Process::Hwnd, TempTitle, sizeof(TempTitle));
			Process::Title = TempTitle;

			char TempClassName[MAX_PATH];
			GetClassName(Process::Hwnd, TempClassName, sizeof(TempClassName));
			Process::ClassName = TempClassName;

			char TempPath[MAX_PATH];
			GetModuleFileNameEx(Process::Handle, NULL, TempPath, sizeof(TempPath));
			Process::Path = TempPath;

			WindowFocus = true;
			g_Logger.LogStep("Oyun penceresi odaklandi");
			g_Logger.LogValue("Pencere Basligi", std::string(TempTitle));
		}
		
		focusWaitCounter++;
		if (focusWaitCounter > 300) { // 30 saniye bekle max
			g_Logger.LogError("FOCUS_TIMEOUT", 
				"Oyun penceresi 30 saniye icerisinde odaklanamadi.",
				"Oyunu on plana getirin (Alt+Tab ile)");
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	
	// ========== CONFIG ALREADY LOADED BEFORE THREADS ==========
	// Config::Load() was already called once before threads started.
	// DO NOT load again here - it causes race conditions with running threads.
	g_Logger.LogStep("Config zaten yuklu, ikinci yukleme atlanıyor...");
	if(DEBUG_MODE) MessageBox(NULL, "STEP 5: Config already loaded", "DEBUG", MB_OK);
	
	// ========== OVERLAY PENCERESI ==========
	g_Logger.LogStep("Overlay penceresi olusturuluyor...");
	if(DEBUG_MODE) MessageBox(NULL, "STEP 6: Setting up Window...", "DEBUG", MB_OK);
	SetupWindow();
	g_Logger.LogStep("Overlay penceresi olusturuldu");
	g_Logger.LogValue("Overlay HWND", (void*)OverlayWindow::Hwnd);
	
	// ========== DIRECTX BASLATMA ==========
	g_Logger.LogStep("DirectX 9 baslatiliyor...");
	if(DEBUG_MODE) MessageBox(NULL, "STEP 7: Initializing DirectX...", "DEBUG", MB_OK);
	
	if (!DirectXInit()) {
		g_Logger.LogError("DX9_INIT_FAILED", 
			"DirectX 9 baslatilamadi. Direct3DCreate9Ex veya CreateDeviceEx basarisiz.",
			"1. DirectX End-User Runtime yukleyin: microsoft.com/download/details.aspx?id=35\n   2. Visual C++ 2015-2022 Redistributable (x86) yukleyin: aka.ms/vs/17/release/vc_redist.x86\n   3. Ekran karti suruculerini guncelleyin\n   4. Bilgisayari yeniden baslatin");
		MessageBox(NULL, "DirectX baslatilamadi!\n\nCOZUM:\n1. DirectX End-User Runtime yukleyin:\n   microsoft.com/download/details.aspx?id=35\n\n2. Visual C++ 2015-2022 Redistributable (x86) yukleyin\n\n3. Bilgisayari yeniden baslatin\n\n[HATA KODU: DX9_INIT_FAILED]", "BEYAZ KOLYE VIP", MB_ICONERROR);
		return 1;
	}
	
	g_Logger.LogSuccess("TUM KONTROLLER BASARILI! Hile calistirildi.");
	g_Logger.LogStep("Ana dongu baslatiliyor...");
	
	while (TRUE) {
		MainLoop();
	}
}
