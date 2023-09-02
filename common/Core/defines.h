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
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/quaternion.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using quat = glm::quat;
using Color = glm::vec4;

#else

#endif


