#pragma once


#include "Ray.h"


class IShape
{
public:
	virtual bool Intersect( Ray& r ) = 0;
};

class Sphere : IShape
{
public:
	Sphere( ) = default;
	Sphere( DirectX::XMFLOAT3 Center, float Radius ) :
		mCenter( Center ), mRadius( Radius ) {};
	DirectX::XMFLOAT3 mCenter;
	float mRadius;
public:
	bool Intersect( Ray& r )
	{
		using namespace DirectX;

		XMVECTOR Center = XMLoadFloat3( &mCenter );
		XMVECTOR RayDir = XMLoadFloat3( &r.mDirection );
		XMVECTOR RayStart = XMLoadFloat3( &r.mStart );

		XMVECTOR OriginToSphere = Center - RayStart;
		float projection = XMVectorGetX( XMVector3Dot( OriginToSphere, RayDir ) );
		XMVECTOR distanceVector = OriginToSphere - projection * RayDir;
		float distanceVectorLengthSQ = XMVectorGetX( XMVector3Dot( distanceVector, distanceVector ) );
		float radiusSQ = mRadius * mRadius;
		if ( distanceVectorLengthSQ > radiusSQ )
			return false;
		
		float newLength = projection - sqrtf( radiusSQ -distanceVectorLengthSQ );
		if ( newLength < 0.0f || newLength > r.mLength )
			return false;

		r.mLength = newLength;

		return true;
	}
};

class Plane : IShape
{
public:
	Plane( ) = default;
	Plane( DirectX::XMFLOAT3 KnownPoint, DirectX::XMFLOAT3 Normal )
		:mCenter( KnownPoint ), mNormal( Normal ) {};

	DirectX::XMFLOAT3 mCenter;
	DirectX::XMFLOAT3 mNormal;
public:
	[[ deprecated ]]
	bool Intersect( Ray& r )
	{
		using namespace DirectX;

		XMVECTOR PlaneNormal = XMLoadFloat3( &mNormal );
		XMVECTOR KnownPoint = XMLoadFloat3( &mCenter );
		XMVECTOR RayDir = XMLoadFloat3( &r.mDirection );
		XMVECTOR RayStart = XMLoadFloat3( &r.mStart );

		float denom = XMVectorGetX( XMVector3Dot( RayDir, PlaneNormal ) );

		if ( denom > DX::EPSILON )
		{
			float t = XMVectorGetX( XMVector3Dot( ( KnownPoint - RayStart ), PlaneNormal ) ) / denom;
			if ( t >= 0 )
			{
				return true;
			}
		}

		return false;
	}
};