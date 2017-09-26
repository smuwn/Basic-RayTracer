#include "Scene.h"



Scene::Scene( HINSTANCE Instance, bool bFullscreen ) :
	mInstance( Instance )
{
	try
	{
		InitWindow( bFullscreen );
		InitD3D( bFullscreen );
		mCamPos = DirectX::XMFLOAT3( 0.0f, 0.0f, -2.0f );
		mShader = std::make_shared<Shader>( mDevice, mImmediateContext );
		mPixels = std::make_unique<PixelManager>( mDevice, mImmediateContext, mShader,
			( mWidth + 1 ) * ( mHeight + 1 ), mWidth, mHeight );
		Sphere *sp = new Sphere( DirectX::XMFLOAT3( 0.0f, 5.0f, 5.0f ), 5.0f, DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ) );
		sp->mReflectivity = 0.5f;
		mShapes.push_back( reinterpret_cast< IShape* >( sp ) );
		sp = new Sphere( DirectX::XMFLOAT3( 0.0f, -500.0f, 0.0f ), 500.0f, DirectX::XMFLOAT3( 0.0f, 1.0f, 0.0f ) );
		mShapes.push_back( reinterpret_cast< IShape* >( sp ) );
		sp = new Sphere( DirectX::XMFLOAT3( 0.0f, 3.0f, -5.0f ), 3.0f, DirectX::XMFLOAT3( 1.0f, 0.0f, 0.0f ) );
		sp->mReflectivity = 0.5f;
		mShapes.push_back( reinterpret_cast< IShape* > ( sp ) );
		mLights.emplace_back( DirectX::XMFLOAT3( 0.0f, 10.0f, 0.0f ), DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ) );
		mAmbient = DirectX::XMFLOAT3( 0.2f, 0.2f, 0.2f );
	}
	CATCH;
}


Scene::~Scene( )
{
	for ( size_t i = 0; i < mShapes.size( ); ++i )
	{
		if ( mShapes[ i ] != nullptr )
		{
			delete mShapes[ i ];
			mShapes[ i ] = nullptr;
		}
	}

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
	

	mRotationY += mInput->GetHorizontalMouseMove( ) * 0.001f;
	mRotationX += mInput->GetVerticalMouseMove( ) * 0.001f;

	DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationX( mRotationX ) * DirectX::XMMatrixRotationY( mRotationY );
	DirectX::XMVECTOR Forward, Right;
	Forward = DirectX::XMVector3TransformCoord(
		DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ),
		Rotation );
	Right = DirectX::XMVector3TransformCoord(
		DirectX::XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f ),
		Rotation );
	

	if ( mInput->isKeyPressed( DIK_W ) )
	{
		DirectX::XMVECTOR CamPos;
		CamPos = DirectX::XMLoadFloat3( &mCamPos );
		CamPos = DirectX::XMVectorAdd( CamPos, Forward );
		DirectX::XMStoreFloat3( &mCamPos, CamPos );
	}
	if ( mInput->isKeyPressed( DIK_S ) )
	{
		DirectX::XMVECTOR CamPos;
		CamPos = DirectX::XMLoadFloat3( &mCamPos );
		CamPos = DirectX::XMVectorSubtract( CamPos, Forward );
		DirectX::XMStoreFloat3( &mCamPos, CamPos );
	}
	if ( mInput->isKeyPressed( DIK_A ) )
	{
		DirectX::XMVECTOR CamPos;
		CamPos = DirectX::XMLoadFloat3( &mCamPos );
		CamPos = DirectX::XMVectorSubtract( CamPos, Right );
		DirectX::XMStoreFloat3( &mCamPos, CamPos );
	}
	if ( mInput->isKeyPressed( DIK_D ) )
	{
		DirectX::XMVECTOR CamPos;
		CamPos = DirectX::XMLoadFloat3( &mCamPos );
		CamPos = DirectX::XMVectorAdd( CamPos, Right );
		DirectX::XMStoreFloat3( &mCamPos, CamPos );
	}
}

void Scene::Render( )
{
	using namespace DirectX;
	static FLOAT BackColor[ 4 ] = { 0,0,0,0 };
	EnableBackbuffer( );
	mImmediateContext->ClearRenderTargetView( mBackbuffer.Get( ), BackColor );

	XMMATRIX Rotation = XMMatrixRotationX( mRotationX ) * XMMatrixRotationY( mRotationY );

	XMVECTOR CamPos = XMLoadFloat3( &mCamPos );
	XMVECTOR ViewDir = XMVector3TransformCoord( XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ), Rotation );
	float screenDistance = 1.0f;

	XMVECTOR ScreenCenter = CamPos + screenDistance * ViewDir;
	XMVECTOR P0 = ScreenCenter + XMVector3TransformCoord( XMVectorSet( -1, 1, 0, 0 ), Rotation ); // Top-left
	XMVECTOR P1 = ScreenCenter + XMVector3TransformCoord( XMVectorSet( 1, 1, 0, 0 ), Rotation ); // Top-right
	XMVECTOR P2 = ScreenCenter + XMVector3TransformCoord( XMVectorSet( -1, -1, 0, 0 ), Rotation ); // Bottom-left


	XMVECTOR PointOnScreen;
	XMVECTOR RayDirection;
	mPixels->Begin( );
	for ( float y = 0; y < mHeight; ++y )
		for ( float x = 0; x < mWidth; ++x )
		{
			float u = x / mWidth;
			float v = y / mHeight;
			PointOnScreen = P0 + ( P1 - P0 ) * u + ( P2 - P0 ) * v;
			RayDirection = XMVector3Normalize( PointOnScreen - CamPos );
			Ray r;
			XMStoreFloat3( &r.mDirection, RayDirection );
			XMStoreFloat3( &r.mStart, CamPos );
			r.mLength = MAX_DISTANCE;
			DirectX::XMFLOAT3 Color;
			if ( Trace( r, Color ) )
			{
				mPixels->Point( x, y, Color.x, Color.y, Color.z );
			}
		}
	mPixels->End( );
	mPixels->Render( );
	mSwapChain->Present( 0, 0 );
}

DirectX::XMFLOAT3 Scene::CalculateColor( Ray const& r, int hitIndex )
{
	using namespace DirectX;

	XMVECTOR RayStart = XMLoadFloat3( &r.mStart );
	XMVECTOR RayDirection = XMLoadFloat3( &r.mDirection );
	XMVECTOR HitPoint = RayStart + RayDirection * r.mLength;

	XMVECTOR sphereCenter = XMLoadFloat3( &mShapes[ hitIndex ]->GetCenter( ) );
	XMVECTOR Normal = HitPoint - sphereCenter;
	Normal = XMVector3Normalize( Normal );

	XMVECTOR InverseLightDir, LightPos, LightColor, ToLight;
	XMVECTOR FinalColor = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
	XMVECTOR ObjectColor = XMLoadFloat3( &mShapes[ hitIndex ]->GetColor( ) );
	for ( size_t i = 0; i < mLights.size( ); ++i )
	{
		LightPos = DirectX::XMLoadFloat3( &mLights[ i ].mPosition );
		bool bIsInShadow = false;
		ToLight = LightPos - HitPoint;
		for ( size_t j = 0; j < mShapes.size( ); ++j )
		{
			XMFLOAT3 NewRayPos, NewRayDir;
			XMStoreFloat3( &NewRayPos, HitPoint );
			XMStoreFloat3( &NewRayDir, XMVector3Normalize( ToLight ) );
			Ray r( NewRayPos, NewRayDir, MAX_DISTANCE );
			if ( mShapes[ j ]->Intersect( r ) )
			{
				bIsInShadow = true;
				break;
			}
		}

		if ( !bIsInShadow )
		{
			LightColor = DirectX::XMLoadFloat3( &mLights[ i ].mDiffuse );
			InverseLightDir = LightPos - HitPoint;
			InverseLightDir = XMVector3Normalize( InverseLightDir );
			float D = XMVectorGetX( XMVector3Dot( InverseLightDir, Normal ) );
			if ( D > 0.0f )
			{
				FinalColor += LightColor * D;
			}
		}
	}
	XMVECTOR Ambient = XMLoadFloat3( &mAmbient );
	FinalColor += Ambient;

	FinalColor = XMVectorSaturate( FinalColor );
	FinalColor *= ObjectColor;

	XMFLOAT3 Color;
	XMStoreFloat3( &Color, FinalColor );

	return Color;
}

bool Scene::Trace( Ray& r, DirectX::XMFLOAT3& Color, const int depth )
{
	int hitIndex = -1;
	for ( size_t i = 0; i < mShapes.size( ); ++i )
	{
		if ( mShapes[ i ]->Intersect( r ) )
			hitIndex = i;
	}
	if ( hitIndex == -1 )
		return false;

	if ( ( mShapes[ hitIndex ]->GetReflectivity( ) > 0 || mShapes[ hitIndex ]->GetTransparency( ) > 0 )
		&& depth < MAX_DEPTH )
	{
		using namespace DirectX;

		// c - current; n - new
		XMVECTOR cRayDirection = XMLoadFloat3( &r.mDirection );
		XMVECTOR cRayPosition = XMLoadFloat3( &r.mStart );

		XMVECTOR RayStart = XMLoadFloat3( &r.mStart );
		XMVECTOR RayDirection = XMLoadFloat3( &r.mDirection );
		XMVECTOR HitPoint = RayStart + RayDirection * r.mLength;

		XMVECTOR sphereCenter = XMLoadFloat3( &mShapes[ hitIndex ]->GetCenter( ) );
		XMVECTOR Normal = HitPoint - sphereCenter;
		Normal = XMVector3Normalize( Normal );
		bool bInside = false;
		if ( XMVectorGetX( XMVector3Dot( RayDirection, Normal ) ) > 0 )
		{
			Normal = -Normal;
			bInside = true;
		}
		
		XMVECTOR ReflectedDirection = XMVector3Reflect( RayDirection, Normal );

		HitPoint = HitPoint + ReflectedDirection * 0.1f;
		Ray nR;
		XMStoreFloat3( &nR.mDirection, ReflectedDirection );
		XMStoreFloat3( &nR.mStart, HitPoint );
		nR.mLength = MAX_DISTANCE;
		XMFLOAT3 ReflectionColor;
		float Reflectivity = mShapes[ hitIndex ]->GetReflectivity( );
		if ( !Trace( nR, ReflectionColor, depth + 1 ) )
		{
			Color = CalculateColor( r, hitIndex );
			Color.x *= ( 1.0f - Reflectivity );
			Color.y *= ( 1.0f - Reflectivity );
			Color.z *= ( 1.0f - Reflectivity );
			return true;
		}

		XMVECTOR RColor = XMLoadFloat3( &ReflectionColor );
		XMVECTOR DColor = XMLoadFloat3( &CalculateColor( r, hitIndex ) );

		DColor = ( 1.0f - Reflectivity ) * DColor + ( Reflectivity ) * RColor;
		XMStoreFloat3( &Color, DColor );
	}
	else
	{
		Color = CalculateColor( r, hitIndex );
	}
	return true;
}