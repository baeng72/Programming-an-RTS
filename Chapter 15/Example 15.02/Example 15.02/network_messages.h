
#include <windows.h>
#include "dplay8.h"     // directplay
#include "debug.h"
#include "terrain.h"
#include "intpoint.h"

#define GAME_MSG_TEXT 0
#define GAME_MSG_COMMAND 1
#define GAME_MSG_PLAYER 2
#define GAME_MSG_TERRAIN 3
#define GAME_MSG_OBJECT 4
#define GAME_MSG_ORDER 5

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
		done = false;
	}

	DPNID id;
	char name[64];
	bool done;
};

struct RTS_MSG
{
	DWORD type;
};

struct MSG_COMMAND : public RTS_MSG		//Game command
{
	MSG_COMMAND(DWORD cmd, DWORD att) { command = cmd; attribute = att; type = GAME_MSG_COMMAND; }
	DWORD command;		//0 = Ask for Name, 1 = Send Terrain Piece, 2 = Send Object, 3 = Add Players, 4 = Start Game
	DWORD attribute;
};

struct MSG_TEXT : public RTS_MSG		//Text Message
{
	MSG_TEXT(char txt[])
	{
		//strcpy(text, txt);
		strcpy_s(text, sizeof(text), txt);
		type = GAME_MSG_TEXT;
	}

	char text[100];
};

struct MSG_PLAYER : public RTS_MSG				//Player Message
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

struct MSG_TERRAIN : public RTS_MSG
{
	MSG_TERRAIN(TERRAIN& terrain, DWORD packNo)
	{
		packageNo = packNo;
		type = GAME_MSG_TERRAIN;
		size = terrain.m_size;

		memset(hm, 0, sizeof(float) * 100);

		for (int i = packageNo * 100, i2 = 0; i2 < 100 && i < size.x * size.y; i++, i2++)
			hm[i2] = terrain.m_pMapTiles[i].m_height;
	}

	DWORD packageNo;
	INTPOINT size;
	float hm[100];
};

struct MSG_OBJECT : public RTS_MSG
{
	MSG_OBJECT(OBJECT& obj, DWORD packNo)
	{
		type = GAME_MSG_OBJECT;
		objType = obj.m_type;
		mp = obj.m_mappos;
		pos = obj.m_meshInstance.m_pos;
		rot = obj.m_meshInstance.m_rot;
		sca = obj.m_meshInstance.m_sca;
		packageNo = packNo;
	}

	int objType;
	DWORD packageNo;
	INTPOINT mp;
	D3DXVECTOR3 pos, rot, sca;
};

struct MSG_ORDER : public RTS_MSG
{
	MSG_ORDER(int _player, int _unitID, INTPOINT _dest)
	{
		type = GAME_MSG_ORDER;
		unitID = _unitID;
		dest = _dest;
		player = _player;
	}

	int unitID, player;
	INTPOINT dest;
};