#include "GLRenderDevice.h"
#include "GLERR.h"
namespace GL {
	
	GLRenderDevice::GLRenderDevice(void* nativeWindowHandle)
	{
		_window = reinterpret_cast<GLFWwindow*>(nativeWindowHandle);
	}
	GLRenderDevice::~GLRenderDevice()
	{
	}
	void GLRenderDevice::Init()
	{
		for (int i = 0; i < 3; i++) {
			_clearColors[i] = 0.f;
		}
		_clearColors[3] = 1.f;
		if (_depthTest) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);
		}
	}
	void GLRenderDevice::StartRender()
	{
		//EASY_FUNCTION(profiler::colors::Cyan);
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);
		
		GLERR();
		glViewport(0, 0, width, height);
		GLERR();
		glClearColor(_clearColors[0], _clearColors[1], _clearColors[2], _clearColors[3]);
		GLERR();
		GLuint flags = GL_COLOR_BUFFER_BIT;
		if (_depthTest)
			flags |= GL_DEPTH_BUFFER_BIT;
		glClear(flags);
		GLERR();
		
	}
	void GLRenderDevice::EndRender()
	{
		//EASY_FUNCTION(profiler::colors::Red);
		glfwSwapBuffers(_window);
	}
	void GLRenderDevice::StartShadowRender()
	{
	}
	void GLRenderDevice::EndShadowRender()
	{
	}
	void GLRenderDevice::SetVSync(bool vsync)
	{
		glfwSwapInterval(vsync ? 1 : 0);
	}
	void GLRenderDevice::EnableGeometry(bool geom)
	{
	}
	void GLRenderDevice::EnableWireframe(bool wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, wireframe? GL_LINE : GL_FILL);
	}
	void GLRenderDevice::EnableLines(bool lines)
	{
	}
	void GLRenderDevice::EnableDepthBuffer(bool depth)
	{
		_depthTest = depth;
		if (depth) {
			glEnable(GL_DEPTH_TEST);
			GLERR();
		}
		else {
			glDisable(GL_DEPTH_TEST);
			GLERR();
		}
	}
	void* GLRenderDevice::GetDeviceContext() const
	{
		return nullptr;
	}
	void* GLRenderDevice::GetCurrentFrameData() const
	{
		return nullptr;
	}
	void* GLRenderDevice::GetDefaultResources() const
	{
		return nullptr;
	}
	void GLRenderDevice::Wait() const
	{
		
	}
	void GLRenderDevice::GetDimensions(int* width, int* height) const
	{
		glfwGetFramebufferSize(_window, width, height);
	}
	void GLRenderDevice::SetClearColor(float r, float g, float b, float a)
	{
		_clearColors[0] = r;
		_clearColors[1] = g;
		_clearColors[2] = b;
		_clearColors[3] = a;
	}
	void GLRenderDevice::Clear(Rect& r, Color clr)
	{
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);
		glEnable(GL_SCISSOR_TEST);
		GLERR();
		glScissor(r.left, height-r.bottom, r.Width(), r.Height());//flip y
		GLERR();
		glClearColor(clr.r, clr.g, clr.b, clr.a);
		GLenum clearFlags = GL_COLOR_BUFFER_BIT;
		/*if (_depthTest)
			clearFlags |= GL_DEPTH_BUFFER_BIT;*/
		glClear(clearFlags);
		GLERR();
		glDisable(GL_SCISSOR_TEST);
		GLERR();
	}
	void GLRenderDevice::SetViewport(ViewPort& vp)
	{
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);
		glViewport((GLsizei)vp.x, (GLsizei)height-(GLsizei)vp.y-(GLsizei)vp.height, (GLsizei)vp.width, (GLsizei)vp.height);
		GLERR();
	}
	float GLRenderDevice::GetCurrentTicks()
	{
		return (float)glfwGetTime();
	}
}