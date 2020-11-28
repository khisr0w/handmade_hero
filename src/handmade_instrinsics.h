/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  Undefined                                                     |
    |    Last Modified:  11/27/2020 5:11:22 AM                                         |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright © All rights reserved |======+  */

#if !defined(HANDMADE_INSTRINSICS_H)

// TODO Convert all to intrinsic
#include "math.h"
#define Pi32 3.141592653589

inline int32_t
SignOf(int32_t Value)
{
	return (Value >= 0) ? 1 : -1;
}

inline real32
SquareRoot(real32 Real32)
{
	real32 Result = sqrtf(Real32);

	return Result;
}

inline real32
AbsoluteValue(real32 Real32)
{
	real32 Result = (real32)fabs(Real32);
	return Result;
}

inline uint32_t
RotateLeft(uint32_t Value, int32_t Amount)
{
#if COMPILER_MSVC
	uint32_t Result = _rotl(Value, Amount);
#else
	Amount &= 31;
	uint32_t Result = ((Value << Amount) | (Value >> (32 - Amount)));
#endif
	return Result;
}

inline uint32_t
RotateRight(uint32_t Value, int32_t Amount)
{
#if COMPILER_MSVC
	uint32_t Result = _rotr(Value, Amount);
#else
	Amount &= 31;
	uint32_t Result = ((Value >> Amount) | (Value << (32 - Amount)));
#endif
	return Result;
}

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

inline int32_t
FloorReal32ToInt32(real32 Real32)
{
	return (int32_t)floorf(Real32);
}

inline int32_t
CeilReal32ToInt32(real32 Real32)
{
	return (int32_t)ceilf(Real32);
}

inline uint32_t
TruncateReal32ToUInt32(real32 Real32)
{
	return (uint32_t)(Real32);
}

inline real32 Sin(real32 Angle)
{
	return sinf(Angle);
}

inline real32 Cos(real32 Angle)
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

