/**
 *  All of the interfaces, exports found in the real dlls, namespaces,
 *  functions, you name it. This is kinda necessary, and also really
 *  touchy. So try to be careful here, if you even look at it sideways
 *  it'll take that as a sign to just throw the whole build process lol.
 *
 *  ~veeλnti<3 2026
 */

#pragma once

#include <Windows.h>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <set>
#include <string>

#define STEAM_API_EXPORTS
#include "include/sdk/steam_api.h"
#include "include/sdk/steamclientpublic.h"
#include "include/sdk/steam_gameserver.h"
#include "include/sdk/isteamapplist.h"

inline ISteamNetworkingUtils::~ISteamNetworkingUtils() {}
inline ISteamNetworkingSockets::~ISteamNetworkingSockets() {}

const uint32 k_unServerFlagNone    = 0x00;
const uint32 k_unServerFlagSecure  = 0x01;
const uint32 k_unServerFlagPrivate = 0x02;

#define STEAMGAMESEARCH_INTERFACE_VERSION "STEAMGAMESEARCH_INTERFACE_VERSION001"

class CSteamAPIContext;
extern HSteamUser g_ClientUser;
extern HSteamUser g_ServerUser;
extern uint32 g_ForcedAppId;

void UCOLOG(const char* fmt, ...);
void DumpTicketBytes(const char* source, const void* ticket, uint32 ticketSize);

extern std::set<std::string> g_RequestedInterfaces;
extern std::map<std::string, bool> g_InterfaceStubStatus;

inline void TrackInterfaceRequest(const char* version)
{
    if (version && version[0])
        g_RequestedInterfaces.insert(version);
}

inline void MarkInterfaceImplemented(const char* version)
{
    if (version && version[0])
        g_InterfaceStubStatus[version] = false;
}

inline void MarkInterfaceStub(const char* version)
{
    if (version && version[0])
        g_InterfaceStubStatus[version] = true;
}

void LoadSteamInterfacesFile();
void ReportMissingInterfaces();

class ISteamGameSearch 
{ 
public: 
	virtual ~ISteamGameSearch() {} 
	virtual void AddGameSearchParams(const char*, const char*) = 0;
	virtual void SearchForLobbyWithGameFilters(class CSteamID, int, int, int, int) = 0;
	virtual uint32 GetTotalPlayersInLobby(class CSteamID) = 0;
	virtual uint32 GetNumAvailableSlotsInLobby(class CSteamID) = 0;
	virtual void SubmitPlayerResult(uint64, class CSteamID, int, int) = 0;
	virtual void EndGameSearch(uint64) = 0;
	virtual void SetGameTags(const char**, uint32) = 0;
	virtual uint32 GetGameTags(char*, uint32) = 0;
	virtual uint32 GetNumPlayersSearching() = 0;
	virtual uint64 RequestPlayersForLobby(uint64) = 0;
	virtual bool IsGameSearchInProgress() = 0;
	virtual uint32 GetPlayersInGameSearchResult(uint32) = 0;
	virtual class CSteamID GetPlayerInGameSearchResult(uint32, uint32) = 0;
	virtual bool GetPlayerGameSearchResultData(uint32, uint32, const char*, char*, uint32) = 0;
	virtual uint32 GetPlayerGameSearchResultCount(uint32, uint32) = 0;
};

class CSteamGameSearchStub : public ISteamGameSearch
{
public:
	virtual void AddGameSearchParams( const char *, const char * ) override {}
	virtual void SearchForLobbyWithGameFilters( CSteamID, int, int, int, int ) override {}
	virtual uint32 GetTotalPlayersInLobby( CSteamID ) override { return 0; }
	virtual uint32 GetNumAvailableSlotsInLobby( CSteamID ) override { return 0; }
	virtual void SubmitPlayerResult( uint64, CSteamID, int, int ) override {}
	virtual void EndGameSearch( uint64 ) override {}
	virtual void SetGameTags( const char **, uint32 ) override {}
	virtual uint32 GetGameTags( char *, uint32 ) override { return 0; }
	virtual uint32 GetNumPlayersSearching() override { return 0; }
	virtual SteamAPICall_t RequestPlayersForLobby( uint64 ) override { return k_uAPICallInvalid; }
	virtual bool IsGameSearchInProgress() override { return false; }
	virtual uint32 GetPlayersInGameSearchResult( uint32 ) override { return 0; }
	virtual  CSteamID GetPlayerInGameSearchResult( uint32, uint32 ) override { return k_steamIDNil; }
	virtual bool GetPlayerGameSearchResultData( uint32, uint32, const char *, char *, uint32 ) override { return false; }
	virtual uint32 GetPlayerGameSearchResultCount( uint32, uint32 ) override { return 0; }
};

static CSteamGameSearchStub s_GameSearchStub;

class CSteamUserStub : public ISteamUser
{
public:
    static bool s_bEmulateTicket;
    static uint32 s_unEmulatedAppId;
    static std::vector<uint8_t> s_EncryptedTicket;

    static void SetEmulatedApp(uint32 unAppId) { s_unEmulatedAppId = unAppId; s_bEmulateTicket = true; }
    static bool IsEmulateTicketEnabled() { return s_bEmulateTicket; }

    virtual HSteamUser GetHSteamUser() override { return g_ClientUser; }
    virtual bool BLoggedOn() override { return true; }
    virtual CSteamID GetSteamID() override { return CSteamID(1, k_EUniversePublic, k_EAccountTypeIndividual); }
    virtual int InitiateGameConnection_DEPRECATED(void* pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure) override {
        UCOLOG("[UCOffline] InitiateGameConnection_DEPRECATED to %llu\r\n", steamIDGameServer.ConvertToUint64());
        if (!pAuthBlob || cbMaxAuthBlob < 24) return 0;
        memset(pAuthBlob, 0, cbMaxAuthBlob);

        CSteamID myID = GetSteamID();
        uint32 gcLen = 20;
        uint64 gcToken = ((uint64_t)1 << 32) | (uint64_t)GetTickCount();
        uint64 steamID = myID.ConvertToUint64();
        uint32 ticketGenDate = (uint32_t)time(NULL);

        uint8* p = (uint8*)pAuthBlob;
        *(uint32*)(p + 0) = gcLen;
        *(uint64*)(p + 4) = gcToken;
        *(uint64*)(p + 12) = steamID;
        *(uint32*)(p + 20) = ticketGenDate;

        UCOLOG("[UCOffline] InitiateGameConnection_DEPRECATED returning 24 byte auth blob\r\n");
        return 24;
    }
    virtual void TerminateGameConnection_DEPRECATED(uint32 unIPServer, uint16 usPortServer) override {}
    virtual void TrackAppUsageEvent(CGameID gameID, int eAppUsageEvent, const char* pchExtraInfo) override {}
    virtual bool GetUserDataFolder(char* pchBuffer, int cubBuffer) override {
        if (!pchBuffer || cubBuffer <= 0) return false;
        _snprintf_s(pchBuffer, cubBuffer, _TRUNCATE, "uc_offline_userdata");
        return true;
    }
    virtual void StartVoiceRecording() override {}
    virtual void StopVoiceRecording() override {}
    virtual EVoiceResult GetAvailableVoice(uint32 *pcbCompressed, uint32 *pcbUncompressed_Deprecated = 0, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated = 0) override { return k_EVoiceResultNotRecording; }
    virtual EVoiceResult GetVoice(bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed_Deprecated = false, void *pUncompressedDestBuffer_Deprecated = 0, uint32 cbUncompressedDestBufferSize_Deprecated = 0, uint32 *nUncompressBytesWritten_Deprecated = 0, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated = 0) override { return k_EVoiceResultNotRecording; }
    virtual EVoiceResult DecompressVoice(const void*, uint32, void*, uint32, uint32*, uint32) override { return k_EVoiceResultNotRecording; }
    virtual uint32 GetVoiceOptimalSampleRate() override { return 0; }
    virtual HAuthTicket GetAuthSessionTicket(void* pTicket, int cbMaxTicket, uint32* pcbTicket, const SteamNetworkingIdentity*) override {
        if (!pTicket || cbMaxTicket < 24) {
            if (pcbTicket) *pcbTicket = 0;
            return k_HAuthTicketInvalid;
        }
        memset(pTicket, 0, cbMaxTicket);

        static uint32 s_ticketCounter = 1;
        s_ticketCounter++;
        if (s_ticketCounter == 0) s_ticketCounter = 1;

        CSteamID myID = GetSteamID();
        uint64 steamID = myID.ConvertToUint64();

        uint8* p = (uint8*)pTicket;
        *(uint32*)(p + 0) = 20;                                    // GCLen = 20
        *(uint64*)(p + 4) = ((uint64_t)s_ticketCounter << 32) | (uint64_t)GetTickCount();  // GC Token
        *(uint64*)(p + 12) = steamID;                              // Steam ID
        *(uint32*)(p + 20) = (uint32_t)time(NULL);                 // Ticket gen date

        if (pcbTicket) *pcbTicket = 24;

        HAuthTicket hTicket = (HAuthTicket)s_ticketCounter;

        CCallbackDispatcher* pDisp = GetDispatcher();
        if (pDisp) {
            GetAuthSessionTicketResponse_t resp{};
            resp.m_hAuthTicket = hTicket;
            resp.m_eResult = k_EResultOK;
            pDisp->PostCallback(g_ClientUser, resp.k_iCallback, &resp, sizeof(resp));
        }

        UCOLOG("[UCOffline] GetAuthSessionTicket returning ticket 0x%X (24 bytes)\r\n", hTicket);
        return hTicket;
    }
    virtual HAuthTicket GetAuthTicketForWebApi(const char*) override {
        static uint32 s_webTicketCounter = 0x80000000;
        s_webTicketCounter++;
        if (s_webTicketCounter == 0) s_webTicketCounter = 0x80000001;

        HAuthTicket hTicket = (HAuthTicket)s_webTicketCounter;
        CCallbackDispatcher* pDisp = GetDispatcher();
        if (pDisp) {
            GetAuthSessionTicketResponse_t resp{};
            resp.m_hAuthTicket = hTicket;
            resp.m_eResult = k_EResultOK;
            pDisp->PostCallback(g_ClientUser, resp.k_iCallback, &resp, sizeof(resp));
        }
        UCOLOG("[UCOffline] GetAuthTicketForWebApi returning ticket 0x%X\r\n", hTicket);
        return hTicket;
    }
    virtual EBeginAuthSessionResult BeginAuthSession(const void* pAuthTicket, int cbAuthTicket, CSteamID steamID) override {
        UCOLOG("[UCOffline] BeginAuthSession from %llu\r\n", steamID.ConvertToUint64());
        CCallbackDispatcher* pDisp = GetDispatcher();
        if (pDisp) {
            ValidateAuthTicketResponse_t resp{};
            resp.m_SteamID = steamID;
            resp.m_eAuthSessionResponse = k_EAuthSessionResponseOK;
            resp.m_OwnerSteamID = steamID;
            pDisp->PostCallback(g_ClientUser, resp.k_iCallback, &resp, sizeof(resp));
        }
        return k_EBeginAuthSessionResultOK;
    }
    virtual void EndAuthSession(CSteamID) override {}
    virtual void CancelAuthTicket(HAuthTicket) override {}
    virtual EUserHasLicenseForAppResult UserHasLicenseForApp(CSteamID, AppId_t appID) override {
        if (s_bEmulateTicket && (uint32)appID == s_unEmulatedAppId)
            return k_EUserHasLicenseResultHasLicense;
        if ((uint32)appID == g_ForcedAppId)
            return k_EUserHasLicenseResultHasLicense;
        return k_EUserHasLicenseResultHasLicense;
    }
    virtual bool BIsBehindNAT() override { return false; }
    virtual void AdvertiseGame(CSteamID, uint32, uint16) override {}
    virtual SteamAPICall_t RequestEncryptedAppTicket(void* pDataToInclude, int cbDataToInclude) override {
        s_EncryptedTicket.clear();
        s_EncryptedTicket.resize(128);
        uint32 ticketSize = (uint32)GenerateFakeTicket(s_EncryptedTicket.data(), (uint32)s_EncryptedTicket.size(), pDataToInclude, cbDataToInclude);
        s_EncryptedTicket.resize(ticketSize);
        UCOLOG("[UCOffline] Generated encrypted app ticket (%u bytes)", ticketSize);
        return k_uAPICallInvalid;
    }
    virtual bool GetEncryptedAppTicket(void* pTicket, int cbMaxTicket, uint32* pcbTicket) override {
        if (!pcbTicket) return false;
        *pcbTicket = (uint32)s_EncryptedTicket.size();
        if ((int)*pcbTicket > cbMaxTicket) return false;
        if (pTicket && s_EncryptedTicket.size() > 0) {
            memcpy(pTicket, s_EncryptedTicket.data(), s_EncryptedTicket.size());
            DumpTicketBytes("encrypted_app_ticket", pTicket, *pcbTicket);
        }
        return true;
    }
    virtual int GetGameBadgeLevel(int, bool) override { return 0; }
    virtual int GetPlayerSteamLevel() override { return 0; }
    virtual SteamAPICall_t RequestStoreAuthURL(const char*) override { return k_uAPICallInvalid; }
    virtual bool BIsPhoneVerified() override { return false; }
    virtual bool BIsTwoFactorEnabled() override { return false; }
    virtual bool BIsPhoneIdentifying() override { return false; }
    virtual bool BIsPhoneRequiringVerification() override { return false; }
    virtual SteamAPICall_t GetMarketEligibility() override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t GetDurationControl() override { return k_uAPICallInvalid; }
    virtual bool BSetDurationControlOnlineState(EDurationControlOnlineState) override { return false; }

private:
    static uint32 GenerateFakeTicket(uint8_t* pTicket, uint32 cbMaxTicket, void* pDataToInclude, int cbDataToInclude);
};

// music
class ISteamMusicRemote
{
public:
	virtual ~ISteamMusicRemote() {}
	virtual bool RegisterSteamMusicRemote(const char*) = 0;
	virtual bool DeregisterSteamMusicRemote() = 0;
	virtual bool BIsCurrentMusicRemote() = 0;
	virtual bool BActivationSuccess(bool) = 0;
	virtual bool SetDisplayName(const char*) = 0;
	virtual bool SetPNGIcon_64x64(void*, uint32) = 0;
	virtual bool EnablePlayPrevious(bool) = 0;
	virtual bool EnablePlayNext(bool) = 0;
	virtual bool EnableShuffled(bool) = 0;
	virtual bool EnableLooped(bool) = 0;
	virtual bool EnableQueue(bool) = 0;
	virtual bool EnablePlaylists(bool) = 0;
	virtual bool UpdatePlaybackStatus(int) = 0;
	virtual bool UpdateShuffled(bool) = 0;
	virtual bool UpdateLooped(bool) = 0;
	virtual bool UpdateVolume(float) = 0;
	virtual bool CurrentEntryWillChange() = 0;
	virtual bool CurrentEntryIsAvailable(bool) = 0;
	virtual bool UpdateCurrentEntryText(const char*) = 0;
	virtual bool UpdateCurrentEntryElapsedSeconds(int) = 0;
	virtual bool UpdateCurrentEntryCoverArt(void*, uint32) = 0;
	virtual bool CurrentEntryDidChange() = 0;
	virtual bool QueueWillChange() = 0;
	virtual bool ResetQueueEntries() = 0;
	virtual bool SetQueueEntry(int, int, const char*) = 0;
	virtual bool SetCurrentQueueEntry(int) = 0;
	virtual bool QueueDidChange() = 0;
	virtual bool PlaylistWillChange() = 0;
	virtual bool ResetPlaylistEntries() = 0;
	virtual bool SetPlaylistEntry(int, int, const char*) = 0;
	virtual bool SetCurrentPlaylistEntry(int) = 0;
	virtual bool PlaylistDidChange() = 0;
};

class CSteamMusicRemoteStub : public ISteamMusicRemote
{
public:
	virtual bool RegisterSteamMusicRemote(const char*) override { return false; }
	virtual bool DeregisterSteamMusicRemote() override { return false; }
	virtual bool BIsCurrentMusicRemote() override { return false; }
	virtual bool BActivationSuccess(bool) override { return false; }
	virtual bool SetDisplayName(const char*) override { return false; }
	virtual bool SetPNGIcon_64x64(void*, uint32) override { return false; }
	virtual bool EnablePlayPrevious(bool) override { return false; }
	virtual bool EnablePlayNext(bool) override { return false; }
	virtual bool EnableShuffled(bool) override { return false; }
	virtual bool EnableLooped(bool) override { return false; }
	virtual bool EnableQueue(bool) override { return false; }
	virtual bool EnablePlaylists(bool) override { return false; }
	virtual bool UpdatePlaybackStatus(int) override { return false; }
	virtual bool UpdateShuffled(bool) override { return false; }
	virtual bool UpdateLooped(bool) override { return false; }
	virtual bool UpdateVolume(float) override { return false; }
	virtual bool CurrentEntryWillChange() override { return false; }
	virtual bool CurrentEntryIsAvailable(bool) override { return false; }
	virtual bool UpdateCurrentEntryText(const char*) override { return false; }
	virtual bool UpdateCurrentEntryElapsedSeconds(int) override { return false; }
	virtual bool UpdateCurrentEntryCoverArt(void*, uint32) override { return false; }
	virtual bool CurrentEntryDidChange() override { return false; }
	virtual bool QueueWillChange() override { return false; }
	virtual bool ResetQueueEntries() override { return false; }
	virtual bool SetQueueEntry(int, int, const char*) override { return false; }
	virtual bool SetCurrentQueueEntry(int) override { return false; }
	virtual bool QueueDidChange() override { return false; }
	virtual bool PlaylistWillChange() override { return false; }
	virtual bool ResetPlaylistEntries() override { return false; }
	virtual bool SetPlaylistEntry(int, int, const char*) override { return false; }
	virtual bool SetCurrentPlaylistEntry(int) override { return false; }
	virtual bool PlaylistDidChange() override { return false; }
};

static CSteamMusicRemoteStub s_MusicRemoteStub;

class CSteamAPIContext
{
public:
	ISteamClient* m_pSteamClient;
	ISteamUser* m_pSteamUser;
	ISteamFriends* m_pSteamFriends;
	ISteamUtils* m_pSteamUtils;
	ISteamMatchmaking* m_pSteamMatchmaking;
	ISteamMatchmakingServers* m_pSteamMatchmakingServers;
	ISteamUserStats* m_pSteamUserStats;
	ISteamApps* m_pSteamApps;
	ISteamNetworking* m_pSteamNetworking;
	ISteamRemoteStorage* m_pSteamRemoteStorage;
	ISteamScreenshots* m_pSteamScreenshots;
	ISteamHTTP* m_pSteamHTTP;
	ISteamController* m_pSteamController;
	ISteamUGC* m_pSteamUGC;
	ISteamMusic* m_pSteamMusic;
	ISteamGameSearch* m_pSteamGameSearch;
	ISteamParties* m_pSteamParties;
	ISteamRemotePlay* m_pSteamRemotePlay;
	ISteamTimeline* m_pSteamTimeline;
	ISteamInventory* m_pSteamInventory;
	ISteamHTMLSurface* m_pSteamHTMLSurface;
	ISteamVideo* m_pSteamVideo;
	ISteamParentalSettings* m_pSteamParentalSettings;
	ISteamInput* m_pSteamInput;
	ISteamAppList* m_pSteamAppList;

	bool Init()
	{
		HSteamUser hUser = SteamAPI_GetHSteamUser();
		HSteamPipe hPipe = SteamAPI_GetHSteamPipe();
		if (hPipe == 0) return false;

		m_pSteamClient = (ISteamClient*)SteamInternal_CreateInterface(STEAMCLIENT_INTERFACE_VERSION);
		m_pSteamUser = (ISteamUser*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMUSER_INTERFACE_VERSION);
		m_pSteamFriends = (ISteamFriends*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMFRIENDS_INTERFACE_VERSION);
		m_pSteamUtils = (ISteamUtils*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMUTILS_INTERFACE_VERSION);
		m_pSteamMatchmaking = (ISteamMatchmaking*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMMATCHMAKING_INTERFACE_VERSION);
		m_pSteamMatchmakingServers = (ISteamMatchmakingServers*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION);
		m_pSteamUserStats = (ISteamUserStats*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMUSERSTATS_INTERFACE_VERSION);
		m_pSteamApps = (ISteamApps*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMAPPS_INTERFACE_VERSION);
		m_pSteamNetworking = (ISteamNetworking*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMNETWORKING_INTERFACE_VERSION);
		m_pSteamRemoteStorage = (ISteamRemoteStorage*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMREMOTESTORAGE_INTERFACE_VERSION);
		m_pSteamScreenshots = (ISteamScreenshots*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMSCREENSHOTS_INTERFACE_VERSION);
		m_pSteamHTTP = (ISteamHTTP*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMHTTP_INTERFACE_VERSION);
		m_pSteamController = (ISteamController*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMCONTROLLER_INTERFACE_VERSION);
		m_pSteamUGC = (ISteamUGC*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMUGC_INTERFACE_VERSION);
		m_pSteamMusic = (ISteamMusic*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMMUSIC_INTERFACE_VERSION);
		m_pSteamGameSearch = nullptr;
		m_pSteamParties = (ISteamParties*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMPARTIES_INTERFACE_VERSION);
		m_pSteamRemotePlay = (ISteamRemotePlay*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMREMOTEPLAY_INTERFACE_VERSION);
		m_pSteamTimeline = (ISteamTimeline*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMTIMELINE_INTERFACE_VERSION);
		m_pSteamInventory = (ISteamInventory*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMINVENTORY_INTERFACE_VERSION);
		m_pSteamHTMLSurface = (ISteamHTMLSurface*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMHTMLSURFACE_INTERFACE_VERSION);
		m_pSteamVideo = (ISteamVideo*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMVIDEO_INTERFACE_VERSION);
		m_pSteamParentalSettings = (ISteamParentalSettings*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMPARENTALSETTINGS_INTERFACE_VERSION);
		m_pSteamInput = (ISteamInput*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMINPUT_INTERFACE_VERSION);
		m_pSteamAppList = (ISteamAppList*)SteamInternal_FindOrCreateUserInterface(hUser, STEAMAPPLIST_INTERFACE_VERSION);

		return m_pSteamUser != nullptr && m_pSteamUtils != nullptr;
	}

	void Clear()
	{
		m_pSteamClient = nullptr; m_pSteamUser = nullptr; m_pSteamFriends = nullptr; m_pSteamUtils = nullptr;
		m_pSteamMatchmaking = nullptr; m_pSteamMatchmakingServers = nullptr;
		m_pSteamUserStats = nullptr; m_pSteamApps = nullptr; m_pSteamNetworking = nullptr;
		m_pSteamRemoteStorage = nullptr; m_pSteamScreenshots = nullptr; m_pSteamHTTP = nullptr;
		m_pSteamController = nullptr; m_pSteamUGC = nullptr; m_pSteamMusic = nullptr;
		m_pSteamGameSearch = nullptr; m_pSteamParties = nullptr;
		m_pSteamRemotePlay = nullptr; m_pSteamTimeline = nullptr;
		m_pSteamInventory = nullptr; m_pSteamHTMLSurface = nullptr;
		m_pSteamVideo = nullptr; m_pSteamParentalSettings = nullptr;
		m_pSteamInput = nullptr; m_pSteamAppList = nullptr;
	}

	ISteamClient* SteamClient() { return m_pSteamClient; }
	ISteamUser* SteamUser() { return m_pSteamUser; }
	ISteamFriends* SteamFriends() { return m_pSteamFriends; }
	ISteamUtils* SteamUtils() { return m_pSteamUtils; }
	ISteamMatchmaking* SteamMatchmaking() { return m_pSteamMatchmaking; }
	ISteamMatchmakingServers* SteamMatchmakingServers() { return m_pSteamMatchmakingServers; }
	ISteamUserStats* SteamUserStats() { return m_pSteamUserStats; }
	ISteamApps* SteamApps() { return m_pSteamApps; }
	ISteamNetworking* SteamNetworking() { return m_pSteamNetworking; }
	ISteamRemoteStorage* SteamRemoteStorage() { return m_pSteamRemoteStorage; }
	ISteamScreenshots* SteamScreenshots() { return m_pSteamScreenshots; }
	ISteamHTTP* SteamHTTP() { return m_pSteamHTTP; }
	ISteamController* SteamController() { return m_pSteamController; }
	ISteamUGC* SteamUGC() { return m_pSteamUGC; }
	ISteamMusic* SteamMusic() { return m_pSteamMusic; }
	ISteamGameSearch* SteamGameSearch() { return m_pSteamGameSearch; }
	ISteamParties* SteamParties() { return m_pSteamParties; }
	ISteamRemotePlay* SteamRemotePlay() { return m_pSteamRemotePlay; }
	ISteamInventory* SteamInventory() { return m_pSteamInventory; }
	ISteamHTMLSurface* SteamHTMLSurface() { return m_pSteamHTMLSurface; }
	ISteamVideo* SteamVideo() { return m_pSteamVideo; }
	ISteamParentalSettings* SteamParentalSettings() { return m_pSteamParentalSettings; }
	ISteamInput* SteamInput() { return m_pSteamInput; }
	ISteamTimeline* SteamTimeline() { return m_pSteamTimeline; }
	ISteamAppList* SteamAppList() { return m_pSteamAppList; }

};

extern CSteamAPIContext g_ClientCtx;

class CSteamAppsStub : public ISteamApps
{
public:
    static std::vector<uint32> s_UnlockedDLCAppIds;
    static void SetUnlockedDLCAppIds(const std::vector<uint32>& appIds) { s_UnlockedDLCAppIds = appIds; }
    virtual bool BIsSubscribed() override { return true; }
    virtual bool BIsLowViolence() override { return false; }
    virtual bool BIsCybercafe() override { return false; }
    virtual bool BIsVACBanned() override { return false; }
    virtual const char *GetCurrentGameLanguage() override { return "english"; }
    virtual const char *GetAvailableGameLanguages() override { return "english"; }
    virtual bool BIsSubscribedApp(AppId_t appID) override {
        for (uint32 id : s_UnlockedDLCAppIds) {
            if (id == (uint32)appID) return true;
        }
        return true;
    }
    virtual bool BIsDlcInstalled(AppId_t appID) override {
        for (uint32 id : s_UnlockedDLCAppIds) {
            if (id == (uint32)appID) return true;
        }
        return true;
    }
    virtual uint32 GetEarliestPurchaseUnixTime(AppId_t nAppID) override { return 0; }
    virtual bool BIsSubscribedFromFreeWeekend() override { return false; }
    virtual int GetDLCCount() override { return 0; }
    virtual bool BGetDLCDataByIndex(int iDLC, AppId_t *pAppID, bool *pbAvailable, char *pchName, int cchNameBufferSize) override { return false; }
    virtual void InstallDLC(AppId_t nAppID) override {}
    virtual void UninstallDLC(AppId_t nAppID) override {}
    virtual void RequestAppProofOfPurchaseKey(AppId_t nAppID) override {}
    virtual bool GetCurrentBetaName(char *pchName, int cchNameBufferSize) override { return false; }
    virtual bool MarkContentCorrupt(bool bMissingFilesOnly) override { return false; }
    virtual uint32 GetInstalledDepots(AppId_t appID, DepotId_t *pvecDepots, uint32 cMaxDepots) override { return 0; }
    virtual uint32 GetAppInstallDir(AppId_t appID, char *pchFolder, uint32 cchFolderBufferSize) override {
        if (pchFolder && cchFolderBufferSize > 0) {
            DWORD len = GetModuleFileNameA(nullptr, pchFolder, cchFolderBufferSize);
            if (len > 0) {
                char* lastSlash = strrchr(pchFolder, '\\');
                if (lastSlash) *lastSlash = 0;
                return (uint32)strlen(pchFolder);
            }
            pchFolder[0] = 0;
        }
        return 0;
    }
    virtual bool BIsAppInstalled(AppId_t appID) override { return true; }

    virtual CSteamID GetAppOwner() override { 
        if (g_ClientCtx.SteamUser())
            return g_ClientCtx.SteamUser()->GetSteamID();
        return k_steamIDNil; 
    }
    
    virtual const char *GetLaunchQueryParam(const char *pchKey) override { return ""; }
    virtual bool GetDlcDownloadProgress(AppId_t nAppID, uint64 *punBytesDownloaded, uint64 *punBytesTotal) override { return false; }
    virtual int GetAppBuildId() override {
        UCOLOG("[UCOffline] GetAppBuildId -> %u\r\n", g_ForcedAppId);
        return g_ForcedAppId;
    }
    virtual void RequestAllProofOfPurchaseKeys() override {}
    virtual SteamAPICall_t GetFileDetails(const char* pszFileName) override { return k_uAPICallInvalid; }
    virtual int GetLaunchCommandLine(char *pszCommandLine, int cubCommandLine) override { return 0; }
    virtual bool BIsSubscribedFromFamilySharing() override { return false; }
    virtual bool BIsTimedTrial(uint32* punSecondsAllowed, uint32* punSecondsPlayed) override { return false; }
    virtual bool SetDlcContext(AppId_t nAppID) override { return false; }
    virtual int GetNumBetas(int* pnAvailable, int* pnPrivate) override { return 0; }
    virtual bool GetBetaInfo(int iBetaIndex, uint32 *punFlags, uint32 *punBuildID, char *pchBetaName, int cchBetaName, char *pchDescription, int cchDescription) override { return false; }
    virtual bool SetActiveBeta(const char *pchBetaName) override { return false; }

private:
    static CSteamAppsStub s_AppsStub;
};

std::vector<uint32> CSteamAppsStub::s_UnlockedDLCAppIds;
bool CSteamUserStub::s_bEmulateTicket = false;
uint32 CSteamUserStub::s_unEmulatedAppId = 0;
std::vector<uint8_t> CSteamUserStub::s_EncryptedTicket;
CSteamAppsStub s_AppsStub;
CSteamUserStub s_UserStub;

inline uint32 CSteamUserStub::GenerateFakeTicket(uint8_t* pTicket, uint32 cbMaxTicket, void* pDataToInclude, int cbDataToInclude)
{
    if (cbMaxTicket < 64) return 0;
    
    memset(pTicket, 0, cbMaxTicket);
    
    pTicket[0] = 'S';
    pTicket[1] = 'T';
    pTicket[2] = 'E';
    pTicket[3] = 'A';
    pTicket[4] = 'M';
    
    *reinterpret_cast<uint32_t*>(pTicket + 4) = s_unEmulatedAppId;
    
    *reinterpret_cast<uint32_t*>(pTicket + 8) = GetTickCount();
    
    uint32 dataOffset = 16;
    if (pDataToInclude && cbDataToInclude > 0 && dataOffset + cbDataToInclude < cbMaxTicket) {
        memcpy(pTicket + dataOffset, pDataToInclude, cbDataToInclude);
        dataOffset += cbDataToInclude;
    }
    
    *reinterpret_cast<uint32_t*>(pTicket + 12) = dataOffset;
    
    return dataOffset;
}

class CSteamUtilsStub : public ISteamUtils
{
public:
    virtual uint32 GetSecondsSinceAppActive() override { return 0; }
    virtual uint32 GetSecondsSinceComputerActive() override { return 0; }
    virtual EUniverse GetConnectedUniverse() override { return k_EUniversePublic; }
    virtual uint32 GetServerRealTime() override { return (uint32)time(nullptr); }
    virtual const char *GetIPCountry() override { return "US"; }
    virtual bool GetImageSize(int, uint32*, uint32*) override { return false; }
    virtual bool GetImageRGBA(int, uint8*, int) override { return false; }
    virtual bool GetCSERIPPort(uint32*, uint16*) override { return false; }
    virtual uint8 GetCurrentBatteryPower() override { return 255; }
    virtual uint32 GetAppID() override { return g_ForcedAppId; }
    virtual void SetOverlayNotificationPosition(ENotificationPosition) override {}
    virtual bool IsAPICallCompleted(SteamAPICall_t, bool *pbFailed) override { if (pbFailed) *pbFailed = true; return true; }
    virtual ESteamAPICallFailure GetAPICallFailureReason(SteamAPICall_t) override { return k_ESteamAPICallFailureInvalidHandle; }
    virtual bool GetAPICallResult(SteamAPICall_t, void*, int, int, bool *pbFailed) override { if (pbFailed) *pbFailed = true; return false; }
    virtual void RunFrame() override {}
    virtual uint32 GetIPCCallCount() override { return 0; }
    virtual void SetWarningMessageHook(SteamAPIWarningMessageHook_t) override {}
    virtual bool IsOverlayEnabled() override { return false; }
    virtual bool BOverlayNeedsPresent() override { return false; }
    virtual SteamAPICall_t CheckFileSignature(const char*) override { return k_uAPICallInvalid; }
    virtual bool ShowGamepadTextInput(EGamepadTextInputMode, EGamepadTextInputLineMode, const char*, uint32, const char*) override { return false; }
    virtual uint32 GetEnteredGamepadTextLength() override { return 0; }
    virtual bool GetEnteredGamepadTextInput(char*, uint32) override { return false; }
    virtual const char *GetSteamUILanguage() override { return "english"; }
    virtual bool IsSteamRunningInVR() override { return false; }
    virtual void SetOverlayNotificationInset(int, int) override {}
    virtual bool IsSteamInBigPictureMode() override { return false; }
    virtual void StartVRDashboard() override {}
    virtual bool IsVRHeadsetStreamingEnabled() override { return false; }
    virtual void SetVRHeadsetStreamingEnabled(bool) override {}
    virtual bool IsSteamChinaLauncher() override { return false; }
    virtual bool InitFilterText(uint32 = 0) override { return false; }
    virtual int FilterText(ETextFilteringContext, CSteamID, const char*, char*, uint32) override { return 0; }
    virtual ESteamIPv6ConnectivityState GetIPv6ConnectivityState(ESteamIPv6ConnectivityProtocol) override { return k_ESteamIPv6ConnectivityState_Unknown; }
    virtual bool IsSteamRunningOnSteamDeck() override { return false; }
    virtual bool ShowFloatingGamepadTextInput(EFloatingGamepadTextInputMode, int, int, int, int) override { return false; }
    virtual void SetGameLauncherMode(bool) override {}
    virtual bool DismissFloatingGamepadTextInput() override { return false; }
    virtual bool DismissGamepadTextInput() override { return false; }
};

class CSteamFriendsStub : public ISteamFriends
{
public:
    virtual const char *GetPersonaName() override { return "UCOfflineUser"; }
    virtual EPersonaState GetPersonaState() override { return k_EPersonaStateOnline; }
    virtual int GetFriendCount(int) override { return 0; }
    virtual CSteamID GetFriendByIndex(int, int) override { return k_steamIDNil; }
    virtual EFriendRelationship GetFriendRelationship(CSteamID) override { return k_EFriendRelationshipNone; }
    virtual EPersonaState GetFriendPersonaState(CSteamID) override { return k_EPersonaStateOffline; }
    virtual const char *GetFriendPersonaName(CSteamID) override { return ""; }
    virtual bool GetFriendGamePlayed(CSteamID, FriendGameInfo_t*) override { return false; }
    virtual const char *GetFriendPersonaNameHistory(CSteamID, int) override { return ""; }
    virtual int GetFriendSteamLevel(CSteamID) override { return 0; }
    virtual const char *GetPlayerNickname(CSteamID) override { return ""; }
    virtual int GetFriendsGroupCount() override { return 0; }
    virtual FriendsGroupID_t GetFriendsGroupIDByIndex(int) override { return -1; }
    virtual const char *GetFriendsGroupName(FriendsGroupID_t) override { return ""; }
    virtual int GetFriendsGroupMembersCount(FriendsGroupID_t) override { return 0; }
    virtual void GetFriendsGroupMembersList(FriendsGroupID_t, CSteamID*, int) override {}
    virtual bool HasFriend(CSteamID, int) override { return false; }
    virtual int GetClanCount() override { return 0; }
    virtual CSteamID GetClanByIndex(int) override { return k_steamIDNil; }
    virtual const char *GetClanName(CSteamID) override { return ""; }
    virtual const char *GetClanTag(CSteamID) override { return ""; }
    virtual bool GetClanActivityCounts(CSteamID, int*, int*, int*) override { return false; }
    virtual SteamAPICall_t DownloadClanActivityCounts(CSteamID*, int) override { return k_uAPICallInvalid; }
    virtual int GetFriendCountFromSource(CSteamID) override { return 0; }
    virtual CSteamID GetFriendFromSourceByIndex(CSteamID, int) override { return k_steamIDNil; }
    virtual bool IsUserInSource(CSteamID, CSteamID) override { return false; }
    virtual void SetInGameVoiceSpeaking(CSteamID, bool) override {}
    virtual void ActivateGameOverlay(const char*) override {}
    virtual void ActivateGameOverlayToUser(const char*, CSteamID) override {}
    virtual void ActivateGameOverlayToWebPage(const char*, EActivateGameOverlayToWebPageMode = k_EActivateGameOverlayToWebPageMode_Default) override {}
    virtual void ActivateGameOverlayToStore(AppId_t, EOverlayToStoreFlag) override {}
    virtual void SetPlayedWith(CSteamID) override {}
    virtual void ActivateGameOverlayInviteDialog(CSteamID) override {}
    virtual int GetSmallFriendAvatar(CSteamID) override { return -1; }
    virtual int GetMediumFriendAvatar(CSteamID) override { return -1; }
    virtual int GetLargeFriendAvatar(CSteamID) override { return -1; }
    virtual bool RequestUserInformation(CSteamID, bool) override { return false; }
    virtual SteamAPICall_t RequestClanOfficerList(CSteamID) override { return k_uAPICallInvalid; }
    virtual CSteamID GetClanOwner(CSteamID) override { return k_steamIDNil; }
    virtual int GetClanOfficerCount(CSteamID) override { return 0; }
    virtual CSteamID GetClanOfficerByIndex(CSteamID, int) override { return k_steamIDNil; }
    virtual bool SetRichPresence(const char*, const char*) override { return false; }
    virtual void ClearRichPresence() override {}
    virtual const char *GetFriendRichPresence(CSteamID, const char*) override { return ""; }
    virtual int GetFriendRichPresenceKeyCount(CSteamID) override { return 0; }
    virtual const char *GetFriendRichPresenceKeyByIndex(CSteamID, int) override { return ""; }
    virtual void RequestFriendRichPresence(CSteamID) override {}
    virtual bool InviteUserToGame(CSteamID, const char*) override { return false; }
    virtual int GetCoplayFriendCount() override { return 0; }
    virtual CSteamID GetCoplayFriend(int) override { return k_steamIDNil; }
    virtual int GetFriendCoplayTime(CSteamID) override { return 0; }
    virtual AppId_t GetFriendCoplayGame(CSteamID) override { return k_uAppIdInvalid; }
    virtual SteamAPICall_t JoinClanChatRoom(CSteamID) override { return k_uAPICallInvalid; }
    virtual bool LeaveClanChatRoom(CSteamID) override { return false; }
    virtual int GetClanChatMemberCount(CSteamID) override { return 0; }
    virtual CSteamID GetChatMemberByIndex(CSteamID, int) override { return k_steamIDNil; }
    virtual bool SendClanChatMessage(CSteamID, const char*) override { return false; }
    virtual int GetClanChatMessage(CSteamID, int, void*, int, EChatEntryType*, CSteamID*) override { return 0; }
    virtual bool IsClanChatAdmin(CSteamID, CSteamID) override { return false; }
    virtual bool IsClanChatWindowOpenInSteam(CSteamID) override { return false; }
    virtual bool OpenClanChatWindowInSteam(CSteamID) override { return false; }
    virtual bool CloseClanChatWindowInSteam(CSteamID) override { return false; }
    virtual bool SetListenForFriendsMessages(bool) override { return false; }
    virtual bool ReplyToFriendMessage(CSteamID, const char*) override { return false; }
    virtual int GetFriendMessage(CSteamID, int, void*, int, EChatEntryType*) override { return 0; }
    virtual SteamAPICall_t GetFollowerCount(CSteamID) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t IsFollowing(CSteamID) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t EnumerateFollowingList(uint32) override { return k_uAPICallInvalid; }
    virtual bool IsClanPublic(CSteamID) override { return false; }
    virtual bool IsClanOfficialGameGroup(CSteamID) override { return false; }
    virtual int GetNumChatsWithUnreadPriorityMessages() override { return 0; }
    virtual void ActivateGameOverlayRemotePlayTogetherInviteDialog(CSteamID) override {}
    virtual bool RegisterProtocolInOverlayBrowser(const char*) override { return false; }
    virtual void ActivateGameOverlayInviteDialogConnectString(const char*) override {}
    virtual SteamAPICall_t RequestEquippedProfileItems(CSteamID) override { return k_uAPICallInvalid; }
    virtual bool BHasEquippedProfileItem(CSteamID, ECommunityProfileItemType) override { return false; }
    virtual const char *GetProfileItemPropertyString(CSteamID, ECommunityProfileItemType, ECommunityProfileItemProperty) override { return ""; }
    virtual uint32 GetProfileItemPropertyUint(CSteamID, ECommunityProfileItemType, ECommunityProfileItemProperty) override { return 0; }
};

class CSteamMatchmakingStub : public ISteamMatchmaking
{
public:
    virtual int GetFavoriteGameCount() override { return 0; }
    virtual bool GetFavoriteGame(int, AppId_t*, uint32*, uint16*, uint16*, uint32*, uint32*) override { return false; }
    virtual int AddFavoriteGame(AppId_t, uint32, uint16, uint16, uint32, uint32) override { return -1; }
    virtual bool RemoveFavoriteGame(AppId_t, uint32, uint16, uint16, uint32) override { return false; }
    virtual SteamAPICall_t RequestLobbyList() override { return k_uAPICallInvalid; }
    virtual void AddRequestLobbyListStringFilter(const char*, const char*, ELobbyComparison) override {}
    virtual void AddRequestLobbyListNumericalFilter(const char*, int, ELobbyComparison) override {}
    virtual void AddRequestLobbyListNearValueFilter(const char*, int) override {}
    virtual void AddRequestLobbyListFilterSlotsAvailable(int) override {}
    virtual void AddRequestLobbyListDistanceFilter(ELobbyDistanceFilter) override {}
    virtual void AddRequestLobbyListResultCountFilter(int) override {}
    virtual void AddRequestLobbyListCompatibleMembersFilter(CSteamID) override {}
    virtual CSteamID GetLobbyByIndex(int) override { return k_steamIDNil; }
    virtual SteamAPICall_t CreateLobby(ELobbyType, int) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t JoinLobby(CSteamID) override { return k_uAPICallInvalid; }
    virtual void LeaveLobby(CSteamID) override {}
    virtual bool InviteUserToLobby(CSteamID, CSteamID) override { return false; }
    virtual int GetNumLobbyMembers(CSteamID) override { return 0; }
    virtual CSteamID GetLobbyMemberByIndex(CSteamID, int) override { return k_steamIDNil; }
    virtual const char *GetLobbyData(CSteamID, const char*) override { return ""; }
    virtual bool SetLobbyData(CSteamID, const char*, const char*) override { return false; }
    virtual int GetLobbyDataCount(CSteamID) override { return 0; }
    virtual bool GetLobbyDataByIndex(CSteamID, int, char*, int, char*, int) override { return false; }
    virtual bool DeleteLobbyData(CSteamID, const char*) override { return false; }
    virtual const char *GetLobbyMemberData(CSteamID, CSteamID, const char*) override { return ""; }
    virtual void SetLobbyMemberData(CSteamID, const char*, const char*) override {}
    virtual bool SendLobbyChatMsg(CSteamID, const void*, int) override { return false; }
    virtual int GetLobbyChatEntry(CSteamID, int, CSteamID*, void*, int, EChatEntryType*) override { return 0; }
    virtual bool RequestLobbyData(CSteamID) override { return false; }
    virtual void SetLobbyGameServer(CSteamID, uint32, uint16, CSteamID) override {}
    virtual bool GetLobbyGameServer(CSteamID, uint32*, uint16*, CSteamID*) override { return false; }
    virtual bool SetLobbyMemberLimit(CSteamID, int) override { return false; }
    virtual int GetLobbyMemberLimit(CSteamID) override { return 0; }
    virtual bool SetLobbyType(CSteamID, ELobbyType) override { return false; }
    virtual bool SetLobbyJoinable(CSteamID, bool) override { return false; }
    virtual CSteamID GetLobbyOwner(CSteamID) override { return k_steamIDNil; }
    virtual bool SetLobbyOwner(CSteamID, CSteamID) override { return false; }
    virtual bool SetLinkedLobby(CSteamID, CSteamID) override { return false; }
};

class CSteamMatchmakingServersStub : public ISteamMatchmakingServers
{
public:
    virtual HServerListRequest RequestInternetServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32, ISteamMatchmakingServerListResponse*) override { return nullptr; }
    virtual HServerListRequest RequestLANServerList(AppId_t, ISteamMatchmakingServerListResponse*) override { return nullptr; }
    virtual HServerListRequest RequestFriendsServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32, ISteamMatchmakingServerListResponse*) override { return nullptr; }
    virtual HServerListRequest RequestFavoritesServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32, ISteamMatchmakingServerListResponse*) override { return nullptr; }
    virtual HServerListRequest RequestHistoryServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32, ISteamMatchmakingServerListResponse*) override { return nullptr; }
    virtual HServerListRequest RequestSpectatorServerList(AppId_t, MatchMakingKeyValuePair_t**, uint32, ISteamMatchmakingServerListResponse*) override { return nullptr; }
    virtual void ReleaseRequest(HServerListRequest) override {}
    virtual gameserveritem_t *GetServerDetails(HServerListRequest, int) override { return nullptr; }
    virtual void CancelQuery(HServerListRequest) override {}
    virtual void RefreshQuery(HServerListRequest) override {}
    virtual bool IsRefreshing(HServerListRequest) override { return false; }
    virtual int GetServerCount(HServerListRequest) override { return 0; }
    virtual void RefreshServer(HServerListRequest, int) override {}
    virtual HServerQuery PingServer(uint32, uint16, ISteamMatchmakingPingResponse*) override { return 0; }
    virtual HServerQuery PlayerDetails(uint32, uint16, ISteamMatchmakingPlayersResponse*) override { return 0; }
    virtual HServerQuery ServerRules(uint32, uint16, ISteamMatchmakingRulesResponse*) override { return 0; }
    virtual void CancelServerQuery(HServerQuery) override {}
};

class CSteamUserStatsStub : public ISteamUserStats
{
public:
    virtual bool GetStat(const char*, int32 *pData) override { if (pData) *pData = 0; return true; }
    virtual bool GetStat(const char*, float *pData) override { if (pData) *pData = 0.0f; return true; }
    virtual bool SetStat(const char*, int32) override { return true; }
    virtual bool SetStat(const char*, float) override { return true; }
    virtual bool UpdateAvgRateStat(const char*, float, double) override { return true; }
    virtual bool GetAchievement(const char*, bool *pbAchieved) override { if (pbAchieved) *pbAchieved = false; return true; }
    virtual bool SetAchievement(const char*) override { return true; }
    virtual bool ClearAchievement(const char*) override { return true; }
    virtual bool GetAchievementAndUnlockTime(const char*, bool *pbAchieved, uint32 *punUnlockTime) override { if (pbAchieved) *pbAchieved = false; if (punUnlockTime) *punUnlockTime = 0; return true; }
    virtual bool StoreStats() override { return true; }
    virtual int GetAchievementIcon(const char*) override { return 0; }
    virtual const char *GetAchievementDisplayAttribute(const char*, const char*) override { return "0"; }
    virtual bool IndicateAchievementProgress(const char*, uint32, uint32) override { return true; }
    virtual uint32 GetNumAchievements() override { return 0; }
    virtual const char *GetAchievementName(uint32) override { return ""; }
    virtual SteamAPICall_t RequestUserStats(CSteamID) override { return k_uAPICallInvalid; }
    virtual bool GetUserStat(CSteamID, const char*, int32 *pData) override { if (pData) *pData = 0; return true; }
    virtual bool GetUserStat(CSteamID, const char*, float *pData) override { if (pData) *pData = 0.0f; return true; }
    virtual bool GetUserAchievement(CSteamID, const char*, bool *pbAchieved) override { if (pbAchieved) *pbAchieved = false; return true; }
    virtual bool GetUserAchievementAndUnlockTime(CSteamID, const char*, bool *pbAchieved, uint32 *punUnlockTime) override { if (pbAchieved) *pbAchieved = false; if (punUnlockTime) *punUnlockTime = 0; return true; }
    virtual bool ResetAllStats(bool) override { return true; }
    virtual SteamAPICall_t FindOrCreateLeaderboard(const char*, ELeaderboardSortMethod, ELeaderboardDisplayType) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t FindLeaderboard(const char*) override { return k_uAPICallInvalid; }
    virtual const char *GetLeaderboardName(SteamLeaderboard_t) override { return ""; }
    virtual int GetLeaderboardEntryCount(SteamLeaderboard_t) override { return 0; }
    virtual ELeaderboardSortMethod GetLeaderboardSortMethod(SteamLeaderboard_t) override { return k_ELeaderboardSortMethodNone; }
    virtual ELeaderboardDisplayType GetLeaderboardDisplayType(SteamLeaderboard_t) override { return k_ELeaderboardDisplayTypeNone; }
    virtual SteamAPICall_t DownloadLeaderboardEntries(SteamLeaderboard_t, ELeaderboardDataRequest, int, int) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t DownloadLeaderboardEntriesForUsers(SteamLeaderboard_t, CSteamID*, int) override { return k_uAPICallInvalid; }
    virtual bool GetDownloadedLeaderboardEntry(SteamLeaderboardEntries_t, int, LeaderboardEntry_t*, int32*, int) override { return false; }
    virtual SteamAPICall_t UploadLeaderboardScore(SteamLeaderboard_t, ELeaderboardUploadScoreMethod, int32, const int32*, int) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t AttachLeaderboardUGC(SteamLeaderboard_t, UGCHandle_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t GetNumberOfCurrentPlayers() override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t RequestGlobalAchievementPercentages() override { return k_uAPICallInvalid; }
    virtual int GetMostAchievedAchievementInfo(char*, uint32, float*, bool*) override { return -1; }
    virtual int GetNextMostAchievedAchievementInfo(int, char*, uint32, float*, bool*) override { return -1; }
    virtual bool GetAchievementAchievedPercent(const char*, float*) override { return false; }
    virtual SteamAPICall_t RequestGlobalStats(int) override { return k_uAPICallInvalid; }
    virtual bool GetGlobalStat(const char*, int64*) override { return false; }
    virtual bool GetGlobalStat(const char*, double*) override { return false; }
    virtual int32 GetGlobalStatHistory(const char*, int64*, uint32) override { return 0; }
    virtual int32 GetGlobalStatHistory(const char*, double*, uint32) override { return 0; }
    virtual bool GetAchievementProgressLimits(const char*, int32*, int32*) override { return false; }
    virtual bool GetAchievementProgressLimits(const char*, float*, float*) override { return false; }
};

class CSteamRemoteStorageStub : public ISteamRemoteStorage
{
public:
    virtual bool FileWrite(const char*, const void*, int32) override { return true; }
    virtual int32 FileRead(const char*, void*, int32) override { return 0; }
    virtual SteamAPICall_t FileWriteAsync(const char*, const void*, uint32) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t FileReadAsync(const char*, uint32, uint32) override { return k_uAPICallInvalid; }
    virtual bool FileReadAsyncComplete(SteamAPICall_t, void*, uint32) override { return true; }
    virtual bool FileForget(const char*) override { return true; }
    virtual bool FileDelete(const char*) override { return true; }
    virtual SteamAPICall_t FileShare(const char*) override { return k_uAPICallInvalid; }
    virtual bool SetSyncPlatforms(const char*, ERemoteStoragePlatform) override { return true; }
    virtual UGCFileWriteStreamHandle_t FileWriteStreamOpen(const char*) override { return 1; }
    virtual bool FileWriteStreamWriteChunk(UGCFileWriteStreamHandle_t, const void*, int32) override { return true; }
    virtual bool FileWriteStreamClose(UGCFileWriteStreamHandle_t) override { return true; }
    virtual bool FileWriteStreamCancel(UGCFileWriteStreamHandle_t) override { return true; }
    virtual bool FileExists(const char*) override { return false; }
    virtual bool FilePersisted(const char*) override { return false; }
    virtual int32 GetFileSize(const char*) override { return 0; }
    virtual int64 GetFileTimestamp(const char*) override { return 0; }
    virtual ERemoteStoragePlatform GetSyncPlatforms(const char*) override { return k_ERemoteStoragePlatformNone; }
    virtual int32 GetFileCount() override { return 0; }
    virtual const char *GetFileNameAndSize(int, int32*) override { return ""; }
    virtual bool GetQuota(uint64 *punTotalBytes, uint64 *punAvailableBytes) override { if (punTotalBytes) *punTotalBytes = 1024*1024*100; if (punAvailableBytes) *punAvailableBytes = 1024*1024*100; return true; }
    virtual bool IsCloudEnabledForAccount() override { return true; }
    virtual bool IsCloudEnabledForApp() override { return true; }
    virtual void SetCloudEnabledForApp(bool) override {}
    virtual SteamAPICall_t UGCDownload(UGCHandle_t, uint32) override { return k_uAPICallInvalid; }
    virtual bool GetUGCDownloadProgress(UGCHandle_t, int32*, int32*) override { return false; }
    virtual bool GetUGCDetails(UGCHandle_t, AppId_t*, char**, int32*, CSteamID*) override { return false; }
    virtual int32 UGCRead(UGCHandle_t, void*, int32, uint32, EUGCReadAction) override { return 0; }
    virtual int32 GetCachedUGCCount() override { return 0; }
    virtual UGCHandle_t GetCachedUGCHandle(int32) override { return k_UGCHandleInvalid; }
    virtual SteamAPICall_t PublishWorkshopFile(const char*, const char*, AppId_t, const char*, const char*, ERemoteStoragePublishedFileVisibility, SteamParamStringArray_t*, EWorkshopFileType) override { return k_uAPICallInvalid; }
    virtual PublishedFileUpdateHandle_t CreatePublishedFileUpdateRequest(PublishedFileId_t) override { return k_PublishedFileUpdateHandleInvalid; }
    virtual bool UpdatePublishedFileFile(PublishedFileUpdateHandle_t, const char*) override { return false; }
    virtual bool UpdatePublishedFilePreviewFile(PublishedFileUpdateHandle_t, const char*) override { return false; }
    virtual bool UpdatePublishedFileTitle(PublishedFileUpdateHandle_t, const char*) override { return false; }
    virtual bool UpdatePublishedFileDescription(PublishedFileUpdateHandle_t, const char*) override { return false; }
    virtual bool UpdatePublishedFileVisibility(PublishedFileUpdateHandle_t, ERemoteStoragePublishedFileVisibility) override { return false; }
    virtual bool UpdatePublishedFileTags(PublishedFileUpdateHandle_t, SteamParamStringArray_t*) override { return false; }
    virtual SteamAPICall_t CommitPublishedFileUpdate(PublishedFileUpdateHandle_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t GetPublishedFileDetails(PublishedFileId_t, uint32) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t DeletePublishedFile(PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t EnumerateUserPublishedFiles(uint32) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t SubscribePublishedFile(PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t EnumerateUserSubscribedFiles(uint32) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t UnsubscribePublishedFile(PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual bool UpdatePublishedFileSetChangeDescription(PublishedFileUpdateHandle_t, const char*) override { return false; }
    virtual SteamAPICall_t GetPublishedItemVoteDetails(PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t UpdateUserPublishedItemVote(PublishedFileId_t, bool) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t GetUserPublishedItemVoteDetails(PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t EnumerateUserSharedWorkshopFiles(CSteamID, uint32, SteamParamStringArray_t*, SteamParamStringArray_t*) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t PublishVideo(EWorkshopVideoProvider, const char*, const char*, const char*, AppId_t, const char*, const char*, ERemoteStoragePublishedFileVisibility, SteamParamStringArray_t*) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t SetUserPublishedFileAction(PublishedFileId_t, EWorkshopFileAction) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t EnumeratePublishedFilesByUserAction(EWorkshopFileAction, uint32) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t EnumeratePublishedWorkshopFiles(EWorkshopEnumerationType, uint32, uint32, uint32, SteamParamStringArray_t*, SteamParamStringArray_t*) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t UGCDownloadToLocation(UGCHandle_t, const char*, uint32) override { return k_uAPICallInvalid; }
    virtual int32 GetLocalFileChangeCount() override { return 0; }
    virtual const char *GetLocalFileChange(int, ERemoteStorageLocalFileChange*, ERemoteStorageFilePathType*) override { return nullptr; }
    virtual bool BeginFileWriteBatch() override { return false; }
    virtual bool EndFileWriteBatch() override { return false; }
};

class CSteamHTTPStub : public ISteamHTTP
{
public:
    static HTTPRequestHandle s_NextHandle;
    virtual HTTPRequestHandle CreateHTTPRequest(EHTTPMethod, const char*) override { s_NextHandle++; return s_NextHandle; }
    virtual bool SetHTTPRequestContextValue(HTTPRequestHandle, uint64) override { return true; }
    virtual bool SetHTTPRequestNetworkActivityTimeout(HTTPRequestHandle, uint32) override { return true; }
    virtual bool SetHTTPRequestHeaderValue(HTTPRequestHandle, const char*, const char*) override { return true; }
    virtual bool SetHTTPRequestGetOrPostParameter(HTTPRequestHandle, const char*, const char*) override { return true; }
    virtual bool SendHTTPRequest(HTTPRequestHandle, SteamAPICall_t*) override { return true; }
    virtual bool SendHTTPRequestAndStreamResponse(HTTPRequestHandle, SteamAPICall_t*) override { return true; }
    virtual bool DeferHTTPRequest(HTTPRequestHandle) override { return true; }
    virtual bool PrioritizeHTTPRequest(HTTPRequestHandle) override { return true; }
    virtual bool GetHTTPResponseHeaderSize(HTTPRequestHandle, const char*, uint32*) override { return true; }
    virtual bool GetHTTPResponseHeaderValue(HTTPRequestHandle, const char*, uint8*, uint32) override { return true; }
    virtual bool GetHTTPResponseBodySize(HTTPRequestHandle, uint32 *unBodySize) override { if (unBodySize) *unBodySize = 0; return true; }
    virtual bool GetHTTPResponseBodyData(HTTPRequestHandle, uint8*, uint32) override { return true; }
    virtual bool GetHTTPStreamingResponseBodyData(HTTPRequestHandle, uint32, uint8*, uint32) override { return true; }
    virtual bool ReleaseHTTPRequest(HTTPRequestHandle) override { return true; }
    virtual bool GetHTTPDownloadProgressPct(HTTPRequestHandle, float*) override { return true; }
    virtual bool SetHTTPRequestRawPostBody(HTTPRequestHandle, const char*, uint8*, uint32) override { return true; }
    virtual HTTPCookieContainerHandle CreateCookieContainer(bool) override { return 1; }
    virtual bool ReleaseCookieContainer(HTTPCookieContainerHandle) override { return true; }
    virtual bool SetCookie(HTTPCookieContainerHandle, const char*, const char*, const char*) override { return true; }
    virtual bool SetHTTPRequestCookieContainer(HTTPRequestHandle, HTTPCookieContainerHandle) override { return true; }
    virtual bool SetHTTPRequestUserAgentInfo(HTTPRequestHandle, const char*) override { return true; }
    virtual bool SetHTTPRequestRequiresVerifiedCertificate(HTTPRequestHandle, bool) override { return true; }
    virtual bool SetHTTPRequestAbsoluteTimeoutMS(HTTPRequestHandle, uint32) override { return true; }
    virtual bool GetHTTPRequestWasTimedOut(HTTPRequestHandle, bool *pWasTimedOut) override { if (pWasTimedOut) *pWasTimedOut = false; return true; }
};

HTTPRequestHandle CSteamHTTPStub::s_NextHandle = 0;

class CSteamInventoryStub : public ISteamInventory
{
public:
    virtual EResult GetResultStatus(SteamInventoryResult_t) override { return k_EResultOK; }
    virtual bool GetResultItems(SteamInventoryResult_t, SteamItemDetails_t*, uint32*) override { return true; }
    virtual bool GetResultItemProperty(SteamInventoryResult_t, uint32, const char*, char*, uint32*) override { return true; }
    virtual uint32 GetResultTimestamp(SteamInventoryResult_t) override { return (uint32)time(nullptr); }
    virtual bool CheckResultSteamID(SteamInventoryResult_t, CSteamID) override { return true; }
    virtual void DestroyResult(SteamInventoryResult_t) override {}
    virtual bool GetAllItems(SteamInventoryResult_t*) override { return true; }
    virtual bool GetItemsByID(SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32) override { return true; }
    virtual bool SerializeResult(SteamInventoryResult_t, void*, uint32*) override { return true; }
    virtual bool DeserializeResult(SteamInventoryResult_t*, const void*, uint32, bool = false) override { return true; }
    virtual bool GenerateItems(SteamInventoryResult_t*, const SteamItemDef_t*, const uint32*, uint32) override { return true; }
    virtual bool GrantPromoItems(SteamInventoryResult_t*) override { return true; }
    virtual bool AddPromoItem(SteamInventoryResult_t*, SteamItemDef_t) override { return true; }
    virtual bool AddPromoItems(SteamInventoryResult_t*, const SteamItemDef_t*, uint32) override { return true; }
    virtual bool ConsumeItem(SteamInventoryResult_t*, SteamItemInstanceID_t, uint32) override { return true; }
    virtual bool ExchangeItems(SteamInventoryResult_t*, const SteamItemDef_t*, const uint32*, uint32, const SteamItemInstanceID_t*, const uint32*, uint32) override { return true; }
    virtual bool TransferItemQuantity(SteamInventoryResult_t*, SteamItemInstanceID_t, uint32, SteamItemInstanceID_t) override { return true; }
    virtual void SendItemDropHeartbeat() override {}
    virtual bool TriggerItemDrop(SteamInventoryResult_t*, SteamItemDef_t) override { return true; }
    virtual bool TradeItems(SteamInventoryResult_t*, CSteamID, const SteamItemInstanceID_t*, const uint32*, uint32, const SteamItemInstanceID_t*, const uint32*, uint32) override { return true; }
    virtual bool LoadItemDefinitions() override { return true; }
    virtual bool GetItemDefinitionIDs(SteamItemDef_t*, uint32*) override { return true; }
    virtual bool GetItemDefinitionProperty(SteamItemDef_t, const char*, char*, uint32*) override { return true; }
    virtual SteamAPICall_t RequestEligiblePromoItemDefinitionsIDs(CSteamID) override { return k_uAPICallInvalid; }
    virtual bool GetEligiblePromoItemDefinitionIDs(CSteamID, SteamItemDef_t*, uint32*) override { return false; }
    virtual SteamAPICall_t StartPurchase(const SteamItemDef_t*, const uint32*, uint32) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t RequestPrices() override { return k_uAPICallInvalid; }
    virtual uint32 GetNumItemsWithPrices() override { return 0; }
    virtual bool GetItemsWithPrices(SteamItemDef_t*, uint64*, uint64*, uint32) override { return false; }
    virtual bool GetItemPrice(SteamItemDef_t, uint64*, uint64*) override { return false; }
    virtual SteamInventoryUpdateHandle_t StartUpdateProperties() override { return k_SteamInventoryUpdateHandleInvalid; }
    virtual bool RemoveProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*) override { return false; }
    virtual bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*, const char*) override { return true; }
    virtual bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*, bool) override { return true; }
    virtual bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*, int64) override { return true; }
    virtual bool SetProperty(SteamInventoryUpdateHandle_t, SteamItemInstanceID_t, const char*, float) override { return true; }
    virtual bool SubmitUpdateProperties(SteamInventoryUpdateHandle_t, SteamInventoryResult_t*) override { return true; }
    virtual bool InspectItem(SteamInventoryResult_t*, const char*) override { return true; }
};

class CSteamUGCStub : public ISteamUGC
{
public:
    static UGCQueryHandle_t s_NextQueryHandle;
    static UGCUpdateHandle_t s_NextUpdateHandle;
    virtual UGCQueryHandle_t CreateQueryUserUGCRequest(AccountID_t, EUserUGCList, EUGCMatchingUGCType, EUserUGCListSortOrder, AppId_t, AppId_t, uint32) override { s_NextQueryHandle++; return s_NextQueryHandle; }
    virtual UGCQueryHandle_t CreateQueryAllUGCRequest(EUGCQuery, EUGCMatchingUGCType, AppId_t, AppId_t, uint32) override { s_NextQueryHandle++; return s_NextQueryHandle; }
    virtual UGCQueryHandle_t CreateQueryAllUGCRequest(EUGCQuery, EUGCMatchingUGCType, AppId_t, AppId_t, const char* = NULL) override { s_NextQueryHandle++; return s_NextQueryHandle; }
    virtual UGCQueryHandle_t CreateQueryUGCDetailsRequest(PublishedFileId_t*, uint32) override { s_NextQueryHandle++; return s_NextQueryHandle; }
    virtual SteamAPICall_t SendQueryUGCRequest(UGCQueryHandle_t) override { return k_uAPICallInvalid; }
    virtual bool GetQueryUGCResult(UGCQueryHandle_t, uint32, SteamUGCDetails_t*) override { return false; }
    virtual uint32 GetQueryUGCNumTags(UGCQueryHandle_t, uint32) override { return 0; }
    virtual bool GetQueryUGCTag(UGCQueryHandle_t, uint32, uint32, char*, uint32) override { return false; }
    virtual bool GetQueryUGCTagDisplayName(UGCQueryHandle_t, uint32, uint32, char*, uint32) override { return false; }
    virtual bool GetQueryUGCPreviewURL(UGCQueryHandle_t, uint32, char*, uint32) override { return false; }
    virtual bool GetQueryUGCMetadata(UGCQueryHandle_t, uint32, char*, uint32) override { return false; }
    virtual bool GetQueryUGCChildren(UGCQueryHandle_t, uint32, PublishedFileId_t*, uint32) override { return false; }
    virtual bool GetQueryUGCStatistic(UGCQueryHandle_t, uint32, EItemStatistic, uint64*) override { return false; }
    virtual uint32 GetQueryUGCNumAdditionalPreviews(UGCQueryHandle_t, uint32) override { return 0; }
    virtual bool GetQueryUGCAdditionalPreview(UGCQueryHandle_t, uint32, uint32, char*, uint32, char*, uint32, EItemPreviewType*) override { return false; }
    virtual uint32 GetQueryUGCNumKeyValueTags(UGCQueryHandle_t, uint32) override { return 0; }
    virtual bool GetQueryUGCKeyValueTag(UGCQueryHandle_t, uint32, uint32, char*, uint32, char*, uint32) override { return false; }
    virtual bool GetQueryUGCKeyValueTag(UGCQueryHandle_t, uint32, const char*, char*, uint32) override { return false; }
    virtual uint32 GetNumSupportedGameVersions(UGCQueryHandle_t, uint32) override { return 0; }
    virtual bool GetSupportedGameVersionData(UGCQueryHandle_t, uint32, uint32, char*, char*, uint32) override { return false; }
    virtual uint32 GetQueryUGCContentDescriptors(UGCQueryHandle_t, uint32, EUGCContentDescriptorID*, uint32) override { return 0; }
    virtual bool ReleaseQueryUGCRequest(UGCQueryHandle_t) override { return true; }
    virtual bool AddRequiredTag(UGCQueryHandle_t, const char*) override { return true; }
    virtual bool AddRequiredTagGroup(UGCQueryHandle_t, const SteamParamStringArray_t*) override { return true; }
    virtual bool AddExcludedTag(UGCQueryHandle_t, const char*) override { return true; }
    virtual bool SetReturnOnlyIDs(UGCQueryHandle_t, bool) override { return true; }
    virtual bool SetReturnKeyValueTags(UGCQueryHandle_t, bool) override { return true; }
    virtual bool SetReturnLongDescription(UGCQueryHandle_t, bool) override { return true; }
    virtual bool SetReturnMetadata(UGCQueryHandle_t, bool) override { return true; }
    virtual bool SetReturnChildren(UGCQueryHandle_t, bool) override { return true; }
    virtual bool SetReturnAdditionalPreviews(UGCQueryHandle_t, bool) override { return true; }
    virtual bool SetReturnTotalOnly(UGCQueryHandle_t, bool) override { return true; }
    virtual bool SetReturnPlaytimeStats(UGCQueryHandle_t, uint32) override { return true; }
    virtual bool SetLanguage(UGCQueryHandle_t, const char*) override { return true; }
    virtual bool SetAllowCachedResponse(UGCQueryHandle_t, uint32) override { return true; }
    virtual bool SetAdminQuery(UGCUpdateHandle_t, bool) override { return true; }
    virtual bool SetCloudFileNameFilter(UGCQueryHandle_t, const char*) override { return true; }
    virtual bool SetMatchAnyTag(UGCQueryHandle_t, bool) override { return true; }
    virtual bool SetSearchText(UGCQueryHandle_t, const char*) override { return true; }
    virtual bool SetRankedByTrendDays(UGCQueryHandle_t, uint32) override { return true; }
    virtual bool SetTimeCreatedDateRange(UGCQueryHandle_t, RTime32, RTime32) override { return true; }
    virtual bool SetTimeUpdatedDateRange(UGCQueryHandle_t, RTime32, RTime32) override { return true; }
    virtual bool AddRequiredKeyValueTag(UGCQueryHandle_t, const char*, const char*) override { return true; }
    virtual SteamAPICall_t RequestUGCDetails(PublishedFileId_t, uint32) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t CreateItem(AppId_t, EWorkshopFileType) override { return k_uAPICallInvalid; }
    virtual UGCUpdateHandle_t StartItemUpdate(AppId_t, PublishedFileId_t) override { s_NextUpdateHandle++; return s_NextUpdateHandle; }
    virtual bool SetItemTitle(UGCUpdateHandle_t, const char*) override { return true; }
    virtual bool SetItemDescription(UGCUpdateHandle_t, const char*) override { return true; }
    virtual bool SetItemUpdateLanguage(UGCUpdateHandle_t, const char*) override { return true; }
    virtual bool SetItemMetadata(UGCUpdateHandle_t, const char*) override { return true; }
    virtual bool SetItemVisibility(UGCUpdateHandle_t, ERemoteStoragePublishedFileVisibility) override { return true; }
    virtual bool SetItemTags(UGCUpdateHandle_t, const SteamParamStringArray_t*, bool = false) override { return true; }
    virtual bool SetItemContent(UGCUpdateHandle_t, const char*) override { return true; }
    virtual bool SetItemPreview(UGCUpdateHandle_t, const char*) override { return true; }
    virtual bool SetAllowLegacyUpload(UGCUpdateHandle_t, bool) override { return true; }
    virtual bool RemoveAllItemKeyValueTags(UGCUpdateHandle_t) override { return true; }
    virtual bool RemoveItemKeyValueTags(UGCUpdateHandle_t, const char*) override { return true; }
    virtual bool AddItemKeyValueTag(UGCUpdateHandle_t, const char*, const char*) override { return true; }
    virtual bool AddItemPreviewFile(UGCUpdateHandle_t, const char*, EItemPreviewType) override { return true; }
    virtual bool AddItemPreviewVideo(UGCUpdateHandle_t, const char*) override { return true; }
    virtual bool UpdateItemPreviewFile(UGCUpdateHandle_t, uint32, const char*) override { return true; }
    virtual bool UpdateItemPreviewVideo(UGCUpdateHandle_t, uint32, const char*) override { return true; }
    virtual bool RemoveItemPreview(UGCUpdateHandle_t, uint32) override { return true; }
    virtual bool AddContentDescriptor(UGCUpdateHandle_t, EUGCContentDescriptorID) override { return true; }
    virtual bool RemoveContentDescriptor(UGCUpdateHandle_t, EUGCContentDescriptorID) override { return true; }
    virtual bool SetRequiredGameVersions(UGCUpdateHandle_t, const char*, const char*) override { return true; }
    virtual SteamAPICall_t SubmitItemUpdate(UGCUpdateHandle_t, const char*) override { return k_uAPICallInvalid; }
    virtual EItemUpdateStatus GetItemUpdateProgress(UGCUpdateHandle_t, uint64*, uint64*) override { return k_EItemUpdateStatusPreparingConfig; }
    virtual SteamAPICall_t SetUserItemVote(PublishedFileId_t, bool) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t GetUserItemVote(PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t AddItemToFavorites(AppId_t, PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t RemoveItemFromFavorites(AppId_t, PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t SubscribeItem(PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t UnsubscribeItem(PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual uint32 GetNumSubscribedItems(bool = false) override { return 0; }
    virtual uint32 GetSubscribedItems(PublishedFileId_t*, uint32, bool = false) override { return 0; }
    virtual uint32 GetItemState(PublishedFileId_t) override { return k_EItemStateInstalled; }
    virtual bool GetItemInstallInfo(PublishedFileId_t, uint64*, char*, uint32, uint32*) override { return true; }
    virtual bool GetItemDownloadInfo(PublishedFileId_t, uint64*, uint64*) override { return true; }
    virtual bool DownloadItem(PublishedFileId_t, bool) override { return true; }
    virtual bool BInitWorkshopForGameServer(DepotId_t, const char*) override { return true; }
    virtual void SuspendDownloads(bool) override {}
    virtual SteamAPICall_t StartPlaytimeTracking(PublishedFileId_t*, uint32) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t StopPlaytimeTracking(PublishedFileId_t*, uint32) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t StopPlaytimeTrackingForAllItems() override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t AddDependency(PublishedFileId_t, PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t RemoveDependency(PublishedFileId_t, PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t AddAppDependency(PublishedFileId_t, AppId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t RemoveAppDependency(PublishedFileId_t, AppId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t GetAppDependencies(PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t DeleteItem(PublishedFileId_t) override { return k_uAPICallInvalid; }
    virtual bool ShowWorkshopEULA() override { return true; }
    virtual SteamAPICall_t GetWorkshopEULAStatus() override { return k_uAPICallInvalid; }
    virtual uint32 GetUserContentDescriptorPreferences(EUGCContentDescriptorID*, uint32) override { return 0; }
    virtual bool SetItemsDisabledLocally(PublishedFileId_t*, uint32, bool) override { return true; }
    virtual bool SetSubscriptionsLoadOrder(PublishedFileId_t*, uint32) override { return true; }
};

UGCQueryHandle_t CSteamUGCStub::s_NextQueryHandle = 0;
UGCUpdateHandle_t CSteamUGCStub::s_NextUpdateHandle = 0;

class CSteamNetworkingStub : public ISteamNetworking
{
public:
    virtual bool SendP2PPacket(CSteamID, const void*, uint32, EP2PSend, int = 0) override { return true; }
    virtual bool IsP2PPacketAvailable(uint32*, int = 0) override { return false; }
    virtual bool ReadP2PPacket(void*, uint32, uint32*, CSteamID*, int = 0) override { return false; }
    virtual bool AcceptP2PSessionWithUser(CSteamID) override { return true; }
    virtual bool CloseP2PSessionWithUser(CSteamID) override { return true; }
    virtual bool CloseP2PChannelWithUser(CSteamID, int) override { return true; }
    virtual bool GetP2PSessionState(CSteamID, P2PSessionState_t*) override { return false; }
    virtual bool AllowP2PPacketRelay(bool) override { return true; }
    virtual SNetListenSocket_t CreateListenSocket(int, SteamIPAddress_t, uint16, bool) override { return 1; }
    virtual SNetSocket_t CreateP2PConnectionSocket(CSteamID, int, int, bool) override { return 1; }
    virtual SNetSocket_t CreateConnectionSocket(SteamIPAddress_t, uint16, int) override { return 1; }
    virtual bool DestroySocket(SNetSocket_t, bool) override { return true; }
    virtual bool DestroyListenSocket(SNetListenSocket_t, bool) override { return true; }
    virtual bool SendDataOnSocket(SNetSocket_t, void*, uint32, bool) override { return true; }
    virtual bool IsDataAvailableOnSocket(SNetSocket_t, uint32*) override { return false; }
    virtual bool RetrieveDataFromSocket(SNetSocket_t, void*, uint32, uint32*) override { return false; }
    virtual bool IsDataAvailable(SNetListenSocket_t, uint32*, SNetSocket_t*) override { return false; }
    virtual bool RetrieveData(SNetListenSocket_t, void*, uint32, uint32*, SNetSocket_t*) override { return false; }
    virtual bool GetSocketInfo(SNetSocket_t, CSteamID*, int*, SteamIPAddress_t*, uint16*) override { return false; }
    virtual bool GetListenSocketInfo(SNetListenSocket_t, SteamIPAddress_t*, uint16*) override { return false; }
    virtual ESNetSocketConnectionType GetSocketConnectionType(SNetSocket_t) override { return k_ESNetSocketConnectionTypeNotConnected; }
    virtual int GetMaxPacketSize(SNetSocket_t) override { return 1200; }
};

class CSteamNetworkingMessagesStub : public ISteamNetworkingMessages
{
public:
    virtual EResult SendMessageToUser(const SteamNetworkingIdentity&, const void*, uint32, int, int) override { return k_EResultOK; }
    virtual int ReceiveMessagesOnChannel(int, SteamNetworkingMessage_t**, int) override { return 0; }
    virtual bool AcceptSessionWithUser(const SteamNetworkingIdentity&) override { return true; }
    virtual bool CloseSessionWithUser(const SteamNetworkingIdentity&) override { return true; }
    virtual bool CloseChannelWithUser(const SteamNetworkingIdentity&, int) override { return true; }
    virtual ESteamNetworkingConnectionState GetSessionConnectionInfo(const SteamNetworkingIdentity&, SteamNetConnectionInfo_t*, SteamNetConnectionRealTimeStatus_t*) override { return k_ESteamNetworkingConnectionState_None; }
};

class CSteamNetworkingSocketsStub : public ISteamNetworkingSockets
{
public:
    virtual HSteamListenSocket CreateListenSocketIP(const SteamNetworkingIPAddr&, int, const SteamNetworkingConfigValue_t*) override {
        static uint32 s_listenCounter = 1;
        s_listenCounter++;
        UCOLOG("[UCOffline] CreateListenSocketIP -> %u\r\n", s_listenCounter);
        return (HSteamListenSocket)s_listenCounter;
    }
    virtual HSteamNetConnection ConnectByIPAddress(const SteamNetworkingIPAddr&, int, const SteamNetworkingConfigValue_t*) override {
        static uint32 s_connCounter = 1;
        s_connCounter++;
        UCOLOG("[UCOffline] ConnectByIPAddress -> %u\r\n", s_connCounter);
        return (HSteamNetConnection)s_connCounter;
    }
    virtual HSteamListenSocket CreateListenSocketP2P(int, int, const SteamNetworkingConfigValue_t*) override {
        static uint32 s_p2pListenCounter = 1;
        s_p2pListenCounter++;
        return (HSteamListenSocket)s_p2pListenCounter;
    }
    virtual HSteamNetConnection ConnectP2P(const SteamNetworkingIdentity&, int, int, const SteamNetworkingConfigValue_t*) override {
        static uint32 s_p2pConnCounter = 1;
        s_p2pConnCounter++;
        return (HSteamNetConnection)s_p2pConnCounter;
    }
    virtual EResult AcceptConnection(HSteamNetConnection) override { return k_EResultOK; }
    virtual bool CloseConnection(HSteamNetConnection, int, const char*, bool) override { return true; }
    virtual bool CloseListenSocket(HSteamListenSocket) override { return true; }
    virtual bool SetConnectionUserData(HSteamNetConnection, int64) override { return true; }
    virtual int64 GetConnectionUserData(HSteamNetConnection) override { return 0; }
    virtual void SetConnectionName(HSteamNetConnection, const char*) override {}
    virtual bool GetConnectionName(HSteamNetConnection, char* szName, int cbName) override { if (szName && cbName > 0) { strncpy(szName, "local", cbName); szName[cbName-1] = 0; } return true; }
    virtual EResult SendMessageToConnection(HSteamNetConnection, const void*, uint32, int, int64*) override { return k_EResultOK; }
    virtual void SendMessages(int, SteamNetworkingMessage_t* const*, int64*) override {}
    virtual EResult FlushMessagesOnConnection(HSteamNetConnection) override { return k_EResultOK; }
    virtual int ReceiveMessagesOnConnection(HSteamNetConnection, SteamNetworkingMessage_t**, int) override { return 0; }
    virtual bool GetConnectionInfo(HSteamNetConnection, SteamNetConnectionInfo_t*) override { return true; }
    virtual EResult GetConnectionRealTimeStatus(HSteamNetConnection, SteamNetConnectionRealTimeStatus_t*, int, SteamNetConnectionRealTimeLaneStatus_t*) override { return k_EResultOK; }
    virtual int GetDetailedConnectionStatus(HSteamNetConnection, char*, int) override { return 0; }
    virtual bool GetListenSocketAddress(HSteamListenSocket, SteamNetworkingIPAddr*) override { return true; }
    virtual bool CreateSocketPair(HSteamNetConnection*, HSteamNetConnection*, bool, const SteamNetworkingIdentity*, const SteamNetworkingIdentity*) override { return false; }
    virtual EResult ConfigureConnectionLanes(HSteamNetConnection, int, const int*, const uint16*) override { return k_EResultOK; }
    virtual bool GetIdentity(SteamNetworkingIdentity *pIdentity) override {
        if (pIdentity) {
            pIdentity->Clear();
            pIdentity->SetSteamID64(CSteamID(g_ForcedAppId, k_EUniversePublic, k_EAccountTypeIndividual).ConvertToUint64());
        }
        return true;
    }
    virtual ESteamNetworkingAvailability InitAuthentication() override {
        UCOLOG("[UCOffline] NetworkingSockets InitAuthentication -> Current\r\n");
        CCallbackDispatcher* pDisp = GetDispatcher();
        if (pDisp) {
            SteamNetAuthenticationStatus_t data{};
            data.m_eAvail = k_ESteamNetworkingAvailability_Current;
            strncpy(data.m_debugMsg, "OK", sizeof(data.m_debugMsg));
            pDisp->PostCallback(g_ClientUser, data.k_iCallback, &data, sizeof(data));
        }
        return k_ESteamNetworkingAvailability_Current;
    }
    virtual ESteamNetworkingAvailability GetAuthenticationStatus(SteamNetAuthenticationStatus_t *pDetails) override {
        if (pDetails) {
            pDetails->m_eAvail = k_ESteamNetworkingAvailability_Current;
            strncpy(pDetails->m_debugMsg, "OK", sizeof(pDetails->m_debugMsg));
        }
        return k_ESteamNetworkingAvailability_Current;
    }
    virtual HSteamNetPollGroup CreatePollGroup() override {
        static uint32 s_pollGroupCounter = 1;
        s_pollGroupCounter++;
        return (HSteamNetPollGroup)s_pollGroupCounter;
    }
    virtual bool DestroyPollGroup(HSteamNetPollGroup) override { return true; }
    virtual bool SetConnectionPollGroup(HSteamNetConnection, HSteamNetPollGroup) override { return true; }
    virtual int ReceiveMessagesOnPollGroup(HSteamNetPollGroup, SteamNetworkingMessage_t**, int) override { return 0; }
    virtual bool ReceivedRelayAuthTicket(const void*, int, SteamDatagramRelayAuthTicket*) override { return true; }
    virtual int FindRelayAuthTicketForServer(const SteamNetworkingIdentity&, int, SteamDatagramRelayAuthTicket*) override { return 0; }
    virtual HSteamNetConnection ConnectToHostedDedicatedServer(const SteamNetworkingIdentity&, int, int, const SteamNetworkingConfigValue_t*) override {
        static uint32 s_hostedCounter = 1;
        s_hostedCounter++;
        return (HSteamNetConnection)s_hostedCounter;
    }
    virtual uint16 GetHostedDedicatedServerPort() override { return 0; }
    virtual SteamNetworkingPOPID GetHostedDedicatedServerPOPID() override { return 0; }
    virtual EResult GetHostedDedicatedServerAddress(SteamDatagramHostedAddress*) override { return k_EResultOK; }
    virtual HSteamListenSocket CreateHostedDedicatedServerListenSocket(int, int, const SteamNetworkingConfigValue_t*) override {
        static uint32 s_hostedListenCounter = 1;
        s_hostedListenCounter++;
        return (HSteamListenSocket)s_hostedListenCounter;
    }
    virtual EResult GetGameCoordinatorServerLogin(SteamDatagramGameCoordinatorServerLogin*, int*, void*) override { return k_EResultOK; }
    virtual HSteamNetConnection ConnectP2PCustomSignaling(ISteamNetworkingConnectionSignaling*, const SteamNetworkingIdentity*, int, int, const SteamNetworkingConfigValue_t*) override {
        static uint32 s_customSigCounter = 1;
        s_customSigCounter++;
        return (HSteamNetConnection)s_customSigCounter;
    }
    virtual bool ReceivedP2PCustomSignal(const void*, int, ISteamNetworkingSignalingRecvContext*) override { return true; }
    virtual bool GetCertificateRequest(int*, void*, SteamNetworkingErrMsg&) override { return false; }
    virtual bool SetCertificate(const void*, int, SteamNetworkingErrMsg&) override { return true; }
    virtual void ResetIdentity(const SteamNetworkingIdentity*) override {}
    virtual void RunCallbacks() override {}
    virtual bool BeginAsyncRequestFakeIP(int) override { return true; }
    virtual void GetFakeIP(int, SteamNetworkingFakeIPResult_t*) override {}
    virtual HSteamListenSocket CreateListenSocketP2PFakeIP(int, int, const SteamNetworkingConfigValue_t*) override {
        static uint32 s_fakeIPCounter = 1;
        s_fakeIPCounter++;
        return (HSteamListenSocket)s_fakeIPCounter;
    }
    virtual EResult GetRemoteFakeIPForConnection(HSteamNetConnection, SteamNetworkingIPAddr*) override { return k_EResultOK; }
    virtual ISteamNetworkingFakeUDPPort *CreateFakeUDPPort(int) override { return nullptr; }
};

class CSteamNetworkingUtilsStub : public ISteamNetworkingUtils
{
public:
    virtual SteamNetworkingMessage_t *AllocateMessage(int) override { return nullptr; }
    virtual ESteamNetworkingAvailability GetRelayNetworkStatus(SteamRelayNetworkStatus_t *pDetails) override {
        if (pDetails) {
            pDetails->m_eAvail = k_ESteamNetworkingAvailability_Current;
            pDetails->m_bPingMeasurementInProgress = false;
            pDetails->m_eAvailNetworkConfig = k_ESteamNetworkingAvailability_Current;
            pDetails->m_eAvailAnyRelay = k_ESteamNetworkingAvailability_Current;
            strncpy(pDetails->m_debugMsg, "OK", sizeof(pDetails->m_debugMsg));
        }
        return k_ESteamNetworkingAvailability_Current;
    }
    virtual float GetLocalPingLocation(SteamNetworkPingLocation_t&) override { return 0; }
    virtual int EstimatePingTimeBetweenTwoLocations(const SteamNetworkPingLocation_t&, const SteamNetworkPingLocation_t&) override { return 0; }
    virtual int EstimatePingTimeFromLocalHost(const SteamNetworkPingLocation_t&) override { return 0; }
    virtual void ConvertPingLocationToString(const SteamNetworkPingLocation_t&, char*, int) override {}
    virtual bool ParsePingLocationString(const char*, SteamNetworkPingLocation_t&) override { return false; }
    virtual bool CheckPingDataUpToDate(float) override { return false; }
    virtual int GetPingToDataCenter(SteamNetworkingPOPID, SteamNetworkingPOPID*) override { return 0; }
    virtual int GetDirectPingToPOP(SteamNetworkingPOPID) override { return 0; }
    virtual int GetPOPCount() override { return 0; }
    virtual int GetPOPList(SteamNetworkingPOPID*, int) override { return 0; }
    virtual SteamNetworkingMicroseconds GetLocalTimestamp() override { return 0; }
    virtual void SetDebugOutputFunction(ESteamNetworkingSocketsDebugOutputType, FSteamNetworkingSocketsDebugOutput) override {}
    virtual ESteamNetworkingFakeIPType GetIPv4FakeIPType(uint32) override { return k_ESteamNetworkingFakeIPType_Invalid; }
    virtual EResult GetRealIdentityForFakeIP(const SteamNetworkingIPAddr&, SteamNetworkingIdentity*) override { return k_EResultInvalidParam; }
    virtual bool SetConfigValue(ESteamNetworkingConfigValue, ESteamNetworkingConfigScope, intptr_t, ESteamNetworkingConfigDataType, const void*) override { return false; }
    virtual ESteamNetworkingGetConfigValueResult GetConfigValue(ESteamNetworkingConfigValue, ESteamNetworkingConfigScope, intptr_t, ESteamNetworkingConfigDataType*, void*, size_t*) override { return k_ESteamNetworkingGetConfigValue_BadScopeObj; }
    virtual const char *GetConfigValueInfo(ESteamNetworkingConfigValue, ESteamNetworkingConfigDataType*, ESteamNetworkingConfigScope*) override { return nullptr; }
    virtual ESteamNetworkingConfigValue IterateGenericEditableConfigValues(ESteamNetworkingConfigValue, bool) override { return k_ESteamNetworkingConfig_Invalid; }
    virtual void SteamNetworkingIPAddr_ToString(const SteamNetworkingIPAddr&, char*, size_t, bool) override {}
    virtual bool SteamNetworkingIPAddr_ParseString(SteamNetworkingIPAddr*, const char*) override { return false; }
    virtual ESteamNetworkingFakeIPType SteamNetworkingIPAddr_GetFakeIPType(const SteamNetworkingIPAddr&) override { return k_ESteamNetworkingFakeIPType_Invalid; }
    virtual void SteamNetworkingIdentity_ToString(const SteamNetworkingIdentity&, char*, size_t) override {}
    virtual bool SteamNetworkingIdentity_ParseString(SteamNetworkingIdentity*, const char*) override { return false; }
};

class CSteamInputStub : public ISteamInput
{
public:
    virtual bool Init(bool = false) override { return false; }
    virtual bool Shutdown() override { return false; }
    virtual bool SetInputActionManifestFilePath(const char*) override { return false; }
    virtual void RunFrame(bool = true) override {}
    virtual bool BWaitForData(bool, uint32) override { return false; }
    virtual bool BNewDataAvailable() override { return false; }
    virtual int GetConnectedControllers(InputHandle_t*) override { return 0; }
    virtual void EnableDeviceCallbacks() override {}
    virtual void EnableActionEventCallbacks(SteamInputActionEventCallbackPointer) override {}
    virtual InputActionSetHandle_t GetActionSetHandle(const char*) override { return 0; }
    virtual void ActivateActionSet(InputHandle_t, InputActionSetHandle_t) override {}
    virtual InputActionSetHandle_t GetCurrentActionSet(InputHandle_t) override { return 0; }
    virtual void ActivateActionSetLayer(InputHandle_t, InputActionSetHandle_t) override {}
    virtual void DeactivateActionSetLayer(InputHandle_t, InputActionSetHandle_t) override {}
    virtual void DeactivateAllActionSetLayers(InputHandle_t) override {}
    virtual int GetActiveActionSetLayers(InputHandle_t, InputActionSetHandle_t*) override { return 0; }
    virtual InputDigitalActionHandle_t GetDigitalActionHandle(const char*) override { return 0; }
    virtual InputDigitalActionData_t GetDigitalActionData(InputHandle_t, InputDigitalActionHandle_t) override { return {}; }
    virtual int GetDigitalActionOrigins(InputHandle_t, InputActionSetHandle_t, InputDigitalActionHandle_t, EInputActionOrigin*) override { return 0; }
    virtual const char *GetStringForDigitalActionName(InputDigitalActionHandle_t) override { return ""; }
    virtual InputAnalogActionHandle_t GetAnalogActionHandle(const char*) override { return 0; }
    virtual InputAnalogActionData_t GetAnalogActionData(InputHandle_t, InputAnalogActionHandle_t) override { return {}; }
    virtual int GetAnalogActionOrigins(InputHandle_t, InputActionSetHandle_t, InputAnalogActionHandle_t, EInputActionOrigin*) override { return 0; }
    virtual const char *GetGlyphPNGForActionOrigin(EInputActionOrigin, ESteamInputGlyphSize, uint32) override { return ""; }
    virtual const char *GetGlyphSVGForActionOrigin(EInputActionOrigin, uint32) override { return ""; }
    virtual const char *GetGlyphForActionOrigin_Legacy(EInputActionOrigin) override { return ""; }
    virtual const char *GetStringForActionOrigin(EInputActionOrigin) override { return ""; }
    virtual const char *GetStringForAnalogActionName(InputAnalogActionHandle_t) override { return ""; }
    virtual void StopAnalogActionMomentum(InputHandle_t, InputAnalogActionHandle_t) override {}
    virtual InputMotionData_t GetMotionData(InputHandle_t) override { return {}; }
    virtual void TriggerVibration(InputHandle_t, unsigned short, unsigned short) override {}
    virtual void TriggerVibrationExtended(InputHandle_t, unsigned short, unsigned short, unsigned short, unsigned short) override {}
    virtual void TriggerSimpleHapticEvent(InputHandle_t, EControllerHapticLocation, uint8, char, uint8, char) override {}
    virtual void SetLEDColor(InputHandle_t, uint8, uint8, uint8, unsigned int) override {}
    virtual void Legacy_TriggerHapticPulse(InputHandle_t, ESteamControllerPad, unsigned short) override {}
    virtual void Legacy_TriggerRepeatedHapticPulse(InputHandle_t, ESteamControllerPad, unsigned short, unsigned short, unsigned short, unsigned int) override {}
    virtual bool ShowBindingPanel(InputHandle_t) override { return false; }
    virtual ESteamInputType GetInputTypeForHandle(InputHandle_t) override { return k_ESteamInputType_Unknown; }
    virtual InputHandle_t GetControllerForGamepadIndex(int) override { return 0; }
    virtual int GetGamepadIndexForController(InputHandle_t) override { return -1; }
    virtual const char *GetStringForXboxOrigin(EXboxOrigin) override { return ""; }
    virtual const char *GetGlyphForXboxOrigin(EXboxOrigin) override { return ""; }
    virtual EInputActionOrigin GetActionOriginFromXboxOrigin(InputHandle_t, EXboxOrigin) override { return k_EInputActionOrigin_None; }
    virtual EInputActionOrigin TranslateActionOrigin(ESteamInputType, EInputActionOrigin) override { return k_EInputActionOrigin_None; }
    virtual bool GetDeviceBindingRevision(InputHandle_t, int*, int*) override { return false; }
    virtual uint32 GetRemotePlaySessionID(InputHandle_t) override { return 0; }
    virtual uint16 GetSessionInputConfigurationSettings() override { return 0; }
    virtual void SetDualSenseTriggerEffect(InputHandle_t, const ScePadTriggerEffectParam*) override {}
};

class CSteamControllerStub : public ISteamController
{
public:
    virtual bool Init() override { return false; }
    virtual bool Shutdown() override { return false; }
    virtual void RunFrame() override {}
    virtual int GetConnectedControllers(ControllerHandle_t*) override { return 0; }
    virtual ControllerActionSetHandle_t GetActionSetHandle(const char*) override { return 0; }
    virtual void ActivateActionSet(ControllerHandle_t, ControllerActionSetHandle_t) override {}
    virtual ControllerActionSetHandle_t GetCurrentActionSet(ControllerHandle_t) override { return 0; }
    virtual void ActivateActionSetLayer(ControllerHandle_t, ControllerActionSetHandle_t) override {}
    virtual void DeactivateActionSetLayer(ControllerHandle_t, ControllerActionSetHandle_t) override {}
    virtual void DeactivateAllActionSetLayers(ControllerHandle_t) override {}
    virtual int GetActiveActionSetLayers(ControllerHandle_t, ControllerActionSetHandle_t*) override { return 0; }
    virtual ControllerDigitalActionHandle_t GetDigitalActionHandle(const char*) override { return 0; }
    virtual ControllerDigitalActionData_t GetDigitalActionData(ControllerHandle_t, ControllerDigitalActionHandle_t) override { return {}; }
    virtual int GetDigitalActionOrigins(ControllerHandle_t, ControllerActionSetHandle_t, ControllerDigitalActionHandle_t, EControllerActionOrigin*) override { return 0; }
    virtual ControllerAnalogActionHandle_t GetAnalogActionHandle(const char*) override { return 0; }
    virtual ControllerAnalogActionData_t GetAnalogActionData(ControllerHandle_t, ControllerAnalogActionHandle_t) override { return {}; }
    virtual int GetAnalogActionOrigins(ControllerHandle_t, ControllerActionSetHandle_t, ControllerAnalogActionHandle_t, EControllerActionOrigin*) override { return 0; }
    virtual const char *GetGlyphForActionOrigin(EControllerActionOrigin) override { return ""; }
    virtual const char *GetStringForActionOrigin(EControllerActionOrigin) override { return ""; }
    virtual void StopAnalogActionMomentum(ControllerHandle_t, ControllerAnalogActionHandle_t) override {}
    virtual ControllerMotionData_t GetMotionData(ControllerHandle_t) override { return {}; }
    virtual void TriggerHapticPulse(ControllerHandle_t, ESteamControllerPad, unsigned short) override {}
    virtual void TriggerRepeatedHapticPulse(ControllerHandle_t, ESteamControllerPad, unsigned short, unsigned short, unsigned short, unsigned int) override {}
    virtual void TriggerVibration(ControllerHandle_t, unsigned short, unsigned short) override {}
    virtual void SetLEDColor(ControllerHandle_t, uint8, uint8, uint8, unsigned int) override {}
    virtual bool ShowBindingPanel(ControllerHandle_t) override { return false; }
    virtual ESteamInputType GetInputTypeForHandle(ControllerHandle_t) override { return k_ESteamInputType_Unknown; }
    virtual ControllerHandle_t GetControllerForGamepadIndex(int) override { return 0; }
    virtual int GetGamepadIndexForController(ControllerHandle_t) override { return -1; }
    virtual const char *GetStringForXboxOrigin(EXboxOrigin) override { return ""; }
    virtual const char *GetGlyphForXboxOrigin(EXboxOrigin) override { return ""; }
    virtual EControllerActionOrigin GetActionOriginFromXboxOrigin(ControllerHandle_t, EXboxOrigin) override { return k_EControllerActionOrigin_None; }
    virtual EControllerActionOrigin TranslateActionOrigin(ESteamInputType, EControllerActionOrigin) override { return k_EControllerActionOrigin_None; }
    virtual bool GetControllerBindingRevision(ControllerHandle_t, int*, int*) override { return false; }
};

class CSteamHTMLSurfaceStub : public ISteamHTMLSurface
{
public:
    virtual bool Init() override { return false; }
    virtual bool Shutdown() override { return false; }
    virtual SteamAPICall_t CreateBrowser(const char*, const char*) override { return k_uAPICallInvalid; }
    virtual void RemoveBrowser(HHTMLBrowser) override {}
    virtual void LoadURL(HHTMLBrowser, const char*, const char*) override {}
    virtual void SetSize(HHTMLBrowser, uint32, uint32) override {}
    virtual void StopLoad(HHTMLBrowser) override {}
    virtual void Reload(HHTMLBrowser) override {}
    virtual void GoBack(HHTMLBrowser) override {}
    virtual void GoForward(HHTMLBrowser) override {}
    virtual void AddHeader(HHTMLBrowser, const char*, const char*) override {}
    virtual void ExecuteJavascript(HHTMLBrowser, const char*) override {}
    virtual void MouseUp(HHTMLBrowser, EHTMLMouseButton) override {}
    virtual void MouseDown(HHTMLBrowser, EHTMLMouseButton) override {}
    virtual void MouseDoubleClick(HHTMLBrowser, EHTMLMouseButton) override {}
    virtual void MouseMove(HHTMLBrowser, int, int) override {}
    virtual void MouseWheel(HHTMLBrowser, int32) override {}
    virtual void KeyDown(HHTMLBrowser, uint32, EHTMLKeyModifiers, bool = false) override {}
    virtual void KeyUp(HHTMLBrowser, uint32, EHTMLKeyModifiers) override {}
    virtual void KeyChar(HHTMLBrowser, uint32, EHTMLKeyModifiers) override {}
    virtual void SetHorizontalScroll(HHTMLBrowser, uint32) override {}
    virtual void SetVerticalScroll(HHTMLBrowser, uint32) override {}
    virtual void SetKeyFocus(HHTMLBrowser, bool) override {}
    virtual void ViewSource(HHTMLBrowser) override {}
    virtual void CopyToClipboard(HHTMLBrowser) override {}
    virtual void PasteFromClipboard(HHTMLBrowser) override {}
    virtual void Find(HHTMLBrowser, const char*, bool, bool) override {}
    virtual void StopFind(HHTMLBrowser) override {}
    virtual void GetLinkAtPosition(HHTMLBrowser, int, int) override {}
    virtual void SetCookie(const char*, const char*, const char*, const char* = "/", RTime32 = 0, bool = false, bool = false) override {}
    virtual void SetPageScaleFactor(HHTMLBrowser, float, int, int) override {}
    virtual void SetBackgroundMode(HHTMLBrowser, bool) override {}
    virtual void SetDPIScalingFactor(HHTMLBrowser, float) override {}
    virtual void OpenDeveloperTools(HHTMLBrowser) override {}
    virtual void AllowStartRequest(HHTMLBrowser, bool) override {}
    virtual void JSDialogResponse(HHTMLBrowser, bool) override {}
    virtual void FileLoadDialogResponse(HHTMLBrowser, const char**) override {}
};

class CSteamRemotePlayStub : public ISteamRemotePlay
{
public:
    virtual uint32 GetSessionCount() override { return 0; }
    virtual RemotePlaySessionID_t GetSessionID(int) override { return 0; }
    virtual CSteamID GetSessionSteamID(RemotePlaySessionID_t) override { return k_steamIDNil; }
    virtual const char *GetSessionClientName(RemotePlaySessionID_t) override { return ""; }
    virtual ESteamDeviceFormFactor GetSessionClientFormFactor(RemotePlaySessionID_t) override { return k_ESteamDeviceFormFactorUnknown; }
    virtual bool BGetSessionClientResolution(RemotePlaySessionID_t, int*, int*) override { return false; }
    virtual bool ShowRemotePlayTogetherUI() override { return false; }
    virtual bool BSendRemotePlayTogetherInvite(CSteamID) override { return false; }
    virtual bool BEnableRemotePlayTogetherDirectInput() override { return false; }
    virtual void DisableRemotePlayTogetherDirectInput() override {}
    virtual uint32 GetInput(RemotePlayInput_t*, uint32) override { return 0; }
    virtual void SetMouseVisibility(RemotePlaySessionID_t, bool) override {}
    virtual void SetMousePosition(RemotePlaySessionID_t, float, float) override {}
    virtual RemotePlayCursorID_t CreateMouseCursor(int, int, int, int, const void*, int = 0) override { return 0; }
    virtual void SetMouseCursor(RemotePlaySessionID_t, RemotePlayCursorID_t) override {}
};

class CSteamTimelineStub : public ISteamTimeline
{
public:
    virtual void SetTimelineTooltip(const char*, float) override {}
    virtual void ClearTimelineTooltip(float) override {}
    virtual void SetTimelineGameMode(ETimelineGameMode) override {}
    virtual TimelineEventHandle_t AddInstantaneousTimelineEvent(const char*, const char*, const char*, uint32, float = 0.f, ETimelineEventClipPriority = k_ETimelineEventClipPriority_None) override { return 0; }
    virtual TimelineEventHandle_t AddRangeTimelineEvent(const char*, const char*, const char*, uint32, float = 0.f, float = 0.f, ETimelineEventClipPriority = k_ETimelineEventClipPriority_None) override { return 0; }
    virtual TimelineEventHandle_t StartRangeTimelineEvent(const char*, const char*, const char*, uint32, float, ETimelineEventClipPriority) override { return 0; }
    virtual void UpdateRangeTimelineEvent(TimelineEventHandle_t, const char*, const char*, const char*, uint32, ETimelineEventClipPriority) override {}
    virtual void EndRangeTimelineEvent(TimelineEventHandle_t, float) override {}
    virtual void RemoveTimelineEvent(TimelineEventHandle_t) override {}
    virtual SteamAPICall_t DoesEventRecordingExist(TimelineEventHandle_t) override { return k_uAPICallInvalid; }
    virtual void StartGamePhase() override {}
    virtual void EndGamePhase() override {}
    virtual void SetGamePhaseID(const char*) override {}
    virtual SteamAPICall_t DoesGamePhaseRecordingExist(const char*) override { return k_uAPICallInvalid; }
    virtual void AddGamePhaseTag(const char*, const char*, const char*, uint32) override {}
    virtual void SetGamePhaseAttribute(const char*, const char*, uint32) override {}
    virtual void OpenOverlayToGamePhase(const char*) override {}
    virtual void OpenOverlayToTimelineEvent(TimelineEventHandle_t) override {}
};

class CSteamVideoStub : public ISteamVideo
{
public:
    virtual void GetVideoURL(AppId_t) override {}
    virtual bool IsBroadcasting(int*) override { return false; }
    virtual void GetOPFSettings(AppId_t) override {}
    virtual bool GetOPFStringForApp(AppId_t, char*, int32*) override { return false; }
};

class CSteamParentalSettingsStub : public ISteamParentalSettings
{
public:
    virtual bool BIsParentalLockEnabled() override { return false; }
    virtual bool BIsParentalLockLocked() override { return false; }
    virtual bool BIsAppBlocked(AppId_t) override { return false; }
    virtual bool BIsAppInBlockList(AppId_t) override { return false; }
    virtual bool BIsFeatureBlocked(EParentalFeature) override { return false; }
    virtual bool BIsFeatureInBlockList(EParentalFeature) override { return false; }
};

class CSteamMusicStub : public ISteamMusic
{
public:
    virtual bool BIsEnabled() override { return false; }
    virtual bool BIsPlaying() override { return false; }
    virtual AudioPlayback_Status GetPlaybackStatus() override { return AudioPlayback_Idle; }
    virtual void Play() override {}
    virtual void Pause() override {}
    virtual void PlayPrevious() override {}
    virtual void PlayNext() override {}
    virtual void SetVolume(float) override {}
    virtual float GetVolume() override { return 0.0f; }
};

class CSteamPartiesStub : public ISteamParties
{
public:
    virtual uint32 GetNumActiveBeacons() override { return 0; }
    virtual PartyBeaconID_t GetBeaconByIndex(uint32) override { return k_ulPartyBeaconIdInvalid; }
    virtual bool GetBeaconDetails(PartyBeaconID_t, CSteamID*, SteamPartyBeaconLocation_t*, char*, int) override { return false; }
    virtual SteamAPICall_t JoinParty(PartyBeaconID_t) override { return k_uAPICallInvalid; }
    virtual bool GetNumAvailableBeaconLocations(uint32*) override { return false; }
    virtual bool GetAvailableBeaconLocations(SteamPartyBeaconLocation_t*, uint32) override { return false; }
    virtual SteamAPICall_t CreateBeacon(uint32, SteamPartyBeaconLocation_t*, const char*, const char*) override { return k_uAPICallInvalid; }
    virtual void OnReservationCompleted(PartyBeaconID_t, CSteamID) override {}
    virtual void CancelReservation(PartyBeaconID_t, CSteamID) override {}
    virtual SteamAPICall_t ChangeNumOpenSlots(PartyBeaconID_t, uint32) override { return k_uAPICallInvalid; }
    virtual bool DestroyBeacon(PartyBeaconID_t) override { return false; }
    virtual bool GetBeaconLocationData(SteamPartyBeaconLocation_t, ESteamPartyBeaconLocationData, char*, int) override { return false; }
};

class CSteamScreenshotsStub : public ISteamScreenshots
{
public:
    virtual ScreenshotHandle WriteScreenshot(void*, uint32, int, int) override { static uint32 s_ss = 1; return s_ss++; }
    virtual ScreenshotHandle AddScreenshotToLibrary(const char*, const char*, int, int) override { static uint32 s_ss = 1; return s_ss++; }
    virtual void TriggerScreenshot() override {}
    virtual void HookScreenshots(bool) override {}
    virtual bool SetLocation(ScreenshotHandle, const char*) override { return true; }
    virtual bool TagUser(ScreenshotHandle, CSteamID) override { return true; }
    virtual bool TagPublishedFile(ScreenshotHandle, PublishedFileId_t) override { return true; }
    virtual bool IsScreenshotsHooked() override { return true; }
    virtual ScreenshotHandle AddVRScreenshotToLibrary(EVRScreenshotType, const char*, const char*) override { static uint32 s_ss = 1; return s_ss++; }
};

class CSteamAppListStub : public ISteamAppList
{
public:
    virtual uint32 GetNumInstalledApps() override { return 1; }
    virtual uint32 GetInstalledApps(AppId_t *pvecAppID, uint32 unMaxAppIDs) override { if (unMaxAppIDs > 0 && pvecAppID) pvecAppID[0] = g_ForcedAppId; return unMaxAppIDs > 0 ? 1 : 0; }
    virtual int GetAppName(AppId_t, char*, int) override { return -1; }
    virtual int GetAppInstallDir(AppId_t, char*, int) override { return -1; }
    virtual int GetAppBuildId(AppId_t) override { return 0; }
};

class CSteamGameServerStub : public ISteamGameServer
{
public:
    CSteamGameServerStub() : m_bLoggedOn(false) {}

    virtual void SetProduct(const char*) override {}
    virtual void SetGameDescription(const char*) override {}
    virtual void SetModDir(const char*) override {}
    virtual void SetDedicatedServer(bool) override {}
    virtual void LogOn(const char*) override { m_bLoggedOn = true; }
    virtual void LogOnAnonymous() override {
        UCOLOG("[UCOffline] GS LogOnAnonymous\r\n");
        m_bLoggedOn = true;
        CCallbackDispatcher* pDisp = GetDispatcher();
        if (pDisp) {
            SteamServersConnected_t connected;
            pDisp->PostCallback(g_ServerUser, connected.k_iCallback, &connected, sizeof(connected));
        }
    }
    virtual void LogOff() override { m_bLoggedOn = false; }
    virtual bool BLoggedOn() override { return m_bLoggedOn; }
    virtual bool BSecure() override { return false; }
    virtual CSteamID GetSteamID() override { return CSteamID(g_ForcedAppId, k_EUniversePublic, k_EAccountTypeGameServer); }
    virtual bool WasRestartRequested() override { return false; }
    virtual void SetMaxPlayerCount(int) override {}
    virtual void SetBotPlayerCount(int) override {}
    virtual void SetServerName(const char*) override {}
    virtual void SetMapName(const char*) override {}
    virtual void SetPasswordProtected(bool) override {}
    virtual void SetSpectatorPort(uint16) override {}
    virtual void SetSpectatorServerName(const char*) override {}
    virtual void ClearAllKeyValues() override {}
    virtual void SetKeyValue(const char*, const char*) override {}
    virtual void SetGameTags(const char*) override {}
    virtual void SetGameData(const char*) override {}
    virtual void SetRegion(const char*) override {}
    virtual void SetAdvertiseServerActive(bool) override {}
    virtual HAuthTicket GetAuthSessionTicket(void*, int, uint32*, const SteamNetworkingIdentity*) override { return 0x00001234; }
    virtual EBeginAuthSessionResult BeginAuthSession(const void* pAuthBlob, int cbAuthBlob, CSteamID steamID) override {
        UCOLOG("[UCOffline] GS BeginAuthSession from %llu\r\n", steamID.ConvertToUint64());
        CCallbackDispatcher* pDisp = GetDispatcher();
        if (pDisp) {
            // Like GBE Fork: fire ValidateAuthTicketResponse_t to server (client-side callback)
            ValidateAuthTicketResponse_t resp{};
            resp.m_SteamID = steamID;
            resp.m_eAuthSessionResponse = k_EAuthSessionResponseOK;
            resp.m_OwnerSteamID = steamID;
            pDisp->PostCallback(g_ServerUser, resp.k_iCallback, &resp, sizeof(resp));

            // Also fire GSClientApprove_t to server
            GSClientApprove_t approve{};
            approve.m_SteamID = steamID;
            approve.m_OwnerSteamID = steamID;
            pDisp->PostCallback(g_ServerUser, approve.k_iCallback, &approve, sizeof(approve));
        }
        return k_EBeginAuthSessionResultOK;
    }
    virtual void EndAuthSession(CSteamID) override {}
    virtual void CancelAuthTicket(HAuthTicket) override {}
    virtual EUserHasLicenseForAppResult UserHasLicenseForApp(CSteamID, AppId_t) override { return k_EUserHasLicenseResultHasLicense; }
    virtual bool RequestUserGroupStatus(CSteamID steamIDUser, CSteamID steamIDGroup) override {
        UCOLOG("[UCOffline] RequestUserGroupStatus: user=%llu group=%llu", steamIDUser.ConvertToUint64(), steamIDGroup.ConvertToUint64());
        return false;
    }
    virtual void GetGameplayStats() override {}
    virtual SteamAPICall_t GetServerReputation() override { return k_uAPICallInvalid; }
    virtual SteamIPAddress_t GetPublicIP() override { SteamIPAddress_t addr = {}; addr.m_eType = k_ESteamIPTypeIPv4; addr.m_unIPv4 = 0; return addr; }
    virtual bool HandleIncomingPacket(const void*, int, uint32, uint16) override { return false; }
    virtual int GetNextOutgoingPacket(void*, int, uint32*, uint16*) override { return 0; }
    virtual SteamAPICall_t AssociateWithClan(CSteamID) override { return k_uAPICallInvalid; }
    virtual SteamAPICall_t ComputeNewPlayerCompatibility(CSteamID) override { return k_uAPICallInvalid; }
    virtual bool SendUserConnectAndAuthenticate_DEPRECATED(uint32 unIP, const void* pvAuthBlob, uint32 cubAuthBlob, CSteamID* pSteamIDUser) override {
        UCOLOG("[UCOffline] GS SendUserConnectAndAuthenticate_DEPRECATED from IP %u\r\n", unIP);
        CSteamID clientID(unIP, k_EUniversePublic, k_EAccountTypeIndividual);
        if (pSteamIDUser) *pSteamIDUser = clientID;

        CCallbackDispatcher* pDisp = GetDispatcher();
        if (pDisp) {
            GSClientApprove_t approve{};
            approve.m_SteamID = clientID;
            approve.m_OwnerSteamID = clientID;
            pDisp->PostCallback(g_ServerUser, approve.k_iCallback, &approve, sizeof(approve));
        }
        return true;
    }
    virtual CSteamID CreateUnauthenticatedUserConnection() override { return CSteamID(2, k_EUniversePublic, k_EAccountTypeIndividual); }
    virtual void SendUserDisconnect_DEPRECATED(CSteamID steamID) override {
        UCOLOG("[UCOffline] SendUserDisconnect_DEPRECATED: %llu", steamID.ConvertToUint64());
    }
    virtual bool BUpdateUserData(CSteamID, const char*, uint32) override { return false; }
    virtual void SetMasterServerHeartbeatInterval_DEPRECATED(int) override {}
    virtual void ForceMasterServerHeartbeat_DEPRECATED() override {}
    virtual bool InitGameServer(uint32, uint16, uint16, uint32, AppId_t, const char*) override { m_bLoggedOn = true; return true; }

private:
    bool m_bLoggedOn;
};

class CSteamGameServerStatsStub : public ISteamGameServerStats
{
public:
    virtual SteamAPICall_t RequestUserStats(CSteamID) override { return k_uAPICallInvalid; }
    virtual bool GetUserStat(CSteamID, const char*, int32*) override { return false; }
    virtual bool GetUserStat(CSteamID, const char*, float*) override { return false; }
    virtual bool GetUserAchievement(CSteamID, const char*, bool*) override { return false; }
    virtual bool SetUserStat(CSteamID, const char*, int32) override { return false; }
    virtual bool SetUserStat(CSteamID, const char*, float) override { return false; }
    virtual bool UpdateUserAvgRateStat(CSteamID, const char*, float, double) override { return false; }
    virtual bool SetUserAchievement(CSteamID, const char*) override { return false; }
    virtual bool ClearUserAchievement(CSteamID, const char*) override { return false; }
    virtual SteamAPICall_t StoreUserStats(CSteamID) override { return k_uAPICallInvalid; }
};

class CSteamClientStub : public ISteamClient
{
public:
    virtual HSteamPipe CreateSteamPipe() override { return ++s_NextPipe; }
    virtual bool BReleaseSteamPipe(HSteamPipe) override { return true; }
    virtual HSteamUser ConnectToGlobalUser(HSteamPipe hSteamPipe) override { return hSteamPipe ? ++s_NextUser : 0; }
    virtual HSteamUser CreateLocalUser(HSteamPipe *phSteamPipe, EAccountType) override { if (phSteamPipe) *phSteamPipe = CreateSteamPipe(); return ++s_NextUser; }
    virtual void ReleaseUser(HSteamPipe, HSteamUser) override {}
    virtual ISteamUser *GetISteamUser(HSteamUser, HSteamPipe, const char*) override { return &s_UserStub; }
    virtual ISteamGameServer *GetISteamGameServer(HSteamUser, HSteamPipe, const char*) override { return &s_GameServerStub; }
    virtual void SetLocalIPBinding(const SteamIPAddress_t&, uint16) override {}
    virtual ISteamFriends *GetISteamFriends(HSteamUser, HSteamPipe, const char*) override { return &s_FriendsStub; }
    virtual ISteamUtils *GetISteamUtils(HSteamPipe, const char*) override { return &s_UtilsStub; }
    virtual ISteamMatchmaking *GetISteamMatchmaking(HSteamUser, HSteamPipe, const char*) override { return &s_MatchmakingStub; }
    virtual ISteamMatchmakingServers *GetISteamMatchmakingServers(HSteamUser, HSteamPipe, const char*) override { return &s_MatchmakingServersStub; }
    virtual void *GetISteamGenericInterface(HSteamUser, HSteamPipe, const char *pchVersion) override { return ResolveUserInterface(pchVersion); }
    virtual ISteamUserStats *GetISteamUserStats(HSteamUser, HSteamPipe, const char*) override { return &s_UserStatsStub; }
    virtual ISteamGameServerStats *GetISteamGameServerStats(HSteamUser, HSteamPipe, const char*) override { return &s_GameServerStatsStub; }
    virtual ISteamApps *GetISteamApps(HSteamUser, HSteamPipe, const char*) override { return &s_AppsStub; }
    virtual ISteamNetworking *GetISteamNetworking(HSteamUser, HSteamPipe, const char*) override { return &s_NetworkingStub; }
    virtual ISteamRemoteStorage *GetISteamRemoteStorage(HSteamUser, HSteamPipe, const char*) override { return &s_RemoteStorageStub; }
    virtual ISteamScreenshots *GetISteamScreenshots(HSteamUser, HSteamPipe, const char*) override { return &s_ScreenshotsStub; }
    virtual void RunFrame() override {}
    virtual uint32 GetIPCCallCount() override { return 0; }
    virtual void SetWarningMessageHook(SteamAPIWarningMessageHook_t) override {}
    virtual bool BShutdownIfAllPipesClosed() override { return true; }
    virtual ISteamHTTP *GetISteamHTTP(HSteamUser, HSteamPipe, const char*) override { return &s_HTTPStub; }
    virtual ISteamController *GetISteamController(HSteamUser, HSteamPipe, const char*) override { return &s_ControllerStub; }
    virtual ISteamUGC *GetISteamUGC(HSteamUser, HSteamPipe, const char*) override { return &s_UGCStub; }
    virtual ISteamMusic *GetISteamMusic(HSteamUser, HSteamPipe, const char*) override { return &s_MusicStub; }
    virtual ISteamHTMLSurface *GetISteamHTMLSurface(HSteamUser, HSteamPipe, const char*) override { return &s_HTMLSurfaceStub; }
    virtual void DEPRECATED_Set_SteamAPI_CPostAPIResultInProcess(void (*)()) override {}
    virtual void DEPRECATED_Remove_SteamAPI_CPostAPIResultInProcess(void (*)()) override {}
    virtual void Set_SteamAPI_CCheckCallbackRegisteredInProcess(SteamAPI_CheckCallbackRegistered_t) override {}
    virtual ISteamInventory *GetISteamInventory(HSteamUser, HSteamPipe, const char*) override { return &s_InventoryStub; }
    virtual ISteamVideo *GetISteamVideo(HSteamUser, HSteamPipe, const char*) override { return &s_VideoStub; }
    virtual ISteamParentalSettings *GetISteamParentalSettings(HSteamUser, HSteamPipe, const char*) override { return &s_ParentalSettingsStub; }
    virtual ISteamInput *GetISteamInput(HSteamUser, HSteamPipe, const char*) override { return &s_InputStub; }
    virtual ISteamParties *GetISteamParties(HSteamUser, HSteamPipe, const char*) override { return &s_PartiesStub; }
    virtual ISteamRemotePlay *GetISteamRemotePlay(HSteamUser, HSteamPipe, const char*) override { return &s_RemotePlayStub; }
    virtual void DestroyAllInterfaces() override {}

    static CSteamClientStub& Instance()
    {
        return s_ClientStub;
    }

    static void *ResolveUserInterface(const char *pchVersion)
    {
        if (!pchVersion) return nullptr;

        TrackInterfaceRequest(pchVersion);

        // Exact match first (fast path)
        if (strcmp(pchVersion, STEAMCLIENT_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_ClientStub; }
        if (strcmp(pchVersion, STEAMUSER_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_UserStub; }
        if (strcmp(pchVersion, STEAMUTILS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_UtilsStub; }
        if (strcmp(pchVersion, STEAMFRIENDS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_FriendsStub; }
        if (strcmp(pchVersion, STEAMMATCHMAKING_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_MatchmakingStub; }
        if (strcmp(pchVersion, STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_MatchmakingServersStub; }
        if (strcmp(pchVersion, STEAMUSERSTATS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_UserStatsStub; }
        if (strcmp(pchVersion, STEAMAPPS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_AppsStub; }
        if (strcmp(pchVersion, STEAMNETWORKING_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingStub; }
        if (strcmp(pchVersion, STEAMREMOTESTORAGE_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_RemoteStorageStub; }
        if (strcmp(pchVersion, STEAMSCREENSHOTS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_ScreenshotsStub; }
        if (strcmp(pchVersion, STEAMHTTP_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_HTTPStub; }
        if (strcmp(pchVersion, STEAMCONTROLLER_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_ControllerStub; }
        if (strcmp(pchVersion, STEAMUGC_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_UGCStub; }
        if (strcmp(pchVersion, STEAMMUSIC_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_MusicStub; }
        if (strcmp(pchVersion, STEAMGAMESEARCH_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_GameSearchStub; }
        if (strcmp(pchVersion, STEAMPARTIES_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_PartiesStub; }
        if (strcmp(pchVersion, STEAMREMOTEPLAY_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_RemotePlayStub; }
        if (strcmp(pchVersion, STEAMTIMELINE_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_TimelineStub; }
        if (strcmp(pchVersion, STEAMINVENTORY_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_InventoryStub; }
        if (strcmp(pchVersion, STEAMHTMLSURFACE_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_HTMLSurfaceStub; }
        if (strcmp(pchVersion, STEAMVIDEO_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_VideoStub; }
        if (strcmp(pchVersion, STEAMPARENTALSETTINGS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_ParentalSettingsStub; }
        if (strcmp(pchVersion, STEAMINPUT_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_InputStub; }
        if (strcmp(pchVersion, STEAMAPPLIST_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_AppListStub; }
        if (strcmp(pchVersion, STEAMNETWORKINGUTILS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingUtilsStub; }

        // Prefix match for interfaces with many version numbers
        if (strstr(pchVersion, "SteamNetworkingMessages") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingMessagesStub; }
        if (strstr(pchVersion, "SteamNetworkingSockets") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingSocketsStub; }

        // Prefix match fallback: if exact version doesn't match, try prefix match
        // This handles cases where the game uses a slightly different SDK version
        if (strstr(pchVersion, "SteamUser") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_UserStub; }
        if (strstr(pchVersion, "SteamUtils") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_UtilsStub; }
        if (strstr(pchVersion, "SteamFriends") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_FriendsStub; }
        if (strstr(pchVersion, "SteamMatchmaking") == pchVersion && strstr(pchVersion, "Servers") == nullptr) { MarkInterfaceImplemented(pchVersion); return &s_MatchmakingStub; }
        if (strstr(pchVersion, "SteamMatchmakingServers") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_MatchmakingServersStub; }
        if (strstr(pchVersion, "SteamUserStats") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_UserStatsStub; }
        if (strstr(pchVersion, "SteamApps") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_AppsStub; }
        if (strstr(pchVersion, "SteamNetworking") == pchVersion && strstr(pchVersion, "Messages") == nullptr && strstr(pchVersion, "Sockets") == nullptr && strstr(pchVersion, "Utils") == nullptr) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingStub; }
        if (strstr(pchVersion, "SteamRemoteStorage") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_RemoteStorageStub; }
        if (strstr(pchVersion, "SteamScreenshots") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_ScreenshotsStub; }
        if (strstr(pchVersion, "SteamHTTP") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_HTTPStub; }
        if (strstr(pchVersion, "SteamController") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_ControllerStub; }
        if (strstr(pchVersion, "SteamUGC") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_UGCStub; }
        if (strstr(pchVersion, "SteamMusic") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_MusicStub; }
        if (strstr(pchVersion, "SteamParties") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_PartiesStub; }
        if (strstr(pchVersion, "SteamRemotePlay") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_RemotePlayStub; }
        if (strstr(pchVersion, "SteamTimeline") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_TimelineStub; }
        if (strstr(pchVersion, "SteamInventory") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_InventoryStub; }
        if (strstr(pchVersion, "SteamHTMLSurface") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_HTMLSurfaceStub; }
        if (strstr(pchVersion, "SteamVideo") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_VideoStub; }
        if (strstr(pchVersion, "SteamParentalSettings") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_ParentalSettingsStub; }
        if (strstr(pchVersion, "SteamInput") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_InputStub; }
        if (strstr(pchVersion, "SteamAppList") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_AppListStub; }
        if (strstr(pchVersion, "SteamGameSearch") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_GameSearchStub; }
        if (strstr(pchVersion, "SteamGameServerStats") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_GameServerStatsStub; }
        if (strstr(pchVersion, "SteamGameServer") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_GameServerStub; }
        if (strstr(pchVersion, "SteamNetworkingUtils") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingUtilsStub; }
        if (strstr(pchVersion, "SteamClient") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_ClientStub; }

        // STEAM_ all-caps prefix fallbacks (some interfaces use all-caps naming)
        if (strstr(pchVersion, "STEAMNETWORKINGMESSAGES") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingMessagesStub; }
        if (strstr(pchVersion, "STEAMNETWORKINGSOCKETS") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingSocketsStub; }
        if (strstr(pchVersion, "STEAMNETWORKINGUTILS") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingUtilsStub; }
        if (strstr(pchVersion, "STEAMNETWORKING") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingStub; }
        if (strstr(pchVersion, "STEAMUSERSTATS") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_UserStatsStub; }
        if (strstr(pchVersion, "STEAMREMOTEPLAY") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_RemotePlayStub; }
        if (strstr(pchVersion, "STEAMREMOTESTORAGE") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_RemoteStorageStub; }
        if (strstr(pchVersion, "STEAMSCREENSHOTS") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_ScreenshotsStub; }
        if (strstr(pchVersion, "STEAMTIMELINE") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_TimelineStub; }
        if (strstr(pchVersion, "STEAMUGC") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_UGCStub; }
        if (strstr(pchVersion, "STEAMAPPS") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_AppsStub; }
        if (strstr(pchVersion, "STEAMHTTP") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_HTTPStub; }
        if (strstr(pchVersion, "STEAMCONTROLLER") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_ControllerStub; }
        if (strstr(pchVersion, "STEAMAPPLIST") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_AppListStub; }
        if (strstr(pchVersion, "STEAMMUSICREMOTE") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_MusicStub; }
        if (strstr(pchVersion, "STEAMMUSIC") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_MusicStub; }
        if (strstr(pchVersion, "STEAMPARENTALSETTINGS") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_ParentalSettingsStub; }
        if (strstr(pchVersion, "STEAMINPUT") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_InputStub; }
        if (strstr(pchVersion, "STEAMVIDEO") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_VideoStub; }
        if (strstr(pchVersion, "STEAMHTMLSURFACE") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_HTMLSurfaceStub; }
        if (strstr(pchVersion, "STEAMINVENTORY") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_InventoryStub; }
        if (strstr(pchVersion, "STEAMPARTIES") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_PartiesStub; }
        if (strstr(pchVersion, "STEAMGAMESEARCH") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_GameSearchStub; }

        UCOLOG("[UCOffline] ResolveUserInterface: unknown interface '%s'\r\n", pchVersion);
        return nullptr;
    }

    static void *ResolveGameServerInterface(const char *pchVersion)
    {
        if (!pchVersion) return nullptr;
        TrackInterfaceRequest(pchVersion);
        if (strcmp(pchVersion, STEAMCLIENT_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_ClientStub; }
        if (strcmp(pchVersion, STEAMGAMESERVER_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_GameServerStub; }
        if (strcmp(pchVersion, STEAMGAMESERVERSTATS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_GameServerStatsStub; }
        if (strcmp(pchVersion, STEAMUTILS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_UtilsStub; }
        if (strcmp(pchVersion, STEAMHTTP_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_HTTPStub; }
        if (strcmp(pchVersion, STEAMINVENTORY_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_InventoryStub; }
        if (strcmp(pchVersion, STEAMUGC_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_UGCStub; }
        if (strcmp(pchVersion, STEAMAPPS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_AppsStub; }
        if (strcmp(pchVersion, STEAMNETWORKING_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingStub; }
        if (strstr(pchVersion, "SteamNetworkingMessages") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingMessagesStub; }
        if (strstr(pchVersion, "SteamNetworkingSockets") == pchVersion) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingSocketsStub; }
        if (strcmp(pchVersion, STEAMNETWORKINGUTILS_INTERFACE_VERSION) == 0) { MarkInterfaceImplemented(pchVersion); return &s_NetworkingUtilsStub; }
        return ResolveUserInterface(pchVersion);
    }

private:
    static uint32 s_NextPipe;
    static uint32 s_NextUser;
    static CSteamUtilsStub s_UtilsStub;
    static CSteamFriendsStub s_FriendsStub;
    static CSteamMatchmakingStub s_MatchmakingStub;
    static CSteamMatchmakingServersStub s_MatchmakingServersStub;
    static CSteamUserStatsStub s_UserStatsStub;
    static CSteamRemoteStorageStub s_RemoteStorageStub;
    static CSteamHTTPStub s_HTTPStub;
    static CSteamInventoryStub s_InventoryStub;
    static CSteamUGCStub s_UGCStub;
    static CSteamNetworkingStub s_NetworkingStub;
    static CSteamNetworkingMessagesStub s_NetworkingMessagesStub;
    static CSteamNetworkingSocketsStub s_NetworkingSocketsStub;
    static CSteamNetworkingUtilsStub s_NetworkingUtilsStub;
    static CSteamInputStub s_InputStub;
    static CSteamControllerStub s_ControllerStub;
    static CSteamHTMLSurfaceStub s_HTMLSurfaceStub;
    static CSteamRemotePlayStub s_RemotePlayStub;
    static CSteamTimelineStub s_TimelineStub;
    static CSteamVideoStub s_VideoStub;
    static CSteamParentalSettingsStub s_ParentalSettingsStub;
    static CSteamMusicStub s_MusicStub;
    static CSteamPartiesStub s_PartiesStub;
    static CSteamScreenshotsStub s_ScreenshotsStub;
    static CSteamAppListStub s_AppListStub;
    static CSteamGameServerStub s_GameServerStub;
    static CSteamGameServerStatsStub s_GameServerStatsStub;
    static CSteamClientStub s_ClientStub;
};

uint32 CSteamClientStub::s_NextPipe = 1000;
uint32 CSteamClientStub::s_NextUser = 2000;
CSteamUtilsStub CSteamClientStub::s_UtilsStub;
CSteamFriendsStub CSteamClientStub::s_FriendsStub;
CSteamMatchmakingStub CSteamClientStub::s_MatchmakingStub;
CSteamMatchmakingServersStub CSteamClientStub::s_MatchmakingServersStub;
CSteamUserStatsStub CSteamClientStub::s_UserStatsStub;
CSteamRemoteStorageStub CSteamClientStub::s_RemoteStorageStub;
CSteamHTTPStub CSteamClientStub::s_HTTPStub;
CSteamInventoryStub CSteamClientStub::s_InventoryStub;
CSteamUGCStub CSteamClientStub::s_UGCStub;
CSteamNetworkingStub CSteamClientStub::s_NetworkingStub;
CSteamNetworkingMessagesStub CSteamClientStub::s_NetworkingMessagesStub;
CSteamNetworkingSocketsStub CSteamClientStub::s_NetworkingSocketsStub;
CSteamNetworkingUtilsStub CSteamClientStub::s_NetworkingUtilsStub;
CSteamInputStub CSteamClientStub::s_InputStub;
CSteamControllerStub CSteamClientStub::s_ControllerStub;
CSteamHTMLSurfaceStub CSteamClientStub::s_HTMLSurfaceStub;
CSteamRemotePlayStub CSteamClientStub::s_RemotePlayStub;
CSteamTimelineStub CSteamClientStub::s_TimelineStub;
CSteamVideoStub CSteamClientStub::s_VideoStub;
CSteamParentalSettingsStub CSteamClientStub::s_ParentalSettingsStub;
CSteamMusicStub CSteamClientStub::s_MusicStub;
CSteamPartiesStub CSteamClientStub::s_PartiesStub;
CSteamScreenshotsStub CSteamClientStub::s_ScreenshotsStub;
CSteamAppListStub CSteamClientStub::s_AppListStub;
CSteamGameServerStub CSteamClientStub::s_GameServerStub;
CSteamGameServerStatsStub CSteamClientStub::s_GameServerStatsStub;
CSteamClientStub CSteamClientStub::s_ClientStub;

static CSteamClientStub& LocalClientStub()
{
    return CSteamClientStub::Instance();
}

class CSteamGameServerAPIContext
{
public:
	ISteamClient* m_pSteamClient;
	ISteamGameServer* m_pSteamGameServer;
	ISteamUtils* m_pSteamUtils;
	ISteamNetworking* m_pSteamNetworking;
	ISteamGameServerStats* m_pSteamGameServerStats;
	ISteamHTTP* m_pSteamHTTP;
	ISteamInventory* m_pSteamInventory;
	ISteamUGC* m_pSteamUGC;
	ISteamApps* m_pSteamApps;

	bool Init()
	{
		HSteamUser hUser = SteamGameServer_GetHSteamUser();
		HSteamPipe hPipe = SteamGameServer_GetHSteamPipe();
		if (hPipe == 0) return false;

		m_pSteamClient = (ISteamClient*)SteamInternal_FindOrCreateGameServerInterface(hUser, STEAMCLIENT_INTERFACE_VERSION);
		m_pSteamGameServer = (ISteamGameServer*)SteamInternal_FindOrCreateGameServerInterface(hUser, STEAMGAMESERVER_INTERFACE_VERSION);
		m_pSteamUtils = (ISteamUtils*)SteamInternal_FindOrCreateGameServerInterface(hUser, STEAMUTILS_INTERFACE_VERSION);
		m_pSteamNetworking = (ISteamNetworking*)SteamInternal_FindOrCreateGameServerInterface(hUser, STEAMNETWORKING_INTERFACE_VERSION);
		m_pSteamGameServerStats = (ISteamGameServerStats*)SteamInternal_FindOrCreateGameServerInterface(hUser, STEAMGAMESERVERSTATS_INTERFACE_VERSION);
		m_pSteamHTTP = (ISteamHTTP*)SteamInternal_FindOrCreateGameServerInterface(hUser, STEAMHTTP_INTERFACE_VERSION);
		m_pSteamInventory = (ISteamInventory*)SteamInternal_FindOrCreateGameServerInterface(hUser, STEAMINVENTORY_INTERFACE_VERSION);
		m_pSteamUGC = (ISteamUGC*)SteamInternal_FindOrCreateGameServerInterface(hUser, STEAMUGC_INTERFACE_VERSION);
		m_pSteamApps = (ISteamApps*)SteamInternal_FindOrCreateGameServerInterface(hUser, STEAMAPPS_INTERFACE_VERSION);

		return m_pSteamGameServer != nullptr && m_pSteamUtils != nullptr;
	}

	void Clear()
	{
		m_pSteamClient = nullptr; m_pSteamGameServer = nullptr; m_pSteamUtils = nullptr;
		m_pSteamNetworking = nullptr; m_pSteamGameServerStats = nullptr;
		m_pSteamHTTP = nullptr; m_pSteamInventory = nullptr; m_pSteamUGC = nullptr; m_pSteamApps = nullptr;
	}

	ISteamClient* SteamClient() { return m_pSteamClient; }
	ISteamGameServer* SteamGameServer() { return m_pSteamGameServer; }
	ISteamUtils* SteamGameServerUtils() { return m_pSteamUtils; }
	ISteamNetworking* SteamGameServerNetworking() { return m_pSteamNetworking; }
	ISteamGameServerStats* SteamGameServerStats() { return m_pSteamGameServerStats; }
	ISteamHTTP* SteamHTTP() { return m_pSteamHTTP; }
	ISteamInventory* SteamInventory() { return m_pSteamInventory; }
	ISteamUGC* SteamUGC() { return m_pSteamUGC; }
	ISteamApps* SteamApps() { return m_pSteamApps; }
};

// ============================================================

extern HMODULE g_ClientModule;
extern HSteamPipe g_ClientPipe;
extern HSteamUser g_ClientUser;
extern ISteamClient* g_pSteamClient;
extern ISteamClient* g_pSteamClientSafe;
extern ISteamUtils* g_pUtilsForCallbacks;
extern ISteamController* g_pControllerForCallbacks;
extern ISteamInput* g_pInputForCallbacks;
extern bool g_bClientReady;
extern SRWLOCK g_CtxLock;

extern HMODULE g_ServerModule;
extern HSteamPipe g_ServerPipe;
extern HSteamUser g_ServerUser;
extern ISteamClient* g_ServerClient;
extern ISteamClient* g_pServerClient;
extern ISteamClient* g_pSteamClientGameServer_Latest;
extern ISteamGameServer* g_pGameServer;
extern ISteamUtils* g_pServerUtils;
extern CSteamGameServerAPIContext g_ServerCtx;
extern bool g_bServerReady;
extern EServerMode g_ServerMode;

extern bool g_bUsingBreakpad;
extern bool g_bFullDumps;
extern void* g_BreakpadCtx;
extern PFNPreMinidumpCallback g_BreakpadCallback;
extern char g_BreakpadVer[64];
extern char g_BreakpadTimestamp[64];
extern uint32 g_BreakpadAppID;
extern uint64 g_BreakpadSteamID;

extern bool g_bTryCatch;
extern int g_DispatchMode;
extern char g_InstallPath[MAX_PATH];
extern bool g_bHaveInstallPath;
extern SRWLOCK g_CallbackLock;
extern uint32 g_ForcedAppId;

typedef void* (S_CALLTYPE* Fn_CreateInterface)(const char* pName, int* pReturnCode);
extern Fn_CreateInterface g_pfnCreateInterface;

typedef void (S_CALLTYPE* Fn_ReleaseThreadLocal)(int bThreadExit);
extern Fn_ReleaseThreadLocal g_pfnReleaseThreadLocal;

typedef bool (S_CALLTYPE* Fn_IsKnownInterface)(const char* Interface);
extern Fn_IsKnownInterface g_pfnIsKnownInterface;

typedef void (S_CALLTYPE* Fn_NotifyMissing)(HSteamPipe hPipe, const char* Interface);
extern Fn_NotifyMissing g_pfnNotifyMissing;

typedef void (S_CALLTYPE* Fn_BreakpadInit)(uint32 AppID, const char* ver, const char* date, bool bFull, void* ctx, PFNPreMinidumpCallback cb);
extern Fn_BreakpadInit g_pfnBreakpadInit;

typedef void (S_CALLTYPE* Fn_BreakpadSetAppID)(uint32 AppID);
extern Fn_BreakpadSetAppID g_pfnBreakpadSetAppID;

typedef void (S_CALLTYPE* Fn_BreakpadSetSteamID)(uint64 SteamID);
extern Fn_BreakpadSetSteamID g_pfnBreakpadSetSteamID;

typedef void (S_CALLTYPE* Fn_BreakpadSetComment)(const char* Comment);
extern Fn_BreakpadSetComment g_pfnBreakpadSetComment;

typedef void (S_CALLTYPE* Fn_BreakpadWriteDump)(uint32 Code, void* Info, uint32 BuildID);
extern Fn_BreakpadWriteDump g_pfnBreakpadWriteDump;

extern uintp g_CtxCounter;

void UCOLOG(const char* fmt, ...);
void UCOColor(WORD color, const char* text);
void* InitSteamClient(HMODULE* phModule, bool bLocal, const char* iface);
void LoadBreakpadSymbols(HMODULE hMod);
void UpdateMinidumpSteamID(uint64 sid);

void DumpTicketBytes(const char* source, const void* ticket, uint32 ticketSize)
{
    if (!source) source = "ticket";
    if (!ticket || ticketSize == 0)
    {
        UCOLOG("[UCOffline] %s ticket dump skipped: empty ticket", source);
        return;
    }

    char dir[MAX_PATH] = { 0 };
    char binPath[MAX_PATH] = { 0 };
    char hexPath[MAX_PATH] = { 0 };
    const uint8_t* bytes = static_cast<const uint8_t*>(ticket);

    if (GetModuleFileNameA(nullptr, dir, MAX_PATH) != 0)
    {
        char* slash = strrchr(dir, '\\');
        if (slash) *slash = '\0';

        _snprintf_s(binPath, sizeof(binPath), _TRUNCATE, "%s\\uc_offline_%s_ticket.bin", dir, source);
        _snprintf_s(hexPath, sizeof(hexPath), _TRUNCATE, "%s\\uc_offline_%s_ticket.hex.txt", dir, source);
    }
    else
    {
        _snprintf_s(binPath, sizeof(binPath), _TRUNCATE, "uc_offline_%s_ticket.bin", source);
        _snprintf_s(hexPath, sizeof(hexPath), _TRUNCATE, "uc_offline_%s_ticket.hex.txt", source);
    }

    FILE* f = nullptr;
    if (fopen_s(&f, binPath, "wb") == 0 && f)
    {
        fwrite(bytes, 1, ticketSize, f);
        fclose(f);
    }
    else
    {
        UCOLOG("[UCOffline] Failed to write %s ticket file: %s", source, binPath);
    }

    UCOLOG("[UCOffline] %s ticket dumped to %s (%u bytes)", source, binPath, ticketSize);
    UCOLOG("[UCOffline] %s ticket hex written to %s", source, hexPath);

    if (fopen_s(&f, hexPath, "wt") == 0 && f)
    {
        static const char hex[] = "0123456789ABCDEF";
        char line[80] = { 0 };
        const uint32 maxDump = ticketSize < 256 ? ticketSize : 256;

        for (uint32 offset = 0; offset < maxDump; offset += 16)
        {
            uint32 lineLen = 0;
            for (uint32 i = 0; i < 16 && offset + i < maxDump; ++i)
            {
                uint8_t value = bytes[offset + i];
                line[lineLen++] = hex[value >> 4];
                line[lineLen++] = hex[value & 0x0F];
                if (i != 15) line[lineLen++] = ' ';
            }
            line[lineLen] = '\0';
            fprintf(f, "%04u: %s\n", offset, line);
        }
        if (ticketSize > maxDump)
            fprintf(f, "... truncated at %u of %u bytes\n", maxDump, ticketSize);

        fclose(f);

        for (uint32 offset = 0; offset < maxDump; offset += 16)
        {
            uint32 lineLen = 0;
            for (uint32 i = 0; i < 16 && offset + i < maxDump; ++i)
            {
                uint8_t value = bytes[offset + i];
                line[lineLen++] = hex[value >> 4];
                line[lineLen++] = hex[value & 0x0F];
                if (i != 15) line[lineLen++] = ' ';
            }
            line[lineLen] = '\0';
            UCOLOG("[UCOffline] %s[%04u] %s", source, offset, line);
        }
        if (ticketSize > maxDump)
            UCOLOG("[UCOffline] %s hex truncated at %u of %u bytes", source, maxDump, ticketSize);
    }
    else
    {
        UCOLOG("[UCOffline] Failed to write %s ticket hex file: %s", source, hexPath);
    }
}

