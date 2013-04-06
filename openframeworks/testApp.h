#pragma once

#include "ofMain.h"
#include "ps3eye_driver.h"

class ps3eyeUpdate : public ofThread{
    
public:
    ps3eyeUpdate(){
    }
    
    void start(){
        startThread(false, false);   // blocking, verbose
    }
    
    void stop(){
        stopThread();
    }
    
    //--------------------------
    void threadedFunction()
    {
        while( isThreadRunning() != 0 )
        {
            int res = ps3eye_update();
            if(res != 0)
            {
                break;
            }
        }
    }
};

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
    
        struct ps3eye_cam *list;
        struct ps3eye_cam *eye_cam;
        int num_eyes;
    
        int camFrameCount;
        int camFpsLastSampleFrame;
        float camFpsLastSampleTime;
        float camFps;
    
        ps3eyeUpdate threadUpdate;
    
        unsigned char * videoFrame;
        ofTexture videoTexture;
		
};
