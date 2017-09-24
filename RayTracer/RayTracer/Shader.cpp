#include "Shader.h"



Shader::Shader( Microsoft::WRL::ComPtr<ID3D11Device> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context ) :
	mDevice( Device ),
	mContext( Context )
{
	try
	{
		ID3D11VertexShader ** VS = &mVertexShader;
		ID3D11PixelShader ** PS = &mPixelShader;
		ShaderHelper::CreateShaderFromFile( L"Shaders/VertexShader.cso", "vs_4_0",
			mDevice.Get( ), &mBlobs[ 0 ], reinterpret_cast< ID3D11DeviceChild** >( VS ) );
		ShaderHelper::CreateShaderFromFile( L"Shaders/PixelShader.cso", "ps_4_0",
			mDevice.Get( ), &mBlobs[ 1 ], reinterpret_cast< ID3D11DeviceChild** >( PS ) );
		D3D11_INPUT_ELEMENT_DESC layout[ 2 ];
		layout[ 0 ].AlignedByteOffset = 0;
		layout[ 0 ].Format = DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
		layout[ 0 ].InputSlot = 0;
		layout[ 0 ].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		layout[ 0 ].InstanceDataStepRate = 0;
		layout[ 0 ].SemanticIndex = 0;
		layout[ 0 ].SemanticName = "POSITION";
		layout[ 1 ].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		layout[ 1 ].Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
		layout[ 1 ].InputSlot = 0;
		layout[ 1 ].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		layout[ 1 ].InstanceDataStepRate = 0;
		layout[ 1 ].SemanticIndex = 0;
		layout[ 1 ].SemanticName = "COLOR";
		int Size = ARRAYSIZE( layout );
		DX::ThrowIfFailed(
			mDevice->CreateInputLayout(
				layout, Size, mBlobs[ 0 ]->GetBufferPointer( ), mBlobs[ 0 ]->GetBufferSize( ), &mLayout
			)
		);
	}
	CATCH;
}


Shader::~Shader( )
{
	for ( size_t i = 0; i < mBlobs.size( ); ++i )
		mBlobs[ i ].Reset( );
	mDevice.Reset( );
	mContext.Reset( );
	mLayout.Reset( );
	mVertexShader.Reset( );
	mPixelShader.Reset( );
}


void Shader::SetShaders( )
{
	mContext->VSSetShader( mVertexShader.Get( ), 0, 0 );
	mContext->PSSetShader( mPixelShader.Get( ), 0, 0 );
	mContext->IASetInputLayout( mLayout.Get( ) );
}

void Shader::Draw( UINT VertexCount, UINT VertexOffset )
{
	mContext->Draw( VertexCount, VertexOffset );
}