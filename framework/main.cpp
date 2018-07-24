#include "framework.h"
#include "ps3eye.h"
#include "ps3mic.h"

#define CAM_SX 640
#define CAM_SY 480
#define CAM_FPS 60 // Frames per second. At 320x240 this can go as high as 187 fps.

#define CAM_GRAYSCALE false // If true the camera will output a grayscale image instead of rgb color.

#define DO_SUM true // Calculates and draws the sum of all four microphones. The effect of this is that the summed channel amplifies sounds coming from the front, while reducing ambient noise and sounds coming from the sides. Try it out by making hissing sounds from different directions towards the camera!

#define NUM_CHANNELS (DO_SUM ? 5 : 4) // The number of channels to record. Normally this would be four, but when DO_SUM is enabled, this becomes five.

#define AUDIO_HISTORY_LENGTH .5f // Length of the audio history buffer in seconds.

using namespace ps3eye;

static std::string errorString;

static uint8_t imageData[CAM_SX * CAM_SY * 3];

struct MyAudioCallback : AudioCallback
{
	std::vector<float> histories[NUM_CHANNELS];
	int maxHistoryFrames = 0;
	int numHistoryFrames = 0;
	int firstHistoryFrame = 0;
	
	MyAudioCallback(const float seconds)
	{
		maxHistoryFrames = seconds * PS3EYEMic::kSampleRate;
		
		for (int i = 0; i < NUM_CHANNELS; ++i)
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
			
		#if DO_SUM
			histories[4][firstHistoryFrame] =
				(
					histories[0][firstHistoryFrame] +
					histories[1][firstHistoryFrame] +
					histories[2][firstHistoryFrame] +
					histories[3][firstHistoryFrame]
				) / 4.f;
		#endif
			
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

	for (int c = 0; c < NUM_CHANNELS; ++c)
	{
		auto & history = audioCallback.histories[c];
		
		gxPushMatrix();
		{
			gxTranslatef(0, (c + 1.f) / (NUM_CHANNELS + 1), 0);
			gxScalef(1, 1.f / NUM_CHANNELS, 1);
			
			const Color colors[NUM_CHANNELS] =
			{
				colorRed,
				colorGreen,
				colorBlue,
				colorYellow,
				colorWhite
			};
			
			setColor(colors[c]);
			hqBegin(HQ_LINES, true);
			{
				for (int i = audioCallback.firstHistoryFrame, n = 0; n + 1 < audioCallback.maxHistoryFrames; ++n)
				{
					const int index1 = i;
					const int index2 = i + 1 == audioCallback.maxHistoryFrames ? 0 : i + 1;
					
					const float value1 = history[index1];
					const float value2 = history[index2];
					const float strokeSize = .4f;
					
					hqLine(n + 0, value1, strokeSize, n + 1, value2, strokeSize);
					
					//
					
					i = index2;
				}
			}
			hqEnd();
		}
		gxPopMatrix();
	}
}

#if 0 // todo : remove

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

#endif

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

#if 0 // todo : remove
	if (eye != nullptr)
	{
		testAudioStreaming(eye.get());
	}
#endif
	
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
	
	PS3EYEMic mic;
	MyAudioCallback audioCallback(AUDIO_HISTORY_LENGTH);
	
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
		
		if (eye != nullptr && eye->isStreaming())
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
	
	mic.shut();
	
	if (eye != nullptr)
	{
		eye->stop();
		eye = nullptr;
	}
	
	framework.shutdown();
	
	return 0;
}
