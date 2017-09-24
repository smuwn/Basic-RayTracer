#include "Scene.h"

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine, int iShow )
{
	Scene* SC = new Scene( hInstance );
	SC->Run( );
	delete SC;
}