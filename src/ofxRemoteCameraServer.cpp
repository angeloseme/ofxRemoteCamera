/*
 *  ofxRemoteCameraServer.cpp
 *  remoteKinectClientExample
 *
 *  Created by Angelo on 08/07/2011.
 *  Modified by Wray Bowling on 09/02/2012.
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
	cout << "size:" << size << endl;
	return size;
}

//------------------------------------
void ofxRemoteCameraServer::setPort(int port_){
	if(! XML.loadFile(NETWORK_CONFIG_FILE) )

	XML.setValue("server_port", port_);

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
		cout<<"settings.xml loaded!\n";
		port=XML.getValue("server_port", DEFAULT_PORT);
	}else{
		cout<<"Unable to load Network_settings.xml check data/ folder. Loading default values\n";

		XML.setValue("server_port", DEFAULT_PORT);
		port=DEFAULT_PORT;
	}
	cout << "SERVER using local:" << port << "\n";

}

//------------------------------------
frame_t ofxRemoteCameraServer::parseRequest(string request,unsigned char* &inBuffer){
	int w,h,type,comp;
	int pos;
	frame_t toReturn;
	string auxString=string(request);
	if(imageMap.count(auxString)==0){
	/*	toReturn.buffer=inBuffer;
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
		
			if(comp<NO_COMPRESSION){
				toReturn.buffer=(unsigned char*)malloc(toReturn.size*sizeof(char));
				toReturn.size=compress(inBuffer, toReturn.buffer, comp, w, h, type);
			}
			else {*/
				toReturn.buffer=inBuffer;
				toReturn.size=camWidth*camHeight*pixSize;
				return toReturn; 
		//	}

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
	tcpServer->close();
	for (map<string,frame_t>::iterator it=imageMap.begin() ; it != imageMap.end(); it++ )
		free((*it).second.buffer);
	imageMap.clear();
	if(handle) 
		tjDestroy(handle);
		
}

