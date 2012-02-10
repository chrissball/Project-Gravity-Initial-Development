#ifndef __PGFRAMELISTENER_h_
#define __PGFRAMELISTENER_h_

#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN

class PGFrameListener : 
	public Ogre::FrameListener, 
	public Ogre::WindowEventListener, 
	public OIS::KeyListener,
	public OIS::MouseListener,
	public Ogre::RenderTargetListener
{
private:
	SceneManager* mSceneMgr; 
	OgreBulletDynamics::DynamicsWorld *mWorld;	// OgreBullet World
	OgreBulletCollisions::DebugDrawer *debugDrawer;
	int mNumEntitiesInstanced;
	
    //OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*    mMouse;
    OIS::Keyboard* mKeyboard;

	RenderWindow* mWindow;
	Camera* mCamera;
	Ogre::TerrainGroup* mTerrainGroup;
    bool mTerrainsImported;
	
	Real mMoveSpeed;
	Overlay* mDebugOverlay;
	float mMoveScale;
	float mSpeedLimit;
	Degree mRotScale;
	// just to stop toggles flipping too fast
	Radian mRotX, mRotY;
	TextureFilterOptions mFiltering;
	int mAniso;
	Vector3 mTranslateVector;
	Real mCurrentSpeed;
	bool mStatsOn;
	String mDebugText;
	unsigned int mNumScreenShots;
	int mSceneDetailIndex ;
    bool mShutDown;
	
	//Camera controls
	Ogre::Real mTopSpeed;
	Ogre::Vector3 mVelocity;
	bool mGoingForward;
	bool mGoingBack;
	bool mGoingLeft;
	bool mGoingRight;
	bool mGoingUp;
	bool mGoingDown;
	bool mFastMove;
	
	//fish controls
	bool nGoingForward;
	bool nGoingBack;
	bool nGoingLeft;
	bool nGoingRight;
	bool nGoingUp;
	bool nGoingDown;
	bool nYaw;

	Ogre::Vector3 transVector;
	
    Ogre::Entity *mEntity;                 // The Entity we are animating
    Ogre::SceneNode *mNode;                // The SceneNode that the Entity is attached to
    std::deque<Ogre::Vector3> mWalkList;   // The list of points we are walking to

    Ogre::AnimationState *mAnimationState; // The current animation state of the object
	Ogre::AnimationState* anim;
	Ogre::AnimationState* gunAnimate;
	bool gunActive;
	bool shotGun;
    Ogre::Real mDistance;                  // The distance the object has left to travel
    Ogre::Vector3 mDirection;              // The direction the object is moving
    Ogre::Vector3 mDestination;            // The destination the object is moving towards
    Ogre::Real mWalkSpeed;                 // The speed at which the object is moving

	bool freeRoam;

    Ogre::RaySceneQuery *mRaySceneQuery;// The ray scene query pointer
    bool mRMouseDown;		// True if the mouse buttons are down
    int mCount;							// The number of robots on the screen
    Ogre::SceneNode *mCurrentObject;	// The newly created object
    CEGUI::Renderer *mGUIRenderer;		// CEGUI renderer
	Hydrax::Hydrax *mHydrax;
	bool mPaused;
	Caelum::CaelumSystem *mCaelumSystem;
    float mSpeedFactor;

	
	// Bullet objects
	std::deque<OgreBulletDynamics::RigidBody *>         mBodies;
	std::deque<OgreBulletCollisions::CollisionShape *>  mShapes;
	OgreBulletCollisions::HeightmapCollisionShape *mTerrainShape;

	//JESS
	OgreBulletDynamics::RigidBody *mPickedBody;
	Ogre::Vector3 mOldPickingPos;
    Ogre::Vector3 mOldPickingDist;
	OgreBulletDynamics::TypedConstraint *mPickConstraint;
	OgreBulletCollisions::CollisionClosestRayResultCallback *mCollisionClosestRayResultCallback;
	ManualObject* myManualObject;
	SceneNode* myManualObjectNode;
	MaterialPtr myManualObjectMaterial;

	//Player collision box
	OgreBulletCollisions::CapsuleCollisionShape *playerBoxShape;
	OgreBulletDynamics::RigidBody *playerBody;
	//Player velocity
	btScalar linVelX;
	btScalar linVelY;
	btScalar linVelZ;
	
	// Cubemap gravity gun
	Ogre::SceneNode* gravityGun;
	Ogre::SceneNode* pivotNode;
	Ogre::SceneNode* pivotNodePitch;
	Ogre::SceneNode* pivotNodeRoll;
	Camera* mCubeCamera;
	RenderTarget* mTargets[6];
	Radian fovy;
	int camAsp;
	Ogre::Vector3 gunPosBuffer;
	Ogre::Vector3 gunPosBuffer2;
	Ogre::Vector3 gunPosBuffer3;
	Ogre::Vector3 gunPosBuffer4;
	Ogre::Vector3 gunPosBuffer5;
	Ogre::Vector3 gunPosBuffer6;
	Ogre::Quaternion gunOrBuffer;
	Ogre::Quaternion gunOrBuffer2;
	Ogre::Quaternion gunOrBuffer3;
	Ogre::Quaternion gunOrBuffer4;
	Ogre::Quaternion gunOrBuffer5;
	Ogre::Quaternion gunOrBuffer6;
	Ogre::Quaternion rotate1;
	Ogre::Quaternion rotate2;
	int gunCount;
	Ogre::Radian differenceYaw;
	Ogre::Radian differencePit;
	Ogre::Radian differenceRol;
	Ogre::Radian differenceYawSpeed;
	Ogre::Radian differencePitSpeed;
	Ogre::Radian differenceRolSpeed;

	boost::posix_time::ptime t1;
	boost::posix_time::ptime t2;

	int mFramesPerSecond; // say 60 
	Ogre::Real mRenderTimeSlice; // 1000 / mFramesPerSecond 
	SceneNode* ocean;
	SceneNode* oceanFade;
	TexturePtr mTexture;
	Ogre::Entity* mOceanSurfaceEnt;
	Ogre::Entity* mOceanFadeEnt;


public:
    PGFrameListener(
  		SceneManager *sceneMgr, 
 		RenderWindow* mWin, 
 		Camera* cam,
 		Vector3 &gravityVector,
 		AxisAlignedBox &bounds,
		Hydrax::Hydrax *mHyd);
	~PGFrameListener();

	bool frameStarted(const FrameEvent& evt);
	bool frameEnded(const FrameEvent& evt);

    // OIS::KeyListener
    bool keyPressed( const OIS::KeyEvent& evt );
    bool keyReleased( const OIS::KeyEvent& evt );
    // OIS::MouseListener
    bool mouseMoved( const OIS::MouseEvent& evt );
    bool mousePressed( const OIS::MouseEvent& evt, OIS::MouseButtonID id );
    bool mouseReleased( const OIS::MouseEvent& evt, OIS::MouseButtonID id );

	bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	void updateStats(void);
	void windowResized(Ogre::RenderWindow* rw);
	void windowClosed(Ogre::RenderWindow* rw);
	void moveCamera(Ogre::Real timeSinceLastFrame);
	void showDebugOverlay(bool show);
	void movefish(Ogre::Real timeSinceLastFrame);
	CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID);
    bool quit(const CEGUI::EventArgs &e);
    bool nextLocation(void);
	void UpdateSpeedFactor(double factor);
	void spawnBox(void);
	void createBulletTerrain(void);
	void createRobot(void);
	void createCaelumSystem(void);
	void createCubeMap();
	void postRenderTargetUpdate(const RenderTargetEvent& evt);
	void preRenderTargetUpdate(const RenderTargetEvent& evt);
	void gunController(void);
	//void loadMaterialControlsFile(MaterialControlsContainer& controlsContainer, const Ogre::String& filename)
};

#endif