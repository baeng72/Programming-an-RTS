#ifndef _RTS_APP_
#define _RTS_APP_

#include <windows.h>
#include <d3dx9.h>
#include "debug.h"
#include "mouse.h"
#include "network.h"
#include "lobby.h"
#include "camera.h"
#include "terrain.h"
#include "player.h"

extern NETWORK network;

class APPLICATION
{
	friend class LOBBY;
public:

	APPLICATION();
	HRESULT Init(HINSTANCE hInstance, int width, int height, bool windowed);
	HRESULT Update(float deltaTime);
	HRESULT Render();
	HRESULT Cleanup();
	HRESULT Quit();
	DWORD FtoDword(float f) { return *((DWORD*)&f); }
	void AddPlayers(int num, int activePlayer);
	void HandleNetworkOrder(RTS_MSG* msg);

	//Public variables
	TERRAIN m_terrain;
	CAMERA m_camera;
	MOUSE m_mouse;
	LOBBY m_lobby;
	std::vector<PLAYER*> m_players;

private:

	IDirect3DDevice9* m_pDevice;
	int m_thisPlayer;
	HWND m_mainWindow;
};

#endif
