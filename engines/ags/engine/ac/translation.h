/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#ifndef AGS_ENGINE_AC_TRANSLATION_H
#define AGS_ENGINE_AC_TRANSLATION_H

#include "ags/shared/util/string.h"

namespace AGS3 {

using AGS::Shared::String;
struct TreeMap;

void close_translation();
bool init_translation(const String &lang, const String &fallback_lang, bool quit_on_error);
// Returns current translation name, or empty string if default translation is used
String get_translation_name();
// Returns fill path to the translation file, or empty string if default translation is used
String get_translation_path();
// Returns translation map for reading only
const TreeMap *get_translation_tree();

} // namespace AGS3

#endif
