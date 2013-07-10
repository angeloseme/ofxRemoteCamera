/*
 *  ofxRemoteCameraManager.h
 *  ofxRemoteCameraServerExample
 *
 *  Created by Angelo on 22/09/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "ofxNetwork.h"
#include "ofxXmlSettings.h"
#include "turbojpeg.h"


#define DEFAULT_IP_ADDRESS "127.0.0.1"
#define DEFAULT_PORT 11899
#define NETWORK_CONFIG_FILE "settings.xml"

#define DATA_SEPARATOR "-"
#define SIZE_SEPARATOR "x"

#define NO_COMPRESSION 101
#define	MSG_SIZE		20
#define SLEEPING_TIME 2


class ofxRemoteCameraManager{
public:
	
	int getGLCode(int tp){
		switch (tp) {
			case OF_IMAGE_COLOR_ALPHA:return GL_RGBA;
			case OF_IMAGE_COLOR: return GL_RGB;
			case OF_IMAGE_GRAYSCALE:return GL_LUMINANCE;
		}
		return GL_RGB;
	}
	
	
	//------------------------------------
	int	getPixelSize(int tp){
		switch (tp) {
			case OF_IMAGE_COLOR_ALPHA:return 4;
			case OF_IMAGE_COLOR:return 3;
			case OF_IMAGE_GRAYSCALE:return 1;
			default: return 3;
		}
	}
	
	//------------------------------------
	int  sendData(ofxTCPManager &manager, unsigned char* rawBytes, int numBytes, int maxMillis=1500, bool verbose=false){
		if(numBytes <= 0){ 
			cout<<"SEND ERROR: number of bytes has to be positive\n";
			return -1;
		}
		int dataSent=0;
		int lastTime=ofGetElapsedTimeMillis();
		while (dataSent<numBytes) {
			int result=manager.Send((const char *)&rawBytes[dataSent], numBytes-dataSent);
			
			if((result<0 && errno!=EAGAIN ) || result==0){
				if(verbose)
					cout << "SEND ERROR : errno = "<<errno<<"\n";
//				return -1;
			}
			else{
				if(result>0){
					dataSent+=result;
				}
			}
			if (ofGetElapsedTimeMillis()-lastTime>maxMillis) {
				if(maxMillis>0)
					cout << "SEND TIMEOUT\n";
				return 0;
			}
		}
		return numBytes;
	}
	
	//------------------------------------
	int receiveData(ofxTCPManager &manager,unsigned char* buffer,int size, int maxMillis=1500, bool verbose=false){
		static int dataReceived=0;
		int result;
		int lastTime=ofGetElapsedTimeMillis();
		while(dataReceived < size){
			
			result = manager.Receive((char*)&buffer[dataReceived], size-dataReceived);
			if((errno!=EAGAIN && result<0) || result==0){
				if(verbose)
					cout << "RECIEVE ERROR : errno = "<<errno<<"\n";
				return -1;
			}
			else{
				if(result>0){
					dataReceived+=result;
					if (dataReceived==size){
						dataReceived=0;
						return size;
					}
				}
			}
			if (ofGetElapsedTimeMillis()-lastTime>maxMillis) {
				if(maxMillis>0 && verbose)
					cout << "RECEIVE TIMEOUT\n";
				return 0;
			}
		}
		return dataReceived;
	}
};