#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	grabber.initGrabber(640,480);
	remoteCamera.init(grabber.getWidth(), grabber.getHeight(), OF_IMAGE_COLOR);
}

//--------------------------------------------------------------
void testApp::update(){
	grabber.update();
	if(grabber.isFrameNew())
		remoteCamera.update(grabber.getPixels());
}


//--------------------------------------------------------------
void testApp::draw(){
	ofBackground(20,20,20);
	grabber.draw(0,0);
	ofDrawBitmapString("SERVER - FPS: "+ofToString(remoteCamera.getFPS(), 1), 10, remoteCamera.getHeight()+10,false);
}

//--------------------------------------------------------------
void testApp::exit(){
	remoteCamera.close();
	grabber.close();
}
