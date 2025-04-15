#pragma once
// ----------------------------------------------------------------------------
//
// enumeration for all possible different game states
//
// ----------------------------------------------------------------------------
enum
{
	// list of all game states 
	GS_BATTLE = 0,
	GS_DEATHMATCH,
	GS_MAINMENU,
	
	// special game state IDs
	GS_RESTART,
	GS_QUIT, 
	GS_NUM
};
