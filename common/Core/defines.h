#pragma once
//#define __USE__TRIPLE__BUFFERING__
#if defined __USE__TRIPLE__BUFFERING__
constexpr int MAX_FRAMES = 3;
#else 
constexpr int MAX_FRAMES = 2;
#endif

//may want to use different linear algebra stuff?
#define __USE__GLM__ 
#if defined __USE__GLM__
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using quat = glm::quat;
using Color = glm::vec4;
using ivec4 = glm::ivec4;

#else

#endif
inline void decomposeMatrix(mat4& m, vec3& pos, quat& rot, vec3& scale) {
	mat4 mat = m;
	pos = vec3(mat[3][0], mat[3][1], mat[3][2]);
	mat[3] = vec4(0.f, 0.f, 0.f, 1.f);//zero out
	scale.x = glm::length(m[0]);
	scale.y = glm::length(m[1]);
	scale.z = glm::length(m[2]);
	mat[0][0] /= scale.x; mat[0][1] /= scale.y; mat[0][2] /= scale.z;
	mat[1][0] /= scale.x; mat[1][1] /= scale.y; mat[1][2] /= scale.z;
	mat[2][0] /= scale.x; mat[2][1] /= scale.y; mat[2][2] /= scale.z;
	rot = glm::quat(mat);
}
/// <summary>
/// GL Projections
/// </summary>
inline mat4 glOrthoLH(float width, float height, float zn, float zf) {
	//glOrthoLH(-width,width,-height,height,zn,zf);
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / width;
	mat[1][1] = 2.f / -height;
	mat[2][2] = -2.f / (zf - zn);
	mat[3][2] = -2.f*zn / (zf - zn);	
	return mat;
}
inline mat4 glOrthoRH(float width, float height, float zn, float zf) {
	//glOrthoRH(-width,width,-height,height,zn,zf);
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / width;
	mat[1][1] = 2.f / -height;
	mat[2][2] = -2.f / (zf - zn);
	mat[3][2] = -2.f * zn / (zf - zn);
	return mat;
}

inline mat4 glOrthoRH(float left, float right, float top, float bottom, float zn, float zf) {
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / (right - left);
	mat[1][1] = 2.f / (top - bottom);
	mat[2][2] = -2.f / (zf - zn);
	mat[3][0] = -(right + left) / (right - left);
	mat[3][1] = -(top + bottom) / (top - bottom);
	mat[3][2] = -(zf + zn) / (zf - zn);

	return mat;
}

inline mat4 glOrthoLH(float left, float right, float top, float bottom, float zn, float zf) {
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / (right - left);
	mat[1][1] = 2.f / (top - bottom);
	mat[2][2] = 2.f / (zf - zn);
	mat[3][0] = -(right + left) / (right - left);
	mat[3][1] = -(top + bottom) / (top - bottom);
	mat[3][2] = -(zf + zn) / (zf - zn);

	return mat;
}
//////////////////////////////
//Vulkan projections
//////////////////////////////
inline mat4 vulkOrthoLH(float width, float height, float zn, float zf) {
	//vulkOrthoLH(-width,width,-height,height,zn,zf);
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / width;
	mat[1][1] = -2.f / height;//flip y
	mat[2][2] = 1.f / (zf - zn);
	mat[3][2] = -zn / (zf - zn);
	//mat[1][1] *= -1;//flip y for Vulkan
	return mat;
}

inline mat4 vulkOrthoRH(float width, float height, float zn, float zf) {
	//vulkOrthoRH(-width,width,-height,height,zn,zf);
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / width;
	mat[1][1] = -2.f / height;//flip y
	mat[2][2] = -1.f / (zf - zn);
	mat[3][2] = zn / (zf - zn);
	//mat[1][1] *= -1;//flip y for Vulkan
	return mat;
}

inline mat4 vulkOrthoRH(float left, float right, float top, float bottom, float zn, float zf) {
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / (right - left);
	mat[1][1] = -2.f / (bottom-top);//flip y
	mat[2][2] = -1.f / (zf - zn);
	mat[3][0] = -(right + left) / (right - left);
	mat[3][1] = -(bottom+top) / (bottom-top);
	mat[3][2] = -zn / (zf - zn);
	//mat[1][1] *= -1;//flip y for Vulkan
	return mat;
}

inline mat4 vulkOrthoLH(float left, float right, float top, float bottom, float zn, float zf) {
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / (right - left);
	mat[1][1] = -2.f / (bottom - top);//flip y
	mat[2][2] = 1.f / (zf - zn);
	mat[3][0] = -(right + left) / (right - left);
	mat[3][1] = -(top + bottom) / (top - bottom);
	mat[3][2] = -zn / (zf - zn);
	//mat[1][1] *= -1;//flip y for Vulkan
	return mat;
}

#ifdef _WIN32

inline mat4 dxOrthoLH(float width, float height, float zn, float zf) {	
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / width;
	mat[1][1] = 2.f / height;
	mat[2][2] = 1.f / (zf - zn);
	mat[3][2] = -zn / (zf - zn);
	return mat;		
}

inline mat4 dxOrthoRH(float width, float height, float zn, float zf) {
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / width;
	mat[1][1] = 2.f / height;
	mat[2][2] = -1.f / (zf - zn);
	mat[3][2] = zn / (zf - zn);
	return mat;
}

inline mat4 dxOrthoRH(float left, float right, float top, float bottom, float zn, float zf) {
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / (right - left);
	mat[1][1] = 2.f / (top - bottom);
	mat[2][2] = -1.f / (zf - zn);
	mat[3][0] = -(right + left) / (right - left);
	mat[3][1] = -(top + bottom) / (top - bottom);
	mat[3][2] = -zn / (zf - zn);	
	return mat;
}

inline mat4 dxOrthoLH(float left, float right, float top, float bottom, float zn, float zf) {
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / (right - left);
	mat[1][1] = 2.f / (top - bottom);
	mat[2][2] = 1.f / (zf - zn);
	mat[3][0] = -(right + left) / (right - left);
	mat[3][1] = -(top + bottom) / (top - bottom);
	mat[3][2] = -zn / (zf - zn);	
	return mat;
}
#endif