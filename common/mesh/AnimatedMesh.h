#pragma once
#include "../Renderer/RenderDevice.h"
//#include "../Renderer/Shader.h"

#include "../Renderer/Buffer.h"
#include "../Renderer/ShaderTypes.h"
#include "MeshTypes.h"
#include "BaseMesh.h"
#include "AnimationController.h"

namespace Mesh {
	//std::function<void(void*)> fnnewctrl;
	class AnimatedMesh : public BaseMesh {
	public:
		static AnimatedMesh* Create(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& attributes,Skeleton&skeleton, std::vector<AnimationClip>& animations);
		virtual ~AnimatedMesh() = default;
		virtual void Render(/*Renderer::Shader* pshader,*/ AnimationController* pcontroller) = 0;
		//virtual void UpdateShader(Renderer::Shader*pshader) = 0;
		virtual AnimationController* GetController() = 0;
		//virtual void SetAnimation(int anim, bool fade=true) = 0;
		//virtual void SetPose(float time,bool loop=true)=0;
		//virtual void GetPoseXForms(std::vector<mat4>& xforms) = 0;//before bind pose applied
		//virtual void GetPose(std::vector<mat4>& poseXForms) = 0;
		//virtual void GetBindPose(std::vector<mat4>& poseXForms) = 0;
		//virtual void GetAnimations(std::vector<AnimationClip>& animations) = 0;
		//virtual void GetBoneNames(std::vector<std::string>& boneNames) = 0;
		//virtual int GetBoneIndex(const char* boneName) = 0;
		//virtual int GetBonePoseXForm(int boneID, mat4& xform) = 0;
		virtual Renderer::Buffer* GetBoneBuffer()const =0;
		
	};
}

