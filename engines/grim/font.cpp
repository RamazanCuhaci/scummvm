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

#include "common/endian.h"

#include "graphics/fonts/ttf.h"
#include "graphics/font.h"
#include "graphics/surface.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"
#include "engines/grim/font.h"
#include "engines/grim/resource.h"
#include "engines/grim/gfx_base.h"

namespace Grim {

void Font::save(const Font *font, SaveGame *state) {
	const FontTTF *ttf = dynamic_cast<const FontTTF *>(font);
	if (ttf) {
		state->writeLESint32(-2);
		state->writeLESint32(ttf->getId());
		return;
	}
	const BitmapFont *bitmapFont = dynamic_cast<const BitmapFont *>(font);
	if (bitmapFont) {
		state->writeLESint32(bitmapFont->getId());
		return;
	}
	state->writeLESint32(-1);
}

Font *Font::load(SaveGame *state) {
	int32 fontId = state->readLESint32();
	if (fontId == -1) {
		return nullptr;
	}
	if (fontId == -2) {
		fontId = state->readLESint32();
		return FontTTF::getPool().getObject(fontId);
	}

	return BitmapFont::getPool().getObject(fontId);
}

Font *Font::getByFileName(const Common::String& fontName) {
	for (Font *f : BitmapFont::getPool()) {
		if (f->getFilename() == fontName) {
			return f;
		}
	}
	for (Font *f : FontTTF::getPool()) {
		if (f->getFilename() == fontName) {
			return f;
		}
	}
	return nullptr;
}

Font *Font::getFirstFont() {
	if (BitmapFont::getPool().begin() != BitmapFont::getPool().end())
		return *BitmapFont::getPool().begin();
	if (FontTTF::getPool().begin() != FontTTF::getPool().end())
		return *FontTTF::getPool().begin();
	return nullptr;
}

bool BitmapFont::is8Bit() const {
	return !_isDBCS;
}

BitmapFont::BitmapFont() :
		_userData(nullptr),
		_fontData(nullptr), _charHeaders(nullptr),
		_numChars(0), _dataSize(0), _kernedHeight(0), _baseOffsetY(0),
		_firstChar(0), _lastChar(0) {
}

BitmapFont::~BitmapFont() {
	_fwdCharIndex.clear();
	delete[] _charHeaders;
	delete[] _fontData;
	g_driver->destroyFont(this);
}

void BitmapFont::load(const Common::String &filename, Common::SeekableReadStream *data) {
	_filename = filename;
	_numChars = data->readUint32LE();
	_dataSize = data->readUint32LE();
	_kernedHeight = data->readUint32LE();
	_baseOffsetY = data->readUint32LE();
	data->seek(24, SEEK_SET);
	_firstChar = data->readUint32LE();
	_lastChar = data->readUint32LE();

	_isDBCS = g_grim->getGameLanguage() == Common::ZH_CHN && _numChars > 0xff;

	if (_isDBCS) {
		// Read character indexes - are the key/value reversed?
		_fwdCharIndex.resize(_numChars);
		for (uint i = 0; i < _numChars; ++i) {
			uint16 point = data->readUint16LE();
			_fwdCharIndex[i] = point ? point : -1;
		}
		_scale = 2;
	} else {
		// Read character indexes - are the key/value reversed?
		Common::Array<uint16> revCharIndex;
		revCharIndex.resize(_numChars);
		uint16 maxPoint = 0;
		for (uint i = 0; i < _numChars; ++i) {
			uint16 point = data->readUint16LE();
			revCharIndex[i] = point;
			if (point > maxPoint)
				maxPoint = point;
		}
		_fwdCharIndex.resize(maxPoint + 1, -1);
		for (uint i = 0; i < _numChars; ++i) {
			_fwdCharIndex[revCharIndex[i]] = i;
		}

		// In order to ensure the correct character codes for
		// accented characters it is necessary to check the
		// requested code against the index of characters for
		// the font.  Previously, signed characters were
		// causing the problem but it might be possible for
		// an invalid character to be called for other reasons.
		//
		// Example: Without this fix when Manny greets Eva
		// for the first time and he says "Buenos Días" the
		// 'í' character will either show up as a different
		// character or it crashes the game.
		for (uint i = 0; i < _numChars; ++i) {
			if (revCharIndex[i] == i)
				_fwdCharIndex[revCharIndex[i]] = i;
		}
		_scale = 1;
	}

	// Read character headers
	_charHeaders = new CharHeader[_numChars];
	for (uint i = 0; i < _numChars; ++i) {
		_charHeaders[i].offset = data->readUint32LE();
		// Kerned character size
		_charHeaders[i].kernedWidth = data->readSByte();
		_charHeaders[i].startingCol = data->readSByte();
		_charHeaders[i].startingLine = data->readSByte();
		data->seek(1, SEEK_CUR);
		// Character bitmap size
		_charHeaders[i].bitmapWidth = data->readUint32LE();
		_charHeaders[i].bitmapHeight = data->readUint32LE();
	}
	// Read font data
	_fontData = new byte[_dataSize];

	data->read(_fontData, _dataSize);

	g_driver->createFont(this);
}

uint16 BitmapFont::getCharIndex(uint16 c) const {
	int res = c < _fwdCharIndex.size() ? _fwdCharIndex[c] : -1;
	if (res >= 0)
		return res;
	Debug::warning(Debug::Fonts, "The requested character (code 0x%x) does not correspond to anything in the font data!", c);
	// If we couldn't find the character then default to
	// the first character in the font so that something
	// gets loaded to prevent the game from crashing
	return 0;
}

int BitmapFont::getKernedStringLength(const Common::String &text) const {
	int result = 0;
	for (uint32 i = 0; i < text.size(); ++i) {
		uint16 ch = uint8(text[i]);
		if (_isDBCS && i + 1 < text.size() && (ch & 0x80)) {
			ch = (ch << 8) | (text[++i] & 0xff);
		}
		result += getCharKernedWidth(ch);
	}
	return result;
}

int BitmapFont::getBitmapStringLength(const Common::String &text) const {
	int result = 0;
	for (uint32 i = 0; i < text.size(); ++i) {
		uint16 ch = uint8(text[i]);
		if (_isDBCS && i + 1 < text.size() && (ch & 0x80)) {
			ch = (ch << 8) | (text[++i] & 0xff);
		}
		result += getCharKernedWidth(ch) + getCharStartingCol(ch);
	}
	return result;
}

int BitmapFont::getStringHeight(const Common::String &text) const {
	int result = 0;
	for (uint32 i = 0; i < text.size(); ++i) {
		uint16 ch = uint8(text[i]);
		if (_isDBCS && i + 1 < text.size() && (ch & 0x80)) {
			ch = (ch << 8) | (text[++i] & 0xff);
		}

		int verticalOffset = getCharStartingLine(ch) + getBaseOffsetY();
		int charHeight = verticalOffset + getCharBitmapHeight(ch);
		if (charHeight > result)
			result = charHeight;
	}
	return result;
}

void BitmapFont::saveState(SaveGame *state) const {
	state->writeString(getFilename());
}

void BitmapFont::restoreState(SaveGame *state) {
	Common::String fname = state->readString();
	Common::SeekableReadStream *stream;

	g_driver->destroyFont(this);
	delete[] _fontData;
	_fontData = nullptr;
	_fwdCharIndex.clear();
	delete[] _charHeaders;
	_charHeaders = nullptr;

	stream = g_resourceloader->openNewStreamFile(fname.c_str(), true);
	load(fname, stream);
	delete stream;
}

void FontTTF::saveState(SaveGame *state) const {
	state->writeString(getFilename());
	state->writeLESint32(_size);
}

void FontTTF::restoreState(SaveGame *state) {
	Common::String fname = state->readString();
	int size = state->readLESint32();
	Common::SeekableReadStream *stream;

	g_driver->destroyFont(this);
	delete _font;

	stream = g_resourceloader->openNewStreamFile(fname.c_str(), true);
	loadTTF(fname, stream, size);
	delete stream;
}

void BitmapFont::render(Graphics::Surface &buf, const Common::String &currentLine,
			const Graphics::PixelFormat &pixelFormat, uint32 blackColor, uint32 color, uint32 colorKey) const {
	int width = getBitmapStringLength(currentLine) + 1;
	int height = getStringHeight(currentLine) + 1;

	int startColumn = 0;

	buf.create(width, height, pixelFormat);
	buf.fillRect(Common::Rect(0, 0, width, height), colorKey);

	for (unsigned int d = 0; d < currentLine.size(); d++) {
		uint16 ch = uint8(currentLine[d]);
		if (_isDBCS && d + 1 < currentLine.size()) {
			ch = (ch << 8) | (currentLine[++d] & 0xff);
		}
		int32 charBitmapWidth = getCharBitmapWidth(ch);
		int32 charBitmapHeight = getCharBitmapHeight(ch);
		int8 fontRow = getCharStartingLine(ch) + getBaseOffsetY();
		int8 fontCol = getCharStartingCol(ch);

		for (int line = 0; line < charBitmapHeight; line++) {
			int lineOffset = (fontRow + line * _scale);
			int columnOffset = startColumn + fontCol;
			int fontOffset = (charBitmapWidth * line);
			for (int bitmapCol = 0; bitmapCol < charBitmapWidth; bitmapCol++, columnOffset += _scale, fontOffset++) {
				byte pixel = getCharData(ch)[fontOffset];
				if (pixel == 0x80) {
					for (uint i = 0; i < _scale; i++)
						for (uint j = 0; j < _scale; j++)
							buf.setPixel(columnOffset + i, lineOffset + j, blackColor);
				} else if (pixel == 0xFF) {
					for (uint i = 0; i < _scale; i++)
						for (uint j = 0; j < _scale; j++)
							buf.setPixel(columnOffset + i, lineOffset + j, color);
				}
			}
		}
		startColumn += getCharKernedWidth(ch);
	}
}

void FontTTF::loadTTF(const Common::String &filename, Common::SeekableReadStream *data, int size) {
	_filename = filename;
	_size = size;
#ifdef USE_FREETYPE2
	_font = Graphics::loadTTFFont(*data, size);
#else
	_font = nullptr;
#endif
}

void FontTTF::render(Graphics::Surface &surface, const Common::String &currentLine, const Graphics::PixelFormat &pixelFormat, uint32 blackColor, uint32 color, uint32 colorKey) const {
#ifdef USE_FREETYPE2
	Common::Rect bbox = _font->getBoundingBox(currentLine);
	surface.create(bbox.right, bbox.bottom, pixelFormat);
	surface.fillRect(Common::Rect(0, 0, bbox.right, bbox.bottom), colorKey);
	_font->drawString(&surface, currentLine, 0, 0, bbox.right, 0xFFFFFFFF);
#endif
}

// Hardcoded default font for FPS, GUI, etc
const uint8 BitmapFont::emerFont[][13] = {
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x36},
{0x00, 0x00, 0x00, 0x66, 0x66, 0xff, 0x66, 0x66, 0xff, 0x66, 0x66, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x7e, 0xff, 0x1b, 0x1f, 0x7e, 0xf8, 0xd8, 0xff, 0x7e, 0x18},
{0x00, 0x00, 0x0e, 0x1b, 0xdb, 0x6e, 0x30, 0x18, 0x0c, 0x76, 0xdb, 0xd8, 0x70},
{0x00, 0x00, 0x7f, 0xc6, 0xcf, 0xd8, 0x70, 0x70, 0xd8, 0xcc, 0xcc, 0x6c, 0x38},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x0c, 0x0e},
{0x00, 0x00, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0c},
{0x00, 0x00, 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x18, 0x30},
{0x00, 0x00, 0x00, 0x00, 0x99, 0x5a, 0x3c, 0xff, 0x3c, 0x5a, 0x99, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x18, 0x1c, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18, 0x0c, 0x0c, 0x06, 0x06, 0x03, 0x03},
{0x00, 0x00, 0x3c, 0x66, 0xc3, 0xe3, 0xf3, 0xdb, 0xcf, 0xc7, 0xc3, 0x66, 0x3c},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x38, 0x18},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0xe7, 0x7e},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0x07, 0x03, 0x03, 0xe7, 0x7e},
{0x00, 0x00, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0xff, 0xcc, 0x6c, 0x3c, 0x1c, 0x0c},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x03, 0x03, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x03, 0x7f, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x18, 0x1c, 0x1c, 0x00, 0x00, 0x1c, 0x1c, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06},
{0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60},
{0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x18, 0x0c, 0x06, 0x03, 0xc3, 0xc3, 0x7e},
{0x00, 0x00, 0x3f, 0x60, 0xcf, 0xdb, 0xd3, 0xdd, 0xc3, 0x7e, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x18},
{0x00, 0x00, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x7e, 0xe7, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0xfc, 0xce, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7, 0xce, 0xfc},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xcf, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e},
{0x00, 0x00, 0x7c, 0xee, 0xc6, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06},
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc, 0xc6, 0xc3},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0xff, 0xff, 0xe7, 0xc3},
{0x00, 0x00, 0xc7, 0xc7, 0xcf, 0xcf, 0xdf, 0xdb, 0xfb, 0xf3, 0xf3, 0xe3, 0xe3},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x3f, 0x6e, 0xdf, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c},
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0xe0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0xc3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x7e, 0x0c, 0x06, 0x03, 0x03, 0xff},
{0x00, 0x00, 0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c},
{0x00, 0x03, 0x03, 0x06, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x30, 0x30, 0x60, 0x60},
{0x00, 0x00, 0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x66, 0x3c, 0x18},
{0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x38, 0x30, 0x70},
{0x00, 0x00, 0x7f, 0xc3, 0xc3, 0x7f, 0x03, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x7e, 0xc3, 0xc0, 0xc0, 0xc0, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x7f, 0xc3, 0xc3, 0xc3, 0xc3, 0x7f, 0x03, 0x03, 0x03, 0x03, 0x03},
{0x00, 0x00, 0x7f, 0xc0, 0xc0, 0xfe, 0xc3, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x33, 0x1e},
{0x7e, 0xc3, 0x03, 0x03, 0x7f, 0xc3, 0xc3, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00},
{0x38, 0x6c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x00, 0x00, 0x0c, 0x00},
{0x00, 0x00, 0xc6, 0xcc, 0xf8, 0xf0, 0xd8, 0xcc, 0xc6, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78},
{0x00, 0x00, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xfc, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00},
{0xc0, 0xc0, 0xc0, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x03, 0x03, 0x03, 0x7f, 0xc3, 0xc3, 0xc3, 0xc3, 0x7f, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe0, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xfe, 0x03, 0x03, 0x7e, 0xc0, 0xc0, 0x7f, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x1c, 0x36, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x00},
{0x00, 0x00, 0x7e, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xe7, 0xff, 0xdb, 0xc3, 0xc3, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0xc0, 0x60, 0x60, 0x30, 0x18, 0x3c, 0x66, 0x66, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xff, 0x60, 0x30, 0x18, 0x0c, 0x06, 0xff, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x0f, 0x18, 0x18, 0x18, 0x38, 0xf0, 0x38, 0x18, 0x18, 0x18, 0x0f},
{0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
{0x00, 0x00, 0xf0, 0x18, 0x18, 0x18, 0x1c, 0x0f, 0x1c, 0x18, 0x18, 0x18, 0xf0},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x8f, 0xf1, 0x60, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

} // end of namespace Grim
