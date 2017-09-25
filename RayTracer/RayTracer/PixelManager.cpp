#include "PixelManager.h"



PixelManager::PixelManager( Microsoft::WRL::ComPtr<ID3D11Device> Device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context,
	std::shared_ptr<Shader> Shader, UINT Pixels,
	UINT WindowWidth, UINT WindowHeight ) :
	mDevice( Device ),
	mContext( Context ),
	mShader( Shader ),
	mWindowWidth( WindowWidth ),
	mWindowHeight( WindowHeight ),
	mHalfWindowWidth( float( WindowWidth ) / 2.0f ),
	mHalfWindowHeight( float( WindowHeight ) / 2.0f )
{
	try
	{
		ShaderHelper::CreateBuffer(
			mDevice.Get( ), &mVertexBuffer, D3D11_USAGE::D3D11_USAGE_DYNAMIC,
			D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER, Pixels * sizeof( SVertex ),
			D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE );
	}
	CATCH;
}


PixelManager::~PixelManager( )
{
	mVertexBuffer.Reset( );
	mDevice.Reset( );
	mContext.Reset( );
}

void PixelManager::Begin( )
{
	static D3D11_MAPPED_SUBRESOURCE MappedData;
	DX::ThrowIfFailed(
		mContext->Map( mVertexBuffer.Get( ), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &MappedData )
	);
	mStartAddress = MappedData.pData;
	mCurrent = 0;
}

void PixelManager::End( )
{
	mContext->Unmap( mVertexBuffer.Get( ), 0);
}

void PixelManager::Point( float x, float y, float r, float g, float b )
{
	x += 1;
	y += 1;
	y = mWindowHeight - y;
	x -= mHalfWindowWidth;
	y -= mHalfWindowHeight;
	x /= mHalfWindowWidth;
	y /= mHalfWindowHeight;
	SVertex * Vertices = ( SVertex* ) mStartAddress;
	Vertices[ mCurrent++ ] = SVertex( x, y, r, g, b );
}

void PixelManager::Render( )
{
	static UINT Stride = sizeof( SVertex );
	static UINT Offset = 0;
	mContext->IASetVertexBuffers( 0, 1, mVertexBuffer.GetAddressOf( ), &Stride, &Offset );
	mContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

	mShader->SetShaders( );
	mShader->Draw( mCurrent, 0 );
}