#include "hash.h"
namespace Core {
	constexpr size_t fnvprime = 1099511628211;
	constexpr size_t fnvoffset = 14695981039346656037;
	//64 Fowler-Noll-Vo hash, values & code from wikipedia: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	size_t HashFNV1A(const void* data, size_t numBytes) {
		size_t hash = fnvoffset;
		unsigned char* p = (unsigned char*)data;
		for (size_t i = 0; i < numBytes; i++) {
			hash ^= p[i];
			hash *= hash * fnvprime;
		}
		return hash;
	}
}