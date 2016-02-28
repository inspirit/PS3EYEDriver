#pragma once

#include "ofMain.h"
#include "ps3eye.h"

class testApp : public ofBaseApp{

	public:
		void setup();
        void exit();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        ps3eye::PS3EYECam::PS3EYERef eye;
    
        int camFrameCount;
        int camFpsLastSampleFrame;
        float camFpsLastSampleTime;
        float camFps;
        
        unsigned char * videoFrame;
        ofTexture videoTexture;
		
};
