#pragma once

#include <DirectXMath.h>


class Ray
{
	friend class IShape;
public:
	Ray( ) = default;
	Ray( DirectX::XMFLOAT3 Start, DirectX::XMFLOAT3 Direction, float Length ) :
		mStart( Start ), mDirection( Direction ), mLength( Length ) {};
	DirectX::XMFLOAT3 mStart;
	DirectX::XMFLOAT3 mDirection;
	float mLength;
};