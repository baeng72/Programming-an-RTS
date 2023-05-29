#include "network.h"
#include "app.h"

// {D962EEE5-7783-4018-9C0D-C326821F808F}, Unique application ID
GUID RTS_APP_ID = { 0xd962eee5, 0x7783, 0x4018, {0x9c, 0xd, 0xc3, 0x26, 0x82, 0x1f, 0x80, 0x8f} };

NETWORK network;	//Global network variable
extern APPLICATION app;

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
		CoInitialize(0);
		strcpy(playerName, _playerName);
		connected = false;

		if (server)		//Init Server
		{
			if (FAILED(CoCreateInstance(CLSID_DirectPlay8Server, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Server, (LPVOID*)&m_pServer)))debug.Print("Failed to Create Server.\n");
			if (FAILED(m_pServer->Initialize(NULL, ServerCallback, 0)))debug.Print("Failed to Init Server.\n");
		}
		else			//Init Client
		{
			if (FAILED(CoCreateInstance(CLSID_DirectPlay8Client, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Client, (LPVOID*)&m_pClient)))debug.Print("Failed to Create Client.\n");
			if (FAILED(m_pClient->Initialize(NULL, ClientCallback, 0)))debug.Print("Failed to Init Client.\n");
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

	if (FAILED(m_pClient->EnumHosts(&desc,		// pApplicationDesc
		m_pServerAddress,    // Host Address
		m_pMyAddress,        // Device Address
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

	strcpy(activeSession, sessions[index].name.c_str());

	HRESULT hr = m_pClient->Connect(&sessions[index].desc,		// pdnAppDesc
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

void NETWORK::Send(RTS_MSG* msg, bool guaranteed)
{
	//Sends the message to all players
	SendTo(DPNID_ALL_PLAYERS_GROUP, msg, guaranteed);
}

void NETWORK::SendTo(DPNID id, RTS_MSG* msg, bool guaranteed)
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
		else if (msg->type == GAME_MSG_TERRAIN)
			desc.dwBufferSize = sizeof(MSG_TERRAIN);
		else if (msg->type == GAME_MSG_OBJECT)
			desc.dwBufferSize = sizeof(MSG_OBJECT);
		else if (msg->type == GAME_MSG_ORDER)
			desc.dwBufferSize = sizeof(MSG_ORDER);

		DWORD flags = DPNSEND_SYNC | DPNSEND_NOLOOPBACK;
		if (guaranteed)flags |= DPNSEND_GUARANTEED;

		if (server && m_pServer)
		{
			m_pServer->SendTo(id,					      // dpnid
				&desc,		              // pBufferDesc
				1,                        // cBufferDesc
				0,                        // dwTimeOut
				NULL,                     // pvAsyncContext
				NULL,                     // pvAsyncHandle
				flags);			          // dwFlags							    
		}
		else if (m_pClient)
		{
			m_pClient->Send(&desc,				// pBufferDesc
				1,                      // cBufferDesc
				0,                      // dwTimeOut
				NULL,                   // pvAsyncContext
				NULL,                   // pvAsyncHandle
				flags);    // dwFlags
		}
	}
	catch (...)
	{
		debug.Print("Error in NETWORK::SendTo()");
	}
}

std::vector<DP_PLAYER>::iterator FindPlayer(std::vector<DP_PLAYER>& players, int id) {
	for (std::vector<DP_PLAYER>::iterator it = players.begin(); it != players.end(); it++) {
		if (it->id == id)
			return it;
	}
	return players.end();
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
				network.players[network.players.size() - 1].done = true;
			}
			else
			{
				//Ask for new players name
				MSG_COMMAND cmd(0, 0);
				network.SendTo(msg->dpnidPlayer, &cmd, true);
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
				network.Send(&play, true);
				auto it = FindPlayer(network.players, player->id);
				if(it!=network.players.end())
					network.players.erase(it);
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
				network.Send(msg, true);
				break;
			}
			case GAME_MSG_COMMAND:		//Game Command
			{
				MSG_COMMAND* cmd = (MSG_COMMAND*)msg;

				if (cmd->command == 1)	//Next terrain piece
				{
					if (cmd->attribute * 100 < app.m_terrain.m_size.x * app.m_terrain.m_size.y)
					{
						MSG_TERRAIN terr(app.m_terrain, cmd->attribute);
						network.SendTo(data->dpnidSender, &terr, true);
					}
					else if (player != NULL)	//Start transmitting objects
					{
						MSG_OBJECT obj(app.m_terrain.m_objects[0], 0);
						network.SendTo(data->dpnidSender, &obj, true);
					}
				}
				else if (cmd->command == 2)	//Next Terrain Object
				{
					if (cmd->attribute >= app.m_terrain.m_objects.size())	//Done
					{
						player->done = true;

						//Check that all players are done
						bool allDone = true;
						for (int i = 0; i < network.players.size(); i++)
							if (!network.players[i].done)
								allDone = false;

						if (allDone)		//Start game
						{
							for (int i = 1; i < network.players.size(); i++)	//Add players
							{
								MSG_COMMAND cplay(3, i);
								network.SendTo(network.players[i].id, &cplay, true);
							}

							MSG_COMMAND cmd(4, 0);
							network.Send(&cmd, true);
							app.m_lobby.room = 10;

							srand(0);
							app.AddPlayers(network.players.size(), 0);
						}
					}
					else		//Send next object
					{
						MSG_OBJECT obj(app.m_terrain.m_objects[cmd->attribute], cmd->attribute);
						network.SendTo(data->dpnidSender, &obj, true);
					}
				}

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
						network.SendTo(data->dpnidSender, &ply, true);
					}

					MSG_PLAYER newPlayer(network.players[network.players.size() - 1], 0);
					network.Send(&newPlayer, true);
				}
				break;
			}
			case GAME_MSG_ORDER:		//Unit Order
			{
				app.HandleNetworkOrder(msg);
				network.Send(msg, true);	//Relay order
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
			wcstombs(sessionName, desc->pwszSessionName, wcslen(desc->pwszSessionName) + 1);
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
					network.Send(&play, true);
				}
				else if (cmd->command == 3)	//Add players
				{
					srand(0);
					app.m_terrain.InitPathfinding();
					app.AddPlayers(network.players.size(), cmd->attribute);
				}
				else if (cmd->command == 4)	//Start game
				{
					app.m_lobby.room = 10;
					app.m_terrain.m_updateTerrain = true;		//Creates patches, pathfinding etc...
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
					if (playerToErase != NULL) {
						auto it = FindPlayer(network.players, playerMessage->player.id);
						if(it!=network.players.end())
							network.players.erase(it);
					}
				}
				if (playerMessage->operation == 2 && player)		//Update Name
					strcpy(player->name, playerMessage->player.name);

				break;
			}
			case GAME_MSG_TERRAIN:		//Receive heightmap from server
			{
				MSG_TERRAIN* terr = (MSG_TERRAIN*)msg;

				app.m_lobby.room = 5;

				for (int i = terr->packageNo * 100, i2 = 0; i2 < 100 && i < terr->size.x * terr->size.y; i++, i2++)
					app.m_terrain.m_pHeightMap->m_pHeightMap[i] = terr->hm[i2];

				MSG_COMMAND cmd(1, terr->packageNo + 1);
				network.Send(&cmd, true);

				break;
			}
			case GAME_MSG_OBJECT:		//Add terrain objects
			{
				MSG_OBJECT* obj = (MSG_OBJECT*)msg;
				if (obj->packageNo == 0)
					app.m_terrain.m_objects.clear();

				app.m_terrain.m_objects.push_back(OBJECT(obj->objType, obj->mp, obj->pos, obj->rot, obj->sca));

				MSG_COMMAND cmd(2, obj->packageNo + 1);
				network.Send(&cmd, true);

				break;
			}
			case GAME_MSG_ORDER:		//Unit Order
			{
				app.HandleNetworkOrder(msg);
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