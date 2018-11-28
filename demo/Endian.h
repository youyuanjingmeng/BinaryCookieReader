#ifndef _COMM_ENDIAN_H
#define _COMM_ENDIAN_H

#include <assert.h>
#include <cstdint>

class CENDIAN_CONVERT
{
public:
	template <typename T16>
	T16 swap16(const T16 &v)
	{
		assert(sizeof(T16) == 2);
		return (((v >> 8) & 0xff) 
			| (((v)& 0xff) << 8));
	}

	template <typename T32>
	T32 swap32(const T32 &v)
	{
		assert(sizeof(T32) == 4);
		return (((v)& 0xff000000u) >> 24) 
			| (((v)& 0x00ff0000u) >> 8)
			| (((v)& 0x0000ff00u) << 8) 
			| (((v)& 0x000000ffu) << 24);

	}

	template <typename T64>
	T64 swap64(const T64 &v)
	{
		assert(sizeof(T64) == 8);

		return  ((((x)& 0xff00000000000000ull) >> 56)
			| (((x)& 0x00ff000000000000ull) >> 40)
			| (((x)& 0x0000ff0000000000ull) >> 24)
			| (((x)& 0x000000ff00000000ull) >> 8)
			| (((x)& 0x00000000ff000000ull) << 8)
			| (((x)& 0x0000000000ff0000ull) << 24)
			| (((x)& 0x000000000000ff00ull) << 40)
			| (((x)& 0x00000000000000ffull) << 56))
	}
public:
	CENDIAN_CONVERT()
	{

	}
	~CENDIAN_CONVERT()
	{

	}
}ENDIAN_CONVERT;
#endif