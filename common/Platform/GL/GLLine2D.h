#include "../../Renderer/Line2D.h"
#include "ShaderUtil.h"
namespace GL {
	
		class GLLine2D : public Renderer::Line2D {
			Renderer::RenderDevice* _pdevice;
			ShaderUtil _shader;
			
			mat4 projection;
			vec2 start;
			vec2 end;
			vec4 color;
			GLuint _vao;
			int _width;
			int _height;
		public:
			GLLine2D(Renderer::RenderDevice* pdevice);
			virtual ~GLLine2D();
			virtual void Draw(vec2* pvertices, uint32_t count, vec4 color, float width = 1) override;
			virtual void Update(int width, int height)override;
		};
	
}