
#include "AEEngine.h"
#include "GameStateList.h"

// ----------------------------------------------------------------------------
//
// Externs
//
// ----------------------------------------------------------------------------
extern unsigned int gGameStateInit;
extern unsigned int gGameStateCurr;
extern unsigned int gGameStatePrev;
extern unsigned int gGameStateNext;

extern void (*GameStateLoad)();
extern void (*GameStateInit)();
extern void (*GameStateUpdate)();
extern void (*GameStateDraw)();
extern void (*GameStateFree)();
extern void (*GameStateUnload)();


void GameStateMgrInit(unsigned int gameStateInit);

void GameStateMgrUpdate();
