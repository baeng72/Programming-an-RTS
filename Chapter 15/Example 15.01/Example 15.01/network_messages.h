#include <windows.h>
#include "dplay8.h"     // directplay
#include <vector>
#include "debug.h"

#define GAME_MSG_TEXT 0
#define GAME_MSG_COMMAND 1
#define GAME_MSG_PLAYER 2

struct SESSION
{
	SESSION(char _name[], DPN_APPLICATION_DESC _desc, IDirectPlay8Address* _address)
	{
		name = _name;
		desc = _desc;
		if (_address != NULL)_address->Duplicate(&address);
	}

	std::string name;
	DPN_APPLICATION_DESC desc;
	IDirectPlay8Address* address;
};

struct DP_PLAYER
{
	DP_PLAYER() {}
	DP_PLAYER(DPNID _id, char _name[])
	{
		id = _id;
		//strcpy(name, _name);
		strcpy_s(name, sizeof(name), _name);
	}

	DPNID id;
	char name[64];
};

struct RTS_MSG
{
	DWORD type;
};

struct MSG_COMMAND : public RTS_MSG		//Game command
{
	MSG_COMMAND(DWORD cmd, DWORD att) { command = cmd; attribute = att; type = GAME_MSG_COMMAND; }
	DWORD command;		//0 = Ask for Name, 
	DWORD attribute;
};

struct MSG_TEXT : public RTS_MSG		//Text Message
{
	MSG_TEXT(char txt[])
	{
		//copy text
		//strcpy(text, txt);
		strcpy_s(text, sizeof(text), txt);

		//Set message type
		type = GAME_MSG_TEXT;
	}

	char text[100];
};

struct MSG_PLAYER : RTS_MSG				//Player Message
{
	MSG_PLAYER(DP_PLAYER dpp, BYTE _operation)
	{
		player = dpp;
		operation = _operation;			//0 = Add Player, 1 = Remove Player, 2 = Get Name
		type = GAME_MSG_PLAYER;
	}

	DP_PLAYER player;
	BYTE operation;
};
