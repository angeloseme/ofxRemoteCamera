/*
 *  ofxRemoteCameraServer.cpp
 *  remoteKinectClientExample
 *
 *  Created by Angelo on 08/07/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxRemoteCameraServer.h"


	
//------------------------------------
void	ofxRemoteCameraServer::init(int w,int h, int imageType_){
	camWidth=w;
	camHeight=h;
	imageType=imageType_;
	fps=0;
	pixSize=getPixelSize(imageType);
	verbose=false;
	handle=tjInitCompress();
	setupTCPServer();
}

//------------------------------------
long ofxRemoteCameraServer::compress(unsigned char* inBuffer, unsigned char* outBuffer, int jpegQuality, int w, int h, int type){
	int pitch=0, flags=0, jpegsubsamp=0;
	unsigned long size=0;
	if(getPixelSize(type)==1)
		jpegsubsamp=TJ_GRAYSCALE;
	tjCompress(handle,inBuffer, w, pitch, h, getPixelSize(type), outBuffer, &size, jpegsubsamp, jpegQuality, flags);
	return size;
	
}

//------------------------------------
void ofxRemoteCameraServer::setPort(int port_){
	if(! XML.loadFile(NETWORK_CONFIG_FILE) )
		XML.saveFile(NETWORK_CONFIG_FILE);
	XML.setValue("port", port_);
	XML.saveFile(NETWORK_CONFIG_FILE);
	updateNetworkSettings();
}

//------------------------------------
int	ofxRemoteCameraServer::getWidth(){
	return camWidth;
}

//------------------------------------
int	ofxRemoteCameraServer::getHeight(){
	return camHeight;
}

//------------------------------------
void ofxRemoteCameraServer::setupTCPServer(){
	updateNetworkSettings();
	tcpServer=new ofxTCPServer();
	tcpServer->setup(port,false);
}

//------------------------------------
void ofxRemoteCameraServer::updateNetworkSettings(){
	if( XML.loadFile(NETWORK_CONFIG_FILE) ){
		cout<<"Network_settings.xml loaded!\n";
		port=XML.getValue("port", DEFAULT_PORT);
	}else{
		cout<<"Unable to load Network_settings.xml check data/ folder. Loading default values\n";
		XML.saveFile(NETWORK_CONFIG_FILE);
		XML.setValue("port", DEFAULT_PORT);
		port=DEFAULT_PORT;
	}
	cout << "Using Port: "<<port<<"\n";
	XML.saveFile(NETWORK_CONFIG_FILE);
}

//------------------------------------
frame_t ofxRemoteCameraServer::parseRequest(string request,unsigned char* &inBuffer){
	IplImage* aux=NULL,*aux1=NULL; 
	int w,h,type,comp;
	int pos;
	frame_t toReturn;
	string auxString=string(request);
	if(imageMap.count(auxString)==0){
		toReturn.buffer=inBuffer;
		toReturn.size=camWidth*camHeight*pixSize;
		
		pos=auxString.find(SIZE_SEPARATOR);
		w=max(1,min(camWidth,ofToInt(auxString.substr(0,pos))));
		auxString=auxString.substr(pos+1);
		
		pos=auxString.find(DATA_SEPARATOR);
		h=max(1,min(camHeight,ofToInt(auxString.substr(0,pos))));
		auxString=auxString.substr(pos+1);
		
		pos=auxString.find(DATA_SEPARATOR);
		comp=min(max(1,ofToInt(auxString.substr(0,pos))),NO_COMPRESSION);
		auxString=auxString.substr(pos+1);
		
		pos=auxString.find(DATA_SEPARATOR);
		type=min((int)imageType,(int)max((int)OF_IMAGE_GRAYSCALE,ofToInt(auxString.substr(0,pos))));
		
		
		
		//CHANGING WIDTH, HEIGHT AND IMAGE TYPE WITH OPENCV.
		//NO OVERSCALE (ex. 320x240 -> 640x480) AND 
		//DUMMY TYPE CONVERSION (ex. GL_RGB -> GL_RGBA) ARE NOT ALLOWED.
		
		if(w<camWidth || h<camHeight || type < imageType){
			aux= cvCreateImage( cvSize(camWidth,camHeight), IPL_DEPTH_8U, pixSize);
			memcpy(aux->imageData, inBuffer, camWidth*camHeight*pixSize);
			if(w<camWidth || h<camHeight){ 
				aux1=cvCreateImage( cvSize(w,h), IPL_DEPTH_8U, pixSize);
				cvResize( aux, aux1);
				cvReleaseImage(&aux);
				aux=aux1;
				toReturn.size=w*h*pixSize;
			}
			if (type!=imageType) {
				switch (imageType){
					case OF_IMAGE_GRAYSCALE://DUMMY CASE
						type=OF_IMAGE_GRAYSCALE;
						break;
					case OF_IMAGE_COLOR:
						if(type == OF_IMAGE_GRAYSCALE){
								aux1=cvCreateImage( cvSize(w,h), IPL_DEPTH_8U,1);
								cvCvtColor( aux, aux1, CV_RGB2GRAY );
								cvReleaseImage(&aux);
								aux=aux1;
								toReturn.size=w*h;
						}else {
							
							type=OF_IMAGE_COLOR;
						}
						break;
					case OF_IMAGE_COLOR_ALPHA:
						switch (type) {
							case OF_IMAGE_COLOR:
								aux1=cvCreateImage( cvSize(w,h), IPL_DEPTH_8U,3);
								cvCvtColor( aux, aux1, CV_RGBA2RGB );
								cvReleaseImage(&aux);
								aux=aux1;
								toReturn.size=w*h*3;
								break;
							case OF_IMAGE_GRAYSCALE:
								aux1=cvCreateImage( cvSize(w,h), IPL_DEPTH_8U,1);
								cvCvtColor( aux, aux1, CV_RGBA2GRAY );
								cvReleaseImage(&aux);
								aux=aux1;
								toReturn.size=w*h;
								break;
							default:
								type=OF_IMAGE_COLOR_ALPHA;
								break;
						}
						break;
				}
			}
			toReturn.buffer=(unsigned char*)malloc(toReturn.size*sizeof(char));
			if(comp<NO_COMPRESSION){
				toReturn.size=compress((unsigned char*)aux->imageData, toReturn.buffer, comp, w, h, type);
			}else {
				memcpy(toReturn.buffer, aux->imageData, toReturn.size);
			}
			cvReleaseImage(&aux);
		}
		else {
			if(comp<NO_COMPRESSION){
				toReturn.buffer=(unsigned char*)malloc(toReturn.size*sizeof(char));
				toReturn.size=compress(inBuffer, toReturn.buffer, comp, w, h, type);
			}
			else {
				toReturn.buffer=inBuffer;
				toReturn.size=camWidth*camHeight*pixSize;
				return toReturn; 
			}

		}
		imageMap[request]=toReturn;
	}
	else{
		toReturn= imageMap[request];
	}

	return toReturn;
}

//------------------------------------
void ofxRemoteCameraServer::update(unsigned char *pixels){
	int result=0;
	unsigned char *buffer;
	int dataSize;
	vector<string> requestList;
	string which,  msgSize;
	
	while(lastRequests.size()>0){
		parseRequest(lastRequests.back(), pixels);
		lastRequests.pop_back();
	}
	
	for(int clientId = 0; clientId < tcpServer->getLastID(); clientId++){
		if(tcpServer->isClientSetup(clientId) && tcpServer->isClientConnected(clientId)){
			reqManagers.push_back(new RequestManager(pixels,this,clientId));
			reqManagers.back()->startThread(true,false);
		}
	}
	
	while (reqManagers.size()>0) {
		reqManagers.back()->waitForThread();
		delete reqManagers.back();
		reqManagers.pop_back();
	}
	
	for (map<string,frame_t>::iterator it=imageMap.begin() ; it != imageMap.end(); it++ ){
		free(it->second.buffer);
		it->second.buffer=NULL;
	}
	imageMap.clear();
	
	while(deadClients.size()>0) {
		if(verbose)
			cout << "DISCONNECTING DEAD CLIENT\n";
		tcpServer->disconnectClient(deadClients.back());
		deadClients.pop_back();
	}
	updateFPS();	
}

int ofxRemoteCameraServer::getNumClients(){
	return tcpServer->getLastID();
}

//------------------------------------
void ofxRemoteCameraServer::updateFPS(){
	static int lastMillis=ofGetElapsedTimeMillis();
	static int count=0;
	count++;
	if(ofGetElapsedTimeMillis()-lastMillis>=1000){
		fps=count;
		count=0;
		lastMillis=ofGetElapsedTimeMillis();
	}
}

//------------------------------------
void ofxRemoteCameraServer::close(){
	while (reqManagers.size()>0) {
		reqManagers.back()->stopThread();
		delete reqManagers.back();
		reqManagers.pop_back();
	}
	for(int clientId = 0; clientId < tcpServer->getLastID(); clientId++)
		tcpServer->disconnectClient(clientId);
	for (map<string,frame_t>::iterator it=imageMap.begin() ; it != imageMap.end(); it++ )
		free((*it).second.buffer);
	imageMap.clear();
	if(handle) 
		tjDestroy(handle);
		
}

