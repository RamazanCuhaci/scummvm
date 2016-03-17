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

#ifndef TITANIC_VIDEO_SURFACE_H
#define TITANIC_VIDEO_SURFACE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "titanic/font.h"
#include "titanic/direct_draw.h"
#include "titanic/core/list.h"
#include "titanic/core/resource_key.h"

namespace Titanic {

class CScreenManager;

class CVideoSurface : public ListItem {
protected:
	static int _videoSurfaceCounter;
protected:
	CScreenManager *_screenManager;
	CResourceKey _resourceKey;
	DirectDrawSurface *_ddSurface;
	uint16 *_pixels;
	void *_field34;
	bool _pendingLoad;
	int _field3C;
	int _field40;
	int _field44;
	int _field48;
	int _videoSurfaceNum;
	int _field50;
	int _lockCount;
public:
	CVideoSurface(CScreenManager *screenManager);
	virtual ~CVideoSurface();

	/**
	 * Set the underlying surface for this video surface
	 */
	void setSurface(CScreenManager *screenManager, DirectDrawSurface *surface);

	/**
	 * Load the surface with the passed resource
	 */
	virtual void loadResource(const CResourceKey &key) = 0;

	/**
	 * Loads a Targa image file specified by the resource key
	 */
	virtual void loadTarga() = 0;

	/**
	 * Loads a JPEG image file specified by the resource key
	 */
	virtual void loadJPEG() = 0;

	/**
	 * Loads a movie file specified by the resource key
	 */
	virtual void loadMovie() = 0;

	/**
	 * Lock the surface for direct access to the pixels
	 */
	virtual bool lock() = 0;

	/**
	 * Unlocks the surface after prior calls to lock()
	 */
	virtual void unlock() = 0;

	/**
	 * Returns true if an underlying raw surface has been set
	 */
	virtual bool hasSurface() = 0;

	/**
	 * Returns the width of the surface
	 */
	virtual int getWidth() const = 0;

	/**
	 * Returns the height of the surface
	 */
	virtual int getHeight() const = 0;

	/**
	 * Returns the pitch of the surface in bytes
	 */
	virtual int getPitch() const = 0;
	
	/**
	 * Reiszes the surface
	 */
	virtual void resize(int width, int height) = 0;

	/**
	 * Loads the surface data based on the currently set resource key
	 */
	virtual bool load() = 0;

	/**
	 * Loads the surface's resource if there's one pending
	 */
	virtual bool loadIfReady() = 0;

	/**
	 * Frees the underlying surface
	 */
	virtual int freeSurface() { return 0; }
};

class OSVideoSurface : public CVideoSurface {
public:
	OSVideoSurface(CScreenManager *screenManager, DirectDrawSurface *surface);
	OSVideoSurface(CScreenManager *screenManager, const CResourceKey &key, bool flag = false);

	/**
	 * Load the surface with the passed resource
	 */
	virtual void loadResource(const CResourceKey &key);

	/**
	 * Loads a Targa image file specified by the resource key
	 */
	virtual void loadTarga();

	/**
	 * Loads a JPEG image file specified by the resource key
	 */
	virtual void loadJPEG();

	/**
	 * Loads a movie file specified by the resource key
	 */
	virtual void loadMovie();

	/**
	 * Lock the surface for direct access to the pixels
	 */
	virtual bool lock();

	/**
	 * Unlocks the surface after prior calls to lock()
	 */
	virtual void unlock();

	/**
	 * Returns true if an underlying raw surface has been set
	 */
	virtual bool hasSurface();

	/**
	 * Returns the width of the surface
	 */
	virtual int getWidth() const;

	/**
	 * Returns the height of the surface
	 */
	virtual int getHeight() const;

	/**
	 * Returns the pitch of the surface in bytes
	 */
	virtual int getPitch() const;

	/**
	 * Reiszes the surface
	 */
	virtual void resize(int width, int height);

	/**
	 * Loads the surface data based on the currently set resource key
	 */
	virtual bool load();

	/**
	 * Loads the surface's resource if there's one pending
	 */
	virtual bool loadIfReady();

	/**
	 * Frees the underlying surface
	 */
	virtual int freeSurface();
};

} // End of namespace Titanic

#endif /* TITANIC_VIDEO_SURFACE_H */
