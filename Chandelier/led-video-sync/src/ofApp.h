#pragma once

#include "ofMain.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp {
   public:
    void setup();
    void update();
    void draw();
    void initSerial();
    void initGui();
    void getVideos();
    void loadJSON(string jsonPath);
    void loadSubtitles(string srtPath);
    void drawStats();
    void updateSerial();
    void updateControls();
    void updateEffects();
    void updateScrubber(int &value);
    void requestHandshake();

    // Video choreography from .srt
    struct effect {
        int frame;
        int startTime;
        int code;
        string type;
    };
    vector<effect> effects;

    // Video
    ofVideoPlayer player;
    const int videoWidth = 2880 / 3;   // 1024;
    const int videoHeight = 1620 / 3;  // 435;
    int frameRate = 24; // temp rate to calculate duration in millis, matches rate of test video

    // Gui
    ofxPanel gui;

    ofxGuiGroup assetsGroup;

    ofxGuiGroup videoGroup;
    ofxButton videoStartButton;
    ofxButton videoStopButton;

    ofxGuiGroup ledGroup;
    ofxButton ledButtonOn;
    ofxButton ledButtonOff;
    ofxButton ledButtonBreathe;
    ofxButton ledButtonCandle;
    ofxButton ledButtonStrobe;

    ofxPanel scrub;
    ofParameter<int> scrubber;

    // Serial
    ofSerial mserial;
    char receivedData[10];
    bool useSerial = true;
    bool completedHandshake = false;
    char handshakeMessage = 0;
};
