/*
 *  ofxRemoteCameraServer.h
 *  remoteKinectClientExample
 *
 *  Created by Angelo on 08/07/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#pragma once

#include "ofxRemoteCameraManager.h"



struct frame_t{	
	int size;   
    unsigned char *buffer;
};  

class ofxRemoteCameraServer: public ofxRemoteCameraManager{
	
public:
	void			init(int w,int h, int imageType=OF_IMAGE_COLOR);
	void			update(unsigned char *pixels);
	void			setPort(int port);
	int				getWidth();
	int				getHeight();
	int				getFPS(){return fps;}
	void			close();
	void			setVerbose(bool v){verbose=v;}
	frame_t			parseRequest(string request,unsigned char* &inBuffer);
	int				getNumClients();
	vector<int>		deadClients;
	ofxTCPServer	*tcpServer;
	vector<string>	lastRequests;
	bool			verbose;
	
private:
	void			updateNetworkSettings();
	void			updateFPS();
	void			setupTCPServer();
	long			compress(unsigned char* inBuffer, unsigned char* outBuffer, int jpegQuality, int w, int h, int type);
	
	vector<ofThread*> reqManagers;
	map<string,frame_t> imageMap;
	int				fps;
	int				camWidth,camHeight;
	
	ofxXmlSettings	XML;
	int				port;
	int				imageType;
	tjhandle		handle;
	int				pixSize;
	
};

class RequestManager: public ofThread{
public:
	
	//--------------------------
	RequestManager(unsigned char *pixels_, ofxRemoteCameraServer *serv_, int clientId_){
		camServer=serv_;
		clientId=clientId_;
		pixels=pixels_;
	}
	int clientId;
private:
	ofxRemoteCameraServer *camServer;
	unsigned char* pixels;
	
	//--------------------------
	void threadedFunction(){
		unsigned char *buffer;
		int dataSize;
		string which,  msgSize;
		int result;
		char req[MSG_SIZE];
		
		if(camServer->tcpServer->isClientConnected(clientId)){
			result=camServer->receiveData(camServer->tcpServer->TCPConnections[clientId].TCPClient,(unsigned char*)req, MSG_SIZE,0);
			if(result>0){
				camServer->lastRequests.push_back(req);
				if(camServer->verbose)cout << "REQUEST: "<<req<<"\n";
				frame_t frame=camServer->parseRequest(req, pixels);
				buffer=frame.buffer;
				dataSize=frame.size;
				msgSize=ofToString(dataSize, 0);
				while (msgSize.size()<MSG_SIZE) 
					msgSize="0"+msgSize;
				
				if(camServer->verbose)cout << "SIZE: "<<dataSize<<" msg:"<<msgSize <<"\n";
				
				result=camServer->sendData(camServer->tcpServer->TCPConnections[clientId].TCPClient,(unsigned char *)msgSize.c_str(), MSG_SIZE);
				if(result>0){
					if(camServer->verbose)cout << "START SENDING DATA \n";
					result=camServer->sendData(camServer->tcpServer->TCPConnections[clientId].TCPClient,(unsigned char*)buffer, dataSize);
					if(result>0){
						if(camServer->verbose)cout << "DATA SENT\n";
						return;
					}
				}
			}
		}
		camServer->deadClients.push_back(clientId);
		
	}
	
	
	
};