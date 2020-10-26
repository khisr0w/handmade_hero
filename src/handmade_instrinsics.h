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

#define HANDMADE_INSTRINSICS_H
#endif

