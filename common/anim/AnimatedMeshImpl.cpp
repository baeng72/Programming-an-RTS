#pragma once
#include "AnimatedMeshImpl.h"

#include "../Platform/Vulkan/VulkState.h"
#include "../Platform/Vulkan/VulkSwapchain.h"

namespace Mesh {
	AnimatedMesh* AnimatedMesh::Create(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t vertStride, uint32_t* pindices, uint32_t indSize, Skeleton& skeleton, std::vector<AnimationClip>& animations) {
		return new Animation::AnimatedMeshImpl(pdevice, pvertices, vertSize, vertStride, pindices, indSize, skeleton, animations);
	}
}

namespace Animation {
	
	AnimatedMeshImpl::AnimatedMeshImpl(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t vertStride, uint32_t* pindices, uint32_t indSize, Mesh::Skeleton& skeleton, std::vector<Mesh::AnimationClip>& animations)
		:_pdevice(pdevice),_skeleton(skeleton),_animations(animations),_boneCount((uint32_t)skeleton.boneHierarchy.size()){
		_skeletonsAllocated = 0u;
		_controllerCount = 0;
		/*_blendClip = -1;
		_blendPcnt = 0.f;
		_currClip = -1;
		_clipTime = 0.f;
		_clipIndex = 0;
		_matrixPalette = skeleton.bonePoseMatrices;
		_poseXForms = skeleton.bonePoseMatrices;
		_animationFrames.resize(_animations.size());
		
		for (size_t a = 0; a < _animations.size();++a) {
			auto& ani = _animations[a];
			auto& dstani = _animationFrames[a];
			
			for (size_t c = 0; c < ani.channels.size();++c) {
				auto& channel = ani.channels[c];
				
				assert(channel.positionTimes.size() == channel.rotationTimes.size());
				assert(channel.positionTimes.size()== channel.scaleTimes.size());
				for (size_t t = 0; t < channel.positions.size(); ++t) {
					float time = channel.positionTimes[t];
					auto it = std::find(dstani.times.begin(), dstani.times.end(), time);
					size_t index = 0;
					if (it == dstani.times.end()) {
						dstani.times.push_back(time);
						it = std::find(dstani.times.begin(), dstani.times.end(), time);
					}
					
					
					index =(size_t)std::distance(dstani.times.begin(), it);
					vec3 pos = channel.positions[t];
					if (dstani.positions.size() < (size_t)( index + 1))
						dstani.positions.push_back(std::vector<vec3>(ani.channels.size(), vec3(1.f)));
					dstani.positions[index][c] = pos;
				}
				for (size_t t = 0; t < channel.rotations.size(); ++t) {
					float time = channel.rotationTimes[t];
					auto it = std::find(dstani.times.begin(), dstani.times.end(), time);
					size_t index = 0;
					if (it == dstani.times.end()) {
						dstani.times.push_back(time);
						it = std::find(dstani.times.begin(), dstani.times.end(), time);
					}
					
					index = (size_t)std::distance(dstani.times.begin(), it);
					quat rot = channel.rotations[t];
					if (dstani.rotations.size() < (size_t)(index + 1))
						dstani.rotations.push_back(std::vector<quat>(ani.channels.size()));
					dstani.rotations[index][c] = rot;

				}
				for (size_t t = 0; t < channel.scales.size(); ++t) {
					float time = channel.scaleTimes[t];
					auto it = std::find(dstani.times.begin(), dstani.times.end(), time);
					size_t index = 0;
					if (it == dstani.times.end()) {
						dstani.times.push_back(time);
						it = std::find(dstani.times.begin(), dstani.times.end(), time);
					}
					index = std::distance(dstani.times.begin(), it);
						
					vec3 sca = channel.scales[t];
					if (dstani.scales.size() < (size_t)(index + 1))
						dstani.scales.push_back(std::vector<vec3>(ani.channels.size(), vec3(1.f)));
					dstani.scales[index][c] = sca;
				}
			}
			
		}*/
		_matrixPalette.resize(_skeleton.boneHierarchy.size());//reserve space for bone hierarchy
		AllocateBoneBuffer(4);
		
		Create(pvertices, vertSize, pindices, indSize);
		
	}
	
	AnimatedMeshImpl::~AnimatedMeshImpl() {
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;

		Vulkan::cleanupBuffer(context.device, _vertexBuffer);
		Vulkan::cleanupBuffer(context.device, _indexBuffer);
	}
	void AnimatedMeshImpl::Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize) {
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		{


			std::vector<uint32_t> vertexLocations;
			Vulkan::VertexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddVertices(vertSize, pvertices)
				.build(_vertexBuffer, vertexLocations);
		}
		{
			std::vector<uint32_t> indexLocations;
			Vulkan::IndexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddIndices(indSize, pindices)
				.build(_indexBuffer, indexLocations);
			_indexCount = indSize / sizeof(uint32_t);
		}
	}

	Mesh::AnimationController* AnimatedMeshImpl::GetController()
	{
		//AllocateBoneBuffer((uint32_t)(_boneOffsetMap.size() + 1));
		AllocateBoneBuffer((uint32_t)(_controllerCount + 1));
		//int index = (int)_boneOffsetMap.size();
		auto pcontroller= Mesh::AnimationController::Create(_animations, _skeleton,_controllerCount++);
		size_t ctrlhash = reinterpret_cast<size_t>(pcontroller);
		
		//_boneOffsetMap[ctrlhash] = index;
		return pcontroller;
	}

	Renderer::Buffer* AnimatedMeshImpl::GetBoneBuffer() const
	{
		return _boneBuffer.get();
	}
	
	void AnimatedMeshImpl::Render(/*Renderer::Shader* pshader,*/ Mesh::AnimationController* pcontroller)
	{
		uint32_t boneCount = _boneCount;
		uint32_t skeletonSize = sizeof(mat4) * boneCount;
		uint32_t offset = pcontroller->GetControllerOffset();// _boneOffsetMap[reinterpret_cast<size_t>(pcontroller)];
		mat4* ppalette = &_bonePtrBase[offset];
		pcontroller->GetPose(ppalette,skeletonSize);
		//uint32_t dynoffsets[1] = { offset*skeletonSize};
		//pshader->Bind(dynoffsets,1);
		Vulkan::VulkFrameData* pframedata = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& frameData = *pframedata;
		vkCmdBindIndexBuffer(frameData.cmd, _indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(frameData.cmd, 0, 1, &_vertexBuffer.buffer, offsets);
		vkCmdDrawIndexed(frameData.cmd, _indexCount, 1, 0, 0, 0);
	}

	/*void AnimatedMeshImpl::UpdateShader(Renderer::Shader* pshader)
	{
		pshader->SetStorageBuffer("skeleton", _boneBuffer.get(), true);
	}*/

	void AnimatedMeshImpl::AllocateBoneBuffer(uint32_t count) {
		constexpr int buffer_block = 4;
		if (count >= _skeletonsAllocated) {
			count = (count + buffer_block-1) & ~(buffer_block-1);
			VkDeviceSize size = sizeof(mat4) * std::max(_boneCount,20u);//initialial allocation
			_boneBuffer.reset(Renderer::Buffer::Create(_pdevice, (uint32_t)size,count,false, true));
			_bonePtrBase = (mat4*)_boneBuffer->MapPtr();//may need to change for OpenGL
			_skeletonsAllocated = (uint32_t)count;
		}
	}
	//void AnimatedMeshImpl::SetAnimation(int anim, bool fade)
	//{
	//	if (_currClip == anim)
	//		return;
	//	if (fade) {
	//		_blendClip = _currClip;	
	//		_blendIndex = _clipIndex;
	//		_blendTime = _clipTime;
	//		_blendLength = _clipLength;
	//	}
	//	else {
	//		_blendClip = -1;			
	//	}
	//	_blendPcnt = 0.f;
	//	_currClip = anim;
	//	_clipTime = 0.f;
	//	_clipIndex = 0;
	//	_clipLength = GetClipLength();
	//	
	//}
	//void AnimatedMeshImpl::SetPose(float time, bool loop)
	//{
	//	//sample current animation
	//	_lastClipTime = _clipTime;
	//	_clipTime += time * _animations[_currClip].ticksPerSecond;
	//	if (loop) {
	//		while (_clipTime > _clipLength) {
	//			_clipTime -= _clipLength;
	//			_clipIndex = 0;
	//		}
	//		if (_blendClip >= 0) {
	//			while (_blendTime > _blendLength) {
	//				_blendTime -= _blendLength;
	//				_blendIndex = 0;
	//			}
	//		}
	//	}
	//	else {
	//		if (_clipTime > _clipLength)
	//			_clipTime = _clipLength;
	//		if (_blendClip >= 0) {
	//			if (_blendTime > _blendLength)
	//				_blendTime = _blendLength;
	//		}
	//			
	//	}

	//	size_t boneCount = _skeleton.boneHierarchy.size();
	//	if (_matrixPalette.size() != boneCount)
	//		_matrixPalette.resize(boneCount);
	//	if (_poseXForms.size() != boneCount)
	//		_poseXForms.resize(boneCount);
	//	std::vector<vec3> samplepos(boneCount);
	//	std::vector<quat> samplerot(boneCount);
	//	std::vector<vec3> samplesca(boneCount);
	//	
	//	{
	//		//auto& ani = _animations[_currClip];
	//		auto& ani = _animationFrames[_currClip];
	//		//auto& timestamps = ani.channels[0].positionTimes;
	//		auto& timestamps = ani.times;
	//		while (_clipIndex + 1 < timestamps.size() && _clipTime > timestamps[_clipIndex+1])
	//			_clipIndex++;
	//		
	//		int nextClipIndex = _clipIndex + 1;
	//		if (_clipIndex + 1 >= timestamps.size()) {
	//			nextClipIndex = _clipIndex + 1;
	//			_clipIndex = 0;
	//			_clipTime -= _clipLength;
	//			
	//		}
	//		
	//		size_t boneCount = _skeleton.boneHierarchy.size();
	//		float t = ((_clipTime - timestamps[_clipIndex]) / (timestamps[nextClipIndex] - timestamps[_clipIndex]));
	//		auto& pos1 = ani.positions[_clipIndex];
	//		auto& pos2 = ani.positions[_clipIndex + 1];
	//		for (size_t i = 0; i < boneCount; ++i) {
	//			samplepos[i] = mix(pos1[i], pos2[i], t);
	//		}
	//		auto& rot1 = ani.rotations[_clipIndex];
	//		auto& rot2 = ani.rotations[_clipIndex + 1];
	//		for (size_t i = 0; i < boneCount; ++i) {
	//			samplerot[i] = slerp(rot1[i], rot2[i], t);
	//		}
	//		auto& sca1 = ani.scales[_clipIndex];
	//		auto& sca2 = ani.scales[_clipIndex + 1];
	//		for (size_t i = 0; i < boneCount; i++) {
	//			samplesca[i] = mix(sca1[i], sca2[i], t);
	//		}
	//		
	//		/*for (size_t i = 0; i < ani.channels.size(); ++i) {
	//			auto& channel = ani.channels[i];
	//			auto& positions = channel.positions;
	//			auto& rotations = channel.rotations;
	//			auto& scales = channel.scales;
	//			vec3 pos = mix(positions[_clipIndex], positions[nextClipIndex], t);
	//			quat rota = rotations[_clipIndex];
	//			quat rotb = rotations[nextClipIndex];
	//			if (dot(rota, rotb) < 0.f) {
	//				rotb = -rotb;
	//			}
	//			quat rot = slerp(rota, rotb, t);
	//			vec3 sca = mix(scales[_clipIndex], scales[nextClipIndex], t);
	//			samplepos[i] = pos;
	//			samplerot[i] = rot;
	//			samplesca[i] = sca;
	//			
	//		}*/
	//	}
	//	if (_blendClip>=0) {
	//		auto& ani = _animations[_blendClip];
	//		auto& timestamps = ani.channels[0].positionTimes;
	//		while (_blendIndex + 1 < timestamps.size() && _blendTime > timestamps[_blendIndex])
	//			_blendIndex++;
	//		if (_blendIndex + 1 >= timestamps.size())
	//			_blendIndex = 0;

	//		float t = (_blendTime - timestamps[_blendIndex] / (timestamps[_blendIndex+1] - timestamps[_blendIndex]));
	//		
	//		for (size_t i = 0; i < ani.channels.size();++i) {
	//			auto& channel = ani.channels[i];
	//			auto& positions = channel.positions;
	//			auto& rotations = channel.rotations;
	//			auto& scales = channel.scales;
	//			vec3 pos = mix(positions[_blendIndex], positions[_blendIndex+1], t);
	//			quat rota = rotations[_blendIndex];
	//			quat rotb = rotations[_blendIndex + 1];
	//			if (dot(rota, rotb) < 0.f) {
	//				rotb = -rotb;
	//			}
	//			quat rot = slerp(rota,rotb, t);
	//			vec3 sca = mix(positions[_blendIndex], positions[_blendIndex + 1], t);
	//			//interpolate or transform?
	//			samplepos[i] = mix(samplepos[i], pos,_blendPcnt);
	//			samplerot[i] = slerp(samplerot[i],rot,_blendPcnt);
	//			samplesca[i] = mix(samplesca[i],sca,_blendPcnt);
	//			
	//		}
	//		_blendPcnt += 0.1f;//just do 10 steps?
	//		if (_blendPcnt >= 1.f)
	//			_blendClip = -1;//finish blending
	//	}
	//	//we have our local sampled data for this time.
	//	//now we need to do some muliplications....
	//	auto& boneHierarchy = _skeleton.boneHierarchy;
	//	for (int i = 1; i < boneCount; i++) {
	//		int parentID = boneHierarchy[i];
	//		vec3 sca = samplesca[parentID] * samplesca[i];
	//		quat rot = samplerot[parentID] * samplerot[i];
	//		vec3 pos = samplerot[parentID] * (samplesca[parentID] * samplepos[i]);
	//		pos += samplepos[parentID];
	//		samplepos[i] = pos;
	//		samplerot[i] = rot;
	//		samplesca[i] = sca;
	//	}

	//	//we now should have our transforms ready
	//	auto& invBindMatrices = _skeleton.boneInvBindMatrices;
	//	for (int i = 0; i < boneCount; i++) {
	//		//rotation
	//		vec3 x = samplerot[i] * vec3(1.f, 0.f, 0.f);
	//		vec3 y = samplerot[i] * vec3(0.f, 1.f, 0.f);
	//		vec3 z = samplerot[i] * vec3(0.f, 0.f, 1.f);
	//		//scale
	//		x *= samplesca[i].x;
	//		y *= samplesca[i].y;
	//		z *= samplesca[i].z;
	//		mat4 xform = mat4(
	//			x.x, x.y, x.z, 0.f,
	//			y.x, y.y, y.z, 0.f,
	//			z.x, z.y, z.z, 0.f,
	//			samplepos[i].x,samplepos[i].y,samplepos[i].z, 1.f
	//		);
	//		_poseXForms[i] = xform;
	//		_matrixPalette[i] = xform * invBindMatrices[i];
	//		
	//	}


	//}
	//void AnimatedMeshImpl::GetPoseXForms(std::vector<mat4>& poseXForms) {
	//	poseXForms = _poseXForms;
	//}
	//void AnimatedMeshImpl::GetBoneNames(std::vector<std::string>& boneNames)
	//{
	//	boneNames = _skeleton.boneNames;
	//}
	//int AnimatedMeshImpl::GetBoneIndex(const char* boneName)
	//{
	//	auto it = std::find(_skeleton.boneNames.begin(), _skeleton.boneNames.end(), boneName);
	//	if (it != _skeleton.boneNames.end())
	//		return (int)std::distance(_skeleton.boneNames.begin(), it);
	//	return -1;
	//}
	//int AnimatedMeshImpl::GetBonePoseXForm(int boneID, mat4& xform)
	//{
	//	if (boneID >= 0 && boneID < _poseXForms.size()) {
	//		xform = _poseXForms[boneID];
	//		return boneID;
	//	}
	//	xform = glm::mat4(1.f);
	//	return -1;
	//}
	//void AnimatedMeshImpl::GetPose(std::vector<mat4>& pose)
	//{
	//	pose = _matrixPalette;
	//}
	//void AnimatedMeshImpl::GetBindPose(std::vector<mat4>& poseXForms)
	//{
	//	poseXForms.resize(_skeleton.boneInvBindMatrices.size());
	//	poseXForms[0] = _skeleton.boneInvBindMatrices[0];
	//	for (size_t i = 1; i < _skeleton.boneInvBindMatrices.size(); i++) {
	//		poseXForms[i] = inverse(_skeleton.boneInvBindMatrices[i]);
	//	}
	//	//for (size_t i = 1; i < _skeleton.boneInvBindMatrices.size(); ++i) {
	//	//	//undo combined world to bone xform
	//	//	int parentID = _skeleton.boneHierarchy[i];
	//	//	poseXForms[i] = poseXForms[i]*inverse(poseXForms[parentID]);
	//	//}
	//	
	//	/*for (size_t i = poseXForms.size()-1; i > 0; i--) {
	//		int parentID = _skeleton.boneHierarchy[i];
	//		poseXForms[i] = poseXForms[parentID]* inverse(_skeleton.boneInvBindMatrices[i]);

	//	}*/
	//	//poseXForms[0] = inverse(poseXForms[0]);
	//	/*for (size_t i = 0; i < _skeleton.boneInvBindMatrices.size(); i++) {
	//		poseXForms[i] = inverse(_skeleton.boneInvBindMatrices[i]);
	//	}
	//	for (size_t i = 1; i < poseXForms.size(); i++) {
	//		int parentID = _skeleton.boneHierarchy[i];
	//		mat4 parent = poseXForms[parentID];
	//		mat4 xform = poseXForms[i];
	//		poseXForms[i] =  xform*inverse(parent);
	//	}*/
	//	
	//}
	//void AnimatedMeshImpl::GetAnimations(std::vector<Mesh::AnimationClip>& animations)
	//{
	//	animations = _animations;
	//}

	//float AnimatedMeshImpl::GetClipLength()
	//{
	//	float length = 0.f;
	//	auto& ani = _animations[_currClip];
	//	for (auto& channel : ani.channels) {
	//		length = std::max(channel.positionTimes[channel.positionTimes.size() - 1], length);
	//		length = std::max(channel.rotationTimes[channel.rotationTimes.size() - 1], length);
	//		length = std::max(channel.scaleTimes[channel.scaleTimes.size() - 1], length);
	//	}
	//	return length;
	//}
}