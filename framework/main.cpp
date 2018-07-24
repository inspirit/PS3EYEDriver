#include "framework.h"
#include "ps3eye.h"
#include "ps3mic.h"

#define CAM_SX 640
#define CAM_SY 480
#define CAM_FPS 60

#define CAM_GRAYSCALE true

using namespace ps3eye;

static std::string errorString;

static uint8_t imageData[CAM_SX * CAM_SY * 3];

struct MyAudioCallback : PS3EYEMic::AudioCallback
{
	std::vector<float> history;
	
	virtual void handleAudioData(const int16_t * frames, const int numFrames) override
	{
		//printf("received %d frames!\n", numFrames);
		
		for (int i = 0; i < numFrames; ++i)
		{
			const float value = frames[i * 4 + 0] / float(1 << 15);
			
			history.push_back(value);
		}
	}
};

static void testAudioStreaming(PS3EYECam * eye)
{
	PS3EYEMic mic;
	MyAudioCallback audioCallback;
	
	if (mic.init(eye->getDevice(), &audioCallback))
	{
		const auto endTime = SDL_GetTicks() + 10000;
		
		while (SDL_GetTicks() < endTime && !keyboard.wentDown(SDLK_SPACE))
		{
			framework.process();
			
			framework.beginDraw(0, 0, 0, 0);
			{
				if (audioCallback.history.size() < 2)
					continue;
				
				gxScalef(CAM_SX / float(audioCallback.history.size() - 1), CAM_SY / 2.f, 1.f);
				gxTranslatef(0, 1, 0);
				
				setColor(colorWhite);
				hqBegin(HQ_LINES, true);
				for (int i = 0; i + 1 < audioCallback.history.size(); ++i)
				{
					const float value1 = audioCallback.history[i + 0];
					const float value2 = audioCallback.history[i + 1];
					
					hqLine(i + 0, value1, .4f, i + 1, value2, .4f);
				}
				hqEnd();
			}
			framework.endDraw();
		}
		
		mic.shut();
	}
}

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
	
	if (eye != nullptr)
	{
		testAudioStreaming(eye.get());
	}
	
	if (eye != nullptr && !eye->init(CAM_SX, CAM_SY, CAM_FPS,
		CAM_GRAYSCALE
		? PS3EYECam::EOutputFormat::Gray
		: PS3EYECam::EOutputFormat::RGB))
		errorString = "failed to initialize PS3 eye camera";
	else
		eye->start();
	
	if (!errorString.empty())
	{
		logError("error: %s", errorString.c_str());
	}
	
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
			
			if (CAM_GRAYSCALE)
			{
				texture = createTextureFromR8(imageData, CAM_SX, CAM_SY, false, true);
				
				glBindTexture(GL_TEXTURE_2D, texture);
				GLint swizzleMask[4] = { GL_RED, GL_RED, GL_RED, GL_ONE };
				glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			else
			{
				texture = createTextureFromRGB8(imageData, CAM_SX, CAM_SY, false, true);
			}
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
