#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxRemoteCameraServer.h"



class testApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void exit();
		ofxRemoteCameraServer remoteCamera;
		ofVideoGrabber grabber;
	
};

#endif
