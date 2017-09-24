#pragma once


#include "commonincludes.h"
#include "ShaderHelper.h"
#include "Shader.h"


class PixelManager
{
public:
	struct SVertex
	{
		DirectX::XMFLOAT2 Position;
		DirectX::XMFLOAT3 Color;
		SVertex( float x, float y, float r, float g, float b )
			: Position( x, y ), Color( r, g, b ) {};
	};
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;

	void * mStartAddress;
	int mCurrent;

	UINT mWindowWidth;
	UINT mWindowHeight;

	float mHalfWindowWidth;
	float mHalfWindowHeight;

	std::shared_ptr<Shader> mShader;

	Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> mContext;
public:
	PixelManager( ) = delete;
	PixelManager( Microsoft::WRL::ComPtr<ID3D11Device> Device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context,
		std::shared_ptr<Shader> Shader, UINT Pixels,
		UINT WindowWidth, UINT WindowHeight );
	~PixelManager( );
public:
	void Begin( );
	void Point( float x, float y, float r, float g, float b );
	void End( );
	void Render( );
};

