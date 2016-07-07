#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    using namespace ps3eye;
    
    camFrameCount = 0;
    camFpsLastSampleFrame = 0;
    camFpsLastSampleTime = 0;
    camFps = 0;
    
    // list out the devices
    std::vector<PS3EYECam::PS3EYERef> devices( PS3EYECam::getDevices() );
    
    if(devices.size())
    {
        eye = devices.at(0);
        bool res = eye->init(640, 480, 60);
        eye->start();
        
        videoFrame 	= new unsigned char[eye->getWidth()*eye->getHeight()*3];
        videoTexture.allocate(eye->getWidth(), eye->getHeight(), GL_BGR_EXT);
    }
}
void testApp::exit(){
    // You should stop before exiting
    // otherwise the app will keep working
    if(eye) eye->stop();
	delete[] videoFrame;
}

//--------------------------------------------------------------
void testApp::update()
{
    if(eye)
    {
		eye->getFrame(videoFrame);
        videoTexture.loadData(videoFrame, eye->getWidth(),eye->getHeight(), GL_BGR_EXT);

        camFrameCount++;
        float timeNow = ofGetElapsedTimeMillis();
        if( timeNow > camFpsLastSampleTime + 1000 ) {
            uint32_t framesPassed = camFrameCount - camFpsLastSampleFrame;
            camFps = (float)(framesPassed / ((timeNow - camFpsLastSampleTime)*0.001f));
            
            camFpsLastSampleTime = timeNow;
            camFpsLastSampleFrame = camFrameCount;
        }
    }
    
}

//--------------------------------------------------------------
void testApp::draw()
{
    ofSetHexColor(0xffffff);
    videoTexture.draw(0,0,eye->getWidth(),eye->getHeight());
    
    string str = "app fps: ";
	str += ofToString(ofGetFrameRate(), 2);
    str += "\neye fps: " + ofToString(camFps, 2);
    ofDrawBitmapString(str, 10, 15);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}