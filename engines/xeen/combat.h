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

#ifndef XEEN_COMBAT_H
#define XEEN_COMBAT_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "xeen/sprites.h"

namespace Xeen {

#define MAX_NUM_MONSTERS 107

enum DamageType {
	DT_PHYSICAL = 0, DT_MAGICAL = 1, DT_FIRE = 2, DT_ELECTRICAL = 3,
	DT_COLD = 4, DT_POISON = 5, DT_ENERGY = 6, DT_SLEEP = 7,
	DT_FINGEROFDEATH = 8, DT_HOLYWORD = 9, DT_MASS_DISTORTION = 10,
	DT_UNDED = 11, DT_BEASTMASTER = 12, DT_DRAGONSLEEP = 13,
	DT_GOLEMSTOPPER = 14, DT_HYPNOTIZE = 15, DT_INSECT_SPRAY = 16,
	DT_POISON_VALLEY = 17, DT_MAGIC_ARROW = 18
};

enum SpecialAttack {
	SA_NONE = 0, SA_MAGIC = 1, SA_FIRE = 2, SA_ELEC = 3, SA_COLD = 4,
	SA_POISON = 5, SA_ENERGY = 6, SA_DISEASE = 7, SA_INSANE = 8,
	SA_SLEEP = 9, SA_CURSEITEM = 10, SA_INLOVE = 11, SA_DRAINSP = 12,
	SA_CURSE = 13, SA_PARALYZE = 14, SA_UNCONSCIOUS = 15,
	SA_CONFUSE = 16, SA_BREAKWEAPON = 17, SA_WEAKEN = 18,
	SA_ERADICATE = 19, SA_AGING = 20, SA_DEATH = 21, SA_STONE = 22
};

class XeenEngine;
class Character;

class Combat {
private:
	XeenEngine *_vm;
public:
	Common::Array<Character *> _combatParty;
	Common::Array<bool> _charsBlocked;
	Common::Array<int> _charsGone;
	SpriteResource _powSprites;
	int _attackMonsters[26];
	int _monster2Attack;
	int _charsArray1[12];
	bool _monPow[12];
	int _monsterScale[12];
	int _elemPow[12];
	int _elemScale[12];
	Common::Array<int> _speedTable;
	int _shooting[8];
	int _globalCombat;
	int _whosTurn;
	bool _itemFlag;
	int _monsterMap[32][32];
	bool _monsterMoved[MAX_NUM_MONSTERS];
	bool _rangeAttacking[MAX_NUM_MONSTERS];
	int _gmonHit[36];
	bool _monstersAttacking;
	int _combatMode;
	int _monsterIndex;
	bool _partyRan;
	int _whosSpeed;

	void monstersAttack();

	void setupMonsterAttack(int monsterDataIndex, const Common::Point &pt);

	bool monsterCanMove(const Common::Point &pt, int wallShift,
		int v1, int v2, int monsterId);

	void moveMonster(int monsterId, const Common::Point &pt);

	void attackMonster(int monsterId);

	void endAttack();

	void monsterOvercome();
	
	bool stopAttack(const Common::Point &diffPt);
public:
	Combat(XeenEngine *vm);

	void clear();

	void giveCharDamage(int damage, DamageType attackType, int charIndex);

	void moveMonsters();

	void setupCombatParty();

	void setSpeedTable();

	bool allHaveGone() const;

	bool charsCantAct() const;

	Common::String getMonsterDescriptions();

	void attack(Character &c, int v2);

	void block();

	bool castSpell(bool flag);

	void quickFight();

	void giveTreasure();

	void run();
};

} // End of namespace Xeen

#endif	/* XEEN_COMBAT_H */
