#include "../../common.h"

namespace Utils {
	class ShapeUtils : public Mesh::Shape {
		Renderer::RenderDevice* _pdevice;
	public:
		ShapeUtils(Renderer::RenderDevice* pdevice);
		virtual ~ShapeUtils();
		virtual Mesh::Mesh* CreateSphere(float radius, int32_t slices, int32_t stacks) override;
		virtual Mesh::Mesh* CreateCube(float side) override;
	};
}