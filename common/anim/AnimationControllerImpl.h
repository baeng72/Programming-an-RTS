#pragma once
#include "../mesh/AnimationController.h"

namespace Animation {
	class AnimationControllerImpl : public Mesh::AnimationController {
		std::vector<Mesh::AnimationClip> _animations;
		Mesh::Skeleton _skeleton;
		struct AnimationData {
			std::vector<float> times;
			std::vector<std::vector<vec3>> positions;
			std::vector<std::vector<quat>> rotations;
			std::vector<std::vector<vec3>> scales;
		};


		std::vector<AnimationData> _animationFrames;
		std::vector<float> _clipTimes;
		std::vector<mat4> _poseXForms;
		std::vector<mat4> _matrixPalette;

		int _currClip;
		size_t _clipIndex;
		int _blendClip;
		size_t _blendIndex;
		float _blendPcnt;
		float _clipLength;
		//float _clipTime;
		//float _lastClipTime;
		float _blendLength;
		//float _blendTime;
		float GetClipLength();
	public:
		AnimationControllerImpl(std::vector<Mesh::AnimationClip>& clips, Mesh::Skeleton& skeleton);
		~AnimationControllerImpl();
		
		virtual void SetAnimation(int anim, bool fade = true) override;
		virtual void SetPose(float time, bool loop = true) override;
		virtual void GetPoseXForms(std::vector<mat4>& xforms) override;//before bind pose applied
		virtual void GetPose(std::vector<mat4>& poseXForms) override;
		virtual void GetPose(mat4* pxforms, uint32_t size) override;
		virtual void GetBindPose(std::vector<mat4>& poseXForms) override;
		virtual void GetAnimations(std::vector<Mesh::AnimationClip>& animations) override;
		virtual int GetBoneCount()const override;
		virtual void GetBoneNames(std::vector<std::string>& boneNames) override;
		virtual int GetBoneIndex(const char* boneName) override;
		virtual int GetBonePoseXForm(int boneID, mat4& xform) override;
	};
}