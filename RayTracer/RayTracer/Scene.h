#pragma once

#include "commonincludes.h"
#include "Input.h"
#include "HRTimer.h"
#include "Shader.h"
#include "PixelManager.h"
#include "Shapes.h"

ALIGN16 class Scene
{
private:
	HWND mWindow;
	HINSTANCE mInstance;
	D3D11_VIEWPORT mFullscreenViewport;

	CHRTimer mTimer;
	WCHAR* mGPUDescription;

	int mWidth;
	int mHeight;

	std::unique_ptr<PixelManager> mPixels;

	std::shared_ptr<CInput> mInput;

	std::shared_ptr<Shader> mShader;

	DirectX::XMFLOAT3 mCamPos;

	float mRotationX;
	float mRotationY;

private: // D3D objects
	Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> mImmediateContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mBackbuffer;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
public:
	Scene( HINSTANCE Instance, bool bFullscreen = false );
	~Scene( );
public:
	void Run( );
private:
	void InitWindow( bool bFullscreen );
	void InitD3D( bool bFullscreen );
private:
	void EnableBackbuffer( );
	void Update( );
	void Render( );
public:
	static LRESULT CALLBACK WndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam );
public:
	inline void* operator new( size_t size )
	{
		return _aligned_malloc( size, 16 );
	}
	inline void operator delete( void* object )
	{
		return _aligned_free( object );
	}
};

