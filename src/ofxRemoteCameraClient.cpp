/*
 *  ofxRemoteCameraClient.cpp
 *  remoteKinectClientExample
 *
 *  Created by Angelo on 08/07/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxRemoteCameraClient.h"


//------------------------------------
ofxRemoteCameraClient::ofxRemoteCameraClient(){
	//superclass constructor
}

//------------------------------------
void ofxRemoteCameraClient::initGrabber(int w,int h, int imageType_, bool useTexture_){
	newData=false;
	connected=false;
	changeRequested=false;
	frameNew=false;
	verbose=false;
	fps=0;	
	intSize=0;
	client=NULL;
	
	camWidth=requestedWidth=tmpW=w;
	camHeight=requestedHeight=tmpH=h;
	imageType=requestedImageType=tmpImageType=imageType_;
	
	compressionQuality=tmpCompressionQuality=NO_COMPRESSION;
	
	pixels=(unsigned char*)malloc(getPixelSize(imageType)*camWidth*camHeight*sizeof(char));
	auxPixels=(unsigned char*)malloc(getPixelSize(imageType)*camWidth*camHeight*sizeof(char));
	compressedData=(unsigned char*)malloc(getPixelSize(imageType)*camWidth*camHeight*sizeof(char));
	handle=tjInitDecompress();
	setUseTexture(useTexture_);
}


void ofxRemoteCameraClient::start(){
	startThread(true, false);
}

void ofxRemoteCameraClient::setRequestedSize(int w,int h){
	if(w<=camWidth && w>0 && h<=camHeight && h>0){
		tmpW=w;
		tmpH=h;
	}
	else 
		cout << "Wrong size!\n";
}

void ofxRemoteCameraClient::setRequestedImageType(int tp){
	if(tp>=OF_IMAGE_GRAYSCALE && tp<=imageType)
		tmpImageType=tp;
	else 
		cout << "Wrong image type!\n";
	//OF_IMAGE_GRAYSCALE,min(,OF_IMAGE_COLOR_ALPHA));
}

void ofxRemoteCameraClient::setRequestedCompressionQuality(int quality){
	if(quality>=1 && quality<=NO_COMPRESSION)
		tmpCompressionQuality=quality;
	else 
		cout << "Wrong compression quality value!\n";
	
}

//------------------------------------
void ofxRemoteCameraClient::initGrabber(int w,int h,string address, int port, int imageType_, bool useTexture){
	setNetworkSettings(address,port);
	initGrabber(w,h,imageType_, useTexture);
}

//------------------------------------
void ofxRemoteCameraClient::update(){
	updateFPS();
	if(!changeRequested){
		if(newData){
			lock();
			
			/*swapping buffers*/
			unsigned char *tmp=pixels;
			pixels=auxPixels;
			auxPixels=tmp;
			/******************/
			
			if(useTexture)
				texture.loadData((unsigned char*)pixels,requestedWidth,requestedHeight, getGLCode(requestedImageType));
			frameNew=true;
			newData=false;
			unlock();
		}
		changeRequested=tmpImageType!=requestedImageType || compressionQuality!=tmpCompressionQuality || tmpW!=requestedWidth || requestedHeight!= tmpH;	
	}
}


bool ofxRemoteCameraClient::decompress(unsigned char *pixels, int size, int w, int h, int pixelsize){
	return tjDecompress(handle, pixels, size, compressedData, w, 0, h, pixelsize, 0);
}

//------------------------------------
void ofxRemoteCameraClient::threadedFunction(){
	string msg;
	while (isThreadRunning()) {
		if(connected){
			/*********SENDING THE REQUEST*********/
			if(changeRequested){
				requestedImageType=tmpImageType;
				compressionQuality=tmpCompressionQuality;
				requestedWidth=tmpW;
				requestedHeight=tmpH;
				changeRequested=false;
			}
			msg.clear();
			msg=msg+ofToString(requestedWidth, 0)+SIZE_SEPARATOR+ofToString(requestedHeight, 0)+DATA_SEPARATOR;
			msg=msg+ofToString(compressionQuality,0)+DATA_SEPARATOR;
			msg=msg+ofToString(requestedImageType, 0)+DATA_SEPARATOR;
			
			while (msg.size()<MSG_SIZE) {
				msg=msg+"0";
			}
			
			if(verbose)
				cout << "SENDING REQUEST: "<<msg<<"\n";	
			
			if(sendData(client->TCPClient,(unsigned char*)msg.c_str(), MSG_SIZE)>0){
				if(verbose)cout << "SIZE:";	
				string s;
				s.resize(MSG_SIZE);
				if(receiveData(client->TCPClient,(unsigned char*)s.c_str(),MSG_SIZE)>0){
					intSize=ofToInt(s);
					if(verbose)cout<< " "<< intSize<<"\n";
					lock();
					if(verbose)cout <<"START RECEIVING DATA\n";
					if(receiveData(client->TCPClient,auxPixels,intSize)>0) {
						if(compressionQuality<NO_COMPRESSION){
							decompress(auxPixels,intSize, requestedWidth, requestedHeight,getPixelSize(requestedImageType));
							unsigned char *tmp=(unsigned char*)auxPixels;
							auxPixels=compressedData;
							compressedData=tmp;
						}
						if(verbose)
							cout << "DATA RECIEVED AND COMPRESSED\n";
						newData=connected;
						unlock();
						continue;
					}
					unlock();
				}
			}
			cout<<"Error!!!\n";
			connected=false;
		}
		else{
			setupConnection();
			if(!isConnected()){
				cout << "Connection refused. Sleeping....";
				#ifdef TARGET_WIN32 
					Sleep(SLEEPING_TIME);	
				#else 
					sleep(SLEEPING_TIME);
				#endif
				cout << "done\n";
			}
			else {
				cout << "CLIENT Connected to "<<address<<":"<<port<<"\n";
			}

		}
	}
}


//------------------------------------
ofTexture & ofxRemoteCameraClient::getTextureReference(){
	if(!texture.bAllocated())
		cout << "WARNING: Texture is not allocated\n";
	return texture;
}

//------------------------------------
void ofxRemoteCameraClient::setUseTexture(bool bUse){
	useTexture=bUse;
	if(bUse && !texture.bAllocated()){
		texture.allocate(camWidth,camHeight, GL_RGBA);
	}
	else {
		texture.clear();
	}
}

//------------------------------------
void ofxRemoteCameraClient::updateFPS(){
	static int count=0;
	if(newData)
		count++;
	static int lastMillis=ofGetElapsedTimeMillis();
	if(ofGetElapsedTimeMillis()-lastMillis>=1000){
		fps=count;
		count=0;
		lastMillis=ofGetElapsedTimeMillis();
	}
}


//------------------------------------
bool ofxRemoteCameraClient::isFrameNew(){
	if (frameNew) {
		frameNew=false;
		return connected;
	}
	return false;	
}

//------------------------------------
void ofxRemoteCameraClient::draw(float _x, float _y, float _w, float _h){
	if (useTexture && texture.bAllocated()){
		if(connected) {
			texture.draw(_x, _y,_w,_h);
		}
		else {
			ofSetColor(70, 70, 70);
			ofRect(_x,_y, _w, _h);
			ofSetColor(200, 200, 200);
			ofDrawBitmapString("Connecting...", camWidth/10,camHeight/10);
		}
	}
	
} 

//------------------------------------
void ofxRemoteCameraClient::draw(float _x, float _y){
	draw(_x, _y, camWidth, camHeight);
}

//------------------------------------
void ofxRemoteCameraClient::draw(){
	draw(0,0);
}

//------------------------------------
void ofxRemoteCameraClient::updateNetworkSettings(){
	if( XML.loadFile(NETWORK_CONFIG_FILE) ){
		cout<<"settings.xml loaded!\n";
		port=XML.getValue("client_port", DEFAULT_PORT);
		address=XML.getValue("client_address", DEFAULT_IP_ADDRESS);
		
	}else{
		cout<<"Unable to load Network_settings.xml check /data/ folder. Loading default values\n";

		XML.setValue("client_port", DEFAULT_PORT);
		XML.setValue("client_address", DEFAULT_IP_ADDRESS);
		port=DEFAULT_PORT;
		address=DEFAULT_IP_ADDRESS;
	}
	cout << "CLIENT using " << address << ":" << port << "\n";

}


//------------------------------------
void ofxRemoteCameraClient::setNetworkSettings(string address_, int port_){
	if(! XML.loadFile(NETWORK_CONFIG_FILE) )

	XML.setValue("client_port", port_);
	XML.setValue("client_address", address_);

	address=address_;
	port=port_;
}


//------------------------------------
void ofxRemoteCameraClient::setupConnection(){
	updateNetworkSettings();
	if(client!=NULL){
		client->close();
		delete client;
	}
	client= new ofxTCPClient();
	client->setVerbose(false);
	connected=client->setup(address, port,true);
}

//------------------------------------
unsigned char *ofxRemoteCameraClient::getPixels(){
	return pixels;
}		


//------------------------------------
void ofxRemoteCameraClient::close(){
	connected=false;
	if(client!=NULL){
		client->close();
		delete client;
	}
	stopThread(true);
	free(pixels);
	free(auxPixels);
	free(compressedData);
}

