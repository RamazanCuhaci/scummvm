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

#include "mm/mm1/views/encounter.h"
#include "mm/mm1/game/encounter.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {

Encounter::Encounter() : TextView("Encounter") {
}

bool Encounter::msgFocus(const FocusMessage &msg) {
	_mode = ALERT;
	return true;
}

void Encounter::draw() {
	Game::Encounter &enc = g_globals->_encounters;

	switch (_mode) {
	case ALERT:
		writeString(9, 6, "            ");
		writeString(9, 7, STRING["dialogs.encounter.title"]);
		writeString(9, 8, "            ");

		delaySeconds(2);
		break;

	case SURPRISED_BY_MONSTERS:
		writeString(6, 21, STRING["dialogs.encounter.surprised"]);
		enc._encounterFlag = Game::FORCE_SURPRISED;
		delaySeconds(2);
		break;

	case SURPRISED_MONSTERS:
		writeString(2, 21, STRING["dialogs.encounter.surprise"]);
		writeString(1, 22, STRING["dialogs.encounter.surprise"]);
		break;

	case ENCOUNTER_OPTIONS: {
		// Clear the commands area
		Graphics::ManagedSurface s = getSurface();
		s.fillRect(Common::Rect(31 * 8, 0, 320, 17 * 8), 0);

		// Write the encounter options
		clearLines(20, 24);
		writeString(0, 21, STRING["dialogs.encounter.option1"]);
		writeString(10, 22, STRING["dialogs.encounter.option2"]);
		break;
	}

	case NOWHERE_TO_RUN:
		clearLines(20, 24);
		writeString(11, 21, STRING["dialogs.encounter.nowhere_to_run"]);
		delaySeconds(2);
		break;

	case SURRENDER_FAILED:
		clearLines(20, 24);
		writeString(2, 21, STRING["dialogs.encounter.surrender_failed"]);
		delaySeconds(2);
		break;

	case NO_RESPONSE:
		clearLines(20, 24);
		writeString(12, 21, STRING["dialogs.encounter.no_response"]);
		delaySeconds(2);
		break;

	case BRIBE:
		enc._val1++;
		enc._val3++;
		writeString(5, 21, Common::String::format(
			STRING["dialogs.encounter.give_up"].c_str(),
			_bribeTypeStr.c_str()));
		break;

	case NOT_ENOUGH:
		clearLines(20, 24);
		writeString(14, 21, STRING["dialogs.encounter.not_enough"]);
		delaySeconds(2);
		break;

	case COMBAT:
		clearLines(20, 24);
		writeString(16, 21, STRING["dialogs.encounter.combat"]);
		delaySeconds(2);
		break;

	default:
		break;
	}

	if (_mode != ALERT) {
		// Display the monster
		drawMonster(enc._val8);

		// Write the monster list
		for (uint i = 0; i < enc._monsterList.size(); ++i) {
			writeChar(22, i, 'A' + i);
			writeString(") ");
			writeString(enc._monsterList[i]._name);
		}
	}

	if (_mode == NO_RESPONSE || _mode == NOT_ENOUGH ||
			_mode == COMBAT || _mode == SURPRISED_BY_MONSTERS) {
		if (enc._val2) {
			writeString(8, 23, STRING["dialogs.encounter.alignment_slips"]);
			Sound::sound(SOUND_2);
		}

		_mode = ALIGNMENT_CHECK;
	}
}

void Encounter::drawMonster(int monsterNum) {
	Graphics::ManagedSurface img =
		g_globals->_monsters.getMonsterImage(monsterNum);
	getSurface().blitFrom(img, Common::Point(64, 16));
}

void Encounter::timeout() {
	const Game::Encounter &enc = g_globals->_encounters;
	const Maps::Map &map = *g_maps->_currentMap;

	switch (_mode) {
	case ALERT:
		// Finished displaying initial encounter alert
		if (enc._encounterFlag < 0 /* FORCE_SURPRISED */) {
			_mode = SURPRISED_BY_MONSTERS;
		} else if (enc._encounterFlag == Game::NORMAL_SURPRISED ||
			/* ENCOUNTER_OPTIONS */
			g_engine->getRandomNumber(1, 100) > map[21]) {
			// Potentially surprised. Check for guard dog spell
			if (g_globals->_spells._s.guard_dog ||
				g_engine->getRandomNumber(1, 100) > map[20])
				_mode = ENCOUNTER_OPTIONS;
			else
				_mode = SURPRISED_BY_MONSTERS;
		} else {
			_mode = SURPRISED_MONSTERS;
		}
		break;

	case ALIGNMENT_CHECK:
		_mode = BATTLE;
		break;

	default:
		break;
	}

	redraw();
}

bool Encounter::msgKeypress(const KeypressMessage &msg) {
	const Maps::Map &map = *g_maps->_currentMap;

	switch (_mode) {
	case SURPRISED_MONSTERS:
		if (msg.keycode == Common::KEYCODE_y) {
			_mode = ENCOUNTER_OPTIONS;
			redraw();
		} else if (msg.keycode == Common::KEYCODE_n) {
			encounterEnded();
		}
		break;

	case ENCOUNTER_OPTIONS:
		switch (msg.keycode) {
		case Common::KEYCODE_a:
			attack();
			break;
		case Common::KEYCODE_b:
			bribe();
			break;
		case Common::KEYCODE_r:
			retreat();
			break;
		case Common::KEYCODE_s:
			surrender();
			break;
		default:
			break;
		}
		break;

	case BRIBE:
		if (msg.keycode == Common::KEYCODE_y) {
			if (getRandomNumber(1, 100) > map[Maps::MAP_BRIBE_THRESHOLD]) {
				_mode = NOT_ENOUGH;
				redraw();
			} else {
				switch (_bribeType) {
				case BRIBE_GOLD:
					g_globals->_party.clearPartyGold();
					break;
				case BRIBE_GEMS:
					g_globals->_party.clearPartyGems();
					break;
				case BRIBE_FOOD:
					g_globals->_party.clearPartyFood();
					break;
				}

				encounterEnded();
			}
		} else if (msg.keycode == Common::KEYCODE_n) {
			_mode = ENCOUNTER_OPTIONS;
			redraw();
		}
		break;

	default:
		break;
	}

	return true;
}

void Encounter::encounterEnded() {
	close();
	g_events->send("Game", GameMessage("UPDATE"));
}

void Encounter::attack() {
	const Game::Encounter &enc = g_globals->_encounters;

	if (!enc.checkSurroundParty() || !enc.checkSurroundParty() ||
			!enc.checkSurroundParty()) {
		increaseAlignments();
	}

	_mode = COMBAT;
	redraw();
}

void Encounter::bribe() {
	const Game::Encounter &enc = g_globals->_encounters;

	if (enc.checkSurroundParty()) {
		if (!enc._val3)
			decreaseAlignments();

		_mode = NO_RESPONSE;
		redraw();

	} else if (getRandomNumber(1, 7) == 5 && !enc._val1) {
		// Rare chance to abort combat immediately
		encounterEnded();

	} else {
		_mode = BRIBE;

		int val = getRandomNumber(1, 100);
		if (val < 6) {
			_bribeType = BRIBE_GEMS;
			_bribeTypeStr = STRING["dialogs.encounter.gems"];
		} else if (val < 16) {
			_bribeType = BRIBE_FOOD;
			_bribeTypeStr = STRING["dialogs.encounter.food"];
		} else {
			_bribeType = BRIBE_GOLD;
			_bribeTypeStr = STRING["dialogs.encounter.gold"];
		}

		redraw();
	}
}

void Encounter::retreat() {
	const Game::Encounter &enc = g_globals->_encounters;
	int val = getRandomNumber(1, 110);

	if (val >= 100) {
		flee();
	} else if (val > Maps::MAP_FLEE_THRESHOLD) {
		_mode = NOWHERE_TO_RUN;
		redraw();
	} else if (enc._val5 < (int)g_globals->_party.size() || !enc.checkSurroundParty()) {
		flee();
	}
}

void Encounter::surrender() {
	const Game::Encounter &enc = g_globals->_encounters;
	const Maps::Map &map = *g_maps->_currentMap;

	if (getRandomNumber(1, 100) > map[Maps::MAP_SURRENDER_THRESHOLD] ||
			getRandomNumber(1, 100) > enc._fleeThreshold) {
		_mode = SURRENDER_FAILED;
		redraw();
	} else {
		g_maps->_mapPos.x = map[Maps::MAP_SURRENDER_X];
		g_maps->_mapPos.y = map[Maps::MAP_SURRENDER_Y];

		// Randomly remove food, gems, or gold from the party
		int val = getRandomNumber(1, 200);
		if (val < 51) {
		} else if (val < 151) {
			g_globals->_party.clearPartyGold();
		} else if (val < 161) {
			g_globals->_party.clearPartyGems();
		} else if (val < 171) {
			g_globals->_party.clearPartyFood();
		} else if (val < 191) {
			g_globals->_party.clearPartyFood();
			g_globals->_party.clearPartyGold();
		} else if (val < 200) {
			g_globals->_party.clearPartyGold();
			g_globals->_party.clearPartyGems();
		} else {
			g_globals->_party.clearPartyGems();
			g_globals->_party.clearPartyFood();
			g_globals->_party.clearPartyGold();
		}

		encounterEnded();
	}
}

void Encounter::flee() {
	const Maps::Map &map = *g_maps->_currentMap;
	g_maps->_mapPos.x = map[Maps::MAP_FLEE_X];
	g_maps->_mapPos.y = map[Maps::MAP_FLEE_Y];
	encounterEnded();
}

void Encounter::decreaseAlignments() {
	Game::Encounter &enc = g_globals->_encounters;

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		g_globals->_currCharacter = &c;

		if (c._alignmentCtr) {
			--c._alignmentCtr;
			if (c._alignmentCtr == 0)
				enc.changeCharAlignment(GOOD);
			else if (c._alignmentCtr == 16)
				enc.changeCharAlignment(NEUTRAL);
		}
	}
}

void Encounter::increaseAlignments() {
	Game::Encounter &enc = g_globals->_encounters;

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		g_globals->_currCharacter = &c;

		if (c._alignmentCtr != 32) {
			++c._alignmentCtr;
			if (c._alignmentCtr == 32)
				enc.changeCharAlignment(EVIL);
			else if (c._alignmentCtr == 16)
				enc.changeCharAlignment(NEUTRAL);
		}
	}
}

} // namespace Views
} // namespace MM1
} // namespace MM
