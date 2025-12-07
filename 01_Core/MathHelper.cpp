//***************************************************************************************
// MathHelper.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "MathHelper.h"
#include <float.h>
#include <cmath>

using namespace DirectX;

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi       = 3.1415926535f;

float MathHelper::AngleFromXY(float x, float y)
{
	float theta = 0.0f;
 
	// Quadrant I or IV
	if(x >= 0.0f) 
	{
		// If x = 0, then atanf(y/x) = +pi/2 if y > 0
		//                atanf(y/x) = -pi/2 if y < 0
		theta = atanf(y / x); // in [-pi/2, +pi/2]

		if(theta < 0.0f)
			theta += 2.0f*Pi; // in [0, 2*pi).
	}

	// Quadrant II or III
	else      
		theta = atanf(y/x) + Pi; // in [0, 2*pi).

	return theta;
}

// Quaternion -> Euler
XMFLOAT3 MathHelper::QuaternionToEuler(const DirectX::XMVECTOR& q)
{
	XMFLOAT4 quat;
	XMStoreFloat4(&quat, q);

	float ysqr = quat.y * quat.y;

	// roll (x-axis rotation)
	float t0 = +2.0f * (quat.w * quat.x + quat.y * quat.z);
	float t1 = +1.0f - 2.0f * (quat.x * quat.x + ysqr);
	float roll = std::atan2(t0, t1);

	// pitch (y-axis rotation)
	float t2 = +2.0f * (quat.w * quat.y - quat.z * quat.x);
	t2 = MathHelper::Clamp(t2, -1.0f, +1.0f);
	float pitch = std::asin(t2);

	// yaw (z-axis rotation)
	float t3 = +2.0f * (quat.w * quat.z + quat.x * quat.y);
	float t4 = +1.0f - 2.0f * (ysqr + quat.z * quat.z);
	float yaw = std::atan2(t3, t4);

	auto FixZero = [](float v) { return (std::abs(v) < 1e-6f) ? 0.0f : v; };

	return { FixZero(pitch), FixZero(yaw), FixZero(roll) };
}

void MathHelper::WrapAngle360(float& angle)
{
	angle = fmodf(angle, 360.0f); // 360으로 나눈 나머지
	if (angle < 0.0f)             // 음수면 0~360 범위로 보정
		angle += 360.0f;
}

XMVECTOR MathHelper::RandUnitVec3()
{
	XMVECTOR One  = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	// Keep trying until we get a point on/in the hemisphere.
	while(true)
	{
		// Generate random point in the cube [-1,1]^3.
		XMVECTOR v = XMVectorSet(MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), 0.0f);

		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.

		if( XMVector3Greater( XMVector3LengthSq(v), One) )
			continue;

		return XMVector3Normalize(v);
	}
}

XMVECTOR MathHelper::RandHemisphereUnitVec3(XMVECTOR n)
{
	XMVECTOR One  = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	// Keep trying until we get a point on/in the hemisphere.
	while(true)
	{
		// Generate random point in the cube [-1,1]^3.
		XMVECTOR v = XMVectorSet(MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), 0.0f);

		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.
		
		if( XMVector3Greater( XMVector3LengthSq(v), One) )
			continue;

		// Ignore points in the bottom hemisphere.
		if( XMVector3Less( XMVector3Dot(n, v), Zero ) )
			continue;

		return XMVector3Normalize(v);
	}
}