/** \file VtSys.cpp

* Copyright (c) 2013 by
* All Rights Reserved
*
* REVISIONS: 
* $Log: VtSys.cpp,v $
* Revision 1.2  2013/05/27 12:04:19  david
* Ver 1
*
*/

//*********************************************************************
// INCLUDES
//*********************************************************************

#pragma message( "VtSysdefs.h" )
#include "VtSysdefs.h"
#pragma message( "VtErrors.h" )
#include "VtErrors.h"
#pragma message( "VtImage.h" )
#include "VtImage.h"

#include <windows.h>
#include <direct.h> // for getcwd
#include <time.h>
#include <sys\timeb.h>
#include <crtdbg.h>

#include <iostream>
#include <fstream>
#include <queue>

#include "VtAPI.h"
#include "VtDataset.h"


// use in wizard's device-specific generated code

#include "../include/wdu_lib.h"
#include "../include/status_strings.h"
#include "../include/utils.h"
#include "../include/usb_diag_lib.h"

// parser stuff

#include "VtPipeData.h"
#include "VtParser.h"
#include "VtpcLineParser.h"
#include "VthdsLineParser.h"

// driver stuff

#include "../ez_lib/ezusb_lib.h"
#include "../ez_lib/VtFirmware.h"
#include "../ez_lib/VtDrvrAPI.h"
#include "../ez_lib/VtUsbDriver.h"

// pano headers

#include "VtABDiff.h"
#include "VtPanoramicCalibration.h"
#include "VtpcImpAPI.h"

// hds headers

#include "VthdsCalib.h"
#include "VthdsImpAPI.h"
#include "VtSys.h"

using namespace Vt;

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


//*********************************************************************
// GLOBAL VARIABLES
//*********************************************************************
CVtSys* CVtSys::theSystem = NULL;// initialize pointer


//*********************************************************************
// GetAPI
//*********************************************************************
CVtAPI& Vt::GetAPI( CVtAPI::API_TYPE api, Vt::BIN_MODE bin_mode ) 
{ 
	return CVtSys::Instance( api, bin_mode );
}

//*********************************************************************
// GetAPI
// gets the current API (the most recently accessed)
//*********************************************************************

CVtAPI& Vt::GetAPI() 
{ 
	return CVtSys::Instance();
}

//*********************************************************************
// SystemEnd
//*********************************************************************
void Vt::CloseAPI()
{
	CVtSys::End();
}

