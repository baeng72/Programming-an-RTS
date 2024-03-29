#include "GLFont.h"

#include "../../Core/Log.h"
#include <ft2build.h>
#include FT_FREETYPE_H
namespace GL {
	
	GLFont::GLFont()
		:_width(0),_height(0),invBmpWidth(0.f)
	{
		currhash = 0;
		bmpHeight = 0;
		currFrame = UINT32_MAX;
		for (int i = 0; i < MAX_FRAMES; i++) {
			frames[i].indexBuffer = -1;
			frames[i].indSize = 0;
			frames[i].vertexBuffer = -1;
			frames[i].vertSize = 0;
			frames[i].hash = SIZE_MAX;
		}
	}
	GLFont::~GLFont()
	{
		glDeleteVertexArrays(1,&_vao);
		for (int i = 0; i < MAX_FRAMES; i++) {
			glDeleteBuffers(1,&frames[i].vertexBuffer);
			glDeleteBuffers(1, &frames[i].indexBuffer);
		}
		glDeleteTextures(1, &_texture);
		
	}
	void GLFont::Init(Renderer::RenderDevice* pdevice, const char* pfont, int fontSize)
	{
		LOG_INFO("Compiling GLFont shaders...");
		const char* vertexSrc = R"(
#version 460 core
layout(location=0) in vec3 inPos;
layout(location=1) in vec4 inColor;
layout(location=2) in vec2 inTexCoords;
layout (location=0) out vec2 aTexCoords;
layout (location=1) out vec4 aColor;



uniform mat4 projection;

void main(){
	gl_Position = projection * vec4(inPos,1.0);
	aTexCoords = inTexCoords;
	aColor = inColor;
}
)";

		const char* fragmentSrc = R"(
#version 460 core
layout (location=0) in vec2 aTexCoords;
layout (location=1) in vec4 aColor;

out vec4 outFragColor;

uniform sampler2D text;

void main() {
	vec4 sampled = vec4(1.0,1.0,1.0,texture(text,aTexCoords).r);
	outFragColor = aColor * sampled;
}
)";
		_shader.compile(vertexSrc, nullptr, fragmentSrc);

		//build texture-atlas
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
					assert(pitch == width);
					for ( int i = 0;i< rows; i ++) {
						for ( int j = 0; j < width; j++) {
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
			uint32_t alignedWidth = bmpWidth + 3 & ~3;
			//OpenGL textures rows have to be multiple of 4
			std::vector<uint8_t> swap(bmpHeight * alignedWidth);
			for (uint32_t y = 0; y < bmpHeight; y++) {
				for (uint32_t x = 0; x < bmpWidth; x++) {
					uint8_t b = buffer[y * bmpWidth + x];
					swap[(bmpHeight - y-1) * alignedWidth + x] = b;
				}
			}
#if 0
			glCreateTextures(GL_TEXTURE_2D, 1, &_texture);
			glTextureParameteri(_texture, GL_TEXTURE_MAX_LEVEL, 0);
			glTextureParameteri(_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureStorage2D(_texture, 0, GL_RED, bmpWidth, bmpHeight);
			glTextureSubImage2D(_texture, 0, 0, 0, bmpWidth, bmpHeight, GL_RED, GL_UNSIGNED_BYTE, buffer);
#else
			glGenTextures(1, &_texture);			
			glBindTexture(GL_TEXTURE_2D, _texture);			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, bmpWidth, bmpHeight,0, GL_RED, GL_UNSIGNED_BYTE, swap.data());
			glGenerateMipmap(GL_TEXTURE_2D);
			//glBindTexture(GL_TEXTURE_2D, 0);
#endif
			delete[]buffer;
			FT_Done_FreeType(ft);
		}
		pdevice->GetDimensions(&_width, &_height);
#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__
		_orthoproj = vulkOrthoRH(0.f, (float)_width, 0.f, (float)_height, -1.f, 1.f);// glm::ortho(0.f, (float)width, (float)height, (float)0.f, -1.f, 1.f);
#else
		_orthoproj = glOrthoRH(0.f, (float)width, 0.f, (float)height, -1.f, 1.f);// glm::ortho(0.f, (float)width, (float)height, (float)0.f, -1.f, 1.f);
#endif
		//_orthoproj = glOrthoRH(0.f, (float)_width, (float)_height,0.f, -1.f, 1.f);
		GLuint vaos[2];
		glGenVertexArrays(2, vaos);
		frames[0].vao = vaos[0];
		frames[1].vao = vaos[1];
	}
	void GLFont::Update()
	{
		//hash vertices
		
		uint32_t vertSize = (uint32_t)( sizeof(FontVertex) * _vertices.size());
		uint32_t indSize = (uint32_t)( sizeof(uint32_t) * _indices.size());
		size_t hash = Core::HashFNV1A(_vertices.data(), vertSize);
		currFrame++;
		uint32_t frameIdx = currFrame % MAX_FRAMES;
		uint32_t maxSize = std::max(vertSize, indSize);

		FrameData& frame = frames[frameIdx];
		currFrame = frameIdx;
		glBindVertexArray(frame.vao);
		GLenum bufferDraw = GL_STREAM_DRAW;
		if (frame.hash == currhash) {
			glBindBuffer(GL_ARRAY_BUFFER, frame.vertexBuffer);
			
			glBufferData(GL_ARRAY_BUFFER, vertSize, nullptr, bufferDraw);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frame.indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, nullptr, bufferDraw);
			
			glBufferData(GL_ARRAY_BUFFER, vertSize, _vertices.data(), bufferDraw);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, _indices.data(), bufferDraw);
			GLERR();
			frame.indSize = indSize;
			frame.numIndices = (uint32_t)_indices.size();
		}
		else 
		{

			frame.hash = currhash;

			if (frame.vertSize == 0 || frame.vertSize < vertSize) {
				if (frame.vertexBuffer != -1) {
					glDeleteBuffers(1, &frame.vertexBuffer);

				}
				glGenBuffers(1, &frame.vertexBuffer);
				glBindBuffer(GL_ARRAY_BUFFER, frame.vertexBuffer);				
				glBufferData(GL_ARRAY_BUFFER, vertSize, _vertices.data(), bufferDraw);								
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FontVertex), 0);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(FontVertex), (void*)sizeof(vec3));
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FontVertex), (void*)(sizeof(vec3) + sizeof(vec4)));
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);
				GLERR();
			}
			else if (vertSize > 0) {
				glBindBuffer(GL_ARRAY_BUFFER, frame.vertexBuffer);				
				glBufferData(GL_ARRAY_BUFFER, vertSize, _vertices.data(), bufferDraw);				
				GLERR();
			}
			frame.vertSize = vertSize;
			if (frame.indSize == 0 || frame.indSize < indSize) {
				if (frame.indexBuffer != -1) {
					glDeleteBuffers(1, &frame.indexBuffer);
				}
				glGenBuffers(1, &frame.indexBuffer);				
			}
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frame.indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, _indices.data(), bufferDraw);
			GLERR();
			frame.indSize = indSize;
			frame.numIndices = (uint32_t)_indices.size();
		}
		
		_vertices.clear();
		_indices.clear();
		currhash = 0;
		
	}
	void GLFont::Draw(const char* ptext, int xpos, int ypos, glm::vec4 color)
	{
		

		//std::vector<FontVertex> vertices;
		//std::vector<uint32_t> indices;
		uint32_t indexoffset = (uint32_t)_vertices.size();
		size_t len = strlen(ptext);

		size_t texthash = Core::HashFNV1A(ptext, len);
		if (currhash == 0)
			currhash = texthash;
		else {
			currhash *= Core::fnvprime;
			currhash *= texthash;
		}

		float x = (float)xpos;
		float y = (float)ypos;
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

			FontVertex topleft = { {xpos,ypos,0.0f},color,{u0,1.f} };
			FontVertex topright = { { xpos + w,ypos,0.0f},color,{u1,1.f} };
			FontVertex bottomleft = { { xpos,(ypos + h),0.0f},color,{u0,1.f-v} };
			FontVertex bottomright = { {xpos + w,(ypos + h),0.0f},color,{u1,1.f-v} };
			_vertices.push_back(topleft);
			_vertices.push_back(topright);
			_vertices.push_back(bottomleft);
			_vertices.push_back(bottomright);
			_indices.push_back(indexoffset + 0);
			_indices.push_back(indexoffset + 1);
			_indices.push_back(indexoffset + 2);
			_indices.push_back(indexoffset + 1);
			_indices.push_back(indexoffset + 3);
			_indices.push_back(indexoffset + 2);
			indexoffset += 4;
			x += (character.advance >> 6) * scale;

		}
		//_vertices.insert(_vertices.end(), vertices.begin(), vertices.end());
		//_indices.insert(_indices.end(), indices.begin(), indices.end());

	}
	void GLFont::Render()
	{
		Update();
		FrameData& frame = frames[currFrame];
		
		//glBindTexture(GL_TEXTURE_2D, _texture);		
		//glBindVertexArray(_vao);
		//glUseProgram(_shader);
		//glDisable(GL_DEPTH_TEST);
		GLint polygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
		if(polygonMode!=GL_FILL)
			glPolygonMode(GL_POLYGON_MODE, GL_FILL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		_shader.Bind();
		_shader.setMat4("projection", _orthoproj);
		_shader.SetTextures("text",(int*)&_texture,1u);
		//glFrontFace(GL_CW);
		glDrawElements(GL_TRIANGLES, frame.numIndices, GL_UNSIGNED_INT, 0);
		if(polygonMode!=GL_FILL)
			glPolygonMode(GL_POLYGON_MODE, polygonMode);
		glBindVertexArray(0);
		GLERR();
	}
	void GLFont::GetTextSize(const char* ptext, float& width, float& height)
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
	void GLFont::SetDimensions(int width, int height)
	{
		_width = width;
		_height = height;
		//_orthoproj = glm::ortho(0.f, (float)_width, 0.f, (float)_height, -1.f, 1.f);
#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__
		_orthoproj = vulkOrthoRH(0.f, (float)_width, 0.f, (float)_height, -1.f, 1.f);// glm::ortho(0.f, (float)width, (float)height, (float)0.f, -1.f, 1.f);
#else
		_orthoproj = glOrthoRH(0.f, (float)width, 0.f, (float)height, -1.f, 1.f);// glm::ortho(0.f, (float)width, (float)height, (float)0.f, -1.f, 1.f);
#endif
		//_orthoproj = glm::orthoRH_NO(0.f, (float)_width, (float)_height, 0.f, -1.f, 1.f);
	}
}