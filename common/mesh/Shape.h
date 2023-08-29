#pragma once
#include "../Core/defines.h"
#include "../Renderer/RenderDevice.h"
#include "Mesh.h"

namespace Mesh {
	class Shape {
	public:
		static Shape* Create(Renderer::RenderDevice* pdevice);
		virtual ~Shape() = default;
		virtual Mesh* CreateSphere(float radius, int32_t slices, int32_t stacks)=0;
		virtual Mesh* CreateCube(float side) = 0;
	};
}
