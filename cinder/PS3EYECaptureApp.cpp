#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"

#include "ciUI.h"

#include "ps3eye.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class eyeFPS : public ciUIFPS
{
public:
    eyeFPS(float x, float y, int _size):ciUIFPS(x,y,_size)
    {
    }
    
    eyeFPS(int _size):ciUIFPS(_size)
    {
    }
    
	void update_fps(float fps)
	{
		setLabel("FPS: " + numToString(fps, labelPrecision));
	}
};

class PS3EYECaptureApp : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void shutdown();

	ciUICanvas *gui;
    eyeFPS *eyeFpsLab;
    void guiEvent(ciUIEvent *event);

    ps3eye::PS3EYECam::PS3EYERef eye;

	gl::Texture mTexture;
	uint8_t *frame_bgr;
	Surface mFrame;

	// mesure cam fps
	Timer					mTimer;
	uint32_t				mCamFrameCount;
	float					mCamFps;
	uint32_t				mCamFpsLastSampleFrame;
	double					mCamFpsLastSampleTime;
};

void PS3EYECaptureApp::setup()
{
    using namespace ps3eye;

    // list out the devices
    std::vector<PS3EYECam::PS3EYERef> devices( PS3EYECam::getDevices() );
	console() << "found " << devices.size() << " cameras" << std::endl;

	mTimer = Timer(true);
	mCamFrameCount = 0;
	mCamFps = 0;
	mCamFpsLastSampleFrame = 0;
	mCamFpsLastSampleTime = 0;

	gui = new ciUICanvas(0,0,320, 480);
    
    float gh = 15;
    float slw = 320 - 20;

    if(devices.size())
    {   
        eye = devices.at(0);
        bool res = eye->init(640, 480, 60);
        console() << "init eye result " << res << std::endl;
        eye->start();
        
		frame_bgr = new uint8_t[eye->getWidth()*eye->getHeight()*3];
		mFrame = Surface(frame_bgr, eye->getWidth(), eye->getHeight(), eye->getWidth()*3, SurfaceChannelOrder::BGR);
		memset(frame_bgr, 0, eye->getWidth()*eye->getHeight()*3);
		        
        gui->addWidgetDown(new ciUILabel("EYE", CI_UI_FONT_MEDIUM));
        
        eyeFpsLab = new eyeFPS(CI_UI_FONT_MEDIUM);
        gui->addWidgetRight(eyeFpsLab);
        
        // controls
        gui->addWidgetDown(new ciUIToggle(gh, gh, false, "auto gain"));
        gui->addWidgetRight(new ciUIToggle(gh, gh, false, "auto white balance"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 63, eye->getGain(), "gain"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 63, eye->getSharpness(), "sharpness"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye->getExposure(), "exposure"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye->getBrightness(), "brightness"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye->getContrast(), "contrast"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye->getHue(), "hue"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye->getBlueBalance(), "blue balance"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye->getRedBalance(), "red balance"));
        
        gui->registerUIEvents(this, &PS3EYECaptureApp::guiEvent);
    }
}

void PS3EYECaptureApp::guiEvent(ciUIEvent *event)
{
    string name = event->widget->getName();
    if(name == "auto gain")
    {
        ciUIToggle *t = (ciUIToggle * ) event->widget;
        eye->setAutogain(t->getValue());
    }
    else if(name == "auto white balance")
    {
        ciUIToggle *t = (ciUIToggle * ) event->widget;
        eye->setAutoWhiteBalance(t->getValue());
    }
    else if(name == "gain")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        eye->setGain(static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "sharpness")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        eye->setSharpness(static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "exposure")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        eye->setExposure(static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "brightness")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        eye->setBrightness(static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "contrast")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        eye->setContrast(static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "hue")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        eye->setHue(static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "blue balance")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        eye->setBlueBalance(static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "red balance")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        eye->setRedBalance(static_cast<uint8_t>(s->getScaledValue()));
    }
}

void PS3EYECaptureApp::shutdown()
{
    // You should stop before exiting
    // otherwise the app will keep working
	if (eye)
		eye->stop();
    //
	delete[] frame_bgr;
	delete gui;
}

void PS3EYECaptureApp::mouseDown( MouseEvent event )
{
    
}

void PS3EYECaptureApp::update()
{
    if(eye)
    {
		eye->getFrame(frame_bgr);
        mTexture = gl::Texture( mFrame );

        mCamFrameCount++;
        double now = mTimer.getSeconds();
        if( now > mCamFpsLastSampleTime + 1 ) {
            uint32_t framesPassed = mCamFrameCount - mCamFpsLastSampleFrame;
            mCamFps = (float)(framesPassed / (now - mCamFpsLastSampleTime));

            mCamFpsLastSampleTime = now;
            mCamFpsLastSampleFrame = mCamFrameCount;
        }
    
        gui->update();
        eyeFpsLab->update_fps(mCamFps);
    }
}

void PS3EYECaptureApp::draw()
{
	gl::clear( Color::black() );
	gl::disableDepthRead();	
	gl::disableDepthWrite();		
	gl::enableAlphaBlending();

	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
	if( mTexture ) {
		glPushMatrix();
		gl::draw( mTexture );
		glPopMatrix();
	}
	
	gui->draw();
}

CINDER_APP_BASIC( PS3EYECaptureApp, RendererGl )
