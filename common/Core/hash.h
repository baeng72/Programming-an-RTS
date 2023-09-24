#pragma once

namespace Core{
	
	size_t HashFNV1A(const void* data, size_t numBytes);
#define HASH(a) (Core::HashFNV1A(a,sizeof(a)))
}