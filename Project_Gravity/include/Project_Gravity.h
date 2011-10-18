#ifndef __GRAVITY_SHOCK_h_
#define __GRAVITY_SHOCK_h_

#include "PGFrameListener.h"
#include "OgreBulletDynamicsRigidBody.h"				 // for OgreBullet
#include "Shapes/OgreBulletCollisionsBoxShape.h"		 // for Boxes

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

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

class Gravity_Shock : public Ogre::WindowEventListener
{
public:
	Gravity_Shock();
	~Gravity_Shock();

	void go(void);
    bool setup();
	void createCamera(void);
	bool configure(void);
    void chooseSceneManager(void);
	void createScene(void);
	void createFrameListener(void);
	void destroyScene(void);
    void createViewports(void);
    void setupResources(void);
    void loadResources(void);
	void createResourceListener(void);
    bool quit(const CEGUI::EventArgs &e);
	void createWindows(void);

private:
	GSFrameListener* mFrameListener;

    Ogre::Root *mRoot;
    Ogre::Camera* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;

    // OgreBites
    OgreBites::SdkCameraMan* mCameraMan;     // basic camera controller
    bool mCursorWasVisible;                  // was cursor visible before dialog appeared

    Ogre::TerrainGlobalOptions* mTerrainGlobals;
    Ogre::TerrainGroup* mTerrainGroup;
    bool mTerrainsImported;
 
	CEGUI::OgreRenderer* mRenderer;
	Ogre::SceneNode *ninjaNode;

	// Hydrax pointer
	Hydrax::Hydrax *mHydrax;
};
 
 #endif // #ifndef __GRAVITY_SHOCK_h_