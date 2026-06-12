S_API void* S_CALLTYPE SteamInternal_CreateInterface(const char* ver)
{
	if (ver)
	{
		UCOLOG("[UCOffline] SteamInternal_CreateInterface -> %s\r\n", ver);
		void* pIface = LocalClientStub().ResolveUserInterface(ver);
		if (pIface)
			return pIface;
	}

	UCOColor(FOREGROUND_RED | FOREGROUND_INTENSITY, "[UCOffline] CreateInterface: returning null\r\n");
	return nullptr;
}

S_API void* S_CALLTYPE SteamGameServerInternal_CreateInterface(const char* iface)
{
	if (iface)
	{
		UCOLOG("[UCOffline] SteamGameServerInternal_CreateInterface -> %s\r\n", iface);
		void* pIface = LocalClientStub().ResolveGameServerInterface(iface);
		if (pIface)
			return pIface;
	}

	UCOColor(FOREGROUND_RED | FOREGROUND_INTENSITY, "[UCOffline] GameServerCreateInterface: returning null\r\n");
	return nullptr;
}

S_API void* S_CALLTYPE SteamInternal_FindOrCreateUserInterface(HSteamUser hUser, const char* ver)
{
	if (ver)
	{
		UCOLOG("[UCOffline] FindOrCreateUserInterface -> %s\r\n", ver);
		void* pIface = LocalClientStub().ResolveUserInterface(ver);
		if (pIface)
			return pIface;
		WarnMissingInterface(g_ClientPipe, ver);
	}

	UCOColor(FOREGROUND_RED | FOREGROUND_INTENSITY, "[UCOffline] FindOrCreateUserInterface: returning null\r\n");
	return nullptr;
}

S_API void* S_CALLTYPE SteamInternal_FindOrCreateGameServerInterface(HSteamUser hUser, const char* ver)
{
	if (ver)
	{
		UCOLOG("[UCOffline] FindOrCreateGameServerInterface -> %s\r\n", ver);
		void* pIface = LocalClientStub().ResolveGameServerInterface(ver);
		if (pIface)
			return pIface;
		WarnMissingInterface(g_ServerPipe, ver);
	}

	UCOColor(FOREGROUND_RED | FOREGROUND_INTENSITY, "[UCOffline] FindOrCreateGameServerInterface: returning null\r\n");
	return nullptr;
}
