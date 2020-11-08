union v2
{
	struct
	{
		real32 X, Y;
	};
	real32 E[2];
};
//real32 &operator[](int Index){ return ((&X)[Index]);}

inline v2
V2(real32 X, real32 Y)
{
	v2 Result;

	Result.X = X;
	Result.Y = Y;

	return Result;
}

inline v2
operator*(real32 A, v2 B)
{
	v2 Result;

	Result.X = A * B.X;
	Result.Y = A * B.Y;

	return Result;
}

inline v2
operator*(v2 B, real32 A)
{
	v2 Result;

	Result.X = A * B.X;
	Result.Y = A * B.Y;

	return Result;
}

inline v2
&operator*=(v2 &B, real32 A)
{
	B = B * A;

	return B;
}

inline v2
operator-(v2 A)
{
	v2 Result;

	Result.X = -A.X;
	Result.Y = -A.Y;
	
	return Result;
}

inline v2
operator+(v2 A, v2 B)
{
	v2 Result;

	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	
	return Result;
}

inline v2
&operator+=(v2 &A, v2 B)
{
	A = A + B;

	return A;
}
inline v2
operator-(v2 A, v2 B)
{
	v2 Result;

	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	
	return Result;
}

inline real32
Square(real32 A)
{
	real32 Result = A * A;
	return Result;
}

inline real32
Inner(v2 A, v2 B)
{
	real32 Result = A.X*B.X + A.Y*B.Y;
	return Result;
}

inline real32
LengthSq(v2 A)
{
	real32 Result = Inner(A, A);
	return Result;
}