
#include "Main.h"

// ----------------------------------------------------------------------------
//
// "Load" function - Main Menu
//
// ----------------------------------------------------------------------------
void GameStateMainMenuLoad(void)
{
}

// ----------------------------------------------------------------------------
//
// "Init" function - Main Menu
//
// ----------------------------------------------------------------------------
void GameStateMainMenuInit(void)
{
	g_fixedDT = 0.01667;
	g_dt = g_fixedDT;

	AEGfxSetCamPosition(0.0f, 0.0f);
}

// ----------------------------------------------------------------------------
//
// "Update" function - Main Menu
//
// ----------------------------------------------------------------------------
void GameStateMainMenuUpdate(void)
{
	if (AEInputCheckTriggered(AEVK_1))
		gGameStateNext = GS_BATTLE;
	else if (AEInputCheckTriggered(AEVK_2))
		gGameStateNext = GS_DEATHMATCH;
	else if (AEInputCheckTriggered(AEVK_Q))
		gGameStateNext = GS_QUIT;
}

// ----------------------------------------------------------------------------
//
// "Draw" function - Main Menu
//
// ----------------------------------------------------------------------------
void GameStateMainMenuDraw(void)
{
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxTextureSet(NULL, 0, 0);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	AEGfxPrint(fontId, "Press '1' for Level 1", -0.25f, 0.25f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxPrint(fontId, "Press '2' for Level 2", -0.25f, 0.15f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxPrint(fontId, "Press 'Q' to Quit", -0.25f, 0.05f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

	// display help text and highscore
	AEGfxPrint(fontId, "Platformer", -0.25f, 0.45f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f);
}

// ----------------------------------------------------------------------------
//
// "Free" function - Main Menu
//
// ----------------------------------------------------------------------------
void GameStateMainMenuFree(void)
{
}

// ----------------------------------------------------------------------------
//
// "Unload" function - Main Menu
//
// ----------------------------------------------------------------------------
void GameStateMainMenuUnload(void)
{
}