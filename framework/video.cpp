#include "framework.h"
#include "ps3eye.h"

#define CAM_SX 640
#define CAM_SY 480
#define CAM_FPS 60 // Frames per second. At 320x240 this can go as high as 187 fps.

#define CAM_GRAYSCALE false // If true the camera will output a grayscale image instead of rgb color.

using namespace ps3eye;

static std::string errorString;

static uint8_t imageData[CAM_SX * CAM_SY * 3];

int main(int argc, char * argv[])
{
	if (!framework.init(640, 480))
		return -1;
	
	auto devices = PS3EYECam::getDevices();
	
	logInfo("found %d devices", devices.size());
	
	PS3EYECam::PS3EYERef eye;
	
	if (devices.empty())
		errorString = "no PS3 eye camera found";
	else
		eye = devices[0];
	
	devices.clear();
	
	if (eye != nullptr)
	{
		if (!eye->init(CAM_SX, CAM_SY, CAM_FPS,
			CAM_GRAYSCALE
			? PS3EYECam::EOutputFormat::Gray
			: PS3EYECam::EOutputFormat::RGB))
		{
			errorString = "failed to initialize PS3 eye camera";
		}
		else
		{
			eye->start();
		}
	}
	
	if (!errorString.empty())
	{
		logError("error: %s", errorString.c_str());
	}
	
	GxTexture texture;
	
	bool stressTest = false;
	
	while (!framework.quitRequested)
	{
		framework.process();
		
		if (keyboard.wentDown(SDLK_ESCAPE))
			framework.quitRequested = true;
		
		if (keyboard.isDown(SDLK_LSHIFT) && keyboard.wentDown(SDLK_t))
			stressTest = !stressTest;
		
		if (eye != nullptr && eye->isStreaming())
		{
			eye->getFrame(imageData);
			
			//
			
			if (CAM_GRAYSCALE)
			{
				texture.allocate(CAM_SX, CAM_SY, GX_R8_UNORM, false, true);
				texture.upload(imageData, 1, 0);
				texture.setSwizzle(0, 0, 0, GX_SWIZZLE_ONE);
			}
			else
			{
				texture.allocate(CAM_SX, CAM_SY, GX_RGB8_UNORM, false, true);
				texture.upload(imageData, 1, 0);
			}
		}
		
		framework.beginDraw(0, 0, 0, 0);
		{
			if (texture.isValid())
			{
				pushBlend(BLEND_OPAQUE);
				{
					setColor(colorWhite);
					gxSetTexture(texture.id);
					drawRect(0, 0, CAM_SX, CAM_SY);
					gxSetTexture(0);
				}
				popBlend();
			}
			
			hqBegin(HQ_FILLED_CIRCLES);
			setColor((eye != nullptr && eye->isStreaming()) ? colorGreen : colorRed);
			hqFillCircle(14, 14, 10);
			hqEnd();
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
