#include <vector>
#include "network.h"
#include "mouse.h"

extern NETWORK network;

//Textbox is used for text input
struct TEXTBOX
{
	void Init(RECT r, int _maxLength)
	{
		active = false;
		rect = r;
		maxLength = _maxLength;
		lastKey = -1;
		lastKeydown = GetTickCount();
		time = 0.0f;
	}

	void Update(float deltaTime, MOUSE& mouse)
	{
		time += deltaTime;
		if (time > 0.5f) { time = 0.0f; blink = !blink; }

		//Active true/false
		if (mouse.ClickLeft())
			active = mouse.Over(rect);

		//Keyboard input
		if (active && strlen(text.c_str()) < maxLength)
			for (int i = 32; i < 126; i++)
				if (KEYDOWN(i))
				{
					if (GetTickCount() - lastKeydown > 200 || i != lastKey)
					{
						lastKeydown = GetTickCount();
						lastKey = i;
						text += (char)i;
					}

					break;
				}

		if (GetTickCount() - lastKeydown > 200)
		{
			if (KEYDOWN(VK_BACK) && strlen(text.c_str()) > 0)
			{
				lastKeydown = GetTickCount();
				text.erase(text.end() - 1);
			}
			else if (KEYDOWN(VK_OEM_PERIOD))
			{
				lastKeydown = GetTickCount();
				text += ".";
			}
			else if (KEYDOWN(VK_OEM_COMMA))
			{
				lastKeydown = GetTickCount();
				text += ",";
			}
		}

		//Implement any other keyboard input you would like here...
	}

	std::string GetText()
	{
		std::string str = text;
		if (blink && active)str += "_";
		return str;
	}

	RECT rect;
	bool active, blink;
	int maxLength;
	DWORD lastKeydown;
	int lastKey;
	float time;
	std::string text;
};

class LOBBY
{
public:
	LOBBY();
	~LOBBY();

	void Init(IDirect3DDevice9* Dev);
	void Update(float deltaTime, MOUSE& mouse);
	void Draw(MOUSE& mouse);

	void DrawTextbox(TEXTBOX& tb);
	bool TextButton(const char text[], RECT r, bool border, MOUSE& mouse);
	void Square(RECT r, DWORD col);

private:
	int m_room;
	IDirect3DTexture9* m_pBackGround;
	IDirect3DTexture9* m_pUI;
	ID3DXFont* m_pMiniText;
	ID3DXFont* m_pSmallText;
	ID3DXFont* m_pCaption;
	IDirect3DDevice9* m_pDevice;
	ID3DXSprite* m_pSprite;
	ID3DXLine* m_pLine;

	//Lobby Content
	TEXTBOX m_nameTB, m_sessionTB, m_sendTB;
	std::string m_error;
};