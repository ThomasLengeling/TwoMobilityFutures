#pragma once

#include <cstddef>
#include <stdint.h>

#include "ofMain.h"
#include "ofxHPVPlayer.h"

#include "config.h"

#include "ofxDatGui.h"
#include "ofParameterGroup.h"
#include "ofParameter.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "ofxNetwork.h"
#include "Chandelier.h"

#include "ofxAudioFile.h"
#include "ofxSoundPlayerObject.h"

/*
 
 4K ->3840 x 2160 or 4096 x 2160
 
 Video Card GeForce 2080
 Max Resolution
 7680x4320 -> 2x2 4K
 
 */

#define WIDTH_4K    3840
#define HEIGHT_4K   2160

#define WIDTH_HD    1920
#define HEIGHT_HD   1080

class CommonState;

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
   
    
    //min frame of all the videos;
    int mMinFrame;

    //debug and release mode

    ofxPanel mGui;
    bool mDrawGUI;

    void setupGui();
    void drawGui();

    ofParameterGroup      parameters;     
    ofParameter<bool>     mPlayVideos;
    ofParameter<bool>     mResetVideos;
    ofParameter<bool>     mStopVideos;
    ofParameter<float>    mMasterAudio;
    ofParameter<float>    mGainAudio;
    

    
    void resetVideos(bool & value);
    void playVideos(bool & value);
    void audioSlider(float & value);

    
    bool                    mPause;
    
    std::string             ofPath;

    void setupAudioSystem();
    void loadAudio(int seq);

    ofSoundStream stream;
    ofxSoundOutputMultiplexer output; //
    // these are all subclasses of ofSoundObject
    ofxSoundPlayerObject player;

    std::string mAudioDevice;
    int mAudioOutputs;

    ofEventListener playerEndListener;
    void playerEnded(size_t& id);

    //sound path names
    void addSoundPath(std::string name) {
        mAudioPaths.push_back(name);
    }
    std::vector<std::string> mAudioPaths;

    //load a sequence
    void loadSequence(int id);


    //send recevied UDP
    ofxUDPManager udpReceiver; //receiver

    ofxUDPManager udpSendCenter; //send
    ofxUDPManager udpSendLeft;   //send

    //send = true, receiver = false;
    bool mMasterUDP; //sender or receiver
    bool mSlaveUDP;
    bool   mAudioDone;

    //master send to center and left
    int mUDPPortCenter;
    int mUDPPortLeft;
    bool mHandshakeUDP;

    int mUDPPortReceiver; //udp to send to center or left

    void setupUDP();
    void updateUDP();


    //udp commands
    void sendAudioPosUDP(float audioPos);
    void stopAudio();

    //osc
    int             mPort;
    ofxOscReceiver  receiver;
    void            setupOSC();
    void            updateOSC();

    bool            playNewVideos;

    bool            mInitialize;
    float           mInitTimer;

    //shared memory
    void setupCommonState();
    shared_ptr<CommonState> mCommon;

    //utilities
    std::vector<std::string> string_split(const std::string& str);


    //video clock values
    bool mLoadedVidoes;
    int  initCouter;
    void startMasterVideo();

    bool mVideoSyncPlaying;
    float mStartMS;

    int mLoopCount;
    int mMaxLoopCount;

    int frameCount;

    void setSyncMode(int value);
    bool mLockFpsUpdate;
    bool mLockFpsAudio;
    bool mLockFpsUDPAudio;
    bool mWaitPeriod;
    int mCurrSyncMode;

    int mPlayListCounter;
    int mPlayListMax;
    bool mLoadPlayList;

    int mDeltaFrame;
    bool mDeltaInc;
    bool mDeltaDec;
    float mDeltaSoundTime;

    //Chandelier
    ChandelierRef mChanderlier;
    void setupChanderlier();
    void updateChandelier();

 
};

//---------------------------
//Common class that memory is shared across screens
class CommonState{
public:

    CommonState() {
        mSequenceId = 0;
        audioPos = 0;
        commonFrame = 0;
        mFrameSync = false;
        mAudioSync = true;
    }

   
    int maxFrames;

    //frame current pos
    int commonFrame;

    //video changes 
    double audioPos;


    //sync
    bool mAudioSync;
    bool mFrameSync;

    bool mNewVideo;

    vector<bool> mNewVideos;
    vector<bool> mLoadedVideos;

    //current window to activate

    //window id and name
    std::string mAlias;
    int mId;

    std::string mVideoType;

    int mSequenceId; //current sequence id
    std::string mCurrentSeqName; //sequence name
    std::vector<std::string> mSeqNames;
};