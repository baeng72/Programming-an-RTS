#include "VulkanImGuiWrapper.h"

#include "../../common/Platform/Vulkan/VulkState.h"
#include "../../common/Platform/Vulkan/VulkSwapchain.h"
#include "../../common/Platform/Vulkan/ShaderCompiler.h"

namespace Vulkan {
	VulkanImGuiWrapper::VulkanImGuiWrapper(Renderer::RenderDevice* pdevice, Core::Window* pwindow)
		:_pdevice(pdevice),_pwindow(pwindow)
	{
		
		const char* vertexSrc = R"(
#version 450 
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outUV;
layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;


void main()
{
	outColor=aColor;
	outUV=aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
)";
		const char* fragmentSrc = R"(
#version 450 core

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inUV;

layout(location = 0) out vec4 fColor;


layout(set=0, binding=0) uniform sampler2D sTexture;

void main()
{
    fColor = inColor * texture(sTexture, inUV);
}
)";
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(pdevice->GetCurrentFrameData());
		VulkFrameData& framedata = *framedataptr;
		ShaderCompiler compiler;
		std::vector<uint32_t> vertexSpirv;
		std::vector<uint32_t> fragmentSpirv;
		vertexSpirv = compiler.compileShader(vertexSrc, VK_SHADER_STAGE_VERTEX_BIT);
		fragmentSpirv = compiler.compileShader(fragmentSrc, VK_SHADER_STAGE_FRAGMENT_BIT);

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
		//create texture
		Vulkan::Texture fontTexture;
		
		//create font
		{
		
			unsigned char* pixels;
			int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
			size_t upload_size = width * height * 4 * sizeof(char);
			fontTexture = TextureBuilder::begin(context.device, context.memoryProperties)
				.setDimensions(width, height)
				.setFormat(PREFERRED_IMAGE_FORMAT)
				.setImageUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
				.setImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
				.build();
			VkDeviceSize stagingSize = upload_size;
			Vulkan::Buffer stagingBuffer = StagingBufferBuilder::begin(context.device, context.memoryProperties)
				.setSize(stagingSize)
				.build();
			uint8_t* ptr = (uint8_t*)Vulkan::mapBuffer(context.device, stagingBuffer);
			memcpy(ptr, pixels, upload_size);
			Vulkan::transitionImage(context.device, context.queue, context.commandBuffer, fontTexture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			Vulkan::CopyBufferToImage(context.device, context.queue, context.commandBuffer, stagingBuffer, fontTexture, width, height);
			Vulkan::unmapBuffer(context.device, stagingBuffer);
			Vulkan::cleanupBuffer(context.device, stagingBuffer);
			Vulkan::transitionImage(context.device, context.queue, context.commandBuffer, fontTexture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			fontTexturePtr = std::make_unique<VulkanTexture>(context.device, fontTexture);

		}

		VkDescriptorSet descriptorSet;
		VkDescriptorSetLayout descriptorLayout;
		DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(descriptorSet, descriptorLayout);
		fontDescriptorPtr = std::make_unique<VulkanDescriptor>(context.device, descriptorSet);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = fontTexture.imageView;
		imageInfo.sampler = fontTexture.sampler;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		DescriptorSetUpdater::begin(context.pLayoutCache, descriptorLayout, descriptorSet)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfo)
			.update();
		
		std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConst)} };
		VkPipelineLayout pipelineLayout;
		PipelineLayoutBuilder::begin(context.device)
			.AddDescriptorSetLayout(descriptorLayout)
			.AddPushConstants(pushConstants)
			.build(pipelineLayout);
		fontPipelineLayoutPtr = std::make_unique<VulkanPipelineLayout>(context.device, pipelineLayout);
		std::vector<ShaderModule> shaders;
		VkVertexInputBindingDescription vertexInputDescription = {};
		std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
		ShaderProgramLoader::begin(context.device)
			.AddShaderSpirv(vertexSpirv)
			.AddShaderSpirv(fragmentSpirv)
			.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
		vertexAttributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		vertexInputDescription.stride = sizeof(ImDrawVert);//need to set this

		VkPipeline pipeline;
		PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
			.setBlend(VK_TRUE)
			//.setCullMode(VK_CULL_MODE_FRONT_BIT)
			.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.build(pipeline);
		fontPipelinePtr = std::make_unique<VulkanPipeline>(context.device, pipeline);
		for (auto& shader : shaders) {
			cleanupShaderModule(context.device, shader.shaderModule);
		}

		pwindow->SetEventHandler(std::bind(&VulkanImGuiWrapper::OnEvent, this, std::placeholders::_1));

		currFrame = UINT32_MAX;
		for (int i = 0; i < MAX_FRAMES; i++) {
			frames[i].indexBuffer.buffer = VK_NULL_HANDLE;
			frames[i].indSize = 0;
			frames[i].vertexBuffer.buffer = VK_NULL_HANDLE;
			frames[i].vertSize = 0;
			frames[i].pverts = nullptr;
			frames[i].pindices = nullptr;
		}
		
		int width, height;
		pdevice->GetDimensions(&width, &height);
		io.DisplaySize = ImVec2((float)width, (float)height);
		io.DisplayFramebufferScale = ImVec2(1, 1);
		_viewport.x = _viewport.y = 0;
		_viewport.width = width;
		_viewport.height = height;
		_viewport.ffar = 1000.f;
		_viewport.fnear = 0.1f;
	}
	VulkanImGuiWrapper::~VulkanImGuiWrapper()
	{
		_pdevice->Wait();
		ImGui::DestroyContext();
		VulkContext& context = *reinterpret_cast<VulkContext*>(_pdevice->GetDeviceContext());
		for (int i = 0; i < MAX_FRAMES; i++) {
			cleanupBuffer(context.device, frames[i].indexBuffer);
			cleanupBuffer(context.device, frames[i].vertexBuffer);
			
		}
		fontTexturePtr.reset();
		fontPipelineLayoutPtr.reset();
		fontPipelinePtr.reset();
	}
	
	void VulkanImGuiWrapper::Update(float delta)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = delta;
		
	}
	void VulkanImGuiWrapper::SetViewport(ViewPort& vp) {
		_viewport = vp;
	}
	void VulkanImGuiWrapper::Start() {
		ImGui::NewFrame();
	}

	void VulkanImGuiWrapper::End()
	{
		VulkContext& context = *reinterpret_cast<VulkContext*>(_pdevice->GetDeviceContext());
		VulkFrameData& frameData = *reinterpret_cast<VulkFrameData*>(_pdevice->GetCurrentFrameData());
		VkCommandBuffer cmd = frameData.cmd;
		currFrame++;
		uint32_t frameIdx = currFrame % MAX_FRAMES;
		int width, height;
		//_pdevice->GetDimensions(&width, &height);
		width = (int)_viewport.width;
		height = (int)_viewport.height;
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
			if (draw_data->TotalVtxCount > 0) {
				//create or resize vertex/index buffers
				VkDeviceSize vertSize = draw_data->TotalVtxCount * sizeof(ImDrawVert);
				VkDeviceSize indSize = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
				VkDeviceSize maxSize = std::max(vertSize, indSize);
				
				
				if (frame.vertSize == 0 || frame.vertSize < vertSize) {
					if (frame.vertexBuffer.buffer != VK_NULL_HANDLE) {
						cleanupBuffer(context.device, frame.vertexBuffer);
					}
					std::vector<uint32_t> vertexLocations;
					VertexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
						.AddVertices(vertSize,nullptr,true )
						.build(frame.vertexBuffer, vertexLocations,(void**)&frame.pverts);
					frame.vertSize = vertSize;
				}
				
				if (frame.indSize == 0 || frame.indSize < indSize) {
					if (frame.indexBuffer.buffer != VK_NULL_HANDLE) {
						cleanupBuffer(context.device, frame.indexBuffer);
					}
					std::vector<uint32_t> indexLocations;
					IndexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
						.AddIndices(indSize, nullptr,true)
						.build(frame.indexBuffer, indexLocations,(void**)&frame.pindices);
					frame.indSize = indSize;
				}
				
				ImDrawVert* vptr = (ImDrawVert*)frame.pverts;
				ImDrawIdx* iptr = (ImDrawIdx*)frame.pindices;
				//upload vertex/index data


				for (int32_t n = 0; n < draw_data->CmdListsCount; n++) {
					const ImDrawList* cmd_list = draw_data->CmdLists[n];
					memcpy(vptr, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
					memcpy(iptr, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

					vptr += cmd_list->VtxBuffer.Size;
					iptr += cmd_list->IdxBuffer.Size;

				}
				Vulkan::flushBuffer(context.device, frame.vertexBuffer, frame.vertSize);
				Vulkan::flushBuffer(context.device, frame.indexBuffer, frame.indSize);
			}
			if (frame.vertSize > 0) {
				int width, height;
				_pdevice->GetDimensions(&width, &height);
				VkPipelineLayout pipelineLayout = *fontPipelineLayoutPtr;
				VkDescriptorSet descriptorSet = *fontDescriptorPtr;
				VkPipeline pipeline = *fontPipelinePtr;
				VkDescriptorSet descriptor = *fontDescriptorPtr;
				// Will project scissor/clipping rectangles into framebuffer space
				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
				VkDeviceSize offsets[1] = { 0 };
				vkCmdBindVertexBuffers(cmd, 0, 1, &frame.vertexBuffer.buffer, offsets);
				vkCmdBindIndexBuffer(cmd, frame.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
				// Setup viewport:
				{
					VkViewport viewport;
					viewport.x = _viewport.x;
					viewport.y = _viewport.y;
					viewport.width = (float)fb_width;
					viewport.height = (float)fb_height;
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
					vkCmdSetViewport(cmd, 0, 1, &viewport);
				}
				// Setup scale and translation:
		   // Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
				struct PushConst {
					glm::vec2 scale;
					glm::vec2 translate;
				}pushConst;
				{

					pushConst.scale.x = 2.0f / draw_data->DisplaySize.x;
					pushConst.scale.y = 2.0f / draw_data->DisplaySize.y;
					pushConst.translate.x = -1.0f - draw_data->DisplayPos.x * pushConst.scale.x;
					pushConst.translate.y = -1.0f - draw_data->DisplayPos.y * pushConst.scale.y;
					vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConst), &pushConst);

				}
				ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
				ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
				 // Render command lists
				// (Because we merged all buffers into a single one, we maintain our own offset into them)
				int32_t global_vtx_offset = 0;
				int32_t global_idx_offset = 0;
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptor, 0, nullptr);
				for (int32_t n = 0; n < draw_data->CmdListsCount; n++) {
					const ImDrawList* cmd_list = draw_data->CmdLists[n];
					for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
					{
						const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
						// Project scissor/clipping rectangles into framebuffer space
						ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
						ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
						// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
						if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
						if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
						if (clip_max.x > width) { clip_max.x = (float)fb_width; }
						if (clip_max.y > height) { clip_max.y = (float)fb_height; }
						if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
							continue;
						// Apply scissor/clipping rectangle
						VkRect2D scissor;
						scissor.offset.x = (int32_t)(clip_min.x);
						scissor.offset.y = (int32_t)(clip_min.y);
						scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
						scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);
						vkCmdSetScissor(cmd, 0, 1, &scissor);


						vkCmdDrawIndexed(cmd, pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
					}
					global_idx_offset += cmd_list->IdxBuffer.Size;
					global_vtx_offset += cmd_list->VtxBuffer.Size;
				}
				//Vulkan::flushBuffer(context.device, vertexBuffer, global_vtx_offset);
				//Vulkan::flushBuffer(context.device, indexBuffer, global_idx_offset);
				// Note: at this point both vkCmdSetViewport() and vkCmdSetScissor() have been called.
		  //// Our last values will leak into user/application rendering IF:
		  // - Your app uses a pipeline with VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR dynamic state
		  // - And you forgot to call vkCmdSetViewport() and vkCmdSetScissor() yourself to explicitly set that state.
		  // If you use VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR you are responsible for setting the values before rendering.
		  // In theory we should aim to backup/restore those values but I am not sure this is possible.
		  // We perform a call to vkCmdSetScissor() to set back a full viewport which is likely to fix things for 99% users but technically this is not perfect. (See github #4644)
				VkRect2D scissor = { { 0, 0 }, { (uint32_t)fb_width, (uint32_t)fb_height } };
				vkCmdSetScissor(cmd, 0, 1, &scissor);
			}
		}
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();

			ImGui::RenderPlatformWindowsDefault();
		}

	}
	void VulkanImGuiWrapper::OnEvent(Event& e)
	{

		EventDispatcher dispatcher(e);
		
		dispatcher.Dispatch<KeyPressedEvent>(EventType::KeyPressed, std::bind(&VulkanImGuiWrapper::OnKeyPressed, this, std::placeholders::_1));
		dispatcher.Dispatch<KeyReleasedEvent>(EventType::KeyReleased, std::bind(&VulkanImGuiWrapper::OnKeyReleased, this, std::placeholders::_1));
		dispatcher.Dispatch<KeyTypedEvent>(EventType::KeyTyped, std::bind(&VulkanImGuiWrapper::OnKeyTyped, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseMovedEvent>(EventType::MouseMoved, std::bind(&VulkanImGuiWrapper::OnMouseMoved, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseScrolledEvent>(EventType::MouseScrolled, std::bind(&VulkanImGuiWrapper::OnMouseScrolled, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseButtonPressedEvent>(EventType::MouseButtonPressed, std::bind(&VulkanImGuiWrapper::OnMouseButtonDown, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(EventType::MouseButtonReleased, std::bind(&VulkanImGuiWrapper::OnMouseButtonUp, this, std::placeholders::_1));

	}
	bool VulkanImGuiWrapper::OnKeyPressed(KeyPressedEvent& e)
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

	bool VulkanImGuiWrapper::OnKeyReleased(KeyReleasedEvent& e) {
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
	bool VulkanImGuiWrapper::OnMouseMoved(MouseMovedEvent& e)
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
	bool VulkanImGuiWrapper::OnMouseScrolled(MouseScrolledEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		
		float wheel_x = (e.xOffset > 0) ? 1.f : (e.xOffset < 0) ? -1.f : 0.f;
		float wheel_y = (e.yOffset > 0) ? 1.f : (e.yOffset < 0) ? -1.f : 0.f;
		io.AddMouseWheelEvent(wheel_x, wheel_y);
		return true;
	}
	bool VulkanImGuiWrapper::OnMouseButtonDown(MouseButtonPressedEvent& e)
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
	bool VulkanImGuiWrapper::OnMouseButtonUp(MouseButtonReleasedEvent& e)
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
	bool VulkanImGuiWrapper::OnKeyTyped(KeyTypedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		char buffer[4] = { (char)e.key,0,0,0 };
		io.AddInputCharactersUTF8(buffer);
		return true;
	}
}