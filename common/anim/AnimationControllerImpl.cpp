#include "AnimationControllerImpl.h"
#include "../Core/profiler.h"
namespace Mesh {
	AnimationController* AnimationController::Create(std::vector<AnimationClip>& clips, Skeleton& skeleton,int id) {
		return new Animation::AnimationControllerImpl(clips, skeleton,id);
	}
}

namespace Animation {
	
	AnimationControllerImpl::AnimationControllerImpl(std::vector<Mesh::AnimationClip>& animations, Mesh::Skeleton& skeleton,int id)
		: _skeleton(skeleton), _animations(animations),_id(id)
	{
		_boneCount = _skeleton.boneHierarchy.size();
		//avoid repeated reallocation/freeing
		_matrixPalette.resize(_boneCount);
		_poseXForms.resize(_boneCount);
		samplepos.resize(_boneCount);
		samplerot.resize(_boneCount);
		samplesca.resize(_boneCount);
		_blendClip = -1;
		_blendPcnt = 0.f;
		_currClip = -1;
		//_clipTime = 0.f;
		_clipIndex = 0;
		_matrixPalette = skeleton.bonePoseMatrices;
		_poseXForms = skeleton.bonePoseMatrices;
		_animationFrames.resize(_animations.size());

		for (size_t a = 0; a < _animations.size(); ++a) {
			auto& ani = _animations[a];
			auto& dstani = _animationFrames[a];

			for (size_t c = 0; c < ani.channels.size(); ++c) {
				auto& channel = ani.channels[c];

				assert(channel.positionTimes.size() == channel.rotationTimes.size());
				assert(channel.positionTimes.size() == channel.scaleTimes.size());
				for (size_t t = 0; t < channel.positions.size(); ++t) {
					float time = channel.positionTimes[t];
					auto it = std::find(dstani.times.begin(), dstani.times.end(), time);
					size_t index = 0;
					if (it == dstani.times.end()) {
						dstani.times.push_back(time);
						it = std::find(dstani.times.begin(), dstani.times.end(), time);
					}


					index = (size_t)std::distance(dstani.times.begin(), it);
					vec3 pos = channel.positions[t];
					if (dstani.positions.size() < (size_t)(index + 1))
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

		}
	}
	AnimationControllerImpl::~AnimationControllerImpl()
	{
	}
	
	void AnimationControllerImpl::SetAnimation(int anim, bool fade)
	{
			if (_currClip == anim)
			return;
		if (fade) {
			_blendClip = _currClip;	
			_blendIndex = _clipIndex;
			_blendTime = _clipTime;
			_blendLength = _clipLength;
		}
		else {
			_blendClip = -1;			
		}
		_blendPcnt = 0.f;
		_currClip = anim;
		_clipTime = 0.f;
		_clipIndex = 0;
		_clipLength = GetClipLength();
		
	}
	void AnimationControllerImpl::Advance(float time,bool loop) {
		_clipTime += time * _animations[_currClip].ticksPerSecond;
		if (_clipTime > _clipLength) {
			if (loop) {
				_clipTime = fmodf(_clipTime, _clipLength);
				_clipIndex = 0;
			}
			else
				_clipTime = _clipLength;
		}
		if (_blendClip >= 0) {
			if (_blendTime > _blendLength) {
				_blendTime = fmodf(_blendTime, _blendLength);
				_blendIndex = 0;
			}
			else
				_blendTime = _blendLength;
		}
		{
			//auto& ani = _animations[_currClip];
			auto& ani = _animationFrames[_currClip];
			//auto& timestamps = ani.channels[0].positionTimes;
			auto& timestamps = ani.times;
			if (_clipIndex == 22) {
				int z = 0;
			}
			while (_clipIndex + 1 < timestamps.size() && _clipTime > timestamps[_clipIndex + 1])
				_clipIndex++;

			size_t nextClipIndex = _clipIndex + 1;
			/*if (_clipIndex + 1 >= timestamps.size()) {
				nextClipIndex = _clipIndex + 1;
				_clipIndex = 0;
				_clipTime -= _clipLength;

			}*/

			size_t boneCount = _skeleton.boneHierarchy.size();
			float t = ((_clipTime - timestamps[_clipIndex]) / (timestamps[nextClipIndex] - timestamps[_clipIndex]));
			auto& pos1 = ani.positions[_clipIndex];
			auto& pos2 = ani.positions[_clipIndex + 1];
			for (size_t i = 0; i < boneCount; ++i) {
				samplepos[i] = mix(pos1[i], pos2[i], t);
			}
			auto& rot1 = ani.rotations[_clipIndex];
			auto& rot2 = ani.rotations[_clipIndex + 1];
			for (size_t i = 0; i < boneCount; ++i) {
				samplerot[i] = slerp(rot1[i], rot2[i], t);
			}
			auto& sca1 = ani.scales[_clipIndex];
			auto& sca2 = ani.scales[_clipIndex + 1];
			for (size_t i = 0; i < boneCount; i++) {
				samplesca[i] = mix(sca1[i], sca2[i], t);
			}

			/*for (size_t i = 0; i < ani.channels.size(); ++i) {
				auto& channel = ani.channels[i];
				auto& positions = channel.positions;
				auto& rotations = channel.rotations;
				auto& scales = channel.scales;
				vec3 pos = mix(positions[_clipIndex], positions[nextClipIndex], t);
				quat rota = rotations[_clipIndex];
				quat rotb = rotations[nextClipIndex];
				if (dot(rota, rotb) < 0.f) {
					rotb = -rotb;
				}
				quat rot = slerp(rota, rotb, t);
				vec3 sca = mix(scales[_clipIndex], scales[nextClipIndex], t);
				samplepos[i] = pos;
				samplerot[i] = rot;
				samplesca[i] = sca;

			}*/
		}
		if (_blendClip >= 0) {
#ifdef __PROFILE__
			EASY_BLOCK("Calculate blend animation");
#endif
			auto& ani = _animations[_blendClip];
			auto& timestamps = ani.channels[0].positionTimes;
			while (_blendIndex + 1 < timestamps.size() && _blendTime > timestamps[_blendIndex])
				_blendIndex++;
			if (_blendIndex + 1 >= timestamps.size())
				_blendIndex = 0;

			float t = (_blendTime - timestamps[_blendIndex] / (timestamps[_blendIndex + 1] - timestamps[_blendIndex]));

			for (size_t i = 0; i < ani.channels.size(); ++i) {
				auto& channel = ani.channels[i];
				auto& positions = channel.positions;
				auto& rotations = channel.rotations;
				auto& scales = channel.scales;
				vec3 pos = mix(positions[_blendIndex], positions[_blendIndex + 1], t);
				quat rota = rotations[_blendIndex];
				quat rotb = rotations[_blendIndex + 1];
				if (dot(rota, rotb) < 0.f) {
					rotb = -rotb;
				}
				quat rot = slerp(rota, rotb, t);
				vec3 sca = mix(positions[_blendIndex], positions[_blendIndex + 1], t);
				//interpolate or transform?
				samplepos[i] = mix(samplepos[i], pos, _blendPcnt);
				samplerot[i] = slerp(samplerot[i], rot, _blendPcnt);
				samplesca[i] = mix(samplesca[i], sca, _blendPcnt);

			}
			_blendPcnt += 0.1f;//just do 10 steps?
			if (_blendPcnt >= 1.f)
				_blendClip = -1;//finish blending
		}
		{

#ifdef __PROFILE__
			EASY_BLOCK("Combine animation");
#endif
			auto& boneHierarchy = _skeleton.boneHierarchy;
			for (int i = 1; i < _boneCount; i++) {
				int parentID = boneHierarchy[i];
				vec3 sca = samplesca[parentID] * samplesca[i];
				quat rot = samplerot[parentID] * samplerot[i];
				vec3 pos = samplerot[parentID] * (samplesca[parentID] * samplepos[i]);
				pos += samplepos[parentID];
				samplepos[i] = pos;
				samplerot[i] = rot;
				samplesca[i] = sca;
			}
		}
		//we now should have our transforms ready
		{

			auto& invBindMatrices = _skeleton.boneInvBindMatrices;

			{
#ifdef __PROFILE__
				EASY_BLOCK("Make xform matrices");
#endif
				for (int i = 0; i < _boneCount; i++) {
					//rotation

					vec3 x = samplerot[i] * vec3(1.f, 0.f, 0.f);
					vec3 y = samplerot[i] * vec3(0.f, 1.f, 0.f);
					vec3 z = samplerot[i] * vec3(0.f, 0.f, 1.f);
					//scale
					x *= samplesca[i].x;
					y *= samplesca[i].y;
					z *= samplesca[i].z;
					mat4 xform = mat4(
						x.x, x.y, x.z, 0.f,
						y.x, y.y, y.z, 0.f,
						z.x, z.y, z.z, 0.f,
						samplepos[i].x, samplepos[i].y, samplepos[i].z, 1.f
					);
					_poseXForms[i] = xform;


				}
			}
			{
				union Pack {
					__m128 m[4];
					mat4	v;
				};
#ifdef __PROFILE__
				EASY_BLOCK("Multiply by invbind matrices");
#endif
				for (int i = 0; i < _boneCount; i++) {
#if 1
					__m128 a;
					__m128 b;
					mat4 bind = invBindMatrices[i];
					Pack x;
					Pack out;
					x.v = _poseXForms[i];
					//col 0
					a = _mm_mul_ps(x.m[0], _mm_set_ps1(bind[0].x));
					b = _mm_mul_ps(x.m[1], _mm_set_ps1(bind[0].y));
					out.m[0] = _mm_add_ps(a, b);
					a = _mm_mul_ps(x.m[2], _mm_set_ps1(bind[0].z));
					out.m[0] = _mm_add_ps(out.m[0], a);
					a = _mm_mul_ps(x.m[3], _mm_set_ps1(bind[0].w));
					out.m[0] = _mm_add_ps(out.m[0], a);
					//col 1
					a = _mm_mul_ps(x.m[0], _mm_set_ps1(bind[1].x));
					b = _mm_mul_ps(x.m[1], _mm_set_ps1(bind[1].y));
					out.m[1] = _mm_add_ps(a, b);
					a = _mm_mul_ps(x.m[2], _mm_set_ps1(bind[1].z));
					out.m[1] = _mm_add_ps(out.m[1], a);
					a = _mm_mul_ps(x.m[3], _mm_set_ps1(bind[1].w));
					out.m[1] = _mm_add_ps(out.m[1], a);
					//col 2
					a = _mm_mul_ps(x.m[0], _mm_set_ps1(bind[2].x));
					b = _mm_mul_ps(x.m[1], _mm_set_ps1(bind[2].y));
					out.m[2] = _mm_add_ps(a, b);
					a = _mm_mul_ps(x.m[2], _mm_set_ps1(bind[2].z));
					out.m[2] = _mm_add_ps(out.m[2], a);
					a = _mm_mul_ps(x.m[3], _mm_set_ps1(bind[2].w));
					out.m[2] = _mm_add_ps(out.m[2], a);
					//col 3
					a = _mm_mul_ps(x.m[0], _mm_set_ps1(bind[3].x));
					b = _mm_mul_ps(x.m[1], _mm_set_ps1(bind[3].y));
					out.m[3] = _mm_add_ps(a, b);
					a = _mm_mul_ps(x.m[2], _mm_set_ps1(bind[3].z));
					out.m[3] = _mm_add_ps(out.m[3], a);
					a = _mm_mul_ps(x.m[3], _mm_set_ps1(bind[3].w));
					out.m[3] = _mm_add_ps(out.m[3], a);


					_matrixPalette[i] = out.v;
#else
					_matrixPalette[i] = _poseXForms[i] * invBindMatrices[i];
#endif
				}
			}
		}
	}
	void AnimationControllerImpl::SetPose(float time, bool loop)
	{
#ifdef __PROFILE__
		EASY_FUNCTION(profiler::colors::Red)
#endif
			//sample current animation
		//_lastClipTime = _clipTime;
		float clipTime = time * _animations[_currClip].ticksPerSecond;
		
		float currClipTime = clipTime;
		float blendClipTime = clipTime;

		{
#ifdef __PROFILE__
			EASY_BLOCK("Calc anim time");
#endif
			if (loop) {
				
				if(currClipTime > _clipLength) {
					currClipTime =fmodf(currClipTime,_clipLength);
					_clipIndex = 0;
				}
				if (_blendClip >= 0) {
					if(blendClipTime > _blendLength) {
						blendClipTime =fmodf(blendClipTime,_blendLength);
						_blendIndex = 0;
					}
				}
			}
			else {
				if (currClipTime > _clipLength)
					currClipTime = _clipLength;
				if (_blendClip >= 0) {
					if (blendClipTime > _blendLength)
						blendClipTime = _blendLength;
				}

			}
		}
		/*size_t boneCount = _skeleton.boneHierarchy.size();
		if (_matrixPalette.size() != boneCount)
			_matrixPalette.resize(boneCount);
		if (_poseXForms.size() != boneCount)
			_poseXForms.resize(boneCount);
		std::vector<vec3> samplepos(boneCount);
		std::vector<quat> samplerot(boneCount);
		std::vector<vec3> samplesca(boneCount);*/
		{
#ifdef __PROFILE__
			EASY_BLOCK("Calculate Animation");
#endif

			{
				//auto& ani = _animations[_currClip];
				auto& ani = _animationFrames[_currClip];
				//auto& timestamps = ani.channels[0].positionTimes;
				auto& timestamps = ani.times;
				while (_clipIndex + 1 < timestamps.size() && currClipTime > timestamps[_clipIndex + 1])
					_clipIndex++;

				size_t nextClipIndex = _clipIndex + 1;
				if (_clipIndex + 1 >= timestamps.size()) {
					nextClipIndex = _clipIndex + 1;
					_clipIndex = 0;
					currClipTime -= _clipLength;

				}

				size_t boneCount = _skeleton.boneHierarchy.size();
				float t = ((currClipTime - timestamps[_clipIndex]) / (timestamps[nextClipIndex] - timestamps[_clipIndex]));
				auto& pos1 = ani.positions[_clipIndex];
				auto& pos2 = ani.positions[_clipIndex + 1];
				for (size_t i = 0; i < boneCount; ++i) {
					samplepos[i] = mix(pos1[i], pos2[i], t);
				}
				auto& rot1 = ani.rotations[_clipIndex];
				auto& rot2 = ani.rotations[_clipIndex + 1];
				for (size_t i = 0; i < boneCount; ++i) {
					samplerot[i] = slerp(rot1[i], rot2[i], t);
				}
				auto& sca1 = ani.scales[_clipIndex];
				auto& sca2 = ani.scales[_clipIndex + 1];
				for (size_t i = 0; i < boneCount; i++) {
					samplesca[i] = mix(sca1[i], sca2[i], t);
				}

				/*for (size_t i = 0; i < ani.channels.size(); ++i) {
					auto& channel = ani.channels[i];
					auto& positions = channel.positions;
					auto& rotations = channel.rotations;
					auto& scales = channel.scales;
					vec3 pos = mix(positions[_clipIndex], positions[nextClipIndex], t);
					quat rota = rotations[_clipIndex];
					quat rotb = rotations[nextClipIndex];
					if (dot(rota, rotb) < 0.f) {
						rotb = -rotb;
					}
					quat rot = slerp(rota, rotb, t);
					vec3 sca = mix(scales[_clipIndex], scales[nextClipIndex], t);
					samplepos[i] = pos;
					samplerot[i] = rot;
					samplesca[i] = sca;

				}*/
			}
		}

		if (_blendClip>=0) {
#ifdef __PROFILE__
			EASY_BLOCK("Calculate blend animation");
#endif
			auto& ani = _animations[_blendClip];
			auto& timestamps = ani.channels[0].positionTimes;
			while (_blendIndex + 1 < timestamps.size() && blendClipTime > timestamps[_blendIndex])
				_blendIndex++;
			if (_blendIndex + 1 >= timestamps.size())
				_blendIndex = 0;

			float t = (blendClipTime - timestamps[_blendIndex] / (timestamps[_blendIndex+1] - timestamps[_blendIndex]));
			
			for (size_t i = 0; i < ani.channels.size();++i) {
				auto& channel = ani.channels[i];
				auto& positions = channel.positions;
				auto& rotations = channel.rotations;
				auto& scales = channel.scales;
				vec3 pos = mix(positions[_blendIndex], positions[_blendIndex+1], t);
				quat rota = rotations[_blendIndex];
				quat rotb = rotations[_blendIndex + 1];
				if (dot(rota, rotb) < 0.f) {
					rotb = -rotb;
				}
				quat rot = slerp(rota,rotb, t);
				vec3 sca = mix(positions[_blendIndex], positions[_blendIndex + 1], t);
				//interpolate or transform?
				samplepos[i] = mix(samplepos[i], pos,_blendPcnt);
				samplerot[i] = slerp(samplerot[i],rot,_blendPcnt);
				samplesca[i] = mix(samplesca[i],sca,_blendPcnt);
				
			}
			_blendPcnt += 0.1f;//just do 10 steps?
			if (_blendPcnt >= 1.f)
				_blendClip = -1;//finish blending
		}
		//we have our local sampled data for this time.
		//now we need to do some muliplications....
		{

#ifdef __PROFILE__
			EASY_BLOCK("Combine animation");
#endif
			auto& boneHierarchy = _skeleton.boneHierarchy;
			for (int i = 1; i < _boneCount; i++) {
				int parentID = boneHierarchy[i];
				vec3 sca = samplesca[parentID] * samplesca[i];
				quat rot = samplerot[parentID] * samplerot[i];
				vec3 pos = samplerot[parentID] * (samplesca[parentID] * samplepos[i]);
				pos += samplepos[parentID];
				samplepos[i] = pos;
				samplerot[i] = rot;
				samplesca[i] = sca;
			}
		}
		//we now should have our transforms ready
		{
			
			auto& invBindMatrices = _skeleton.boneInvBindMatrices;
			
			{
#ifdef __PROFILE__
				EASY_BLOCK("Make xform matrices");
#endif
				for (int i = 0; i < _boneCount; i++) {
					//rotation

					vec3 x = samplerot[i] * vec3(1.f, 0.f, 0.f);
					vec3 y = samplerot[i] * vec3(0.f, 1.f, 0.f);
					vec3 z = samplerot[i] * vec3(0.f, 0.f, 1.f);
					//scale
					x *= samplesca[i].x;
					y *= samplesca[i].y;
					z *= samplesca[i].z;
					mat4 xform = mat4(
						x.x, x.y, x.z, 0.f,
						y.x, y.y, y.z, 0.f,
						z.x, z.y, z.z, 0.f,
						samplepos[i].x, samplepos[i].y, samplepos[i].z, 1.f
					);
					_poseXForms[i] = xform;


				}
			}
			{
				union Pack {
					__m128 m[4];
					mat4	v;
				};
#ifdef __PROFILE__
				EASY_BLOCK("Multiply by invbind matrices");
#endif
				for (int i = 0; i < _boneCount; i++) {
#if 1
					__m128 a;
					__m128 b;
					mat4 bind = invBindMatrices[i];
					Pack x;					
					Pack out;
					x.v = _poseXForms[i];
					//col 0
					a = _mm_mul_ps(x.m[0], _mm_set_ps1(bind[0].x));
					b = _mm_mul_ps(x.m[1], _mm_set_ps1(bind[0].y));
					out.m[0] = _mm_add_ps(a, b);
					a = _mm_mul_ps(x.m[2], _mm_set_ps1(bind[0].z));
					out.m[0] = _mm_add_ps(out.m[0], a);
					a = _mm_mul_ps(x.m[3], _mm_set_ps1(bind[0].w));
					out.m[0] = _mm_add_ps(out.m[0], a);
					//col 1
					a = _mm_mul_ps(x.m[0], _mm_set_ps1(bind[1].x));
					b = _mm_mul_ps(x.m[1], _mm_set_ps1(bind[1].y));
					out.m[1] = _mm_add_ps(a, b);
					a = _mm_mul_ps(x.m[2], _mm_set_ps1(bind[1].z));
					out.m[1] = _mm_add_ps(out.m[1], a);
					a = _mm_mul_ps(x.m[3], _mm_set_ps1(bind[1].w));
					out.m[1] = _mm_add_ps(out.m[1], a);
					//col 2
					a = _mm_mul_ps(x.m[0], _mm_set_ps1(bind[2].x));
					b = _mm_mul_ps(x.m[1], _mm_set_ps1(bind[2].y));
					out.m[2] = _mm_add_ps(a, b);
					a = _mm_mul_ps(x.m[2], _mm_set_ps1(bind[2].z));
					out.m[2] = _mm_add_ps(out.m[2], a);
					a = _mm_mul_ps(x.m[3], _mm_set_ps1(bind[2].w));
					out.m[2] = _mm_add_ps(out.m[2], a);
					//col 3
					a = _mm_mul_ps(x.m[0], _mm_set_ps1(bind[3].x));
					b = _mm_mul_ps(x.m[1], _mm_set_ps1(bind[3].y));
					out.m[3] = _mm_add_ps(a, b);
					a = _mm_mul_ps(x.m[2], _mm_set_ps1(bind[3].z));
					out.m[3] = _mm_add_ps(out.m[3], a);
					a = _mm_mul_ps(x.m[3], _mm_set_ps1(bind[3].w));
					out.m[3] = _mm_add_ps(out.m[3], a);
					

					_matrixPalette[i] = out.v;
#else
					_matrixPalette[i] = _poseXForms[i] * invBindMatrices[i];
#endif
				}
			}
		}

	}
	void AnimationControllerImpl::GetPoseXForms(std::vector<mat4>& poseXForms)
	{
		poseXForms = _poseXForms;
	}
	void AnimationControllerImpl::GetPose(std::vector<mat4>& poseFinalXForms)
	{
		poseFinalXForms = _matrixPalette;
	}
	void AnimationControllerImpl::GetPose(mat4* pxforms, uint32_t size) {
		uint32_t sz = std::min(size, (uint32_t)(_matrixPalette.size() * sizeof(mat4)));
		memcpy(pxforms, _matrixPalette.data(), sz);
	}
	void AnimationControllerImpl::GetBindPose(std::vector<mat4>& poseXForms)
	{
			poseXForms.resize(_skeleton.boneInvBindMatrices.size());
		poseXForms[0] = _skeleton.boneInvBindMatrices[0];
		for (size_t i = 1; i < _skeleton.boneInvBindMatrices.size(); i++) {
			poseXForms[i] = inverse(_skeleton.boneInvBindMatrices[i]);
		}
		//for (size_t i = 1; i < _skeleton.boneInvBindMatrices.size(); ++i) {
		//	//undo combined world to bone xform
		//	int parentID = _skeleton.boneHierarchy[i];
		//	poseXForms[i] = poseXForms[i]*inverse(poseXForms[parentID]);
		//}
		
		/*for (size_t i = poseXForms.size()-1; i > 0; i--) {
			int parentID = _skeleton.boneHierarchy[i];
			poseXForms[i] = poseXForms[parentID]* inverse(_skeleton.boneInvBindMatrices[i]);

		}*/
		//poseXForms[0] = inverse(poseXForms[0]);
		/*for (size_t i = 0; i < _skeleton.boneInvBindMatrices.size(); i++) {
			poseXForms[i] = inverse(_skeleton.boneInvBindMatrices[i]);
		}
		for (size_t i = 1; i < poseXForms.size(); i++) {
			int parentID = _skeleton.boneHierarchy[i];
			mat4 parent = poseXForms[parentID];
			mat4 xform = poseXForms[i];
			poseXForms[i] =  xform*inverse(parent);
		}*/
	}
	void AnimationControllerImpl::GetAnimations(std::vector<Mesh::AnimationClip>& animations)
	{
		animations = _animations;
	}
	int AnimationControllerImpl::GetBoneCount()const {
		return (int)_skeleton.boneNames.size();
	}
	void AnimationControllerImpl::GetBoneNames(std::vector<std::string>& boneNames)
	{
		boneNames = _skeleton.boneNames;
	}
	int AnimationControllerImpl::GetBoneIndex(const char* boneName)
	{
			auto it = std::find(_skeleton.boneNames.begin(), _skeleton.boneNames.end(), boneName);
		if (it != _skeleton.boneNames.end())
			return (int)std::distance(_skeleton.boneNames.begin(), it);
		return -1;
	}
	int AnimationControllerImpl::GetBonePoseXForm(int boneID, mat4& xform)
	{
			if (boneID >= 0 && boneID < _poseXForms.size()) {
			xform = _poseXForms[boneID];
			return boneID;
		}
		xform = glm::mat4(1.f);
		return -1;
	}

	float AnimationControllerImpl::GetClipLength()
	{
			float length = 0.f;
		auto& ani = _animations[_currClip];
		for (auto& channel : ani.channels) {
			length = std::max(channel.positionTimes[channel.positionTimes.size() - 1], length);
			length = std::max(channel.rotationTimes[channel.rotationTimes.size() - 1], length);
			length = std::max(channel.scaleTimes[channel.scaleTimes.size() - 1], length);
		}
		return length;
	}
}