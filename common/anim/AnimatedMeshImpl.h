#pragma once
#include <vector>
#include "../mesh/AnimatedMesh.h"
#include "../Platform/Vulkan/VulkanEx.h"
namespace Animation {
	
	class AnimatedMeshImpl : public Mesh::AnimatedMesh {
		Renderer::RenderDevice* _pdevice;
		std::vector<Mesh::AnimationClip> _animations;
		/*struct AnimationData {
			std::vector<float> times;
			std::vector<std::vector<vec3>> positions;
			std::vector<std::vector<quat>> rotations;
			std::vector<std::vector<vec3>> scales;
		};
		
		
		std::vector<AnimationData> _animationFrames;*/
		Mesh::Skeleton _skeleton;
		std::vector<mat4> _matrixPalette;
		/*std::vector<float> _clipTimes;
		std::vector<mat4> _poseXForms;
		std::vector<mat4> _matrixPalette;
		
		int _currClip;
		int _clipIndex;
		int _blendClip;
		int _blendIndex;
		float _blendPcnt;
		float _clipLength;
		float _clipTime;
		float _lastClipTime;
		float _blendLength;
		float _blendTime;
		float GetClipLength();*/
		Vulkan::Buffer _vertexBuffer;
		Vulkan::Buffer _indexBuffer;
		std::unique_ptr<Renderer::Buffer> _boneBuffer;
		uint32_t		_indexCount;
		uint32_t		_boneCount;
		uint32_t		_skeletonsAllocated;
		mat4* _bonePtrBase;
		//std::unordered_map<size_t,uint32_t> _boneOffsetMap;
		int				_controllerCount;
		void AllocateBoneBuffer(uint32_t count);
		void Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize);
	public:
		AnimatedMeshImpl(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t vertStride, uint32_t* pindices, uint32_t indSize,Mesh::Skeleton&skeleton, std::vector<Mesh::AnimationClip>& animations);
		virtual ~AnimatedMeshImpl();
		virtual void Render(/*Renderer::Shader* pshader,*/ Mesh::AnimationController* pcontroller)override;
		//virtual void UpdateShader(Renderer::Shader* pshader) override;
		virtual Mesh::AnimationController* GetController() override;
		//virtual void SetAnimation(int anim, bool fade=true)override;
		//virtual void SetPose(float time, bool loop = true)override;
		//virtual void GetPoseXForms(std::vector<mat4>& xforms)override;//before bind pose applied
		//virtual void GetPose(std::vector<mat4>& poseXForms)override;
		//virtual void GetBindPose(std::vector<mat4>& poseXForms) override;
		//virtual void GetAnimations(std::vector<Mesh::AnimationClip>& animations)override;
		//virtual void GetBoneNames(std::vector<std::string>& boneNames)override;
		//virtual int GetBoneIndex(const char* boneName) override;
		//virtual int GetBonePoseXForm(int boneID, mat4& xform) override;
		virtual Renderer::Buffer* GetBoneBuffer()const override;
	};
}

