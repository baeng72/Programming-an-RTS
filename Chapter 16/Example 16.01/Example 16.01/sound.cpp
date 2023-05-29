#include "sound.h"
#include <cassert>

//////////////////////////////////////////////////////////////
//					SOUNDFILE								//
//////////////////////////////////////////////////////////////

SOUNDFILE::SOUNDFILE()
{
	m_pSegment = NULL;
}

SOUNDFILE::~SOUNDFILE()
{
	if (m_pSegment)
		m_pSegment->Release();
	m_pSegment = NULL;
}

void SOUNDFILE::Load(WCHAR fileName[], SOUND& sound)
{

	//Create new segment 
	CoCreateInstance(CLSID_DirectMusicSegment, NULL,
		CLSCTX_INPROC, IID_IDirectMusicSegment8,
		(void**)&m_pSegment);

	//Load from file using the loader
	sound.m_pLoader->LoadObjectFromFile(CLSID_DirectMusicSegment,
		IID_IDirectMusicSegment8,
		fileName, (void**)&m_pSegment);

	//Download sound to the performance interface
	m_pSegment->Download(sound.m_pPerformance);
}

//////////////////////////////////////////////////////////////
//					SOUND									//
//////////////////////////////////////////////////////////////

SOUND::SOUND()
{
	m_pPerformance = NULL;
	m_pLoader = NULL;
}

SOUND::~SOUND()
{
	//Delete sound files
	for (int i = 0; i < m_sounds.size(); i++)
		if (m_sounds[i] != NULL)
			delete m_sounds[i];
	m_sounds.clear();

	//Release the loader and the performance
	if (m_pLoader)
		m_pLoader->Release();
	m_pLoader = NULL;

	if (m_pPerformance)
	{
		m_pPerformance->CloseDown();
		m_pPerformance->Release();
	}
	m_pPerformance = NULL;
}

void SOUND::Init(HWND windowHandle)
{
	CoInitialize(NULL);

	HRESULT hr;
	//Create performance object

	CoCreateInstance(CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC,
		IID_IDirectMusicPerformance8, (void**)&m_pPerformance);
	//Create loader
	CoCreateInstance(CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC,
		IID_IDirectMusicLoader8, (void**)&m_pLoader);


	//Initialize the performance object
	m_pPerformance->InitAudio(NULL, NULL, windowHandle,
		DMUS_APATH_SHARED_STEREOPLUSREVERB,
		64, DMUS_AUDIOF_ALL, NULL);

	//Load sound files
	std::vector<const WCHAR*> fileNames;

	fileNames.push_back(L"sounds/sound1.wav");
	fileNames.push_back(L"sounds/sound2.wav");
	fileNames.push_back(L"sounds/sound3.wav");

	for (int i = 0; i < fileNames.size(); i++)
	{
		SOUNDFILE* snd = new SOUNDFILE();
		snd->Load((WCHAR*)fileNames[i], *this);
		m_sounds.push_back(snd);
	}

	SetMasterVolume(1.0f);
}

void SOUND::PlaySound(int soundID, bool loop)
{
	//Faulty Sound ID
	if (soundID < 0 || soundID >= m_sounds.size())return;

	//Loop sound or not
	if (loop)
		m_sounds[soundID]->m_pSegment->SetRepeats(DMUS_SEG_REPEAT_INFINITE);
	else m_sounds[soundID]->m_pSegment->SetRepeats(0);

	//Play Sound
	m_pPerformance->PlaySegment(m_sounds[soundID]->m_pSegment, DMUS_SEGF_SECONDARY, 0, NULL);
}

void SOUND::SetMasterVolume(float volume)
{
	//Cap volume to the range [0.0, 1.0]
	if (volume < 0.0f)volume = 0.0f;
	if (volume > 1.0f)volume = 1.0f;
	m_masterVolume = volume;

	//Translate to the decibel range
	long vol = 1000 - 5000 * (1.0f - sqrt(volume));
	//IID guid_PerfMasterVolume;
	//CLSIDFromString(L"{D2AC28B1-B39B-11D1-8704-00600893B1BD}", &guid_PerfMasterVolume);
	//Set master volume
	if (m_pPerformance)
		m_pPerformance->SetGlobalParam(GUID_PerfMasterVolume, (void*)&vol, sizeof(long));
	//m_pPerformance->SetGlobalParam(guid_PerfMasterVolume, (void*)&vol, sizeof(long));
}


//////////////////////////////////////////////////////////////
//					MP3PLAYER								//
//////////////////////////////////////////////////////////////

MP3::MP3()
{
	m_pGraphBuilder = NULL;
	m_pMediaControl = NULL;
	m_pMediaPosition = NULL;
	m_pBasicAudio = NULL;
}

MP3::~MP3()
{
	Release();
}

void MP3::Release()
{
	if (m_pBasicAudio)m_pBasicAudio->Release();
	if (m_pMediaPosition)m_pMediaPosition->Release();
	if (m_pMediaControl)m_pMediaControl->Release();
	if (m_pGraphBuilder)m_pGraphBuilder->Release();

	m_pGraphBuilder = NULL;
	m_pMediaControl = NULL;
	m_pMediaPosition = NULL;
	m_pBasicAudio = NULL;
}

void MP3::Init()
{
	Release();

	// Initialise COM
	CoInitialize(NULL);

	// Create the Filter Graph Manager
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
		IID_IGraphBuilder, (void**)&m_pGraphBuilder);

	m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void**)&m_pMediaControl);
	m_pGraphBuilder->QueryInterface(IID_IMediaPosition, (void**)&m_pMediaPosition);
	m_pGraphBuilder->QueryInterface(IID_IBasicAudio, (void**)&m_pBasicAudio);
}

void MP3::LoadSong(const WCHAR fName[])
{
	//Init the DirectShow objects
	Init();

	//Create standard graph
	m_pGraphBuilder->RenderFile(fName, NULL);
}

void MP3::Play()
{
	//rewind...
	m_pMediaPosition->put_CurrentPosition(0);

	//Play
	m_pMediaControl->Run();
}

void MP3::Stop()
{
	m_pMediaControl->Stop();
}

bool MP3::IsPlaying()
{
	REFTIME currentPos;
	REFTIME duration;

	m_pMediaPosition->get_CurrentPosition(&currentPos);
	m_pMediaPosition->get_Duration(&duration);

	if (currentPos < duration)
		return true;
	else return false;
}

void MP3::SetVolume(float volume)
{
	if (volume < 0.0f)volume = 0.0f;
	if (volume > 1.0f)volume = 1.0f;

	if (m_pBasicAudio)
	{
		long vol = -10000 * (1.0f - sqrt(volume));
		m_pBasicAudio->put_Volume(vol);
	}
}

void MP3::SetBalance(float balance)
{
	if (balance < -1.0f)balance = -1.0f;
	if (balance > 1.0f)balance = 1.0f;

	if (m_pBasicAudio)
	{
		float bal = 10000 * balance;
		m_pBasicAudio->put_Balance(bal);
	}
}