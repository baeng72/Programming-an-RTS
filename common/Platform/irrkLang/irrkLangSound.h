#include "../../sound/sound.h"
#include <unordered_map>
#include <irrKlang-64bit-1.6.0/include/irrKlang.h>
using namespace irrklang;

namespace irrkLangSound {
	struct SoundEventReceiver : public ISoundStopEventReceiver {
		virtual void OnSoundStopped(ISound* pSound, E_STOP_EVENT_CAUSE reason, void* pData) override;
	};
	class irrkLangSoundPlayer : public Sound::SoundPlayer {
		friend SoundEventReceiver;
		ISoundEngine* SoundEngine{ nullptr };
		
		std::unordered_map<size_t, ISoundSource*> sources;
		std::unordered_map<size_t, ISound*> sounds;
#ifdef _DEBUG
		std::unordered_map<size_t, std::string> soundNameMap;
		std::unordered_map<ISound*, size_t> soundRevMap;
#endif
	public:
		irrkLangSoundPlayer();
		virtual ~irrkLangSoundPlayer();
		virtual size_t loadSong(const char*,bool loop=false) override;
		virtual void play(size_t soundid) override;
		virtual void stop(size_t soundid) override;
		virtual bool isPlaying(size_t soundid) override;
		virtual void setVolume(size_t soundid,float volume) override;
		virtual void setBalance(size_t soundid, float balance) override;
		virtual void setMasterVolume(float volume)override;
		virtual float getMasterVolume()override;
	};
	
}