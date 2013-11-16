// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: ISteamGameServer009 implementation
//
// Initial author: NTAuthority
// Started: 2010-09-28
// ==========================================================

#include "StdInc.h"
#include "SteamGameServer009.h"
#include <process.h>

#define CURL_STATICLIB
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

extern dvar_t* aiw_secure;

int failureCount = 0;

size_t DataReceived(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t rsize = (size * nmemb);
	char* text = (char*)ptr;
	CSteamID steamID = *(CSteamID*)data;

	if (strcmp(text, "invalid") == 0)
	{
		GSClientDeny_t* retvals = (GSClientDeny_t*)malloc(sizeof(GSClientDeny_t));

		retvals->m_SteamID = steamID;
		retvals->m_eDenyReason = k_EDenyCheater;

		CSteamBase::ReturnCall(retvals, sizeof(GSClientDeny_t), GSClientDeny_t::k_iCallback, 0);
	}
	else
	{
		GSClientApprove_t* retvals = (GSClientApprove_t*)malloc(sizeof(GSClientApprove_t));

		retvals->m_SteamID = steamID;

		CSteamBase::ReturnCall(retvals, sizeof(GSClientApprove_t), GSClientApprove_t::k_iCallback, 0);
	}

	return (rsize);
}

void ValidateConnection(void* state)
{
	CSteamID steamID = CSteamID(*(uint64*)state);

	if (aiw_secure && !aiw_secure->current.integer)
	{
		GSClientApprove_t* retvals = (GSClientApprove_t*)malloc(sizeof(GSClientApprove_t));
		retvals->m_SteamID = steamID;
		CSteamBase::ReturnCall(retvals, sizeof(GSClientApprove_t), GSClientApprove_t::k_iCallback, 0);
		return;
	}

	curl_global_init(CURL_GLOBAL_ALL);

	CURL* curl = curl_easy_init();

	if (curl)
	{
		char url[255];
		_snprintf(url, sizeof(url), "http://%s:13000/clean/%lld", "server.alteriw.net", steamID.ConvertToUint64());

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DataReceived);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&steamID);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, VERSIONSTRING);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

		CURLcode code = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		curl_global_cleanup();

		if (code == CURLE_OK)
		{
			return;
		}
	}

	curl_global_cleanup();

	failureCount++;

	if (failureCount > 5)
	{
		// as a last resort, just allow them
		GSClientApprove_t* retvals = (GSClientApprove_t*)malloc(sizeof(GSClientApprove_t));

		retvals->m_SteamID = steamID;

		CSteamBase::ReturnCall(retvals, sizeof(GSClientApprove_t), GSClientApprove_t::k_iCallback, 0);
	}
}

void CSteamGameServer009::LogOn() {
	
}

void CSteamGameServer009::LogOff() {
	
}

bool CSteamGameServer009::LoggedOn() {
	
	return true;
}

bool CSteamGameServer009::Secure() {
	
	return false;
}

CSteamID CSteamGameServer009::GetSteamID() {
	
	return CSteamID(1337, k_EUniversePublic, k_EAccountTypeGameServer);
}

bool CSteamGameServer009::SendUserConnectAndAuthenticate( uint32 unIPClient, const void *pvAuthBlob, uint32 cubAuthBlobSize, CSteamID *pSteamIDUser ) {
	
	int steamID = *(int*)pvAuthBlob;
	CSteamID realID = CSteamID( steamID, 1, k_EUniversePublic, k_EAccountTypeIndividual );

	_beginthread(ValidateConnection, 0, (void*)&realID);

	*pSteamIDUser = realID;
	return true;
}

CSteamID CSteamGameServer009::CreateUnauthenticatedUserConnection() {
	
	return CSteamID(1337, k_EUniversePublic, k_EAccountTypeIndividual);
}

void CSteamGameServer009::SendUserDisconnect( CSteamID steamIDUser ) {
	
}

bool CSteamGameServer009::UpdateUserData( CSteamID steamIDUser, const char *pchPlayerName, uint32 uScore ) {
	
	return true;
}

bool CSteamGameServer009::SetServerType( uint32 unServerFlags, uint32 unGameIP, uint16 unGamePort, uint16 unSpectatorPort, uint16 usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode ) {
	
	return true;
}

void CSteamGameServer009::UpdateServerStatus( int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pSpectatorServerName, const char *pchMapName ) {
	
}

void CSteamGameServer009::UpdateSpectatorPort( uint16 unSpectatorPort ) { }

void CSteamGameServer009::SetGameType( const char *pchGameType ) { }

bool CSteamGameServer009::GetUserAchievementStatus( CSteamID steamID, const char *pchAchievementName ) {
	
	return false;
}

void CSteamGameServer009::GetGameplayStats( ) {}

bool CSteamGameServer009::RequestUserGroupStatus( CSteamID steamIDUser, CSteamID steamIDGroup ) {
	
	return false;
}

uint32 CSteamGameServer009::GetPublicIP() {
	
	return 0;
}

void CSteamGameServer009::SetGameData( const char *pchGameData) {
	
}

EUserHasLicenseForAppResult CSteamGameServer009::UserHasLicenseForApp( CSteamID steamID, AppId_t appID ) {
	return k_EUserHasLicenseResultHasLicense;
}
