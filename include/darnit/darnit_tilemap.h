#ifndef __DARNIT_TILEMAP_H__
#define	__DARNIT_TILEMAP_H__


typedef struct {
	int		w;
	int		h;
	unsigned int	*data;
	void		*render;
} DARNIT_TILEMAP;


DARNIT_TILEMAP *darnitRenderTilemapCreate(void *handle, const char *fname, unsigned int invisibility_divider, void *tilesheet);
void darnitRenderTilemapTileSet(DARNIT_TILEMAP *tilemap, int x, int y, int tile);
void darnitRenderTilemapCameraMove(DARNIT_TILEMAP *tilemap, int cam_x, int cam_y);
void darnitRenderTilemap(void *handle, DARNIT_TILEMAP *tilemap);
DARNIT_TILEMAP *darnitRenderTilemapDelete(DARNIT_TILEMAP *tilemap);

#endif