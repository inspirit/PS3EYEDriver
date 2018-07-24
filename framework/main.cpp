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

struct MyAudioCallback : AudioCallback
{
	std::vector<float> histories[4];
	int maxHistoryFrames = 0;
	int numHistoryFrames = 0;
	int firstHistoryFrame = 0;
	
	MyAudioCallback(const float seconds)
	{
		maxHistoryFrames = seconds * PS3EYEMic::kSampleRate;
		
		for (int i = 0; i < 4; ++i)
		{
			histories[i].resize(maxHistoryFrames, 0.f);
		}
	}
	
	virtual void handleAudioData(const AudioFrame * frames, const int numFrames) override
	{
		//printf("received %d frames!\n", numFrames);
		
		for (int i = 0; i < numFrames; ++i)
		{
			for (int c = 0; c < 4; ++c)
			{
				auto & history = histories[c];
				
				const float value = frames[i].channel[c] / float(1 << 15);
				
				history[firstHistoryFrame] = value;
			}
			
			if (numHistoryFrames < maxHistoryFrames)
				numHistoryFrames++;
			
			firstHistoryFrame++;
			
			if (firstHistoryFrame == maxHistoryFrames)
				firstHistoryFrame = 0;
		}
	}
};

static void drawAudioHistory(const MyAudioCallback & audioCallback)
{
	gxScalef(CAM_SX / float(audioCallback.maxHistoryFrames - 1), CAM_SY, 1.f);

	for (int c = 0; c < 4; ++c)
	{
		auto & history = audioCallback.histories[c];
		
		gxPushMatrix();
		{
			gxTranslatef(0, (c + 1) / 5.f, 0);
			gxScalef(1, 1 / 4.f, 1);
			
			const Color colors[4] =
			{
				colorRed,
				colorGreen,
				colorBlue,
				colorYellow
			};
			
			setColor(colors[c]);
			hqBegin(HQ_LINES, true);
			{
				for (int i = audioCallback.firstHistoryFrame, n = 0; n + 1 < audioCallback.maxHistoryFrames; ++n)
				{
					const float value1 = history[i + 0];
					const float value2 = history[i + 1];
					const float strokeSize = .4f;
					
					hqLine(n + 0, value1, strokeSize, n + 1, value2, strokeSize);
					
					//
					
					i++;
					
					if (i == audioCallback.maxHistoryFrames)
						i = 0;
				}
			}
			hqEnd();
		}
		gxPopMatrix();
	}
}

static void testAudioStreaming(PS3EYECam * eye)
{
	const int testDuration = 10; // seconds
	const int historySize = 1; // seconds
	
	PS3EYEMic mic;
	MyAudioCallback audioCallback(historySize);
	
	if (mic.init(eye->getDevice(), &audioCallback))
	{
		const auto endTime = SDL_GetTicks() + testDuration * 1000;
		
		while (SDL_GetTicks() < endTime && !keyboard.wentDown(SDLK_SPACE))
		{
			framework.process();
			
			framework.beginDraw(0, 0, 0, 0);
			{
				drawAudioHistory(audioCallback);
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
		//testAudioStreaming(eye.get());
	}
	
	if (eye != nullptr && !eye->init(CAM_SX, CAM_SY, CAM_FPS,
		CAM_GRAYSCALE
		? PS3EYECam::EOutputFormat::Gray
		: PS3EYECam::EOutputFormat::RGB))
		errorString = "failed to initialize PS3 eye camera";
	else
		eye->start();
	
	PS3EYEMic mic;
	MyAudioCallback audioCallback(.5f);
	
	if (eye != nullptr)
	{
		mic.init(eye->getDevice(), &audioCallback);
	}
	
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
			if (texture != 0)
			{
				pushBlend(BLEND_OPAQUE);
				{
					setColor(colorWhite);
					gxSetTexture(texture);
					drawRect(0, 0, CAM_SX, CAM_SY);
					gxSetTexture(0);
				}
				popBlend();
			}
			
			drawAudioHistory(audioCallback);
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
