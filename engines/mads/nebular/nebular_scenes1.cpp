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
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/nebular/nebular_scenes1.h"

namespace MADS {

namespace Nebular {

void Scene1xx::setAAName() {
	int idx = (_scene->_nextSceneId > 103 && _scene->_nextSceneId < 112) ? 1 : 0;
	_game._aaName = Resources::formatAAName(idx);
}

void Scene1xx::sceneEntrySound() {
	if (_vm->_musicFlag) {
		switch (_scene->_nextSceneId) {
		case 101:
			_vm->_sound->command(11);
			break;
		case 102:
			_vm->_sound->command(12);
			break;
		case 103:
			_vm->_sound->command(3);
			_vm->_sound->command(25);
			break;
		case 109:
			_vm->_sound->command(13);
			break;
		case 110:
			_vm->_sound->command(10);
			break;
		case 111:
			_vm->_sound->command(3);
			break;
		case 112:
			_vm->_sound->command(15);
			break;
		default:
			if (_scene->_priorSceneId < 104 || _scene->_priorSceneId > 108)
				_vm->_sound->command(10);
			break;
		}
	}
}

void Scene1xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;
	if (_scene->_nextSceneId <= 103 || _scene->_nextSceneId == 111) {
		if (_globals[0] == SEX_FEMALE) {
			_game._player._spritesPrefix = "ROX";
		} else {
			_game._player._spritesPrefix = "RXM";
			_globals[0] = SEX_MALE;
		}
	} else if (_scene->_nextSceneId <= 110) {
		_game._player._spritesPrefix = "RXSW";
		_globals[0] = SEX_UNKNOWN;
	} else if (_scene->_nextSceneId == 112) {
		_game._player._spritesPrefix = "";
	}

	if (oldName == _game._player._spritesPrefix)
		_game._player._spritesChanged = true;
	if (_scene->_nextSceneId == 105 || (_scene->_nextSceneId == 109 && _globals[15])) {
		_game._player._spritesChanged = true;
		_game._v3 = 0;
	}

	_game._player._unk3 = 0;
	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

/*------------------------------------------------------------------------*/

void Scene101::setup() {
}

void Scene101::enter() {
}

void Scene101::step() {
}

void Scene101::preActions() {
}

void Scene101::actions() {
}

void Scene101::postActions() {
}

/*------------------------------------------------------------------------*/

void Scene103::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene103::enter() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', -1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('h', -1));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('m', -1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('t', -1));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('r', -1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('c', -1));

	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RXMBD_2");
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites("*RXMRD_3");
	_globals._spriteIndexes[15] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[0], false, 7, 0, 1, 0);
	_globals._spriteIndexes[16] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[1], false, 6, 0, 2, 0);
	_scene->_sequences.setDepth(_globals._spriteIndexes[16], 0);

	_globals._spriteIndexes[17] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[2], false, 6, 0, 0, 25);	
	_scene->_sequences.addSubEntry(_globals._spriteIndexes[17], SM_FRAME_INDEX, 2, 72);
	_globals._spriteIndexes[18] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[3], false, 6, 0, 1, 37);
	_scene->_sequences.addSubEntry(_globals._spriteIndexes[18], SM_FRAME_INDEX, 2, 73);

	_globals._spriteIndexes[23] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[8], false, 8);
	_globals._spriteIndexes[22] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[7], false, 6);
	_globals._spriteIndexes[19] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[4], false, 6);
	_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[5], false, 6);

	if (_game._objects.isInRoom(OBJ_TIMER_MODULE)) {
		_vm->_game->_scene._hotspots.activate(371, false);
	} else {
		_globals._spriteIndexes[26] = _scene->_sequences.addSpriteCycle(
			_globals._spriteIndexes[11], false, 6);
	}

	if (_game._objects.isInRoom(OBJ_REBREATHER)) {
		_vm->_game->_scene._hotspots.activate(289, false);
	} else {
		_globals._spriteIndexes[25] = _scene->_sequences.addSpriteCycle(
			_globals._spriteIndexes[10], false, 6);
	}
	
	if (_globals[11]) {
		_globals._spriteIndexes[24] = _scene->_sequences.addSpriteCycle(
			_globals._spriteIndexes[9], false, 6);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[24], -2, -2);
		_scene->_hotspots.activate(362, false);
	}

	if (_scene->_priorSceneId != -2)
		_game._player._playerPos = Common::Point(237, 74);
	if (_scene->_priorSceneId == 102) {
		_game._player._stepEnabled = false;

		_globals._spriteIndexes[21] = _scene->_sequences.addReverseSpriteCycle(
			_globals._spriteIndexes[6], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_0, 0, 70);
	}

	sceneEntrySound();
	_vm->_game->loadQuoteSet(70, 51, 71, 7, 73, 0);

	if (!_game._visitedScenes._sceneRevisited) {
		int msgIndex = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 
			34, 0, 120, _game.getQuote(70));
		_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
	}

	if (_scene->_priorSceneId == 102)
		_vm->_sound->command(20);

	_vm->_palette->setEntry(252, 63, 63, 10);
	_vm->_palette->setEntry(253, 45, 45, 10);
	_globals._v0 = 0;
	_globals._frameTime = _scene->_frameStartTime;
}

void Scene103::step() {
	Common::Point pt;
	int dist;

	switch (_vm->_game->_abortTimers) {
	case 70:
		_vm->_game->_player._stepEnabled = true;
		break;

	case 72:
		pt = _vm->_game->_player._playerPos;
		dist = _vm->hypotenuse(pt.x - 58, pt.y - 93);
		_vm->_sound->command(27, (dist * -128 / 378) + 127);
		break;

	case 73:
		pt = _vm->_game->_player._playerPos;
		dist = _vm->hypotenuse(pt.x - 266, pt.y - 81);
		_vm->_sound->command(27, (dist * -127 / 378) + 127);
		break;

	default:
		break;
	}
	
	if (_globals._frameTime <= _scene->_frameStartTime) {
		pt = _vm->_game->_player._playerPos;
		int dist = _vm->hypotenuse(pt.x - 79, pt.y - 137);
		_vm->_sound->command(29, (dist * -127 / 378) + 127);

		pt = _vm->_game->_player._playerPos;
		dist = _vm->hypotenuse(pt.x - 69, pt.y - 80);
		_vm->_sound->command(30, (dist * -127 / 378) + 127);

		pt = _vm->_game->_player._playerPos;
		dist = _vm->hypotenuse(pt.x - 266, pt.y - 138);
		_vm->_sound->command(32, (dist * -127 / 378) + 127);

		_globals._frameTime = _scene->_frameStartTime + _vm->_game->_player._ticksAmount;
	}
}

void Scene103::preActions() {
}

void Scene103::actions() {
	if (_action._savedLookFlag) {
		Dialog::show(10322);
	} else if (_action.isAction(395, 110, 0)) {
		switch (_vm->_game->_abortTimers) {
		case 0:
			_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(
				_globals._spriteIndexes[6], false, 6, 1);
			_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_0, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			break;

		case 1:
			_vm->_sound->command(1);
			_scene->_nextSceneId = 102;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	}
	else if (_action.isAction(OBJ_REBREATHER, 371, 0) && _game._objects.isInRoom(OBJ_TIMER_MODULE)) {
		switch (_vm->_game->_abortTimers) {
		case 0:
			_scene->changeDepthSurface(1);
			_globals._spriteIndexes[28] = _scene->_sequences.startReverseCycle(
				_globals._spriteIndexes[13], false, 3, 2);
			_game.sub71312(_globals._spriteIndexes[28]);

			_scene->_sequences.addSubEntry(_globals._spriteIndexes[28], SM_FRAME_INDEX, 7, 1);
			_scene->_sequences.addSubEntry(_globals._spriteIndexes[28], SM_0, 0, 2);
			_vm->_game->_player._visible = false;
			_vm->_game->_player._stepEnabled = false;
			break;

		case 1:
			_scene->_sequences.remove(_globals._spriteIndexes[26]);
			break;

		case 2:
			_vm->_sound->command(22);
			_game._objects.addToInventory(OBJ_TIMER_MODULE);
			_scene->changeDepthSurface(0);
			_scene->drawElements(kTransitionNone, false);
			_scene->_hotspots.activate(371, false);
			_vm->_game->_player._visible = true;
			_vm->_game->_player._stepEnabled = true;
			_vm->_dialogs->showPicture(OBJ_REBREATHER, 804);
			break;

		default:
			break;
		}
	} else if (_action.isAction(OBJ_REBREATHER, 289, 0) && _game._objects.isInRoom(OBJ_REBREATHER)) {
		switch (_vm->_game->_abortTimers) {
		case 0:
			_scene->changeDepthSurface(1);
			_globals._spriteIndexes[27] = _scene->_sequences.startReverseCycle(
				_globals._spriteIndexes[12], false, 3, 2);
			_game.sub71312(_globals._spriteIndexes[27]);

			_scene->_sequences.addSubEntry(_globals._spriteIndexes[27], SM_FRAME_INDEX, 6, 1);
			_scene->_sequences.addSubEntry(_globals._spriteIndexes[27], SM_0, 0, 2);
			_vm->_game->_player._visible = false;
			_vm->_game->_player._stepEnabled = false;
			break;

		case 1:
			_scene->_sequences.remove(_globals._spriteIndexes[25]);
			break;

		case 2:
			_vm->_sound->command(22);
			_game._objects.addToInventory(OBJ_REBREATHER);
			_scene->_hotspots.activate(289, false);
			_vm->_game->_player._visible = true;
			_vm->_game->_player._stepEnabled = true;
			_vm->_dialogs->showPicture(OBJ_REBREATHER, 804);
			break;

		default:
			break;
		}
	} else if (_action.isAction(OBJ_STUFFED_FISH, 362, 0)) {
		Dialog::show(10301);
	} else if (_action.isAction(OBJ_REBREATHER, 362, 0)) {
		if (!_vm->_game->_abortTimers)
			_vm->_sound->command(31);

		if (_vm->_game->_abortTimers2 < 2) {
			_globals._spriteIndexes[24] = _scene->_sequences.addSpriteCycle(
				_globals._spriteIndexes[9], false, 6,
				_vm->_game->_abortTimers < 1 ? 1 : 0);
			if (_vm->_game->_abortTimers) {
				_scene->_sequences.setAnimRange(_globals._spriteIndexes[24], -1, 0);
				Common::String msg = _game.getQuote(51);
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 18, 0, 60, msg);
				_scene->_sequences.addTimer(120, _vm->_game->_abortTimers);
			} else {
				_scene->_sequences.addSubEntry(_globals._spriteIndexes[24],
					SM_0, 0, 1);
			}

			_game._player._stepEnabled = _game._abortTimers == 2;
			_globals[11] = -1;
			if (_game._abortTimers == 2) {
				Dialog::show(1030);
				_scene->_hotspots.activate(362, false);
			}
		}
	} else if (_action.isAction(OBJ_STUFFED_FISH, 250, 0)) {
		Dialog::show(!_globals[11] ? 10323 : 10303);
	}
	else if (_action.isAction(OBJ_PLANT_STALK, 27, 0)) {
		switch (_vm->_game->_abortTimers) {
		case 0: {
			_game._player._stepEnabled = false;
			Common::String msg = _game.getQuote(71);
			_scene->_kernelMessages.add(Common::Point(), 0x1110, 18, 1, 120, msg);
			break;
		}

		case 1: {
			Common::String msg = _game.getQuote(72);
			_scene->_kernelMessages.add(Common::Point(310, 132), 0xFDFC, 16, 2, 120, msg);
			_globals._v0 = 0;
			break;
		}
			
		case 2:
			_scene->_kernelMessages.reset();
			_scene->_sequences.addTimer(1, 3);
			break;

		case 3:
			_game._player._stepEnabled = true;
			Dialog::show(10306);
			break;

		default:
			break;
		}
	} else if (!_action.isAction(OBJ_STUFFED_FISH, 27, 0)) {
		Dialog::show(10304);
	} else if (!_action.isAction(OBJ_STUFFED_FISH, 36, 0)) {
		Dialog::show(10307);
	} else if (!_action.isAction(OBJ_STUFFED_FISH, 55, 0)) {
		Dialog::show(10308);
	} else if (!_action.isAction(OBJ_REBREATHER, 315, 0)) {
		Dialog::show(10309);
	} else if (!_action.isAction(OBJ_REBREATHER, 85, 0)) {
		Dialog::show(10310);
	} else if (!_action.isAction(OBJ_STUFFED_FISH, 144, 0)) {
		Dialog::show(10312);
	} else if (!_action.isAction(OBJ_BIG_LEAVES, 144, 0)) {
		Dialog::show(10313);
	} else if (!_action.isAction(OBJ_BONE, 27, 0)) {
		Dialog::show(10314);
	} else if (!_action.isAction(OBJ_STUFFED_FISH, 310, 0)) {
		Dialog::show(10315);
	} else if (!_action.isAction(OBJ_STUFFED_FISH, 178, 0)) {
		Dialog::show(10316);
	} else if (!_action.isAction(OBJ_STUFFED_FISH, 283, 0)) {
		Dialog::show(10317);
	} else if (!_action.isAction(OBJ_STUFFED_FISH, 120, 0)) {
		Dialog::show(10318);
	} else if (_action.isAction(OBJ_STUFFED_FISH, 289, 0) &&
			_game._objects.isInInventory(OBJ_REBREATHER)) {
		Dialog::show(10319);
	} else if (_action.isAction(OBJ_STUFFED_FISH, 371, 0) &&
			_game._objects.isInInventory(OBJ_TIMER_MODULE)) {
		Dialog::show(10320);
	} else if (!_action.isAction(OBJ_STUFFED_FISH, 137, 0)) {
		Dialog::show(10321);
	} else if (_action.isAction(OBJ_STUFFED_FISH, 409, 0)) {
		Dialog::show(_game._objects.isInInventory(OBJ_TIMER_MODULE) ? 10324 : 10325);
	}

	_action._inProgress = false;
}

void Scene103::postActions() {
	if (_action.isAction(27) && !_action.isAction(13)) {
		Dialog::show(0x2841);
		_action._inProgress = false;
	} else {
		if (_action.isAction(7, 85, 144)) {
			Common::String msg = _game.getQuote(73);
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110,
				34, 0, 120, msg);
			_action._inProgress = false;
		}
	}
}

} // End of namespace Nebular

} // End of namespace MADS
