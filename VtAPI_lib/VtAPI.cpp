// VtAPI.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <string>
#include "VtAPI.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


// This is an example of an exported variable
VTAPI_API int nVtAPI=0;

// This is an example of an exported function.
VTAPI_API int fnVtAPI(void)
{
	return 42;
}

