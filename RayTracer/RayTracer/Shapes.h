#pragma once


#include "Ray.h"


class IShape
{
public:
	virtual bool Intersect( Ray& r ) = 0;
	virtual DirectX::XMVECTOR GetNormal( DirectX::FXMVECTOR const& hitPoint ) = 0;
	virtual float GetTransparency( ) = 0;
	virtual float GetReflectivity( ) = 0;
	virtual DirectX::XMFLOAT3 GetColor( ) = 0;
};

class Sphere : IShape
{
public:
	Sphere( ) = default;
	Sphere( DirectX::XMFLOAT3 Center, float Radius, DirectX::XMFLOAT3 Color ) :
		mCenter( Center ), mRadius( Radius ), mColor( Color ) {};
	DirectX::XMFLOAT3 mCenter;
	DirectX::XMFLOAT3 mColor;
	float mRadius;
	float mTransparency = 0.0f;
	float mReflectivity = 0.0f;
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

	DirectX::XMVECTOR GetNormal( DirectX::FXMVECTOR const& hitPoint )
	{
		using namespace DirectX;
		return hitPoint - XMLoadFloat3( &mCenter );

	}
	DirectX::XMFLOAT3 GetColor( )
	{
		return mColor;
	}
	DirectX::XMFLOAT3 GetCenter( )
	{
		return mCenter;
	}
	float GetTransparency( )
	{
		return mTransparency;
	}
	float GetReflectivity( )
	{
		return mReflectivity;
	}
};


class Triangle : IShape
{
public:
	Triangle( DirectX::XMFLOAT3 V0,
		DirectX::XMFLOAT3 V1,
		DirectX::XMFLOAT3 V2,
		DirectX::XMFLOAT3 Normal,
		DirectX::XMFLOAT3 Color )
		: V0( V0 ), V1( V1 ), V2( V2 ), mNormal( Normal ), Color( Color )
	{};

	DirectX::XMFLOAT3 V0;
	DirectX::XMFLOAT3 V1;
	DirectX::XMFLOAT3 V2;
	DirectX::XMFLOAT3 mNormal;

	DirectX::XMFLOAT3 Color;

	float Transparency = 0.0f;
	float Reflectivity = 0.0f;

public:
	static inline bool isPointInTriangle( DirectX::XMFLOAT3 const& V1,
		DirectX::XMFLOAT3 const& V2, DirectX::XMFLOAT3 const& V3,
		DirectX::XMFLOAT3 const& Point )
	{
		using namespace DirectX;
		XMVECTOR A = DirectX::XMLoadFloat3( &V1 );
		XMVECTOR B = DirectX::XMLoadFloat3( &V2 );
		XMVECTOR C = DirectX::XMLoadFloat3( &V3 );
		XMVECTOR P = DirectX::XMLoadFloat3( &Point );

		XMVECTOR v0 = C - A;
		XMVECTOR v1 = B - A;
		XMVECTOR v2 = P - A;

		float dot00 = XMVectorGetX( XMVector3Dot( v0, v0 ) );
		float dot01 = XMVectorGetX( XMVector3Dot( v0, v1 ) );
		float dot02 = XMVectorGetX( XMVector3Dot( v0, v2 ) );
		float dot11 = XMVectorGetX( XMVector3Dot( v1, v1 ) );
		float dot12 = XMVectorGetX( XMVector3Dot( v1, v2 ) );

		float invDenom = 1.0f / ( dot00 * dot11 - dot01 * dot01 );
		float u, v;
		u = ( dot11 * dot02 - dot01 * dot12 ) * invDenom;
		v = ( dot00 * dot12 - dot01 * dot02 ) * invDenom;

		return u >= 0 && v >= 0 && u + v < 1;
	}


public:

	bool Intersect( Ray& r )
	{
		using namespace DirectX;

		XMVECTOR Normal = XMLoadFloat3( &mNormal );
		XMVECTOR RayOrigin = XMLoadFloat3( &r.mStart );
		XMVECTOR RayDirection = XMLoadFloat3( &r.mDirection );
		XMVECTOR PointInTriangle = XMLoadFloat3( &V0 );

		float NormalDotDirection = XMVectorGetX( XMVector3Dot( Normal, RayDirection ) );

		if ( fabs( NormalDotDirection ) < DX::EPSILON )
			return false;

		float d = XMVectorGetX( XMVector3Dot( Normal, PointInTriangle ) );

		float t = -( XMVectorGetX( XMVector3Dot( RayOrigin, Normal ) ) + d ) / NormalDotDirection;
		if ( t < 0 )
			return false;

		XMVECTOR hitPoint = RayOrigin + t * RayDirection;

		XMFLOAT3 hitPointXM;
		DirectX::XMStoreFloat3( &hitPointXM, hitPoint );

		if ( isPointInTriangle( V0, V1, V2, hitPointXM ) && r.mLength > t )
		{
			r.mLength = t;
			return true;
		}


		return false;
	}

	DirectX::XMFLOAT3 GetColor( )
	{
		return Color;
	}
	float GetTransparency( )
	{
		return Transparency;
	}
	float GetReflectivity( )
	{
		return Reflectivity;
	}
	DirectX::XMFLOAT3 GetCenter( )
	{
		/*
		C
		|\
		| \
		|  \
		|---B
		A
		*/
		using namespace DirectX;
		XMVECTOR AB;
		XMVECTOR AC;

		AB = XMLoadFloat3( &V1 ) - XMLoadFloat3( &V0 );
		AC = XMLoadFloat3( &V2 ) - XMLoadFloat3( &V0 );

		XMFLOAT3 ret;
		DirectX::XMStoreFloat3( &ret, 0.5f * AB + 0.5f * AC );

		return ret;
	}
	DirectX::XMVECTOR GetNormal( DirectX::FXMVECTOR& )
	{
		return DirectX::XMLoadFloat3( &mNormal );
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