/* Start Header ************************************************************************/
/*!
\file		Main.cpp
\author		DigiPen, DigiPen, DigiPen
\par		digipen@digipen.edu
\date		January, 22, 2025
\brief		Entry point file with WinMain implementing the 2 nested loops.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "main.h"
#include <memory>

// ----------------------------------------------------------------------------
//
// Globals
//
// ----------------------------------------------------------------------------
s8		fontId;			// Font for drawing text


// ----------------------------------------------------------------------------
//
// Starting point of the application
//
// ----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE instanceH, HINSTANCE prevInstanceH, LPSTR command_line, int show)
{
	UNREFERENCED_PARAMETER(prevInstanceH);
	UNREFERENCED_PARAMETER(command_line);

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Initialize the system
	AESysInit(instanceH, show, 800, 600, 1, 60, false, NULL);

	// Changing the window title
	AESysSetWindowTitle("Platformer Demo!");

	// Set background color
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

	// Font is created here, and is used for all your levels
	fontId = AEGfxCreateFont("..\\Resources\\Fonts\\Arial_Italic.ttf", 24);


	GameStateMgrInit(GS_MAINMENU);

	while (gGameStateCurr != GS_QUIT)
	{
		// Reset the system modules
		AESysReset();

		// If not restarting, load the gamestate
		if (gGameStateCurr != GS_RESTART)
		{
			GameStateMgrUpdate();
			GameStateLoad();
		}
		else
			gGameStateNext = gGameStateCurr = gGameStatePrev;

		// Initialize the gamestate
		GameStateInit();

		while (gGameStateCurr == gGameStateNext)
		{
			AESysFrameStart();

			GameStateUpdate();

			GameStateDraw();

			AESysFrameEnd();

			// check if forcing the application to quit
			if (AESysDoesWindowExist() == false)
				gGameStateNext = GS_QUIT;

			g_dt = AEFrameRateControllerGetFrameTime();

			//hack
			g_dt = g_fixedDT;

			g_appTime += g_dt;
		}

		GameStateFree();

		if (gGameStateNext != GS_RESTART)
			GameStateUnload();

		gGameStatePrev = gGameStateCurr;
		gGameStateCurr = gGameStateNext;
	}

	//free you font here
	AEGfxDestroyFont(fontId);

	// free the system
	AESysExit();
}