#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include "vita2d.h"
#include "font_atlas.h"
#include "utils.h"
#include "shared.h"

#define ATLAS_DEFAULT_W 512
#define ATLAS_DEFAULT_H 512

#define FONT_GLYPH_MARGIN 1

typedef enum
{
	VITA2D_LOAD_FONT_FROM_FILE,
	VITA2D_LOAD_FONT_FROM_MEM
} vita2d_load_font_from;

typedef struct vita2d_font
{
	vita2d_load_font_from load_from;
	union
	{
		char *filename;
		struct
		{
			const void *font_buffer;
			unsigned int buffer_size;
		};
	};
	FT_Library ftlibrary;
	FT_Face ftface;
	font_atlas *atlas;
	SceKernelLwMutexWork mutex;
	int font_size;
	int max_height;
	int max_ascender;
	int max_descender;
	int line_space;
} vita2d_font;

static int vita2d_load_font_post(vita2d_font *font, unsigned int size)
{
	FT_Error error;

	error = FT_Init_FreeType(&font->ftlibrary);

	if (error != FT_Err_Ok)
		return 0;

	error = FT_Err_Cannot_Open_Resource;

	if (font->load_from == VITA2D_LOAD_FONT_FROM_FILE)
	{
		error = FT_New_Face(
			font->ftlibrary,
			font->filename,
			0,
			&font->ftface);
	}
	else if (font->load_from == VITA2D_LOAD_FONT_FROM_MEM)
	{
		error = FT_New_Memory_Face(
			font->ftlibrary,
			font->font_buffer,
			font->buffer_size,
			0,
			&font->ftface);
	}

	if (error != FT_Err_Ok)
		return 0;

	error = FT_Select_Charmap(font->ftface, FT_ENCODING_UNICODE);
	if (error != FT_Err_Ok)
		return 0;

	error = FT_Set_Pixel_Sizes(font->ftface, 0, size);
	if (error != FT_Err_Ok)
		return 0;

	font->font_size = size;
	font->max_height = (font->ftface->size->metrics.height >> 6) + 0.5f;
	font->max_ascender = (font->ftface->size->metrics.ascender >> 6) + 0.5f;
	font->max_descender = (font->ftface->size->metrics.descender >> 6) + 0.5f;

	font->atlas = font_atlas_create(ATLAS_DEFAULT_W, ATLAS_DEFAULT_H, SCE_GXM_TEXTURE_FORMAT_U8_R111);
	if (!font->atlas)
		return 0;

	sceKernelCreateLwMutex(&font->mutex, "vita2d_font_mutex", 2, 0, NULL);
	return 1;
}

vita2d_font *vita2d_load_font_file(const char *filename, unsigned int size)
{
	vita2d_font *font = malloc(sizeof(*font));
	if (!font)
		return NULL;
	memset(font, 0, sizeof(vita2d_font));

	font->load_from = VITA2D_LOAD_FONT_FROM_FILE;
	font->filename = strdup(filename);

	if (!vita2d_load_font_post(font, size))
	{
		vita2d_free_font(font);
		return NULL;
	}

	return font;
}

vita2d_font *vita2d_load_font_mem(const void *buffer, unsigned int buffer_size, unsigned int size)
{
	vita2d_font *font = malloc(sizeof(*font));
	if (!font)
		return NULL;
	memset(font, 0, sizeof(vita2d_font));

	font->load_from = VITA2D_LOAD_FONT_FROM_MEM;
	font->font_buffer = buffer;
	font->buffer_size = buffer_size;

	if (!vita2d_load_font_post(font, size))
	{
		vita2d_free_font(font);
		return NULL;
	}

	return font;
}

void vita2d_free_font(vita2d_font *font)
{
	if (font)
	{
		sceKernelDeleteLwMutex(&font->mutex);

		if (font->ftface)
			FT_Done_Face(font->ftface);
		if (font->ftlibrary)
			FT_Done_FreeType(font->ftlibrary);
		if (font->load_from == VITA2D_LOAD_FONT_FROM_FILE && font->filename)
			free(font->filename);
		if (font->atlas)
			font_atlas_free(font->atlas);
		free(font);
	}
}

static int atlas_add_glyph(vita2d_font *font, unsigned int character)
{
	int i, j;
	vita2d_position position;
	vita2d_texture *tex = NULL;
	unsigned char *tex_data;
	unsigned int tex_stride;
	FT_GlyphSlot glyph_slot;
	FT_Bitmap *bitmap;

	if (FT_Load_Char(font->ftface, character, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL))
		return 0;

	if (FT_Render_Glyph(font->ftface->glyph, FT_RENDER_MODE_NORMAL))
		return 0;

	glyph_slot = font->ftface->glyph;
	bitmap = &glyph_slot->bitmap;

	font_glyph glyph_data = {
		bitmap->width + FONT_GLYPH_MARGIN * 2,
		bitmap->rows + FONT_GLYPH_MARGIN * 2,
		glyph_slot->bitmap_left,
		glyph_slot->bitmap_top,
		glyph_slot->advance.x >> 6,
		glyph_slot->advance.y >> 6,
		font->font_size};

	if (!font_atlas_insert(font->atlas, character, &tex, &position, &glyph_data))
		return 0;

	tex_stride = vita2d_texture_get_stride(tex);
	tex_data = (unsigned char *)vita2d_texture_get_datap(tex);

	for (i = 0; i < bitmap->rows; i++)
	{
		for (j = 0; j < bitmap->width; j++)
		{
			if (bitmap->pixel_mode == FT_PIXEL_MODE_MONO)
			{
				tex_data[position.x + FONT_GLYPH_MARGIN + j + (position.y + FONT_GLYPH_MARGIN + i) * tex_stride] =
					(bitmap->buffer[i * bitmap->pitch + j / 8] & (1 << (7 - j % 8))) ? 0xFF : 0;
			}
			else if (bitmap->pixel_mode == FT_PIXEL_MODE_GRAY)
			{
				tex_data[position.x + FONT_GLYPH_MARGIN + j + (position.y + FONT_GLYPH_MARGIN + i) * tex_stride] =
					bitmap->buffer[i * bitmap->pitch + j];
			}
		}
	}

	return 1;
}

static int generic_font_draw_text(vita2d_font *font, int draw, int *height,
								  int x, int y, unsigned int color,
								  const char *text)
{
	sceKernelLockLwMutex(&font->mutex, 1, NULL);

	int i;
	unsigned int character;
	vita2d_position position;
	font_glyph glyph;
	vita2d_texture *tex = NULL;	
	int start_x = x;
	int max_x = x;
	int pen_x = x;
	int pen_y = y;
	int line_height = vita2d_font_get_lineheight(font);

	for (i = 0; text[i];)
	{
		i += utf8_to_ucs2(&text[i], &character);

		if (character == '\n')
		{
			if (pen_x > max_x)
				max_x = pen_x;
			pen_x = start_x;
			pen_y += (line_height + font->line_space);
			continue;
		}

		if (!font_atlas_get(font->atlas, character, &tex, &position, &glyph))
		{
			if (!atlas_add_glyph(font, character))
				continue;

			if (!font_atlas_get(font->atlas, character, &tex, &position, &glyph))
				continue;
		}

		if (draw)
		{
			vita2d_draw_texture_tint_part(tex,
				pen_x + glyph.bitmap_left,
				pen_y + font->max_ascender - glyph.bitmap_top,
				position.x, position.y,
				glyph.bitmap_width, glyph.bitmap_height,
				color);
		}

		pen_x += (glyph.advance_x + FONT_GLYPH_MARGIN);
	}

	if (pen_x > max_x)
		max_x = pen_x;

	if (height)
		*height = pen_y + line_height - y;

	sceKernelUnlockLwMutex(&font->mutex, 1);

	return max_x - x;
}

int vita2d_font_draw_text(vita2d_font *font, int x, int y,
						  unsigned int color, const char *text)
{
	return generic_font_draw_text(font, 1, NULL, x, y, color, text);
}

int vita2d_font_draw_textf(vita2d_font *font, int x, int y,
						  unsigned int color, const char *text, ...)
{
	char buf[1024];
	va_list argptr;

	va_start(argptr, text);
	vsnprintf(buf, sizeof(buf), text, argptr);
	va_end(argptr);

	return vita2d_font_draw_text(font, x, y, color, buf);
}

void vita2d_font_text_dimensions(vita2d_font *font, const char *text, int *width, int *height)
{
	int w;
	w = generic_font_draw_text(font, 0, height, 0, 0, 0, text);

	if (width)
		*width = w;
}

int vita2d_font_text_width(vita2d_font *font, const char *text)
{
	int width;
	vita2d_font_text_dimensions(font, text, &width, NULL);
	return width;
}

int vita2d_font_text_height(vita2d_font *font, const char *text)
{
	int height;
	vita2d_font_text_dimensions(font, text, NULL, &height);
	return height;
}

void vita2d_font_set_linespace(vita2d_font *font, int line_space)
{
	font->line_space = line_space;
}

int vita2d_font_get_linespace(vita2d_font *font)
{
	return font->line_space;
}

int vita2d_font_get_lineheight(vita2d_font *font)
{
	return font->max_height + FONT_GLYPH_MARGIN * 2;
}

void vita2d_font_set_fontsize(vita2d_font *font, unsigned int size)
{
	if (font->font_size != size)
	{
		sceKernelLockLwMutex(&font->mutex, 1, NULL);
		font->font_size = size;
		if (font->atlas)
			font_atlas_free(font->atlas);
		font->atlas = font_atlas_create(ATLAS_DEFAULT_W, ATLAS_DEFAULT_H, SCE_GXM_TEXTURE_FORMAT_U8_R111);
		sceKernelUnlockLwMutex(&font->mutex, 1);
	}
}

unsigned int vita2d_font_get_fontsize(vita2d_font *font)
{
	return font->font_size;
}
