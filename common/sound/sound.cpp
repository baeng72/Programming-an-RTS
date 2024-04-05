#include "sound.h"
#include "../Core/Log.h"
#include "../Platform/irrkLang/irrkLangSound.h"
namespace Sound {
	SoundPlayer* SoundPlayer::Create() {
		return new irrkLangSound::irrkLangSoundPlayer();
	}
}