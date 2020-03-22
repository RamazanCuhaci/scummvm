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

#ifndef ULTIMA4_MAPLOADER_H
#define ULTIMA4_MAPLOADER_H

#include "ultima/ultima4/map.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class U4FILE;
class Dungeon;

struct MapType_Hash {
	uint operator()(const Map::Type &v) const {
		return (uint)v;
	}
};


/**
 * The generic map loader interface.  Map loaders should override the
 * load method to load a map from the meta data already initialized in
 * the map object passed in. They must also register themselves with
 * registerLoader for one or more Map::Types.
 *
 * @todo
 * <ul>
 *      <li>
 *          Instead of loading dungeon room data into a u4dos-style structure and converting it to
 *          an xu4 Map when it's needed, convert it to an xu4 Map immediately upon loading it.
 *      </li>
 * </ul>
 */
class MapLoader {
public:
	virtual ~MapLoader() {}

	static MapLoader *getLoader(Map::Type type);

	virtual bool load(Map *map) = 0;

protected:
	static MapLoader *registerLoader(MapLoader *loader, Map::Type type);
	static bool loadData(Map *map, U4FILE *f);
	static bool isChunkCompressed(Map *map, int chunk);

private:
	static Std::map<Map::Type, MapLoader *, MapType_Hash> *loaderMap;
};

class CityMapLoader : public MapLoader {
	static MapLoader *_instance;

public:
	virtual bool load(Map *map);

};

class ConMapLoader : public MapLoader {
	static MapLoader *_instance;

public:
	virtual bool load(Map *map);

};

class DngMapLoader : public MapLoader {
	static MapLoader *_instance;

public:
	virtual bool load(Map *map);

private:
	void initDungeonRoom(Dungeon *dng, int room);
};

class WorldMapLoader : public MapLoader {
	static MapLoader *_instance;

public:
	virtual bool load(Map *map);

};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
