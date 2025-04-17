/* Start Header ************************************************************************/
/*!
\file		GameState_Platform.cpp
\author		DigiPen, DigiPen, DigiPen
\par		digipen@digipen.edu
\date		January, 22, 2025
\brief		This file implements the main six functions of the Platform game state.
			The functions are used for both gameplay levels of the platformer game.
			It also contains some helper functions.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameState_Platform.h"

void Load_AllMeshes(void);


// ----------------------------------------------------------------------------
//
// Variables and containers to use
//
// ----------------------------------------------------------------------------
int				Player1_Lives = 0;
int				Player2_Lives = 0;

// list of original objects
GameObj *		sGameObjList = nullptr;
unsigned int	sGameObjNum = 0;

// list of object instances
GameObjInst *	sGameObjInstList = nullptr;
unsigned int	sGameObjInstNum = 0;

//Binary map data
int **			MapData = nullptr;
int **			BinaryCollisionArray = nullptr;
int				BINARY_MAP_WIDTH = 160;
int				BINARY_MAP_HEIGHT = 90;
GameObjInst*	pBlackInstance = nullptr;
GameObjInst*	pWhiteInstance = nullptr;
AEMtx33			MapTransform;

//We need a pointer to the hero's instance for faster access
GameObjInst*	pPlayer1 = nullptr;
GameObjInst*	pPlayer2 = nullptr;

// We need a pointer to the arrow's instance for faster access
GameObjInst* pArrow1 = nullptr;
GameObjInst* pArrow2 = nullptr;

// We need a pointer to the weapon's instance for faster access
//Weapon* pWeapon1 = nullptr;
//Weapon* pWeapon2 = nullptr;


// ----------------------------------------------------------------------------
//
// "Load" function of this state
// This function loads all necessary assets for Level 1 or Level 2
// It should be called once before the start of the level
//
// ----------------------------------------------------------------------------
void GameStatePlatformLoad(void)
{
	sGameObjList = (GameObj *)calloc(GAME_OBJ_NUM_MAX, sizeof(GameObj));
	sGameObjInstList = (GameObjInst *)calloc(GAME_OBJ_INST_NUM_MAX, sizeof(GameObjInst));
	sGameObjNum = 0;

	// Helper function to load all meshes
	Load_AllMeshes();

	// Helper function to load the current level's map data
	Import_MapData();

	// Helper function to compute MapTransform matrix
	Compute_MapTransformMatrix();
}

// ----------------------------------------------------------------------------
//
// "Initialize" function of this game state. Accounts for Level 1 and Level 2.
// This function is used to prepare the state's data in order to be used for the
// first time. If a state is restarted or reset, this cycle function is called 
// once.
//
// ----------------------------------------------------------------------------
void GameStatePlatformInit(void)
{
	g_fixedDT = 0.01667;
	g_dt = g_fixedDT;

	// Helper function to create all the initial game objects instances
	Starting_GameObjectsInstances();
	Init_GameObjects();
}

// ----------------------------------------------------------------------------
//
// "Update" function of this game state. Accounts for Level 1 and Level 2.
//
// ----------------------------------------------------------------------------
void GameStatePlatformUpdate(void)
{
	int i = 0;
	GameObjInst *pInst = nullptr;

	Update_Input_Physics();	

	Apply_GravityPhysics();
	

	/***********
	Save previous positions
	  -- For all instances
	***********/
	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		pInst = sGameObjInstList + i;

		// skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		pInst->posPrev.x = pInst->posCurr.x;
		pInst->posPrev.y = pInst->posCurr.y;
	}
	

	Update_BoundingBoxes();

	Update_Positions();

	Check_GridBinaryCollision();
	
	Update_AABBCollisions();

	Update_ObjectsTransformations();	

	if (gGameStateCurr == GS_DEATHMATCH)
	{
		Update_CameraPosition_Level2();
	}
}

// ----------------------------------------------------------------------------
//
// "Draw" function of this game state. Accounts for Level 1 and Level 2.
//
// ----------------------------------------------------------------------------
void GameStatePlatformDraw(void)
{
	//Drawing setup
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxTextureSet(NULL, 0, 0);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);


	//Drawing the tile map
	Draw_TileMap_BackgroundGrid();
	
	//Drawing the dynamic object instances
	Draw_DynamicObjectsInstances();


	//Drawing setup
	AEGfxSetBlendMode(AE_GFX_BM_NONE);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);


	//Display total coins and hero lives texts
	Display_GameStats();
}

// ----------------------------------------------------------------------------
//
// "Free" function of this game state. Accounts for Level 1 and Level 2.
//
// ----------------------------------------------------------------------------
void GameStatePlatformFree(void)
{
	Destroy_ObjectsInstances();
}

// ----------------------------------------------------------------------------
//
// "Unload" function of this game state. Accounts for Level 1 and Level 2.
//
// ----------------------------------------------------------------------------
void GameStatePlatformUnload(void)
{
	Free_AllAllocatedMemory();
}

// =========================================================
// 
// creates all needed meshes
// 
// =========================================================
void Load_AllMeshes(void)
{
	GameObj* pObj;

	//Creating the black object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_EMPTY;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF87CEEB, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF87CEEB, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF87CEEB, 0.0f, 0.0f);

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFF87CEEB, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF87CEEB, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFF87CEEB, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");


	//Creating the white object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_COLLISION;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF8B4513, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF8B4513, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF8B4513, 0.0f, 0.0f);

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFF8B4513, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF8B4513, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFF8B4513, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");


	// Creating player 1 object (BLUE, tall)
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_PLAYER1;

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF0000FF, 0.0f, 0.0f,  // Blue
		0.5f, -0.5f, 0xFF0000FF, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f);

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF0000FF, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");

	// Creating player 2 object (RED, tall)
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_PLAYER2;

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,  // Red
		0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f);

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");

	// Creating arrow object (YELLOW, pointing right)
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_ARROW;

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.25f, 0xFFFFFFFF, 0.0f, 0.0f,  // Yellow
		-0.5f, 0.25f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, 0.0f, 0xFFFFFFFF, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create arrow object!!");

}

// ----------------------------------------------------------------------------
//
// A helper function to create a game object instance
//
// ----------------------------------------------------------------------------
GameObjInst* gameObjInstCreate( unsigned long type, 
								AEVec2* scale,
								AEVec2* pPos, 
								AEVec2* pVel, 
								float dir)
{
	AEVec2 zero;
	AEVec2Zero(&zero);

	AE_ASSERT_PARM(type < sGameObjNum);
	
	// loop through the object instance list to find a non-used object instance
	for (unsigned int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;

		// check if current instance is not used
		if (pInst->flag == 0)
		{
			// it is not used => use it to create the new instance
			pInst->pObject			 = sGameObjList + type;
			pInst->flag				 = FLAG_ACTIVE | FLAG_VISIBLE;
			pInst->scale			 = *scale;
			pInst->posCurr			 = pPos ? *pPos : zero;
			pInst->velCurr			 = pVel ? *pVel : zero;
			pInst->dirCurr			 = dir;
			pInst->pOwner			 = 0;
			pInst->gridCollisionFlag = 0;
			pInst->state			 = STATE_GOING_LEFT;
			pInst->innerState		 = INNER_STATE_ON_ENTER;
			pInst->idle_counter		 = 0;
			pInst->lifetime			 = 0.0f;
			pInst->jumpCount		 = 0;
			pInst->maxJumps			 = 1;
			pInst->isJumping		 = false;
			
			// return the newly created instance
			return pInst;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
//
// A helper function to destroy a game object instance
//
// ----------------------------------------------------------------------------
void gameObjInstDestroy(GameObjInst* pInst)
{
	// if instance is destroyed before, just return
	if (pInst->flag == 0)
		return;

	// zero out the flag
	pInst->flag = 0;
}
