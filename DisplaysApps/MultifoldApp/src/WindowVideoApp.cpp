#include "WindowVideoApp.h"


void WindowVideoApp::setVideoName(std::string videoName) {
	mVideoName = videoName;
}

void WindowVideoApp::setup() {

	//ofBackground(mBkgColor);
	//ofSetFullscreen(true);
	ofSetFrameRate(24);
	ofDisableSmoothing();
	ofSetBackgroundAuto(false);
	ofSetVerticalSync(false);

	//load Videos
	inc = 0;
	mLoadVideo = false;

	//ofSetCircleResolution(100);

		//start HPV engine
	ofLog(OF_LOG_NOTICE) << "Video VW :  "  << mId;
	ofLog(OF_LOG_NOTICE) << "Set id :  "<< mCommon->mId << std::endl;

		

	//0 -> HAP
	//1 -> HPV
	//2 -> MOV
	//std::string tmpName = "videos/Sequence_05_1.mov";

	int vtype = 1;

	if(!mVideoSets.empty()){
		mCurrentSetId = 0;
		if (mCommon->mVideoType == "hpv") {
			vtype = 1;
		}
		mVideoPlayer = inn::VideoPlayers::create(vtype, 0);
		mVideoPlayer->loadVideo(mVideoSets[mCurrentSetId]);
		mCommon->maxFrames = mVideoPlayer->getTotalNumFrames();
	}
	else {
		ofLog(OF_LOG_NOTICE) << "Video Set is empty " << mId;
	}

	loadDebugImg();

	ofLog(OF_LOG_NOTICE) << "Done Loading VW"<< mId;
}

void WindowVideoApp::update() {

	//Sync video with frame count
	if (mCommon->mFrameSync) {
		mVideoPlayer->updateFrame(mCommon->commonFrame);
	}
	//Sync video with audio count
	else if (mCommon->mAudioSync) {
		mVideoPlayer->setPosition(mCommon->audioPos);
	}

	//if the videos have not been initilized yet then init
	if(mCommon->mNewVideos[mId]){
		mCurrentSetId = mCommon->mSequenceId;
		if (mCurrentSetId < mVideoSets.size()) {
			mVideoPlayer->close();
			mVideoPlayer->loadVideo(mVideoSets[mCurrentSetId]);
			mCommon->maxFrames = mVideoPlayer->getTotalNumFrames();
			mCommon->mNewVideos[mId] = false;
			mCommon->maxFrames = mVideoPlayer->getTotalNumFrames();
			ofLog(OF_LOG_NOTICE) << "Loaded Init Video " << mId;
		}
	}

	

	//update sync
	mVideoPlayer->update();
	
	///---?
	HPV::Update();
}

//--------------------------------------------------------------
void WindowVideoApp::loadDebugImg() {
	if (mDebugImg.load("imgs/display_4K_"+to_string(mId + mCommon->mId*3 + 1)+".png")) {
		ofLog(OF_LOG_NOTICE) << "Loaded Debug Img";
	}
	else {
		ofLog(OF_LOG_NOTICE) << "Error Debug Img";
	}
}

//--------------------------------------------------------------
void WindowVideoApp::draw() {

	//draw the debug img
	if (!mCommon->mLoadedVideos[mId]) {
		mDebugImg.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	}
	else {

		ofSetColor(255);
		mVideoPlayer->draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	}

}

//--------------------------------------------------------------
void WindowVideoApp::setBackground(ofColor bkg) {
	mBkgColor = bkg;
}

//--------------------------------------------------------------
void WindowVideoApp::exit() {
	
}