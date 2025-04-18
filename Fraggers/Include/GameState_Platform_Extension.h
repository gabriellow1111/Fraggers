
#pragma once

#include "AEEngine.h"
#include "Math.h"
#include "AABB_Collision.h"
#include "GameStateConstants.h"
#include "main.h"


// ----------------------------------------------------------------------------
//
// object flag definition
//
// ----------------------------------------------------------------------------
enum TYPE_OBJECT
{
	TYPE_OBJECT_EMPTY,			//0
	TYPE_OBJECT_COLLISION,		//1
	TYPE_OBJECT_PLAYER1,			//2
	TYPE_OBJECT_PLAYER2,			//3
	TYPE_OBJECT_ARROW,			//4	
	TYPE_OBJECT_BULLET,			//5
};

// ----------------------------------------------------------------------------
//
// Weapon types
//
// ----------------------------------------------------------------------------
enum TYPE_WEAPON
{
	TYPE_WEAPON_BOW,				//1
	TYPE_WEAPON_CROSSBOW,			//2
	TYPE_WEAPON_GUN,				//3
	TYPE_WEAPON_ROCKET_LAUNCHER,	//4
	TYPE_WEAPON_RAILGUN,			//5
	TYPE_WEAPON_NUM					//6
};

// ----------------------------------------------------------------------------
//
// state machine states
//
// ----------------------------------------------------------------------------
enum STATE
{
	STATE_NONE,
	STATE_GOING_LEFT,
	STATE_GOING_RIGHT
};

// ----------------------------------------------------------------------------
//
// state machine inner states
//
// ----------------------------------------------------------------------------
enum INNER_STATE
{
	INNER_STATE_ON_ENTER,
	INNER_STATE_ON_UPDATE,
	INNER_STATE_ON_EXIT
};

// ----------------------------------------------------------------------------
//
// Game object structure
//
// ----------------------------------------------------------------------------
struct GameObj
{
	unsigned long		type;		// object type
	AEGfxVertexList *	pMesh;		// This will hold the triangles which will form the shape of the object
};

// ----------------------------------------------------------------------------
//
// Game object instance structure
//
// ----------------------------------------------------------------------------
struct GameObjInst
{
	GameObj * pObject;						// pointer to the 'original'
	unsigned int	flag;					// bit flag or-ed together
	AEVec2			scale;					// scaling value of the object instance
	AEVec2			posCurr;				// object current position
	AEVec2			velCurr;				// object current velocity
	float			dirCurr;				// object current direction

	AEVec2			posPrev;				// object previous position -> it's the position calculated in the previous loop

	AEMtx33			transform;				// object transformation matrix: Each frame, 
											// calculate the object instance's transformation matrix and save it here.

	AABB			boundingBox;			// object bouding box that encapsulates the object
	int				gridCollisionFlag;		// used to hold the current object's collision flag status

	int				pOwner;					// pointer to the owner of this object instance
	void*			pUserData;				// pointer to custom data specific for each object type

	enum			STATE state;			// state of the object instance
	enum			INNER_STATE innerState;	// inner State of the object instance		
	double			idle_counter;			// general purpose counter (this variable will be used in the enemy state machine for the idle time counting)

	//For Particle System
	float			lifetime;				// particle's lifetime value if this instance is a particle

	// For player jump management
	int jumpCount;                            // current number of jumps made
	int maxJumps;                             // maximum allowed jumps (e.g., 2 for double jump)
	bool isJumping;                         // flag to check if the player is currently jumping
};

// ----------------------------------------------------------------------------
//
// Externs
//
// ----------------------------------------------------------------------------
extern int				Player1_Lives;
extern int				Player2_Lives;
extern int				Player1_Health;
extern int				Player2_Health;
extern GameObj *		sGameObjList;
extern unsigned int		sGameObjNum;
extern GameObjInst *	sGameObjInstList;
extern unsigned int		sGameObjInstNum;
extern int **			MapData;
extern int **			BinaryCollisionArray;
extern int				BINARY_MAP_WIDTH;
extern int				BINARY_MAP_HEIGHT;
extern GameObjInst *	pBlackInstance;
extern GameObjInst *	pWhiteInstance;
extern AEMtx33			MapTransform;
extern GameObjInst*		pPlayer1;
extern GameObjInst*		pPlayer2;
extern GameObjInst*			pArrow1;
extern GameObjInst*			pArrow2;
//extern Weapon*			pWeapon1;
//extern Weapon*			pWeapon2;
extern bool					onValueChange;
extern bool					stopPlaying;

// ----------------------------------------------------------------------------
// New functions
// ----------------------------------------------------------------------------
void Init_GameObjects(void);

void Import_MapData(void);
void Compute_MapTransformMatrix(void);
void Starting_GameObjectsInstances(void);
void Update_Input_Physics(void);
void Apply_GravityPhysics(void);
void Update_BoundingBoxes(void);
void Update_Positions(void);
void Check_GridBinaryCollision(void);
void Update_AABBCollisions(void);
void Update_ObjectsTransformations(void);
void Update_CameraPosition_Level2(void);
void Draw_TileMap_BackgroundGrid(void);
void Draw_DynamicObjectsInstances(void);
void Display_GameStats(void);
void Destroy_ObjectsInstances(void);
void Free_AllAllocatedMemory(void);


// ----------------------------------------------------------------------------
//
// Helper functions
//
// ----------------------------------------------------------------------------
GameObjInst* gameObjInstCreate( unsigned long type,
								AEVec2* scale,
								AEVec2* pPos,
								AEVec2* pVel,
								float dir);
void gameObjInstDestroy(GameObjInst* pInst);


// ----------------------------------------------------------------------------
//
// Assignment 3 Part 1 - APIs
//
// ----------------------------------------------------------------------------
int		GetCellValue(int X, int Y);
int		CheckInstanceBinaryMapCollision(float PosX, float PosY,
										float scaleX, float scaleY);
void	SnapToCell(float* Coordinate);
void	SnapBottomCollision(float* posY, float playerHeight);
void	SnapTopCollision(float* posY, float playerHeight);
void	SnapLeftCollision(float* posX, float playerWidth);
void	SnapRightCollision(float* posX, float playerWidth);
void	SpawnPlayers(void);
int		GenerateRandomMap(void);
void	FreeMapData(void);
void	PrintRetrievedInformation(void);
void	Update_Collisions(void);
void	RemoveBullets(void);
