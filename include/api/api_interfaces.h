S_API ISteamApps* S_CALLTYPE SteamApps()
{
    return g_bClientReady ? &s_AppsStub : nullptr;  // Always return our stub
}

S_API ISteamClient* S_CALLTYPE SteamClient()
{
	UCOLOG("[UCOffline] SteamClient\r\n");
	return g_pSteamClientSafe;
}

S_API ISteamController* S_CALLTYPE SteamController()
{
	UCOLOG("[UCOffline] SteamController\r\n");
	return g_bClientReady ? g_ClientCtx.SteamController() : nullptr;
}

S_API ISteamFriends* S_CALLTYPE SteamFriends()
{
	UCOLOG("[UCOffline] SteamFriends\r\n");
	return g_bClientReady ? g_ClientCtx.SteamFriends() : nullptr;
}

S_API ISteamClient* S_CALLTYPE SteamGameServerSteamClient()
{
	UCOLOG("[UCOffline] SteamGameServerSteamClient\r\n");
	return g_bServerReady ? g_ServerCtx.SteamClient() : nullptr;
}

S_API ISteamGameServer* S_CALLTYPE SteamGameServer()
{
	UCOLOG("[UCOffline] SteamGameServer\r\n");
	return g_bServerReady ? g_ServerCtx.SteamGameServer() : nullptr;
}

S_API ISteamApps* S_CALLTYPE SteamGameServerApps()
{
	UCOLOG("[UCOffline] SteamGameServerApps\r\n");
	return g_bServerReady ? g_ServerCtx.SteamApps() : nullptr;
}

S_API ISteamHTTP* S_CALLTYPE SteamGameServerHTTP()
{
	UCOLOG("[UCOffline] SteamGameServerHTTP\r\n");
	return g_bServerReady ? g_ServerCtx.SteamHTTP() : nullptr;
}

S_API ISteamInventory* S_CALLTYPE SteamGameServerInventory()
{
	UCOLOG("[UCOffline] SteamGameServerInventory\r\n");
	return g_bServerReady ? g_ServerCtx.SteamInventory() : nullptr;
}

S_API ISteamNetworking* S_CALLTYPE SteamGameServerNetworking()
{
	UCOLOG("[UCOffline] SteamGameServerNetworking\r\n");
	return g_bServerReady ? g_ServerCtx.SteamGameServerNetworking() : nullptr;
}

S_API ISteamGameServerStats* S_CALLTYPE SteamGameServerStats()
{
	UCOLOG("[UCOffline] SteamGameServerStats\r\n");
	return g_bServerReady ? g_ServerCtx.SteamGameServerStats() : nullptr;
}

S_API ISteamUGC* S_CALLTYPE SteamGameServerUGC()
{
	UCOLOG("[UCOffline] SteamGameServerUGC\r\n");
	return g_bServerReady ? g_ServerCtx.SteamUGC() : nullptr;
}

S_API ISteamUtils* S_CALLTYPE SteamGameServerUtils()
{
	UCOLOG("[UCOffline] SteamGameServerUtils\r\n");
	return g_bServerReady ? g_ServerCtx.SteamGameServerUtils() : nullptr;
}

S_API ISteamGameSearch* S_CALLTYPE SteamGameSearch()
{
	UCOLOG("[UCOffline] SteamGameSearch\r\n");
	return g_bClientReady ? g_ClientCtx.SteamGameSearch() : nullptr;
}

S_API ISteamHTMLSurface* S_CALLTYPE SteamHTMLSurface()
{
	UCOLOG("[UCOffline] SteamHTMLSurface\r\n");
	return g_bClientReady ? g_ClientCtx.SteamHTMLSurface() : nullptr;
}

S_API ISteamHTTP* S_CALLTYPE SteamHTTP()
{
	UCOLOG("[UCOffline] SteamHTTP\r\n");
	return g_bClientReady ? g_ClientCtx.SteamHTTP() : nullptr;
}

S_API ISteamInput* S_CALLTYPE SteamInput()
{
	UCOLOG("[UCOffline] SteamInput\r\n");
	return g_bClientReady ? g_ClientCtx.SteamInput() : nullptr;
}

S_API ISteamInventory* S_CALLTYPE SteamInventory()
{
	UCOLOG("[UCOffline] SteamInventory\r\n");
	return g_bClientReady ? g_ClientCtx.SteamInventory() : nullptr;
}

S_API ISteamMatchmaking* S_CALLTYPE SteamMatchmaking()
{
	UCOLOG("[UCOffline] SteamMatchmaking\r\n");
	return g_bClientReady ? g_ClientCtx.SteamMatchmaking() : nullptr;
}

S_API ISteamMatchmakingServers* S_CALLTYPE SteamMatchmakingServers()
{
	UCOLOG("[UCOffline] SteamMatchmakingServers\r\n");
	return g_bClientReady ? g_ClientCtx.SteamMatchmakingServers() : nullptr;
}

S_API ISteamMusic* S_CALLTYPE SteamMusic()
{
	UCOLOG("[UCOffline] SteamMusic\r\n");
	return g_bClientReady ? g_ClientCtx.SteamMusic() : nullptr;
}

S_API ISteamMusicRemote* S_CALLTYPE SteamMusicRemote()
{
	UCOLOG("[UCOffline] SteamMusicRemote\r\n");
	return &s_MusicRemoteStub;
}

S_API ISteamNetworking* S_CALLTYPE SteamNetworking()
{
	UCOLOG("[UCOffline] SteamNetworking\r\n");
	return g_bClientReady ? g_ClientCtx.SteamNetworking() : nullptr;
}

S_API ISteamParentalSettings* S_CALLTYPE SteamParentalSettings()
{
	UCOLOG("[UCOffline] SteamParentalSettings\r\n");
	return g_bClientReady ? g_ClientCtx.SteamParentalSettings() : nullptr;
}

S_API ISteamParties* S_CALLTYPE SteamParties()
{
	UCOLOG("[UCOffline] SteamParties\r\n");
	return g_bClientReady ? g_ClientCtx.SteamParties() : nullptr;
}

S_API ISteamRemotePlay* S_CALLTYPE SteamRemotePlay()
{
	UCOLOG("[UCOffline] SteamRemotePlay\r\n");
	return g_bClientReady ? g_ClientCtx.SteamRemotePlay() : nullptr;
}

S_API ISteamRemoteStorage* S_CALLTYPE SteamRemoteStorage()
{
	UCOLOG("[UCOffline] SteamRemoteStorage\r\n");
	return g_bClientReady ? g_ClientCtx.SteamRemoteStorage() : nullptr;
}

S_API ISteamScreenshots* S_CALLTYPE SteamScreenshots()
{
	UCOLOG("[UCOffline] SteamScreenshots\r\n");
	return g_bClientReady ? g_ClientCtx.SteamScreenshots() : nullptr;
}

S_API ISteamUGC* S_CALLTYPE SteamUGC()
{
	UCOLOG("[UCOffline] SteamUGC\r\n");
	return g_bClientReady ? g_ClientCtx.SteamUGC() : nullptr;
}

S_API ISteamUser* S_CALLTYPE SteamUser()
{
	UCOLOG("[UCOffline] SteamUser\r\n");
	if (CSteamUserStub::IsEmulateTicketEnabled())
		return &s_UserStub;
	return g_bClientReady ? g_ClientCtx.SteamUser() : nullptr;
}

S_API ISteamUserStats* S_CALLTYPE SteamUserStats()
{
	UCOLOG("[UCOffline] SteamUserStats\r\n");
	return g_bClientReady ? g_ClientCtx.SteamUserStats() : nullptr;
}

S_API ISteamUtils* S_CALLTYPE SteamUtils()
{
	UCOLOG("[UCOffline] SteamUtils\r\n");
	return g_bClientReady ? g_ClientCtx.SteamUtils() : nullptr;
}

S_API ISteamVideo* S_CALLTYPE SteamVideo()
{
	UCOLOG("[UCOffline] SteamVideo\r\n");
	return g_bClientReady ? g_ClientCtx.SteamVideo() : nullptr;
}
