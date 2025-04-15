/* Start Header ************************************************************************/
/*!
\file		AABB_Collision.cpp
\author 	Gabriel Low, gabriel.low, 620006124
\par    	gabriel.low@digipen.edu
\date   	7 March 2025
\brief		This file implements the static and dynamic collision functions, as well as
			the main AABB-AABB collision function implementation.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "AABB_Collision.h"
#include "main.h"

// ----------------------------------------------------------------------------
//
// Globals
//
// ----------------------------------------------------------------------------
// fixed dt value to force it
f64					g_fixedDT = 0.01667;

// stores the game loop time that you must use in all your physics calculations
f64					g_dt = g_fixedDT;

// stores to total application time until the current loop
f64					g_appTime = 0.0;


// ----------------------------------------------------------------------------
//
// Static AABB collision:
// AABB collision between two rectangles that are possibly already overlapping
// This is a rigid check, considering both rectangles are not moving
// If this function finds intersection between the two rectangles, there will be
// no need to call the dynamic collision check.
// return true if there is overlap.
//
// ----------------------------------------------------------------------------
bool CollisionIntersection_RectRect_Static(const AABB& aabb1,				//Input
	const AABB& aabb2)				//Input
{
	/*
	Implement the dynamic collision intersection over here.

	The steps "from the lecture slides" are :

	Step 1 : Check for static collision detection between rectangles(static : before moving).
		If the check returns no overlap, you continue with the dynamic collision test
		with the subsequent next steps 2 to 5 (dynamic : with velocities).
		Otherwise you return collision is true, and you stop.
	*/

	// Check if the right edge of aabb1 is to the left of the left edge of aabb2
	if (aabb1.min.x > aabb2.max.x) return false;

	// Check if the right edge of aabb2 is to the left of the left edge of aabb1
	if (aabb2.min.x > aabb1.max.x) return false;

	// Check if the top edge of aabb1 is below the bottom edge of aabb2
	if (aabb1.max.y < aabb2.min.y) return false;

	// Check if the top edge of aabb2 is below the bottom edge of aabb1
	if (aabb2.max.y < aabb1.min.y) return false;

	// If none of the above conditions are true, the two AABBs are overlapping
	return true;
}

// ----------------------------------------------------------------------------
//
// Dynamic AABB collision:
// AABB collision between two rectangles, that we already know they are not 
// overlapping.
// Both rectangles may have velocities and the function must return the first
// time of intersection if there is any.
// return true if there is intersection.
//
// ----------------------------------------------------------------------------
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,				//Input
	const AEVec2& vel1,				//Input 
	const AABB& aabb2,				//Input 
	const AEVec2& vel2,				//Input
	float& firstTimeOfCollision)	//Output: the calculated value of tFirst
{
	/*
	Implement the dynamic collision intersection over here.

	The steps "from the lecture slides" are:

	Step 2: Initialize and calculate the new "relative" velocity of Vb
			tFirst = 0  //tFirst variable is commonly used for both the x-axis and y-axis
			tLast = dt  //tLast variable is commonly used for both the x-axis and y-axis

	Step 3: Working with one dimension (x-axis).
			if(Vb < 0)
				case 1
				case 4
			else if(Vb > 0)
				case 2
				case 3
			else //(Vb == 0)
				case 5

			case 6

	Step 4: Repeat step 3 on the y-axis

	Step 5: Return true: the rectangles intersect
	*/

	// Step 2 Initialize
	float tFirst = 0.0f;
	float tLast = float(g_fixedDT);

	AEVec2 velrel{ 0,0 };
	velrel.x = vel2.x - vel1.x;
	velrel.y = vel2.y - vel1.y;

	// ------------------------------------------------------
	// X-AXIS
	// ------------------------------------------------------
	if (velrel.x < 0) { // moving to the left
		// Case 1
		if (aabb1.min.x > aabb2.max.x) return false;
		// Case 4
		if (aabb1.max.x < aabb2.min.x) { 
			tFirst = max((aabb1.max.x - aabb2.min.x) / velrel.x, tFirst);
		}
		if (aabb1.min.x < aabb2.max.x) {
			tLast = min((aabb1.min.x - aabb2.max.x) / velrel.x, tLast);
		}
	}
	else if (velrel.x > 0) { // moving to the right
		// Case 3
		if (aabb1.max.x < aabb2.min.x) { // no intersect (moving away)
			return false;
		}
		// Case 2
		if (aabb1.min.x > aabb2.max.x) { 
			tFirst = max((aabb1.min.x - aabb2.max.x) / velrel.x, tFirst);
		}
		if (aabb1.max.x > aabb2.min.x) {
			tLast = min((aabb1.max.x - aabb2.min.x) / velrel.x, tLast);
		}
	}
	else { // not moving towards or away
		// Case 5
		if (aabb1.max.x < aabb2.min.x) { // check if collided
			return false;
		}
		else if (aabb1.min.x > aabb2.max.x) { // check other side
			return false;
		}
	}


	// ------------------------------------------------------
	// Y-AXIS
	// ------------------------------------------------------
	if (velrel.y < 0) { // moving  left
		// Case 1
		if (aabb1.min.y > aabb2.max.y) return false;
		// Case 4
		if (aabb1.max.y < aabb2.min.y) {
			tFirst = max((aabb1.max.y - aabb2.min.y) / velrel.y, tFirst);
		}
		if (aabb1.min.y < aabb2.max.y) {
			tLast = min((aabb1.min.y - aabb2.max.y) / velrel.y, tLast);
		}
	}
	else if (velrel.y > 0) { // moving right
		// Case 3
		if (aabb1.max.y < aabb2.min.y) { // no intersect (moving away)
			return false;
		}
		// Case 2 (moving towards each other)
		if (aabb1.min.y > aabb2.max.y) { 
			tFirst = max((aabb1.min.y - aabb2.max.y) / velrel.y, tFirst);
		}
		if (aabb1.max.y > aabb2.min.y) { 
			tLast = min((aabb1.max.y - aabb2.min.y) / velrel.y, tLast);
		}
	}
	else { // not moving towards or away
		// Case 5
		if (aabb1.max.y < aabb2.min.y) { // check if collided
			return false;
		}
		else if (aabb1.min.y > aabb2.max.y) { // check other side
			return false;
		}
	}
	// Case 6 
	if (tFirst > tLast) return false;

	// Step 5
	firstTimeOfCollision = tFirst;

	return true; // Collision detected
}