#pragma once
//#define __USE__TRIPLE__BUFFERING__
#if defined __USE__TRIPLE__BUFFERING__
constexpr int MAX_FRAMES = 3;
#else 
constexpr int MAX_FRAMES = 2;
#endif
#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
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
constexpr float pi = glm::pi<float>();
constexpr float twopi = 2.f * glm::pi<float>();
constexpr float halfpi = 0.5f * glm::pi<float>();
constexpr float quaterpi = 0.25f * glm::pi<float>();
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
inline mat4 glPerspectiveLH(float fov, float width, float height, float zn, float zf) {
	const float rad = fov;
	const float h = glm::cos(static_cast<float>(0.5f) * rad) / glm::sin(static_cast<float>(0.5f) * rad);
	const float w = h * height / width; ///todo max(width , Height) / min(width , Height)?

	mat4 result = mat4(1.f);
	result[0][0] = w;
	result[1][1] = h;
	result[2][2] = (zf + zn) / (zf - zn);
	result[2][3] = static_cast<float>(1.f);
	result[3][2] = -(static_cast<float>(2.f) * zf * zn) / (zf - zn);
	return result;
}
inline mat4 glOrthoLH(float width, float height, float zn, float zf) {
	//glOrthoLH(-width,width,-height,height,zn,zf);
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / width;
	mat[1][1] = 2.f / height;
	mat[2][2] = 1.f / (zf - zn);
	mat[3][2] = -zn / (zf - zn);
	mat[2][2] = -2.f / (zf - zn);
	mat[3][2] = 2.f*zn / (zf - zn);	
	return mat;
}
inline mat4 glOrthoRH(float width, float height, float zn, float zf) {
	//glOrthoRH(-width,width,-height,height,zn,zf);
	mat4 mat = mat4(1.f);
	mat[0][0] = 2.f / width;
	mat[1][1] = 2.f / height;
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

inline vec3 glProject(vec3 const& obj, mat4 const& model, mat4 const& mvp, vec4 const& viewport)
{
	vec4 tmp = vec4(obj, 1.f);
	tmp = model * tmp;
	tmp = mvp * tmp;
	tmp.y *= -1;
	tmp /= tmp.w;
	tmp = tmp * 0.5f + 0.5f;
	tmp[0] = tmp[0] * (viewport[2]) + (viewport[0]);
	//tmp[1] = tmp[1] * (viewport[3]) + (viewport[1]);
	tmp[1] = tmp[1] * (viewport[3]) + (viewport[1]);
	/*float py = 2.f * tmp.y / (float)viewport.w - 1.f;
	py *= -1;
	tmp.y = viewport.w * (py + 1.f) * 0.5f;*/
	return vec3(tmp);
}
//////////////////////////////
//Vulkan projections
//////////////////////////////

inline mat4 vulkPerspectiveLH(float fov, float width, float height, float zn, float zf) {
	

	float rad = fov * 0.25f;
	const float h = 1 / glm::tan(rad);// glm::cos(rad) / glm::sin(rad);
	const float w = h * height / width; ///todo max(width , Height) / min(width , Height)?
	mat4 result=mat4(1.f);	
	result[0][0] = w;
	result[1][1] = -h;//flip y
	result[2][2] = zf / (zf - zn);
	result[2][3] = 1.f;
	result[3][2] = -zf * zn / (zf - zn);

	return result;
}

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
	mat[3][2] = -zn / (zf - zn);
	//mat[1][1] *= -1;//flip y for Vulkan
	return mat;
}

inline mat4 vulkOrthoLH(float left, float right, float top, float bottom, float zn, float zf) {
	mat4 mat = mat4(1.f);
	float height = bottom - top;
	float width = right - left;
	mat[0][0] = 2.f / (width);
	mat[1][1] = -2.f / (top - bottom);//flip y
	mat[2][2] = 1.f / (zf - zn);
	mat[3][0] = -(right + left) / (width);
	mat[3][1] = -(top + bottom) / (height);
	mat[3][2] = -zn / (zf - zn);
	//mat[1][1] *= -1;//flip y for Vulkan
	return mat;
}

inline mat4 vulkOrthoRH(float left, float right, float top, float bottom, float zn, float zf) {
	mat4 mat = mat4(1.f);
	float height = bottom - top;
	float width = right - left;
	mat[0][0] = 2.f / (width);
	mat[1][1] = -2.f / (top - bottom);//flip y
	mat[2][2] = -1.f / (zf - zn);
	mat[3][0] = -(right + left) / (width);
	mat[3][1] = -(top + bottom) / (height);
	mat[3][2] = -zn / (zf - zn);
	//mat[1][1] *= -1;//flip y for Vulkan
	return mat;
}

inline vec3 vulkProject(const vec3&obj, const mat4&model,const mat4&mvp,const vec4&viewport){
	vec4 tmp = vec4(obj, 1.f);
	tmp = model * tmp;
	tmp = mvp * tmp;

	tmp /= tmp.w;
	tmp.x = tmp.x * static_cast<float>(0.5f) + static_cast<float>(0.5f);
	tmp.y = tmp.y * static_cast<float>(0.5f) + static_cast<float>(0.5f);

	tmp[0] = tmp[0] * (viewport[2]) + (viewport[0]);
	tmp[1] = tmp[1] * (viewport[3]) + (viewport[1]);

	return vec3(tmp);
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

inline vec4 planeFromPointNormal(vec3 point, vec3 normal) {
	
	float dist = dot(point, normal);
	return vec4(normal, -dist);
}

inline bool planeIntersectLine(vec3& out, vec4 pp, vec3 pv1, vec3 pv2) {
	out = vec4(0.f);
	vec3 normal = vec3(pp);
	vec3 dir = pv2 - pv1;
	float d = dot(normal, dir);
	if (!d)
		return false;
	float temp = (pp.w + dot(normal, pv1)) / d;
	out = pv1 - dir * temp;
	return true;
}