/*
Copyright (c) 2011-2013 Steven Arnow
'mt_sprite.c' - This file is part of libdarnit

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

void mtSpriteCalcCacheTile(TILESHEET *ts, TILE_CACHE *cache, int x, int y, int w, int h, int rx, int ry) {
	renderCalcTilePosCache(cache, ts, rx, ry, 0);
	renderSetTileCoord(cache, ts, x, y, w, h);

	return;
}


void mtSpriteSetAsFrame(MTSPRITE_ENTRY *spr, int time) {
	MTSPRITE_FRAME *tmp;
	if (spr == NULL)
		return;
	
	if ((tmp = realloc(spr->frame, sizeof(MTSPRITE_FRAME) * spr->frames + 1)) == NULL)
		return;
	spr->frame = tmp;
	spr->frame[spr->frames].cache = &spr->cache[spr->tiles - 1];
	spr->frame[spr->frames].tiles = 1;
	spr->frame[spr->frames].time = time;
	
	spr->frames++;

	return;
}


void mtSpriteAddTile(MTSPRITE_ENTRY *spr, int x, int y, int w, int h, int rx, int ry) {
	TILE_CACHE *tmp;
	if (spr == NULL)
		return;
	
	if ((tmp = realloc(spr->cache, sizeof(TILE_CACHE) * spr->tiles + 1)) == NULL)
		return;
	spr->cache = tmp;
	mtSpriteCalcCacheTile(spr->ts, &spr->cache[spr->tiles], x, y, w, h, rx, ry);

	spr->tiles++;
	
	return;
}


void *mtSpriteNew(int tiles, int frames, void *ts) {
	MTSPRITE_ENTRY *spr;

	if ((spr = malloc(sizeof(MTSPRITE_ENTRY))) == NULL)
		return NULL;
	if ((spr->cache = malloc(sizeof(TILE_CACHE) * tiles)) == NULL) {
		free(spr);
		return NULL;
	}
	
	if ((spr->frame = malloc(sizeof(MTSPRITE_FRAME) * frames)) == NULL) {
		free(spr->cache);
		free(spr);
		return NULL;
	}

	spr->frames = frames;
	spr->tiles = tiles;
	spr->cur_frame = 0;
	spr->time_left = 0;
	spr->time_last = 0;
	spr->animate = 0;
	spr->ts = ts;

	return spr;
}


void *mtSpriteLoad(const char *fname) {
	MTSPRITE_ENTRY *spr;
	FILESYSTEM_FILE *fp;
	int frames, loctiles, tiles, x, y, w, h, t, rx, ry;
	char c, buff[512];
	void *ts;

	if ((fp = fsFileOpen(fname, "rb")) == NULL) {
		fprintf(stderr, "libDarnit: Unable to open mt-sprite %s\n", fname);
		return NULL;
	}
	
	ts = NULL;
	frames = tiles = 0;
	while (!fsFileEOF(fp)) {
		fsFileRead((void *) &c, 1, fp);
		switch (c) {
			case 'F':
				fsFileGets(buff, 512, fp);
				frames++;
				break;
			case 'T':
				fsFileGets(buff, 512, fp);
				tiles++;
				break;
			case 'R':	/* ONE resource per file, and it must be the first thing in the file */
				fsFileSkipWhitespace(fp);
				fsFileGets(buff, 512, fp);
				if (buff[strlen(buff) - 1] == '\n')
					buff[strlen(buff) - 1] = 0;
				ts = renderTilesheetLoad(buff, 32, 32, PFORMAT_RGB5A1);
				break; 
			case '\n':
				break;
			default:
				fsFileGets(buff, 512, fp);
				break;
		}
	}

	if ((spr = mtSpriteNew(tiles, frames, ts)) == NULL) {
		fsFileClose(fp);
		free(spr->cache);
		free(spr);
		return NULL;
	}

	fsFileSeek(fp, 0, SEEK_SET);

	/* I know, two-pass parsing is kinda paper-tape era-ish. But I don't know what else to do D: */
	frames = loctiles = tiles = 0;
	while (!fsFileEOF(fp)) {
		fsFileRead(&c, 1, fp);
		switch (c) {
			case 'F':
				fsFileGets(buff, 512, fp);
				t = atoi(buff);
				if (frames != 0)
					spr->frame[frames-1].tiles = loctiles;
				loctiles = 0;
				spr->frame[frames].cache = &spr->cache[tiles];
				spr->frame[frames].time = t;
				frames++;
				break;
			case 'T':
				fsFileGets(buff, 512, fp);
				sscanf(buff, "%i %i %i %i %i %i\n", &x, &y, &w, &h, &rx, &ry);
				mtSpriteCalcCacheTile(spr->ts, &spr->cache[tiles], x, y, w, h, rx, ry);
				tiles++;
				loctiles++;
				break;
			case '\n':
				break;
			default:
				fsFileGets(buff, 512, fp);
				break;
		}
	}

	spr->frame[frames-1].tiles = loctiles;
	spr->time_left = spr->frame->time;
	spr->repeat = 1;

	fsFileClose(fp);

	return spr;
}


void mtSpriteAnimate(MTSPRITE_ENTRY *spr) {
	if (spr == NULL) return;

	int time;

	if (spr->animate == 0)
		return;
	
	time = tpw_ticks();
	time -= spr->time_last;

	spr->time_left -= time;

	while (spr->time_left <= 0) {
		spr->cur_frame++;
		if (spr->cur_frame >= spr->frames)
			spr->cur_frame = (spr->repeat) ? 0 : spr->cur_frame - 1;
		spr->time_left += spr->frame[spr->cur_frame].time;
	}

	spr->time_last = tpw_ticks();
	
	return;
}


void mtSpriteDraw(MTSPRITE_ENTRY *spr) {
	if (spr == NULL) return;

	mtSpriteAnimate(spr);
	renderCache(spr->frame[spr->cur_frame].cache, spr->ts, spr->frame[spr->cur_frame].tiles);

	return;
}


void mtSpriteEnableAnimation(MTSPRITE_ENTRY *spr) {
	if (spr == NULL) return;

	spr->time_last = tpw_ticks();
	spr->animate = 1;

	return;
}


void mtSpritePauseAnimation(MTSPRITE_ENTRY *spr) {
	if (spr == NULL) return;

	spr->animate = 0;

	return;
}


void mtSpriteDisableAnimation(MTSPRITE_ENTRY *spr) {
	if (spr == NULL) return;

	spr->animate = 0;
	spr->cur_frame = 0;
	spr->time_left = spr->frame[spr->cur_frame].time;
	
	return;
}


void mtSpriteSetRepeat(MTSPRITE_ENTRY *spr, int repeat) {
	if (!spr)
		return;
	spr->repeat = (repeat) ? 1 : 0;
	
	return;
}


void *mtSpriteDelete(MTSPRITE_ENTRY *spr) {
	if (spr == NULL) return NULL;
	
	spr->ts = renderTilesheetFree(spr->ts);
	free(spr->cache);
	free(spr->frame);
	free(spr);

	return NULL;
}

// DARNIT_HEADLESS
#endif
