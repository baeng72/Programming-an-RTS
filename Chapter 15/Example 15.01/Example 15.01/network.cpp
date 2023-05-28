#include "network.h"

// {D962EEE5-7783-4018-9C0D-C326821F808F}, Unique application ID
GUID RTS_APP_ID = { 0xd962eee5, 0x7783, 0x4018, {0x9c, 0xd, 0xc3, 0x26, 0x82, 0x1f, 0x80, 0x8f} };

NETWORK network;	//Global network variable

NETWORK::NETWORK()
{
	m_pServer = NULL;
	m_pClient = NULL;
	m_pMyAddress = NULL;
	m_pServerAddress = NULL;
}

NETWORK::~NETWORK()
{
	Release();
}

void NETWORK::Init(bool _server, char _playerName[])
{
	try
	{
		server = _server;
		CoInitialize(NULL);
		strcpy(playerName, _playerName);
		
		connected = false;
		HRESULT hr;
		if (server)		//Init Server
		{
			
			if (FAILED(CoCreateInstance(CLSID_DirectPlay8Server, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Server, (LPVOID*)&m_pServer)))debug.Print("Failed to Create Server.\n");
			

			if (FAILED(m_pServer->Initialize(NULL, ServerCallback, 0)))
			
				debug.Print("Failed to Init Server.\n");
		}
		else			//Init Client
		{
		
			if (FAILED(CoCreateInstance(CLSID_DirectPlay8Client, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Client, (LPVOID*)&m_pClient)))debug.Print("Failed to Create Client.\n");
		
		}
		
		//Create My address
		if (FAILED(CoCreateInstance(CLSID_DirectPlay8Address, NULL, CLSCTX_ALL, IID_IDirectPlay8Address, (LPVOID*)&m_pMyAddress)))debug.Print("Failed to Create m_pMyAddress.\n");
		
		if (FAILED(m_pMyAddress->SetSP(&CLSID_DP8SP_TCPIP)))debug.Print("Failed to set Service Protocol");
		

		//Create Server address
		if (FAILED(CoCreateInstance(CLSID_DirectPlay8Address, NULL, CLSCTX_ALL, IID_IDirectPlay8Address, (LPVOID*)&m_pServerAddress)))debug.Print("Failed to Create m_pServerAddress.\n");
		
		if (FAILED(m_pServerAddress->SetSP(&CLSID_DP8SP_TCPIP)))debug.Print("Failed to set Service Protocol");
		

		CoUninitialize();
	}
	catch (...)
	{
		debug.Print("Error in NETWORK::Init()");
	}
}

void NETWORK::Release()
{
	try
	{
		if (m_pMyAddress)m_pMyAddress->Release();
		if (m_pServerAddress)m_pServerAddress->Release();
		m_pMyAddress = NULL;
		m_pServerAddress = NULL;

		//Close connections
		if (m_pServer)
		{
			m_pServer->Close(DPNCLOSE_IMMEDIATE);
			m_pServer = NULL;
		}
		if (m_pClient)
		{
			m_pClient->Close(DPNCLOSE_IMMEDIATE);
			m_pClient = NULL;
		}
	}
	catch (...)
	{
		debug.Print("Error in NETWORK::Release()");
	}
}

void NETWORK::HostNewSession(char sessionName[])
{
	try
	{
		WCHAR strHost[128];
		DPN_APPLICATION_DESC desc;
		mbstowcs(strHost, sessionName, strlen(sessionName) + 1);
		strcpy(activeSession, sessionName);
		

		// Set up the Application Description.
		ZeroMemory(&desc, sizeof(DPN_APPLICATION_DESC));
		desc.dwSize = sizeof(DPN_APPLICATION_DESC);
		desc.dwFlags = DPNSESSION_CLIENT_SERVER;	// Flag describing the app
		desc.guidApplication = RTS_APP_ID;          // GUID for the app
		desc.pwszSessionName = strHost;				// Session name

		// Host the application.
		if (FAILED(m_pServer->Host(&desc, &m_pMyAddress, 1, NULL, NULL, this, 0)))
			debug.Print("Failed to host session");
		else connected = true;
	}
	catch (...)
	{
		debug.Print("Error in NETWORK::HostNewSession()");
	}
}

void NETWORK::FindSessions()
{
	if (m_pClient == NULL)return;

	sessions.clear();

	//Setup what kind of sessions you are looking for
	DPN_APPLICATION_DESC desc;
	ZeroMemory(&desc, sizeof(DPN_APPLICATION_DESC));
	desc.dwSize = sizeof(DPN_APPLICATION_DESC);
	desc.guidApplication = RTS_APP_ID;

	if (FAILED(m_pClient->EnumHosts(&desc,     // pApplicationDesc
		m_pServerAddress,       // Host Address
		m_pMyAddress,           // Device Address
		NULL, 0,             // pvUserEnumData, size
		1,                   // dwEnumCount
		0,                   // dwRetryInterval
		0,                   // dwTimeOut
		NULL,                // pvUserContext
		NULL,                // pAsyncHandle
		DPNENUMHOSTS_SYNC))) // dwFlags
		debug.Print("Failed to enumerate Sessions");
}

HRESULT NETWORK::ConnectToSession(int index)
{
	if (index < 0 || index >= sessions.size() || m_pClient == NULL)
	{
		debug.Print("Not correct session to connect to, or m_pClient is NULL");
		return E_FAIL;
	}

	//strcpy(activeSession, sessions[index].name.c_str());
	strcpy_s(activeSession, sizeof(activeSession), sessions[index].name.c_str());

	HRESULT hr = m_pClient->Connect(&sessions[index].desc,    // pdnAppDesc
		sessions[index].address,      // pHostAddr
		m_pMyAddress,					// pDeviceInfo
		NULL,							// pdnSecurity
		NULL,							// pdnCredentials
		NULL, 0,						// pvUserConnectData, Size
		NULL,							// pvAsyncContext
		NULL,							// pvAsyncHandle
		DPNCONNECT_SYNC);				// dwFlags

	if (SUCCEEDED(hr))connected = true;

	return hr;
}

DP_PLAYER* NETWORK::FindPlayer(DPNID id)
{
	//Finds a player in the player vector with matching ID
	for (int i = 0; i < network.players.size(); i++)
		if (network.players[i].id == id)
			return &network.players[i];

	return NULL;
}

void NETWORK::Send(RTS_MSG* msg)
{
	//Sends the message to all players
	SendTo(DPNID_ALL_PLAYERS_GROUP, msg);
}

void NETWORK::SendTo(DPNID id, RTS_MSG* msg)
{
	try
	{
		DPN_BUFFER_DESC desc;
		desc.pBufferData = (BYTE*)msg;

		//Message size
		if (msg->type == GAME_MSG_TEXT)
			desc.dwBufferSize = sizeof(MSG_TEXT);
		else if (msg->type == GAME_MSG_COMMAND)
			desc.dwBufferSize = sizeof(MSG_COMMAND);
		else if (msg->type == GAME_MSG_PLAYER)
			desc.dwBufferSize = sizeof(MSG_PLAYER);

		if (server && m_pServer)
		{
			m_pServer->SendTo(id,					      // dpnid
				&desc,		              // pBufferDesc
				1,                        // cBufferDesc
				0,                        // dwTimeOut
				NULL,                     // pvAsyncContext
				NULL,                     // pvAsyncHandle
				DPNSEND_SYNC |            // dwFlags
				DPNSEND_NOLOOPBACK);
		}
		else if (m_pClient)
		{
			m_pClient->Send(&desc,					// pBufferDesc
				1,                      // cBufferDesc
				0,                      // dwTimeOut
				NULL,                   // pvAsyncContext
				NULL,                   // pvAsyncHandle
				DPNSEND_SYNC |
				DPNSEND_NOLOOPBACK);    // dwFlags
		}
	}
	catch (...)
	{
		debug.Print("Error in NETWORK::SendTo()");
	}
}

HRESULT WINAPI ServerCallback(PVOID pvUserContext, DWORD dwMessageType, PVOID pMessage)
{
	try
	{
		switch (dwMessageType)
		{
		case DPN_MSGID_CREATE_PLAYER:		//New player has connected
		{
			PDPNMSG_CREATE_PLAYER msg = (PDPNMSG_CREATE_PLAYER)pMessage;

			if (msg->pvPlayerContext == &network)	//Local player
			{
				network.m_localID = msg->dpnidPlayer;
				network.players.push_back(DP_PLAYER(msg->dpnidPlayer, network.playerName));
			}
			else
			{
				//Ask for new players name
				MSG_COMMAND cmd(0, 0);
				network.SendTo(msg->dpnidPlayer, &cmd);
			}

			break;
		}
		case DPN_MSGID_DESTROY_PLAYER:		//Player has left the game
		{
			PDPNMSG_DESTROY_PLAYER msg = (PDPNMSG_DESTROY_PLAYER)pMessage;

			DP_PLAYER* player = network.FindPlayer(msg->dpnidPlayer);
			if (player != NULL)
			{
				MSG_PLAYER play(*player, 1);
				network.Send(&play);


				//auto it = std::find(network.players.begin(), network.players.end(), player);
				///if(it!=network.players.end())
				for (std::vector<DP_PLAYER>::iterator it = network.players.begin(); it != network.players.end(); ++it) {

					if (it->id == player->id) {
						network.players.erase(it);
					}
				}


			}

			break;
		}
		case DPN_MSGID_TERMINATE_SESSION:	//Session terminated
		{
			network.players.clear();
			network.connected = false;

			break;
		}
		case DPN_MSGID_RECEIVE:		//Data received
		{
			PDPNMSG_RECEIVE data = (PDPNMSG_RECEIVE)pMessage;
			RTS_MSG* msg = (RTS_MSG*)data->pReceiveData;
			DP_PLAYER* player = network.FindPlayer(data->dpnidSender);

			switch (msg->type)
			{
			case GAME_MSG_TEXT:		//Text message
			{
				MSG_TEXT* textMessage = (MSG_TEXT*)msg;
				network.chat.push_back(textMessage->text);
				if (network.chat.size() > 20)network.chat.erase(network.chat.begin());

				//Relay text message to other players
				network.Send(msg);
				break;
			}
			case GAME_MSG_COMMAND:		//Game Command
			{

				break;
			}
			case GAME_MSG_PLAYER:		//Add, Remove players etc...
			{
				MSG_PLAYER* playerMessage = (MSG_PLAYER*)msg;

				if (playerMessage->operation == 2)		//Get Name
				{
					network.players.push_back(DP_PLAYER(data->dpnidSender, playerMessage->player.name));

					//Send all other players to the new player...
					for (int i = 0; i < network.players.size(); i++)
					{
						MSG_PLAYER ply(network.players[i], 0);
						network.SendTo(data->dpnidSender, &ply);
					}

					MSG_PLAYER newPlayer(network.players[network.players.size() - 1], 0);
					network.Send(&newPlayer);
				}
				break;
			}
			}

			break;
		}
		}

		return S_OK;
	}
	catch (...)
	{
		debug.Print("Error in ServerCallback()");
		return E_FAIL;
	}
}

HRESULT WINAPI ClientCallback(PVOID pvUserContext, DWORD dwMessageType, PVOID pMessage)
{
	try
	{
		switch (dwMessageType)
		{
		case DPN_MSGID_ENUM_HOSTS_RESPONSE:		// Enumerate sessions responses
		{
			PDPNMSG_ENUM_HOSTS_RESPONSE msg = (PDPNMSG_ENUM_HOSTS_RESPONSE)pMessage;
			const DPN_APPLICATION_DESC* desc = msg->pApplicationDescription;
			char sessionName[128];
			size_t retVal;
			wcstombs_s(&retVal, sessionName, desc->pwszSessionName, sizeof(sessionName));
			network.sessions.push_back(SESSION(sessionName, *desc, msg->pAddressSender));
			break;
		}
		case DPN_MSGID_TERMINATE_SESSION:		//Server died, or some such...
		{
			network.players.clear();
			network.connected = false;

			break;
		}
		case DPN_MSGID_RECEIVE:			//Data...
		{
			PDPNMSG_RECEIVE data = (PDPNMSG_RECEIVE)pMessage;
			RTS_MSG* msg = (RTS_MSG*)data->pReceiveData;
			DP_PLAYER* player = network.FindPlayer(data->dpnidSender);

			switch (msg->type)
			{
			case GAME_MSG_TEXT:		//Chat/text message
			{
				MSG_TEXT* textMessage = (MSG_TEXT*)msg;
				network.chat.push_back(textMessage->text);
				if (network.chat.size() > 20)network.chat.erase(network.chat.begin());

				break;
			}
			case GAME_MSG_COMMAND:		//Game command
			{
				MSG_COMMAND* cmd = (MSG_COMMAND*)msg;

				if (cmd->command == 0)		//Ask for name
				{
					MSG_PLAYER play(DP_PLAYER(0, network.playerName), 2);
					network.Send(&play);
				}

				break;
			}
			case GAME_MSG_PLAYER:		//Add, Remove, Update Name of players...
			{
				MSG_PLAYER* playerMessage = (MSG_PLAYER*)msg;

				if (playerMessage->operation == 0)	//Add Player
				{
					DP_PLAYER* oldPlayer = network.FindPlayer(playerMessage->player.id);

					if (oldPlayer == NULL)
					{
						network.players.push_back(playerMessage->player);
						printf("Player %s joined game\n", playerMessage->player.name);
					}
				}
				else if (playerMessage->operation == 1)	//Remove Player
				{
					DP_PLAYER* playerToErase = network.FindPlayer(playerMessage->player.id);
					if (playerToErase != nullptr) {
						for (std::vector<DP_PLAYER>::iterator it = network.players.begin(); it != network.players.end(); ++it) {
							if (it->id == playerToErase->id) {
								network.players.erase(it);
							}
						}

						//if (playerToErase != NULL)network.players.erase(playerToErase);
					}
				}
				if (playerMessage->operation == 2 && player)		//Update Name
					//strcpy(player->name, playerMessage->player.name);
					strcpy_s(player->name, sizeof(player->name), playerMessage->player.name);

				break;
			}
			}

			break;
		}
		}

		return S_OK;
	}
	catch (...)
	{
		debug.Print("Error in ClientCallback()");
		return E_FAIL;
	}
}