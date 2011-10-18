#ifndef __Project_Gravity_h_
#define __Project_Gravity_h_

#include "stdafx.h"
#include "PGFrameListener.h"

class Project_Gravity : public Ogre::WindowEventListener
{
public:
	Project_Gravity();
	~Project_Gravity();

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
	PGFrameListener* mFrameListener;

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
 
 #endif // #ifndef __Project_Gravity_h_