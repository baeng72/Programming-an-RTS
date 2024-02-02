#include "GLImGuiWrapper.h"

namespace GL {
	GLImGuiWrapper::GLImGuiWrapper(Renderer::RenderDevice* pdevice, Core::Window* pwindow)
		:_pdevice(pdevice),_pwindow(pwindow)
	{
		currFrame = UINT32_MAX;
		for (int i = 0; i < MAX_FRAMES; i++) {
			frames[i].indexBuffer = -1;
			frames[i].indSize = 0;
			frames[i].vertexBuffer = -1;
			frames[i].vertSize = 0;
		}
		const char* vertexSrc = R"(
#version 460 core
layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec4 Color;
uniform mat4 ProjMtx;
out vec2 Frag_UV;
out vec4 Frag_Color;
void main()
{
	Frag_UV = UV;
    Frag_Color = Color;
    gl_Position = ProjMtx * vec4(Position.xy,0,1);
}
)";
		const char* fragmentSrc = R"(
#version 460 core
layout (location=0) in vec2 Frag_UV;
layout (location=1) in vec4 Frag_Color;
uniform sampler2D sTexture;
layout (location = 0) out vec4 Out_Color;
void main()
{
    Out_Color = Frag_Color * texture(sTexture, Frag_UV);
}
)";
		_shader.compile(vertexSrc, nullptr, fragmentSrc);
		_matloc = _shader.GetUniformLocation("ProjMtx");
		_texloc = _shader.GetUniformLocation("sTexture");
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		//create font
		{

			unsigned char* pixels;
			int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
			//size_t upload_size = width * height * 4 * sizeof(char);

			glGenTextures(1, &_texture);
			glBindTexture(GL_TEXTURE_2D, _texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
			
			GLERR();
			//glGenerateMipmap(GL_TEXTURE_2D);
			///io.Fonts->SetTexID((ImTextureID)(intptr_t)_texture);

		}
		//glGenVertexArrays(1, &_vao);
		GLuint vaos[2];
		glGenVertexArrays(2, vaos);
		frames[0].vao = vaos[0];
		frames[1].vao = vaos[1];
		{
			int width, height;
			pdevice->GetDimensions(&width, &height);
			io.DisplaySize = ImVec2((float)width, (float)height);
			io.DisplayFramebufferScale = ImVec2(1, 1);
			_viewport.x = 0.f;
			_viewport.y = 0.f;
			_viewport.width = (float)width;
			_viewport.height = (float)height;
			_viewport.fnear = 0.f;
			_viewport.ffar = 1.f;
		}
		pwindow->SetEventHandler(std::bind(&GLImGuiWrapper::OnEvent, this, std::placeholders::_1));

	}
	GLImGuiWrapper::~GLImGuiWrapper()
	{
		ImGui::DestroyContext();
		for (size_t i = 0; i < MAX_FRAMES; i++) {
			glDeleteBuffers(1, &frames[i].vertexBuffer);
			glDeleteBuffers(1, &frames[i].indexBuffer);
		}
		GLuint vaos[2] = { frames[0].vao,frames[1].vao };
		glDeleteVertexArrays(2, vaos);
		//glDeleteVertexArrays(1,&_vao);
		glDeleteTextures(1, &_texture);
	}
	
	void GLImGuiWrapper::Update(float delta)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = delta;
	}
	void GLImGuiWrapper::SetViewport(ViewPort& vp) {
		_viewport = vp;
	}
	void GLImGuiWrapper::Start() {
		ImGui::NewFrame();
	}
	void GLImGuiWrapper::End()
	{
		currFrame++;
		uint32_t frameIdx = currFrame % MAX_FRAMES;
		int width, height;
		_pdevice->GetDimensions(&width, &height);
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)width, (float)height);
		io.DisplayFramebufferScale = ImVec2(1, 1);
		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		FrameData& frame = frames[frameIdx];
		if (!(draw_data->DisplaySize.x <= 0.f || draw_data->DisplaySize.y < 0.f)) {
			int32_t fb_width = (int32_t)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
			int32_t fb_height = (int32_t)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
			if (fb_width < 0 || fb_height < 0)
				return;
			//create or resize vertex/index buffers
			uint32_t vertSize = draw_data->TotalVtxCount * sizeof(ImDrawVert);
			uint32_t indSize = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
			if (draw_data->TotalVtxCount > 0) {
				glBindVertexArray(frame.vao);
				if (frame.vertSize == 0 || frame.vertSize < vertSize) {
					if (frame.vertexBuffer != -1) {
						glDeleteBuffers(1, &frame.vertexBuffer);

					}
					glGenBuffers(1, &frame.vertexBuffer);

					glBindBuffer(GL_ARRAY_BUFFER, frame.vertexBuffer);
					glBufferData(GL_ARRAY_BUFFER, vertSize, nullptr, GL_STREAM_DRAW);
					glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), 0);
					glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void*)sizeof(vec2));
					glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE,  GL_TRUE, sizeof(ImDrawVert), (void*)(sizeof(vec2) + sizeof(vec2)));
					glEnableVertexAttribArray(0);
					glEnableVertexAttribArray(1);
					glEnableVertexAttribArray(2);
					GLERR();
					frame.vertSize = vertSize;
				}
				else {
					glBindBuffer(GL_ARRAY_BUFFER, frame.vertexBuffer);
				}
				if (frame.indSize == 0 || frame.indSize < indSize) {
					if (frame.indexBuffer != -1) {
						glDeleteBuffers(1, &frame.indexBuffer);
					}
					glGenBuffers(1, &frame.indexBuffer);

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frame.indexBuffer);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, nullptr, GL_STREAM_DRAW);
					GLERR();
					frame.indSize = indSize;

				}
				else {
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frame.indexBuffer);
				}

				//glBindVertexArray(_vao);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				_shader.Bind();
				mat4 mat = vulkOrthoRH(0.f,(float)width,0.f, (float)height, -1.f, 1.f);
				
				_shader.setMat4("ProjMtx", mat);
				
				
				_shader.SetTextures("sTexture", (int*)&_texture, 1u);
				
				GLERR();
				GLint oldvp[4];
				glGetIntegerv(GL_VIEWPORT, oldvp);
				glViewport((GLint)_viewport.x, (GLint)_viewport.y, (GLint)std::min((int32_t)_viewport.width, fb_width), (GLint)std::min((int32_t)_viewport.height, fb_height));
				GLERR();
				// Will project scissor/clipping rectangles into framebuffer space
				ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
				ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
				for (int32_t n = 0; n < draw_data->CmdListsCount; n++) {
					const ImDrawList* cmd_list = draw_data->CmdLists[n];
					const GLsizeiptr vtx_buffer_size = (GLsizeiptr)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert);
					const GLsizeiptr idx_buffer_size = (GLsizeiptr)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx);
					glBufferData(GL_ARRAY_BUFFER, vtx_buffer_size, (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_buffer_size, (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);
					GLERR();
					for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
					{
						const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
						ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
						ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
						if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
							continue;
						//glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID());
						
						glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)), (GLint)pcmd->VtxOffset);
						GLERR();
					}
					
				}
				glViewport(oldvp[0],oldvp[1],oldvp[2],oldvp[3]);
				glBindVertexArray(0);
				GLERR();
			}
		}
	}
	void GLImGuiWrapper::OnEvent(Event& e)
	{

		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<KeyPressedEvent>(EventType::KeyPressed, std::bind(&GLImGuiWrapper::OnKeyPressed, this, std::placeholders::_1));
		dispatcher.Dispatch<KeyReleasedEvent>(EventType::KeyReleased, std::bind(&GLImGuiWrapper::OnKeyReleased, this, std::placeholders::_1));
		dispatcher.Dispatch<KeyTypedEvent>(EventType::KeyTyped, std::bind(&GLImGuiWrapper::OnKeyTyped, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseMovedEvent>(EventType::MouseMoved, std::bind(&GLImGuiWrapper::OnMouseMoved, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseScrolledEvent>(EventType::MouseScrolled, std::bind(&GLImGuiWrapper::OnMouseScrolled, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseButtonPressedEvent>(EventType::MouseButtonPressed, std::bind(&GLImGuiWrapper::OnMouseButtonDown, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(EventType::MouseButtonReleased, std::bind(&GLImGuiWrapper::OnMouseButtonUp, this, std::placeholders::_1));

	}
	bool GLImGuiWrapper::OnKeyPressed(KeyPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		bool ctrl = e.mod & KEY_MOD_CONTROL;
		bool shift = e.mod & KEY_MOD_SHIFT;
		bool alt = e.mod & KEY_MOD_ALT;
		bool super = e.mod & KEY_MOD_SUPER;
		io.AddKeyEvent(ImGuiMod_Ctrl, ctrl);

		io.AddKeyEvent(ImGuiMod_Shift, shift);
		io.AddKeyEvent(ImGuiMod_Alt, alt);
		io.AddKeyEvent(ImGuiMod_Super, super);

		ImGuiKey key = ImGui::KeycodeToImGuiKey(e.key);
		io.AddKeyEvent(key, true);
		io.SetKeyEventNativeData(key, e.key, e.scancode, e.scancode); // To support legacy indexing (<1.87 user code). Legacy backend uses SDLK_*** as indices to IsKeyXXX() functions.
		return true;

	}

	bool GLImGuiWrapper::OnKeyReleased(KeyReleasedEvent& e) {
		ImGuiIO& io = ImGui::GetIO();
		bool ctrl = e.mod & KEY_MOD_CONTROL;
		bool shift = e.mod & KEY_MOD_SHIFT;
		bool alt = e.mod & KEY_MOD_ALT;
		bool super = e.mod & KEY_MOD_SUPER;
		io.AddKeyEvent(ImGuiMod_Ctrl, ctrl);

		io.AddKeyEvent(ImGuiMod_Shift, shift);
		io.AddKeyEvent(ImGuiMod_Alt, alt);
		io.AddKeyEvent(ImGuiMod_Super, super);

		ImGuiKey key = ImGui::KeycodeToImGuiKey(e.key);
		io.AddKeyEvent(key, false);
		io.SetKeyEventNativeData(key, e.key, e.scancode, e.scancode); // To support legacy indexing (<1.87 user code). Legacy backend uses SDLK_*** as indices to IsKeyXXX() functions.
		return true;
	}
	bool GLImGuiWrapper::OnMouseMoved(MouseMovedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		float x = e.xPos;
		float y = e.yPos;
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			int window_x, window_y;
			_pwindow->GetWindowPos(window_x, window_y);

			x += window_x;
			y += window_y;
		}

		io.AddMousePosEvent(x, y);
		return true;
	}
	bool GLImGuiWrapper::OnMouseScrolled(MouseScrolledEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();

		float wheel_x = (e.xOffset > 0) ? 1.f : (e.xOffset < 0) ? -1.f : 0.f;
		float wheel_y = (e.yOffset > 0) ? 1.f : (e.yOffset < 0) ? -1.f : 0.f;
		io.AddMouseWheelEvent(wheel_x, wheel_y);
		return true;
	}
	bool GLImGuiWrapper::OnMouseButtonDown(MouseButtonPressedEvent& e)
	{

		ImGuiIO& io = ImGui::GetIO();
		int mouse_button = -1;
		if (e.button == MOUSE_BUTTON_LEFT) { mouse_button = 0; }
		if (e.button == MOUSE_BUTTON_RIGHT) { mouse_button = 1; }
		if (e.button == MOUSE_BUTTON_MIDDLE) { mouse_button = 2; }
		//if (e.button == SDL_BUTTON_X1) { mouse_button = 3; }
		//if (e.button == SDL_BUTTON_X2) { mouse_button = 4; }
		if (mouse_button == -1)
			return true;

		io.AddMouseButtonEvent(mouse_button, true);
		return true;
	}
	bool GLImGuiWrapper::OnMouseButtonUp(MouseButtonReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		int mouse_button = -1;
		if (e.button == MOUSE_BUTTON_LEFT) { mouse_button = 0; }
		if (e.button == MOUSE_BUTTON_RIGHT) { mouse_button = 1; }
		if (e.button == MOUSE_BUTTON_MIDDLE) { mouse_button = 2; }
		//if (e.button == SDL_BUTTON_X1) { mouse_button = 3; }
		//if (e.button == SDL_BUTTON_X2) { mouse_button = 4; }
		if (mouse_button == -1)
			return true;
		io.AddMouseButtonEvent(mouse_button, false);
		return true;
	}
	bool GLImGuiWrapper::OnKeyTyped(KeyTypedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		char buffer[4] = { (char)e.key,0,0,0 };
		io.AddInputCharactersUTF8(buffer);
		return true;
	}
}