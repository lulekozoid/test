#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include "struct.h"
#include "xorstr.hpp"

// Translation Helper
const char* t(const char* en, const char* tr) {
    return (language == 1) ? tr : en;
}

class Config {
private:
    static std::string GetPath() {
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        std::string::size_type pos = std::string(path).find_last_of(xorstr_("\\/"));
        return std::string(path).substr(0, pos) + xorstr_("\\settings.ini");
    }

    static void WriteInt(const char* section, const char* key, int val) {
        WritePrivateProfileStringA(section, key, std::to_string(val).c_str(), GetPath().c_str());
    }

    static void WriteFloat(const char* section, const char* key, float val) {
        WritePrivateProfileStringA(section, key, std::to_string(val).c_str(), GetPath().c_str());
    }

    static int ReadInt(const char* section, const char* key, int def) {
        return GetPrivateProfileIntA(section, key, def, GetPath().c_str());
    }

    static float ReadFloat(const char* section, const char* key, float def) {
        char result[255];
        GetPrivateProfileStringA(section, key, std::to_string(def).c_str(), result, 255, GetPath().c_str());
        return std::stof(result);
    }

public:
    static void Save() {
        // Visuals
        WriteInt(xorstr_("Visuals"), xorstr_("MasterSwitch"), esp);
        WriteInt(xorstr_("Visuals"), xorstr_("Box"), enemy_box);
        WriteInt(xorstr_("Visuals"), xorstr_("Name"), enemy_name);
        WriteInt(xorstr_("Visuals"), xorstr_("EnemyOnly"), def_models);
        
        // Aimbot
        WriteInt(xorstr_("Aimbot"), xorstr_("Active"), aimbot_active);
        WriteFloat(xorstr_("Aimbot"), xorstr_("FOV"), aimbot_fov);
        WriteFloat(xorstr_("Aimbot"), xorstr_("Smooth"), aimbot_smooth);
        WriteFloat(xorstr_("Aimbot"), xorstr_("HeadOffset"), aimbot_bone_z);
        WriteInt(xorstr_("Aimbot"), xorstr_("Key"), aimbot_key);

        // System
        WriteInt(xorstr_("System"), xorstr_("Language"), language);

        // Colors
        WriteFloat(xorstr_("Colors"), xorstr_("BoxR"), BoxColor.R);
        WriteFloat(xorstr_("Colors"), xorstr_("BoxG"), BoxColor.G);
        WriteFloat(xorstr_("Colors"), xorstr_("BoxB"), BoxColor.B);
        
        WriteFloat(xorstr_("Colors"), xorstr_("NameR"), NameColor.R);
        WriteFloat(xorstr_("Colors"), xorstr_("NameG"), NameColor.G);
        WriteFloat(xorstr_("Colors"), xorstr_("NameB"), NameColor.B);
    }

    static void Load() {
        // Visuals
        esp = ReadInt(xorstr_("Visuals"), xorstr_("MasterSwitch"), 1);
        enemy_box = ReadInt(xorstr_("Visuals"), xorstr_("Box"), 1);
        enemy_name = ReadInt(xorstr_("Visuals"), xorstr_("Name"), 1);
        def_models = ReadInt(xorstr_("Visuals"), xorstr_("EnemyOnly"), 1);

        // Aimbot
        aimbot_active = ReadInt(xorstr_("Aimbot"), xorstr_("Active"), 0);
        aimbot_fov = ReadFloat(xorstr_("Aimbot"), xorstr_("FOV"), 100.0f);
        aimbot_smooth = ReadFloat(xorstr_("Aimbot"), xorstr_("Smooth"), 5.0f);
        aimbot_bone_z = ReadFloat(xorstr_("Aimbot"), xorstr_("HeadOffset"), 22.0f);
        aimbot_key = ReadInt(xorstr_("Aimbot"), xorstr_("Key"), 0x01); // VK_LBUTTON

        // System
        language = ReadInt(xorstr_("System"), xorstr_("Language"), 0);

        // Colors
        BoxColor.R = ReadFloat(xorstr_("Colors"), xorstr_("BoxR"), 0);
        BoxColor.G = ReadFloat(xorstr_("Colors"), xorstr_("BoxG"), 231);
        BoxColor.B = ReadFloat(xorstr_("Colors"), xorstr_("BoxB"), 255);
        
        // Update temp arrays for ColorPicker
        boxTemp[0] = BoxColor.R / 255.0f;
        boxTemp[1] = BoxColor.G / 255.0f;
        boxTemp[2] = BoxColor.B / 255.0f;

        NameColor.R = ReadFloat(xorstr_("Colors"), xorstr_("NameR"), 0);
        NameColor.G = ReadFloat(xorstr_("Colors"), xorstr_("NameG"), 231);
        NameColor.B = ReadFloat(xorstr_("Colors"), xorstr_("NameB"), 255);
        
        nameTemp[0] = NameColor.R / 255.0f;
        nameTemp[1] = NameColor.G / 255.0f;
        nameTemp[2] = NameColor.B / 255.0f;
    }
};
