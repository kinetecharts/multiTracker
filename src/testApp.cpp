#include "testApp.h"

using namespace ofxCv;
using namespace cv;

const float dyingTime = 1;

void Glow::setup(const cv::Rect& track) {
	color.setHsb(ofRandom(0, 255), 255, 255);
	cur = toOf(track).getCenter();
	smooth = cur;
}

void Glow::update(const cv::Rect& track) {
	cur = toOf(track).getCenter();
	smooth.interpolate(cur, .5);
	all.addVertex(smooth);
}

void Glow::kill() {
	float curTime = ofGetElapsedTimef();
	if(startedDying == 0) {
		startedDying = curTime;
	} else if(curTime - startedDying > dyingTime) {
		dead = true;
	}
}

void Glow::draw() {
	ofPushStyle();
	float size = 16;
	ofSetColor(255);
	if(startedDying) {
		ofSetColor(ofColor::red);
		size = ofMap(ofGetElapsedTimef() - startedDying, 0, dyingTime, size, 0, true);
	}
	ofNoFill();
	ofCircle(cur, size);
	ofSetColor(color);
	all.draw();
	ofSetColor(255);
	ofDrawBitmapString(ofToString(label), cur);
	ofPopStyle();

}
void Glow::report(){
    if(startedDying>0)cout<<"Dead"<<endl;
    cout<<label<<": "<< cur[0]<<", "<<cur[1]<<endl;
}

ofVec2f Glow::getCur(){
    return cur;
}
void testApp::setup() {
	ofSetVerticalSync(true);
    ofSetFrameRate(30);
	ofBackground(0);
    
    numPS3Eye = ps3Eye.listDevices().size();
    if(numPS3Eye>0){
        bUsePS3Eye = true;
    }else{
        bUsePS3Eye = false;
    }
	
    if(bUsePS3Eye){
//        ps3Eye.setDeviceID(numPS3Eye-1);
        ps3Eye.setDesiredFrameRate(15);
        ps3Eye.initGrabber(camW, camH);
        
        ps3Eye.setAutoGainAndShutter(false);
        ps3Eye.setGain(camGain);   //longer shutter results in more stable images. less affected by flicker on both utility and monitor. Should set it to 1 as much as possible
        ps3Eye.setShutter(camShutter);
        ps3Eye.setGamma(0.4);
        ps3Eye.setBrightness(0.6);
        ps3Eye.setHue(camHue);
        ps3Eye.setFlicker(2);/* 0 - no flicker, 1 - 50hz, 2 - 60hz */
        ps3Eye.setWhiteBalance(2); /* 1 - linear, 2 - indoor, 3 - outdoor, 4 - auto */
    }else{
        numCam=cam.listDevices().size();
        int buildinCam=0;
        for(int i=0; i<numCam; i++){
            cout<<i<<" "<<cam.listDevices()[i].deviceName<<" "<<cam.listDevices()[i].hardwareName;
            cout<<" compare to CamTwist is: "<<cam.listDevices()[i].deviceName.compare("CamTwist")<<endl;
            
            if(cam.listDevices()[i].deviceName.compare("CamTwist") > 0){ //string not equal, so it's not a camtwist device
                buildinCam = i;
            }
        }

        cam.setDeviceID(buildinCam);
        cam.initGrabber(camW, camH);
    }
    
    background.setLearningTime(learnTime); //default to 900
    background.setThresholdValue(10);
    
//	movie.loadMovie("video.mov");
//	movie.play();
	
	contourFinder.setMinAreaRadius(minRadius);
	contourFinder.setMaxAreaRadius(maxRadius);
	contourFinder.setThreshold(15);
	
	// wait for half a frame before forgetting something
	tracker.setPersistence(15);
	// an object can move up to 50 pixels per frame
	tracker.setMaximumDistance(50);
    
    setupGui();
    cout<<oscHost<<endl;
    
    sender.setup(oscHost, PORT);

}
void testApp::update() {
    bool bFrameNew = false;
    if(bUsePS3Eye){
        ps3Eye.update();
        bFrameNew = ps3Eye.isFrameNew();
    }
    else{
        cam.update();
        bFrameNew = cam.isFrameNew();
    }
    
    if(bFrameNew){
        if(bUsePS3Eye)
            background.update(ps3Eye, thresholded);
        else
            background.update(cam, thresholded);
        thresholded.update();
		blur(thresholded, 10);
        thresholded.update();
		contourFinder.findContours(thresholded);
		tracker.track(contourFinder.getBoundingRects());
    }
    
    
//    if(camGain!=camGainSave){
//        ps3Eye.setGain(camGain);
//        camGainSave = camGain;
//        cout<<"gain changed to "<<camGain<<endl;
//    }
//    if(camShutter!=camShutterSave){
//        ps3Eye.setShutter(camShutter);
//        camShutterSave = camShutter;
//        cout<<"shutter changed to "<<camShutter<<endl;
//    }
//    if(learnTime!=learnTimeSave){
//        background.setLearningTime(learnTime);
//        learnTimeSave = learnTime;
//        cout<<"Learn time is "<<learnTime<<endl;
//    }

    //	movie.update();
//	if(movie.isFrameNew()) {
//		blur(movie, 10);
//		contourFinder.findContours(movie);
//		tracker.track(contourFinder.getBoundingRects());
//	}
}

void testApp::draw() {	
	ofSetColor(255);
    if(bUsePS3Eye)
        ps3Eye.draw(camW, 0);
    else
        cam.draw(camW,0);
    
    thresholded.draw(0,0);
	contourFinder.draw();

    vector<Glow>& followers = tracker.getFollowers();
    if(bSendCenters && (skiped ==0) ){
        for(int i = 0; i < followers.size(); i++) {
            followers[i].draw();
            oscSendCur(followers[i]);
        }
    }
    
    if(bSendTargetDetail && (skiped ==0)){
        const vector<ofPolyline>& contours = contourFinder.getPolylines();
        for(int i=0; i< contours.size(); i++){
            int label = contourFinder.getLabel(i);
            oscSendContour(label, contours[i]);
        }
    }
    
    skiped++;
    if(skiped > skipSample) skiped = 0;
    
    stringstream reportStream;
    if(bUsePS3Eye){
        reportStream<<"Using ps3 eye"<<endl;
    }else{
        reportStream << "Using building cam, numCam: "<<numCam<<endl;
        for(int i=0; i<numCam; i++){
            reportStream<<"   "<<i<<" "<<cam.listDevices()[i].deviceName<<endl;
        }
    }

    reportStream<<"num target: "<<followers.size()<<endl;
    
    ofDrawBitmapString(reportStream.str(), camW, camH+20);
	if (toggleGuiDraw) {
		guiFPS = ofGetFrameRate();
		gui.draw();
	}


}

void testApp::setupGui() {
	gui.setup("PS3Eye", "ps3eye.xml");
	gui.setPosition(660,20);
	gui.add(guiFPS.set("FPS", 0, 0, 60));
    gui.add(toggleGuiDraw.set("show gui (G)", false));

    ofxToggle * autoGainAndShutter = new ofxToggle();
    autoGainAndShutter->setup("Auto Gain and Shutter", false);
    autoGainAndShutter->addListener(this, &testApp::onAutoGainAndShutterChange);
    gui.add(autoGainAndShutter);

    gui.add(bSendCenters.set("Send Centers", false));
    gui.add(bSendTargetDetail.set("Send Target Detail", false));
    gui.add(bSendContours.set("Send Contours", false));
    gui.add(skipSample.set("Skip Sample", 10, 0, 60));
    gui.add(oscHost.set("OSC Host", "127.0.0.1"));
    
//    ofxToggle * sendCenters = new ofxToggle();
//    sendCenters->setup("Send Centers", true);
//    sendCenters->addListener(this, &testApp::onSendCenters);
//    gui.add(sendCenters);
    
    
    
    
    ofxFloatSlider * gain = new ofxFloatSlider();
    gain->setup("Gain", 0.5, 0.0, 1.0);
    gain->addListener(this, &testApp::onGainChange);
    gui.add(gain);
    
    ofxFloatSlider * shutter = new ofxFloatSlider();
    shutter->setup("Shutter", 0.5, 0.0, 1.0);
    shutter->addListener(this, &testApp::onShutterChange);
    gui.add(shutter);
    
    ofxFloatSlider * gamma = new ofxFloatSlider();
    gamma->setup("Gamma", 0.5, 0.0, 1.0);
    gamma->addListener(this, &testApp::onGammaChange);
    gui.add(gamma);
    
    ofxFloatSlider * brightness = new ofxFloatSlider();
    brightness->setup("Brightness", 0.5, 0.0, 1.0);
    brightness->addListener(this, &testApp::onBrightnessChange);
    gui.add(brightness);
    
    ofxFloatSlider * contrast = new ofxFloatSlider();
    contrast->setup("Contrast", 0.5, 0.0, 1.0);
    contrast->addListener(this, &testApp::onContrastChange);
    gui.add(contrast);
    
    ofxFloatSlider * hue = new ofxFloatSlider();
    hue->setup("Hue", 0.5, 0.0, 1.0);
    hue->addListener(this, &testApp::onHueChange);
    gui.add(hue);
    
    ofxIntSlider * flicker = new ofxIntSlider();
    flicker->setup("Flicker Type", 0, 0, 2);
    flicker->addListener(this, &testApp::onFlickerChange);
    gui.add(flicker);
    
    ofxIntSlider * wb = new ofxIntSlider();
    wb->setup("White Balance Mode", 4, 1, 4);
    wb->addListener(this, &testApp::onFlickerChange);
    gui.add(wb);
	
	ofxToggle * led = new ofxToggle();
    led->setup("LED", true);
	led->addListener(this, &testApp::onLedChange);
	gui.add(led);
	
    
    ofxFloatSlider * minRadius = new ofxFloatSlider();
    minRadius->setup("Blob Min Radius", 30, 0, 500);
    minRadius->addListener(this, &testApp::onMinRadiusChange);
    gui.add(minRadius);
    
    ofxFloatSlider * maxRadius = new ofxFloatSlider();
    maxRadius->setup("Blob Max Radius", 300, 0, 500);
    maxRadius->addListener(this, &testApp::onMaxRadiusChange);
    gui.add(maxRadius);

    ofxFloatSlider * learningTime = new ofxFloatSlider();
    learningTime->setup("Learning Time", 900, 0, 1000);
    learningTime->addListener(this, &testApp::onLearningTimeChange);
    gui.add(learningTime);

	// Load initial values
    
    gui.loadFromFile("ps3eye.xml");
    bool b;
    float f;
    int i;
//    b = gui.getToggle("Send Centers");
//    onSendCenters(b);
    b = gui.getToggle("Auto Gain and Shutter");
    onAutoGainAndShutterChange(b);
    f = gui.getFloatSlider("Gain");
    onGainChange(f);
    f = gui.getFloatSlider("Shutter");
    onShutterChange(f);
    f = gui.getFloatSlider("Gamma");
    onGammaChange(f);
    f = gui.getFloatSlider("Brightness");
    onBrightnessChange(f);
    f = gui.getFloatSlider("Contrast");
    onContrastChange(f);
    f = gui.getFloatSlider("Hue");
    onHueChange(f);
    b = gui.getToggle("LED");
    onLedChange(b);
    i = gui.getIntSlider("Flicker Type");
    onFlickerChange(i);
    i = gui.getIntSlider("White Balance Mode");
    onWhiteBalanceChange(i);
    
    f=gui.getFloatSlider("Blob Min Radius");
    onMinRadiusChange(f);
    f=gui.getFloatSlider("Blob Max Radius");
    onMaxRadiusChange(f);
    f=gui.getFloatSlider("Learning Time");
    onLearningTimeChange(f);
    
    
    
//	gui.setup("settings");
//	gui.setDefaultBackgroundColor(ofColor(0, 0, 0, 127));
//	gui.setDefaultFillColor(ofColor(160, 160, 160, 160));
//	gui.add(guiFPS.set("FPS", 0, 0, 60));
//	gui.add(toggleGuiDraw.set("show gui (G)", false));
//	gui.add(doFlipCamera.set("flip camera (C)", true));
//    gui.add(camGain.set("Gain", 0.5, 0, 1));
//    gui.add(camShutter.set("Shutter", 1, 0, 1));
//    gui.add(learnTime.set("LearnTime", 900, 0, 1000));
//    gui.add(minRadius.set("minRadius", 30, 0, 200));
//    gui.add(maxRadius.set("maxRadius", 100, 0, 400));
//	
//	int guiColorSwitch = 0;
//	ofColor guiHeaderColor[2];
//	guiHeaderColor[0].set(160, 160, 80, 200);
//	guiHeaderColor[1].set(80, 160, 160, 200);
//	ofColor guiFillColor[2];
//	guiFillColor[0].set(160, 160, 80, 200);
//	guiFillColor[1].set(80, 160, 160, 200);
//	
//	gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
//	gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
//	guiColorSwitch = 1 - guiColorSwitch;
//    
//	gui.loadFromFile("settings.xml");
//	gui.minimizeAll();
	
	toggleGuiDraw = true;
	
}



void testApp::oscSendCur(Glow & follower){
//    cout<<"sending Cur: cam Gain is: "<<camGain<<" "<<endl;
    //        followers[i].report();
    cout<<follower.getLabel()<<": "<<follower.getCur()[0]<<", "<<follower.getCur()[1]<<endl;
    ofxOscMessage m;
    stringstream ss;
    ss<<"/cur";
    m.setAddress(ss.str());
    m.addIntArg(follower.getLabel());
    m.addFloatArg(follower.getCur()[0]);
    m.addFloatArg(follower.getCur()[1]);
    sender.sendMessage(m);
    
//    follower.
}

void testApp::oscSendContour(int label, const ofPolyline &polyline){
    ofxOscMessage m;
    stringstream ss;
    ss<<"/contour";
    m.setAddress(ss.str());
    
    int size = polyline.size();
    m.addIntArg(label);
    m.addIntArg(size);
    cout<<"contour: "<<label<<" size: "<<size<<endl;
    const ofRectangle& rect = polyline.getBoundingBox();
    m.addIntArg(rect.getTopLeft().x);
    m.addIntArg(rect.getTopLeft().y);
    m.addIntArg(rect.getBottomRight().x);
    m.addIntArg(rect.getBottomRight().y);
    
    if(bSendContours){
        const vector<ofPoint> points = polyline.getVertices();
        for(int i=0; i< size; i++){
            m.addFloatArg(points[i].x);
            m.addFloatArg(points[i].y);
        }
    }
    sender.sendMessage(m);
}

void testApp::keyPressed(int key){
    switch(key){
        case ' ':
            background.reset();
            break;
        case 'G':
            toggleGuiDraw = !toggleGuiDraw;
            break;
    }
}


void testApp::onAutoGainAndShutterChange(bool & value){
	ps3Eye.setAutoGainAndShutter(value);
    bPs3AutoExposure = value;
    cout<<"autoexposure is "<<bPs3AutoExposure<<endl;
}
//void testApp::onSendCenters(bool &value){
//    bSendCenters = value;
//}

//void testApp::onSendTargetDetail(bool &value){
//    bSendTargetDetail = value;
//}
//void testApp::onSendContours(bool &value){
//    bSendContours = value;
//}
//--------------------------------------------------------------
void testApp::onGainChange(float & value){
    cout<<"calling gain change"<<endl;
	// Only set if auto gain & shutter is off
//	if(!(bool&)gui.getToggle("Auto Gain and Shutter")){
    if(!bPs3AutoExposure){
        ps3Eye.setGain(value);
        cout<<"gain is "<<value<<endl;
	}
}

//--------------------------------------------------------------
void testApp::onShutterChange(float & value){
	// Only set if auto gain & shutter is off
//	if(!(bool&)gui.getToggle("Auto Gain and Shutter")){
    if(!bPs3AutoExposure){
        ps3Eye.setShutter(value);
	}
}

//--------------------------------------------------------------
void testApp::onGammaChange(float & value){
	ps3Eye.setGamma(value);
}

//--------------------------------------------------------------
void testApp::onBrightnessChange(float & value){
	ps3Eye.setBrightness(value);
}

//--------------------------------------------------------------
void testApp::onContrastChange(float & value){
	ps3Eye.setContrast(value);
}

//--------------------------------------------------------------
void testApp::onHueChange(float & value){
	ps3Eye.setHue(value);
}

//--------------------------------------------------------------
void testApp::onLedChange(bool & value){
	ps3Eye.setLed(value);
}

//--------------------------------------------------------------
void testApp::onFlickerChange(int & value){
	ps3Eye.setFlicker(value);
}

//--------------------------------------------------------------
void testApp::onWhiteBalanceChange(int & value){
	ps3Eye.setWhiteBalance(value);
}

void testApp::onMinRadiusChange(float &value){
    contourFinder.setMinAreaRadius(value);
}
void testApp::onMaxRadiusChange(float &value){
    contourFinder.setMaxAreaRadius(value);
}
void testApp::onLearningTimeChange(float &value){
    background.setLearningTime(value);
}


