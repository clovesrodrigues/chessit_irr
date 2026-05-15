#ifndef INC_IRRAI_H
#define INC_IRRAI_H

#include "IAIManager.h"
#include "IAIEntity.h"
#include "IFieldOfView.h"
#include "IDebugFOVSceneNode.h"
#include "IPlayerAIEntity.h"
#include "INPC.h"
#include "ICombatNPC.h"
#include "IPathFinder.h"
#include "IWaypoint.h"
#include "IAISensor.h"

#include <irrlicht.h>

using namespace irr;

/**
The version of the SDK
*/
#define IRRAI_VERSION 0.50f

/*! \mainpage IrrAI 0.50 API documentation
 *
 * <div align="center"><img src="irrlichtirrailogo.png" ></div>
 *
 * \section intro Introduction
 *
 * Welcome to the IrrAI API documentation.
 * Here you'll find any information you'll need to develop applications with
 * the IrrAI library. If you are looking for a tutorial on how to start, you'll
 * find some inside the SDK in the examples directory.
 *
 * IrrAI is intended to be an easy-to-use AI library for the Irrlicht Graphics Engine, so
 * this documentation is an important part of it. If you have any questions or
 * suggestions, just send a email to the author of the library, Chris Mash
 * (chris.mash (at) talk21.com).
 *
 *
 * \section links Links
 *
 * <A HREF="annotated.html">Class list</A>: List of all classes with descriptions.<BR>
 * <A HREF="functions.html">Class members</A>: Good place to find forgotten features.<BR>
 *
 */

/*! \file IrrAI.h
	\brief Main header file of IrrAI, the only file needed to include.
*/

namespace IrrAI {

/**
\brief Returns an AI manager instance
\param device - The Irrlicht Device you wish to use the AI manager with
\return an IAIManager*
*/
IAIManager* createAIManager(IrrlichtDevice* device); 
/**
\brief Removes the specified AI manager and clears up its resources
\param aimgr - The AI manager to be removed
*/
void removeAIManager(IAIManager* aimgr); 

} // end of IrrAI namespace

#endif /* INC_IRRAI_H */
