#pragma once

#define UNICODE

#define ZeroMemoryAndDeclare(type, name) type name;\
ZeroMemory(&name,sizeof(type));
#define ZeroVariable(name) ZeroMemory(&name,sizeof(decltype(name)))
#define ALIGN16 __declspec(align(16))

#define ENGINE_NAMEW L"Apa Borsec"
#define ENGINE_NAMEA "Apa Borsec"

#ifdef UNICODE
#define ENGINE_NAME ENGINE_NAMEW
#else
#define ENGINE_NAME ENGINE_NAMEA
#endif


#if _DEBUG || DEBUG
#define CATCH catch(std::exception const& e) { \
char buffer[500]; sprintf_s(buffer, "Error: %s\n", e.what()); \
OutputDebugStringA(buffer); PostQuitMessage(0); }\
catch( ... ) { \
OutputDebugStringA( "Unexpected error occured\n" ); PostQuitMessage(0);}
#else
#define CATCH catch(std::exception const& e) { \
char buffer[500]; sprintf_s(buffer, "Error: %s", e.what());\
MessageBoxA(NULL,buffer,"Error",MB_ICONERROR| MB_OK); PostQuitMessage(0);}\
catch (...) {\
MessageBoxA(NULL,"Unexpected error occured", "Error", MB_ICONERROR| MB_OK); PostQuitMessage(0);\
}
#endif

#include <windows.h>
#include <d3d11.h>
#include <D3DX11.h>
#include <d3d10.h>
#include <D3DX10.h>
#include <dxgi.h>
#include <D3Dcompiler.h>
#include <wrl.h>
#include <comdecl.h>
#include <comdef.h>
#include <DirectXMath.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>

#include <random>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <algorithm>
#include <memory>
#include <fstream>
#include <sstream>
#include <map>
#include <unordered_map>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3d10.lib")
#pragma comment (lib, "d3dx10.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "D3Dcompiler.lib")



namespace DX
{
	static constexpr const float EPSILON = 0.0001f;
	inline void ThrowIfFailed( HRESULT hr )
	{
		if ( FAILED( hr ) )
		{
			_com_error err( hr );
			const wchar_t* errorMessage = err.ErrorMessage( );
			wchar_t buffer[ 500 ];
#if DEBUG || _DEBUG
			int line = __LINE__;
			const char* file = __FILE__;
			swprintf_s( buffer, L"DirectX Error occured at line %d in file %hs;\nMessage: %ws\n", line, file, errorMessage );
			OutputDebugString( buffer );
#else
			swprintf_s( buffer, L"DirectX Error occured; Message: %ws", errorMessage );
			MessageBox( NULL, buffer, L"Error", MB_ICONERROR | MB_OK );
#endif
			throw std::exception( "DirectX Error" );
		}
	};
	inline void OutputVDebugString( const wchar_t * format, ... )
	{
		static wchar_t Sequence[ 1024 ];
		va_list args;
		va_start( args, format );
		_vsnwprintf_s( Sequence, sizeof( Sequence ), format, args );
		va_end( args );
		OutputDebugStringW( Sequence );
	}
	inline void SafeRelease( IUnknown *& object )
	{
		OutputVDebugString( L"Please don't use SafeRelease(); Use smart pointers instead" );
		if ( object )
		{
			object->Release( );
			object = nullptr;
		}
	}
	template <class type>
	type clamp( type& x, type lower, type upper )
	{
		return max( lower, min( x, upper ) );
	}
}