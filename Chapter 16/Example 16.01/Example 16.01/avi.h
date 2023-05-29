#ifndef _CJ_AVI_
#define _CJ_AVI_

#include <d3dx9.h>
#include <vfw.h>
#include <vector>
#include "debug.h"
#include "sound.h"

struct AUDIO_STREAM {
	AUDIO_STREAM()
	{
		m_pStream = NULL;
		m_pData = NULL;
	}

	~AUDIO_STREAM()
	{
		if (m_pStream)AVIStreamRelease(m_pStream);
		if (m_pData)delete[] m_pData;
	}

	PAVISTREAM m_pStream;
	BYTE* m_pData;
};

struct VIDEO_STREAM {
	VIDEO_STREAM()
	{
		m_pStream = NULL;
		m_pGetFrame = NULL;
	}

	~VIDEO_STREAM()
	{
		if (m_pGetFrame)AVIStreamGetFrameClose(m_pGetFrame);
		if (m_pStream)AVIStreamRelease(m_pStream);
	}

	PAVISTREAM m_pStream;
	PGETFRAME m_pGetFrame;
	long m_runTime;
};

class AVI {
public:
	AVI();
	~AVI();

	HRESULT Load(const char fileName[], IDirect3DDevice9* Dev);
	void Release();

	void Play();
	void Stop();
	bool Done();
	void Update(float deltaTime);

	IDirect3DTexture9* m_pCurrentFrame;

private:

	IDirect3DDevice9* m_pDevice;
	IAVIFile* m_pAviFile;

	std::vector<AUDIO_STREAM*> m_audio;
	std::vector<VIDEO_STREAM*> m_video;

	long m_timeMS;
	bool m_playing, m_done;
	int m_activeStream, m_lastFrame;
};

#endif