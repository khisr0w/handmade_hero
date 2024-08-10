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
	// TODO(Abid); Need gcc/LLVM equivalents!
#endif

inline int32
SignOf(int32 Value) {
    int32 Result = (Value >= 0) ? 1 : -1;
    return Result;
}

inline real32
SquareRoot(real32 Real32) {
    real32 Result = sqrtf(Real32);
    return Result;
}

inline real32
AbsoluteValue(real32 Real32) {
    real32 Result = (real32 )fabs(Real32);
    return Result;
}

inline uint32
RotateLeft(uint32 Value, int32 Amount) {
#if COMPILER_MSVC
    uint32 Result = _rotl(Value, Amount);
#else
    // TODO(Abid): Actually port this to other compiler platforms!
    Amount &= 31;
    uint32 Result = ((Value << Amount) | (Value >> (32 - Amount)));
#endif

    return Result;
}

inline uint32
RotateRight(uint32 Value, int32 Amount) {
#if COMPILER_MSVC
    uint32 Result = _rotr(Value, Amount);
#else
    // TODO(Abid): Actually port this to other compiler platforms!
    Amount &= 31;
    uint32 Result = ((Value >> Amount) | (Value << (32 - Amount)));
#endif

    return Result;
}

inline int32
RoundReal32ToInt32(real32 Real32) {
    int32 Result = (int32)roundf(Real32);
    return Result;
}

inline uint32
RoundReal32ToUInt32(real32 Real32) {
    uint32 Result = (uint32)roundf(Real32);
    return Result;
}

inline int32 
FloorReal32ToInt32(real32 Real32) {
    int32 Result = (int32)floorf(Real32);
    return Result;
}

inline int32 
CeilReal32ToInt32(real32 Real32) {
    int32 Result = (int32)ceilf(Real32);
    return Result;
}

inline int32
TruncateReal32ToInt32(real32 Real32) {
    int32 Result = (int32)Real32;
    return Result;
}

inline real32
Sin(real32 Angle) {
    real32 Result = sinf(Angle);
    return Result;
}

inline real32
Cos(real32 Angle) {
    real32 Result = cosf(Angle);
    return Result;
}

inline real32
ATan2(real32 Y, real32 X) {
    real32 Result = atan2f(Y, X);
    return Result;
}

struct bit_scan_result {
    bool32 Found;
    uint32 Index;
};
inline bit_scan_result
FindLeastSignificantSetBit(uint32 Value)
{
    bit_scan_result Result = {};

#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(uint32 Test = 0; Test < 32; ++Test) {
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
