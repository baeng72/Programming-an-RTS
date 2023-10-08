
#include "../../Core/Log.h"
#include "VulkanFont.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
#include "ShaderCompiler.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Vulkan {
	
	inline mat4 myvulkOrthoRH(float left, float right, float top, float bottom, float zn, float zf) {
		mat4 mat = mat4(1.f);
		float height = bottom - top;
		float width = right - left;
		mat[0][0] = 2.f / (width);
		mat[1][1] = -2.f / (top-bottom);//flip y
		mat[2][2] = -1.f / (zf - zn);
		mat[3][0] = -(right + left) / (width);
		mat[3][1] = -(top + bottom) / (height);
		mat[3][2] = -zn / (zf - zn);
		//mat[1][1] *= -1;//flip y for Vulkan
		return mat;
	}


	VulkanFont::VulkanFont() :_renderdevice(nullptr),_width(0),_height(0),invBmpWidth(0.f) {
		bmpHeight = 0;
		currFrame = UINT32_MAX;
		for (int i = 0; i < MAX_FRAMES; i++) {
			frames[i].indexBuffer.buffer = VK_NULL_HANDLE;
			frames[i].indSize = 0;
			frames[i].vertexBuffer.buffer = VK_NULL_HANDLE;
			frames[i].vertSize = 0;
		}
	}

	VulkanFont::~VulkanFont()
	{
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(_renderdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		vkDeviceWaitIdle(context.device);
	}

	void VulkanFont::Init(Renderer::RenderDevice* pdevice, const char* pfont, int fontSize)
	{
		const char* vertexSrc = R"(
#version 450
layout (location=0) in vec3 inPos;
layout (location=1) in vec4 inColor;
layout (location=2) in vec2 inTexCoords;
layout (location=0) out vec2 outTexCoords;
layout (location=1) out vec4 outColor;

layout (push_constant) uniform PushConst{
	mat4 projection;
	
};

void main(){
	
	
	gl_Position = projection * vec4(inPos,1.0);
	
	outTexCoords = inTexCoords;
	outColor = inColor;
}
)";

		const char* fragmentSrc = R"(
#version 450
layout (location=0) in vec2 inTextCoords;
layout (location=1) in vec4 inColor;
layout (location=0) out vec4 outFragColor;

layout (set=0,binding=0) uniform sampler2D text;

void main(){
	vec4 sampled = vec4(1.0,1.0,1.0,texture(text,inTextCoords).r);
	outFragColor = inColor*sampled;
}
)";
		_renderdevice = pdevice;
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(pdevice->GetCurrentFrameData());
		VulkFrameData& framedata = *framedataptr;

		ShaderCompiler compiler;
		std::vector<uint32_t> vertexSpirv;
		std::vector<uint32_t> fragmentSpirv;
		vertexSpirv = compiler.compileShader(vertexSrc, VK_SHADER_STAGE_VERTEX_BIT);
		fragmentSpirv = compiler.compileShader(fragmentSrc, VK_SHADER_STAGE_FRAGMENT_BIT);


		{
			if (fontSize < 1 || fontSize > 120) {
				fontSize = 18;
			}
			FT_Library ft;
			FT_Error res = FT_Init_FreeType(&ft);
			ASSERT(!res, "Unable init FreeType.");
			FT_Face face;

			res = FT_New_Face(ft, pfont, 0, &face);
			ASSERT(!res, "Unable to load font: {0}.", pfont);
			assert(res == 0);
			FT_Set_Pixel_Sizes(face, 0, fontSize);
			uint32_t bmpWidth = 0;
			std::vector<uint8_t> pixels;

			std::unordered_map<char, std::vector<uint8_t>> data;
			for (unsigned char c = 0; c < 128; c++)
			{

				res = FT_Load_Char(face, c, FT_LOAD_RENDER);
				assert(res == 0);


				bmpHeight = std::max(bmpHeight, face->glyph->bitmap.rows);

				unsigned int pitch = face->glyph->bitmap.pitch;
				Character character = {
					glm::ivec2(face->glyph->bitmap.width,face->glyph->bitmap.rows),
					glm::ivec2(face->glyph->bitmap_left,face->glyph->bitmap_top),
					bmpWidth,
					static_cast<unsigned int>(face->glyph->advance.x)
				};

				Characters.insert(std::pair<char, Character>(c, character));
				if (face->glyph->bitmap.width > 0) {
					void* ptr = face->glyph->bitmap.buffer;

					std::vector<uint8_t> charData(face->glyph->bitmap.width * face->glyph->bitmap.rows);

					int rows = face->glyph->bitmap.rows;
					int width = face->glyph->bitmap.width;
					for (int i = 0; i < rows; i++) {
						for (int j = 0; j < width; j++) {
							uint8_t byte = face->glyph->bitmap.buffer[i * pitch + j];
							charData[i * pitch + j] = byte;
						}
					}
					data.insert(std::pair<char, std::vector<uint8_t>>(c, charData));
				}
				bmpWidth += face->glyph->bitmap.width;
			}

			res = FT_Done_Face(face);
			assert(res == 0);

			invBmpWidth = 1 / (float)bmpWidth;

			uint8_t* buffer = new uint8_t[bmpHeight * bmpWidth];
			memset(buffer, 0, bmpHeight * bmpWidth);

			uint32_t xpos = 0;
			for (unsigned char c = 0; c < 128; c++)
			{
				Character& character = Characters[c];

				std::vector<uint8_t>& charData = data[c];
				uint32_t width = character.size.x;
				uint32_t height = character.size.y;
				for (uint32_t i = 0; i < height; i++) {
					for (uint32_t j = 0; j < width; j++) {
						uint8_t byte = charData[i * width + j];
						buffer[i * bmpWidth + xpos + j] = byte;
					}
				}
				xpos += width;
			}

			Texture texture;

			VkDeviceSize pixsize = 1;
			bool enableLod = false;

			VkDeviceSize imageSize = (uint64_t)bmpWidth * (uint64_t)bmpHeight * pixsize;
			TextureProperties props;
			props.format = VK_FORMAT_R8_UNORM;
			props.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			props.height = bmpHeight;
			props.width = bmpWidth;
			props.layout = VK_IMAGE_LAYOUT_UNDEFINED;
			props.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			props.mipLevels = enableLod ? 0 : 1;
#ifdef __USE__VMA__
			props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
#else
			props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
			initTexture(context.device, context.memoryProperties, props, texture);
			transitionImage(context.device, context.queue, context.commandBuffer, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mipLevels);

			Buffer stagingBuffer;
			BufferProperties bufProps;
#ifdef __USE__VMA__
			bufProps.usage = VMA_MEMORY_USAGE_CPU_ONLY;
#else
			bufProps.memoryProps = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
#endif
			bufProps.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufProps.size = imageSize;
			initBuffer(context.device, context.memoryProperties, bufProps, stagingBuffer);
			void* ptr = mapBuffer(context.device, stagingBuffer);
			memcpy(ptr, buffer, imageSize);
			CopyBufferToImage(context.device, context.queue, context.commandBuffer, stagingBuffer, texture, bmpWidth, bmpHeight);
			generateMipMaps(context.device, context.queue, context.commandBuffer, texture);
			unmapBuffer(context.device, stagingBuffer);
			cleanupBuffer(context.device, stagingBuffer);

			fontTexturePtr = std::make_unique<VulkanTexture>(context.device, texture);

			delete[] buffer;

			VkDescriptorSet descriptorSet;
			VkDescriptorSetLayout descriptorLayout;
			DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
				.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build(descriptorSet, descriptorLayout);
			fontDescriptorPtr = std::make_unique<VulkanDescriptor>(context.device, descriptorSet);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageView = texture.imageView;
			imageInfo.sampler = texture.sampler;
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
			FT_Done_FreeType(ft);


		}
		pdevice->GetDimensions(&_width, &_height);
		_orthoproj = myvulkOrthoRH(0.f, (float)_width, 0.f, (float)_height, -1.f, 1.f);


	}

	void VulkanFont::Draw(const char* ptext, int xstart, int ystart,glm::vec4 color)
	{


		std::vector<FontVertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t indexoffset = (uint32_t)_vertices.size();
		size_t len = strlen(ptext);
		float x = (float)xstart;
		float y = (float)ystart;
		float fbmpHeight = (float)bmpHeight;

		float width, height;
		GetTextSize(ptext, width, height);
		float scale = 1.f;


		for (size_t i = 0; i < len; i++) {
			char c = ptext[i];
			Character& character = Characters[c];
			float xpos = x + character.bearing.x * scale;
			float ypos = y + (height - character.bearing.y) * scale;// (character.Size.y - character.Bearing.y)* scale;

			float w = (float)character.size.x * scale;
			float hraw = (float)character.size.y;
			float h = hraw * scale;
			float u0 = (float)character.offset * invBmpWidth;
			float v = (hraw) / fbmpHeight;
			float u1 = (float)(character.offset + character.size.x) * invBmpWidth;

			FontVertex topleft = { {xpos,ypos,0.0f},color,{u0,0.f} };
			FontVertex topright = { { xpos + w,ypos,0.0f},color,{u1,0.f} };
			FontVertex bottomleft = { { xpos,ypos + h,0.0f},color,{u0,v} };
			FontVertex bottomright = { {xpos + w,ypos + h,0.0f},color,{u1,v} };
			vertices.push_back(topleft);
			vertices.push_back(topright);
			vertices.push_back(bottomleft);
			vertices.push_back(bottomright);
			indices.push_back(indexoffset + 0);
			indices.push_back(indexoffset + 1);
			indices.push_back(indexoffset + 2);
			indices.push_back(indexoffset + 1);
			indices.push_back(indexoffset + 3);
			indices.push_back(indexoffset + 2);
			indexoffset += 4;
			x += (character.advance >> 6) * scale;

		}
		_vertices.insert(_vertices.end(), vertices.begin(), vertices.end());
		_indices.insert(_indices.end(), indices.begin(), indices.end());




	}
	void VulkanFont::Update() {
		VulkContext& context = *reinterpret_cast<VulkContext*>(_renderdevice->GetDeviceContext());
		VulkFrameData& frameData = *reinterpret_cast<VulkFrameData*>(_renderdevice->GetCurrentFrameData());
		VkCommandBuffer cmd = frameData.cmd;
		VkDeviceSize vertSize = sizeof(FontVertex) * _vertices.size();
		VkDeviceSize indSize = sizeof(uint32_t) * _indices.size();
		currFrame++;
		uint32_t frameIdx = currFrame % MAX_FRAMES;
		VkDeviceSize maxSize = std::max(vertSize, indSize);
		
		FrameData& frame = frames[frameIdx];
		bool needStaging = !(frame.vertSize == 0 || frame.vertSize < vertSize || frame.indSize == 0 || frame.indSize < indSize);
		Vulkan::Buffer stagingBuffer;
		void* ptr = nullptr;
		if (needStaging) {
			stagingBuffer = StagingBufferBuilder::begin(context.device, context.memoryProperties)
				.setSize(maxSize)
				.build();
			ptr = Vulkan::mapBuffer(context.device, stagingBuffer);
		}
		if (frame.vertSize == 0 || frame.vertSize < vertSize) {
			if (frame.vertexBuffer.buffer != VK_NULL_HANDLE) {
				cleanupBuffer(context.device, frame.vertexBuffer);
			}
			std::vector<uint32_t> vertexLocations;
			VertexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddVertices(vertSize, (float*)_vertices.data())
				.build(frame.vertexBuffer, vertexLocations);
		
		}
		else if (vertSize > 0) {
			//stream vertices
			
			memcpy(ptr, _vertices.data(), vertSize);			
			Vulkan::CopyBufferTo(context.device, context.queue, context.commandBuffer, stagingBuffer, frame.vertexBuffer, vertSize);
			

		}
		frame.vertSize = vertSize;
		if (frame.indSize == 0 || frame.indSize < indSize) {
			if (frame.indexBuffer.buffer != VK_NULL_HANDLE) {
				cleanupBuffer(context.device, frame.indexBuffer);
			}
			std::vector<uint32_t> indexLocations;
			IndexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddIndices(indSize, _indices.data())
				.build(frame.indexBuffer, indexLocations);
			
		}
		else if (indSize > 0) {
			//stream indices
			memcpy(ptr, _indices.data(), indSize);
			Vulkan::CopyBufferTo(context.device, context.queue, context.commandBuffer, stagingBuffer, frame.indexBuffer, indSize);
			
		}
		frame.indSize = indSize;
		frame.numIndices = (uint32_t)_indices.size();
		if (needStaging) {
			Vulkan::unmapBuffer(context.device, stagingBuffer);
			Vulkan::cleanupBuffer(context.device, stagingBuffer);
		}
		currFrame = frameIdx;
		_vertices.clear();
		_indices.clear();
	}

	void VulkanFont::Render() {
		if (_vertices.size() == 0)
			return;
		Update();
		FrameData& frame = frames[currFrame];
		VulkFrameData& frameData = *reinterpret_cast<VulkFrameData*>(_renderdevice->GetCurrentFrameData());
		VkCommandBuffer cmd = frameData.cmd;
		VkPipelineLayout pipelineLayout = *fontPipelineLayoutPtr;
		VkDescriptorSet descriptorSet = *fontDescriptorPtr;
		VkPipeline pipeline = *fontPipelinePtr;
		PushConst pushConst{ _orthoproj};
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, &frame.vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(cmd, frame.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConst), &pushConst);
		vkCmdDrawIndexed(cmd, frame.numIndices, 1, 0, 0, 0);
	}

	

	void VulkanFont::GetTextSize(const char* ptext, float& width, float& height)
	{
		size_t len = strlen(ptext);
		float x = 0.f;
		float y = 0.f;
		for (size_t i = 0; i < len; i++) {
			char c = ptext[i];
			Character& character = Characters[c];
			float w = (float)character.size.x;
			float h = (float)character.size.y;


			x += (character.advance >> 6);
			y = std::max(y, h);
		}
		width = x;
		height = y;

	}

	void VulkanFont::SetDimensions(int width, int height) {
		_width = width;
		_height = height;
		_orthoproj = glm::ortho(0.f, (float)_width, 0.f, (float)_height, -1.f, 1.f);
	}
}