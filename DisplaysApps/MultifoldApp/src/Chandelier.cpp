#include "Chandelier.h"

Chandelier::Chandelier() {
    useSerial = true;
    completedHandshake = false;
    handshakeMessage = "s";
    mSerialId = 0;
    mSerialBaudRate = 115200;
}

//---------------------------------------------------------
void Chandelier::initSerial(int portid, int baud) {
    ofLog(OF_LOG_NOTICE) << "init Serial  ";

    mSerialBaudRate = baud;
    mSerial.listDevices();

    vector<ofSerialDeviceInfo> deviceList = mSerial.getDeviceList();

    // default values:
    mSerialId = portid;
    if (!deviceList.empty() && portid < deviceList.size()) {
        mSerialName = deviceList.at(portid).getDeviceName();
    }

    // find the correponding serial port
    for (auto& devices : deviceList) {
        ofLog(OF_LOG_NOTICE) << devices.getDeviceName() << " " << devices.getDeviceID();
        if (mSerialName == devices.getDeviceName()) {
            mSerialName = devices.getDeviceName();
            mSerialId = devices.getDeviceID();
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
    if (!useSerial) return;
    if (!completedHandshake) requestHandshake();

    // NOTE: only triggered by GUI
    if (ledButtonOn) {
        char* buf = "1";
        mSerial.writeBytes(buf, 8);
        printf("pressed on\n");
    }
    if (ledButtonOff) {
        char* buf = "2,2000";
        mSerial.writeBytes(buf, 8);
        printf("pressed off\n");
    }
    if (ledButtonBreathe) {
        char* buf = "3,500";
        mSerial.writeBytes(buf, 8);
        printf("pressed breathe\n");
    }
    if (ledButtonCandle) {
        char* buf = "4,500";
        mSerial.writeBytes(buf, 8);
        printf("pressed candle\n");
    }
    if (ledButtonStrobe) {
        char* buf = "5,500";
        mSerial.writeBytes(buf, 8);
        printf("pressed strobe\n");
    }
    if (ledButtonRandomStrobe) {
        char* buf = "6,500";
        mSerial.writeBytes(buf, 8);
        printf("pressed random strobe\n");
    }
    if (ledButtonFadeOn) {
        char* buf = "7,500";
        mSerial.writeBytes(buf, 8);
        printf("pressed fade on\n");
    }
    if (ledButtonFadeOff) {
        char* buf = "8,500";
        mSerial.writeBytes(buf, 8);
        printf("pressed fade off\n");
    }
    if (ledButtonFadeOnSequential) {
        char* buf = "9,500";
        mSerial.writeBytes(buf, 8);
        printf("pressed fade on seq\n");
    }
    if (ledButtonFadeOffSequential) {
        char* buf = "10,500";
        mSerial.writeBytes(buf, 8);
        printf("pressed fade off seq\n");
    }

    cout.flush();
}

// Initilize handshake with Teensy
//--------------------------------------------------------
void Chandelier::requestHandshake() {
    if (mSerial.available()) {
        mSerial.readBytes(receivedData, 8);

        if (receivedData != NULL && !completedHandshake) {
            if (receivedData[0] == 's') {
                cout << "handshake success" << endl;
                completedHandshake = true;
            }
        }
    }

    if (!completedHandshake) {
        cout << "requesting handshake" << endl;
        mSerial.writeBytes(handshakeMessage, 8);
    }
}

/*
-- GUI --
*/
//--------------------------------------------------------
void Chandelier::initGui() {
    videoGroup.setup("Video Controls");
    ledGroup.setup("LED Controls");
    ledGroup.add(ledButtonOn.setup("LED On"));
    ledGroup.add(ledButtonOff.setup("LED Off"));
    ledGroup.add(ledButtonBreathe.setup("LED Breathe"));
    ledGroup.add(ledButtonCandle.setup("LED Candle"));
    ledGroup.add(ledButtonStrobe.setup("LED Strobe"));
    ledGroup.add(ledButtonRandomStrobe.setup("LED Random Strobe"));
    ledGroup.add(ledButtonFadeOn.setup("Fade In"));
    ledGroup.add(ledButtonFadeOff.setup("Fade Off"));
    ledGroup.add(ledButtonFadeOnSequential.setup("Fade In Seq"));
    ledGroup.add(ledButtonFadeOffSequential.setup("Fade Off Seq"));
}
//--------------------------------------------------------

void Chandelier::updateControls() {
    if (videoStartButton) {
        printf("start\n");
        cout.flush();
    }
    if (!videoStopButton) {
        printf("stop\n");
        cout.flush();
    }
}

void Chandelier::updateScrubber(int& value) {
    printf("set frame: %i\n", value);
    cout.flush();
}

/*
-- Data / Effects --
*/


void Chandelier::loadVideo(string videoName) {
    for (int i = 0; i < videos.size(); i++) {
        video v = videos[i];
        cout << v.name << " " << v.name << endl;
        if (v.name == videoName) {
            currentVideo = v;
            break;
        }
    }
    // Turn off chandelier on each load TODO:
    char* buf = "2,2000";
    mSerial.writeBytes(buf, 8);
    printf("reset chandelier\n");
    cout << "current video " << currentVideo.name;
}

// Load SRT file associated with each video name
//--------------------------------------------------------
void Chandelier::initVideoEffects(vector<string> videoNames) {
    // TODO: prevent having to duplicate this from ofApp.cpp
    string subtitlesDir = ofFilePath::getAbsolutePath(ofToDataPath("C:/Users/Bizon/Desktop/App/data/")) + "subtitles/";
    for (int i = 0; i < videoNames.size(); i++) {
        string videoName = videoNames[i];
        string videoSubtitlesPath = subtitlesDir + videoName + ".srt";

        video v;
        v.name = videoName;
        v.subtitlesPath = videoSubtitlesPath;
        v.effects = parseVideoEffects(videoSubtitlesPath);
        cout << videoSubtitlesPath << endl;
        videos.push_back(v);
    }
}

// Access data from individual subtitle caption (seperated because this could later get more complex/ nuanced)
//--------------------------------------------------------
vector<effect> Chandelier::parseVideoEffects(string subtitleFilesPath) {
    SubtitleParserFactory* subParserFactory = new SubtitleParserFactory(subtitleFilesPath);

    SubtitleParser* parser = subParserFactory->getParser();
    vector<SubtitleItem*> subs = parser->getSubtitles();

    vector<effect> effects;
    if (subs.size() > 0) {
        for (SubtitleItem* element : subs) {
            vector<std::string> words = element->getIndividualWords();
            if (words.empty()) continue;

            effect e;
            e.startTime = element->getStartTime();
            cout << words.front() << " " << e.startTime << endl;
            e.type = words.front();
            e.code = std::stoi(words.at(1));
            // check for speed, not all effects have a speed modifier specified
            try {
                e.speed = std::stoi(words.at(2));
            } catch (const std::out_of_range& ex) {
                e.speed = 0;
            }
            effects.push_back(e);
        }
    }
    cout.flush();
    return effects;
}

// Retrieve list of videos in project data directory (e.g. used to select videos
// via GUI)
//--------------------------------------------------------
void Chandelier::getVideos() {
    string path = "videos/";
    ofDirectory dir(path);
    dir.listDir();
    for (int i = 0; i < dir.size(); i++) {
        ofLogNotice(dir.getPath(i));
    }
}

// Trigger specified effect at timestamp
//--------------------------------------------------------
void Chandelier::updateEffects(int currentFrame, float currentFPS) {
    float currentMillis = currentFrame / currentFPS * 1000;

    for (auto effect = currentVideo.effects.begin(); effect != currentVideo.effects.end(); ++effect) {
        // check if it's time for next effect
        if (currentMillis < effect->startTime) continue;

        // construct command from effect
        string command = to_string(effect->code) + "," + to_string(effect->speed);

        // send comma seperated command to hardware
        mSerial.writeBytes(command.c_str(), 8);
        cout << "----------------------------------------" << endl;
        cout << command << endl;
        cout << "----------------------------------------" << endl;

        // remove effect from list (gets repopulated when a new video loads)
        currentVideo.effects.erase(currentVideo.effects.begin());
        break;
    }
    cout.flush();
}