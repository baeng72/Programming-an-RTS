#pragma once
#include <common.h>
#include "intpoint.h"
INTPOINT GetScreenPos(vec3 pos, mat4& matVP, vec4& viewport);
INTPOINT GetScreenPos(vec3 pos,mat4&matProj,mat4&matView,vec4&viewport);

