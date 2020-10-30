#if !defined(HANDMADE_INSTRINSICS_H)

// TODO Convert all to intrinsic
#include "math.h"

inline int32_t
RoundReal32ToInt32(real32 Real32)
{
	return (int32_t)roundf(Real32);
}

inline uint32_t
RoundReal32ToUInt32(real32 Real32)
{
	return (uint32_t)roundf(Real32);
}

inline uint32_t
FloorReal32ToUInt32(real32 Real32)
{
	return (uint32_t)floorf(Real32);
}

inline uint32_t
TruncateReal32ToUInt32(real32 Real32)
{
	return (uint32_t)(Real32);
}

inline real32 sines(real32 Angle)
{
	return sinf(Angle);
}

inline real32 Cosine (real32 Angle)
{
	return cosf(Angle);
}

inline real32 ATan2(real32 Y, real32 X)
{
	atan2f(Y, X);
}

struct bit_scan_result
{
	bool32 Found;
	uint32_t Index;
};
// TODO move this into the intrinsics and implement the MSVC version of it
inline bit_scan_result
FindLeastSignificantSetBit(uint32_t Value)
{
	bit_scan_result Result = {};
#if COMPILER_MSVC
	Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
	for(uint32_t Test = 0;
		Test < 32;
		++Test)
	{
		if(Value & (1 << Test))
		{
			Result.Index = Test;
			Result.Found = true;
			break;
		}
	}
#endif
	return Result;
}

#define HANDMADE_INSTRINSICS_H
#endif

