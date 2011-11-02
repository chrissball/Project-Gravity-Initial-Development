#include "stdafx.h"
#include "PGFrameListener.h"
#include <iostream>

using namespace std;

PGFrameListener::PGFrameListener (
			SceneManager *sceneMgr, 
			RenderWindow* mWin, 
			Camera* cam,
			Vector3 &gravityVector,
			AxisAlignedBox &bounds,
			Hydrax::Hydrax *mHyd)
			:
			mSceneMgr(sceneMgr), mWindow(mWin), mCamera(cam), mHydrax(mHyd), mDebugOverlay(0),
			mInputManager(0), mMouse(0), mKeyboard(0), mShutDown(false), mTopSpeed(150), 
			mVelocity(Ogre::Vector3::ZERO), mGoingForward(false), mGoingBack(false), mGoingLeft(false), 
			mGoingRight(false), mGoingUp(false), mGoingDown(false), mFastMove(false), nYaw(false), 
			nGoingForward(false), nGoingBack(false), nGoingLeft(false), nGoingRight(false), nGoingUp(false), 
			nGoingDown(false), freeRoam(true), mPaused(true)
{
	// Initialize Ogre and OIS (OIS used for mouse and keyboard input)
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));



	//Jess Initialising OgreBulletInputListener
	mCollisionClosestRayResultCallback = NULL;
	mPickedBody = NULL;
	//mInputListener = new OgreBulletInputListener(, mWindow);
	myManualObject =  mSceneMgr->createManualObject("manual1"); 
	myManualObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("manual1_node"); 
 
	myManualObjectMaterial = MaterialManager::getSingleton().create("manual1Material","debugger"); 
	myManualObjectMaterial->setReceiveShadows(false); 
	myManualObjectMaterial->getTechnique(0)->setLightingEnabled(true); 
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setDiffuse(1,0,0,0); 
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setAmbient(1,0,0); 
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(1,0,0);
	myManualObjectNode->attachObject(myManualObject);

	
	// Initialize input system
    mInputManager = OIS::InputManager::createInputSystem( pl );
    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));
	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);

	// Initialize window size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

	// Start Bullet
	mNumEntitiesInstanced = 0; // how many shapes are created
	mWorld = new OgreBulletDynamics::DynamicsWorld(mSceneMgr, bounds, gravityVector);
	createBulletTerrain();

	// Create the walking robot
	createRobot();

	// For the mouse cursor on pause
	CEGUI::MouseCursor::getSingleton().setVisible(false);

	// Setup default variables for the pause menu
    mCount = 0;
    mCurrentObject = NULL;
    mLMouseDown = false;

	// Create RaySceneQuery
    mRaySceneQuery = mSceneMgr->createRayQuery(Ogre::Ray());

	// Initialize the pause variable
	mPaused = false;

	// Create the day/night system
	createCaelumSystem();
	mCaelumSystem->getSun()->setSpecularMultiplier(Ogre::ColourValue(0.3, 0.3, 0.3));
}

PGFrameListener::~PGFrameListener()
{
	// We created the query, and we are also responsible for deleting it.
    mSceneMgr->destroyQuery(mRaySceneQuery);
 	delete mWorld->getDebugDrawer();
 	mWorld->setDebugDrawer(0);
 	delete mWorld;

	if (mCaelumSystem) {
		mCaelumSystem->shutdown (false);
		mCaelumSystem = 0;
	}
	
	// OgreBullet physic delete - RigidBodies
 	std::deque<OgreBulletDynamics::RigidBody *>::iterator itBody = mBodies.begin();
 	while (mBodies.end() != itBody)
 	{   
 		delete *itBody; 
 		++itBody;
 	}	
 	// OgreBullet physic delete - Shapes
 	std::deque<OgreBulletCollisions::CollisionShape *>::iterator itShape = mShapes.begin();
 	while (mShapes.end() != itShape)
 	{   
 		delete *itShape; 
 		++itShape;
 	}
 	mBodies.clear();
 	mShapes.clear();

	Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
	windowClosed(mWindow);
}

bool PGFrameListener::frameStarted(const FrameEvent& evt)
{
	// Check camera height and make sure it doesnt go through island
	Ogre::RaySceneQuery *raySceneQuery = 
		mSceneMgr->
			    createRayQuery(Ogre::Ray(mCamera->getPosition() + Ogre::Vector3(0,1000000,0), 
				            Vector3::NEGATIVE_UNIT_Y));
	Ogre::RaySceneQueryResult& qryResult = raySceneQuery->execute();
    Ogre::RaySceneQueryResult::iterator i = qryResult.begin();
    if (i != qryResult.end() && i->worldFragment)
    {
		//Prevents flying or diving camera
		if (mCamera->getPosition().y < i->worldFragment->singleIntersection.y + 30)
		{
            mCamera->
				    setPosition(mCamera->getPosition().x, 
                                i->worldFragment->singleIntersection.y + 30, 
                                mCamera->getPosition().z);
		}
		else if (mCamera->getPosition().y > i->worldFragment->singleIntersection.y + 30)
		{
            mCamera->
				    setPosition(mCamera->getPosition().x, 
                                i->worldFragment->singleIntersection.y + 30, 
                                mCamera->getPosition().z);
			
		}
		//Allow for swimming (non-underwater)
		if (floor(mCamera->getPosition().y) < 110)
		{
            mCamera->
				    setPosition(mCamera->getPosition().x, 
                                110, 
                                mCamera->getPosition().z);
		}
    }

	delete raySceneQuery;

	// Move the sun
	Ogre::Vector3 sunPosition = mCamera->getDerivedPosition();
	sunPosition -= mCaelumSystem->getSun()->getLightDirection() * 80000;
	
	mHydrax->setSunPosition(sunPosition);
	mHydrax->setSunColor(Ogre::Vector3(mCaelumSystem->getSun()->getBodyColour().r,
		mCaelumSystem->getSun()->getBodyColour().g,
		mCaelumSystem->getSun()->getBodyColour().b));
	//CAN ALSO CHANGE THE COLOUR OF THE WATER
	
	// Update Hydrax
    mHydrax->update(evt.timeSinceLastFrame);
	
 	(void)evt;
 
 	mWorld->stepSimulation(evt.timeSinceLastFrame);	// update Bullet Physics animation
 
 	return true;
}

bool PGFrameListener::frameEnded(const FrameEvent& evt)
{
	// This was used to update the FPS of the ogre interface but that has been replaced
	// by the cegui library and so this function should be changed to output the
	// same numbers on the console
 	updateStats();
 
 	mWorld->stepSimulation(evt.timeSinceLastFrame);	// update Bullet Physics animation
 
 	return true;
}

bool PGFrameListener::keyPressed(const OIS::KeyEvent& evt)
{
	if (evt.key == OIS::KC_W || evt.key == OIS::KC_UP) mGoingForward = true; // mVariables for camera movement
	else if (evt.key == OIS::KC_S || evt.key == OIS::KC_DOWN) mGoingBack = true;
	else if (evt.key == OIS::KC_A || evt.key == OIS::KC_LEFT) mGoingLeft = true;
	else if (evt.key == OIS::KC_D || evt.key == OIS::KC_RIGHT) mGoingRight = true;
	else if (evt.key == OIS::KC_PGUP) mGoingUp = true;
	else if (evt.key == OIS::KC_PGDOWN) mGoingDown = true;
	else if (evt.key == OIS::KC_LSHIFT) mFastMove = true;
	else if (evt.key == OIS::KC_I) nGoingForward = true; // nVariables for ninja movement
	else if (evt.key == OIS::KC_K) nGoingBack = true;
	else if (evt.key == OIS::KC_J) nGoingLeft = true;
	else if (evt.key == OIS::KC_L) nGoingRight = true;
	else if (evt.key == OIS::KC_U) nGoingUp = true;
	else if (evt.key == OIS::KC_O) nGoingDown = true;
	else if (evt.key == OIS::KC_RSHIFT) nYaw = true;
    else if (evt.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
    }
    else if(evt.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (evt.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
	else if(evt.key == (OIS::KC_B)) // OgreBullet tutorial to spawn boxes
 	{
		spawnBox();
 	}
    else if (evt.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }

	// This will be used for the pause menu interface
	CEGUI::System &sys = CEGUI::System::getSingleton();
	sys.injectKeyDown(evt.key);
	sys.injectChar(evt.text);

 	return true;
}

bool PGFrameListener::keyReleased(const OIS::KeyEvent &evt)
{
	if (evt.key == OIS::KC_W || evt.key == OIS::KC_UP) mGoingForward = false; // mVariables for camera movement
	else if (evt.key == OIS::KC_S || evt.key == OIS::KC_DOWN) mGoingBack = false;
	else if (evt.key == OIS::KC_A || evt.key == OIS::KC_LEFT) mGoingLeft = false;
	else if (evt.key == OIS::KC_D || evt.key == OIS::KC_RIGHT) mGoingRight = false;
	else if (evt.key == OIS::KC_PGUP) mGoingUp = false;
	else if (evt.key == OIS::KC_PGDOWN) mGoingDown = false;
	else if (evt.key == OIS::KC_LSHIFT) mFastMove = false;
	else if (evt.key == OIS::KC_I) nGoingForward = false; // nVariables for ninja movement
	else if (evt.key == OIS::KC_K) nGoingBack = false;
	else if (evt.key == OIS::KC_J) nGoingLeft = false;
	else if (evt.key == OIS::KC_L) nGoingRight = false;
	else if (evt.key == OIS::KC_U) nGoingUp = false;
	else if (evt.key == OIS::KC_O) nGoingDown = false;
	else if (evt.key == OIS::KC_RSHIFT) nYaw = false;

	//This will be used for pause menu interface
	CEGUI::System::getSingleton().injectKeyUp(evt.key);

	return true;
}

bool PGFrameListener::mouseMoved( const OIS::MouseEvent &evt )
{
	if(mPickedBody != NULL){
		if (mPickConstraint)
		{
			CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
			Ogre::Ray rayTo = mCamera->getCameraToViewportRay (mousePos.d_x/mWindow->getWidth(), mousePos.d_y/mWindow->getHeight());

			mPickedBody->setPosition(mCamera->getDerivedPosition());
			OgreBulletDynamics::PointToPointConstraint * p2p = static_cast <OgreBulletDynamics::PointToPointConstraint *>(mPickConstraint);
			const Ogre::Vector3 eyePos(mCamera->getDerivedPosition());
			Ogre::Vector3 dir = rayTo.getDirection () * mOldPickingDist;
			const Ogre::Vector3 newPos (eyePos + dir);
			p2p->setPivotB (newPos);

			/*// dragging
			//add a point to point constraint for picking	
			CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
			Ogre::Ray rayTo = mCamera->getCameraToViewportRay (mousePos.d_x/mWindow->getWidth(), mousePos.d_y/mWindow->getHeight());
			std::cout << "Move constraint" << std::endl;
			//move the constraint pivot
			OgreBulletDynamics::PointToPointConstraint * p2p = static_cast <OgreBulletDynamics::PointToPointConstraint *>(mPickConstraint);
			//keep it at the same picking distance
			std::cout << "Get derived Pos" << std::endl;
			const Ogre::Vector3 eyePos(mCamera->getDerivedPosition());

			//Ogre::Vector3 dir = rayTo.getDirection () - eyePos;
			//dir.normalise();
			//dir *= mOldPickingDist;
			std::cout << "Dir" << std::endl;
			Ogre::Vector3 dir = rayTo.getDirection () * mOldPickingDist;
			std::cout << "Dir normalise" << std::endl;
			//dir.normalise();
			std::cout << "Sum new pos" << std::endl;
			const Ogre::Vector3 newPos (eyePos + dir);
			std::cout << "Set new pos" << std::endl;
			p2p->setPivotB (newPos);*/    
		}
	}
	if (freeRoam) // freeroam is the in game camera movement
	{
		mCamera->yaw(Ogre::Degree(-evt.state.X.rel * 0.15f));
		mCamera->pitch(Ogre::Degree(-evt.state.Y.rel * 0.15f));
		CEGUI::MouseCursor::getSingleton().setVisible(false);
	}
	else // if it is false then the pause menu is activated, the cursor is shown and the camera stops
	{
		CEGUI::System &sys = CEGUI::System::getSingleton();
		sys.injectMouseMove(evt.state.X.rel, evt.state.Y.rel);
		// Scroll wheel.
		if (evt.state.Z.rel)
			sys.injectMouseWheelChange(evt.state.Z.rel / 120.0f);
		CEGUI::MouseCursor::getSingleton().setVisible(true);
	}

	// This is to move a spawned robot to the center of the screen using raycasting
	// eg hold mouse button down when looking at island and move mouse
    if (mLMouseDown)
    {
        CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
        Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(mousePos.d_x/float(evt.state.width),mousePos.d_y/float(evt.state.height));
        mRaySceneQuery->setRay(mouseRay);
 
        Ogre::RaySceneQueryResult &result = mRaySceneQuery->execute();
        Ogre::RaySceneQueryResult::iterator itr = result.begin();
 
        if (itr != result.end() && itr->worldFragment)
            mCurrentObject->setPosition(itr->worldFragment->singleIntersection);
    }
		
	return true;
}

bool PGFrameListener::mousePressed( const OIS::MouseEvent &evt, OIS::MouseButtonID id )
{
	std::cout << ("mousepressed") << std::endl;
	// pick a body and try to drag it.
    Ogre::Vector3 pickPos;
    Ogre::Ray rayTo;
	OgreBulletDynamics::RigidBody * body = NULL;
	CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();

	rayTo = mCamera->getCameraToViewportRay (mousePos.d_x/mWindow->getWidth(), mousePos.d_y/mWindow->getHeight());

	if(mCollisionClosestRayResultCallback != NULL) {
		std::cout << "deleting!" << std::endl;
		delete mCollisionClosestRayResultCallback;
		std::cout << "deleted!" << std::endl;
	} else {
		std::cout << "no delete necessary!" << std::endl;
	}

	mCollisionClosestRayResultCallback = new OgreBulletCollisions::CollisionClosestRayResultCallback(rayTo, mWorld, mCamera->getFarClipDistance());

	std::cout << "rayTo.." << "(" << mCollisionClosestRayResultCallback->getRayEndPoint().x << ", " << mCollisionClosestRayResultCallback->getRayEndPoint().y << ", " << mCollisionClosestRayResultCallback->getRayEndPoint().z << ")" << std::endl;
    mWorld->launchRay (*mCollisionClosestRayResultCallback);

	//Draw ray
	myManualObjectNode->detachObject(myManualObject);
	myManualObject->begin("manual1Material", Ogre::RenderOperation::OT_LINE_LIST); 
	myManualObject->position(rayTo.getOrigin().x, rayTo.getOrigin().y, rayTo.getOrigin().z); 
	myManualObject->position(mCollisionClosestRayResultCallback->getRayEndPoint().x, mCollisionClosestRayResultCallback->getRayEndPoint().y, mCollisionClosestRayResultCallback->getRayEndPoint().z); 
	myManualObject->end(); 
	myManualObjectNode->attachObject(myManualObject);

	//If there was a collision, select the one nearest the player
    if (mCollisionClosestRayResultCallback->doesCollide ())
    {
		std::cout << "Collision found" << std::endl;
        body = static_cast <OgreBulletDynamics::RigidBody *> 
            (mCollisionClosestRayResultCallback->getCollidedObject());
		
		pickPos = mCollisionClosestRayResultCallback->getCollisionPoint ();
        std::cout << body->getName() << std::endl;
	} else {
		 std::cout << "No collisions found" << std::endl;
	}


	//If there was a collision..
    if (body != NULL)
    {  
		std::cout << "if (body)" << std::endl;
        std::cout << "Body is solid?" << body->isStaticObject() << std::endl;

        if (!(body->isStaticObject() 
            //|| body->isKinematicObject()
            ))
		{
            mPickedBody = body;
            mPickedBody->disableDeactivation();		
            const Ogre::Vector3 localPivot (body->getCenterOfMassPivot(pickPos));
            OgreBulletDynamics::PointToPointConstraint *p2pConstraint  = new OgreBulletDynamics::PointToPointConstraint(body, localPivot);

            mWorld->addConstraint(p2pConstraint);					    

            //save mouse position for dragging
            mOldPickingPos = pickPos;
            const Ogre::Vector3 eyePos(mCamera->getDerivedPosition());
            mOldPickingDist  = (pickPos - eyePos).length();

            //very weak constraint for picking
            p2pConstraint->setTau (0.1f);
            mPickConstraint = p2pConstraint;

        }
  
    } else {
		std::cout << "Else (body)" << std::endl;
	}

    //if (mGuiListener->getGui()->injectMouse(mInputListener->getAbsMouseX ()*mWindow->getWidth(), 
    //    mInputListener->getAbsMouseY ()*mWindow->getHeight(), true))
    //{
    //    mGuiListener->hideMouse();
    //}
    //else
    //{
    //    mGuiListener->showMouse ();
    //}

	// Left mouse button down spawns a robot
	/*if (id == OIS::MB_Left)
	{
		// Setup the ray scene query, use CEGUI's mouse position
        CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
        Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(mousePos.d_x/float(evt.state.width), 
															mousePos.d_y/float(evt.state.height));
        mRaySceneQuery->setRay(mouseRay);
		
		// Execute query
        Ogre::RaySceneQueryResult &result = mRaySceneQuery->execute();
        Ogre::RaySceneQueryResult::iterator itr = result.begin( );
 
        // Get results, create a node/entity on the position
        if (itr != result.end() && itr->worldFragment)
        {
			char name[16];
            sprintf( name, "Robot%d", mCount++ );
			Ogre::Entity *ent = mSceneMgr->createEntity(name, "robot.mesh");
            mCurrentObject = mSceneMgr->getRootSceneNode()->createChildSceneNode(std::string(name) + "Node", itr->worldFragment->singleIntersection);
            mCurrentObject->attachObject(ent);
            mCurrentObject->setScale(0.1f, 0.1f, 0.1f);
        }
 
        mLMouseDown = true;
    } 
	else if (id == OIS::MB_Right) // The right mouse button toggles freeroam or pause
	{
		freeRoam = !freeRoam;

		if (freeRoam)
			CEGUI::MouseCursor::getSingleton().setVisible(false);
		else
		{
			CEGUI::MouseCursor::getSingleton().setPosition(CEGUI::Point(400, 300));
			CEGUI::MouseCursor::getSingleton().setVisible(true);
		}
	}*/

	// This is for the pause menu interface
    CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(id));
    return true;
}

//void moveTargetedObject(OgreBulletDynamics::RigidBody* body)
//{
//	Ogre::Vector3 oldPosition(body->getCenterOfMassPosition());
//	body->setPosition(oldPosition.x+100, oldPosition.y, oldPosition.z);
//}


//OgreBulletDynamics::RigidBody* getBodyUnderCursorUsingBullet(Ogre::Vector3 &intersectionPoint, Ray &rayTo)
//{
	//OgreBulletDynamics::RigidBody
    /*rayTo = mCamera->getCameraToViewportRay (mInputListener->getAbsMouseX(), mInputListener->getAbsMouseY());

	delete mCollisionClosestRayResultCallback;
	mCollisionClosestRayResultCallback = new CollisionClosestRayResultCallback(rayTo, mWorld, mCamera->getFarClipDistance());

    mWorld->launchRay (*mCollisionClosestRayResultCallback);
    if (mCollisionClosestRayResultCallback->doesCollide ())
    {
        OgreBulletDynamics::RigidBody * body = static_cast <OgreBulletDynamics::RigidBody *> 
            (mCollisionClosestRayResultCallback->getCollidedObject());
		
		intersectionPoint = mCollisionClosestRayResultCallback->getCollisionPoint ();
        setDebugText("Hit :" + body->getName());
        return body;
    }*/
//    return 0;
//}

bool PGFrameListener::mouseReleased( const OIS::MouseEvent &evt, OIS::MouseButtonID id )
{
	//mPickedBody->disableDeactivation();	
	if(mPickedBody != NULL) {
		std::cout << "releasing object" << std::endl;
		//mPickedBody->setLinearVelocity(0, 9.81, 0);
		//mPickedBody->applyForce(Ogre::Vector3(0, -9.81*mPickedBody->, 0), mPickedBody->getCenterOfMassPosition());
		mPickedBody = NULL;
	}
	

	// Left mouse button up
    if (id == OIS::MB_Left)
    {
        mLMouseDown = false;
    }

	// This is for the pause menu interface
	CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(id));
	return true;
}

CEGUI::MouseButton PGFrameListener::convertButton(OIS::MouseButtonID buttonID)
{
	// This function converts the button id from the OIS listener to the cegui id
    switch (buttonID)
    {
    case OIS::MB_Left:
        return CEGUI::LeftButton;
 
    case OIS::MB_Right:
        return CEGUI::RightButton;
 
    case OIS::MB_Middle:
        return CEGUI::MiddleButton;
 
    default:
        return CEGUI::LeftButton;
    }
}

bool PGFrameListener::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	// Ensures the sun is not too reflective on the island
	//mCaelumSystem->getSun()->setSpecularMultiplier(Ogre::ColourValue(0.3, 0.3, 0.3));

	// Setup the scene query
    Ogre::Vector3 camPos = mCamera->getPosition();
    Ogre::Ray cameraRay(Ogre::Vector3(camPos.x, 5000.0f, camPos.z), Ogre::Vector3::NEGATIVE_UNIT_Y);
    mRaySceneQuery->setRay(cameraRay);

	// Perform the scene query
	Ogre::RaySceneQueryResult &result = mRaySceneQuery->execute();
	Ogre::RaySceneQueryResult::iterator itr = result.begin();

	// Get the results, set the camera height
    if (itr != result.end() && itr->worldFragment)
    {
		Ogre::Real terrainHeight = itr->worldFragment->singleIntersection.y;
        if ((terrainHeight + 10.0f) > camPos.y)
			mCamera->setPosition( camPos.x, terrainHeight + 10.0f, camPos.z );
    }
	
	// Move the robot
	if (mDirection == Ogre::Vector3::ZERO) 
    {
        if (nextLocation()) 
        {
            // Set walking animation
			mAnimationState = mSceneMgr->getEntity("Robot")->getAnimationState("Walk");
            mAnimationState->setLoop(true);
            mAnimationState->setEnabled(true);
        }
    }
	else
    {
        Ogre::Real move = mWalkSpeed * evt.timeSinceLastFrame;
        mDistance -= move;
	
		if (mDistance <= 0.0f)
		{
			mSceneMgr->getSceneNode("RobotNode")->setPosition(mDestination);
			mDirection = Ogre::Vector3::ZERO;

				// Set animation based on if the robot has another point to walk to. 
			if (! nextLocation())
			{
				// Set Idle animation                     
				mAnimationState = mSceneMgr->getEntity("Robot")->getAnimationState("Idle");
				mAnimationState->setLoop(true);
				mAnimationState->setEnabled(true);
			} 
			else
			{
				Ogre::Vector3 mDirection = mDestination - mNode->getPosition(); 
				Ogre::Vector3 src = mNode->getOrientation() * Ogre::Vector3::UNIT_X;

				if ((1.0f + src.dotProduct(mDirection)) < 0.0001f) 
				{
					mNode->yaw(Ogre::Degree(180));
				}
				else
				{
					src.y = 0;                                                    // Ignore pitch difference angle
					mDirection.y = 0;
					src.normalise();
					Real mDistance = mDirection.normalise( );                     // Both vectors modified so renormalize them
					Ogre::Quaternion quat = src.getRotationTo(mDirection);
					mNode->rotate(quat);
				} 
			}
        }
		else
        {
            mSceneMgr->getSceneNode("RobotNode")->translate(mDirection * move);
        } 
    } 

	// Animate the robot
	mAnimationState->addTime(evt.timeSinceLastFrame);

	// Make the secondary camera look at the robot
	mSceneMgr->getCamera("RTTCam")->lookAt(mSceneMgr->getSceneNode("RobotNode")->getPosition());

	if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

	moveCamera(evt.timeSinceLastFrame);
	moveNinja(evt.timeSinceLastFrame);

	// So that the caelum system is updated for both cameras
	mCaelumSystem->notifyCameraChanged(mSceneMgr->getCamera("PlayerCam"));
	mCaelumSystem->notifyCameraChanged(mSceneMgr->getCamera("RTTCam"));
 
    return true;
}

void PGFrameListener::updateStats(void)
{
	// CHANGE TO DISPLAY ON CONSOLE

	/*static String currFps = "Current FPS: ";
	static String avgFps = "Average FPS: ";
	static String bestFps = "Best FPS: ";
	static String worstFps = "Worst FPS: ";
	static String tris = "Triangle Count: ";
	static String batches = "Batch Count: ";

	// update stats when necessary
	try {
		OverlayElement* guiAvg = OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
		OverlayElement* guiCurr = OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
		OverlayElement* guiBest = OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
		OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");

		const RenderTarget::FrameStats& stats = mWindow->getStatistics();
		guiAvg->setCaption(avgFps + StringConverter::toString(stats.avgFPS));
		guiCurr->setCaption(currFps + StringConverter::toString(stats.lastFPS));
		guiBest->setCaption(bestFps + StringConverter::toString(stats.bestFPS)
			+" "+StringConverter::toString(stats.bestFrameTime)+" ms");
		guiWorst->setCaption(worstFps + StringConverter::toString(stats.worstFPS)
			+" "+StringConverter::toString(stats.worstFrameTime)+" ms");

		OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
		guiTris->setCaption(tris + StringConverter::toString(stats.triangleCount));

		OverlayElement* guiBatches = OverlayManager::getSingleton().getOverlayElement("Core/NumBatches");
		guiBatches->setCaption(batches + StringConverter::toString(stats.batchCount));

		OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
		guiDbg->setCaption(mDebugText);
	}
	catch(...) {  }
	*/
}

//Adjust mouse clipping area for OIS
void PGFrameListener::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void PGFrameListener::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}

void PGFrameListener::moveCamera(Ogre::Real timeSinceLastFrame)
{
	// build our acceleration vector based on keyboard input composite
	Ogre::Vector3 accel = Ogre::Vector3::ZERO;
	if (mGoingForward) accel += mCamera->getDirection();
	if (mGoingBack) accel -= mCamera->getDirection();
	if (mGoingRight) accel += mCamera->getRight();
	if (mGoingLeft) accel -= mCamera->getRight();
	if (mGoingUp) accel += mCamera->getUp();
	if (mGoingDown) accel -= mCamera->getUp();

	// if accelerating, try to reach top speed in a certain time
	Ogre::Real topSpeed = mFastMove ? mTopSpeed * 20 : mTopSpeed;
	if (accel.squaredLength() != 0)
	{
		accel.normalise();
		mVelocity += accel * topSpeed * timeSinceLastFrame * 10;
	}
	// if not accelerating, try to stop in a certain time
	else mVelocity -= mVelocity * timeSinceLastFrame * 10;

	Ogre::Real tooSmall = std::numeric_limits<Ogre::Real>::epsilon();

	// keep camera velocity below top speed and above epsilon
	if (mVelocity.squaredLength() > topSpeed * topSpeed)
	{
		mVelocity.normalise();
		mVelocity *= topSpeed;
	}
	else if (mVelocity.squaredLength() < tooSmall * tooSmall)
		mVelocity = Ogre::Vector3::ZERO;

	if (mVelocity != Ogre::Vector3::ZERO) mCamera->move(mVelocity * timeSinceLastFrame);
}

void PGFrameListener::showDebugOverlay(bool show)
{
	if (mDebugOverlay)
	{
		if (show)
			mDebugOverlay->show();
		else
			mDebugOverlay->hide();
	}
}

void PGFrameListener::moveNinja(Ogre::Real timeSinceLastFrame)
{
	if (nGoingForward) mSceneMgr->getSceneNode("NinjaNode")->translate(Ogre::Vector3(0, 0, -200) * timeSinceLastFrame, Ogre::Node::TS_LOCAL);
	if (nGoingBack) mSceneMgr->getSceneNode("NinjaNode")->translate(Ogre::Vector3(0, 0, 200) * timeSinceLastFrame, Ogre::Node::TS_LOCAL);
	if (nGoingUp) mSceneMgr->getSceneNode("NinjaNode")->translate(Ogre::Vector3(0, 200, 0) * timeSinceLastFrame, Ogre::Node::TS_LOCAL);
	if (nGoingDown) mSceneMgr->getSceneNode("NinjaNode")->translate(Ogre::Vector3(0, -200, 0) * timeSinceLastFrame, Ogre::Node::TS_LOCAL);

	if (nGoingRight)
		if (nYaw)
			mSceneMgr->getSceneNode("NinjaNode")->yaw(Ogre::Degree(-1.3 * 100) * timeSinceLastFrame);
		else
			mSceneMgr->getSceneNode("NinjaNode")->translate(Ogre::Vector3(200, 0, 0) * timeSinceLastFrame, Ogre::Node::TS_LOCAL);

	if (nGoingLeft)
		if (nYaw)
			mSceneMgr->getSceneNode("NinjaNode")->yaw(Ogre::Degree(1.3 * 100) * timeSinceLastFrame);
		else
			mSceneMgr->getSceneNode("NinjaNode")->translate(Ogre::Vector3(-200, 0, 0) * timeSinceLastFrame, Ogre::Node::TS_LOCAL);
}

bool PGFrameListener::quit(const CEGUI::EventArgs &e)
{
    mShutDown = true;
	return true;
}

bool PGFrameListener::nextLocation(void)
{
	// Get the new location for the robot to walk to

	if (mWalkList.empty())
             return false;

	mDestination = mWalkList.front();  // this gets the front of the deque
    mWalkList.pop_front();             // this removes the front of the deque
 
    mDirection = mDestination - mNode->getPosition();
    mDistance = mDirection.normalise();

	return true;
}

// Update speed factors
void PGFrameListener::UpdateSpeedFactor(double factor)
{
    mSpeedFactor = factor;
	mCaelumSystem->getUniversalClock ()->setTimeScale (mPaused ? 0 : mSpeedFactor);
}

void PGFrameListener::spawnBox(void)
{
	Vector3 size = Vector3::ZERO;	// size of the box
 	// starting position of the box
 	Vector3 position = (mCamera->getDerivedPosition() + mCamera->getDerivedDirection().normalisedCopy() * 10);
 
 	// create an ordinary, Ogre mesh with texture
 	Entity *entity = mSceneMgr->createEntity(
 			"Box" + StringConverter::toString(mNumEntitiesInstanced),
 			"cube.mesh");			    
 	entity->setCastShadows(true);
 	// we need the bounding box of the box to be able to set the size of the Bullet-box
 	AxisAlignedBox boundingB = entity->getBoundingBox();
 	size = boundingB.getSize(); size /= 2.0f; // only the half needed
 	size *= 0.95f;	// Bullet margin is a bit bigger so we need a smaller size
 							// (Bullet 2.76 Physics SDK Manual page 18)
 	entity->setMaterialName("Examples/BumpyMetal");
 	SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
 	node->attachObject(entity);
 	node->scale(0.05f, 0.05f, 0.05f);	// the cube is too big for us
 	size *= 0.05f;						// don't forget to scale down the Bullet-box too
 
 	// after that create the Bullet shape with the calculated size
 	OgreBulletCollisions::BoxCollisionShape *sceneBoxShape = new OgreBulletCollisions::BoxCollisionShape(size);
 	// and the Bullet rigid body
 	OgreBulletDynamics::RigidBody *defaultBody = new OgreBulletDynamics::RigidBody(
 			"defaultBoxRigid" + StringConverter::toString(mNumEntitiesInstanced), 
 			mWorld);
 	defaultBody->setShape(	node,
 				sceneBoxShape,
 				0.6f,			// dynamic body restitution
 				0.6f,			// dynamic body friction
 				1.0f, 			// dynamic bodymass
 				position,		// starting position of the box
 				Quaternion(0,0,0,1));// orientation of the box
 		mNumEntitiesInstanced++;				
 
 	defaultBody->setLinearVelocity(
 				mCamera->getDerivedDirection().normalisedCopy() * 7.0f ); // shooting speed
 	// push the created objects to the dequese
 	mShapes.push_back(sceneBoxShape);
 	mBodies.push_back(defaultBody);
}

void PGFrameListener::createBulletTerrain(void)
{
	// Create the bullet waterbed plane
	OgreBulletCollisions::CollisionShape *Shape;
	Shape = new OgreBulletCollisions::StaticPlaneCollisionShape(Ogre::Vector3(0,1,0), 0); // (normal vector, distance)
	OgreBulletDynamics::RigidBody *defaultPlaneBody = new OgreBulletDynamics::RigidBody("BasePlane", mWorld);
	defaultPlaneBody->setStaticShape(Shape, 0.1, 0.8, Ogre::Vector3(0, 10, 0));// (shape, restitution, friction)

	// push the created objects to the deques
	mShapes.push_back(Shape);
	mBodies.push_back(defaultPlaneBody);

	Ogre::ConfigFile config;
	config.loadFromResourceSystem("Island.cfg", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, "=", true);

	unsigned page_size = Ogre::StringConverter::parseUnsignedInt(config.getSetting( "PageSize" ));

	Ogre::Vector3 terrainScale(Ogre::StringConverter::parseReal( config.getSetting( "PageWorldX" ) ) / (page_size-1),
								Ogre::StringConverter::parseReal( config.getSetting( "MaxHeight" ) ),
								Ogre::StringConverter::parseReal( config.getSetting( "PageWorldZ" ) ) / (page_size-1));

	Ogre::String terrainfileName = config.getSetting( "Heightmap.image" );

	float *heights = new float [page_size*page_size];

	Ogre::Image terrainHeightMap;
	terrainHeightMap.load(terrainfileName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
        
	for(unsigned y = 0; y < page_size; ++y)
	{
		for(unsigned x = 0; x < page_size; ++x)
		{
			Ogre::ColourValue color = terrainHeightMap.getColourAt(x, y, 0);
			heights[x + y * page_size] = color.r;
		}
	}

	mTerrainShape = new OgreBulletCollisions::HeightmapCollisionShape (
		page_size, 
		page_size, 
		terrainScale, 
		heights, 
		true);

	OgreBulletDynamics::RigidBody *defaultTerrainBody = new OgreBulletDynamics::RigidBody(
		"Terrain", 
		mWorld);

	const float      terrainBodyRestitution  = 0.1f;
	const float      terrainBodyFriction     = 0.8f;

	Ogre::Vector3 terrainShiftPos( (terrainScale.x * (page_size - 1) / 2), \
									0,
									(terrainScale.z * (page_size - 1) / 2));

	terrainShiftPos.y = terrainScale.y / 2 * terrainScale.y;

	Ogre::SceneNode* pTerrainNode = mSceneMgr->getRootSceneNode ()->createChildSceneNode ();
	defaultTerrainBody->setStaticShape (pTerrainNode, mTerrainShape, terrainBodyRestitution, terrainBodyFriction, terrainShiftPos);

	mBodies.push_back(defaultTerrainBody);
	mShapes.push_back(mTerrainShape);
	
 	// Add Debug info display tool - creates a wire frame for the bullet objects
	debugDrawer = new OgreBulletCollisions::DebugDrawer();
	debugDrawer->setDrawWireframe(false);	// we want to see the Bullet containers
	mWorld->setDebugDrawer(debugDrawer);
	mWorld->setShowDebugShapes(false);	// enable it if you want to see the Bullet containers
	showDebugOverlay(false);
	SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode("debugDrawer", Ogre::Vector3::ZERO);
	node->attachObject(static_cast <SimpleRenderable *> (debugDrawer));
}

void PGFrameListener::createRobot(void)
{
	// Create the robot
    mEntity = mSceneMgr->createEntity("Robot", "robot.mesh");
    mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("RobotNode", Ogre::Vector3(0.0f, 300.0f, 25.0f));
    mNode->attachObject(mEntity);

	// Create the walking list for the robot to walk
    mWalkList.push_back(Ogre::Vector3(550.0f,  250.0f,  50.0f ));
    mWalkList.push_back(Ogre::Vector3(-100.0f,  600.0f, -200.0f));

	// Create the knots for the robot to walk between
    Ogre::Entity *ent;
    Ogre::SceneNode *knode;
 
    ent = mSceneMgr->createEntity("Knot1", "knot.mesh");
    knode = mSceneMgr->getRootSceneNode()->createChildSceneNode("Knot1Node",
        Ogre::Vector3(0.0f, 290.0f,  25.0f));
    knode->attachObject(ent);
    knode->setScale(0.1f, 0.1f, 0.1f);
 
    ent = mSceneMgr->createEntity("Knot2", "knot.mesh");
    knode = mSceneMgr->getRootSceneNode()->createChildSceneNode("Knot2Node",
        Ogre::Vector3(550.0f, 240.0f,  50.0f));
    knode->attachObject(ent);
    knode->setScale(0.1f, 0.1f, 0.1f);
 
    ent = mSceneMgr->createEntity("Knot3", "knot.mesh");
    knode = mSceneMgr->getRootSceneNode()->createChildSceneNode("Knot3Node",
        Ogre::Vector3(-100.0f, 590.0f,-200.0f));
    knode->attachObject(ent);
    knode->setScale(0.1f, 0.1f, 0.1f);

	// Set idle animation for the robot
	mAnimationState = mSceneMgr->getEntity("Robot")->getAnimationState("Idle");
    mAnimationState->setLoop(true);
    mAnimationState->setEnabled(true);

	// Set default values for variables
    mWalkSpeed = 35.0f;
    mDirection = Ogre::Vector3::ZERO;
}

void PGFrameListener::createCaelumSystem(void)
{
	// Initialize the caelum day/night weather system
	// Each on below corresponds to each element in the system
    Caelum::CaelumSystem::CaelumComponent componentMask;
	componentMask = static_cast<Caelum::CaelumSystem::CaelumComponent> (
		Caelum::CaelumSystem::CAELUM_COMPONENT_SUN |				
		Caelum::CaelumSystem::CAELUM_COMPONENT_MOON |
		Caelum::CaelumSystem::CAELUM_COMPONENT_SKY_DOME |
		Caelum::CaelumSystem::CAELUM_COMPONENT_IMAGE_STARFIELD |
		Caelum::CaelumSystem::CAELUM_COMPONENT_POINT_STARFIELD |
		Caelum::CaelumSystem::CAELUM_COMPONENT_CLOUDS |
		0);
	componentMask = Caelum::CaelumSystem::CAELUM_COMPONENTS_DEFAULT;
    mCaelumSystem = new Caelum::CaelumSystem(Root::getSingletonPtr(), mSceneMgr, componentMask);
	((Caelum::SpriteSun*) mCaelumSystem->getSun())->setSunTextureAngularSize(Ogre::Degree(6.0f));

    // Set time acceleration.
	mCaelumSystem->setSceneFogDensityMultiplier(0.0008f); // or some other small value.
	mCaelumSystem->setManageSceneFog(false);
	mCaelumSystem->getUniversalClock()->setTimeScale (128); // This sets the timescale for the day/night system

    // Register caelum as a listener.
    mWindow->addListener(mCaelumSystem);
	Root::getSingletonPtr()->addFrameListener(mCaelumSystem);

    UpdateSpeedFactor(mCaelumSystem->getUniversalClock ()->getTimeScale ());
}