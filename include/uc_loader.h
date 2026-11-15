/**
 *  I know it's named dll_loader.h, but in reality it does more than just that.
 *  I added the other features after I had made it and established it would be
 *  just a dll injector / loader and didn't realize my fuckup. I'm gonna try and
 *  change it at some point and hope it doesn't break anything later on down the
 *  road. Right now, this handles the injection loader, the ini configuration,
 *  and the appid customization stuff. And now, it handles the steam stub loading
 *  and live patching as well. The actual code can be found at the end of dllmain.cpp.
 *
 *  ~veeλnti<3 2026
 */

/** 
 *  Finally got around to this, lol.
 *  - 5/18/2026
 * 
 *  ~vλ<3
 */
#pragma once

#include <Windows.h>
#include <Shlwapi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>

void UCOLOG(const char* fmt, ...);

class CDLLLoader
{
private:
	std::vector<HMODULE> m_Modules;
	char m_IniPath[MAX_PATH];

	bool GetExecutableDirectory(char* outDir, size_t outSize)
	{
		if (!outDir || outSize == 0)
			return false;

		char exePath[MAX_PATH] = { 0 };
		DWORD len = GetModuleFileNameA(nullptr, exePath, MAX_PATH);
		if (len == 0 || len >= MAX_PATH)
			return false;

		if (!PathRemoveFileSpecA(exePath))
			return false;

		return _snprintf_s(outDir, outSize, _TRUNCATE, "%s", exePath) != -1;
	}

	uint32 ParseAppId(const char* text)
	{
		if (!text)
			return 0;

		while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n')
			text++;

		char* end = nullptr;
		unsigned long id = std::strtoul(text, &end, 10);
		if (end == text)
			return 0;

		while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')
			end++;

		return (*end == '\0') ? (uint32)id : 0;
	}

public:
	CDLLLoader() { m_IniPath[0] = '\0'; }
	~CDLLLoader() { UnloadAll(); }

	void ReadConfig()
	{
		char exeDir[MAX_PATH] = { 0 };
		DWORD len = GetModuleFileNameA(nullptr, exeDir, MAX_PATH);
		if (len == 0) return;
		if (!PathRemoveFileSpecA(exeDir)) return;

		_snprintf_s(m_IniPath, MAX_PATH, _TRUNCATE, "%s\\union-crax.ini", exeDir);

		DWORD attribs = GetFileAttributesA(m_IniPath);
		if (attribs == INVALID_FILE_ATTRIBUTES)
			m_IniPath[0] = '\0';
	}

	uint32 GetAppId()
	{
		char exeDir[MAX_PATH] = { 0 };
		if (!GetExecutableDirectory(exeDir, MAX_PATH))
			return 0;

		char appIdPath[MAX_PATH] = { 0 };
		if (_snprintf_s(appIdPath, MAX_PATH, _TRUNCATE, "%s\\steam_appid.txt", exeDir) == -1)
			return 0;

		DWORD attribs = GetFileAttributesA(appIdPath);
		if (attribs == INVALID_FILE_ATTRIBUTES || (attribs & FILE_ATTRIBUTE_DIRECTORY))
			return 0;

		FILE* f = nullptr;
		if (fopen_s(&f, appIdPath, "rb") != 0 || !f)
			return 0;

		char buf[64] = { 0 };
		size_t bytesRead = fread(buf, 1, sizeof(buf) - 1, f);
		fclose(f);
		buf[bytesRead] = '\0';

		return ParseAppId(buf);
	}

uint32 GetOgAppId()
  	{
  		if (m_IniPath[0] == '\0')
  	        return 0;

  		char buf[16] = { 0 };
  		GetPrivateProfileStringA("Settings", "ogAppId", "", buf, sizeof(buf), m_IniPath);

  		if (buf[0] == '\0')
  			return 0;

  		uint32 id = (uint32)strtoul(buf, nullptr, 10);
  		return id;
  	}

	std::vector<uint32> GetUnlockDLCAppIds()
	{
		std::vector<uint32> appIds;
		if (m_IniPath[0] == '\0')
			return appIds;

		char buf[1024] = { 0 };
		GetPrivateProfileStringA("Settings", "UnlockDLC", "", buf, sizeof(buf), m_IniPath);

		if (buf[0] == '\0')
			return appIds;

		char* token = strtok(buf, ",");
		while (token != nullptr)
		{
			while (*token == ' ' || *token == '\t') token++;
			if (token[0] != '\0')
			{
				uint32 id = (uint32)strtoul(token, nullptr, 10);
				if (id != 0)
					appIds.push_back(id);
			}
			token = strtok(nullptr, ",");
		}

		return appIds;
	}

	bool GetEmulateTicketEnabled()
	{
		if (m_IniPath[0] == '\0')
			return false;

		char buf[8] = { 0 };
		GetPrivateProfileStringA("Settings", "EmulateTicket", "false", buf, sizeof(buf), m_IniPath);

		return (_stricmp(buf, "true") == 0 || _stricmp(buf, "1") == 0 || _stricmp(buf, "yes") == 0);
	}

	bool GetSteamStubEnabled()
	{
		if (m_IniPath[0] == '\0')
			return false;

		char buf[8] = { 0 };
		GetPrivateProfileStringA("Settings", "GetStubbedLol", "false", buf, sizeof(buf), m_IniPath);

		return (_stricmp(buf, "true") == 0 || _stricmp(buf, "1") == 0 || _stricmp(buf, "yes") == 0);
	}

	uint64 GetCustomSteamID()
	{
		if (m_IniPath[0] == '\0')
			return 0;

		char buf[32] = { 0 };
		GetPrivateProfileStringA("User", "SteamID", "", buf, sizeof(buf), m_IniPath);

		if (buf[0] == '\0')
			return 0;

		char* end = nullptr;
		unsigned long long id = std::strtoull(buf, &end, 10);
		if (end == buf)
			return 0;

		return id;
	}

	const char* GetCustomPersonaName()
	{
		if (m_IniPath[0] == '\0')
			return "UCOfflineUser";

		static char name[128] = { 0 };
		GetPrivateProfileStringA("User", "PersonaName", "UCOfflineUser", name, sizeof(name), m_IniPath);
		return name;
	}

	uint32 GetCustomSteamLevel()
	{
		if (m_IniPath[0] == '\0')
			return 0;

		char buf[16] = { 0 };
		GetPrivateProfileStringA("User", "SteamLevel", "0", buf, sizeof(buf), m_IniPath);

		return (uint32)strtoul(buf, nullptr, 10);
	}

	const char* GetCustomIPCountry()
	{
		if (m_IniPath[0] == '\0')
			return "US";

		static char country[8] = { 0 };
		GetPrivateProfileStringA("User", "IPCountry", "US", country, sizeof(country), m_IniPath);
		return country;
	}

	bool GetBLoggedOn()
	{
		if (m_IniPath[0] == '\0')
			return true;

		char buf[8] = { 0 };
		GetPrivateProfileStringA("User", "BLoggedOn", "true", buf, sizeof(buf), m_IniPath);

		return !(_stricmp(buf, "false") == 0 || _stricmp(buf, "0") == 0);
	}

	bool GetForceOwnership()
	{
    	if (m_IniPath[0] == '\0')
        	return true;
    
    	char buf[8] = { 0 };
    		GetPrivateProfileStringA("Settings", "ForceOwnership", "true", buf, sizeof(buf), m_IniPath);
    	return (_stricmp(buf, "true") == 0);
	}

	void LoadPlugins()
	{
		if (m_IniPath[0] == '\0')
			return;

		char exeDir[MAX_PATH] = { 0 };
		GetModuleFileNameA(nullptr, exeDir, MAX_PATH);
		PathRemoveFileSpecA(exeDir);

		char folderName[MAX_PATH] = { 0 };
		GetPrivateProfileStringA("Settings", "PluginsFolder", "", folderName, MAX_PATH, m_IniPath);

		if (folderName[0] == '\0')
			return;

		char dllPath[MAX_PATH] = { 0 };
		if (_snprintf_s(dllPath, MAX_PATH, _TRUNCATE, "%s\\%s", exeDir, folderName) == _TRUNCATE)
			return;

		DWORD folderAttribs = GetFileAttributesA(dllPath);
		if (folderAttribs == INVALID_FILE_ATTRIBUTES || !(folderAttribs & FILE_ATTRIBUTE_DIRECTORY))
			return;

		char findPattern[MAX_PATH] = { 0 };
		if (_snprintf_s(findPattern, MAX_PATH, _TRUNCATE, "%s\\*.dll", dllPath) == _TRUNCATE)
			return;

		std::vector<std::string> names;
		std::vector<std::string> paths;

		WIN32_FIND_DATAA fd = { 0 };
		HANDLE hFind = FindFirstFileA(findPattern, &fd);

		if (hFind == INVALID_HANDLE_VALUE)
			return;

		do
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			char fullPath[MAX_PATH] = { 0 };
			if (_snprintf_s(fullPath, MAX_PATH, _TRUNCATE, "%s\\%s", dllPath, fd.cFileName) == _TRUNCATE)
				continue;

			names.push_back(fd.cFileName);
			paths.push_back(fullPath);
		} while (FindNextFileA(hFind, &fd));

		FindClose(hFind);

		for (size_t i = 0; i < names.size(); i++)
		{
			size_t minIdx = i;
			for (size_t j = i + 1; j < names.size(); j++)
			{
				if (_stricmp(names[j].c_str(), names[minIdx].c_str()) < 0)
					minIdx = j;
			}

			if (minIdx != i)
			{
				std::swap(names[i], names[minIdx]);
				std::swap(paths[i], paths[minIdx]);
			}
		}

		for (size_t i = 0; i < names.size(); i++)
		{
			HMODULE hMod = LoadLibraryExA(paths[i].c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
			if (hMod)
			{
				m_Modules.push_back(hMod);
				UCOLOG("[UCOffline] Loaded plugin: %s", names[i].c_str());
			}
			else
			{
				UCOLOG("[UCOffline] Failed to load plugin: %s (error %lu)", names[i].c_str(), GetLastError());
			}
		}
	}

	void UnloadAll()
	{
		for (size_t i = 0; i < m_Modules.size(); i++)
		{
			if (m_Modules[i])
				FreeLibrary(m_Modules[i]);
		}
		m_Modules.clear();
	}

	size_t LoadedCount() const { return m_Modules.size(); }
};
