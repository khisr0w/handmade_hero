/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  Undefined                                                     |
    |    Last Modified:  11/27/2020 5:11:22 AM                                         |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright © All rights reserved |======+  */

#if !defined(HANDMADE_INSTRINSICS_H)

//
// TODO(Abid): Convert all of these to platform-efficient versions
// and remove math.h
//

#include "math.h"

#if COMPILER_MSVC
#define CompletePreviousWritesBeforeFutureWrites _WriteBarrier()
// NOTE(Abid): volatile; it essentially tells the compiler that the value of the variable can change without action from the visible code.
inline uint32 AtomicCompareExchangeUInt32(uint32 volatile *Value, uint32 Expected, uint32 New) {
    uint32 Result = _InterlockedCompareExchange((long *)Value, Expected, New);
    return (Result);
}
#else
	// TODO(Abid); Need gcc/clang equivalents!
#endif

inline f32
SignOf(f32 Value) {
    f32 Result = (Value >= 0.f) ? 1.0f : -1.0f;
    return Result;
}
inline i32
SignOf(i32 Value) {
    i32 Result = (Value >= 0) ? 1 : -1;
    return Result;
}

inline f32
SquareRoot(f32 Real32) {
    real32 Result = sqrtf(Real32);
    return Result;
}

inline f32
AbsoluteValue(f32 Real32) {
    real32 Result = (f32 )fabs(Real32);
    return Result;
}

inline u32
RotateLeft(uint32 Value, i32 Amount) {
#if COMPILER_MSVC
    u32 Result = _rotl(Value, Amount);
#else
    // TODO(Abid): Actually port this to other compiler platforms!
    Amount &= 31;
    u32 Result = ((Value << Amount) | (Value >> (32 - Amount)));
#endif

    return Result;
}

inline u32
RotateRight(u32 Value, i32 Amount) {
#if COMPILER_MSVC
    u32 Result = _rotr(Value, Amount);
#else
    // TODO(Abid): Actually port this to other compiler platforms!
    Amount &= 31;
    u32 Result = ((Value >> Amount) | (Value << (32 - Amount)));
#endif

    return Result;
}

inline i32
RoundReal32ToInt32(f32 Real32) {
    i32 Result = (i32)roundf(Real32);
    return Result;
}

inline u32
RoundReal32ToUInt32(f32 Real32) {
    u32 Result = (u32)roundf(Real32);
    return Result;
}

inline i32 
FloorReal32ToInt32(f32 Real32) {
    i32 Result = (i32)floorf(Real32);
    return Result;
}

inline i32 
CeilReal32ToInt32(f32 Real32) {
    i32 Result = (i32)ceilf(Real32);
    return Result;
}

inline i32
TruncateReal32ToInt32(f32 Real32) {
    i32 Result = (i32)Real32;
    return Result;
}

inline f32
Sin(f32 Angle) {
    f32 Result = sinf(Angle);
    return Result;
}

inline f32
Cos(f32 Angle) {
    f32 Result = cosf(Angle);
    return Result;
}

inline f32
ATan2(f32 Y, f32 X) {
    f32 Result = atan2f(Y, X);
    return Result;
}

struct bit_scan_result {
    bool32 Found;
    u32 Index;
};
inline bit_scan_result
FindLeastSignificantSetBit(u32 Value)
{
    bit_scan_result Result = {};

#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(u32 Test = 0; Test < 32; ++Test) {
        if(Value & (1 << Test)) {
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
