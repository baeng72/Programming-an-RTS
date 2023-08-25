#include "../../common.h"

namespace Utils {
	class ShapeUtils : public Renderer::Shape {
		Renderer::RenderDevice* _pdevice;
	public:
		ShapeUtils(Renderer::RenderDevice* pdevice);
		virtual ~ShapeUtils();
		virtual Renderer::Mesh* CreateSphere(float radius, int32_t slices, int32_t stacks) override;
		virtual Renderer::Mesh* CreateCube(float side) override;
	};
}