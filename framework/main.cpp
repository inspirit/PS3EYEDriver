#include "framework.h"
#include "ps3eye.h"

#define CAM_SX 640
#define CAM_SY 480
#define CAM_FPS 60

using namespace ps3eye;

static std::string errorString;

static uint8_t imageData[CAM_SX * CAM_SY * 3];

int main(int argc, char * argv[])
{
	if (!framework.init(0, nullptr, 640, 480))
		return -1;
	
	auto devices = PS3EYECam::getDevices();
	
	logInfo("found %d devices", devices.size());
	
	PS3EYECam::PS3EYERef eye;
	
	if (devices.empty())
		errorString = "no PS3 eye camera found";
	else
		eye = devices[0];
	
	devices.clear();
	
	if (eye != nullptr && !eye->init(CAM_SX, CAM_SY, CAM_FPS, PS3EYECam::EOutputFormat::RGB))
		errorString = "failed to initialize PS3 eye camera";
	else
		eye->start();
	
	GLuint texture = 0;
	
	while (!framework.quitRequested)
	{
		framework.process();
		
		if (keyboard.wentDown(SDLK_ESCAPE))
			framework.quitRequested = true;
		
		if (eye->isStreaming())
		{
			eye->getFrame(imageData);
			
			//
			
			if (texture != 0)
				glDeleteTextures(1, &texture);
			
			texture = createTextureFromRGB8(imageData, CAM_SX, CAM_SY, false, true);
		}
		
		framework.beginDraw(0, 0, 0, 0);
		{
			setBlend(BLEND_OPAQUE);
			
			if (texture != 0)
			{
				setColor(colorWhite);
				gxSetTexture(texture);
				drawRect(0, 0, CAM_SX, CAM_SY);
				gxSetTexture(0);
			}
		}
		framework.endDraw();
	}
	
	if (eye != nullptr)
	{
		eye->stop();
		eye = nullptr;
	}
	
	framework.shutdown();
	
	return 0;
}
