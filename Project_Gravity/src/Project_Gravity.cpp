#include "stdafx.h"
#include "Project_Gravity.h"

#include <iostream>

using namespace std;

Project_Gravity::Project_Gravity(void) : 
	mTerrainGlobals(0),
    mTerrainGroup(0),
    mTerrainsImported(false),
	mHydrax(0)
{
}

Project_Gravity::~Project_Gravity()
{
	delete mRoot;
}
 
void Project_Gravity::createCamera(void)
{
	// Create the camera
	mCamera = mSceneMgr->createCamera("PlayerCam");
	mCamera->setNearClipDistance(5);
    mCamera->setFarClipDistance(99999*6);
	mCamera->setPosition(Ogre::Vector3(528, 155, 2960));

	// Set camera look point
    mCamera->lookAt(Vector3(0,0,-300));
	mCamera->setDirection(Ogre::Vector3(0.333, 0.071, -0.953));
    mCamera->setNearClipDistance(5);
}

bool Project_Gravity::configure(void)
{
	// Show the configuration dialog and initialise the system
 	if(mRoot->showConfigDialog())
	{
		// If returned true, user clicked OK so initialise
		// Here we choose to let the system create a default rendering window by passing 'true'
		mWindow = mRoot->initialise(true, "Project Gravity");
 		// Let's add a nice window icon
 		HWND hwnd;
 		mWindow->getCustomAttribute("WINDOW", (void*)&hwnd);
 		LONG iconID   = (LONG)LoadIcon( GetModuleHandle(0), MAKEINTRESOURCE(IDI_APPICON) );
 		SetClassLong( hwnd, GCL_HICON, iconID );
		cout << "loading" << endl;
		return true;
	}
	else
	{
		return false;
	}
}
 
void Project_Gravity::createScene(void)
{		
	Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
    Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(7);
 
	// Set ambiant lighting



    mSceneMgr->setAmbientLight(Ogre::ColourValue(1, 1, 1));
	// Create Hydrax ocean
	mHydrax = new Hydrax::Hydrax(mSceneMgr, mCamera, mWindow->getViewport(0));

	// Create our projected grid module  
	Hydrax::Module::ProjectedGrid *mModule 
		= new Hydrax::Module::ProjectedGrid(// Hydrax parent pointer
											mHydrax,
											// Noise module
											new Hydrax::Noise::Perlin(/*Generic one*/),
											// Base plane
											Ogre::Plane(Ogre::Vector3(0,1,0), Ogre::Vector3(0,0,0)),
											// Normal mode
											Hydrax::MaterialManager::NM_VERTEX,
											// Projected grid options
											Hydrax::Module::ProjectedGrid::Options(/*264 Generic one*/));

	// Set our module
	mHydrax->setModule(static_cast<Hydrax::Module::Module*>(mModule));

	// Load all parameters from config file
	mHydrax->loadCfg("GSOcean.hdx");

	// Create water
	mHydrax->create();

	// Produce the island from the config file
	mSceneMgr->setWorldGeometry("Island.cfg");

	// Adds depth so the water is darker the deeper you go
	mHydrax->getMaterialManager()->addDepthTechnique(
		static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingleton().getByName("Island"))
		->createTechnique());

	// Fixes horizon error where sea meets skydome
	std::vector<Ogre::RenderQueueGroupID> caelumskyqueue;
	caelumskyqueue.push_back(static_cast<Ogre::RenderQueueGroupID>(Ogre::RENDER_QUEUE_SKIES_EARLY + 2));
	mHydrax->getRttManager()->setDisableReflectionCustomNearCliplPlaneRenderQueues (caelumskyqueue);
	
	// Initializes the second camera window in the top right
	this->createWindows();

		// Create fish
	Ogre::Entity* fishEntity = mSceneMgr->createEntity("fish", "Icosphere.mesh");
	fishNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("fishNode");
	fishNode->attachObject(fishEntity);
	fishNode->setPosition(Ogre::Vector3(568, 155, 2880));
	fishNode->setOrientation(Ogre::Quaternion (Degree(270), Vector3::UNIT_X));
	mCamera->lookAt(fishNode->getPosition());
}
 
void Project_Gravity::createFrameListener(void)
{
	// Create the frame listener for keyboard and mouse inputs along with frame dependant processing
	mFrameListener = new PGFrameListener( mSceneMgr, 
 								mWindow, 
 								mCamera,
								Vector3(0,-9.81,0), // gravity vector for Bullet
 								AxisAlignedBox (Ogre::Vector3 (-10000, -10000, -10000), //aligned box for Bullet
  									Ogre::Vector3 (10000,  10000,  10000)),
									mHydrax);

    mRoot->addFrameListener(mFrameListener);
}

void Project_Gravity::createViewports(void)
{
    // Create one viewport, entire window
	Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
}

void Project_Gravity::createWindows(void)
{	
	// Initializes CEGUI
	mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
	CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
	CEGUI::Font::setDefaultResourceGroup("Fonts");
	CEGUI::Scheme::setDefaultResourceGroup("Schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
	CEGUI::WindowManager::setDefaultResourceGroup("Layouts");
	CEGUI::SchemeManager::getSingleton().create("WindowsLook.scheme");
	CEGUI::System::getSingleton().setDefaultMouseCursor("WindowsLook", "MouseArrow");
	CEGUI::FontManager::getSingleton().create("DejaVuSans-10.font");
	CEGUI::System::getSingleton().setDefaultFont("DejaVuSans-10");

	// Create themed window
	CEGUI::WindowManager  &wmgr = CEGUI::WindowManager::getSingleton();
	CEGUI::Window *sheet = wmgr.createWindow("DefaultWindow", "CEGUIDemo/Sheet");

	// Create quit button
	CEGUI::Window *quit = wmgr.createWindow("WindowsLook/Button", "CEGUIDemo/QuitButton");
	quit->setSize(CEGUI::UVector2(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.05, 0)));
	quit->setText("Quit");
	//quit->setFont("DejaVuSans");
	sheet->addChildWindow(quit);
	CEGUI::System::getSingleton().setGUISheet(sheet);
	quit->subscribeEvent(CEGUI::PushButton::EventClicked, 
		CEGUI::Event::Subscriber(&Project_Gravity::quit, this));

	// Create the window which uses render to texture technique
	Ogre::TexturePtr tex = mRoot->getTextureManager()->createManual(
		"RTT",
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D,
		512,
		512,
		0,
		Ogre::PF_R8G8B8,
		Ogre::TU_RENDERTARGET);
	Ogre::RenderTexture *rtex = tex->getBuffer()->getRenderTarget();

	// Create second camera and viewport to be used for this window
	Ogre::Camera *cam = mSceneMgr->createCamera("RTTCam");
	cam->setPosition(100, 400, -400);
	Ogre::Viewport *v = rtex->addViewport(cam);
	v->setOverlaysEnabled(false);
	v->setClearEveryFrame(true);
	v->setBackgroundColour(Ogre::ColourValue::Black);

	// Render to texture
	CEGUI::Texture &guiTex = mRenderer->createTexture(tex);

	CEGUI::Imageset &imageSet =	CEGUI::ImagesetManager::getSingleton().create("RTTImageset", guiTex);
	imageSet.defineImage("RTTImage",
						 CEGUI::Point(0.0f, 0.0f),
						 CEGUI::Size(guiTex.getSize().d_width,
									 guiTex.getSize().d_height),
						 CEGUI::Point(0.0f, 0.0f));

	CEGUI::Window *si = CEGUI::WindowManager::getSingleton().createWindow("WindowsLook/StaticImage", "RTTWindow");
	si->setSize(CEGUI::UVector2(CEGUI::UDim(0.25f, 0), CEGUI::UDim(0.2f, 0)));
	si->setPosition(CEGUI::UVector2(CEGUI::UDim(0.75f, 0), CEGUI::UDim(0.0f, 0)));
	si->setProperty("Image", CEGUI::PropertyHelper::imageToString(&imageSet.getImage("RTTImage")));

	sheet->addChildWindow(si);
}

void Project_Gravity::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}

void Project_Gravity::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void Project_Gravity::go(void)
{
	// Setup resource file
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup())
        return;

    mRoot->startRendering();

    // clean up
    destroyScene();
}

bool Project_Gravity::setup(void)
{
	// Setup resources
    mRoot = new Ogre::Root(mPluginsCfg);
    setupResources();

	// Configure the settings
    bool carryOn = configure();
    if (!carryOn) return false;

	// Initialize ogre elements
    chooseSceneManager();
    createCamera();
    createViewports();

	/*Ogre::TextAreaOverlayElement* loading;
	loading->initialise();
	loading->setCaption("LOADING...");
	loading->setColour(Ogre::ColourValue::White);
	loading->show();
	*/
    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();

    // Load resources
    loadResources();

    // Create the scene
    createScene();

	// Create the frame listener
    createFrameListener();
    
	return true;
}

void Project_Gravity::destroyScene(void)
{
}

void Project_Gravity::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager("TerrainSceneManager");
}

void Project_Gravity::createResourceListener(void)
{

}

bool Project_Gravity::quit(const CEGUI::EventArgs &e)
{
	// Quit the frame listener and therefore game
	mFrameListener->quit(e);
	return true;
}