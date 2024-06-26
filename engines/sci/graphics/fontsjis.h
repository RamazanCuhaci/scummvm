/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCI_GRAPHICS_FONTSJIS_H
#define SCI_GRAPHICS_FONTSJIS_H

#include "sci/graphics/helpers.h"

namespace Graphics {
class FontSJIS;
}

namespace Sci {

/**
 * Special Font class, handles SJIS inside sci games, uses ScummVM SJIS support
 */
class GfxFontSjis : public GfxFont {
public:
	GfxFontSjis(GfxScreen *screen, GuiResourceId resourceId);
	~GfxFontSjis() override;

	GuiResourceId getResourceId() override;
	byte getHeight() override;
	bool isDoubleByte(uint16 chr) override;
	byte getCharWidth(uint16 chr) override;
	void draw(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput) override;

private:
	GfxScreen *_screen;
	GuiResourceId _resourceId;

	Graphics::FontSJIS *_commonFont;
};

} // End of namespace Sci

#endif // SCI_GRAPHICS_FONTSJIS_H
