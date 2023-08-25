#pragma once
#include "../Core/defines.h"
#include "RenderDevice.h"
#include "Mesh.h"

namespace Renderer {
	class Shape {
	public:
		static Shape* Create(RenderDevice* pdevice);
		virtual ~Shape() = default;
		virtual Mesh* CreateSphere(float radius, int32_t slices, int32_t stacks)=0;
		virtual Mesh* CreateCube(float side) = 0;
	};
}
