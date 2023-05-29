#ifndef _RTS_NETWORK_
#define _RTS_NETWORK_

#include <windows.h>
#include "dplay8.h"     // directplay
#include <vector>
#include "debug.h"
#include "terrain.h"
#include "network_messages.h"

class NETWORK
{
	friend HRESULT WINAPI ServerCallback(PVOID pvUserContext, DWORD dwMessageType, PVOID pMessage);
	friend HRESULT WINAPI ClientCallback(PVOID pvUserContext, DWORD dwMessageType, PVOID pMessage);
public:
	NETWORK();
	~NETWORK();

	void Init(bool _server, char _playerName[]);
	void Release();
	void HostNewSession(char sessionName[]);
	void FindSessions();									//Enumerate all available sessions
	HRESULT ConnectToSession(int index);
	DP_PLAYER* FindPlayer(DPNID id);
	void Send(RTS_MSG* msg, bool guaranteed);				//Send to all (or server)
	void SendTo(DPNID id, RTS_MSG* msg, bool guaranteed);	//Send To specific player 

	//Public Variables
	bool server, connected;
	char playerName[64];				//Name of local player
	char activeSession[50];				//name of active session
	std::vector<SESSION> sessions;
	std::vector<DP_PLAYER> players;
	std::vector<std::string> chat;

private:
	IDirectPlay8Server* m_pServer;
	IDirectPlay8Client* m_pClient;
	IDirectPlay8Address* m_pMyAddress, * m_pServerAddress;
	DPNID m_localID;
};

//Network callback functions
HRESULT WINAPI ServerCallback(PVOID pvUserContext, DWORD dwMessageType, PVOID pMessage);
HRESULT WINAPI ClientCallback(PVOID pvUserContext, DWORD dwMessageType, PVOID pMessage);

#endif