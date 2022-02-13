#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetDataPathRoot("C:/Users/Bizon/Desktop/App/data");
	path = ofFilePath::getAbsolutePath(ofToDataPath("C:/Users/Bizon/Desktop/App/data/"));
	selectedPath = "";
	loadPath = false;

	fileExtension = "";
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){


	//ofDrawBitmapString(" " + path, 20, 20);

	ofSetColor(255, 200, 50);
	ofDrawBitmapString("Setp 1: Select video folder with key '1'" + selectedName, 20, 10);

	ofSetColor(255);
	ofDrawBitmapString("Selected Name: " + selectedName, 20, 30);

	if (loadPath) {
		ofSetColor(255);
		ofDrawBitmapString("Files: " + selectedPath, 20, 50);
		ofDrawBitmapString("File Extension: " + fileExtension, 20, 65);
		

		ofDirectory dir;
		dir.listDir(selectedPath);
		dir.sort();
		for (int i = 0; i < (int)dir.size(); i++) {
			string path = dir.getPath(i);
			ofFile file(dir.getPath(i));
			string extension = file.getExtension();

			if (extension == "mov") {
				fileExtension = "MOV";
				ofSetColor(0, 255, 200);
			}
			if (extension == "mp4") {
				fileExtension = "MP4";
				ofSetColor(0, 200, 255);
			}
			float x = 50;
			float y = 100 + i * 15;

			ofDrawBitmapString(" " + path, x, y);
		}
	}	

	ofSetColor(255, 200, 50);
	ofDrawBitmapString("Setp 2: Select create videos with key '2' - " + selectedName, 20, 520);


	ofSetColor(255, 200, 50);
	ofDrawBitmapString("Setp 3: Update video.json with " + selectedName, 20, 550);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == '1') {
		ofFileDialogResult openFileResult = ofSystemLoadDialog("Convert Folder", true);

		//Check if the user opened a file
		if (openFileResult.bSuccess) {

			ofLogVerbose("User selected a file");

			//We have a file, check it and process it
			selectedName =  openFileResult.getName();
			selectedPath =  openFileResult.getPath();
			loadPath = true;
		}
		else {
			ofLogVerbose("User hit cancel");
			loadPath = false;
		}

	}

	if (key == '2') {
		std::string  videoString = "";
		if (fileExtension == "MOV") {
			 videoString = path + "/videohpv_mov.sh";
		}
		if (fileExtension == "MP4") {
			videoString = path + "/videohpv_mp4.sh";
		}
		videoString += " "+selectedName;

		std::string  videoLoc = "cd " + path;

		//system("cmd");
		//system(videoLoc.c_str());
		system(videoString.c_str());
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
	mFolderNames.clear();
	if (dragInfo.files.size() > 0) {
		//dragPt = dragInfo.position;

		for (unsigned int k = 0; k < dragInfo.files.size(); k++) {
			mFolderNames.push_back(dragInfo.files[k]);
		}
	}
}
