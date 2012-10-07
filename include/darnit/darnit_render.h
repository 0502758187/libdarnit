#ifndef __DARNIT_RENDER_H__
#define	__DARNIT_RENDER_H__

typedef void DARNIT_TILESHEET;
typedef void DARNIT_TILE;


DARNIT_TILESHEET *darnitRenderTilesheetLoad(const char *fname, unsigned int wsq, unsigned int hsq, unsigned int target_format);
DARNIT_TILESHEET *darnitRenderTilesheetNew(int tiles_w, int tiles_h, int tile_w, int tile_h, unsigned int format);
void darnitRenderTilesheetUpdate(DARNIT_TILESHEET *tilesheet, int sheet_x, int sheet_y, int change_w, int change_h, void *data);
DARNIT_TILESHEET *darnitRenderTilesheetFree(DARNIT_TILESHEET *tilesheet);
void darnitRenderTileMove(DARNIT_TILE *tile_p, unsigned int tile, void *tilesheet, unsigned int x, unsigned int y);
void darnitRenderTileSetTilesheetCoord(DARNIT_TILE *tile_p, unsigned int tile, DARNIT_TILESHEET *tilesheet, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void darnitRenderTileSet(DARNIT_TILE *tile_p, unsigned int tile, DARNIT_TILESHEET *tilesheet, unsigned int tile_ts);
void darnitRenderTileClear(DARNIT_TILE *tile_p, unsigned int tile);
void darnitRenderTileDraw(DARNIT_TILE *tile_p, void *tilesheet, unsigned int tiles);
DARNIT_TILE *darnitRenderTileAlloc(unsigned int tiles);
DARNIT_TILE *darnitRenderTileFree(DARNIT_TILE *tile_p);
void darnitRenderBegin();
void darnitRenderEnd();
void darnitRenderBlendingEnable();
void darnitRenderBlendingDisable();
void darnitRenderTint(float r, float g, float b, float a);
void darnitRenderClearColorSet(unsigned char r, unsigned char g, unsigned char b);
void darnitRenderOffset(int x, int y);

void darnitRenderFadeIn(unsigned int time, float r, float g, float b);
void darnitRenderFadeOut(unsigned int time);
int darnitRenderFadeChanging();
void darnitRenderStateRestore();

int darnitRenderTilesheetAnimationApply(DARNIT_TILESHEET *tilesheet, const char *fname);
void darnitRenderTilesheetAnimationAnimate(DARNIT_TILESHEET *tilesheet);

#endif
