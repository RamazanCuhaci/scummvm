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

#ifndef TITANIC_MOVIE_MANAGER_H
#define TITANIC_MOVIE_MANAGER_H

#include "titanic/core/list.h"
#include "titanic/core/resource_key.h"

namespace Titanic {

class CMovie;
class CVideoSurface;

class CMovieManagerBase {
public:
	virtual ~CMovieManagerBase() {}

	virtual CMovie *createMovie(const CResourceKey &key, CVideoSurface *surface) = 0;
};

class CMovieManager : public CMovieManagerBase {
private:
	int _field4;
public:
	CMovieManager() : CMovieManagerBase(), _field4(0) {}
	virtual ~CMovieManager() {}

	virtual CMovie *createMovie(const CResourceKey &key, CVideoSurface *surface);
};

} // End of namespace Titanic

#endif /* TITANIC_MOVIE_MANAGER_H */
