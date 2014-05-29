#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxMacamPs3Eye.h"
#include "ofxGui.h"
#include "ofxOsc.h"

#define HOST "localhost"
#define PORT 3333

class Glow : public ofxCv::RectFollower {
protected:
	ofColor color;
	ofVec2f cur, smooth;
	float startedDying;
	ofPolyline all;
public:
	Glow()
		:startedDying(0) {
	}
	void setup(const cv::Rect& track);
	void update(const cv::Rect& track);
	void kill();
	void draw();
    void report();
    ofVec2f getCur();
};

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
    
    void keyPressed(int key);
//PS3 Eye
    ofxMacamPs3Eye ps3Eye;
    int numPS3Eye;
    bool bUsePS3Eye = true;
    int camH=480;           //on some machine, PS3 eye doesn't work with 320x240!!!
    int camW=640;
    int fieldW=320;
    int fieldH=240;
//    float camGain = 0.2;
//    float camShutter = 1.0;
    float camHue = 0.5;

    
    int numCam;
	
    ofVideoGrabber cam;
    ofxCv::RunningBackground background;
    ofImage thresholded;
    
	ofVideoPlayer movie;
	ofxCv::ContourFinder contourFinder;
	ofxCv::RectTrackerFollower<Glow> tracker;

    
    bool bPs3AutoExposure = false;
    void onAutoGainAndShutterChange(bool & value);
	void onGainChange(float & value);
	void onShutterChange(float & value);
	void onGammaChange(float & value);
	void onBrightnessChange(float & value);
	void onContrastChange(float & value);
	void onHueChange(float & value);
	void onLedChange(bool & value);
	void onFlickerChange(int & value);
	void onWhiteBalanceChange(int & value);

    void onMinRadiusChange(float & value);
    void onMaxRadiusChange(float & value);
    void onLearningTimeChange(float & value);
    
    ofParameter<bool> bSendCenters = false;
    ofParameter<bool> bSendTargetDetail = false;
    ofParameter<bool> bSendContours = false;
    ofParameter<int> skipSample = 10;
    int skiped = 0;
//    bool bSendCenters = true;
//    void onSendCenters(bool & value);
//    bool bSendTargetDetail = false;
//    void onSendTargetDetail(bool & value);
//    bool bSendContours = false;
//    void onSendContours(bool & value);

    
// GUI
	ofxPanel			gui;
	void				setupGui();
	ofParameter<float>	guiFPS;
	ofParameter<bool>	toggleGuiDraw;
	ofParameter<bool>	doFlipCamera;
    ofParameter<float>  camGain = 0.3;
    float       camGainSave=0.3;
    ofParameter<float>  camShutter = 1.0;
    float camShutterSave = 1.0;
    //Tracker
    ofParameter<float>  learnTime=900;
    float           learnTimeSave = learnTime;
    ofParameter<float>  threshould = 10;
    float           threshouldSave = 10;
    float  minRadius = 50;
    float  maxRadius = 300;

    
    
//OSC
    ofxOscSender sender;
    void oscSendCur(Glow & follower);
    void oscSendContour(int label, const ofPolyline & polyline);
};
