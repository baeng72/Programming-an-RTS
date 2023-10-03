#include "ShapeUtils.h"

namespace  Mesh {
	Shape* Shape::Create(Renderer::RenderDevice* pdevice) {
		return new Utils::ShapeUtils(pdevice);
	}
}

namespace Utils {


	ShapeUtils::ShapeUtils(Renderer::RenderDevice* pdevice):_pdevice(pdevice)
	{
	}

	ShapeUtils::~ShapeUtils()
	{
	}

	Mesh::Mesh* ShapeUtils::CreateCube(float side) {
		struct PosNormUV {
			vec3 pos;
			vec3 norm;
			vec2 uv;
			PosNormUV(vec3 p, vec3 n, vec2 uv_) {
				pos = p;
				norm = n;
				uv = uv_;
			}
			PosNormUV() {
				pos = norm = vec3(0.f);
				uv = vec2(0.f);
			}
		};
		std::vector<PosNormUV> vertices(24);
		// Fill in the front face vertex data.
		vertices[0] = PosNormUV(vec3(-side*0.5f, -side*0.5f, -side*0.5f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 1.0f));
		vertices[1] = PosNormUV(vec3(-side*0.5f, +side*0.5f, -side*0.5f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 0.0f));
		vertices[2] = PosNormUV(vec3(+side*0.5f, +side*0.5f, -side*0.5f), vec3(0.0f, 0.0f, -1.0f), vec2(1.0f, 0.0f));
		vertices[3] = PosNormUV(vec3(+side*0.5f, -side*0.5f, -side*0.5f), vec3(0.0f, 0.0f, -1.0f), vec2(1.0f, 1.0f));

		// Fill in the back face verticesertex data.
		vertices[4] = PosNormUV(vec3(-side*0.5f, -side*0.5f, +side*0.5f), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 1.0f));
		vertices[5] = PosNormUV(vec3(+side*0.5f, -side*0.5f, +side*0.5f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 1.0f));
		vertices[6] = PosNormUV(vec3(+side*0.5f, +side*0.5f, +side*0.5f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f));
		vertices[7] = PosNormUV(vec3(-side*0.5f, +side*0.5f, +side*0.5f), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 0.0f));

		// Fill in the top face vertex data.
		vertices[8] = PosNormUV(vec3(-side*0.5f, +side*0.5f, -side*0.5f), vec3(0.0f, 1.0f, 0.0f), vec2(0.0f, 1.0f));
		vertices[9] = PosNormUV(vec3(-side*0.5f, +side*0.5f, +side*0.5f), vec3(0.0f, 1.0f, 0.0f), vec2(0.0f, 0.0f));
		vertices[10] = PosNormUV(vec3(+side*0.5f, +side*0.5f, +side*0.5f), vec3(0.0f, 1.0f, 0.0f), vec2(1.0f, 0.0f));
		vertices[11] = PosNormUV(vec3(+side*0.5f, +side*0.5f, -side*0.5f), vec3(0.0f, 1.0f, 0.0), vec2(1.0f, 1.0f));

		// Fill in the bottom face vertex data.
		vertices[12] = PosNormUV(vec3(-side*0.5f, -side*0.5f, -side*0.5f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 1.0f));
		vertices[13] = PosNormUV(vec3(+side*0.5f, -side*0.5f, -side*0.5f), vec3(0.0f, -1.0f, 0.0f), vec2( 0.0f, 1.0f));
		vertices[14] = PosNormUV(vec3(+side*0.5f, -side*0.5f, +side*0.5f), vec3(0.0f, -1.0f, 0.0f), vec2(0.0f, 0.0f));
		vertices[15] = PosNormUV(vec3(-side*0.5f, -side*0.5f, +side*0.5f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 0.0f));

		// Fill in the left face vertex data.
		vertices[16] = PosNormUV(vec3(-side*0.5f, -side*0.5f, +side*0.5f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 1.0f));
		vertices[17] = PosNormUV(vec3(-side*0.5f, +side*0.5f, +side*0.5f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f));
		vertices[18] = PosNormUV(vec3(-side*0.5f, +side*0.5f, -side*0.5f), vec3(-1.0f, 0.0f, 0.0f), vec2(1.0f, 0.0f));
		vertices[19] = PosNormUV(vec3(-side*0.5f, -side*0.5f, -side*0.5f), vec3(-1.0f, 0.0f, 0.0f), vec2(1.0f, 1.0f));

		// Fill in the right face verticesertex data.
		vertices[20] = PosNormUV(vec3(+side*0.5f, -side*0.5f, -side*0.5f), vec3(1.0f, 0.0f, 0.0f), vec2(0.0f, 1.0f));
		vertices[21] = PosNormUV(vec3(+side*0.5f, +side*0.5f, -side*0.5f), vec3(1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f));
		vertices[22] = PosNormUV(vec3(+side*0.5f, +side*0.5f, +side*0.5f), vec3(1.0f, 0.0f, 0.0f), vec2(1.0f, 0.0f));
		vertices[23] = PosNormUV(vec3(+side*0.5f, -side*0.5f, +side*0.5f), vec3(1.0f, 0.0f, 0.0f), vec2(1.0f, 1.0f));

		std::vector<uint32_t> indices(36);

		// Fill in the front face index data
		indices[0] = 0; indices[1] = 1; indices[2] = 2;
		indices[3] = 0; indices[4] = 2; indices[5] = 3;

		// Fill in the back face index data
		indices[6] = 4; indices[7] = 5; indices[8] = 6;
		indices[9] = 4; indices[10] = 6; indices[11] = 7;

		// Fill in the top face index data
		indices[12] = 8; indices[13] = 9; indices[14] = 10;
		indices[15] = 8; indices[16] = 10; indices[17] = 11;

		// Fill in the bottom face index data
		indices[18] = 12; indices[19] = 13; indices[20] = 14;
		indices[21] = 12; indices[22] = 14; indices[23] = 15;

		// Fill in the left face index data
		indices[24] = 16; indices[25] = 17; indices[26] = 18;
		indices[27] = 16; indices[28] = 18; indices[29] = 19;

		// Fill in the right face index data
		indices[30] = 20; indices[31] = 21; indices[32] = 22;
		indices[33] = 20; indices[34] = 22; indices[35] = 23;

		uint32_t vertSize = (uint32_t)(sizeof(PosNormUV) * vertices.size());
		uint32_t indSize = (uint32_t)(sizeof(uint32_t) * indices.size());
		Renderer::VertexAttributes attributes = { {Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float2},sizeof(PosNormUV) };
		
		Mesh::Mesh* pmesh = Mesh::Mesh::Create(_pdevice, (float*)vertices.data(), vertSize, indices.data(), indSize,attributes);
		return pmesh;
	}

	Mesh::Mesh* ShapeUtils::CreateSphere(float radius, int32_t slices, int32_t stacks)
	{
		struct PosNormUV {
			vec3 pos;
			vec3 norm;
			vec2 uv;
			PosNormUV(vec3 p, vec3 n, vec2 uv_) {
				pos = p;
				norm = n;
				uv = uv_;
			}
		};
		//code based on https://gist.github.com/Pikachuxxxx/5c4c490a7d7679824e0e18af42918efc
		if (slices < 3)
			slices = 3;
		if (stacks < 2)
			stacks = 2;
		/*std::vector<vec3> vertices;
		std::vector<vec3> normals;
		std::vector<vec2> uvs;*/
		std::vector<PosNormUV> vertices;
		std::vector<uint32_t> indices;

		float nx, ny, nz, lengthInv = 1.f / radius;		//normal
		
		float deltaSlice = glm::pi<float>() / (float)slices;
		float deltaStack = 2.f * glm::pi<float>() / (float) stacks;
		
		float sliceAngle;
		float stackAngle;

		//Compute all vertices first, except normals
		for (int i = 0; i <= slices; i++) {
			sliceAngle = glm::pi<float>() / 2 - i * deltaSlice;	//starting -pi/2 to pi/2
			float xy = radius * cosf(sliceAngle);				// r * cos(phi)
			float z = radius * sinf(sliceAngle);				// r * sin(phi)
			/*
		 * We add (slices + 1) vertices per stack because of equator,
		 * the North pole and South pole are not counted here, as they overlap.
		 * The first and last vertices have same position and normal, but
		 * different tex coords.
		 */
			for (int j = 0; j <= stacks; j++) {
				stackAngle = j * deltaStack;

				vec3 pos;
				pos.x = xy * cosf(stackAngle);		//x = r * cos(phi) * cos(theta)
				pos.y = xy * sinf(stackAngle);		//y = r * cos(phi) * sin(theta);
				pos.z = z;
				vec2 uv;
				uv.s = (float)j / (float)stacks;	//u
				uv.t = (float)i / (float)slices;	//v

				
				

				//normalized vertex normal
				nx = pos.x * lengthInv;
				ny = pos.y * lengthInv;
				nz = pos.z * lengthInv;
				vec3 norm(nx, ny, nz);
				vertices.push_back({ pos, norm, uv });

			}

		}
		/*
	 *  Indices
	 *  k1--k1+1
	 *  |  / |
	 *  | /  |
	 *  k2--k2+1
	 */
		unsigned int k1, k2;
		for (int i = 0; i < slices; ++i)
		{
			k1 = i * (stacks + 1);
			k2 = k1 + stacks + 1;
			// 2 Triangles per latitude block excluding the first and last longitudes blocks
			for (int j = 0; j < stacks; ++j, ++k1, ++k2)
			{
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				if (i != (slices - 1))
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}

		uint32_t vertSize = (uint32_t)(sizeof(PosNormUV) * vertices.size());
		uint32_t indSize = (uint32_t)(sizeof(uint32_t) * indices.size());
		Renderer::VertexAttributes attributes = { {Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float2},sizeof(PosNormUV) };
		Mesh::Mesh* pmesh = Mesh::Mesh::Create(_pdevice, (float*)vertices.data(), vertSize, indices.data(), indSize,attributes);
		return pmesh;
	}

}
