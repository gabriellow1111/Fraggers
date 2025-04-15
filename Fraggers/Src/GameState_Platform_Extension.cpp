/* Start Header ************************************************************************/
/*!
\file		GameState_Platform_Extension.cpp
\author 	Gabriel Low, gabriel.low, 620006124
\par    	gabriel.low@digipen.edu
\date   	7 March 2025
\brief		This file implements important helper functions and
			essential functions for the platform game state implementation.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

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
	BINARY_MAP_WIDTH = 0;
	BINARY_MAP_HEIGHT = 0;

	// LEVEL 1
	if (gGameStateCurr == GS_PLATFORM1)
	{
		//Importing Data - Must Quit the application if "ImportMapDataFromFile" fails
		if (!ImportMapDataFromFile("..\\Resources\\Levels\\Exported.txt"))
			gGameStateNext = GS_QUIT;

	}

	// LEVEL 2
	else if (gGameStateCurr == GS_PLATFORM2)
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
	//Computing the matrix which take a point out of the normalized coordinates system
	//of the binary map
	/***********
	Compute a transformation matrix and save it in "MapTransform".
	This transformation transforms any point from the normalized coordinates system of the binary map.
	Later on, when rendering each object instance, we should concatenate "MapTransform" with the
	object instance's own transformation matrix

	Compute a translation matrix (-GRID_WIDTH_IN_VIEWPORT/2, -GRID_WIDTH_IN_VIEWPORT/2) and save it in "trans"
	Compute a scaling matrix and save it in "scale". 
		The scale must account for the window width and height.
		Alpha engine has 2 helper functions to get the window width and height: AEGfxGetWindowWidth() and AEGfxGetWindowHeight()
		Example: scale along x-axis = AEGfxGetWindowWidth() / static_cast<f32>(GRID_WIDTH_IN_VIEWPORT)
	Concatenate scale and translate and save the result in "MapTransform"
	***********/

	// Declare transformation matrices for scaling and translation.
	AEMtx33 scale, trans;

	// Create a translation matrix to center the grid in the viewport.
	AEMtx33Trans(&trans, -(GRID_WIDTH_IN_VIEWPORT / 2), -(GRID_WIDTH_IN_VIEWPORT / 2));

	// Create a scaling matrix to scale the grid to fit the window dimensions.
	AEMtx33Scale(&scale, AEGfxGetWindowWidth() / static_cast<f32>(GRID_WIDTH_IN_VIEWPORT),
						 AEGfxGetWindowHeight() / static_cast<f32>(GRID_HEIGHT_IN_VIEWPORT));

	// Combine the scaling and translation matrices to form the final transformation matrix.
	AEMtx33Concat(&MapTransform, &scale, &trans);
}

// =========================================================
// 
// creates all initial game objects instances
// 
// =========================================================
void Starting_GameObjectsInstances(void)
{
	int i = 0, j = 0;

	pHero = nullptr;
	pBlackInstance = nullptr;
	pWhiteInstance = nullptr;
	TotalCoins = 0;

	//Create an object instance representing the black cell.
	//This object instance should not be visible. When rendering the grid cells, each time we have
	//a non collision cell, we position this instance in the correct location and then we render it
	AEVec2 scl = { 1.0f, 1.0f };
	pBlackInstance = gameObjInstCreate(TYPE_OBJECT_EMPTY, &scl, 0, 0, 0.0f);
	pBlackInstance->flag ^= FLAG_VISIBLE;
	pBlackInstance->flag |= FLAG_NON_COLLIDABLE;

	//Create an object instance representing the white cell.
	//This object instance should not be visible. When rendering the grid cells, each time we have
	//a collision cell, we position this instance in the correct location and then we render it
	pWhiteInstance = gameObjInstCreate(TYPE_OBJECT_COLLISION, &scl, 0, 0, 0.0f);
	pWhiteInstance->flag ^= FLAG_VISIBLE;
	pWhiteInstance->flag |= FLAG_NON_COLLIDABLE;

	//Setting the inital number of hero lives
	HeroLives = HERO_LIVES;

	GameObjInst* pInst = nullptr;
	AEVec2 Pos = { 0.f,0.f };

	// creating the main character, the enemies and the coins according 
	// to their initial positions in MapData

	/***********
	Loop through all the array elements of MapData
	(which was initialized in the "GameStatePlatformLoad" function
	from the .txt file)
		if the element represents a collidable or non collidable area
			don't do anything

		if the element represents the hero
			Create a hero instance
			Set its position depending on its array indices in MapData
			Save its array indices in Hero_Initial_X and Hero_Initial_Y
			(Used when the hero dies and its position needs to be reset)

		if the element represents an enemy
			Create an enemy instance
			Set its position depending on its array indices in MapData

		if the element represents a coin
			Create a coin instance
			Set its position depending on its array indices in MapData

	***********/

	// Loop through each cell in the binary map grid.
	for (i = 0; i < BINARY_MAP_WIDTH; ++i)
		for (j = 0; j < BINARY_MAP_HEIGHT; ++j)
		{
			// Set the position of the current cell in the world space.
			AEVec2Set(&Pos, (f32)i + 0.5f, (f32)j + 0.5f);

			// Check the type of object in the current map cell.
			switch (MapData[j][i]) 
			{
			case (TYPE_OBJECT_COLLISION):
			case (TYPE_OBJECT_EMPTY):
				// Do nothing for empty or collision cells.
				break;
			case (TYPE_OBJECT_HERO):
				// Store the hero's initial position.
				Hero_Initial_X = i;
				Hero_Initial_Y = j;

				// Create the hero object instance.
				pHero = gameObjInstCreate(TYPE_OBJECT_HERO, &scl, &Pos, 0, 0.0f);
				break;
			case (TYPE_OBJECT_ENEMY):
				// Create an enemy object instance at this position.
				pInst = gameObjInstCreate(TYPE_OBJECT_ENEMY, &scl, &Pos, 0, 0.0f);
				break;
			case (TYPE_OBJECT_COIN):
				// Create a coin object instance at this position.
				pInst = gameObjInstCreate(TYPE_OBJECT_COIN, &scl, &Pos, 0, 0.0f);

				// Increase the total coin counter.
				TotalCoins++;
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
	/***********
	if Right arrow key is pressed
		Set hero velocity X to MOVE_VELOCITY_HERO
	else
	if Left arrow key is pressed
		Set hero velocity X to -MOVE_VELOCITY_HERO
	else
		Set hero velocity X to 0

	if Spacebar key is triggered 
		if Hero is colliding from the bottom
			Set hero velocity Y to JUMP_VELOCITY

	if Escape key is triggered
		Exit to main menu
	***********/

	if (AEInputCheckCurr(AEVK_RIGHT))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{		
		pHero->velCurr.x = MOVE_VELOCITY_HERO; // Move hero to the right
		flip = false; // Ensure the hero faces right.
	}
	else if (AEInputCheckCurr(AEVK_LEFT))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{
		pHero->velCurr.x = -MOVE_VELOCITY_HERO; // Move hero to the left.
		flip = true; // Ensure the hero faces left.
	}
	else {
		pHero->velCurr.x = 0.f; // Stop horizontal movement if no keys are pressed.
	}
	

	if (AEInputCheckTriggered(AEVK_SPACE))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{
		// Allow jumping only if the hero is standing on the ground.
		if (pHero->gridCollisionFlag & COLLISION_BOTTOM) {
			pHero->velCurr.y = JUMP_VELOCITY; // Apply jump velocity.
		}
	}
	
	if (AEInputCheckTriggered(AEVK_ESCAPE))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{		
		gGameStateNext = GS_MAINMENU; // Change game state to main menu.
	}
}

// =========================================================
// 
// Particle System
//  -- Particles creation with random properties
//  -- Do not exceed 100 particles running at once, per frame
//  -- Particles spawn relative to Hero's location
//  -- You can use "AERandFloat()" helper AE function	
// 
// =========================================================
void Hero_Particles_Creation(void)
{
	AEVec2 scl{}, pos{}, vel{};

	int activeParticles = 0;

	// Count active particles
	for (unsigned int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		// Check if the object is active and is a particle
		if (sGameObjInstList[i].flag & FLAG_ACTIVE &&
			sGameObjInstList[i].pObject->type == TYPE_OBJECT_PARTICLE)
		{
			activeParticles++;
		}
	}

	// Limit the number of active particles to 200
	if (activeParticles < 200)
	{
		// Randomized scale
		scl.x = 0.3f + AERandFloat() * 0.2f;
		scl.y = 0.5f + AERandFloat() * 0.1f;

		// Offset from hero position (randomized for spread effect)
		pos.x = pHero->posCurr.x - AERandFloat() * 0.05f - 0.3f;
		pos.y = pHero->posCurr.y + AERandFloat() * 1.0f + 0.3f;

		// Set velocity aimed upward with slight left/right variation
		vel.x = AERandFloat() * -0.01f - 0.8f;
		vel.y = 4.f + AERandFloat() * 3.0f;

		// Flip particles x-pos and vel.x if player facing other side
		if (flip) {
			pos.x += 0.7f;
			vel.x = -vel.x;
		}
		// Create the particle instance
		GameObjInst* pParticle = gameObjInstCreate(TYPE_OBJECT_PARTICLE, &scl, &pos, &vel, 0.0f);

		// Assign a random lifetime between 0.5 and 1.0 seconds
		pParticle->lifetime = 0.5f + AERandFloat() * 0.5f;
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
		if (pInst->pObject->type == TYPE_OBJECT_HERO || pInst->pObject->type == TYPE_OBJECT_ENEMY)
			pInst->velCurr.y += (f32)(GRAVITY * g_dt);
	}
}

// =========================================================
// 
// Particle System
//  -- Particles destruction based on its lifetime	
// 
// =========================================================
void Hero_Particles_Destruction(void)
{
	for (unsigned int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++) 
	{
		GameObjInst* pInst = &sGameObjInstList[i]; 

		// Check if the instance is active and is a particle
		if ((pInst->flag & FLAG_ACTIVE) && pInst->pObject->type == TYPE_OBJECT_PARTICLE) 
		{
			// Reduce lifetime based on delta time 
			pInst->lifetime -= (float)g_dt; 

			// Destroy the particle if its lifetime has expired
			if (pInst->lifetime <= 0.0f) 
			{
				pInst->flag = 0; // Mark as inactive 
			}
		}
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
		if (pInst->pObject->type != TYPE_OBJECT_HERO && pInst->pObject->type != TYPE_OBJECT_ENEMY)
			continue;

		/*************
		Update grid collision flag

		if collision from bottom
			Snap to cell on Y axis
			Velocity Y = 0

		if collision from top
			Snap to cell on Y axis
			Velocity Y = 0

		if collision from left
			Snap to cell on X axis
			Velocity X = 0

		if collision from right
			Snap to cell on X axis
			Velocity X = 0
		*************/

		pInst->gridCollisionFlag = CheckInstanceBinaryMapCollision(pInst->posCurr.x, pInst->posCurr.y, pInst->scale.x, pInst->scale.y);

		// if collision from bottom
		if (pInst->gridCollisionFlag & COLLISION_BOTTOM)
		{
			// Snap to cell on the Y axis
			SnapToCell(&pInst->posCurr.y);
			pInst->velCurr.y = 0;
		}

		// if collision from top
		if (pInst->gridCollisionFlag & COLLISION_TOP)
		{
			// Snap to cell on the Y axis
			SnapToCell(&pInst->posCurr.y);
			pInst->velCurr.y = 0;
		}

		// if collision from left
		if (pInst->gridCollisionFlag & COLLISION_LEFT)
		{
			// Snap to cell on the X axis
			SnapToCell(&pInst->posCurr.x);
			pInst->velCurr.x = 0;
		}

		// if collision from right
		if (pInst->gridCollisionFlag & COLLISION_RIGHT)
		{
			// Snap to cell on the X axis
			SnapToCell(&pInst->posCurr.x);
			pInst->velCurr.x = 0;
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
		if (pInst->pObject->type != TYPE_OBJECT_ENEMY && pInst->pObject->type != TYPE_OBJECT_COIN) 
			continue;

		// Check collision if the object is an enemy
		if (pInst->pObject->type == TYPE_OBJECT_ENEMY) 
		{
			// Check for collision between enemy and hero
			if (CollisionIntersection_RectRect(pInst->boundingBox, pInst->velCurr, pHero->boundingBox, pHero->velCurr, tFirst))
			{
				HeroLives--; // Decrement hero lives by 1

				// Reset hero position if lives remain, otherwise restart level
				if (HeroLives > 0) {
					pHero->posCurr.x = (f32)Hero_Initial_X;
					pHero->posCurr.y = (f32)Hero_Initial_Y;
				}
				else {
					gGameStateNext = GS_RESTART;
				}
			}
		}

		// Check collision if the object is a coin
		if (pInst->pObject->type == TYPE_OBJECT_COIN)
		{
			// Check for collision between coin and hero
			if (CollisionIntersection_RectRect(pInst->boundingBox, pInst->velCurr, pHero->boundingBox, pHero->velCurr, tFirst))
			{
				pInst->flag = 0; // Remove the coin
				TotalCoins--; // Decrease total coin count

				// If all coins are collected, change game state
				if (TotalCoins == 0) {
					if (gGameStateCurr == GS_PLATFORM1)
					{
						gGameStateNext = GS_PLATFORM2; // Move to next level
					}
					else if (gGameStateCurr == GS_PLATFORM2)
					{
						gGameStateNext = GS_MAINMENU; // Return to main menu
					}
				}
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
	AEVec2 camPos{ pHero->posCurr.x * cellWidth - halfWindowWidth, pHero->posCurr.y * cellHeight - halfWindowHeight };
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
	/*********
	for each array element in BinaryCollisionArray (2 loops)
		Each cell needs to be translated by its half width and
		height. That is why you should compute the cell's translation 
		matrix according to its X and Y coordinates and save it in 
		"cellTranslation"
		Concatenate MapTransform with the cell's transformation
		and save the result in "cellFinalTransformation"
		Send "cellFinalTransformation" matrix to the graphics manager using "AEGfxSetTransform"

		Draw the instance's shape depending on the cell's value using "AEGfxMeshDraw"
			Use the black instance in case the cell's value is TYPE_OBJECT_EMPTY
			Use the white instance in case the cell's value is TYPE_OBJECT_COLLISION
	*********/

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
		if (pInst->pObject->type == TYPE_OBJECT_PARTICLE) {
			if (AERandFloat() > 0.8f) {
				AEGfxSetColorToMultiply(0.2f, 1.f, 0.2f, 0.5f); // Green particle
			}
			else {
				AEGfxSetColorToMultiply(0.2f, 0.2f, 1.f, 0.5f); // Blue particle
			}
		}
		else {
			AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f); // No color change
		}

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
	
	char strBuffer[100];
	memset(strBuffer, 0, 100 * sizeof(char));

	// Display Hero Lives
	sprintf_s(strBuffer, "Lives:  %i", HeroLives);
	AEGfxPrint(fontId, strBuffer, 0.7f, 0.9f, 1.f, 0.f, 0.f, 1.f, 1.f);

	// Display Total Coins left
	sprintf_s(strBuffer, "Coins left:  %i", TotalCoins);
	AEGfxPrint(fontId, strBuffer, -0.90f, 0.9f, 1.f, 0.f, 0.f, 1.f, 1.f);
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
// this state machine is used by an enemy to move left and right on a platform.
//
// ----------------------------------------------------------------------------
void EnemyStateMachine(GameObjInst* pInst)
{
	/***********
	This state machine has 2 states: STATE_GOING_LEFT and STATE_GOING_RIGHT
	Each state has 3 inner states: INNER_STATE_ON_ENTER, INNER_STATE_ON_UPDATE, INNER_STATE_ON_EXIT
	Use "switch" statements to determine which state and inner state the enemy is currently in.


	STATE_GOING_LEFT
		INNER_STATE_ON_ENTER
			Set velocity X to -MOVE_VELOCITY_ENEMY
			Set inner state to "on update"

		INNER_STATE_ON_UPDATE
			If collision on left side OR bottom left cell is non collidable
				Initialize the counter to ENEMY_IDLE_TIME
				Set inner state to on exit
				Set velocity X to 0
				Snap to cell the position along the x-axis


		INNER_STATE_ON_EXIT
			Decrement counter by frame time
			if counter is less than 0 (sprite's idle time is over)
				Set state to "going right"
				Set inner state to "on enter"

	STATE_GOING_RIGHT is basically the same, with few modifications.

	***********/

	// Ensure the instance is valid
	if (!pInst) return;
	
	switch (pInst->state)
	{
	case STATE_GOING_LEFT:
	{
		switch (pInst->innerState)
		{
		case INNER_STATE_ON_ENTER:
			pInst->velCurr.x = -MOVE_VELOCITY_ENEMY; // Move left
			pInst->innerState = INNER_STATE_ON_UPDATE; // Switch to update state
			break;

		case INNER_STATE_ON_UPDATE:
			// Check if enemy collides on left OR bottom left tile is not walkable
			if(pInst->gridCollisionFlag & COLLISION_LEFT || 
				!(CheckInstanceBinaryMapCollision(
				pInst->posCurr.x - pInst->scale.x / 2,
				pInst->posCurr.y - pInst->scale.y / 2,
				pInst->scale.x, 
				pInst->scale.y) & COLLISION_BOTTOM))
			{
				pInst->idle_counter = ENEMY_IDLE_TIME; // Initialize idle counter
				pInst->innerState = INNER_STATE_ON_EXIT;
				pInst->velCurr.x = 0; // Stop movement
				SnapToCell(&pInst->posCurr.x);
			}
			break;

		case INNER_STATE_ON_EXIT:
			pInst->idle_counter -= (float)g_dt; // Reduce idle counter
			if (pInst->idle_counter <= 0.0f)
			{
				pInst->state = STATE_GOING_RIGHT; // Switch to right movement
				pInst->innerState = INNER_STATE_ON_ENTER; // Reset inner state
			}
			break;
		}
		break;
	}

	case STATE_GOING_RIGHT:
	{
		switch (pInst->innerState)
		{
		case INNER_STATE_ON_ENTER:
			pInst->velCurr.x = MOVE_VELOCITY_ENEMY; // Move right
			pInst->innerState = INNER_STATE_ON_UPDATE; // Switch to update state
			break;

		case INNER_STATE_ON_UPDATE:
			// Check if enemy collides on right OR bottom right tile is not walkable

			if (pInst->gridCollisionFlag & COLLISION_RIGHT ||
				!(CheckInstanceBinaryMapCollision(
					pInst->posCurr.x + pInst->scale.x / 2,
					pInst->posCurr.y - pInst->scale.y / 2,
					pInst->scale.x,
					pInst->scale.y) & COLLISION_BOTTOM))
			{
				pInst->idle_counter = ENEMY_IDLE_TIME; // Initialize idle counter
				pInst->innerState = INNER_STATE_ON_EXIT;
				pInst->velCurr.x = 0; // Stop movement
				SnapToCell(&pInst->posCurr.x);
			}
			break;

		case INNER_STATE_ON_EXIT:
			pInst->idle_counter -= (float)g_dt; // Reduce idle counter
			if (pInst->idle_counter <= 0.0f)
			{
				pInst->state = STATE_GOING_LEFT; // Switch to right movement
				pInst->innerState = INNER_STATE_ON_ENTER; // Reset inner state
			}
			break;
		}
		break;
	}

	default:
		// Set an initial state if uninitialized
		pInst->state = STATE_GOING_LEFT;
		pInst->innerState = INNER_STATE_ON_ENTER;
		break;
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

	// Check for left side
	float x1 = PosX - scaleX / 2;
	float y1 = PosY + scaleY / 4;
	float x2 = PosX - scaleX / 2;
	float y2 = PosY - scaleY / 4;

	if (GetCellValue(static_cast<int>(x1), static_cast<int>(y1)) == 1 ||
		GetCellValue(static_cast<int>(x2), static_cast<int>(y2)) == 1) {
		Flag |= COLLISION_LEFT; // Set the left collision bit
	}

	// Check for right side
	x1 = PosX + scaleX / 2;
	y1 = PosY + scaleY / 4;
	x2 = PosX + scaleX / 2;
	y2 = PosY - scaleY / 4;

	if (GetCellValue(static_cast<int>(x1), static_cast<int>(y1)) == 1 ||
		GetCellValue(static_cast<int>(x2), static_cast<int>(y2)) == 1) {
		Flag |= COLLISION_RIGHT; // Set the right collision bit
	}

	// Check for top side
	x1 = PosX + scaleX / 4;
	y1 = PosY + scaleY / 2;
	x2 = PosX - scaleX / 4;
	y2 = PosY + scaleY / 2;

	if (GetCellValue(static_cast<int>(x1), static_cast<int>(y1)) == 1 ||
		GetCellValue(static_cast<int>(x2), static_cast<int>(y2)) == 1) {
		Flag |= COLLISION_TOP; // Set the top collision bit
	}

	// Check for bottom side
	x1 = PosX + scaleX / 4;
	y1 = PosY - scaleY / 2;
	x2 = PosX - scaleX / 4;
	y2 = PosY - scaleY / 2;

	if (GetCellValue(static_cast<int>(x1), static_cast<int>(y1)) == 1 ||
		GetCellValue(static_cast<int>(x2), static_cast<int>(y2)) == 1) {
		Flag |= COLLISION_BOTTOM; // Set the bottom collision bit
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

// ----------------------------------------------------------------------------
//
//	This function opens the file name "FileName" and retrieves all the map data.
//	It allocates memory for the 2 arrays: MapData & BinaryCollisionArray
//	The first line in this file is the width of the map.
//	The second line in this file is the height of the map.
//	The remaining part of the file is a series of numbers
//	Each number represents the ID (or value) of a different element in the 
//	double dimensionaly array.
//
//	Example:
//
//	Width 5
//	Height 5
//	1 1 1 1 1
//	1 1 1 3 1
//	1 4 2 0 1
//	1 0 0 0 1
//	1 1 1 1 1
//
//
//	After importing the above data, "MapData" and " BinaryCollisionArray" 
//	should be
//
//	1 1 1 1 1
//	1 1 1 3 1
//	1 4 2 0 1
//	1 0 0 0 1
//	1 1 1 1 1
//
//	and
//
//	1 1 1 1 1
//	1 1 1 0 1
//	1 0 0 0 1
//	1 0 0 0 1
//	1 1 1 1 1
//
//	respectively.
//	
//	Finally, the function returns 1 if the file named "FileName" exists, 
//	otherwise it returns 0
//
// ----------------------------------------------------------------------------
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
	BINARY_MAP_HEIGHT = std::stoi(line.substr(7).c_str());; // Read height digit

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
	for (int i = 0; i < BINARY_MAP_HEIGHT; ++i) {
		for (int j = 0; j < BINARY_MAP_WIDTH; ++j) {
			std::cout << MapData[i][j] << " ";  // Print each value followed by space
		}
		std::cout << std::endl;
	}
}
