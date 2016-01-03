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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef WAGE_H
#define WAGE_H

#include "engines/engine.h"
#include "common/debug.h"
#include "gui/debugger.h"
#include "common/endian.h"
#include "common/rect.h"
#include "common/macresman.h"
#include "common/random.h"

struct ADGameDescription;

namespace Wage {

class Console;
class Chr;
class Designed;
class Gui;
class Obj;
class Scene;
class World;

using Common::String;

enum OperandType {
	OBJ = 0,
	CHR = 1,
	SCENE = 2,
	NUMBER = 3,
	STRING = 4,
	CLICK_INPUT = 5,
	TEXT_INPUT = 6,
	UNKNOWN = 100
};

// our engine debug levels
enum {
	kWageDebugExample = 1 << 0,
	kWageDebugExample2 = 1 << 1
	// next new level must be 1 << 2 (4)
	// the current limitation is 32 debug levels (1 << 31 is the last one)
};

enum {
	kColorBlack = 0,
	kColorGray  = 1,
	kColorWhite = 2,
	kColorGreen = 3
};

Common::String readPascalString(Common::SeekableReadStream *in);
Common::Rect *readRect(Common::SeekableReadStream *in);
const char *getIndefiniteArticle(String &word);

typedef Common::Array<byte *> Patterns;

class WageEngine : public Engine {
public:
	WageEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~WageEngine();

	virtual bool hasFeature(EngineFeature f) const;

	virtual Common::Error run();

	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();

	const char *getGameFile() const;

private:
	bool loadWorld(Common::MacResManager *resMan);
	void performInitialSetup();
	void wearObjs(Chr *chr);
	void processTurn(Common::String *textInput, Designed *clickInput);
	void processTurnInternal(Common::String *textInput, Designed *clickInput);
	void regen();
	void performCombatAction(Chr *npc, Chr *player);

public:
	Common::RandomSource *_rnd;

	Gui *_gui;
	World *_world;

	Scene *_lastScene;
	//PrintStream out;
	int _loopCount;
	int _turn;
	Chr *_monster;
	Chr *_running;
	Obj *_offer;
	int _aim;
	bool _temporarilyHidden;
	bool _isGameOver;

	void playSound(String soundName);
	void setMenu(String soundName);
	void appendText(String &str);
	void gameOver();
	Obj *getOffer();
	Chr *getMonster();
	void processEvents();
	Scene *getSceneByName(String &location);
	void onMove(Designed *what, Designed *from, Designed *to);
	void encounter(Chr *player, Chr *chr);
	void redrawScene();

private:
	Console *_console;

	const ADGameDescription *_gameDescription;

	Common::MacResManager *_resManager;

	bool _commandWasQuick;

};

// Example console class
class Console : public GUI::Debugger {
public:
	Console(WageEngine *vm) {}
	virtual ~Console(void) {}
};

} // End of namespace Wage

#endif
