/*
Copyright (c) 2011-2013 Steven Arnow
'render_tilemap.c' - This file is part of libdarnit

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source
	distribution.
*/


#include "darnit.h"

#ifndef DARNIT_HEADLESS

void renderTilemapCalcPosMap(TILE_CACHE *cache, TILESHEET *ts, int x, int y, int w, int h) {
	float x_start, y_start;
	float *x_adv_buf, *y_adv_buf;
	int i, j, k;
	

	/* Because of the load/store delay, interleaving makes more efficient use of the cycles */
	x_start = x;
	y_start = y;
	x_adv_buf = malloc(sizeof(float) * (w + 1));
	x_start *= ts->sw;
	y_start *= ts->sh;
	y_adv_buf = malloc(sizeof(float) * (h + 1));
	x_start = -1.0f;
	y_start = 1.0f;


	for (i = 0; i < (w + 1); i++) {
		x_adv_buf[i] = x_start + ts->sw*(float)i;
	}
	for (i = 0; i < (h + 1); i++) {
		y_adv_buf[i] = y_start - ts->sh*(float)i;
	}
	
	for (i = k = 0; i < w; i++) {
		for (j = 0; j < h; j++, k++) {
			/* I'll keep it like this for now. I need to re-do this later anyway. FIXME */
			cache[k].vertex[0].coord.x = x_adv_buf[i];
			cache[k].vertex[0].coord.y = y_adv_buf[j];
			cache[k].vertex[1].coord.x = x_adv_buf[i+1];
			cache[k].vertex[1].coord.y = y_adv_buf[j];
			cache[k].vertex[2].coord.x = x_adv_buf[i+1];
			cache[k].vertex[2].coord.y = y_adv_buf[j+1];
			cache[k].vertex[3].coord.x = x_adv_buf[i+1];
			cache[k].vertex[3].coord.y = y_adv_buf[j+1];
			cache[k].vertex[4].coord.x = x_adv_buf[i];
			cache[k].vertex[4].coord.y = y_adv_buf[j+1];
			cache[k].vertex[5].coord.x = x_adv_buf[i];
			cache[k].vertex[5].coord.y = y_adv_buf[j];
		}
	}

	free(x_adv_buf);
	free(y_adv_buf);

	return;
}


void renderTilemapCalcMap(TILE_CACHE *cache, TILESHEET *ts, int x, int y, int w, int h, int map_w, int map_h, unsigned int *tilemap, int inv_div, unsigned int mask) {
	int i, j, k, l;
	int x_cur, y_cur, blank, blank_y;
	int t;
	float *nullbuf;

	x_cur = x;
	for (i = k = 0; i < w; i++, x_cur++) {
		y_cur = y;
		if (x_cur < 0 || x_cur >= map_w)
			blank = 1;
		else
			blank = 0;
		for (j = 0, blank_y = 0; j < h; j++, k++, y_cur++, blank_y = 0) {
			if (y_cur >= 0 && y_cur < map_h && blank == 0)
				t = tilemap[y_cur*map_w + x_cur] & mask;
			else
				blank_y = 1;

			if (blank_y == 0) {
				if (inv_div == 0);
				else if (!(t % inv_div))
					blank_y = 1;
			} 
			
			if (blank_y) {
				nullbuf = (float *) &cache[k];
				for (l = 0; l < 24; l++)
					nullbuf[l] = -1.0f;
			} else {
				/* Ugh. I need to re-do this too I think. Maybe. */
				cache[k].vertex[0].tex.u = ts->tile[t].r;
				cache[k].vertex[0].tex.v = ts->tile[t].s;
				cache[k].vertex[1].tex.u = ts->tile[t].u;
				cache[k].vertex[1].tex.v = ts->tile[t].s;
				cache[k].vertex[2].tex.u = ts->tile[t].u;
				cache[k].vertex[2].tex.v = ts->tile[t].v;
				cache[k].vertex[3].tex.u = ts->tile[t].u;
				cache[k].vertex[3].tex.v = ts->tile[t].v;
				cache[k].vertex[4].tex.u = ts->tile[t].r;
				cache[k].vertex[4].tex.v = ts->tile[t].v;
				cache[k].vertex[5].tex.u = ts->tile[t].r;
				cache[k].vertex[5].tex.v = ts->tile[t].s;
			}
		}
	}

	return;
}



void renderTilemapCameraMove(RENDER_TILEMAP *tm, int cam_x, int cam_y) {
	int x, y, w, h, map_w, map_h;
	
	tm->cam_x = tm->ts->swgran * (cam_x % tm->ts->wsq) * -1;
	tm->cam_y = tm->ts->shgran * (cam_y % tm->ts->hsq);

	x = floorf((float) cam_x / tm->ts->wsq);
	y = floorf((float) cam_y / tm->ts->hsq);
	
	if (x == tm->cam_xi && y == tm->cam_yi)
		return;

	w = tm->w;
	h = tm->h;
	map_w = tm->map_w;
	map_h = tm->map_h;

	renderTilemapCalcPosMap(tm->cache, tm->ts, x, y, w, h);
	renderTilemapCalcMap(tm->cache, tm->ts, x, y, w, h, map_w, map_h, tm->map, tm->inv_div, tm->mask);

	tm->cam_xi = x;
	tm->cam_yi = y;

	return;
}


void *renderTilemapCreate(unsigned int w, unsigned int h, unsigned int *map, int camera_x, int camera_y, unsigned int inv_div, TILESHEET *ts, unsigned int mask) {
	RENDER_TILEMAP *tm;
	
	if ((tm = malloc(sizeof(RENDER_TILEMAP))) == NULL) {
		return NULL;
	}

	tm->map_w = w;
	tm->map_h = h;
	tm->cam_xi = ~0;
	tm->cam_yi = ~0;
	tm->map = map;
	tm->inv_div = inv_div;
	tm->ts = ts;
	tm->w = d->video.w / ts->wsq + 2;
	tm->h = d->video.h / ts->hsq + 2;
	tm->mask = mask;

	tm->cache = malloc(sizeof(TILE_CACHE) * tm->w * tm->h);
	renderTilemapCameraMove(tm, camera_x, camera_y);

	return tm;
}


void renderTilemapRender(RENDER_TILEMAP *tm) {
	glLoadIdentity();
	glTranslatef(tm->cam_x, tm->cam_y, 0);
	renderCache(tm->cache, tm->ts, tm->w * tm->h);
	glLoadIdentity();
	glTranslatef(d->video.swgran * d->video.offset_x, d->video.shgran * d->video.offset_y, 0);

	return;
}


void renderTilemapFree(RENDER_TILEMAP *tm) {
	free(tm->cache);
	free(tm);

	return;
}


void renderTilemapTileSet(RENDER_TILEMAP *tm, int x, int y, int tile) {
	int ci;

	if (x >= tm->map_w || x < 0 || y >= tm->map_h || y < 0)
		return;
	
	tm->map[tm->map_w * y + x] = tile;

	x -= tm->cam_xi;
	y -= tm->cam_yi;

	if (x < 0 || x >= tm->w || y < 0 || y >= tm->h)
		return;
	
	ci = x * tm->h + y;	/* For some reason, I made the cache indexing in cols instead of rows */

	renderCalcTileCache(&tm->cache[ci], tm->ts, tm->map[tm->map_w * y + x] & tm->mask);
	renderCalcTilePosCache(&tm->cache[ci], tm->ts, x * tm->ts->wsq, y * tm->ts->hsq);

	return;
}


void renderTilemapForceRecalc(RENDER_TILEMAP *tm) {
	if (tm == NULL) return;

	renderTilemapCalcPosMap(tm->cache, tm->ts, tm->cam_xi, tm->cam_yi, tm->w, tm->h);
	renderTilemapCalcMap(tm->cache, tm->ts, tm->cam_xi, tm->cam_yi, tm->w, tm->h, tm->map_w, tm->map_h, tm->map, tm->inv_div, tm->mask);

	return;
}


// DARNIT_HEADLESS
#endif
