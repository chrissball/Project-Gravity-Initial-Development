#include "windows.h"

#include "ExampleApplication.h"
#include "../res/resource.h"
#include "OgreBulletDynamicsRigidBody.h"				 // for OgreBullet
#include "OgreBulletListener.h"							 //For ray casting...
#include "OgreBulletDynamics.h"							
//#include "OgreBulletDynamicsPreRequisites.h"			
#include "OgreBulletCollisions.h"
#include "OgreBulletCollisionsRay.h"
#include "Constraints/OgreBulletDynamicsPoint2pointConstraint.h"
#include "Shapes/OgreBulletCollisionsStaticPlaneShape.h" // for static planes
#include "Shapes/OgreBulletCollisionsBoxShape.h"		 // for Boxes
#include "Shapes/OgreBulletCollisionsTerrainShape.h"	 // for the terrain
#include "Shapes/OgreBulletCollisionsCapsuleShape.h"	// for player capsule

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreWindowEventUtilities.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>

#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include <CEGUI.h>
#include <RendererModules/Ogre/CEGUIOgreRenderer.h>

#include "Hydrax/Hydrax.h"
#include "Hydrax/Noise/Perlin/Perlin.h"
#include "Hydrax/Modules/ProjectedGrid/ProjectedGrid.h"

#include "Caelum.h"