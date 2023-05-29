#include "lobby.h"
#include "app.h"

extern APPLICATION app;

LOBBY::LOBBY()
{
	room = 0;
	m_pBackGround = m_pUI = NULL;
	m_pMiniText = m_pSmallText = m_pCaption = NULL;
	m_pDevice = NULL;
	m_pSprite = NULL;
}

LOBBY::~LOBBY()
{
	//Release all resources
	if (m_pBackGround)m_pBackGround->Release();
	if (m_pUI)m_pUI->Release();
	if (m_pMiniText)m_pMiniText->Release();
	if (m_pSmallText)m_pSmallText->Release();
	if (m_pCaption)m_pCaption->Release();
	if (m_pSprite)m_pSprite->Release();
}

void LOBBY::Init(IDirect3DDevice9* Dev)
{
	room = 0;
	m_pDevice = Dev;

	//Load textures
	D3DXCreateTextureFromFile(m_pDevice, "textures/background.jpg", &m_pBackGround);
	D3DXCreateTextureFromFile(m_pDevice, "textures/UI.dds", &m_pUI);

	//Create fonts
	D3DXCreateFont(m_pDevice, 14, 0, 0, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Courier New", &m_pMiniText);

	D3DXCreateFont(m_pDevice, 18, 0, 0, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Courier New", &m_pSmallText);

	D3DXCreateFont(m_pDevice, 24, 0, FW_BOLD, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Courier New", &m_pCaption);

	//Create sprite and line interface
	D3DXCreateSprite(m_pDevice, &m_pSprite);
	D3DXCreateLine(m_pDevice, &m_pLine);
	m_pLine->SetAntialias(true);

	//Init Lobby Content
	RECT nRect[] = { {200, 300, 600, 330}, {30, 540, 640, 570} };
	m_nameTB.Init(nRect[0], 12);
	m_sessionTB.Init(nRect[0], 32);
	m_sendTB.Init(nRect[1], 50);
}

void LOBBY::Update(float deltaTime, MOUSE& mouse)
{
	//Update textboxes
	if (room == 0)
		m_nameTB.Update(deltaTime, mouse);
	else if (room == 2)
		m_sessionTB.Update(deltaTime, mouse);
	else if (room == 4)
		m_sendTB.Update(deltaTime, mouse);
}

void LOBBY::Draw(MOUSE& mouse)
{
	//Draw m_pBackGround
	D3DXMATRIX sca;
	D3DXMatrixScaling(&sca, 3.125f, 2.35f, 1.0f);

	m_pSprite->SetTransform(&sca);
	m_pSprite->Begin(0);
	D3DXVECTOR3 vec = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_pSprite->Draw(m_pBackGround, NULL, NULL, &vec, 0xffffffff);
	m_pSprite->End();

	D3DXMatrixIdentity(&sca);
	m_pSprite->SetTransform(&sca);

	switch (room)
	{
	case 0:	//Create Player room
	{
		RECT rc = { 0, 270, 800, 300 };
		m_pCaption->DrawText(NULL, "Enter Your Name:", -1, &rc, DT_CENTER | DT_TOP | DT_NOCLIP, 0xffffffff);
		m_nameTB.active = true;
		DrawTextbox(m_nameTB);

		RECT nextR = { 520, 360, 600, 390 };
		if (m_nameTB.text.size() > 3 && (TextButton("Next", nextR, true, mouse) || KEYDOWN(VK_RETURN)))
		{
			m_sessionTB.text = m_nameTB.text + "'s Game";
			room = 1;
		}
		break;
	}
	case 1:	//Host or Join room. (server/client)
	{
		RECT r[] = { {250, 250, 550, 280}, {250, 320, 550, 350} };
		if (TextButton("Host Game", r[0], true, mouse))
		{
			network.Init(true, (char*)m_nameTB.text.c_str());
			room = 2;
		}

		if (TextButton("Join Game", r[1], true, mouse))
		{
			network.Init(false, (char*)m_nameTB.text.c_str());
			network.FindSessions();
			room = 3;
		}

		break;
	}
	case 2:	//Create Session room
	{
		RECT rc = { 0, 270, 800, 300 };
		m_pCaption->DrawText(NULL, "Enter Session Name:", -1, &rc, DT_CENTER | DT_TOP | DT_NOCLIP, 0xffffffff);
		m_sessionTB.active = true;
		DrawTextbox(m_sessionTB);

		RECT nextR = { 520, 360, 600, 390 };
		if (m_sessionTB.text.size() > 3 && (TextButton("Next", nextR, true, mouse) || KEYDOWN(VK_RETURN)))
		{
			network.HostNewSession((char*)m_sessionTB.text.c_str());
			room = 4;
		}
		break;
	}
	case 3:	//Join Session room
	{
		RECT r[] = { {50, 50, 750, 550}, {50, 50, 750, 100}, {600, 60, 740, 90} };

		Square(r[0], 0xffffffff);
		Square(r[1], 0xffffffff);

		RECT rc = { 70, 60, 0, 0 };
		m_pCaption->DrawText(NULL, "Active Games", -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
		if (TextButton("Refresh", r[2], true, mouse))network.FindSessions();

		//Draw list of available sessions
		for (int i = 0; i < network.sessions.size(); i++)
		{
			RECT rs = { 60, 110 + 40 * i, 600, 140 + 40 * i };
			char number[10];
			//std::string text = _itoa(i + 1, number, 10);
			std::string text;
			_itoa_s(i + 1, number, sizeof(number), 10);
			text += text;
			text += ": ";
			text += network.sessions[i].name;

			if (TextButton((char*)text.c_str(), rs, false, mouse))
			{
				HRESULT hr = network.ConnectToSession(i);

				//Connection failed...
				if (FAILED(hr))
				{
					if (hr == DPNERR_HOSTREJECTEDCONNECTION)
						m_error = "Host rejected connection";
					else if (hr == DPNERR_NOCONNECTION)
						m_error = "No connection was made";
					else if (hr == DPNERR_NOTHOST)
						m_error = "Host not found";
					else if (hr == DPNERR_SESSIONFULL)
						m_error = "Session is full";
					else if (hr == DPNERR_ALREADYCONNECTED)
						m_error = "You are already connected";
					else if (hr == DPNERR_INVALIDAPPLICATION)
						m_error = "Invalid GUID";
					else if (hr == DPNERR_INVALIDDEVICEADDRESS)
						m_error = "Invalid m_pDevice Address";
					else if (hr == DPNERR_INVALIDFLAGS)
						m_error = "Invalid Flags";
					else if (hr == DPNERR_INVALIDHOSTADDRESS)
						m_error = "Invalid Host Address";
					else if (hr == DPNERR_INVALIDINSTANCE)
						m_error = "Invalid GUID 2";
					else if (hr == DPNERR_INVALIDINTERFACE)
						m_error = "Invalid Interface";
					else if (hr == DPNERR_INVALIDPASSWORD)
						m_error = "Invalid Password";
					else m_error = "Unknown Error";

					room = 999;
				}
				else room = 4;
			}
		}

		break;
	}
	case 4:	//Chat room
	{
		RECT r[] = { {20, 20, 500, 470}, {20, 20, 500, 70}, {520, 20, 780, 470}, {520, 20, 780, 70}, {20, 490, 780, 580} };
		RECT btn[] = { {660, 540, 770, 570} };

		for (int i = 0; i < 5; i++)
			Square(r[i], 0xffffffff);

		m_sendTB.active = true;

		RECT r1[] = { {40, 30, 0, 0}, {540, 30, 0, 0}, {40, 500, 0, 0} };
		m_pCaption->DrawText(NULL, network.activeSession, -1, &r1[0], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
		m_pCaption->DrawText(NULL, "Players", -1, &r1[1], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
		m_pCaption->DrawText(NULL, "Send Text:", -1, &r1[2], DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);

		DrawTextbox(m_sendTB);

		//Send text message
		if (TextButton("Send", btn[0], true, mouse) || KEYDOWN(VK_RETURN))
			if (m_sendTB.text.size() > 0)
			{
				std::string c = network.playerName;
				c += " says: ";
				c += m_sendTB.text;
				if (network.server)network.chat.push_back(c);
				if (network.chat.size() > 20)network.chat.erase(network.chat.begin());

				MSG_TEXT msg((char*)c.c_str());
				network.Send(&msg, true);
				m_sendTB.text.clear();
			}

		//Draw player list
		for (int i = 0; i < network.players.size(); i++)
		{
			RECT rc = { 530, 90 + 40 * i, 0, 0 };
			m_pCaption->DrawText(NULL, network.players[i].name, -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
		}

		//Draw chat
		for (int i = 0; i < network.chat.size(); i++)
		{
			RECT rc = { 30, 80 + 19 * (network.chat.size() - i - 1), 0, 0 };
			m_pSmallText->DrawText(NULL, network.chat[i].c_str(), -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
		}

		//Check if the connection was terminated
		if (!network.connected)
		{
			m_error = "Session Terminated";
			room = 999;
		}
		else if (network.server && network.players.size() > 1)
		{
			RECT rs = { 530, 430, 770, 460 };
			if (TextButton("Start Game", rs, true, mouse))
			{
				//Send terrain to all clients before staring game...
				MSG_TERRAIN terr(app.m_terrain, 0);
				network.Send(&terr, true);
				room = 5;
			}
		}

		break;
	}
	case 5:		//Transfer initial info Room
	{
		for (int i = 0; i < network.players.size(); i++)
		{
			std::string play = network.players[i].name;
			if (network.players[i].done)play += "...Done";

			RECT rc = { 250, 200 + i * 40, 0, 0 };
			m_pCaption->DrawText(NULL, play.c_str(), -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
		}

		break;
	}
	case 999:	//Error room
	{
		RECT rc = { 0, 300, 800, 330 };
		m_pCaption->DrawText(NULL, m_error.c_str(), -1, &rc, DT_CENTER | DT_TOP | DT_NOCLIP, 0xffffffff);

		RECT r = { 300, 330, 500, 360 };
		if (TextButton("OK", r, true, mouse))
		{
			room = 0;
			m_nameTB.text.clear();
			m_sessionTB.text.clear();
			m_sendTB.text.clear();
			network.Release();
		}

		break;
	}
	}
}

void LOBBY::DrawTextbox(TEXTBOX& tb)	//Draws a textbox
{
	Square(tb.rect, 0xffffffff);

	RECT rc = { tb.rect.left + 10, tb.rect.top + 6, 0, 0 };
	m_pSmallText->DrawText(NULL, tb.GetText().c_str(), -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP, 0xffffffff);
}

bool LOBBY::TextButton(const char text[], RECT r, bool border, MOUSE& mouse)	//A text button...
{
	DWORD col = 0xffffffff;
	if (mouse.Over(r))col = 0xffff0000;

	if (border)
	{
		Square(r, col);
		m_pCaption->DrawText(NULL, text, -1, &r, DT_CENTER | DT_VCENTER | DT_NOCLIP, col);
	}
	else
	{
		RECT rc = { r.left + 10, r.top + 4, 0, 0 };
		m_pCaption->DrawText(NULL, text, -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP, col);
	}

	if (mouse.PressInRect(r))
	{
		mouse.DisableInput(300);
		return true;
	}
	else return false;
}

void LOBBY::Square(RECT r, DWORD col)		//Draws a simple square
{
	D3DXMATRIX sca;
	D3DXVECTOR2 scale = D3DXVECTOR2(r.right - r.left, r.bottom - r.top);
	D3DXMatrixScaling(&sca, scale.x, scale.y, 1.0f);
	m_pSprite->SetTransform(&sca);
	RECT src = { 1,1,2,2 };
	m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
	D3DXVECTOR3 vec = D3DXVECTOR3(r.left / scale.x, r.top / scale.y, 0.0f);
	m_pSprite->Draw(m_pUI, &src, NULL, &vec, 0xffffffff);
	m_pSprite->End();

	D3DXMatrixIdentity(&sca);
	m_pSprite->SetTransform(&sca);

	D3DXVECTOR2 rect[] = { D3DXVECTOR2(r.left, r.top), D3DXVECTOR2(r.right, r.top),
						  D3DXVECTOR2(r.right, r.bottom), D3DXVECTOR2(r.left, r.bottom),
						  D3DXVECTOR2(r.left, r.top) };

	m_pLine->SetWidth(2.5f);
	m_pLine->Begin();
	m_pLine->Draw(rect, 5, col);
	m_pLine->End();
}