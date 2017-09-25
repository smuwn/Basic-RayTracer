#pragma once


#include <DirectXMath.h>



class Light
{
public:

	Light( DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Diffuse ) :
		mPosition( Position ), mDiffuse( Diffuse )
	{
	}

	DirectX::XMFLOAT3 mPosition;
	DirectX::XMFLOAT3 mDiffuse;

};

