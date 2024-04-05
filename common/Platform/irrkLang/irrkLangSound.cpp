#include "../../Core/Log.h"
//#define IRRKLANG_STATIC
#include "irrkLangSound.h"
#include "../../Core/hash.h"


namespace irrkLangSound {
	irrkLangSoundPlayer::irrkLangSoundPlayer() {
		/*{
			ISoundDeviceList* devList = createSoundDeviceList();
			LOG_INFO("Enumerating irrkLang sound devices...");
			for (int i = 0; i < devList->getDeviceCount(); ++i) {
				LOG_INFO("Sound Device: {0}", devList->getDeviceDescription(i));
			}
			devList->drop();
		}*/
		SoundEngine = createIrrKlangDevice(ESOD_AUTO_DETECT);
		setMasterVolume(1.f);
	}

	irrkLangSoundPlayer::~irrkLangSoundPlayer() {
		SoundEngine->stopAllSounds();
		
		SoundEngine->drop();
	}

	size_t irrkLangSoundPlayer::loadSong(const char*soundFile,bool loop) {
		size_t hash = Core::HashFNV1A(soundFile, strlen(soundFile));

		ISoundSource* pSrc{ nullptr };
		pSrc = SoundEngine->addSoundSourceFromFile(soundFile);
		//auto fmt = src->getAudioFormat();
		//pSound = SoundEngine->play2D(soundFile, loop, true, true);
		sources[hash] = pSrc;
#ifdef _DEBUG
		soundNameMap[hash] = soundFile;
#endif
		return hash;
	}

	void irrkLangSoundPlayer::play(size_t soundid) {
		ISoundSource* pSrc = sources[soundid];
		SAudioStreamFormat fmt;
		if (pSrc) {
			fmt = pSrc->getAudioFormat();
		}
		ISound*pSound= SoundEngine->play2D(sources[soundid], false, false,true);
		
		auto receiver = new SoundEventReceiver();
		pSound->setSoundStopEventReceiver(receiver, this);
		sounds[soundid] = pSound;
#ifdef _DEBUG
		LOG_INFO("Playing sound {0}", soundNameMap[soundid].c_str());
		soundRevMap[pSound] = soundid;
#endif
		
	}

	void irrkLangSoundPlayer::stop(size_t soundid) {
		sounds[soundid]->setIsPaused(true);		
	}

	void irrkLangSoundPlayer::setVolume(size_t soundid, float volume) {
		sounds[soundid]->setVolume(std::min(1.f, std::max(0.f, volume)));
	}
	

	void irrkLangSoundPlayer::setBalance(size_t soundid,float balance) {
		float pan = std::min(1.f, std::max(-1.f, balance));//clamp between -1,1
		sounds[soundid]->setPan(pan);
	}
	bool irrkLangSoundPlayer::isPlaying(size_t soundid) {
		return !sounds[soundid]->getIsPaused();
	}
	void irrkLangSoundPlayer::setMasterVolume(float volume) {
		SoundEngine->setSoundVolume(std::min(1.f, std::max(0.f, volume)));				
	}

	float irrkLangSoundPlayer::getMasterVolume() {
		return SoundEngine->getSoundVolume();
	}


	void SoundEventReceiver::OnSoundStopped(ISound* pSound, E_STOP_EVENT_CAUSE reason, void* pData) {
		irrkLangSoundPlayer* pPlayer = (irrkLangSoundPlayer*)pData;
#ifdef _DEBUG
		size_t hash = pPlayer->soundRevMap[pSound];
		pSound->drop();
		pPlayer->sounds[hash] = nullptr;		
		LOG_INFO("Sound {0} stopped playing.",pPlayer->soundNameMap[hash]);
#endif
	}
}