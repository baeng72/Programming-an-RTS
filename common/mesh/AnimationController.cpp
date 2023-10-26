#include "AnimationController.h"
#include "AnimationControllerImpl.h"
namespace Mesh {
	AnimationController* AnimationController::Create(std::vector<AnimationClip>& clips, Skeleton& skeleton, int id) {		
			return new AnimationControllerImpl(clips, skeleton, id);
	
		
	}
}