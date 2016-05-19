#ifndef __3DTYPES_H__
#define __3DTYPES_H__

#include <math.h>

class Vertex
{
public:
	Vertex( float x = 0,
	        float y = 0,
	        float z = 0)
	: x(x), y(y), z(z) {}

	Vertex operator-() const // Minus operator
	{
		return Vertex(-x, -y, -z);
	}
	Vertex operator+(const Vertex& right) const // Addition
	{
		return Vertex(x + right.x, y + right.y, z + right.z);
	}
	Vertex operator-(const Vertex& right) const // Subtraction
	{
		return Vertex(x - right.x, y - right.y, z - right.z);
	}
	Vertex operator*(const float& right) const // Multiplication
	{
		return Vertex(x * right, y * right, z * right);
	}
	Vertex operator/(const float& right) const // Division
	{
		return Vertex(x / right, y / right, z / right);
	}

	bool Match(const Vertex& vTest, double dTol = 0.05) const
	{
		return fabs(x - vTest.x) <= dTol &&
			   fabs(y - vTest.y) <= dTol &&
			   fabs(z - vTest.z) <= dTol;
	}

	static double Tangent(const Vertex& v1, const Vertex& v2) // 2D tangent
	{
		return atan2(v1.z - v2.z, v1.x - v2.x);
	}

	static float Distance(const Vertex& v1, const Vertex& v2)
	{
		Vertex v = v1 - v2;
		return (float) sqrt( v.x*v.x + v.y*v.y + v.z*v.z );
	}

	float x;
	float y;
	float z;
};

class Vector : public Vertex
{
public:
	Vector( float x = 0,
	        float y = 0,
	        float z = 0)
	: Vertex(x, y, z) {}

	Vector(const Vertex& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	Vector operator*(const float& right) const // Multiplication
	{
		return Vector(x * right, y * right, z * right);
	}
	Vector operator/(const float& right) const // Division
	{
		return Vector(x / right, y / right, z / right);
	}
	Vector operator*(const Vector& b) const // Multiplication (cross product)
	{
		return Vector( y * b.z - z * b.y,
		               z * b.x - x * b.z,
		               x * b.y - y * b.x );
	}

	float Length(void) const
	{
		return (float) sqrt( pow(x,2) + pow(y,2) + pow(z,2) );
	}

	void Normalize(void)
	{
		float fLength = Length();
		if (fLength > 0)
		{
			x /= fLength;
			y /= fLength;
			z /= fLength;
		}
	}
};

#endif
