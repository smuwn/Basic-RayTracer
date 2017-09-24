#pragma once

#include "commonincludes.h"
#include "Input.h"
#include "HRTimer.h"

class Scene
{
private:
	static constexpr const float NearZ = 0.1f;
	static constexpr const float FarZ = 500.0f;
	static constexpr const float FOV = ( float ) D3DX_PI / 4.0f;
private:
	HWND mWindow;
	HINSTANCE mInstance;
	D3D11_VIEWPORT mFullscreenViewport;

	CHRTimer mTimer;
	WCHAR* mGPUDescription;

	int mWidth;
	int mHeight;


	std::shared_ptr<CInput> mInput;

	DirectX::XMMATRIX mOrthoMatrix;

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
};

