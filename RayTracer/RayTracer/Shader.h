#pragma once

#include "ShaderHelper.h"


class Shader
{
	Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
	std::array<Microsoft::WRL::ComPtr<ID3DBlob>,2> mBlobs;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> mLayout;

	Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> mContext;
public:
	Shader( ) = delete;
	Shader( Microsoft::WRL::ComPtr<ID3D11Device> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context );
	~Shader( );
public:
	void SetShaders( );
	void Draw( UINT VertexCount, UINT OffsetVertex );
};

