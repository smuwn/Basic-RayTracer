#include "Scene.h"



Scene::Scene( HINSTANCE Instance, bool bFullscreen ) :
	mInstance( Instance )
{
	try
	{
		InitWindow( bFullscreen );
		InitD3D( bFullscreen );
		mShader = std::make_shared<Shader>( mDevice, mImmediateContext );
		mPixels = std::make_unique<PixelManager>( mDevice, mImmediateContext, mShader,
			( mWidth + 1 ) * ( mHeight + 1 ), mWidth, mHeight );
	}
	CATCH;
}


Scene::~Scene( )
{
	mDevice.Reset( );
	mImmediateContext.Reset( );
	mBackbuffer.Reset( );
	mSwapChain.Reset( );

	mInput.reset( );

	UnregisterClass( ENGINE_NAME, mInstance );
	DestroyWindow( mWindow );
}


void Scene::InitWindow( bool bFullscreen )
{
	ZeroMemoryAndDeclare( WNDCLASSEX, wndClass );
	wndClass.cbSize = sizeof( WNDCLASSEX );
	wndClass.hbrBackground = ( HBRUSH ) ( GetStockObject( DKGRAY_BRUSH ) );
	wndClass.hInstance = mInstance;
	wndClass.lpfnWndProc = Scene::WndProc;
	wndClass.lpszClassName = ENGINE_NAME;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	if ( !RegisterClassEx( &wndClass ) )
		throw std::exception( "Couldn't register window class" );
	
	if ( !bFullscreen )
	{
		mWidth = 800;
		mHeight = 600;
	}
	else
	{
		mWidth = GetSystemMetrics( SM_CXSCREEN );
		mHeight = GetSystemMetrics( SM_CYSCREEN );
	}

	mWindow = CreateWindow( ENGINE_NAME, ENGINE_NAME,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, mWidth, mHeight,
		nullptr, nullptr, mInstance, nullptr );
	if ( !mWidth )
		throw std::exception( "Couldn't create window" );

	UpdateWindow( mWindow );
	ShowWindow( mWindow, SW_SHOWNORMAL );
	SetFocus( mWindow );

	mFullscreenViewport.Width = ( FLOAT ) mWidth;
	mFullscreenViewport.Height = ( FLOAT ) mHeight;
	mFullscreenViewport.TopLeftX = 0;
	mFullscreenViewport.TopLeftY = 0;
	mFullscreenViewport.MinDepth = 0.0f;
	mFullscreenViewport.MaxDepth = 1.0f;
}

void Scene::InitD3D( bool bFullscreen )
{
	IDXGIFactory * Factory;
	DX::ThrowIfFailed( CreateDXGIFactory( __uuidof( IDXGIFactory ),
		reinterpret_cast< void** >( &Factory ) ) );
	IDXGIAdapter * Adapter;
	DX::ThrowIfFailed( Factory->EnumAdapters( 0, &Adapter ) );
	IDXGIOutput * Output;
	DX::ThrowIfFailed( Adapter->EnumOutputs( 0, &Output ) );
	UINT NumModes;
	DX::ThrowIfFailed( Output->GetDisplayModeList( DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED, &NumModes, nullptr ) );
	DXGI_MODE_DESC * Modes = new DXGI_MODE_DESC[ NumModes ];
	DX::ThrowIfFailed( Output->GetDisplayModeList( DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED, &NumModes, Modes ) );
	DXGI_MODE_DESC FinalMode;
	bool bFound = false;
	for ( size_t i = 0; i < NumModes; ++i )
	{
		if ( Modes[ i ].Width == mWidth && Modes[ i ].Height == mHeight )
		{
			FinalMode = DXGI_MODE_DESC( Modes[ i ] );
			bFound = true;
			break;
		}
	}
	if ( !bFound )
	{
		FinalMode.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		FinalMode.Width = mWidth;
		FinalMode.Height = mHeight;
		FinalMode.RefreshRate.Denominator = 0;
		FinalMode.RefreshRate.Numerator = 60;
		FinalMode.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
		FinalMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	}
	delete[ ] Modes;
	DXGI_ADAPTER_DESC GPU;
	Adapter->GetDesc( &GPU );
	mGPUDescription = GPU.Description;
	ZeroMemoryAndDeclare( DXGI_SWAP_CHAIN_DESC, swapDesc );
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc = FinalMode;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.OutputWindow = mWindow;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = !bFullscreen;

	// MSAA
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	UINT flags = 0;
#if DEBUG || _DEBUG
	flags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driver =
#if NO_GPU
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_WARP
#else
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE
#endif
		;

	DX::ThrowIfFailed(
		D3D11CreateDeviceAndSwapChain( NULL, driver, NULL, flags,
			NULL, NULL, D3D11_SDK_VERSION, &swapDesc, &mSwapChain, &mDevice, NULL, &mImmediateContext )
	);

	ID3D11Texture2D * backBufferResource;
	DX::ThrowIfFailed( mSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ),
		reinterpret_cast< void** >( &backBufferResource ) ) );
	DX::ThrowIfFailed(
		mDevice->CreateRenderTargetView(
			backBufferResource, nullptr, &mBackbuffer
		)
	);

	backBufferResource->Release( );
	Factory->Release( );
	Adapter->Release( );
	Output->Release( );
	mInput = std::make_shared<CInput>( );
	mInput->Initialize( mInstance, mWindow );
}

LRESULT Scene::WndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{
	switch ( Message )
	{
	case WM_QUIT:
		DestroyWindow( hWnd );
		break;
	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;
	default:
		break;
	}
	return DefWindowProc( hWnd, Message, wParam, lParam );
}


void Scene::Run( )
{
	MSG Message;
	mTimer.Start( );
	while ( true )
	{
		if ( PeekMessage( &Message, nullptr, 0, 0, PM_REMOVE ) )
		{
			if ( Message.message == WM_QUIT )
				break;
			TranslateMessage( &Message );
			DispatchMessage( &Message );
		}
		else
		{
			if ( mInput->isKeyPressed( DIK_ESCAPE ) )
				break;
			if ( mTimer.GetTimeSinceLastStart( ) > 1.0f )
				mTimer.Start( );
			Update( );
			Render( );
		}
	}
}

void Scene::EnableBackbuffer( )
{
	mImmediateContext->RSSetViewports( 1, &mFullscreenViewport );
	mImmediateContext->OMSetRenderTargets( 1, mBackbuffer.GetAddressOf( ), nullptr );
}

void Scene::Update( )
{
	mTimer.Frame( );
	mInput->Frame( );
}

void Scene::Render( )
{
	using namespace DirectX;
	static FLOAT BackColor[ 4 ] = { 0,0,0,0 };
	EnableBackbuffer( );
	mImmediateContext->ClearRenderTargetView( mBackbuffer.Get( ), BackColor );

	static Sphere sp( XMFLOAT3( 0.0f, -3.0f, 5.0f ), 1.0f );

	XMVECTOR CamPos = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
	XMVECTOR ViewDir = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );
	float screenDistance = 1.0f;

	XMVECTOR ScreenCenter = CamPos + screenDistance * ViewDir;
	XMVECTOR P0 = ScreenCenter + XMVectorSet( -1, 1, 0, 0 ); // Top-left
	XMVECTOR P1 = ScreenCenter + XMVectorSet( 1, 1, 0, 0 ); // Top-right
	XMVECTOR P2 = ScreenCenter + XMVectorSet( -1, -1, 0, 0 ); // Bottom-left


	XMVECTOR PointOnScreen;
	XMVECTOR RayDirection;
	mPixels->Begin( );
	for ( int y = 0; y < mHeight; ++y )
		for ( int x = 0; x < mWidth; ++x )
		{
			float u = float( x ) / float( mWidth );
			float v = float( y ) / float( mHeight );
			PointOnScreen = P0 + ( P1 - P0 ) * u + ( P2 - P0 ) * v;
			RayDirection = XMVector3Normalize( PointOnScreen - CamPos );
			Ray r;
			XMStoreFloat3( &r.mDirection, RayDirection );
			XMStoreFloat3( &r.mStart, CamPos );
			r.mLength = 100000.0f;
			if ( sp.Intersect( r ) )
				mPixels->Point( x, y, 1.0f, 1.0f, 0.0f );
		}
	for ( int x = 0; x < mWidth; ++x )
	{
		mPixels->Point( x, mWidth - 1, 1.0f, 1.0f, 1.0f );
	}
	mPixels->End( );
	mPixels->Render( );
	mSwapChain->Present( 0, 0 );
}