/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/plugins/ags_sprite_font/sprite_font_renderer_clifftop.h"
#include "ags/plugins/ags_sprite_font/color.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

SpriteFontRendererClifftop::SpriteFontRendererClifftop(IAGSEngine *engine) : SpriteFontRenderer(engine) {
}

SpriteFontRendererClifftop::~SpriteFontRendererClifftop(void) {
}

bool SpriteFontRendererClifftop::SupportsExtendedCharacters(int fontNumber) {
	return true;
}

void SpriteFontRendererClifftop::RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) {
	SpriteFont *font = getFontFor(fontNumber);

	for (int i = 0; i < (int)strlen(text); i++) {
		char c = text[i];
		c -= font->MinChar;
		int row = c / font->Columns;
		int column = c % font->Columns;
		BITMAP *src = _engine->GetSpriteGraphic(font->SpriteNumber);
		Draw(src, destination, x + (i * font->CharWidth), y, column * font->CharWidth, row * font->CharHeight, font->CharWidth, font->CharHeight, colour);
	}
}

void SpriteFontRendererClifftop::Draw(BITMAP *src, BITMAP *dest, int destx, int desty, int srcx, int srcy, int width, int height, int colour) {

	int32 srcWidth = 0, srcHeight = 0, destWidth = 0, destHeight = 0, srcColDepth = 0, destColDepth = 0;

	uint8 *srccharbuffer = _engine->GetRawBitmapSurface(src);
	uint8 *destcharbuffer = _engine->GetRawBitmapSurface(dest);

	uint32 transColor = _engine->GetBitmapTransparentColor(src);
	int srcPitch = _engine->GetBitmapPitch(src);
	int destPitch = _engine->GetBitmapPitch(dest);

	_engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, &srcColDepth);
	_engine->GetBitmapDimensions(dest, &destWidth, &destHeight, &destColDepth);
	int bpp = destColDepth / 8;

	if (srcy + height > srcHeight || srcx + width > srcWidth || srcx < 0 || srcy < 0) return;

	if (width + destx > destWidth) width = destWidth - destx;
	if (height + desty > destHeight) height = destHeight - desty;

	int startx = MAX(0, (-1 * destx));
	int starty = MAX(0, (-1 * desty));


	int srca, srcr, srcg, srcb, desta, destr, destg, destb, finalr, finalg, finalb, finala, col, col_r, col_g, col_b;
	col_r = getr32(colour);
	col_g = getg32(colour);
	col_b = getb32(colour);

	int srcxx = (startx + srcx) * bpp;
	int destxx = (startx + destx) * bpp;
	for (int x = startx; x < width; ++x, srcxx += bpp, destxx += bpp) {

		int srcyy = (starty + srcy) * srcPitch;
		int destyy = (starty + desty) * destPitch;
		for (int y = starty; y <  height; ++y, srcyy += srcPitch, destyy += destPitch) {
			uint8 *srcCol = srccharbuffer + srcyy + srcxx;
			uint8 *destCol = destcharbuffer + destyy + destxx;
			if (destColDepth == 8) {
				if (*srcCol != transColor)
					*destCol = *srcCol;
			} else if (destColDepth == 16) {
				if (*((uint16 *)srcCol) != transColor)
					*((uint16 *)destCol) = *((uint16 *)srcCol);
			} else if (destColDepth == 32) {
				//if (*((uint32*)srcCol) != transColor)
				//  *((uint32*)destCol) = *((uint32*)srcCol);

				uint32 srcargb = *((uint32 *)srcCol);
				uint32 &destargb = *((uint32 *)destCol);

				srca = (geta32(srcargb));

				if (srca != 0) {

					srcr =  getr32(srcargb);
					srcg =  getg32(srcargb);
					srcb =  getb32(srcargb);

					destr =  getr32(destargb);
					destg =  getg32(destargb);
					destb =  getb32(destargb);
					desta =  geta32(destargb);

					finalr = (col_r * srcr) / 255;
					finalg = (col_g * srcg) / 255;
					finalb = (col_b * srcb) / 255;

					finala = 255 - (255 - srca) * (255 - desta) / 255;
					finalr = srca * col_r / finala + desta * destr * (255 - srca) / finala / 255;
					finalg = srca * col_g / finala + desta * destg * (255 - srca) / finala / 255;
					finalb = srca * col_b / finala + desta * destb * (255 - srca) / finala / 255;
					col = makeacol32(finalr, finalg, finalb, finala);
					destargb = col;
				}

			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
	_engine->ReleaseBitmapSurface(dest);
}

} // namespace AGSSpriteFont
} // namespace Plugins
} // namespace AGS3
