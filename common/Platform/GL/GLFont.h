#pragma once
#include <vector>
#include <unordered_map>
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Font.h"
#include <glad/glad.h>
#include "ShaderUtil.h"
namespace GL {
	class GLFont : public Renderer::Font {
		struct Character {
			glm::ivec2 size;
			glm::ivec2 bearing;
			uint32_t offset;
			uint32_t advance;
		};
		struct FontVertex {
			glm::vec3 pos;
			glm::vec4 color;
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
			GLuint vertexBuffer;
			GLuint indexBuffer;
			uint32_t vertSize;
			uint32_t indSize;
			uint32_t numIndices;
			size_t hash;
		};
		size_t currhash;
		FrameData frames[MAX_FRAMES];//double-buffering
		uint32_t currFrame;
		GLuint _texture;
		GLuint _vao;
		ShaderUtil	_shader;
		glm::mat4	_orthoproj;

		Renderer::RenderDevice* _renderdevice;
		void Update();
	public:
		GLFont();
		~GLFont();
		virtual void Init(Renderer::RenderDevice* pdevice, const char* pfont, int fontSize) override;
		virtual void Draw(const char* ptext, int xpos, int ypos, glm::vec4 color) override;
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