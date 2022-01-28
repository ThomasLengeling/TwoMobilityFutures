#include "Chandelier.h"


Chandelier::Chandelier() {
    useSerial = false;;
    completedHandshake = false;
    handshakeMessage = 's';
    mSerialId = 0;
    mSerialBaudRate = 9600;
}
//---------------------------------------------------------
void Chandelier::initSerial(int portid, int baud) {
    ofLog(OF_LOG_NOTICE) << "init Serial  ";

    mSerialBaudRate = baud;
    mSerial.listDevices();

    vector <ofSerialDeviceInfo> deviceList = mSerial.getDeviceList();

    //default values:
    mSerialId = portid;
    if (!deviceList.empty() && portid < deviceList.size()) {
        mSerialName = deviceList.at(portid).getDeviceName();
    }

    //find the correponding serial port 
    for (auto& devices : deviceList) {
        ofLog(OF_LOG_NOTICE) << devices.getDeviceName() << " " << devices.getDeviceID();
        if (mSerialName == devices.getDeviceName()) {
            mSerialName = devices.getDeviceName();
            mSerialId   = devices.getDeviceID();
            break;
        }
    }

    useSerial = mSerial.setup(mSerialId, mSerialBaudRate);
    if (useSerial) {
        ofLog(OF_LOG_NOTICE) << "load device " << mSerialName << " " << mSerialId << " " << mSerialBaudRate;
    }

}

// Read/ write serial

//---------------------------------------------------------
void Chandelier::updateSerial() {
    if (mSerial.available()) {
        // TODO: not fully implemented on arduino end
        if (!completedHandshake) {
            requestHandshake();
        }
        mSerial.readBytes(receivedData, 1);

        if (receivedData == (char*)handshakeMessage) {
            cout << "handshake completed\n";
            completedHandshake = true;
        }

        // NOTE: only triggered by GUI
        if (ledButtonOn) {
            unsigned char myByte = 1;
            mSerial.writeByte(myByte);
            printf("on\n");
        }
        if (ledButtonOff) {
            unsigned char myByte = 2;
            mSerial.writeByte(myByte);
            printf("off\n");
        }
        if (ledButtonBreathe) {
            unsigned char myByte = 3;
            mSerial.writeByte(myByte);
            printf("breath\n");
        }
        if (ledButtonCandle) {
            unsigned char myByte = 4;
            mSerial.writeByte(myByte);
            printf("candle\n");
        }
        if (ledButtonStrobe) {
            unsigned char myByte = 5;
            mSerial.writeByte(myByte);
            printf("strobe\n");
        }
        if (ledButtonRandomStrobe) {
            unsigned char myByte = 6;
            mSerial.writeByte(myByte);
            printf("random strobe\n");
        }
        cout.flush();
    }
}

// Initilize handshake with Teensy
// TODO: not fully implemented on Teensy side

//--------------------------------------------------------
void Chandelier::requestHandshake() {
    //cout << "requesting handshake\n";
    mSerial.writeByte(handshakeMessage);
}

/*
-- GUI --
*/
//--------------------------------------------------------
void Chandelier::initGui() {
   // gui.setup();
    videoGroup.setup("Video Controls");
    //videoGroup.add(videoStartButton.setup("Start"));
    //videoGroup.add(videoStopButton.setup("Stop"));

    ledGroup.setup("LED Controls");
    ledGroup.add(ledButtonOn.setup("LED On"));
    ledGroup.add(ledButtonOff.setup("LED Off"));
    ledGroup.add(ledButtonBreathe.setup("LED Breathe"));
    ledGroup.add(ledButtonCandle.setup("LED Candle"));
    ledGroup.add(ledButtonStrobe.setup("LED Strobe"));
    ledGroup.add(ledButtonRandomStrobe.setup("LED Random Strobe"));
    //gui.add(&videoGroup);
    //gui.add(&ledGroup);

    //scrub.setup();
   // scrub.add(scrubber.set("Scrub", 0, 0, this->getTotalNumFrames()));
    //scrub.setPosition(0, ofGetHeight() - 50);
    //scrub.setSize(ofGetWidth(), 50);
    //scrubber.addListener(this, &Chandelier::updateScrubber);
}
//--------------------------------------------------------
    
void Chandelier::updateControls() {
    if (videoStartButton ) { //&& !player.isPlaying()
        printf("start\n");
        cout.flush();
        //player.play();
    }
    if (!videoStopButton) {
        printf("stop\n");
        cout.flush();
        //player.stop();
    }
}

void Chandelier::updateScrubber(int& value) {
    printf("set frame: %i\n", value);
    cout.flush();
    //player.setFrame(scrubber);
}

void Chandelier::drawStats() {
    //ofDrawBitmapString("Video Speed: " + std::to_string(player.getSpeed()), 20, videoHeight + 20);
    //ofDrawBitmapString("Video Frame: " + std::to_string(player.getCurrentFrame()), 20, videoHeight + 35);
}

/*
-- Data / Effects --
*/

void ofApp::loadVideo(string videoName){
    for (int i = 0; i < videos.size(); i++) {
        video v = videos[i];
        cout << v.name << " " << v.name << endl;
        if (v.name == videoName){
            currentVideo = v;
            break;
        }
    }
}


void ofApp::initVideoEffects(vector<string> videoNames) {
    // TODO: prevent having to duplicate this from ofApp.cpp
    ofPath = ofFilePath::getAbsolutePath(ofToDataPath("C:/Users/Bizon/Desktop/App/data/"));
    string subtitlesDir = ofPath+"subtitles/";

    for (int i = 0; i < videoNames.size(); i++) {
        string videoName = videoNames[i];
        string videoSubtitlesPath = subtitlesDir + videoName + ".srt";

        video v;
        v.name = videoName;
        v.subtitlesPath = videoSubtitlesPath;
        v.effects = parseVideoEffects(videoSubtitlesPath);
        videos.push_back(v);
    }
}

// Access data from individual subtitle caption (seperated because this could later get more complex/ nuanced)
vector<effect> ofApp::parseVideoEffects(string subtitleFilesPath) {
    SubtitleParserFactory *subParserFactory = new SubtitleParserFactory(subtitleFilesPath);
    SubtitleParser *parser = subParserFactory->getParser();
    vector<SubtitleItem *> subs = parser->getSubtitles();
    
    vector<effect> effects;
    for (SubtitleItem *element : subs) {
        vector<std::string> words = element->getIndividualWords();
        if (words.empty()) continue;

        effect e;
        e.startTime = element->getStartTime();
        e.type = words.front();           // name of effect
        e.code = std::stoi(words.at(1));  // byte code TODO: create enum to perform string/ code lookup
        effects.push_back(e);
    }
    cout.flush();
    return effects;
}


// Retrieve list of videos in project data directory (e.g. used to select videos
// via GUI)
void Chandelier::getVideos() {
    string path = "videos/";
    ofDirectory dir(path);
    dir.listDir();
    for (int i = 0; i < dir.size(); i++) {
        ofLogNotice(dir.getPath(i));
    }
}

// Trigger specified effect at timestamp
void Chandelier::updateTimeStamp(int currentFrame) {
    for (auto effect = effects.begin(); effect != effects.end(); ++effect) {
        if (currentFrame == effect->frame) {
            mSerial.writeByte((char)effect->code);
            printf("%c", effect->type.c_str());
            cout.flush();
        }
    }
}

// Trigger specified effect at timestamp
void ofApp::updateEffects() {
    float currentFrame = player.getCurrentFrame();
    float currentMillis = currentFrame / frameRate * 1000;
    cout << "current frame " << currentFrame;
    cout << "\ncurrent position " << currentMillis << "\n";

    for (auto effect = currentVideo.effects.begin(); effect != currentVideo.effects.end(); ++effect) {
        if (currentMillis < effect->startTime) continue;

        mserial.writeByte((char)effect->code);
        printf("----------------------------------------");
        printf("%c", effect->type.c_str());
        printf("----------------------------------------");
        currentVideo.effects.erase(currentVideo.effects.begin());
        break;
    }
    cout.flush();
}