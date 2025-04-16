
#include "GameState_Platform_Extension.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>


// =========================================================
// 
// import map data for the current level (level 1 or level 2)
// 
// =========================================================
void Import_MapData(void)
{
	//Setting intital binary map values
	MapData = nullptr;
	BinaryCollisionArray = nullptr;

	// LEVEL 1
	if (gGameStateCurr == GS_BATTLE)
	{
		GenerateRandomMap();
	}

	// LEVEL 2
	else if (gGameStateCurr == GS_DEATHMATCH)
	{
		//Importing Data - Must Quit the application if "ImportMapDataFromFile" fails
		if (!ImportMapDataFromFile("..\\Resources\\Levels\\Exported2.txt"))
			gGameStateNext = GS_QUIT;
	}
}

// =========================================================
// 
// compute MapTransform matrix - Only once at load time
// 
// =========================================================
void Compute_MapTransformMatrix(void)
{
	// Tile size
	float TILE_SIZE = static_cast<f32>(AEGfxGetWindowWidth() / BINARY_MAP_WIDTH);

	// Map size in pixels based on tile size
	float mapPixelWidth = BINARY_MAP_WIDTH * TILE_SIZE;
	float mapPixelHeight = BINARY_MAP_HEIGHT * TILE_SIZE;

	// Center map in the middle of the screen
	float screenCenterX = AEGfxGetWindowWidth() / 2.0f;
	float screenCenterY = AEGfxGetWindowHeight() / 2.0f;

	float mapOffsetX = screenCenterX - mapPixelWidth;
	float mapOffsetY = screenCenterY - mapPixelHeight;

	// Build transformation matrices
	AEMtx33 scale, trans;
	AEMtx33Scale(&scale, static_cast<f32>(TILE_SIZE), static_cast<f32>(TILE_SIZE));  // Scale to 8px per tile
	AEMtx33Trans(&trans, mapOffsetX, mapOffsetY);  // Translate to center the map

	// Combine: translate first, then scale
	AEMtx33Concat(&MapTransform, &trans, &scale);

}

// =========================================================
// 
// creates all initial game objects instances
// 
// =========================================================
void Starting_GameObjectsInstances(void)
{
	int i = 0, j = 0;

	pBlackInstance = nullptr;
	pWhiteInstance = nullptr;
	//Create an object instance representing the black cell.
	//This object instance should not be visible. When rendering the grid cells, each time we have
	//a non collision cell, we position this instance in the correct location and then we render it
	AEVec2 scl = { 1.f, 1.f };
	pBlackInstance = gameObjInstCreate(TYPE_OBJECT_EMPTY, &scl, 0, 0, 0.0f);
	pBlackInstance->flag ^= FLAG_VISIBLE;
	pBlackInstance->flag |= FLAG_NON_COLLIDABLE;

	//Create an object instance representing the white cell.
	//This object instance should not be visible. When rendering the grid cells, each time we have
	//a collision cell, we position this instance in the correct location and then we render it
	pWhiteInstance = gameObjInstCreate(TYPE_OBJECT_COLLISION, &scl, 0, 0, 0.0f);
	pWhiteInstance->flag ^= FLAG_VISIBLE;
	pWhiteInstance->flag |= FLAG_NON_COLLIDABLE;

	pPlayer1 = nullptr;
	pPlayer2 = nullptr;

	//Setting the inital number of hero lives
	Player1_Lives = HERO_LIVES;
	Player2_Lives = HERO_LIVES;

	//GameObjInst* pInst = nullptr;
	AEVec2 Pos = { 0.f,0.f };

	scl = { 2.f, 4.f };
	// Loop through each cell in the binary map grid.
	for (i = 0; i < BINARY_MAP_WIDTH; ++i)
		for (j = 0; j < BINARY_MAP_HEIGHT; ++j)
		{
			// Set the position of the current cell in the world space.
			AEVec2Set(&Pos, (f32)i + 0.5f, (f32)j + 1.01f);

			// Check the type of object in the current map cell.
			switch (MapData[j][i]) 
			{
			case (TYPE_OBJECT_COLLISION):
			case (TYPE_OBJECT_EMPTY):
				// Do nothing for empty or collision cells.
				break;
			case (TYPE_OBJECT_PLAYER1):
				// Create player 1 object instance.
				pPlayer1 = gameObjInstCreate(TYPE_OBJECT_PLAYER1, &scl, &Pos, 0, 0.0f);
				break;
			case (TYPE_OBJECT_PLAYER2):
				// Create player 2 object instance.
				pPlayer2 = gameObjInstCreate(TYPE_OBJECT_PLAYER2, &scl, &Pos, 0, 0.0f);
				break;
			default:
				// Handle any unknown object types (if necessary).
				break;
			}
		}
}

// Boolean to track if hero is facing left or right
bool flip = false;

// =========================================================
// 
// Handle input and set the physics variables accordingly
// 
// =========================================================
void Update_Input_Physics(void)
{
	// PLAYER 1 CONTROLS
	if (AEInputCheckCurr(AEVK_D))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{		
		pPlayer1->velCurr.x = MOVE_VELOCITY_HERO; // Move hero to the right
		flip = false; // Ensure the hero faces right.
	}
	else if (AEInputCheckCurr(AEVK_A))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{
		pPlayer1->velCurr.x = -MOVE_VELOCITY_HERO; // Move hero to the left.
		flip = true; // Ensure the hero faces left.
	}
	else {
		pPlayer1->velCurr.x = 0.f; // Stop horizontal movement if no keys are pressed.
	}
	
	if (AEInputCheckTriggered(AEVK_W))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{
		if (pPlayer1->gridCollisionFlag & COLLISION_BOTTOM) {
			pPlayer1->velCurr.y = JUMP_VELOCITY;
		}
		else if (pPlayer1->jumpCount < pPlayer1->maxJumps) // Check if player can double jump
		{
			pPlayer1->velCurr.y = JUMP_VELOCITY; // Apply jump velocity.
			pPlayer1->jumpCount++; // Increment jump count
		}
	}
	
	// PLAYER 2 CONTROLS
	if (AEInputCheckCurr(AEVK_RIGHT))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{
		pPlayer2->velCurr.x = MOVE_VELOCITY_HERO; // Move hero to the right
		flip = false; // Ensure the hero faces right.
	}
	else if (AEInputCheckCurr(AEVK_LEFT))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{
		pPlayer2->velCurr.x = -MOVE_VELOCITY_HERO; // Move hero to the left.
		flip = true; // Ensure the hero faces left.
	}
	else {
		pPlayer2->velCurr.x = 0.f; // Stop horizontal movement if no keys are pressed.
	}


	if (AEInputCheckTriggered(AEVK_UP))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{
		if (pPlayer2->gridCollisionFlag & COLLISION_BOTTOM) {
			pPlayer2->velCurr.y = JUMP_VELOCITY;
		}
		else if (pPlayer2->jumpCount < pPlayer2->maxJumps) // Check if player can double jump
		{
			pPlayer2->velCurr.y = JUMP_VELOCITY; // Apply jump velocity.
			pPlayer2->jumpCount++; // Increment jump count
		}
	}
	if (AEInputCheckTriggered(AEVK_ESCAPE))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{		
		gGameStateNext = GS_MAINMENU; // Change game state to main menu.
	}
}

// =========================================================
// 
// Update object instances physics-gravity
//  -- Gravity applied onto enemies and hero only
// 
// =========================================================
void Apply_GravityPhysics(void)
{
	int i = 0;
	GameObjInst* pInst = nullptr;

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		pInst = sGameObjInstList + i;

		// Skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;


		/****************
		Apply gravity
			Velocity Y = Gravity * Frame Time + Velocity Y
		****************/
		if (pInst->pObject->type == TYPE_OBJECT_PLAYER1 || pInst->pObject->type == TYPE_OBJECT_PLAYER2)
			pInst->velCurr.y += (f32)(GRAVITY * g_dt);
	}
}

// ======================================================================
// 
// update colliders of all active game object instances
//  -- Calculate the AABB bounding rectangle of the active instance, using the starting position:
//		boundingRect_min = -(BOUNDING_RECT_SIZE/2.0f) * instance->scale + instance->posPrev
//		boundingRect_max = +(BOUNDING_RECT_SIZE/2.0f) * instance->scale + instance->posPrev
//
// ======================================================================
void Update_BoundingBoxes(void)
{
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i; 

		// Skip inactive objects
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		// Calculate the min and max corners of the bounding box
		pInst->boundingBox.min.x = -(BOUNDING_RECT_SIZE / 2.0f) * pInst->scale.x + pInst->posPrev.x;
		pInst->boundingBox.min.y = -(BOUNDING_RECT_SIZE / 2.0f) * pInst->scale.y + pInst->posPrev.y;
		pInst->boundingBox.max.x = +(BOUNDING_RECT_SIZE / 2.0f) * pInst->scale.x + pInst->posPrev.x;
		pInst->boundingBox.max.y = +(BOUNDING_RECT_SIZE / 2.0f) * pInst->scale.y + pInst->posPrev.y;
	}
}

// ======================================================================
// 
// update physics of all active game object instances
//  -- New position of the active instance is updated here with the velocity calculated earlier
// 
// ======================================================================
void Update_Positions(void)
{
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;

		// Skip inactive objects
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		// Update the position using the velocity
		AEVec2 added;
		AEVec2Set(&added, pInst->velCurr.x * (float)g_dt, pInst->velCurr.y * (float)g_dt); // Position = Velocity * deltaTime
		AEVec2Add(&pInst->posCurr, &pInst->posCurr, &added); // Update position
		/*if (pInst->pObject->type == TYPE_OBJECT_PLAYER1)
			std::cout << "Position: " << pInst->posCurr.x << ", " << pInst->posCurr.y << std::endl;*/
	}
}

// ======================================================================
// 
// check for grid's binary collision
//  -- check hero and enemies against binary grid collision
// 
// ======================================================================
void Check_GridBinaryCollision(void)
{
	int i = 0;
	GameObjInst* pInst = nullptr;

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		pInst = sGameObjInstList + i;

		// Skip non-active object instances
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;
		if (pInst->pObject->type != TYPE_OBJECT_PLAYER1 && pInst->pObject->type != TYPE_OBJECT_PLAYER2)
			continue;


		// Collision response:
		pInst->gridCollisionFlag = CheckInstanceBinaryMapCollision(pInst->posCurr.x, pInst->posCurr.y, pInst->scale.x, pInst->scale.y);

		// If collision from bottom
		if (pInst->gridCollisionFlag & COLLISION_BOTTOM)
		{
			// Snap to the cell on the Y axis for bottom collision
			SnapBottomCollision(&pInst->posCurr.y, pInst->scale.y);
			pInst->velCurr.y = 0;
		}

		// If collision from top
		if (pInst->gridCollisionFlag & COLLISION_TOP)
		{
			// Snap using the new top collision function
			SnapTopCollision(&pInst->posCurr.y, pInst->scale.y);
			pInst->velCurr.y *= 0.9f;
			std::cout << "Top Collision Detected" << std::endl;
		}

		// If collision from left
		if (pInst->gridCollisionFlag & COLLISION_LEFT)
		{
			// Snap using the new left collision function
			SnapLeftCollision(&pInst->posCurr.x, pInst->scale.x);
			pInst->velCurr.x = 0;
			std::cout << "Left Collision Detected" << std::endl;
		}

		// If collision from right
		if (pInst->gridCollisionFlag & COLLISION_RIGHT)
		{
			// Snap using the new right collision function
			SnapRightCollision(&pInst->posCurr.x, pInst->scale.x);
			pInst->velCurr.x = 0;
			std::cout << "Right Collision Detected" << std::endl;
		}

		// Reset jump count if the player is on the ground
		if (pInst->gridCollisionFlag & COLLISION_BOTTOM)
		{
			pInst->jumpCount = 0; // Reset jump count
		}
	}
}

// ======================================================================
// 
// check for rectangle-rectangle collisions
// checking for dynamic collision among object instances:
//  -- Hero against enemies
//  -- Hero against coins
// 
// ======================================================================
void Update_AABBCollisions(void)
{
	/**********
	for each game object instance
		Skip if it's inactive or if it's non collidable

		If it's an enemy
			If collision between the enemy instance and the hero (rectangle - rectangle)
				Decrement hero lives
				Reset the hero's position in case it has lives left, otherwise RESTART the level

		If it's a coin
			If collision between the coin instance and the hero (rectangle - rectangle)
				Remove the coin and decrement the coin counter.
				Go to level2, in case no more coins are left and you are at Level1.
				Quit the game level to the main menu, in case no more coins are left and you are at Level2.
	**********/

	int i = 0;
	GameObjInst* pInst = nullptr;
	float tFirst = 0.f;

	// Loop through all game object instances
	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		pInst = sGameObjInstList + i;

		// Skip non-active object instances
		if ((pInst->flag & FLAG_ACTIVE) == 0) 
			continue;

		// Skip objects that are neither enemies nor coins
		if (pInst->pObject->type != TYPE_OBJECT_PLAYER2) 
			continue;

		// Check collision if the object is an enemy
		if (pInst->pObject->type == TYPE_OBJECT_PLAYER2) 
		{
			// Check for collision between enemy and hero
			if (CollisionIntersection_RectRect(pInst->boundingBox, pInst->velCurr, pPlayer1->boundingBox, pPlayer1->velCurr, tFirst))
			{
			}
		}
	}
}

// =====================================================================
// 
// calculate the concatenated matrix transformation for all objects
// 
// =====================================================================
void Update_ObjectsTransformations(void)
{
	int i = 0;
	GameObjInst* pInst = nullptr;

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		AEMtx33 scale, rot, trans;
		pInst = sGameObjInstList + i;

		// Skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		// Compute the scaling matrix
		AEMtx33Scale(&scale, pInst->scale.x, pInst->scale.y);

		// Compute the rotation matrix
		AEMtx33Rot(&rot, pInst->dirCurr);

		// Compute the translation matrix
		AEMtx33Trans(&trans, pInst->posCurr.x, pInst->posCurr.y);

		// Concatenate the 3 matrix in the correct order in the object instance's "transform" matrix
		AEMtx33Concat(&rot, &rot, &scale);
		AEMtx33Concat(&trans, &trans, &rot);

		// Save the result into the object instance's transformation matrix
		pInst->transform = trans;
	}
}

// =====================================================================
// 
// update camera position
//  -- follows the player
//  -- clamps to the level's borders
// 
// =====================================================================
void Update_CameraPosition_Level2(void)
{
	// Update Camera World position, for Level 2 only!
		// To follow the player's position
		// To clamp the camera position at the level's borders, the clamp must be between (0,0) position
		// and (maxCamPosition.x, maxCamPosition.y) position that you need to calculate
			// You may use an alpha engine helper function to clamp the camera position: AEClamp()
				// Example: camPos.x = AEClamp(camPos.x, 0.0f, maxCamPosition.x);
		// To set the camera position use AEGfxSetCamPosition()

	float cellWidth = AEGfxGetWindowWidth() / static_cast<f32>(GRID_WIDTH_IN_VIEWPORT);
	float levelWidth = BINARY_MAP_WIDTH * cellWidth;

	float cellHeight = AEGfxGetWindowHeight() / static_cast<f32>(GRID_HEIGHT_IN_VIEWPORT);
	float levelHeight = BINARY_MAP_HEIGHT * cellHeight;

	// Calculate half window dimensions:
	float halfWindowWidth = AEGfxGetWindowWidth() * 0.5f;
	float halfWindowHeight = AEGfxGetWindowHeight() * 0.5f;

	// Determine max camera positions if camera is centered:
	float maxCamX = levelWidth - halfWindowWidth*2;
	float maxCamY = levelHeight - halfWindowHeight*2;

	// Clamp the camera position:
	AEVec2 camPos{ pPlayer1->posCurr.x * cellWidth - halfWindowWidth, pPlayer1->posCurr.y * cellHeight - halfWindowHeight };
	camPos.x = AEClamp(camPos.x, 0.f, maxCamX);
	camPos.y = AEClamp(camPos.y, 0.f, maxCamY);

	// Set the camera position
	AEGfxSetCamPosition(camPos.x, camPos.y);
}

// =====================================================================
// 
// drawing the tile map
//  -- it must be drawn before the dynamic objects
// 
// =====================================================================
void Draw_TileMap_BackgroundGrid(void)
{
	int i = 0, j = 0;

	AEMtx33 cellTranslation, cellFinalTransformation;

	for (i = 0; i < BINARY_MAP_WIDTH; ++i)
		for (j = 0; j < BINARY_MAP_HEIGHT; ++j)
		{
			// Compute translation matrix for each cell
			AEMtx33Trans(&cellTranslation, i + 0.5f, j + 0.5f);

			// Concatenate the cell's translation with the MapTransform matrix
			AEMtx33Concat(&cellFinalTransformation, &MapTransform, &cellTranslation);
			

			// Set the current object instance's transform matrix using "AEGfxSetTransform"
			AEGfxSetTransform(cellFinalTransformation.m);//DO NOT change or update or duplicate this code line! Otherwise penalties may apply!


			// Draw the shape used by the current object instance using "AEGfxMeshDraw" here
			if (BinaryCollisionArray[j][i] == TYPE_OBJECT_EMPTY)
			{
				// Draw the black instance for empty cells
				AEGfxMeshDraw(pBlackInstance->pObject->pMesh, AE_GFX_MDM_TRIANGLES);
			}
			else if (BinaryCollisionArray[j][i] == TYPE_OBJECT_COLLISION)

			{
				// Draw the white instance for collision cells
				AEGfxMeshDraw(pWhiteInstance->pObject->pMesh, AE_GFX_MDM_TRIANGLES);
			}
			
		}
}

// ----------------------------------------------------------------------------
//
// draw all dynamic object instances in the list
//  -- it must be drawn after the tile map background
//
// ----------------------------------------------------------------------------
void Draw_DynamicObjectsInstances(void)
{
	/******REMINDER*****
	You need to concatenate MapTransform with the transformation matrix
	of any object you want to draw. MapTransform transform the instance
	from the normalized coordinates system of the binary map

	For each active and visible object instance
		Concatenate MapTransform with its transformation matrix
		and save the result in "objectFinalTransformation"
		Send "objectFinalTransformation" matrix to the graphics manager using "AEGfxSetTransform"
		Draw the instance's shape using "AEGfxMeshDraw"
	*******************/

	AEMtx33 objectFinalTransformation;

	int i = 0;

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;

		// Skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0 || (pInst->flag & FLAG_VISIBLE) == 0)
			continue;

		// Apply any additional rendering setup - if needed here

		// Concatenate the MapTransform matrix with the transformation of each game object instance here
		AEMtx33Concat(&objectFinalTransformation, &MapTransform, &pInst->transform);

		// Set the current object instance's transform matrix using "AEGfxSetTransform"
		AEGfxSetTransform(objectFinalTransformation.m);//DO NOT change or update or duplicate this code line! Otherwise penalties may apply!

		// Draw the shape used by the current object instance using "AEGfxMeshDraw" here
		AEGfxMeshDraw(pInst->pObject->pMesh, AE_GFX_MDM_TRIANGLES);

	}
}

// ----------------------------------------------------------------------------
//
// display game stats using in game text objects
//
// ----------------------------------------------------------------------------
void Display_GameStats(void)
{
	
}

// ----------------------------------------------------------------------------
//
// destroy all object instances in the array using "gameObjInstDestroy"
//
// ----------------------------------------------------------------------------
void Destroy_ObjectsInstances(void)
{
	// destroy all object in the list
	for (unsigned int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
		gameObjInstDestroy(sGameObjInstList + i);
}

// ----------------------------------------------------------------------------
//
// free all mesh data (shapes) of each object using "AEGfxTriFree"
// free objects data and map data
//
// ----------------------------------------------------------------------------
void Free_AllAllocatedMemory(void)
{
	// free all the created meshes
	for (u32 i = 0; i < sGameObjNum; i++)
		AEGfxMeshFree(sGameObjList[i].pMesh);

	// Free the object instances list (sGameObjInstList)
	if (sGameObjInstList)
	{
		free(sGameObjInstList);  // Free the memory for game object instances
		sGameObjInstList = nullptr;  // Set to nullptr after freeing
	}

	// Free the game objects list (sGameObjList)
	if (sGameObjList)
	{
		free(sGameObjList);  // Free the memory for game objects
		sGameObjList = nullptr;  // Set to nullptr after freeing
	}

	// Delete MapData
	if (MapData) {
		for (int i = 0; i < BINARY_MAP_HEIGHT; ++i) {
			delete[] MapData[i];
		}
		delete[] MapData;
		MapData = nullptr;
	}

	// Delete BinaryCollisionArray
	if (BinaryCollisionArray) {
		for (int i = 0; i < BINARY_MAP_HEIGHT; ++i) {
			delete[] BinaryCollisionArray[i];
		}
		delete[] BinaryCollisionArray;
		BinaryCollisionArray = nullptr;
	}
}

// ----------------------------------------------------------------------------
//
//	This function retrieves the value of the element (X;Y) in BinaryCollisionArray.
//	Before retrieving the value, it should check that the supplied X and Y values
//	are not out of bounds (in that case return 0)
//
// ----------------------------------------------------------------------------
int GetCellValue(int X, int Y)
{
	// Check if the provided X and Y are within the bounds
	if (X >= 0 && X < BINARY_MAP_WIDTH && Y >= 0 && Y < BINARY_MAP_HEIGHT) {
		// Return the value if within bounds
		return BinaryCollisionArray[Y][X];
	}
	// Out of bounds
	return 0;
}

// ----------------------------------------------------------------------------
//
//	This function creates 2 hot spots on each side of the object instance, 
//	and checks if each of these hot spots is in a collision area (which means 
//	the cell if falls in has a value of 1).
//	At the beginning of the function, a "Flag" integer should be initialized to 0.
//	Each time a hot spot is in a collision area, its corresponding bit 
//	in "Flag" is set to 1.
//	Finally, the function returns the integer "Flag"
//	The position of the object instance is received as PosX and PosY
//	The size of the object instance is received as scaleX and scaleY
//
//	Note: This function assume the object instance's size is 1 by 1 
//		  (the size of 1 tile)
//
//	Creating the hotspots:
//		-Handle each side separately.
//		-2 hot spots are needed for each collision side.
//		-These 2 hot spots should be positioned on 1/4 above the center 
//		and 1/4 below the center
//
//	Example: Finding the hots spots on the left side of the object instance
//
//	float x1, y1, x2, y2;
//
//	-hotspot 1
//	x1 = PosX + scaleX/2	To reach the right side
//	y1 = PosY + scaleY/4	To go up 1/4 of the height
//	
//	-hotspot 2
//	x2 = PosX + scaleX/2	To reach the right side
//	y2 = PosY - scaleY/4	To go down 1/4 of the height
//
// ----------------------------------------------------------------------------
int CheckInstanceBinaryMapCollision(float PosX, float PosY, float scaleX, float scaleY)
{
	//At the end of this function, "Flag" will be used and returned, to determine which sides
	//of the object instance are colliding. 2 hot spots will be placed on each side.

	int Flag = 0;

	const int numVerticalSamples = 4;
	for (int i = 0; i < numVerticalSamples; ++i) {
		float t = (float)(i + 1) / (numVerticalSamples + 1); // avoids very top and bottom
		float sampleY = PosY + scaleY / 2 - scaleY * t; // from top to bottom, offset inwards

		// LEFT
		float leftX = PosX - scaleX / 2 * 0.99f;
		if (GetCellValue((int)leftX, (int)sampleY) == 1)
			Flag |= COLLISION_LEFT;

		// RIGHT
		float rightX = PosX + scaleX / 2 * 0.99f;
		if (GetCellValue((int)rightX, (int)sampleY) == 1)
			Flag |= COLLISION_RIGHT;
	}


	// Check for top side
	float topY = PosY + scaleY / 2;
	float x1 = PosX - scaleX * 3 / 8.0f;
	float x2 = PosX - scaleX / 4.0f;
	float x3 = PosX + scaleX / 4.0f;
	float x4 = PosX + scaleX * 3 / 8.0f;

	if (GetCellValue((int)x1, (int)topY) == 1 ||
		GetCellValue((int)x2, (int)topY) == 1 ||
		GetCellValue((int)x3, (int)topY) == 1 ||
		GetCellValue((int)x4, (int)topY) == 1) {
		Flag |= COLLISION_TOP;
	}

	// Check for bottom side
	float bottomY = PosY - scaleY / 2;

	if (GetCellValue((int)x1, (int)bottomY) == 1 ||
		GetCellValue((int)x2, (int)bottomY) == 1 ||
		GetCellValue((int)x3, (int)bottomY) == 1 ||
		GetCellValue((int)x4, (int)bottomY) == 1) {
		Flag |= COLLISION_BOTTOM;
	}

	return Flag;
}

// ----------------------------------------------------------------------------
//
//	This function snaps the value sent as parameter to the center of the cell.
//	It is used when a sprite is colliding with a collision area from one 
//	or more side.
//	To snap the value sent by "Coordinate", find its integral part by type 
//	casting it to an integer, then add 0.5 (which is half the cell's width 
//	or height)
//
// ----------------------------------------------------------------------------
void SnapToCell(float* Coordinate)
{
	// Cast the coordinate to an integer to find its integral part
	int integralPart = static_cast<int>(*Coordinate);

	// Add 0.5 to snap it to the center of the cell
	*Coordinate = static_cast<float>(integralPart) + 0.5f;
}

void SnapBottomCollision(float* posY, float playerHeight)
{
	// Calculate the bottom edge of the player.
	float bottomEdge = *posY - playerHeight / 2;
	int cellIndex = static_cast<int>(bottomEdge);  // cell index where the player's bottom is
	// Snap the bottom edge to the top of the cell, which is cellIndex+1 (since cell boundaries are at integers)
	float snappedBottom = static_cast<float>(cellIndex + 1);
	// Re-adjust the center position so that bottom edge is at snappedBottom.
	*posY = snappedBottom + playerHeight / 2;
}

void SnapTopCollision(float* posY, float playerHeight)
{
	// Calculate the top edge of the player.
	float topEdge = *posY + playerHeight / 2;
	int cellIndex = static_cast<int>(topEdge);  // Cell index where the player's top is
	// Snap the top edge to the bottom boundary of that cell (which is simply the cell index).
	float snappedTop = static_cast<float>(cellIndex);
	// Adjust the center so that the top edge is at snappedTop.
	*posY = snappedTop - playerHeight / 2;
}

void SnapLeftCollision(float* posX, float playerWidth)
{
	// Calculate the left edge of the player.
	float leftEdge = *posX - playerWidth / 2;
	int cellIndex = static_cast<int>(leftEdge); // Cell index where the player's left edge is.
	// Snap the left edge to the right boundary of that cell, which is cellIndex + 1.
	float snappedLeft = static_cast<float>(cellIndex + 1);
	// Adjust the center so that the left edge sits exactly at snappedLeft.
	*posX = snappedLeft + playerWidth / 2;
}

void SnapRightCollision(float* posX, float playerWidth)
{
	// Calculate the right edge of the player.
	float rightEdge = *posX + playerWidth / 2;
	int cellIndex = static_cast<int>(rightEdge); // Cell index where the player's right edge is.
	// Snap the right edge to the left boundary of that cell (which is the cell index).
	float snappedRight = static_cast<float>(cellIndex);
	// Adjust the center so that the right edge is exactly at snappedRight.
	*posX = snappedRight - playerWidth / 2;
}

void SpawnPlayers()
{
	bool player1Spawned = false;
	bool player2Spawned = false;

	while ((!player1Spawned || !player2Spawned)) {
		int x = 1 + rand() % (BINARY_MAP_WIDTH - 2);  // Ensure x is within bounds
		int y = 2 + rand() % (BINARY_MAP_HEIGHT - 12); // Ensure y is within bounds (leaving space above)

		//Check the 8 surrounding tiles around the player
		bool isAreaFree = MapData[y][x] == 0 &&
		MapData[y][x + 1] == 0 &&
		MapData[y][x - 1] == 0 &&
		MapData[y - 1][x] == 0 &&
		MapData[y - 1][x + 1] == 0 &&
		MapData[y - 1][x - 1] == 0 &&
		MapData[y + 1][x] == 0 &&
		MapData[y + 1][x + 1] == 0 &&
		MapData[y + 1][x - 1] == 0;

		// Check if the spot is empty and safe for spawn (no terrain in the way)
		bool isBottomSolid = MapData[y - 2][x] == 1 && MapData[y + 1][x] == 0 && MapData[y + 2][x] == 0;

		if (isAreaFree && isBottomSolid) {
			if (!player1Spawned) {
				MapData[y][x] = 2;  // Mark player 1's spawn point
				player1Spawned = true;
			}
			else if (!player2Spawned) {
				// Don't spawn both players on the same tile
				if (MapData[y][x] == 0) {
					MapData[y][x] = 3;  // Mark player 2's spawn point
					player2Spawned = true;
				}
			}
		}
	}
}



int GenerateRandomMap(void)
{
	int width = BINARY_MAP_WIDTH;
	int height = BINARY_MAP_HEIGHT;

	MapData = new int* [height];
	BinaryCollisionArray = new int* [height];

	for (int i = 0; i < height; ++i) {
		MapData[i] = new int[width];
		BinaryCollisionArray[i] = new int[width];
	}

	// Terrain parameters
	int baseGroundHeight = 8;
	int minHeight = baseGroundHeight - 1;
	int maxHeight = baseGroundHeight + 1;
	int currentHeight = baseGroundHeight;

	// Floating platform parameters
	float platformChance = 0.5f;
	int platformMinLength = 3;
	int platformMaxLength = 6;

	// Generate smooth terrain
	for (int j = 0; j < width; ++j)
	{
		// Smooth terrain: usually no change, but occasionally +1 or -1
		int step = 0;

		// Occasionally change the height by +1 or -1 with a low probability
		if (rand() % 100 < 10)  // 10% chance to change the height
		{
			step = (rand() % 2 == 0) ? 2 : -2;  // Change by +1 or -1
		}

		currentHeight += step;

		// Clamp to min/max bounds
		if (currentHeight < minHeight) currentHeight = minHeight;
		if (currentHeight > maxHeight) currentHeight = maxHeight;

		for (int i = 0; i < height; ++i)
		{
			MapData[i][j] = (i <= currentHeight) ? 1 : 0;
		}
	}

	// Add longer floating platforms in the sky, with enough space for 4-block-tall players
	for (int attempt = 0; attempt < 50; ++attempt)
	{
		if ((rand() % 100) < (int)(platformChance * 1000))
		{
			int platformLength = platformMinLength + rand() % (platformMaxLength - platformMinLength + 1);
			int startX = rand() % (width - platformLength - 1);

			// Generate platform high enough to allow player standing space (e.g., 4 blocks of air above)
			int minY = maxHeight + 4;
			int maxY = height - 6; // avoid placing too close to top
			int y = minY + rand() % (maxY - minY);

			// Check clearance before placing
			bool canPlace = true;
			for (int i = 0; i < platformLength && canPlace; ++i)
			{
				int x = startX + i;

				// Check 4 blocks above + 1 below
				for (int offsetY = -4; offsetY <= 4; ++offsetY)
				{
					if (MapData[y - offsetY][x] != 0)
					{
						canPlace = false;
						break;
					}
				}
			}

			if (canPlace)
			{
				// Place platform
				for (int x = startX; x < startX + platformLength; ++x)
				{
					MapData[y][x] = 1;
				}
			}
		}
	}


	// Collision data
	for (int i = 0; i < height; ++i)
		for (int j = 0; j < width; ++j)
			BinaryCollisionArray[i][j] = (MapData[i][j] == 1) ? 1 : 0;

	SpawnPlayers();
	return 1;
}



// ----------------------------------------------------------------------------
//
//	This function opens the file name "FileName" and retrieves all the map data.
//	It allocates memory for the 2 arrays: MapData & BinaryCollisionArray
//	The first line in this file is the width of the map.
//	The second line in this file is the height of the map.
//	The remaining part of the file is a series of numbers
//	Each number represents the ID (or value) of a different element in the 
//	double dimensionaly array.

int ImportMapDataFromFile(char* FileName)
{
	std::ifstream file(FileName);

	if (!file) {
		return 0;
	}

	std::string line;

	// Read the width and height from the file
	std::getline(file, line); // Read Width line
	BINARY_MAP_WIDTH = std::stoi(line.substr(6).c_str()); // Read width digit

	std::getline(file, line); // Read height line
	BINARY_MAP_HEIGHT = std::stoi(line.substr(7).c_str()); // Read height digit

	// Allocate memory for MapData and BinaryCollisionArray
	MapData = new int* [BINARY_MAP_HEIGHT];
	BinaryCollisionArray = new int* [BINARY_MAP_HEIGHT];

	for (int i = 0; i < BINARY_MAP_HEIGHT; ++i) {
		MapData[i] = new int[BINARY_MAP_WIDTH];
		BinaryCollisionArray[i] = new int[BINARY_MAP_WIDTH];
	}

	// Read the map data
	for (int i = 0; i < BINARY_MAP_HEIGHT; ++i) {
		for (int j = 0; j < BINARY_MAP_WIDTH; ++j) {
			file >> MapData[i][j]; // Read the map value

			// Populate the BinaryCollisionArray: 1 for "1", 0 for any other number
			BinaryCollisionArray[i][j] = (MapData[i][j] == 1) ? 1 : 0;
		}
	}

	file.close();
	return 1;
}

// ----------------------------------------------------------------------------
//
//	This function frees the memory that was allocated for the 2 arrays MapData 
//	& BinaryCollisionArray which was allocated in the "ImportMapDataFromFile" 
//	function
//
// ----------------------------------------------------------------------------
void FreeMapData(void)
{
	// Delete MapData
	if (MapData) {
		for (int i = 0; i < BINARY_MAP_HEIGHT; ++i) {
			delete[] MapData[i];
		}
		delete[] MapData;
		MapData = nullptr;
	}

	// Delete BinaryCollisionArray
	if (BinaryCollisionArray) {
		for (int i = 0; i < BINARY_MAP_HEIGHT; ++i) {
			delete[] BinaryCollisionArray[i];
		}
		delete[] BinaryCollisionArray;
		BinaryCollisionArray = nullptr;
	}
}

// ----------------------------------------------------------------------------
//
//	This function prints out the content of the 2D array “MapData”
//	You must print to the console, the same information you are reading from "Exported.txt" file
//	Follow exactly the same format of the file, including the print of the width and the height
//	Add spaces and end lines at convenient places
//
// ----------------------------------------------------------------------------
void PrintRetrievedInformation(void)
{
	// Print Width and Height as per the format
	std::cout << "Width " << BINARY_MAP_WIDTH << std::endl;
	std::cout << "Height " << BINARY_MAP_HEIGHT << std::endl;

	// Loop through the 2D array MapData to print the map
	std::cout << "MapData: " << std::endl;
	for (int i = 0; i < BINARY_MAP_HEIGHT; ++i) {
		for (int j = 0; j < BINARY_MAP_WIDTH; ++j) {
			std::cout << MapData[i][j] << " ";  // Print each value followed by space
		}
		std::cout << std::endl;
	}

	// Loop through the 2D array BinaryCollisionArray to print the map
	std::cout << "\nBinaryCollisionArray: " << std::endl;
	for (int i = 0; i < BINARY_MAP_HEIGHT; ++i) {
		for (int j = 0; j < BINARY_MAP_WIDTH; ++j) {
			std::cout << BinaryCollisionArray[i][j] << " ";  // Print each value followed by space
		}
		std::cout << std::endl;
	}
}
