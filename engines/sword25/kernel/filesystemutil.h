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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * 
 * The class BS_FileSystemUtil represents a wrapper for file system specific 
 * operations that do not have equivalents in the C/C++ libraries.
 *
 * Each supported platform must implement this interface, and the method
 * BS_FileSystemUtil Singleton::getInstance()
 */

#ifndef SWORD25_FILESYSTEMUTIL_H
#define SWORD25_FILESYSTEMUTIL_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common/str.h"
#include "sword25/kernel/common.h"
#include "sword25/kernel/bs_stdint.h"

// -----------------------------------------------------------------------------
// Class definitions
// -----------------------------------------------------------------------------

class BS_FileSystemUtil
{
public:
	static BS_FileSystemUtil &GetInstance();
	virtual ~BS_FileSystemUtil() {};

	/**
	 * This function returns the name of the directory in which all user data is to be stored.
	 *
	 * These are for example Screenshots, game saves, configuration files, log files, ...
	 * @return				Returns the name of the directory for user data.
	 */
	virtual Common::String GetUserdataDirectory() = 0;
	/**
	 * @return				Returns the path seperator
	 */
	virtual Common::String	GetPathSeparator() = 0;
	/**
	 * @param Filename		The path to a file.
	 * @return				Returns the size of the specified file. If the size could not be
	 * determined, or the file does not exist, returns -1
	 */
	virtual uint64_t GetFileSize(const Common::String &Filename) = 0;
	/**
	 * @param Filename		The path to a file.
	 * @return				Returns the timestamp of the specified file. On error it returns 0
	 */
	virtual time_t GetFileTime(const Common::String &Filename) = 0;
	/**
	 * @param Filename		The path to a file.
	 * @return				Returns true if the file exists.
	 */
	virtual bool FileExists(const Common::String &Filename) = 0;
	/**
	 * This function creates a directory
	 * 
	 * If the parameter is "\b\c\d\e" is passed, and "\b\c" already exists, then folder 'd'
	 * will be created, and subdirectory 'e' under it.
	 * @param DirectoryName	The name of the directory to be created
	 * @return				Returns true if the folder(s) could be created, otherwise false.
	 */
	virtual bool CreateDirectory(const Common::String &DirectoryName) = 0;
	/**
	 * Creates a list of filenames in a given directory.
	 * @param Directory		The directory to search
	 * @return				Returns a vector containing all of the found filenames
	 */
	virtual Common::StringArray GetFilesInDirectory(const Common::String &Path) = 0;
};

#endif
