#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Font.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VulkanEx.h"
namespace Vulkan {
	class VulkanFont : public Renderer::Font {
		struct Character {
			glm::ivec2 size;
			glm::ivec2 bearing;
			uint32_t offset;
			uint32_t advance;
		};
		struct FontVertex {
			glm::vec3 pos;
			glm::vec2 uv;
		};
		std::vector<FontVertex> _vertices;
		std::vector<uint32_t> _indices;
		std::unordered_map<char, Character> Characters;
		int _width;
		int _height;
		float invBmpWidth;
		uint32_t bmpHeight;
		struct FrameData {
			Buffer vertexBuffer;
			Buffer indexBuffer;
			VkDeviceSize vertSize;
			VkDeviceSize indSize;
			uint32_t numIndices;
		};
		FrameData frames[2];//double-buffering
		uint32_t currFrame;
		std::unique_ptr<VulkanTexture> fontTexturePtr;
		std::unique_ptr<VulkanDescriptor> fontDescriptorPtr;
		std::unique_ptr<VulkanPipelineLayout> fontPipelineLayoutPtr;
		std::unique_ptr<VulkanPipeline> fontPipelinePtr;
		glm::mat4	_orthoproj;
		Texture		_texture;
		Renderer::RenderDevice* _renderdevice;
		struct PushConst {
			glm::mat4 proj;
			glm::vec4 color;
		};

		void Update();
	public:
		VulkanFont();
		~VulkanFont();
		virtual void Init(Renderer::RenderDevice* pdevice, const char* pfont, int fontSize) override;
		virtual void Draw(const char* ptext, int xpos, int ypos) override;
		virtual void Render()override;
		virtual void GetTextSize(const char* ptext, float& width, float& height) override;
		virtual void Clear()override {
			_vertices.clear();
			_indices.clear();
			frames[0].vertSize = frames[1].vertSize = frames[0].indSize = frames[1].indSize = 0; //force update of buffers
		}
		virtual void SetDimensions(int width, int height) override;
	};
}