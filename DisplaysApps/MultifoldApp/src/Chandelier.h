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

// Video effects
struct effect {
    int frame;
    int startTime;
    int code;
    int speed;
    string type;
};
struct video {
    string name;
    string subtitlesPath;
    vector<effect> effects;
};
class Chandelier {
public:
    Chandelier();

    static ChandelierRef create() {
        return std::make_shared<Chandelier>();
    }

    void initSerial(int portid, int baud = 115200);
    void initGui();
    void getVideos();
    void initVideoEffects(vector<string> videoNames);
    void loadVideo(string videoName);
    void updateSerial();
    void updateControls();
    void updateEffects(int currentFrame, float currentFPS);
    void updateScrubber(int& value);
    void requestHandshake();

    string getSerialName() { return mSerialName; }
    int getSerialId() { return mSerialId; }
    int getBaudRate() { return mSerialBaudRate; }
    bool isSeralConnected() { return useSerial;};

    bool getHandShake() {
        return completedHandshake;
    }
    void updateHandShake() {
        requestHandshake();
    }

    ofxGuiGroup ledGroup;
    // TODO: temp fix, figure out how to determine millis in video
    int frameRate = 25;
    bool useSerial;

private:
    vector<effect> parseVideoEffects(string subtitleFilesPath);

    vector<video> videos;
    video currentVideo;

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
    ofxButton ledButtonRandomStrobe;
    ofxButton ledButtonFadeOn;
    ofxButton ledButtonFadeOff;
    ofxButton ledButtonFadeOnSequential;
    ofxButton ledButtonFadeOffSequential;

    ofxPanel scrub;
    ofParameter<int> scrubber;

    // Serial
    ofSerial mSerial;
    std::string mSerialName;
    int mSerialBaudRate;
    int mSerialId;

    char receivedData[10];

    bool completedHandshake;
    char* handshakeMessage;
};