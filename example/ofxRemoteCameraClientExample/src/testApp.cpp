#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	remoteCam.initGrabber(640,480);
	remoteCam.start();
}

//--------------------------------------------------------------
void testApp::update(){
	remoteCam.update();
	if(remoteCam.isFrameNew()){
		//PROCESS DATA
		//remoteCam.getPixels()
		
	}
	
}


//--------------------------------------------------------------
void testApp::draw(){
	ofBackground(20,20,20,255);
	remoteCam.draw(0,0);
	ofDrawBitmapString("CLIENT - FPS: "+ofToString(remoteCam.getFPS(), 1), 10, remoteCam.getCamHeight()+10,false);
	int y=remoteCam.getCamHeight()+40;
	ofDrawBitmapString("Press 1: 320x240 Grayscale", 10, y,false);
	y+=20;
	ofDrawBitmapString("Press 2: 320x240 Color", 10, y,false);
	y+=20;
	ofDrawBitmapString("Press 3: 640x480 Grayscale", 10, y,false);
	y+=20;
	ofDrawBitmapString("Press 4: 640x480 Color", 10, y,false);
	y+=20;
	string m="Press +/- to set the jpeg quality (0-100) "+ofToString(remoteCam.getRequestedCompressionQuality(),0);
	ofDrawBitmapString(m, 10, y,false);
	y+=20;
	m="Data Rate: "+ofToString( remoteCam.getLastDataSize(), 0)+"/"+ofToString(640*480*3,0)+" - Rate: "+ofToString(640*480*3, 5);
	ofDrawBitmapString(m, 10, y,false);
	
	

}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	switch (key) {
		case '1':
			remoteCam.setRequestedSize(320, 240);
			remoteCam.setRequestedImageType(OF_IMAGE_GRAYSCALE);
			break;
		case '2':
			remoteCam.setRequestedSize(320, 240);
			remoteCam.setRequestedImageType(OF_IMAGE_COLOR);
			break;
		case '3':
			remoteCam.setRequestedSize(640, 480);
			remoteCam.setRequestedImageType(OF_IMAGE_GRAYSCALE);
			break;
		case '4':
			remoteCam.setRequestedSize(640, 480);
			remoteCam.setRequestedImageType(OF_IMAGE_COLOR);
			break;
		case '+':
			remoteCam.setRequestedCompressionQuality(remoteCam.getRequestedCompressionQuality()+1);
			break;
		case '-':
			remoteCam.setRequestedCompressionQuality(remoteCam.getRequestedCompressionQuality()-1);
			break;
	}
	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::resized(int w, int h){

}

//--------------------------------------------------------------
void testApp::exit(){
	remoteCam.close();
}