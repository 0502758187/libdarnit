#include "darner.h"


int main(int argc, char **argv) {
	int i, sfx, sprite;
	void *font, *surface, *handle, *text;
	char test[32];
	DARNER_MOUSE mouse;
	handle = darnerInit("TESTAPP - libDarner");
	
	font = darnerFontLoad(handle, "font.png", 10, 16, 4);
	surface = darnerMenuVerticalCreate(handle, "Hello\nGoodbye\nOther\nNothing\nLess than nothing", 50, 100, font, 200, 10);
//	sfx = darnerSFXLoad(handle, "test.sfx");
//	darnerMusicPlayMod(handle, "latyl-greasy_duck_v1.mod");
	darnerMusicPlayVorbis(handle, "test.ogg");
//	darnerSFXPlay(handle, sfx, 0, 80);

	sprite = darnerSpriteLoad(handle, "test.spr", 0);
	darnerSpriteMove(handle, sprite , 50, 50, 0);
	darnerSpriteAnimationEnable(handle, sprite);
//	darnerRenderOffset(handle, 100, -100);
	text = darnerTextSurfaceAlloc(font, 32, 32, 0, 464);

	for (;;) {
		darnerTextSurfaceReset(text);
		mouse = darnerMouseGet(handle);
		sprintf(test, "X: %i, Y: %i, W: %i", mouse.x, mouse.y, mouse.wheel);
		darnerTextSurfaceStringAppend(text, test);

		darnerRenderBegin();
		darnerTextSurfaceDraw(text);
		darnerSpriteLayerDraw(handle, 0);
		if (darnerMenuHandle(handle, surface) != -1)
			return 0;
		darnerRenderEnd();
		darnerLoop(handle);
	}


	return 0;
}
