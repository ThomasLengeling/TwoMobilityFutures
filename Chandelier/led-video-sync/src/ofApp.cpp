#include "ofApp.h"

#include "srtparser.h"

void ofApp::setup() {
    string videoPath = "videos/A001C000574.mov";
    string jsonPath = "test-video-timestamps.json";
    string srtPath = "/Users/justinblinder/dev/medialab/TwoMobilityFutures/Chandelier/led-video-sync/bin/data/aom-test_SRT_English.srt";

    player.load(videoPath);
    initGui();
    initSerial();
    // loadJSON(jsonPath);
    loadSubtitles(srtPath);
    getVideos();

    ofSetFrameRate(frameRate);
}

void ofApp::update() {
    if (useSerial) updateSerial();

    // TODO: handshake not fully implemented
    // if (!completedHandshake)
    //     return;

    updateControls();
    updateEffects();
    player.update();
}

void ofApp::draw() {
    player.draw(0, 0, videoWidth, videoHeight);
    gui.draw();
    scrub.draw();
    drawStats();
}

/*
-- Serial --
*/

void ofApp::initSerial() {
    if (useSerial) {
        int baud = 9600;
        mserial.setup(0, baud);
    }
}

// Read/ write serial
void ofApp::updateSerial() {
    if (mserial.available()) {
        // TODO: not fully implemented on arduino end
        if (!completedHandshake) {
            requestHandshake();
        }
        mserial.readBytes(receivedData, 10);
        cout << receivedData;
        cout << "\n";
        cout.flush();

        if (receivedData == (char *)handshakeMessage) {
            completedHandshake = true;
        }

        // NOTE: only triggered by GUI
        if (ledButtonOn) {
            unsigned char myByte = 1;
            mserial.writeByte(myByte);
            printf("on\n");
        }
        if (ledButtonOff) {
            unsigned char myByte = 2;
            mserial.writeByte(myByte);
            printf("off\n");
        }
        if (ledButtonBreathe) {
            unsigned char myByte = 3;
            mserial.writeByte(myByte);
            printf("breath\n");
        }
        cout.flush();
    }
}

// Initilize handshake with Teensy
// TODO: not fully implemented on Teensy side
void ofApp::requestHandshake() {
    cout << "requesting handshake\n";
    mserial.writeByte(handshakeMessage);
}

/*
-- GUI --
*/

void ofApp::initGui() {
    gui.setup();
    videoGroup.setup("Video Controls");
    videoGroup.add(videoStartButton.setup("Start"));
    videoGroup.add(videoStopButton.setup("Stop"));

    ledGroup.setup("LED Controls");
    ledGroup.add(ledButtonOn.setup("LED On"));
    ledGroup.add(ledButtonOff.setup("LED Off"));
    ledGroup.add(ledButtonBreathe.setup("LED Breathe"));
    ledGroup.add(ledButtonCandle.setup("LED Candle"));
    ledGroup.add(ledButtonStrobe.setup("LED Strobe"));

    gui.add(&videoGroup);
    gui.add(&ledGroup);

    scrub.setup();
    scrub.add(scrubber.set("Scrub", 0, 0, player.getTotalNumFrames()));
    scrub.setPosition(0, ofGetHeight() - 50);
    scrub.setSize(ofGetWidth(), 50);
    scrubber.addListener(this, &ofApp::updateScrubber);
}

void ofApp::updateControls() {
    if (videoStartButton && !player.isPlaying()) {
        printf("start\n");
        cout.flush();
        player.play();
    }
    if (videoStopButton) {
        printf("stop\n");
        cout.flush();
        player.stop();
    }
}

void ofApp::updateScrubber(int &value) {
    printf("set frame: %i\n", value);
    cout.flush();
    player.setFrame(scrubber);
}

void ofApp::drawStats() {
    ofDrawBitmapString("Video Speed: " + std::to_string(player.getSpeed()), 20, videoHeight + 20);
    ofDrawBitmapString("Video Frame: " + std::to_string(player.getCurrentFrame()), 20, videoHeight + 35);
}

/*
-- Data / Effects --
*/

// Load JSON containing effects to trigger at given frames
void ofApp::loadJSON(string jsonPath) {
    ofFile file(jsonPath);
    if (!file.exists()) return;

    ofJson json = ofLoadJson(jsonPath);
    for (auto &entry : json["timestamps"]) {
        if (!entry.empty()) {
            effect e;
            e.frame = entry["frame"];
            e.type = entry["effect"];
            e.code = entry["code"];
            effects.push_back(e);
        }
    }
}

// Load SubRip .srt file containing effects to trigger at given frames
void ofApp::loadSubtitles(string srtPath) {
    SubtitleParserFactory *subParserFactory = new SubtitleParserFactory(srtPath);
    SubtitleParser *parser = subParserFactory->getParser();
    vector<SubtitleItem *> sub = parser->getSubtitles();
    cout << sub.size();
    for (SubtitleItem *element : sub) {
        vector<std::string> words = element->getIndividualWords();
        if (words.empty()) continue;

        effect e;
        e.startTime = element->getStartTime();
        e.type = words.front(); // name of effect
        e.code = std::stoi(words.at(1)); // byte code TODO: create enum to perform string/ code lookup
        effects.push_back(e);
    }
    cout.flush();
}

// Retrieve list of videos in project data directory (e.g. used to select videos
// via GUI)
void ofApp::getVideos() {
    string path = "videos/";
    ofDirectory dir(path);
    dir.listDir();
    for (int i = 0; i < dir.size(); i++) {
        ofLogNotice(dir.getPath(i));
    }
}

// Trigger specified effect at timestamp
void ofApp::updateEffects() {
    float currentFrame = player.getCurrentFrame();
    float currentMillis = currentFrame/ frameRate * 1000;
    cout << "current frame " << currentFrame;
    cout << "\ncurrent position " << currentMillis << "\n";
    
    for (auto effect = effects.begin(); effect != effects.end(); ++effect) {
        if (currentMillis < effect->startTime)
            continue;

        mserial.writeByte((char)effect->code);
        printf("----------------------------------------");
        printf("%c", effect->type.c_str());
        printf("----------------------------------------");
        effects.erase(effects.begin());
        break;
    }
    cout.flush();
}
