// My attempt at making an emu steamclient. Does it work? Who knows.
// 
// ~vee 2026

#include <Windows.h>
#include <conio.h>
#include <string>
#include <time.h>

#define STEAM_API_EXPORTS

#include "steam_api.h"
#include "steamclientpublic.h"
#include "steam_gameserver.h"
#include "steam_api_common.h"

// ---------------------------------------------------------------------------
// Stub implementations for ISteam* interfaces
// All return sensible defaults for pure-virtual methods.
// ---------------------------------------------------------------------------

static HSteamPipe s_NextPipe = 100;
static HSteamUser s_NextUser = 1000;
static uint32 s_AppID = 480;

// ---- ISteamUtils stub -----------------------------------------------------

class FakeSteamUtils : public ISteamUtils
{
public:
	uint32 GetSecondsSinceAppActive() override { return 0; }
	uint32 GetSecondsSinceComputerActive() override { return 0; }
	EUniverse GetConnectedUniverse() override { return k_EUniversePublic; }
	uint32 GetServerRealTime() override { return (uint32)time(nullptr); }
	const char *GetIPCountry() override { return "US"; }
	bool GetImageSize(int iImage, uint32 *pnWidth, uint32 *pnHeight) override { if (pnWidth) *pnWidth = 0; if (pnHeight) *pnHeight = 0; return false; }
	bool GetImageRGBA(int iImage, uint8 *pubDest, int nDestBufferSize) override { return false; }
	bool GetCSERIPPort(uint32 *unIP, uint16 *usPort) override { return false; }
	uint8 GetCurrentBatteryPower() override { return 255; }
	uint32 GetAppID() override { return s_AppID; }
	void SetOverlayNotificationPosition(ENotificationPosition eNotificationPosition) override {}
	bool IsAPICallCompleted(SteamAPICall_t hSteamAPICall, bool *pbFailed) override { if (pbFailed) *pbFailed = true; return false; }
	ESteamAPICallFailure GetAPICallFailureReason(SteamAPICall_t hSteamAPICall) override { return k_ESteamAPICallFailureNone; }
	bool GetAPICallResult(SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed) override { if (pbFailed) *pbFailed = true; return false; }
	void RunFrame() override {}
	uint32 GetIPCCallCount() override { return 0; }
	void SetWarningMessageHook(SteamAPIWarningMessageHook_t pFunction) override {}
	bool IsOverlayEnabled() override { return false; }
	bool BOverlayNeedsPresent() override { return false; }
	SteamAPICall_t CheckFileSignature(const char *szFileName) override { return k_uAPICallInvalid; }
	bool ShowGamepadTextInput(EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, uint32 unCharMax, const char *pchExistingText) override { return false; }
	uint32 GetEnteredGamepadTextLength() override { return 0; }
	bool GetEnteredGamepadTextInput(char *pchText, uint32 cchText) override { return false; }
	const char *GetSteamUILanguage() override { return "english"; }
	bool IsSteamRunningInVR() override { return false; }
	void SetOverlayNotificationInset(int nHorizontalInset, int nVerticalInset) override {}
	bool IsSteamInBigPictureMode() override { return false; }
	void StartVRDashboard() override {}
	bool IsVRHeadsetStreamingEnabled() override { return false; }
	void SetVRHeadsetStreamingEnabled(bool bEnabled) override {}
	bool IsSteamChinaLauncher() override { return false; }
	bool InitFilterText(uint32 unFilterOptions) override { return false; }
	int FilterText(ETextFilteringContext eContext, CSteamID sourceSteamID, const char *pchInputMessage, char *pchOutFilteredText, uint32 nByteSizeOutFilteredText) override
	{
		if (pchOutFilteredText && pchInputMessage)
			strcpy_s(pchOutFilteredText, nByteSizeOutFilteredText, pchInputMessage);
		return 0;
	}
	ESteamIPv6ConnectivityState GetIPv6ConnectivityState(ESteamIPv6ConnectivityProtocol eProtocol) override { return k_ESteamIPv6ConnectivityState_Good; }
	bool IsSteamRunningOnSteamDeck() override { return false; }
	bool ShowFloatingGamepadTextInput(EFloatingGamepadTextInputMode eKeyboardMode, int nTextFieldXPosition, int nTextFieldYPosition, int nTextFieldWidth, int nTextFieldHeight) override { return false; }
	void SetGameLauncherMode(bool bLauncherMode) override {}
	bool DismissFloatingGamepadTextInput() override { return false; }
	bool DismissGamepadTextInput() override { return false; }
};

// ---- ISteamUser stub ------------------------------------------------------

class FakeSteamUser : public ISteamUser
{
public:
	HSteamUser GetHSteamUser() override { return 0; }
	bool BLoggedOn() override { return true; }
	CSteamID GetSteamID() override { return CSteamID(76561197960265728ULL); }
	int InitiateGameConnection_DEPRECATED(void* pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure) override { return 0; }
	void TerminateGameConnection_DEPRECATED(uint32 unIPServer, uint16 usPortServer) override {}
	void TrackAppUsageEvent(CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo) override {}
	bool GetUserDataFolder(char *pchBuffer, int cubBuffer) override { return false; }
	void StartVoiceRecording() override {}
	void StopVoiceRecording() override {}
	EVoiceResult GetAvailableVoice(uint32 *pcbCompressed, uint32 *pcbUncompressed_Deprecated, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated) override { return k_EVoiceResultNotInitialized; }
	EVoiceResult GetVoice(bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed_Deprecated, void *pUncompressedDestBuffer_Deprecated, uint32 cbUncompressedDestBufferSize_Deprecated, uint32 *nUncompressBytesWritten_Deprecated, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated) override { return k_EVoiceResultNotInitialized; }
	EVoiceResult DecompressVoice(const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, uint32 nDesiredSampleRate) override { return k_EVoiceResultNotInitialized; }
	uint32 GetVoiceOptimalSampleRate() override { return 48000; }
	HAuthTicket GetAuthSessionTicket(void* pTicket, int cbMaxTicket, uint32* pcbTicket, const SteamNetworkingIdentity* pSteamNetworkingIdentity) override { return k_HAuthTicketInvalid; }
	HAuthTicket GetAuthTicketForWebApi(const char* pchIdentity) override { return k_HAuthTicketInvalid; }
	EBeginAuthSessionResult BeginAuthSession(const void *pAuthTicket, int cbAuthTicket, CSteamID steamID) override { return k_EBeginAuthSessionResultInvalidTicket; }
	void EndAuthSession(CSteamID steamID) override {}
	void CancelAuthTicket(HAuthTicket hAuthTicket) override {}
	EUserHasLicenseForAppResult UserHasLicenseForApp(CSteamID steamID, AppId_t appID) override { return k_EUserHasLicenseResultDoesNotHaveLicense; }
	bool BIsBehindNAT() override { return false; }
	void AdvertiseGame(CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer) override {}
	SteamAPICall_t RequestEncryptedAppTicket(void *pDataToInclude, int cbDataToInclude) override { return k_uAPICallInvalid; }
	bool GetEncryptedAppTicket(void *pTicket, int cbMaxTicket, uint32 *pcbTicket) override { return false; }
	int GetGameBadgeLevel(int nSeries, bool bFoil) override { return 0; }
	int GetPlayerSteamLevel() override { return 0; }
	SteamAPICall_t RequestStoreAuthURL(const char *pchRedirectURL) override { return k_uAPICallInvalid; }
	bool BIsPhoneVerified() override { return false; }
	bool BIsTwoFactorEnabled() override { return false; }
	bool BIsPhoneIdentifying() override { return false; }
	bool BIsPhoneRequiringVerification() override { return false; }
	SteamAPICall_t GetMarketEligibility() override { return k_uAPICallInvalid; }
	SteamAPICall_t GetDurationControl() override { return k_uAPICallInvalid; }
	bool BSetDurationControlOnlineState(EDurationControlOnlineState eNewState) override { return false; }
};

// ---- ISteamClient stub (main factory) -------------------------------------

class FakeSteamClient : public ISteamClient
{
public:
	FakeSteamUtils* m_pUtils;
	FakeSteamUser* m_pUser;

	FakeSteamClient() : m_pUtils(new FakeSteamUtils()), m_pUser(new FakeSteamUser()) {}
	~FakeSteamClient() { delete m_pUtils; delete m_pUser; }

	HSteamPipe CreateSteamPipe() override { return s_NextPipe++; }
	bool BReleaseSteamPipe(HSteamPipe hSteamPipe) override { return true; }
	HSteamUser ConnectToGlobalUser(HSteamPipe hSteamPipe) override { return s_NextUser++; }
	HSteamUser CreateLocalUser(HSteamPipe *phSteamPipe, EAccountType eAccountType) override
	{
		if (phSteamPipe) *phSteamPipe = s_NextPipe++;
		return s_NextUser++;
	}
	void ReleaseUser(HSteamPipe hSteamPipe, HSteamUser hUser) override {}
	ISteamUser *GetISteamUser(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return m_pUser; }
	ISteamGameServer *GetISteamGameServer(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	void SetLocalIPBinding(const SteamIPAddress_t &unIP, uint16 usPort) override {}
	ISteamFriends *GetISteamFriends(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamUtils *GetISteamUtils(HSteamPipe hSteamPipe, const char *pchVersion) override { return m_pUtils; }
	ISteamMatchmaking *GetISteamMatchmaking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamMatchmakingServers *GetISteamMatchmakingServers(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	void *GetISteamGenericInterface(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamUserStats *GetISteamUserStats(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamGameServerStats *GetISteamGameServerStats(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamApps *GetISteamApps(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamNetworking *GetISteamNetworking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamRemoteStorage *GetISteamRemoteStorage(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamScreenshots *GetISteamScreenshots(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	void RunFrame() override {}
	uint32 GetIPCCallCount() override { return 0; }
	void SetWarningMessageHook(SteamAPIWarningMessageHook_t pFunction) override {}
	bool BShutdownIfAllPipesClosed() override { return true; }
	ISteamHTTP *GetISteamHTTP(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamController *GetISteamController(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamUGC *GetISteamUGC(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamMusic *GetISteamMusic(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamHTMLSurface *GetISteamHTMLSurface(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	void DEPRECATED_Set_SteamAPI_CPostAPIResultInProcess(void (*)()) override {}
	void DEPRECATED_Remove_SteamAPI_CPostAPIResultInProcess(void (*)()) override {}
	void Set_SteamAPI_CCheckCallbackRegisteredInProcess(SteamAPI_CheckCallbackRegistered_t func) override {}
	ISteamInventory *GetISteamInventory(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamVideo *GetISteamVideo(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamParentalSettings *GetISteamParentalSettings(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamInput *GetISteamInput(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamParties *GetISteamParties(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override { return nullptr; }
	ISteamRemotePlay *GetISteamRemotePlay(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) override { return nullptr; }
	void DestroyAllInterfaces() override {}
};

// ---------------------------------------------------------------------------
// Exported functions (same exports as real steamclient.dll)
// ---------------------------------------------------------------------------

extern "C" {

S_API void* S_CALLTYPE CreateInterface(const char *pName, int *pReturnCode)
{
	if (pName == nullptr)
	{
		if (pReturnCode) *pReturnCode = 1;
		return nullptr;
	}

	_cprintf_s("[FakeSteamClient] CreateInterface --> %s\n", pName);

	if (strcmp(pName, "SteamClient017") == 0 ||
		strcmp(pName, "SteamClient018") == 0 ||
		strcmp(pName, "SteamClient019") == 0 ||
		strcmp(pName, "SteamClient020") == 0 ||
		strcmp(pName, "SteamClient021") == 0 ||
		strcmp(pName, "SteamClient022") == 0 ||
		strcmp(pName, "SteamClient023") == 0 ||
		strcmp(pName, STEAMCLIENT_INTERFACE_VERSION) == 0)
	{
		if (pReturnCode) *pReturnCode = 0;
		return new FakeSteamClient();
	}

	if (pReturnCode) *pReturnCode = 2;
	return nullptr;
}

S_API void S_CALLTYPE Steam_ReleaseThreadLocalMemory(int bThreadExit) {}
S_API bool S_CALLTYPE Steam_IsKnownInterface(const char *Interface) { return true; }
S_API void S_CALLTYPE Steam_NotifyMissingInterface(HSteamPipe hSteamPipe, const char* Interface) {}

S_API bool S_CALLTYPE Steam_BGetCallback(HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg)
{
	return false;
}

S_API void S_CALLTYPE Steam_FreeLastCallback(HSteamPipe hSteamPipe) {}

S_API bool S_CALLTYPE Steam_GetAPICallResult(HSteamPipe hSteamPipe, SteamAPICall_t hSteamAPICall, void* pCallback, int cubCallback, int iCallbackExpected, bool* pbFailed)
{
	if (pbFailed) *pbFailed = true;
	return false;
}

// SteamInternal exports (commonly loaded from steamclient.dll)
S_API void* S_CALLTYPE SteamInternal_CreateInterface(const char *ver)
{
	return CreateInterface(ver, nullptr);
}

S_API void* S_CALLTYPE SteamInternal_ContextInit(void *pContextInitData)
{
	return nullptr;
}

// FileSystem exports
S_API void* S_CALLTYPE FileSystem_Init(const char *basedir, void *filesystemFactory) { return nullptr; }
S_API void S_CALLTYPE FileSystem_Shutdown() {}

// Sys exports
S_API void S_CALLTYPE Sys_InitArgv(const char *OrigCmd) {}
S_API void S_CALLTYPE Sys_ShutdownArgv() {}

// Breakpad exports
S_API void S_CALLTYPE Breakpad_SteamMiniDumpInit(uint32 AppID, const char *pchVersion, const char *Dates, bool bFullMemoryDumps, void *pvContext, PFNPreMinidumpCallback m_pfnPreMinidumpCallback) {}
S_API void S_CALLTYPE Breakpad_SteamSetAppID(uint32 AppID) {}
S_API void S_CALLTYPE Breakpad_SteamSetSteamID(uint64 ulSteamID) {}
S_API void S_CALLTYPE Breakpad_SteamWriteMiniDumpSetComment(const char *Comment) {}
S_API void S_CALLTYPE Breakpad_SteamWriteMiniDumpUsingExceptionInfoWithBuildId(uint32 uStructuredExceptionCode, void* pvExceptionInfo, uint32 uBuildID) {}

} // extern "C"

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		AllocConsole();
		_cprintf_s("[FakeSteamClient] Locked n' loaded in, chief.\n");
		DisableThreadLibraryCalls(hModule);
	}
	return TRUE;
}
