#pragma once

namespace Mesh {
	//base class for all mesh types?
	class BaseMesh {
	public:
		virtual ~BaseMesh() = default;		
		virtual void Bind() = 0;
		virtual size_t GetHash() = 0;
	};
}