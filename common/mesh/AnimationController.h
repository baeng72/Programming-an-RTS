#pragma once

#include "MeshTypes.h"

namespace Mesh {
	class AnimationController {
	public:
		static AnimationController* Create(std::vector<AnimationClip>& clips, Skeleton& skeleton,int id);
		virtual ~AnimationController() = default;
		
		virtual void SetAnimation(int anim, bool fade = true) = 0;
		virtual void Advance(float time, bool loop = true) = 0;
		virtual void SetPose(float time, bool loop = true) = 0;
		virtual void GetPoseXForms(std::vector<mat4>& xforms) = 0;//before bind pose applied
		virtual void GetPose(std::vector<mat4>& poseXForms) = 0;
		virtual void GetPose(mat4* pxforms, uint32_t size) = 0;
		virtual void GetBindPose(std::vector<mat4>& poseXForms) = 0;
		virtual void GetAnimations(std::vector<AnimationClip>& animations) = 0;
		virtual int GetBoneCount()const = 0;
		virtual void GetBoneNames(std::vector<std::string>& boneNames) = 0;
		virtual int GetBoneIndex(const char* boneName) = 0;
		virtual int GetBonePoseXForm(int boneID, mat4& xform) = 0;
		virtual int GetControllerID() = 0;
		virtual int GetControllerOffset()const = 0;
	};
}