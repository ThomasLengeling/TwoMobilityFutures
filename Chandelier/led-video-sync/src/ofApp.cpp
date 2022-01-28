#include "ofApp.h"

void ofApp::setup() {
    string videoPath = "videos/A001C000574.mov";
    // string jsonPath = "test-video-timestamps.json";
    // string srtPath =
    //     "/Users/justinblinder/dev/medialab/TwoMobilityFutures/Chandelier/led-video-sync/bin/data/"
    //     "aom-test_SRT_English.srt";

    player.load(videoPath);
    initGui();
    initSerial();
    // used for test app
    /*
    loadJSON(jsonPath);
    vector<string> videos = { "woman-test" };
    initVideoEffects(videos);
    loadVideo("woman-test");
    */
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
        if (ledButtonCandle) {
            unsigned char myByte = 4;
            mserial.writeByte(myByte);
            printf("candle\n");
        }
        if (ledButtonStrobe) {
            unsigned char myByte = 5;
            mserial.writeByte(myByte);
            printf("strobe\n");
        }
        if (ledButtonRandomStrobe) {
            unsigned char myByte = 6;
            mserial.writeByte(myByte);
            printf("random strobe\n");
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
    ledGroup.add(ledButtonRandomStrobe.setup("LED Random Strobe"));

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
/*
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

void ofApp::initVideoEffects(vector<string> videoNames) { //loadSubtitles(string subtitleFilesPath) {
    // ofPath = ofFilePath::getAbsolutePath(ofToDataPath("C:/Users/Bizon/Desktop/App/data/"));
    // string subtitlesDir = ofPath+"subtitles/";
    string subtitlesDir = "/Users/justinblinder/dev/medialab/TwoMobilityFutures/Chandelier/led-video-sync/bin/data/";

    for (int i = 0; i < videoNames.size(); i++) {
        string videoName = videoNames[i];
        string videoSubtitlesPath = subtitlesDir + videoName + ".srt";
        cout << videoName << " " << videoSubtitlesPath << endl;
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
        cout << words.front() << " " << e.startTime << endl;
        e.type = words.front();           // name of effect
        e.code = std::stoi(words.at(1));  // byte code TODO: create enum to perform string/ code lookup
        effects.push_back(e);
    }
    cout.flush();
    return effects;
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
