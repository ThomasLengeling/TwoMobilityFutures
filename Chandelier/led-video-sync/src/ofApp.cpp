#include "ofApp.h"

void ofApp::setup()
{
    string testVideo = "videos/A001C000574.mov";
    player.load(testVideo);

    initGui();
    initSerial();
    loadJSON();
    getVideos();
}

void ofApp::update()
{
    if (useSerial)
        updateSerial();

    // TODO: handshake not fully implemented
    // if (!completedHandshake)
    //     return;

    updateControls();
    updateTimestamps();
    player.update();
}

void ofApp::draw()
{
    player.draw(0, 0, videoWidth, videoHeight);
    gui.draw();
    scrub.draw();
    drawStats();
}

/*
-- Serial --
*/

void ofApp::initSerial()
{
    if (useSerial)
    {
        int baud = 9600;
        mserial.setup(0, baud);
    }
}

// Read/ write serial
void ofApp::updateSerial()
{
    if (mserial.available())
    {
        // TODO: not fully implemented on arduino end
        if (!completedHandshake){
            requestHandshake();
        }
        mserial.readBytes(receivedData, 10);
        cout << receivedData;
        cout << "\n";
        cout.flush();
        
        if (receivedData == (char *)handshakeMessage)
        {
            completedHandshake = true;
        }
        
        // NOTE: only triggered by GUI
        if (ledButtonOn)
        {
            unsigned char myByte = 1;
            mserial.writeByte(myByte);
            printf("on\n");
        }
        if (ledButtonOff)
        {
            unsigned char myByte = 2;
            mserial.writeByte(myByte);
            printf("off\n");
        }
        if (ledButtonBreathe)
        {
            unsigned char myByte = 3;
            mserial.writeByte(myByte);
            printf("breath\n");
        }
        cout.flush();
    }
}

// Initilize handshake with Teensy
// TODO: not fully implemented on Teensy side
void ofApp::requestHandshake()
{
    cout << "requesting handshake\n";
    mserial.writeByte(handshakeMessage);
}

/* 
-- GUI --
*/

void ofApp::initGui()
{
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

void ofApp::updateControls()
{
    if (videoStartButton && !player.isPlaying())
    {
        printf("start\n");
        cout.flush();
        player.play();
    }
    if (videoStopButton)
    {
        printf("stop\n");
        cout.flush();
        player.stop();
    }
}

void ofApp::updateScrubber(int &value)
{
    printf("set frame: %i\n", value);
    cout.flush();
    player.setFrame(scrubber);
}

void ofApp::drawStats()
{
    ofDrawBitmapString("Video Speed: " + std::to_string(player.getSpeed()), 20, videoHeight + 20);
    ofDrawBitmapString("Video Frame: " + std::to_string(player.getCurrentFrame()), 20, videoHeight + 35);
}

/*
-- Data / Effects --
*/

// Load JSON containing effects to trigger at given frames
void ofApp::loadJSON()
{
    string filepath = "test-video-timestamps.json";
    ofFile file(filepath);
    if (!file.exists())
        return;

    ofJson json = ofLoadJson(filepath);
    for (auto &entry : json["timestamps"])
    {
        if (!entry.empty())
        {
            timestamp t;
            t.frame = entry["frame"];
            t.effect = entry["effect"];
            t.code = entry["code"];
            timestamps.push_back(t);
        }
    }
}

// Retrieve list of videos in project data directory (e.g. used to select videos via GUI)
void ofApp::getVideos()
{
    string path = "videos/";
    ofDirectory dir(path);
    dir.listDir();
    for (int i = 0; i < dir.size(); i++)
    {
        ofLogNotice(dir.getPath(i));
    }
}

// Trigger specified effect at timestamp
void ofApp::updateTimestamps()
{
    int currentFrame = player.getCurrentFrame();
    for (auto timestamp = timestamps.begin(); timestamp != timestamps.end(); ++timestamp)
    {
        if (currentFrame == timestamp->frame)
        {
            mserial.writeByte((char)timestamp->code);
            printf("%c", timestamp->effect.c_str());
            cout.flush();
        }
    }
}

