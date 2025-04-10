#pragma once

#include <cstddef>
#include <cstdint>

using U8 = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;
using USize = size_t;

using I8 = int8_t;
using I16 = int16_t;
using I32 = int32_t;
using I64 = int64_t;
using ISize = ptrdiff_t;

#ifdef __GNUC__
	#define alwaysInline\
		__attribute__((always_inline)) inline
	#define likely(x)\
		__builtin_expect(!!(x), true)
	#define unlikely(x)\
		__builtin_expect(!!(x), false)
#else
	#define alwaysInline\
		inline
#endif

#ifdef __clang__
	#define tailCall(x)\
		__attribute__((musttail)) return x
#else
	#define tailCall(x)\
		return x
#endif
