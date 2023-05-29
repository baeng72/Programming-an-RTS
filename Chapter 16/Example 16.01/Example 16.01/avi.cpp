#include "avi.h"		

AVI::AVI()
{
	AVIFileInit();
	m_pCurrentFrame = NULL;

	m_timeMS = 0;
	m_playing = m_done = false;
	m_activeStream = 0;
	m_lastFrame = -1;
	m_pAviFile = NULL;
}

AVI::~AVI()
{
	AVIFileExit();
	Release();
}

HRESULT AVI::Load(const char fileName[], IDirect3DDevice9* Dev)
{
	try
	{
		m_pDevice = Dev;

		if (AVIFileOpen(&m_pAviFile, fileName, OF_READ, NULL) != 0)
			return E_FAIL;

		//Retreive Audio streams
		long streamNo = 0;
		do
		{
			AUDIO_STREAM* as = new AUDIO_STREAM();

			//Get new m_pStream
			if (AVIFileGetStream(m_pAviFile, &as->m_pStream, streamtypeAUDIO, streamNo++))
			{
				delete as;
				break;
			}

			//Add m_pStream
			m_audio.push_back(as);
		} while (true);

		streamNo = 0;
		do
		{
			VIDEO_STREAM* vs = new VIDEO_STREAM();

			//Get m_video m_pStream
			if (AVIFileGetStream(m_pAviFile, &vs->m_pStream, streamtypeVIDEO, streamNo++))
			{
				delete vs;
				break;
			}

			//GetFrame object
			vs->m_pGetFrame = AVIStreamGetFrameOpen(vs->m_pStream, NULL);

			//Get runtime
			vs->m_runTime = AVIStreamEndTime(vs->m_pStream);

			//Add m_pStream
			m_video.push_back(vs);
		} while (true);

		return S_OK;
	}
	catch (...)
	{
		debug.Print("Error in AVI::Load()");
		return E_FAIL;
	}
}

void AVI::Release()
{
	for (int i = 0; i < m_video.size(); i++)
		if (m_video[i] != NULL)
			delete m_video[i];
	m_video.clear();

	for (int i = 0; i < m_audio.size(); i++)
		if (m_audio[i] != NULL)
			delete m_audio[i];
	m_audio.clear();

	if (m_pCurrentFrame != NULL)
		m_pCurrentFrame->Release();
	m_pCurrentFrame = NULL;

	AVIFileRelease(m_pAviFile);
}

void AVI::Play()
{
	m_timeMS = 0;
	m_playing = true;
	m_done = false;
	m_activeStream = 0;
	m_lastFrame = -1;
}

void AVI::Stop()
{
	m_playing = false;
}

bool AVI::Done()
{
	return m_done;
}

void AVI::Update(float deltaTime)
{
	if (m_playing)m_timeMS += deltaTime * 1000;
	else return;

	long frame;
	if (m_timeMS <= m_video[m_activeStream]->m_runTime)
	{
		frame = AVIStreamTimeToSample(m_video[m_activeStream]->m_pStream, m_timeMS);

		//Retrieve new frame
		if (frame != m_lastFrame && frame != -1)
		{
			m_lastFrame = frame;

			BITMAPINFOHEADER* bip = NULL;
			bip = (BITMAPINFOHEADER*)AVIStreamGetFrame(m_video[m_activeStream]->m_pGetFrame, frame);

			if (bip != NULL)
			{
				if (m_pCurrentFrame != NULL)
					m_pCurrentFrame->Release();

				D3DXCreateTextureFromFileInMemoryEx(m_pDevice, bip, bip->biSize + bip->biWidth * bip->biHeight * bip->biBitCount / 8,
					bip->biWidth, bip->biHeight,
					1, D3DUSAGE_DYNAMIC, D3DFMT_R8G8B8, D3DPOOL_DEFAULT,
					D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &m_pCurrentFrame);
			}
		}
	}
	else
	{
		m_activeStream++;
		m_timeMS = 0;

		//No more streams to play
		if (m_activeStream >= m_video.size())
		{
			Stop();
			m_done = true;
			m_activeStream = 0;
		}
	}
}