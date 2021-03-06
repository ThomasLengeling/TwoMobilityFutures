#include "ofApp.h"

static int64_t prev_frame = 0;
static int64_t cur_frame = 0;
static uint8_t offset = 1;

using namespace std;

//--------------------------------------------------------------
void ofApp::setup(){

    ofLog(OF_LOG_NOTICE) << "Starting App" << std::endl;
    ofSetLogLevel("Logging items", OF_LOG_VERBOSE);

    //change path
    ofSetDataPathRoot("C:/Users/Bizon/Desktop/App/data/");
    ofPath = ofFilePath::getAbsolutePath(ofToDataPath("C:/Users/Bizon/Desktop/App/data/"));
    ofLog(OF_LOG_NOTICE) << " OF data path is: " << ofPath << endl;
    
    ofSetFrameRate(24);
    ofSetVerticalSync(false);
    ofBackground(0);
    ofDisableArbTex();
 
    //Video
    HPV::InitHPVEngine();

    //load UDP information
    setupUDP();

    setupChanderlier();

    //GUI
    setupGui();

    //OSC
    setupOSC();

    //audio
    setupAudioSystem();
    
    //commom state
    setupCommonState();

  

    ofResetElapsedTimeCounter();
    mInitTimer = 14000;//seconds
    mHandshakeUDP = false;

    mLockFpsUpdate = false;
    mLockFpsAudio = false;
    mLockFpsUDPAudio = false;
    mLoadedVidoes = true;
    mWaitPeriod = false;
    mAudioDone = false;
    mCurrSyncMode = 0;

    mLoopCount   = 0;
    initCouter = 0;
    mMaxLoopCount = 10;
    mDeltaFrame = 0;
    mDeltaSoundTime = 0.0;

    mPlayListCounter = 0;
    mPlayListMax = 3;

    mDeltaInc = false;
    mDeltaDec = false;

    ofLog(OF_LOG_NOTICE) << "Finishing setup";
    ofLog(OF_LOG_NOTICE) << "Size" << ofGetWindowWidth() << " " << ofGetWindowHeight();
}


//--------------------------------------------------------------
void ofApp::update() {

    //set the master frame with the current change frame
    //mMasterFrame.set(cur_frame);

    //UPDATE MASTER
    if (mMasterUDP) {

        //update audio pos
        double audioPos = player.getPosition();
        mMasterAudio.set(audioPos);

        mChanderlier->updateHandShake();

        //lock the fps with app frame rate
        if (mLockFpsUpdate) {

            //frame update 
            mCommon->commonFrame++;

            //chandelier update 
            mChanderlier->updateSerial();
            mChanderlier->updateEffects(mCommon->commonFrame, ofGetFrameRate());

            //frame reset
            if (mCommon->commonFrame >= mCommon->maxFrames) {
                mCommon->commonFrame = mCommon->maxFrames;
                mLockFpsUpdate = false;
                mWaitPeriod = true;
                mLoadPlayList = true;

                ofLog(OF_LOG_NOTICE) << "Done Master Frame";
            }

            //send error delta to salves
            if (mCommon->commonFrame % 15 == 0) {
                string message = "e " + to_string(mCommon->commonFrame) + " " + to_string(audioPos);
                udpSendLeft.Send(message.c_str(), message.length());
                udpSendCenter.Send(message.c_str(), message.length());
            }

        } //lock fps with audio update
        else if (mLockFpsAudio) {
            mCommon->audioPos = audioPos;

        }  //lock the fps with udp audio
        else if (mLockFpsUDPAudio) {
            sendAudioPosUDP(audioPos);
        }

        //check if the audio has finished playing
        if (player.getPosition() >= 1.0) {
            mAudioDone = true;
        }

        //------------------------

        //load period
        if (mLoadedVidoes) {

            //init sequence on computer start, only once
            initCouter++;
            if (initCouter > 24 * 15) { //wwiting time
                ofLog(OF_LOG_NOTICE) << "START MOVIE " << std::endl;
                startMasterVideo();

                mLoadedVidoes = false;
                initCouter = 0;

                //load videos
                std::fill(mCommon->mLoadedVideos.begin(), mCommon->mLoadedVideos.end(), true);
                cout.flush();

                //initial comands to start the chandelier
                ///
            }
        }

        //wait and reset
        if (mWaitPeriod && mAudioDone) {

            //waiting period for the load videos 
            initCouter++;
            if (mLoadPlayList) {
                //load videos

                //send to the slaves to enable video
                string message = "l " + to_string(mCurrSyncMode);
                udpSendLeft.Send(message.c_str(), message.length());
                udpSendCenter.Send(message.c_str(), message.length());

                mLoopCount++;

                // load new chandelier effects
                mChanderlier->loadVideo(mCommon->mCurrentSeqName);
                mCommon->commonFrame = 0;
                cout.flush();

                //update counter
                mPlayListCounter++;
                if (mPlayListCounter >= mPlayListMax) {
                    mPlayListCounter = 0;
                }

                //load video
                loadSequence(mPlayListCounter);
                stopAudio();

                setSyncMode(3); //stop
                message = "s " + to_string(mCurrSyncMode);
                udpSendLeft.Send(message.c_str(), message.length());
                udpSendCenter.Send(message.c_str(), message.length());

                mLoadPlayList = false;
                //syncs 
            }

            //reset comands, add anything for reset videos
            if (initCouter > 24 * 3) {
                ofLog(OF_LOG_NOTICE) << "Reset Play";
                mCommon->commonFrame = 0;
                //
                // send to slaves start video
                //send to the slaves to enable video
                string message = "i " + to_string(mCurrSyncMode);
                udpSendLeft.Send(message.c_str(), message.length());
                udpSendCenter.Send(message.c_str(), message.length());

                //sound reset
                //player.setPaused(false);
                if (player.isLoaded()) {
                    player.setPosition(0.0);
                    player.play();
                }
                else {
                    ofLog(OF_LOG_NOTICE) << "Error loading sound on reset";
                };

                //syncs 
                mCurrSyncMode = 1;
                mStartMS = 0.0;
                setSyncMode(mCurrSyncMode);

                mWaitPeriod = false;
                mAudioDone = false;
                initCouter = 0;
                //reset
                mCommon->commonFrame = 0;
            }
        }
    }

    ///-----------------------------
    //if receiving udp then is a salve
    if (mSlaveUDP) {
        //update audio position
        double audioPos = player.getPosition();
        mMasterAudio.set(audioPos);
        mCommon->audioPos = audioPos;

        //send UDP
        updateUDP();

        //frame update main
        if (mLockFpsUpdate) {
            mCommon->commonFrame++;

            //update frame change 
            if (mDeltaFrame > 2) {
                if (ofGetFrameNum() % 10 == 0) {
                    mCommon->commonFrame++;
                    mDeltaInc = true;
                }
                else {
                    mDeltaInc = false;
                }
            }
            if (mDeltaFrame < -2) {
                if (ofGetFrameNum() % 10 == 0) {
                    mCommon->commonFrame--;
                    mDeltaDec = true;
                }
                else {
                    mDeltaDec = false;
                }
            }

            //reset
            if (mCommon->commonFrame >= mCommon->maxFrames) {
                mCommon->commonFrame = mCommon->maxFrames;

                mLockFpsUpdate = false;
                ofLog(OF_LOG_NOTICE) << "Done Master Frame";
            }

        } //lock fps with audio update
        else if (mLockFpsAudio) {
            mMasterAudio.set(audioPos);
            mCommon->audioPos = audioPos;
        }  
        //lock the fps with udp audio
        else if (mLockFpsUDPAudio) {

        }
    }
    //update osc
   // updateOSC();
}

//--------------------------------------------------------------
void ofApp::draw(){
    drawGui();

}

//--------------------------------------------------------------
void ofApp::sendAudioPosUDP(float audioPos) {
        //update udp
    string msg = "t ";
    msg += to_string(audioPos);
    udpSendLeft.Send(msg.c_str(), msg.length());
    udpSendCenter.Send(msg.c_str(), msg.length());
}
//--------------------------------------------------------------
void ofApp::updateUDP() {

    char udpMessage[10000];
    udpReceiver.Receive(udpMessage, 10000);
    string message = udpMessage;
    if (message != "" && message.size() >=1) {
       // ofLog(OF_LOG_NOTICE) << "GOT UDP MSG : " << message.size() << " " << message;

        //handshake
        if (message[0] == 'h') {
            mHandshakeUDP = true;
        }
        //start
        if (message[0] == 'a') {
            
        }

        //load videos
        if (message[0] == 'l') {

        }

        //play videos
        if (message[0] == 'i') {
            auto smsg = string_split(message);
            if (smsg.size() >= 1) {
                mCurrSyncMode = std::stoi(smsg[1]);
            }
            else {
                mCurrSyncMode = 0;
            }
            setSyncMode(mCurrSyncMode);

            //sound reset
            std::fill(mCommon->mLoadedVideos.begin(), mCommon->mLoadedVideos.end(), true);
            mCommon->commonFrame = 0;
            if (player.isLoaded()) {
                player.setPosition(0.0);
                player.setVolume(0.0);
            }
        }

        //d
        if (message[0] == 'd') {

            auto smsg = string_split(message);
            if (smsg.size() >= 1) {
                mCurrSyncMode = std::stoi(smsg[1]);
            }
            else {
                mCurrSyncMode = 0;
            }
            setSyncMode(mCurrSyncMode);

            //sound reset
            std::fill(mCommon->mLoadedVideos.begin(), mCommon->mLoadedVideos.end(), true);
            mCommon->commonFrame = 0;
            if (player.isLoaded()) {
                player.setPosition(0.0);
                player.setVolume(0.0);
                player.play();
            }

        }

        //send commom frame and time
        if (message[0] == 'f') {
            auto smsg = string_split(message);
            if (smsg.size() >= 1) {
                mCommon->audioPos = std::stof(smsg[1]);
                mMasterAudio.set(mCommon->audioPos);
            }
            mCommon->commonFrame = mCommon->maxFrames * mCommon->audioPos;
        }

        //stop
        if (message[0] == 's') {
            //HPV::InitHPVEngine();

            mCommon->commonFrame = 0;
            if (player.isLoaded()) {
                player.setPosition(0.0);
                player.setVolume(0.0);
            }
           // mLockFpsUpdate = false;
            //send UDP start
            setSyncMode(3); //off
           // player.setPosition(0.0);
           // player.setPaused(true);
        }

        // send time
        if (message[0] == 't') {
            auto smsg = string_split(message);
            if (smsg.size() >= 1) {
                mCommon->audioPos = std::stof(smsg[1]);
                mMasterAudio.set(mCommon->audioPos);
            }
        }

        //load new video
        if (message[0] == 'n') {
            ofLog(OF_LOG_NOTICE) << "Slave new video sequence ";
            auto smsg = string_split(message);
            if (smsg.size() >= 1) {
                mCommon->mSequenceId = std::stof(smsg[1]);
                loadSequence(mCommon->mSequenceId);
            }
        }
        //jump to e
        if(message[0] == 'e') {
            auto smsg = string_split(message);
            if (smsg.size() >= 2) {
                mDeltaFrame = std::stoi(smsg[1]) - mCommon->commonFrame;
               mDeltaSoundTime = std::stof(smsg[2]) - mCommon->audioPos;

            }
        }
    }

}

//--------------------------------------------------------------
void ofApp::setupCommonState() {
    setSyncMode(mCurrSyncMode);
}

//--------------------------------------------------------------
void ofApp::setupUDP() {
    ofLog(OF_LOG_NOTICE) << "Loading UDP info ";
    ofJson js;
    std::string configFile = "network.json";
    ofFile file(configFile);

    if (file.exists()) {
        // ofLog(OF_LOG_NOTICE) << " Reading Config File " << configFile;
        file >> js;

        std::string ip = js["network"]["ip"].get<std::string>();
        int port0 = js["network"]["port0"]; //center
        int port1 = js["network"]["port1"]; //left

        ofLog(OF_LOG_NOTICE) << "Network: " << port0 << "  " << port1 << " " << std::endl;

        if (mCommon->mId == 0) {
            ofLog(OF_LOG_NOTICE) << "Set Master UDP ";
            mMasterUDP = true;
            mSlaveUDP = false;
            mUDPPortCenter = port0; //center
            mUDPPortLeft = port1;  //left
        }else{
            ofLog(OF_LOG_NOTICE) << "Set Slave UDP " << mCommon->mId;
            mMasterUDP = false;
            mSlaveUDP = true;
            //Center
            if (mCommon->mId == 1) {
                mUDPPortReceiver = port0;
            }
            //Left
            if (mCommon->mId == 2) {
                mUDPPortReceiver = port1;
            }
        }
    }
    else {
        ofLog(OF_LOG_NOTICE) << "Cannot Load UDP info ";
    }

    //Reivers
    if (mSlaveUDP) {
        ofLog(OF_LOG_NOTICE) << "Setting Receiver Port:  " << mUDPPortReceiver << std::endl;
        ofxUDPSettings settings;
        settings.receiveOn(mUDPPortReceiver);
        settings.blocking = false;
        udpReceiver.Setup(settings);
    }

    //sender Master
    if (mMasterUDP) {
        ofLog(OF_LOG_NOTICE) << "Setting Master Sender Ports: " << mUDPPortCenter << " " << mUDPPortLeft << std::endl;
        ofxUDPSettings settings;
        settings.sendTo("127.0.0.1", mUDPPortCenter);
        settings.blocking = false;
        udpSendCenter.Setup(settings);

        settings.sendTo("127.0.0.1", mUDPPortLeft);
        settings.blocking = false;
        udpSendLeft.Setup(settings);
    }

}

//--------------------------------------------------------------
void ofApp::setupOSC() {
    mPort = 32000;
    ofLog(OF_LOG_NOTICE) << "Listening for OSC messages on port " << mPort;
    receiver.setup(mPort);
}

//--------------------------------------------------------------
void ofApp::loadAudio(int seqId) {

    //audio path;
    std::string audioPath = "";

    //load audio
    ofLog(OF_LOG_NOTICE) << "Loading sound.." << std::endl;
    audioPath = mAudioPaths.at(seqId);

    std::string audioDataPath = ofToDataPath(audioPath);

    //set the following to true if you want to stream the audio data from the disk on demand instead of
    //reading the whole file into memory. Default is false
    player.load(audioDataPath, true);

    player.setPaused(true);
}
//--------------------------------------------------------------
void ofApp::setupAudioSystem() {

    loadAudio(mCommon->mSequenceId);

    ofxSoundUtils::printOutputSoundDevices();
    auto outDevices = ofxSoundUtils::getOutputSoundDevices();

    int outDeviceIndex = ofxSoundUtils::getOutputSoundDevices().size() - 1;
    ofLog(OF_LOG_NOTICE) << ofxSoundUtils::getSoundDeviceString(outDevices[outDeviceIndex], false, true) << endl;

    // audio setup for testing audio file stream 
    ofSoundStreamSettings soundSettings;
    soundSettings.numInputChannels = 0;
    soundSettings.numOutputChannels = 4;
    soundSettings.bufferSize = 512;// 512;// 256
    soundSettings.numBuffers = 2; //2
    soundSettings.sampleRate = player.getSoundFile().getSampleRate();
    
    //master then  channel audio
    if (mMasterUDP) {
        for (int i = 0; i < outDevices.size(); i++) {
            if (outDevices[i].name == "Focusrite USB ASIO") {
                outDeviceIndex = i;
                break;
            }
            else {
                outDeviceIndex = 0;
            }
        }
    }
    else {
        for (int i = 0; i < outDevices.size(); i++) {
            if (outDevices[i].name == "ASIO4ALL v2") {
                outDeviceIndex = i;
                break;
            }
            else {
                outDeviceIndex = 0;
            }
        }
    }
    
    //
    ofLog(OF_LOG_NOTICE) << "Sound Device Read: " << outDeviceIndex;
    ofLog(OF_LOG_NOTICE) << outDevices[outDeviceIndex].name << std::endl;

    soundSettings.setOutDevice(outDevices[outDeviceIndex]);

    mAudioDevice = soundSettings.getOutDevice()->name;
    mAudioOutputs = soundSettings.getOutDevice()->outputChannels;

    ofLog(OF_LOG_NOTICE) << " Sample Rate Sound File " << std::endl;
    ofLog(OF_LOG_NOTICE) << player.getSoundFile().getSampleRate() << std::endl;
    ofLog(OF_LOG_NOTICE) << player.getSoundFile().getNumChannels() << std::endl;
    ofLog(OF_LOG_NOTICE) << player.getSoundFile().getNumFrames() << std::endl;

    //audio volumen setup
    if (mMasterUDP) {
        player.volume = 0.01
            ;
    }
    else {
        player.volume = 0.0; //.3
    }

    //Configure audio channels
    if (player.isLoaded()) {
        if (mMasterUDP) {
            if (player.getSoundFile().getNumChannels() == 2) {
                ofxSoundObject& playerOutput = output.getOrCreateChannelGroup({ 0, 1});
                player.connectTo(playerOutput);
            }
            if (player.getSoundFile().getNumChannels() == 4) {
                ofxSoundObject& playerOutput = output.getOrCreateChannelGroup({ 0, 1, 2, 3 });
                player.connectTo(playerOutput);
            }
        }
        else {
            ofxSoundObject& playerOutput = output.getOrCreateChannelGroup({ 0, 1});
            player.connectTo(playerOutput);
        }

        // set if you want to either have the player looping (playing over and over again) or not (stop once it reaches the its end).

        player.setLoop(false);
        ofLog(OF_LOG_NOTICE) << "Audio added to player: ";
    }
    else {
        ofLog(OF_LOG_NOTICE) << "Error loading adding player to Device output ";
    }

    stream.setup(soundSettings);
    stream.setOutput(output);

    // if the player is not looping you can register  to the end event, which will get triggered when the player reaches the end of the file
    if (!player.getIsLooping()) {
        playerEndListener = player.endEvent.newListener(this, &ofApp::playerEnded);
    }

    ofLog(OF_LOG_NOTICE) << "Finishing setup Audio";
}

//--------------------------------------------------------------
void ofApp::setupGui(){

    //params
    parameters.setName("Param");
    if (mMasterUDP) {
        parameters.add(mPlayVideos.set("Toggle Play", false));
        parameters.add(mResetVideos.set("Reset Videos", false));
    }
    
    //Master
    parameters.add(mMasterAudio.set("Master Audio", 0, 0, 1.0));
    parameters.add(player.volume);

    //add listeners
    if (mMasterUDP) {
        mPlayVideos.addListener(this, &ofApp::playVideos);
        mResetVideos.addListener(this, &ofApp::resetVideos);

     
        //GUI setup
        mGui.setup(parameters);
        mGui.add(&mChanderlier->ledGroup);
        mGui.setPosition(330, 180);
    }

    mMasterAudio.addListener(this, &ofApp::audioSlider);
    

    
    //default values
    mDrawGUI = true;
    mGui.loadFromFile("settings.xml");
}

//----------------------------------------------------------------
void ofApp::setupChanderlier() {



    if (mMasterUDP) {
        ofLog(OF_LOG_NOTICE) << "Loading Chandelier" << std::endl;

        mChanderlier = Chandelier::create();
        mChanderlier->initSerial(0, 115200);
        mChanderlier->useSerial = true;
        mChanderlier->initVideoEffects(mCommon->mSeqNames);
        mChanderlier->initGui();
    }
    else {

    }
}

//--------------------------------------------------------------
void ofApp::audioSlider(float & value){
    float audioPosition = float(value);
    player.setPosition(value);
}
//--------------------------------------------------------------
void ofApp::resetVideos(bool & value){
    
    ofLog(OF_LOG_NOTICE) << "RESET MOVIE "<<value;
}
//--------------------------------------------------------------
void ofApp::playVideos(bool & value){
  
}

//--------------------------------------------------------------
void ofApp::playerEnded(size_t& id) {
    // This function gets called when the player ends. You can do whatever you need to here.
    // This event happens in the main thread, not in the audio thread.
    ofLog(OF_LOG_NOTICE) << "the player's instance " << id << "finished playing" << endl;
    mAudioDone = true;
}

//--------------------------------------------------------------
void ofApp::drawGui() {
    if (mDrawGUI) {
        ofSetColor(255);
        ofDrawBitmapString("Video fps: " + to_string(ofGetFrameRate()), 10, 10);
        ofDrawBitmapString(to_string(player.getPositionMS()) + "  " + to_string(player.getDurationMS()), 10, 25);
        ofDrawBitmapString("D F: " + to_string(mDeltaFrame) + " D S: " + to_string(mDeltaSoundTime), 10, 40);

        if (mDeltaDec) {
            ofSetColor(255, 255, 0);
            ofDrawBitmapString("Inc: " + to_string(mDeltaInc), 210, 40);
        }
            
        if (mDeltaInc) {
            ofSetColor(0, 255, 255);
            ofDrawBitmapString("Dec: " + to_string(mDeltaDec), 210, 40);
        }
        ofSetColor(255);
        ofDrawBitmapString("L Count: " + to_string(mLoopCount) + " " + to_string(initCouter), 10, 55);
        ofDrawBitmapString("Frame : " + to_string(mCommon->commonFrame) + " " + to_string(mCommon->maxFrames), 10, 70);
        ofDrawBitmapString("PlayLisy : " + to_string(mPlayListCounter), 10, 90);
        ofDrawBitmapString(ofPath, 10, 470);
        ofDrawBitmapString("Current Sequence Name: " + mCommon->mCurrentSeqName, 10, 230);

        int i = 0;
        ofSetColor(200);
        for (auto name : mCommon->mSeqNames) {
            if (i < mPlayListMax) {
                if (mCommon->mCurrentSeqName == name) {
                    ofSetColor(255);
                }
                else {
                    ofSetColor(170);
                }
                ofDrawBitmapString(to_string(i) + " " + name, 20, 250 + i * 20);
            }
            i++;
        }

        if (mMasterUDP) {
            ofSetColor(225, 50, 50);
            ofDrawBitmapString("Master " + to_string(mCommon->mId), 320, 100);
            ofDrawRectangle(355, 120, 30, 30);

            if (mChanderlier->getHandShake()) {
                ofSetColor(0, 200, 220);
                ofDrawRectangle(470, 120, 30, 30);
                ofSetColor(255);
                ofDrawBitmapString("HandShake ", 480, 100);
            }
            ofSetColor(200, 50, 50);
            ofDrawBitmapString("Video: " + mCommon->mVideoType, 320, 15);
            ofDrawBitmapString("Audio: " + mAudioDevice + "O: " + to_string(mAudioOutputs)+ " a c: "+ to_string(player.getSoundFile().getNumChannels()), 320, 30);
            ofDrawBitmapString("Serial: " +mChanderlier->getSerialName() + " "+to_string(mChanderlier->getSerialId())+" "+ to_string(mChanderlier->getBaudRate()), 320, 45);
        }

         ofSetColor(255);
         if (mCurrSyncMode == 0) {
             ofDrawBitmapString("Sync Audio", 320, 70);
         }
         else if (mCurrSyncMode == 1) {
             ofDrawBitmapString("Sync Frame Update", 320, 70);
         }
         else if (mCurrSyncMode == 2) {
             ofDrawBitmapString("Sync Audio UDP", 320, 70);
         }

        if (mSlaveUDP) {
            ofSetColor(255);
            ofDrawBitmapString("Slave " + to_string(mUDPPortReceiver)+"   "+to_string(mCommon->mId), 320, 100);
            ofDrawBitmapString("Audio: " + mAudioDevice + "O: " + to_string(mAudioOutputs), 320, 50);
            if (mHandshakeUDP) {
                ofSetColor(200);
                ofDrawBitmapString("Connected", 320, 130);

            }
        }
        mGui.draw();
    }
}

//first video init
//--------------------------------------------------------------
void ofApp::startMasterVideo() {
    ofLog(OF_LOG_NOTICE) << "send Master";

    string message = "d 1";
    udpSendLeft.Send(message.c_str(), message.length());
    udpSendCenter.Send(message.c_str(), message.length());

    mVideoSyncPlaying = true;

    //reset audio pos
    if (player.isLoaded()) {
        player.setPaused(false);
        player.setPosition(0.0);
        player.play();
    }
   
    mStartMS = 0.0;
    mCommon->commonFrame = 0;

    //start video loop
    mCurrSyncMode = 1;
    setSyncMode(mCurrSyncMode);

    // load new chandelier effects
    mChanderlier->loadVideo(mCommon->mCurrentSeqName);
}

//--------------------------------------------------------------
void ofApp::stopAudio() {
    if (mMasterUDP) {
        ofLog(OF_LOG_NOTICE) << "Send Master Stop";

        //reset audio pos
        player.setPosition(0.0);
        player.setPaused(true);

        setSyncMode(3);

        string message = "s";
        udpSendLeft.Send(message.c_str(), message.length());
        udpSendCenter.Send(message.c_str(), message.length());

        mMasterAudio.set(0.0);
        sendAudioPosUDP(0.0);

    }
}


//--------------------------------------------------------------
void ofApp::loadSequence(int id) {
    if (id < mCommon->mSeqNames.size()) {
        ofLog(OF_LOG_NOTICE) << "Loading new sequence " << id;

        mCommon->mSequenceId = id;
        loadAudio(mCommon->mSequenceId);
        //update names
        mCommon->mCurrentSeqName = mCommon->mSeqNames.at(mCommon->mSequenceId);
        player.volume.setName(mCommon->mCurrentSeqName);

        //send to activate video 
        setSyncMode(mCurrSyncMode);

        std::fill(mCommon->mNewVideos.begin(), mCommon->mNewVideos.end(), true);

        //sned to activate video 1
        //stop load playnew
        if (mMasterUDP) {
            string message = "n " + to_string(id);
            udpSendLeft.Send(message.c_str(), message.length());
            udpSendCenter.Send(message.c_str(), message.length());

            // load new chandelier effects
            mChanderlier->loadVideo(mCommon->mCurrentSeqName);
        }


    }
}
//
// mode 0 -> audio
// mode 1 -> frame
// mode 2 -> audio UDP
// mode 3 -> all off
void ofApp::setSyncMode(int value){
    //syncs 
    if (value == 0) {
        mLockFpsUpdate = false;
        mCommon->mFrameSync = false;

        mLockFpsAudio = true;
        mCommon->mAudioSync = true;

        mLockFpsUDPAudio = false;
    }
    else if (value == 1) {
        mLockFpsUpdate = true;
        mCommon->mFrameSync = true;

        mLockFpsAudio = false;
        mCommon->mAudioSync = false;

        mLockFpsUDPAudio = false;
    }
    else if (value == 2) {
        mLockFpsUpdate = false;
        mCommon->mFrameSync = false;

        mLockFpsAudio = false;
        mCommon->mAudioSync = false;

        mLockFpsUDPAudio = true;
    }
    else if (value == 3) {
        mLockFpsUpdate = false;
        mCommon->mFrameSync = false;

        mLockFpsAudio = false;
        mCommon->mAudioSync = false;

        mLockFpsUDPAudio = false;
    }

}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if (key == '0') {
        loadSequence(0);
    }
    if (key == '1') {
        loadSequence(1);
    }
    if (key == '2') {
        loadSequence(2);
    }
    if (key == '3') {
        //loadSequence(3);
    }
    if (key == '4') {
       // loadSequence(4);
    }
    if (key == '5') {
       // loadSequence(5);
    }
    if (key == '6') {
        //loadSequence(6);
    }
    if (key == '7') {
        //loadSequence(7);
    }
    if (key == '8') {
        //loadSequence(8);
    }
    if (key == '9') {
       // loadSequence(9);
    }

    //audio
    if (key == ' ') {
    }
    

    //move to position 0.99
    if (key == 'e') {
        float pos = 0.99;
        mCommon->commonFrame = mCommon->maxFrames * pos;
        player.setPosition(pos);

        string message = "f 99";
        udpSendLeft.Send(message.c_str(), message.length());
        udpSendCenter.Send(message.c_str(), message.length());

        ofLog(OF_LOG_NOTICE) << "pos  mode";
    }
    if (key == 'g') {
        mDrawGUI = !mDrawGUI;
        ofLog(OF_LOG_NOTICE) << "gui";
    }

    //stop audio
    if (key == 's') {
        stopAudio();
        mVideoSyncPlaying = false;
    }
    
    //send start
    if (key == 'a') {
        if (mMasterUDP) {
            ofLog(OF_LOG_NOTICE) << "send Master";

            mVideoSyncPlaying = true;

            //reset audio pos
            player.setPaused(false);
            player.setPosition(0.0);
            player.play();
            mStartMS = 0.0;

            //start video loop
            mCurrSyncMode = 2;
            setSyncMode(mCurrSyncMode);

            mCommon->commonFrame = 0;

            string message = "d 2";
            udpSendLeft.Send(message.c_str(), message.length());
            udpSendCenter.Send(message.c_str(), message.length());
        }
    }

    //start videos with frame loop sync
    if (key == 'f') {  //frame
        if (mMasterUDP) {
            ofLog(OF_LOG_NOTICE) << "send Master";

            mVideoSyncPlaying = true;

            //reset audio pos
            player.setPaused(false);
            player.setPosition(0.0);
            player.play();
            mStartMS = 0.0;
            mCommon->commonFrame = 0;

            //start video loop
            mCurrSyncMode =  1;
            setSyncMode(mCurrSyncMode);

            string message = "d 1";
            udpSendLeft.Send(message.c_str(), message.length());
            udpSendCenter.Send(message.c_str(), message.length());
        }
    }
    if (key == 'd') {
        if (mMasterUDP) {
            mVideoSyncPlaying = true;
            //sync audio

            mCurrSyncMode = 0;
            setSyncMode(mCurrSyncMode);

            //sound reset
            player.setPaused(false);
            player.setPosition(0.0);
            player.play();
            mStartMS = 0.0;
            mCommon->commonFrame = 0;

            string message = "d 0";
            udpSendLeft.Send(message.c_str(), message.length());
            udpSendCenter.Send(message.c_str(), message.length());
        }

    }
    
    //handshake with video slaves
    if(key == 'h'){

        if (mMasterUDP) {
            string message = "h";
            udpSendLeft.Send(message.c_str(), message.length());
            udpSendCenter.Send(message.c_str(), message.length());
        }
    }
   

    if(key == 'x'){
      //  bool play = true;
     //   playMovies(play);
    }

    if (key == 'r') {
       // bool reset = true;
      //  resetMovies(reset);
    }
    
}
//--------------------------------------------------------------
void ofApp::updateOSC() {
    while (receiver.hasWaitingMessages()) {
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);

        if (m.getAddress() == "/time") {
            float time = m.getArgAsFloat(0);

            player.stop();

            player.setPosition(time);


            ofLog(OF_LOG_NOTICE) << "updated time video: " << time;
        }

        if (m.getAddress() == "/stop") {
            bool play = true;
            playVideos(play);
        }

        if (m.getAddress() == "/play") {
            bool play = false;
            playVideos(play);
        }

        if (m.getAddress() == "/reset") {
            bool reset = true;
            resetVideos(reset);
        }


    }
}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}

//--------------------------------------------------------------
void ofApp::exit(){

    HPV::DestroyHPVEngine();

    player.unload();
    stream.close();
}

//--------------------------------------------------------------
std::vector<std::string> ofApp::string_split(const std::string& str) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    for (std::string s; iss >> s; )
        result.push_back(s);
    return result;
}