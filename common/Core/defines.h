#pragma once

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
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using Color = glm::vec4;

#else
#include <cmath>
struct vec2 {
	float x, y;
	vec2(float x_, float y_) :x(x_), y(y_) {}
	vec2() :x(0.f), y(0.f) {}
};
float distance(vec2& lhs, vec2& rhs) { return std::sqrtf((rhs.x - lhs.x) * (rhs.x - lhs.x) + (rhs.y - lhs.y) * (rhs.y - lhs.y)); }
#endif

