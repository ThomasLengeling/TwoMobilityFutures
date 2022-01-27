#pragma once

//
//  VideoWarp.hpp
//  MultifoldApp
//
//  Created by Thomas on 3/3/20.
//

#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "srtparser.h"

class Chandelier;

typedef std::shared_ptr<Chandelier> ChandelierRef;

    // Video choreography from .srt
struct effect {
    int frame;
    int startTime;
    int code;
    string type;
};

class Chandelier {
public:
    Chandelier();

    static ChandelierRef create() {
        return std::make_shared<Chandelier>();
    }

    void initSerial(int portid, int baud);
    void initGui();
    void getVideos();
    void loadJson(string jsonPath);
    void loadSubtitles(string srtPath);
    void drawStats();
    void updateSerial();
    void updateControls();
    void updateTimeStamp(int currentFrame);
    void updateEffects(int currentFrame);
    void updateScrubber(int& value);
    void requestHandshake();

    string getSerialName() { return mSerialName; }
    int getSerialId() { return mSerialId; }
    int getBaudRate() { return mSerialBaudRate; }
    bool isSeralConnected() { return useSerial;};

    ofxGuiGroup ledGroup;
    // TODO: temp fix, figure out how to determine millis in video
    int frameRate = 24;

private:
    vector<effect> effects;

    //gui
    ofxGuiGroup assetsGroup;

    ofxGuiGroup videoGroup;
    ofxButton videoStartButton;
    ofxButton videoStopButton;


    ofxButton ledButtonOn;
    ofxButton ledButtonOff;
    ofxButton ledButtonBreathe;
    ofxButton ledButtonCandle;
    ofxButton ledButtonStrobe;

    ofxPanel scrub;
    ofParameter<int> scrubber;

    // Serial
    ofSerial mSerial;
    std::string mSerialName;
    int mSerialBaudRate;
    int mSerialId;

    char receivedData[10];
    bool useSerial;
    bool completedHandshake;
    char handshakeMessage;
};