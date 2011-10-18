#include "stdafx.h"
#include "windows.h"
#include "Project_Gravity.h"

#define WIN32_LEAN_AND_MEAN

// For the finished release version
/*INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
{
 	// Create application object
 	Project_Gravity app;
 
 	try {
 		app.go();
 	} catch( Ogre::Exception& e ) {
 		MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
 	}
 
 	return 0;
}*/

// For the console debug version
int main( int argc, const char* argv[] )
{
 	// Create application object
 	Project_Gravity app;
 
 	try {
 		app.go();
 	} catch( Ogre::Exception& e ) {
 		MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
 	}
 
 	return 0;
}