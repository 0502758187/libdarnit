#include "darnit.h"


void *textLoadFont(void *handle, const char *fname, int size, int tex_w, int tex_h) {
	DARNIT *m = handle;
	TEXT_FONT *font;
	FILE *fp;
	int flen, asc, desc, linegap;

	if ((font = malloc(sizeof(TEXT_FONT))) == NULL) {
		return NULL;
	}

	if ((fp = fopen(fname, "rb")) == NULL) {
		fprintf(stderr, "Unable to open file %s\n", fname);
		free(font);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	flen = ftell(fp);
	rewind(fp);

	if ((font->font_data = malloc(flen)) == NULL) {
		fclose(fp);
		free(font);
		return NULL;
	}

	fread(font->font_data, flen, 1, fp);
	font->font_data_len = flen;
	fclose(fp);

	if (stbtt_InitFont(&font->face, font->font_data, 0) == 0) {
		free(font->font_data);
		free(font);
		return NULL;
	}

	font->cache = NULL;
	font->font_height = size;
	font->tex_w = bitwiseRoundUpToPow2(tex_w);
	font->tex_h = bitwiseRoundUpToPow2(tex_h);
	font->scale = stbtt_ScaleForPixelHeight(&font->face, font->font_height);

	stbtt_GetFontVMetrics(&font->face, &asc, &desc, &linegap);
	font->line_gap = font->scale * linegap;
	font->ascent = font->scale * asc;
	font->descent = font->scale * desc;

	font->screen_pw = m->video.swgran;
	font->screen_ph = m->video.shgran;
	font->handle = handle;

	return font;
}


int textWillGlyphFit(struct TEXT_FONT_CACHE *cache, int w, int h) {
	int pos_y;
	
	pos_y = (cache->line_pos_x + w >= cache->sheet_w) ? cache->line_pos_y + cache->line_height : cache->line_pos_y;
	if (pos_y + h >= cache->sheet_h)
		return -1;
	return 0;
}


struct TEXT_FONT_CACHE *textFindLastTexture(struct TEXT_FONT_CACHE *cache) {
	if (cache == NULL)
		return NULL;
	while (cache->next != NULL)
		cache = cache->next;
	
	return cache;
}


struct TEXT_FONT_CACHE *textAppendCache(TEXT_FONT *font, struct TEXT_FONT_CACHE *index, int tex_w, int tex_h) {
	struct TEXT_FONT_CACHE *next;

	index = textFindLastTexture(index);

	if ((next = malloc(sizeof(struct TEXT_FONT_CACHE))) == NULL)
		return NULL;
	
	if (index != NULL)
		index->next = next;

	next->next = NULL;
	next->glyph = NULL;

	if ((next->ts = renderNewTilesheet(font->handle, 1, 1, tex_w, tex_h, RENDER_DATA_TYPE_ALPHA)) == NULL) {
		free(next);
		return NULL;
	}

	next->line_pos_x = next->line_pos_y = next->line_height = 0;
	next->sheet_w = tex_w;
	next->sheet_h = tex_h;

	next->sheet_pwf = 1.0f / tex_w;
	next->sheet_phf = 1.0f / tex_h;
	next->line_height = 0;
	next->line_pos_x = next->line_pos_y = 0;

	return next;
}


void textCacheGetCoordinates(struct TEXT_FONT_CACHE *index, int w, int h, int *x, int *y) {
	*x = index->line_pos_x + w + 2;
	*y = index->line_pos_y;
	if (*x >= index->sheet_w) {
		*x = 0;
		*y += index->line_height;
		index->line_pos_y = *y;
		index->line_height = 0;
	} else {
		*x -= w;
		*x += 1;
	}

	if (index->line_height < h)
		index->line_height = h;

	index->line_pos_x = *x + w + 1;
	index->line_pos_y = *y;
	return;
}


void textPushGlyph(const char *data, int w, int h, int x, int y, TILESHEET *ts) {
	int w_w, h_w, i;
	char *tmp;

	w_w = bitwiseRoundUpToPow2(w);
	h_w = bitwiseRoundUpToPow2(h);

	tmp = malloc(w_w * h_w);
	
	for (i = 0; i < h; i++)
		memcpy(tmp, &data[i*w], w);
	
	renderUpdateTilesheet(ts, x, y, tmp, w_w, h_w, RENDER_DATA_TYPE_ALPHA);

	free(tmp);
	return;
}


struct TEXT_FONT_GLYPH *textRenderGlyph(struct TEXT_FONT_CACHE *index, unsigned int glyph, int glyph_index, TEXT_FONT *font) {
	int pos_x, pos_y, w, h, x1, x2, y1, y2, ad, sb;
	unsigned char *data;
	struct TEXT_FONT_GLYPH *next, *alloc;

	if ((alloc = malloc(sizeof(struct TEXT_FONT_GLYPH))) == NULL)
		return NULL;

	next = index->glyph;
	alloc->next = NULL;

	if (next == NULL)
		index->glyph = alloc;
	else {
		while (next->next != NULL)
			next = next->next;
		next->next = alloc;
	}

	x1 = y1 = x2 = y2 = 0;
	data = stbtt_GetGlyphBitmap(&font->face, 0, font->scale, glyph_index, &w, &h, 0, 0);
	textCacheGetCoordinates(index, w, h, &pos_x, &pos_y);
	stbtt_GetGlyphBox(&font->face, glyph_index, &x1, &y1, &x2, &y2);
	stbtt_GetGlyphHMetrics(&font->face, glyph_index, &ad, &sb);

	alloc->u1 = index->sheet_pwf * pos_x + 0.0005f;
	alloc->v1 = index->sheet_phf * pos_y + 0.001f;
	alloc->u2 = index->sheet_pwf * pos_x + index->sheet_pwf * (w) - 0.0005f;
	alloc->v2 = index->sheet_phf * pos_y + index->sheet_phf * (h) - 0.001f;
	alloc->wf = index->sheet_pwf * w;
	alloc->hf = index->sheet_phf * h;
	alloc->rise = index->ts->shgran * ((y2) * font->scale);
	alloc->risei = y2 * font->scale;

	alloc->cw = w;
	alloc->ch = h;
	alloc->adv = font->scale * ad;
	alloc->advf = index->ts->swgran * font->scale *ad;
	alloc->skip = font->scale * x1;
	alloc->skipf = index->ts->swgran * font->scale * x1;

	alloc->glyph = glyph;
	alloc->tex_cache = index;

	renderUpdateTilesheet(index->ts, pos_x, pos_y, data, w, h, RENDER_DATA_TYPE_ALPHA);
	stbtt_FreeBitmap(data, NULL);

	return alloc;
}


struct TEXT_FONT_GLYPH *textRenderGlyphToCache(TEXT_FONT *font, unsigned int glyph) {
	int glyph_index, x1, y1, x2, y2;
	struct TEXT_FONT_CACHE *next;
	
	next = font->cache;
	glyph_index = stbtt_FindGlyphIndex(&font->face, glyph);

	stbtt_GetCodepointBox(&font->face, glyph_index, &x1, &y1, &x2, &y2);

	x1 = font->scale * x1;
	y1 = font->scale * y1;
	x2 = font->scale * x2;
	y2 = font->scale * y2;
	
	while (next != NULL) {
		if (textWillGlyphFit(next, x2 - x1, y2 - y1) == 0)
			break;
		next = next->next;
	}
	
	if (next == NULL)
		if ((next = textAppendCache(font, font->cache, font->tex_w, font->tex_h)) == NULL)
			return NULL;
	
	if (font->cache == NULL)
		font->cache = next;
	
	return textRenderGlyph(next, glyph, glyph_index, font);
}


struct TEXT_FONT_GLYPH *textGetGlyphEntry(TEXT_FONT *font, unsigned int glyph) {
	struct TEXT_FONT_CACHE *next;
	struct TEXT_FONT_GLYPH *next_g;

	next = font->cache;
	while (next != NULL) {
		next_g = next->glyph;
		while (next_g != NULL) {
			if (next_g->glyph == glyph)
				return next_g;
			next_g = next_g->next;
		}
		next = next->next;
	}

	return textRenderGlyphToCache(font, glyph);
}


int textGetGlyphWidth(TEXT_FONT *font, unsigned int glyph) {
	struct TEXT_FONT_GLYPH *glyph_e;

	if ((glyph_e = textGetGlyphEntry(font, glyph)) == NULL)
		return 0;
	return glyph_e->adv;
}

int textFontGetH(TEXT_FONT *font) {
	if (font == NULL) return -1;
	return font->font_height;
}

int textFontGetHS(TEXT_FONT *font) {
	if (font == NULL) return -1;
	return font->font_height + font->line_gap;
}

void textResetSurface(TEXT_SURFACE *srf) {
	if (srf == NULL) return;
	int i;
	float *arr;
	struct TEXT_GLYPH_CACHE *cache, *tmp;

	arr = (float *) srf->cache;

	for (i = 0; i < srf->len*24; i++)
		arr[i] = 0.0f;

	cache = srf->g_cache;
	while (cache != NULL) {
		tmp = cache->next;
		free(cache);
		cache = tmp;
	}

	srf->g_cache = srf->l_cache = NULL;
	srf->pos = 0;
	srf->line = 0;
	srf->index = 0;
	srf->cur_xf = srf->orig_xf;
	srf->cur_yf = srf->orig_yf;


	return;
}


void *textMakeRenderSurface(int chars, TEXT_FONT *font, unsigned int linelen, int x, int y) {
	TEXT_SURFACE *surface;
	DARNIT *m = font->handle;
	int i;
	float *arr;

	if ((surface = malloc(sizeof(TEXT_SURFACE))) == NULL) {
		MALLOC_ERROR
		return NULL;
	}

	if ((surface->cache = malloc(sizeof(TILE_CACHE)*chars)) == NULL) {
		MALLOC_ERROR
		return NULL;
	}

	arr = (float *) surface->cache;

	for (i = 0; i < chars*24; i++)
		arr[i] = 0.0f;

	surface->font = font;
	surface->pos = 0;
	surface->line = 0;
	surface->index = 0;
	
	surface->len = chars;
	surface->linelen = linelen;
	if (linelen == ~0)
		linelen = 0;
	surface->cur_xf = m->video.swgran * -1 * (linelen >> 1);
	surface->cur_yf = m->video.shgran * (font->font_height >> 1) * -1;
	surface->orig_xf = surface->cur_xf;
	surface->orig_yf = surface->cur_yf;
	surface->x = font->screen_pw * (x + (linelen >> 1)) - 1.0f;
	surface->y = 1.0f - font->screen_ph * (y + (font->ascent>>1));

	surface->g_cache = surface->l_cache = NULL;

	return surface;
}


/* continue here */


struct TEXT_GLYPH_CACHE *textGlyphCacheNew(struct TEXT_GLYPH_CACHE *start) {
	struct TEXT_GLYPH_CACHE *new;

	if ((new = malloc(sizeof(struct TEXT_GLYPH_CACHE))) == NULL)
		return NULL;

	new->next = NULL;
	
	if (start != NULL) {
		while (start->next != NULL)
			start = start->next;
		start->next = new;
	}

	return new;
}		


void *textSurfaceGlyphCacheFree(struct TEXT_GLYPH_CACHE *cache) {
	struct TEXT_GLYPH_CACHE *tmp;

	while (cache != NULL) {
		tmp = cache->next;
		free(cache);
		cache = tmp;
	}

	return NULL;
}


int textSurfaceAppendChar(TEXT_SURFACE *surface, const char *ch) {
	if (surface == NULL) return 1;

	unsigned int glyph;
	struct TEXT_FONT_GLYPH *glyph_e;
	float x, y, x2, y2;
	int w;

	if (surface->len == surface->index) return 1;

	glyph = utf8GetChar(ch);
	if ((glyph_e = textGetGlyphEntry(surface->font, glyph)) == NULL)
		return 1;

	if (surface->pos >= surface->linelen) {
		surface->cur_xf = surface->orig_xf;
		surface->cur_yf += surface->orig_yf * 2 + surface->font->line_gap * surface->font->screen_ph;
		surface->pos = 0;
	} 

	w = textGetGlyphWidth(surface->font, glyph);
	x = surface->cur_xf;
	surface->cur_xf += glyph_e->advf;
	surface->pos += w;

	if (*ch == '\n') {
		surface->cur_xf = surface->orig_xf;
		surface->cur_yf += surface->orig_yf * 2 + surface->font->line_gap * surface->font->screen_ph;
		surface->pos = 0;
		return 1;
	}


	x += glyph_e->skipf;
	x2 = x + surface->font->screen_pw * glyph_e->cw;
	y2 = surface->cur_yf + glyph_e->rise;
	y = y2 - surface->font->screen_ph * glyph_e->ch;


	if (*ch != ' ') {							/* "Temporary" work-around that makes OpenGL|ES happier... */
		renderSetTileCoordinates(&surface->cache[surface->index], x, y, x2, y2, glyph_e->u1, glyph_e->v1, glyph_e->u2, glyph_e->v2);

		if (surface->l_cache == NULL || glyph_e->tex_cache != surface->l_cache->f_cache) {
			if ((surface->l_cache = textGlyphCacheNew(surface->g_cache)) == NULL)
				return 1;
			if (surface->g_cache == NULL)
				surface->g_cache = surface->l_cache;
			surface->l_cache->t_cache = &surface->cache[surface->index];
			surface->l_cache->f_cache = glyph_e->tex_cache;
			surface->l_cache->glyphs = 0;
		}

		surface->l_cache->glyphs++;
		surface->index++;
	} 
	
	return utf8GetValidatedCharLength(ch);
}


void textSurfaceAppendString(TEXT_SURFACE *surface, const char *str) {
	int i;

	for (i = 0; str[i] != 0; ) {
		if (surface->index >= surface->len)
			break;
		i += textSurfaceAppendChar(surface, &str[i]);
	}

	return;
}

void textRender(TEXT_SURFACE *surface) {
	struct TEXT_GLYPH_CACHE *next;

	glTranslatef(surface->x, surface->y, 0.0f);
	
	if (surface == NULL) return;
	if (surface->index == 0)
		return;
	next = surface->g_cache;
	while (next != NULL) {
		renderCache(next->t_cache, next->f_cache->ts, next->glyphs);
		next = next->next;
	}

	glTranslatef(-surface->x, -surface->y, 0.0f);

	return;
}


void *textSurfaceDestroy(TEXT_SURFACE *surface) {
	if (surface == NULL)
		return NULL;

	textSurfaceGlyphCacheFree(surface->g_cache);
	free(surface->cache);
	free(surface);

	return NULL;
}
