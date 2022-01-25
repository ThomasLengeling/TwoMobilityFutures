#include "Chandelier.h"


Chandelier::Chandelier() {
    useSerial = false;;
    completedHandshake = false;
    handshakeMessage = 's';
    mSerialId = 0;
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
        mSerial.readBytes(receivedData, 10);
        cout << receivedData;
        cout << "\n";
        cout.flush();

        if (receivedData == (char*)handshakeMessage) {
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
        cout.flush();
    }
}

// Initilize handshake with Teensy
// TODO: not fully implemented on Teensy side

//--------------------------------------------------------
void Chandelier::requestHandshake() {
    cout << "requesting handshake\n";
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

// Load JSON containing effects to trigger at given frames
void Chandelier::loadJson() {
    string filepath = "test-video-timestamps.json";
    ofFile file(filepath);
    if (file.exists()){
        ofLog(OF_LOG_NOTICE) << " Loading Chanderlier time step json: " << filepath;
        ofJson json = ofLoadJson(filepath);
        for (auto& entry : json["timestamps"]) {
            if (!entry.empty()) {
                timestamp t;
                t.frame = entry["frame"];
                t.effect = entry["effect"].get<string>();
                t.code = entry["code"];
                timestamps.push_back(t);
            }
        }
    }
    else {
        ofLog(OF_LOG_NOTICE) << " Error loading File" << filepath;
    }
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
    for (auto timestamp = timestamps.begin(); timestamp != timestamps.end(); ++timestamp) {
        if (currentFrame == timestamp->frame) {
            mSerial.writeByte((char)timestamp->code);
            printf("%c", timestamp->effect.c_str());
            cout.flush();
        }
    }
}
