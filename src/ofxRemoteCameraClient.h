/*
 *  ofxRemoteCameraClient.h
 *  remoteKinectClientExample
 *
 *  Created by Angelo on 08/07/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "ofxRemoteCameraManager.h"

class ofxRemoteCameraClient: public ofThread, public ofxRemoteCameraManager{
	
public:
	
	ofxRemoteCameraClient();
	void			initGrabber(int cameraW, int cameraH, int cameraImgType=OF_IMAGE_COLOR,bool useTexture=true);//OF_IMAGE_GRAYSCALE, OF_IMAGE_COLOR, OF_IMAGE_COLOR_ALPHA
	void			initGrabber(int cameraW, int cameraH, string address, int port, int cameraImgType=OF_IMAGE_COLOR, bool useTexture=true);//OF_IMAGE_GRAYSCALE, OF_IMAGE_COLOR, OF_IMAGE_COLOR_ALPHA
	void			setNetworkSettings(string address_, int port_);
	
	void			start();
	void			update();
	void			close();
	
	bool			isFrameNew();
	void			updateFPS();
	bool			decompress(unsigned char *pixels, int size, int w, int h, int pixelsize);
	
	void			draw(float x, float y, float w, float h);
	void 			draw(float x, float y);
	void 			draw();
	
	
	void			setRequestedCompressionQuality(int quality);
	void			setRequestedImageType(int tp);
	void			setRequestedSize(int w,int h);
	
	int				getRequestedWidth(){return requestedWidth;}
	int				getRequestedHeight(){return requestedHeight;}
	int				getRequestedCompressionQuality(){return compressionQuality;}
	int				getRequestedImageType(){return imageType;}
	
	
	
	bool			isConnected(){return connected;}
	int				getLastDataSize(){return intSize;}
	
	string			getAddress(){return address;}
	int				getPort(){return port;}
	
	int				getFPS(){return fps;}
	unsigned char*	getPixels();
	ofTexture &		getTextureReference();
	
	int				getCamWidth(){return camWidth;}
	int				getCamHeight(){return camHeight;}
	
	void 			setUseTexture(bool bUse);
	void			setVerbose(bool v){verbose=v;}
	void			setupConnection();
	
	
	
private:
	
	void			updateNetworkSettings();
	void			threadedFunction();
	
	
	int				camWidth,requestedWidth,tmpW;
	int				camHeight,requestedHeight,tmpH;
	int				imageType, requestedImageType,tmpImageType;
	int				compressionQuality,tmpCompressionQuality;
	
	
	bool			connected;
	
	ofxTCPClient	*client;
	
	string			address;
	
	int				connectTime;
	int				port;
	ofxXmlSettings	XML;
	unsigned char	*auxPixels;
	unsigned char	*pixels;
	unsigned char	*compressedData;
	bool			newData;
	ofTexture		texture;
	bool			useTexture;
	bool			frameNew;
	int				fps;
	bool			changeRequested;
	tjhandle		handle;
	int				intSize;
	bool			verbose;
	
};