#pragma once

namespace Sound {
	class SoundPlayer {
	public:
		static SoundPlayer* Create();
		virtual ~SoundPlayer()=default;
		virtual size_t loadSong(const char*,bool loop=false)=0;
		virtual void play(size_t soundid)=0;
		virtual void stop(size_t soundid)=0;
		virtual bool isPlaying(size_t soundid)=0;
		virtual void setVolume(size_t soundid, float volume)=0;
		virtual void setBalance(size_t soundid,float balance)=0;
		virtual void setMasterVolume(float volume) = 0;
		virtual float getMasterVolume() = 0;
	};
}