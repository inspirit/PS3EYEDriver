#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"
#include "cinder/Thread.h"

#include "ciUI.h"

#include "ps3eye_driver.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const int ITUR_BT_601_CY = 1220542;
static const int ITUR_BT_601_CUB = 2116026;
static const int ITUR_BT_601_CUG = -409993;
static const int ITUR_BT_601_CVG = -852492;
static const int ITUR_BT_601_CVR = 1673527;
static const int ITUR_BT_601_SHIFT = 20;

static void yuv422_to_rgba(const uint8_t *yuv_src, const int stride, uint8_t *dst, const int width, const int height)
{
    const int bIdx = 0;
    const int uIdx = 0;
    const int yIdx = 0;

    const int uidx = 1 - yIdx + uIdx * 2;
    const int vidx = (2 + uidx) % 4;
    int j, i;

    #define _max(a, b) (((a) > (b)) ? (a) : (b)) 
    #define _saturate(v) static_cast<uint8_t>(static_cast<uint32_t>(v) <= 0xff ? v : v > 0 ? 0xff : 0)

    for (j = 0; j < height; j++, yuv_src += stride)
    {
        uint8_t* row = dst + (width * 4) * j; // 4 channels

        for (i = 0; i < 2 * width; i += 4, row += 8)
        {
            int u = static_cast<int>(yuv_src[i + uidx]) - 128;
            int v = static_cast<int>(yuv_src[i + vidx]) - 128;

            int ruv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVR * v;
            int guv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVG * v + ITUR_BT_601_CUG * u;
            int buv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CUB * u;

            int y00 = _max(0, static_cast<int>(yuv_src[i + yIdx]) - 16) * ITUR_BT_601_CY;
            row[2-bIdx] = _saturate((y00 + ruv) >> ITUR_BT_601_SHIFT);
            row[1]      = _saturate((y00 + guv) >> ITUR_BT_601_SHIFT);
            row[bIdx]   = _saturate((y00 + buv) >> ITUR_BT_601_SHIFT);
            row[3]      = (0xff);

            int y01 = _max(0, static_cast<int>(yuv_src[i + yIdx + 2]) - 16) * ITUR_BT_601_CY;
            row[6-bIdx] = _saturate((y01 + ruv) >> ITUR_BT_601_SHIFT);
            row[5]      = _saturate((y01 + guv) >> ITUR_BT_601_SHIFT);
            row[4+bIdx] = _saturate((y01 + buv) >> ITUR_BT_601_SHIFT);
            row[7]      = (0xff);
        }
    }
}

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

	void eyeUpdateThreadFn();

	ciUICanvas *gui;
    eyeFPS *eyeFpsLab;
    void guiEvent(ciUIEvent *event);

	struct ps3eye_cam *list;
	struct ps3eye_cam *eye_cam;
	int num_eyes;

	bool					mShouldQuit;
	std::thread				mThread;

	gl::Texture mTexture;
	uint8_t *frame_bgra;
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
    int rc;

	mShouldQuit = false;

	num_eyes = ps3eye_list_devices(&list);
	console() << "found " << num_eyes << " cameras" << std::endl;

	mTimer = Timer(true);
	mCamFrameCount = 0;
	mCamFps = 0;
	mCamFpsLastSampleFrame = 0;
	mCamFpsLastSampleTime = 0;

	gui = new ciUICanvas(0,0,320, 480);
    
    float gh = 15;
    float slw = 320 - 20;

    if(num_eyes > 0)
    {
		eye_cam = &list[0];
        rc = ps3eye_init(eye_cam, &ps3eye_modes[1], 60);
        console() << "init eye result " << rc << std::endl;
        rc = ps3eye_start(eye_cam);
        console() << "start eye result " << rc << std::endl;

		frame_bgra = new uint8_t[eye_cam->mode->width*eye_cam->mode->height*4];
		mFrame = Surface(frame_bgra, eye_cam->mode->width, eye_cam->mode->height, eye_cam->mode->width*4, SurfaceChannelOrder::BGRA);
		memset(frame_bgra, 0, eye_cam->mode->width*eye_cam->mode->height*4);
		
		// create and launch the thread
		mThread = thread( bind( &PS3EYECaptureApp::eyeUpdateThreadFn, this ) );
        
        gui->addWidgetDown(new ciUILabel("EYE", CI_UI_FONT_MEDIUM));
        
        eyeFpsLab = new eyeFPS(CI_UI_FONT_MEDIUM);
        gui->addWidgetRight(eyeFpsLab);
        
        // controls
        gui->addWidgetDown(new ciUIToggle(gh, gh, false, "auto gain"));
        gui->addWidgetRight(new ciUIToggle(gh, gh, false, "auto white balance"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 63, eye_cam->gain, "gain"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 63, eye_cam->sharpness, "sharpness"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye_cam->exposure, "exposure"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye_cam->brightness, "brightness"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye_cam->contrast, "contrast"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye_cam->hue, "hue"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye_cam->blueblc, "blue balance"));
        gui->addWidgetDown(new ciUISlider(slw, gh, 0, 255, eye_cam->redblc, "red balance"));
        
        gui->registerUIEvents(this, &PS3EYECaptureApp::guiEvent);
    }
}

void PS3EYECaptureApp::guiEvent(ciUIEvent *event)
{
    string name = event->widget->getName();
    if(name == "auto gain")
    {
        ciUIToggle *t = (ciUIToggle * ) event->widget;
        ps3eye_set_auto_gain(eye_cam, t->getValue() ? 1 : 0);
    }
    else if(name == "auto white balance")
    {
        ciUIToggle *t = (ciUIToggle * ) event->widget;
        ps3eye_set_auto_white_balance(eye_cam, t->getValue() ? 1 : 0);
    }
    else if(name == "gain")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        ps3eye_set_gain(eye_cam, static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "sharpness")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        ps3eye_set_sharpness(eye_cam, static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "exposure")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        ps3eye_set_exposure(eye_cam, static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "brightness")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        ps3eye_set_brightness(eye_cam, static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "contrast")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        ps3eye_set_contrast(eye_cam, static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "hue")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        ps3eye_set_hue(eye_cam, static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "blue balance")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        ps3eye_set_blue_balance(eye_cam, static_cast<uint8_t>(s->getScaledValue()));
    }
    else if(name == "red balance")
    {
        ciUISlider *s = (ciUISlider *) event->widget;
        ps3eye_set_red_balance(eye_cam, static_cast<uint8_t>(s->getScaledValue()));
    }
}

void PS3EYECaptureApp::eyeUpdateThreadFn()
{
	while( !mShouldQuit ) 
	{
		int res = ps3eye_update();
		if(res != 0)
		{
			break;
		}
	}
}

void PS3EYECaptureApp::shutdown()
{
	mShouldQuit = true;
	mThread.join();
	ps3eye_free_devices(&list, num_eyes);
	delete[] frame_bgra;
	delete gui;
}

void PS3EYECaptureApp::mouseDown( MouseEvent event )
{
}

void PS3EYECaptureApp::update()
{
    if(num_eyes)
    {
        const uint8_t* frame;
        int isNewFrame = ps3eye_request_frame(eye_cam, &frame);
        if(isNewFrame)
        {
            yuv422_to_rgba(frame, eye_cam->mode->bytesperline, frame_bgra, mFrame.getWidth(), mFrame.getHeight());
            mTexture = gl::Texture( mFrame );
        }
        mCamFrameCount += isNewFrame;
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
