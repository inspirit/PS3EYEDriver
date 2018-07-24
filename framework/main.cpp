#include "framework.h"
#include "ps3eye.h"

int main(int argc, char * argv[])
{
	if (!framework.init(0, nullptr, 640, 480))
		return -1;
	
	while (!framework.quitRequested)
	{
		framework.process();
		
		if (keyboard.wentDown(SDLK_ESCAPE))
			framework.quitRequested = true;
		
		framework.beginDraw(0, 0, 0, 0);
		{
		
		}
		framework.endDraw();
	}
	
	framework.shutdown();
	
	return 0;
}
