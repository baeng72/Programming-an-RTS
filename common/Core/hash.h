#pragma once

namespace Core{
	constexpr size_t fnvprime = 1099511628211;
	constexpr size_t fnvoffset = 14695981039346656037;
	size_t HashFNV1A(const void* data, size_t numBytes);
#define HASH(a) (Core::HashFNV1A(a,sizeof(a)))
}