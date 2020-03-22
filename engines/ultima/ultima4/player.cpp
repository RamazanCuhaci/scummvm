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

#include "ultima/ultima4/player.h"
#include "ultima/ultima4/annotation.h"
#include "ultima/ultima4/armor.h"
#include "ultima/ultima4/combat.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/game.h"
#include "ultima/ultima4/location.h"
#include "ultima/ultima4/mapmgr.h"
#include "ultima/ultima4/names.h"
#include "ultima/ultima4/graphics/tilemap.h"
#include "ultima/ultima4/graphics/tileset.h"
#include "ultima/ultima4/types.h"
#include "ultima/ultima4/utils.h"
#include "ultima/ultima4/weapon.h"

namespace Ultima {
namespace Ultima4 {

bool isPartyMember(Object *punknown) {
	PartyMember *pm;
	if ((pm = dynamic_cast<PartyMember *>(punknown)) != NULL)
		return true;
	else
		return false;
}

/**
 * PartyMember class implementation
 */
PartyMember::PartyMember(Party *p, SaveGamePlayerRecord *pr) :
	Creature(tileForClass(pr->_class)),
	_player(pr),
	_party(p) {
	/* FIXME: we need to rename movement behaviors */
	setMovementBehavior(MOVEMENT_ATTACK_AVATAR);
	this->_ranged = Weapon::get(pr->_weapon)->getRange() ? 1 : 0;
	setStatus(pr->_status);
}

PartyMember::~PartyMember() {
}

/**
 * Notify the party that this player has changed somehow
 */
void PartyMember::notifyOfChange() {
	if (_party) {
		_party->notifyOfChange(this);
	}
}

/**
 * Provides some translation information for scripts
 */
Common::String PartyMember::translate(Std::vector<Common::String> &parts) {
	if (parts.size() == 0)
		return "";
	else if (parts.size() == 1) {
		if (parts[0] == "hp")
			return xu4_to_string(getHp());
		else if (parts[0] == "max_hp")
			return xu4_to_string(getMaxHp());
		else if (parts[0] == "mp")
			return xu4_to_string(getMp());
		else if (parts[0] == "max_mp")
			return xu4_to_string(getMaxMp());
		else if (parts[0] == "str")
			return xu4_to_string(getStr());
		else if (parts[0] == "dex")
			return xu4_to_string(getDex());
		else if (parts[0] == "int")
			return xu4_to_string(getInt());
		else if (parts[0] == "exp")
			return xu4_to_string(getExp());
		else if (parts[0] == "name")
			return getName();
		else if (parts[0] == "weapon")
			return getWeapon()->getName();
		else if (parts[0] == "armor")
			return getArmor()->getName();
		else if (parts[0] == "sex") {
			Common::String var((char)getSex());
			return var;
		} else if (parts[0] == "class")
			return getClassName(getClass());
		else if (parts[0] == "level")
			return xu4_to_string(getRealLevel());
	} else if (parts.size() == 2) {
		if (parts[0] == "needs") {
			if (parts[1] == "cure") {
				if (getStatus() == STAT_POISONED)
					return "true";
				else return "false";
			} else if (parts[1] == "heal" || parts[1] == "fullheal") {
				if (getHp() < getMaxHp())
					return "true";
				else return "false";
			} else if (parts[1] == "resurrect") {
				if (getStatus() == STAT_DEAD)
					return "true";
				else return "false";
			}
		}
	}
	return "";
}

int PartyMember::getHp() const      {
	return _player->_hp;
}

/**
 * Determine the most magic points a character could have
 * given his class and intelligence.
 */
int PartyMember::getMaxMp() const {
	int max_mp = -1;

	switch (_player->_class) {
	case CLASS_MAGE:            /*  mage: 200% of int */
		max_mp = _player->_intel * 2;
		break;

	case CLASS_DRUID:           /* druid: 150% of int */
		max_mp = _player->_intel * 3 / 2;
		break;

	case CLASS_BARD:            /* bard, paladin, ranger: 100% of int */
	case CLASS_PALADIN:
	case CLASS_RANGER:
		max_mp = _player->_intel;
		break;

	case CLASS_TINKER:          /* tinker: 50% of int */
		max_mp = _player->_intel / 2;
		break;

	case CLASS_FIGHTER:         /* fighter, shepherd: no mp at all */
	case CLASS_SHEPHERD:
		max_mp = 0;
		break;

	default:
		ASSERT(0, "invalid player class: %d", _player->_class);
	}

	/* mp always maxes out at 99 */
	if (max_mp > 99)
		max_mp = 99;

	return max_mp;
}

const Weapon *PartyMember::getWeapon() const {
	return Weapon::get(_player->_weapon);
}
const Armor *PartyMember::getArmor() const   {
	return Armor::get(_player->armor);
}
Common::String PartyMember::getName() const          {
	return _player->name;
}
SexType PartyMember::getSex() const          {
	return _player->_sex;
}
ClassType PartyMember::getClass() const      {
	return _player->_class;
}

CreatureStatus PartyMember::getState() const {
	if (getHp() <= 0)
		return MSTAT_DEAD;
	else if (getHp() < 24)
		return MSTAT_FLEEING;
	else
		return MSTAT_BARELYWOUNDED;
}

/**
 * Determine what level a character has.
 */
int PartyMember::getRealLevel() const {
	return _player->_hpMax / 100;
}

/**
 * Determine the highest level a character could have with the number
 * of experience points he has.
 */
int PartyMember::getMaxLevel() const {
	int level = 1;
	int next = 100;

	while (_player->_xp >= next && level < 8) {
		level++;
		next <<= 1;
	}

	return level;
}

/**
 * Adds a status effect to the player
 */
void PartyMember::addStatus(StatusType s) {
	Creature::addStatus(s);
	_player->_status = _status.back();
	notifyOfChange();
}

/**
 * Adjusts the player's mp by 'pts'
 */
void PartyMember::adjustMp(int pts) {
	AdjustValueMax(_player->_mp, pts, getMaxMp());
	notifyOfChange();
}

/**
 * Advances the player to the next level if they have enough experience
 */
void PartyMember::advanceLevel() {
	if (getRealLevel() == getMaxLevel())
		return;
	setStatus(STAT_GOOD);
	_player->_hpMax = getMaxLevel() * 100;
	_player->_hp = _player->_hpMax;

	/* improve stats by 1-8 each */
	_player->_str   += xu4_random(8) + 1;
	_player->_dex   += xu4_random(8) + 1;
	_player->_intel += xu4_random(8) + 1;

	if (_player->_str > 50) _player->_str = 50;
	if (_player->_dex > 50) _player->_dex = 50;
	if (_player->_intel > 50) _player->_intel = 50;

	if (_party) {
		_party->setChanged();
		PartyEvent event(PartyEvent::ADVANCED_LEVEL, this);
		event._player = this;
		_party->notifyObservers(event);
	}
}

/**
 * Apply an effect to the party member
 */
void PartyMember::applyEffect(TileEffect effect) {
	if (getStatus() == STAT_DEAD)
		return;

	switch (effect) {
	case EFFECT_NONE:
		break;
	case EFFECT_LAVA:
	case EFFECT_FIRE:
		applyDamage(16 + (xu4_random(32)));

		/*else if (player == ALL_PLAYERS && xu4_random(2) == 0)
		    playerApplyDamage(&(c->saveGame->_players[i]), 10 + (xu4_random(25)));*/
		break;
	case EFFECT_SLEEP:
		putToSleep();
		break;
	case EFFECT_POISONFIELD:
	case EFFECT_POISON:
		if (getStatus() != STAT_POISONED) {
			soundPlay(SOUND_POISON_EFFECT, false);
			addStatus(STAT_POISONED);
		}
		break;
	case EFFECT_ELECTRICITY:
		break;
	default:
		ASSERT(0, "invalid effect: %d", effect);
	}

	if (effect != EFFECT_NONE)
		notifyOfChange();
}

/**
 * Award a player experience points.  Maxs out the players xp at 9999.
 */
void PartyMember::awardXp(int xp) {
	AdjustValueMax(_player->_xp, xp, 9999);
	notifyOfChange();
}

/**
 * Perform a certain type of healing on the party member
 */
bool PartyMember::heal(HealType type) {
	switch (type) {

	case HT_NONE:
		return true;

	case HT_CURE:
		if (getStatus() != STAT_POISONED)
			return false;
		removeStatus(STAT_POISONED);
		break;

	case HT_FULLHEAL:
		if (getStatus() == STAT_DEAD ||
		        _player->_hp == _player->_hpMax)
			return false;
		_player->_hp = _player->_hpMax;
		break;

	case HT_RESURRECT:
		if (getStatus() != STAT_DEAD)
			return false;
		setStatus(STAT_GOOD);
		break;

	case HT_HEAL:
		if (getStatus() == STAT_DEAD ||
		        _player->_hp == _player->_hpMax)
			return false;

		_player->_hp += 75 + (xu4_random(0x100) % 0x19);
		break;

	case HT_CAMPHEAL:
		if (getStatus() == STAT_DEAD ||
		        _player->_hp == _player->_hpMax)
			return false;
		_player->_hp += 99 + (xu4_random(0x100) & 0x77);
		break;

	case HT_INNHEAL:
		if (getStatus() == STAT_DEAD ||
		        _player->_hp == _player->_hpMax)
			return false;
		_player->_hp += 100 + (xu4_random(50) * 2);
		break;

	default:
		return false;
	}

	if (_player->_hp > _player->_hpMax)
		_player->_hp = _player->_hpMax;

	notifyOfChange();

	return true;
}

/**
 * Remove status effects from the party member
 */
void PartyMember::removeStatus(StatusType s) {
	Creature::removeStatus(s);
	_player->_status = _status.back();
	notifyOfChange();
}

void PartyMember::setHp(int hp) {
	_player->_hp = hp;
	notifyOfChange();
}

void PartyMember::setMp(int mp) {
	_player->_mp = mp;
	notifyOfChange();
}

EquipError PartyMember::setArmor(const Armor *a) {
	ArmorType type = a->getType();

	if (type != ARMR_NONE && _party->_saveGame->_armor[type] < 1)
		return EQUIP_NONE_LEFT;
	if (!a->canWear(getClass()))
		return EQUIP_CLASS_RESTRICTED;

	ArmorType oldArmorType = getArmor()->getType();
	if (oldArmorType != ARMR_NONE)
		_party->_saveGame->_armor[oldArmorType]++;
	if (type != ARMR_NONE)
		_party->_saveGame->_armor[type]--;

	_player->armor = type;
	notifyOfChange();

	return EQUIP_SUCCEEDED;
}

EquipError PartyMember::setWeapon(const Weapon *w) {
	WeaponType type = w->getType();

	if (type != WEAP_HANDS && _party->_saveGame->_weapons[type] < 1)
		return EQUIP_NONE_LEFT;
	if (!w->canReady(getClass()))
		return EQUIP_CLASS_RESTRICTED;

	WeaponType old = getWeapon()->getType();
	if (old != WEAP_HANDS)
		_party->_saveGame->_weapons[old]++;
	if (type != WEAP_HANDS)
		_party->_saveGame->_weapons[type]--;

	_player->_weapon = type;
	notifyOfChange();

	return EQUIP_SUCCEEDED;
}

/**
 * Applies damage to a player, and changes status to dead if hit
 * points drop below zero.
 *
 * Byplayer is ignored for now, since it should always be false for U4.  (Is
 * there anything special about being killed by a party member in U5?)  Also
 * keeps interface consistent for virtual base function Creature::applydamage()
 */
bool PartyMember::applyDamage(int damage, bool) {
	int newHp = _player->_hp;

	if (getStatus() == STAT_DEAD)
		return false;

	newHp -= damage;

	if (newHp < 0) {
		setStatus(STAT_DEAD);
		newHp = 0;
	}

	_player->_hp = newHp;
	notifyOfChange();

	if (isCombatMap(g_context->_location->_map) && getStatus() == STAT_DEAD) {
		Coords p = getCoords();
		Map *map = getMap();
		map->_annotations->add(p, Tileset::findTileByName("corpse")->getId())->setTTL(_party->size() * 2);

		if (_party) {
			_party->setChanged();
			PartyEvent event(PartyEvent::PLAYER_KILLED, this);
			event._player = this;
			_party->notifyObservers(event);
		}

		/* remove yourself from the map */
		remove();
		return false;
	}

	return true;
}

int PartyMember::getAttackBonus() const {
	if (Weapon::get(_player->_weapon)->alwaysHits() || _player->_dex >= 40)
		return 255;
	return _player->_dex;
}

int PartyMember::getDefense() const {
	return Armor::get(_player->armor)->getDefense();
}

bool PartyMember::dealDamage(Creature *m, int damage) {
	/* we have to record these now, because if we
	   kill the target, it gets destroyed */
	int m_xp = m->getXp();

	if (!Creature::dealDamage(m, damage)) {
		/* half the time you kill an evil creature you get a karma boost */
		awardXp(m_xp);
		return false;
	}
	return true;
}

/**
 * Calculate damage for an attack.
 */
int PartyMember::getDamage() {
	int maxDamage;

	maxDamage = Weapon::get(_player->_weapon)->getDamage();
	maxDamage += _player->_str;
	if (maxDamage > 255)
		maxDamage = 255;

	return xu4_random(maxDamage);
}

/**
 * Returns the tile that will be displayed when the party
 * member's attack hits
 */
const Common::String &PartyMember::getHitTile() const {
	return getWeapon()->getHitTile();
}

/**
 * Returns the tile that will be displayed when the party
 * member's attack fails
 */
const Common::String &PartyMember::getMissTile() const {
	return getWeapon()->getMissTile();
}

bool PartyMember::isDead() {
	return getStatus() == STAT_DEAD;
}

bool PartyMember::isDisabled() {
	return (getStatus() == STAT_GOOD ||
	        getStatus() == STAT_POISONED) ? false : true;
}

/**
 * Lose the equipped weapon for the player (flaming oil, ranged daggers, etc.)
 * Returns the number of weapons left of that type, including the one in
 * the players hand
 */
int PartyMember::loseWeapon() {
	int weapon = _player->_weapon;

	notifyOfChange();

	if (_party->_saveGame->_weapons[weapon] > 0)
		return (--_party->_saveGame->_weapons[weapon]) + 1;
	else {
		_player->_weapon = WEAP_HANDS;
		return 0;
	}
}

/**
 * Put the party member to sleep
 */
void PartyMember::putToSleep() {
	if (getStatus() != STAT_DEAD) {
		soundPlay(SOUND_SLEEP, false);
		addStatus(STAT_SLEEPING);
		setTile(Tileset::findTileByName("corpse")->getId());
	}
}

/**
 * Wakes up the party member
 */
void PartyMember::wakeUp() {
	removeStatus(STAT_SLEEPING);
	setTile(tileForClass(getClass()));
}

MapTile PartyMember::tileForClass(int klass) {
	const char *name = NULL;

	switch (klass) {
	case CLASS_MAGE:
		name = "mage";
		break;
	case CLASS_BARD:
		name = "bard";
		break;
	case CLASS_FIGHTER:
		name = "fighter";
		break;
	case CLASS_DRUID:
		name = "druid";
		break;
	case CLASS_TINKER:
		name = "tinker";
		break;
	case CLASS_PALADIN:
		name = "paladin";
		break;
	case CLASS_RANGER:
		name = "ranger";
		break;
	case CLASS_SHEPHERD:
		name = "shepherd";
		break;
	default:
		ASSERT(0, "invalid class %d in tileForClass", klass);
	}

	const Tile *tile = Tileset::get("base")->getByName(name);
	ASSERT(tile, "no tile found for class %d", klass);
	return tile->getId();
}

/**
 * Party class implementation
 */
Party::Party(SaveGame *s) : _saveGame(s), _transport(0), _torchDuration(0), _activePlayer(-1) {
	if (MAP_DECEIT <= _saveGame->_location && _saveGame->_location <= MAP_ABYSS)
		_torchDuration = _saveGame->_torchduration;
	for (int i = 0; i < _saveGame->_members; i++) {
		// add the members to the party
		_members.push_back(new PartyMember(this, &_saveGame->_players[i]));
	}

	// set the party's transport (transport value stored in savegame
	// hardcoded to index into base tilemap)
	setTransport(TileMap::get("base")->translate(_saveGame->_transport));
}

Party::~Party() {

}

/**
 * Notify the party that something about it has changed
 */
void Party::notifyOfChange(PartyMember *pm, PartyEvent::Type eventType) {
	setChanged();
	PartyEvent event(eventType, pm);
	notifyObservers(event);
}

Common::String Party::translate(Std::vector<Common::String> &parts) {
	if (parts.size() == 0)
		return "";
	else if (parts.size() == 1) {
		// Translate some different items for the script
		if (parts[0] == "transport") {
			if (g_context->_transportContext & TRANSPORT_FOOT)
				return "foot";
			if (g_context->_transportContext & TRANSPORT_HORSE)
				return "horse";
			if (g_context->_transportContext & TRANSPORT_SHIP)
				return "ship";
			if (g_context->_transportContext & TRANSPORT_BALLOON)
				return "balloon";
		} else if (parts[0] == "gold")
			return xu4_to_string(_saveGame->_gold);
		else if (parts[0] == "food")
			return xu4_to_string(_saveGame->_food);
		else if (parts[0] == "members")
			return xu4_to_string(size());
		else if (parts[0] == "keys")
			return xu4_to_string(_saveGame->_keys);
		else if (parts[0] == "torches")
			return xu4_to_string(_saveGame->_torches);
		else if (parts[0] == "gems")
			return xu4_to_string(_saveGame->_gems);
		else if (parts[0] == "sextants")
			return xu4_to_string(_saveGame->_sextants);
		else if (parts[0] == "food")
			return xu4_to_string((_saveGame->_food / 100));
		else if (parts[0] == "gold")
			return xu4_to_string(_saveGame->_gold);
		else if (parts[0] == "party_members")
			return xu4_to_string(_saveGame->_members);
		else if (parts[0] == "moves")
			return xu4_to_string(_saveGame->_moves);
	} else if (parts.size() >= 2) {
		if (parts[0].findFirstOf("member") == 0) {
			// Make a new parts list, but remove the first item
			Std::vector<Common::String> new_parts = parts;
			new_parts.erase(new_parts.begin());

			// Find the member we'll be working with
			Common::String str = parts[0];
			size_t pos = str.findFirstOf("1234567890");
			if (pos != Common::String::npos) {
				str = str.substr(pos);
				int p_member = (int)strtol(str.c_str(), NULL, 10);

				// Make the party member translate its own stuff
				if (p_member > 0)
					return member(p_member - 1)->translate(new_parts);
			}
		}

		else if (parts.size() == 2) {
			if (parts[0] == "weapon") {
				const Weapon *w = Weapon::get(parts[1]);
				if (w)
					return xu4_to_string(_saveGame->_weapons[w->getType()]);
			} else if (parts[0] == "armor") {
				const Armor *a = Armor::get(parts[1]);
				if (a)
					return xu4_to_string(_saveGame->_armor[a->getType()]);
			}
		}
	}
	return "";
}

void Party::adjustFood(int food) {
	int oldFood = _saveGame->_food;
	AdjustValue(_saveGame->_food, food, 999900, 0);
	if ((_saveGame->_food / 100) != (oldFood / 100)) {
		notifyOfChange();
	}
}

void Party::adjustGold(int gold) {
	AdjustValue(_saveGame->_gold, gold, 9999, 0);
	notifyOfChange();
}

/**
 * Adjusts the avatar's karma level for the given action.  Notify
 * observers with a lost eighth event if the player has lost
 * avatarhood.
 */
void Party::adjustKarma(KarmaAction action) {
	int timeLimited = 0;
	int v, newKarma[VIRT_MAX], maxVal[VIRT_MAX];

	/*
	 * make a local copy of all virtues, and adjust it according to
	 * the game rules
	 */
	for (v = 0; v < VIRT_MAX; v++) {
		newKarma[v] = _saveGame->_karma[v] == 0 ? 100 : _saveGame->_karma[v];
		maxVal[v] = _saveGame->_karma[v] == 0 ? 100 : 99;
	}

	switch (action) {
	case KA_FOUND_ITEM:
		AdjustValueMax(newKarma[VIRT_HONOR], 5, maxVal[VIRT_HONOR]);
		break;
	case KA_STOLE_CHEST:
		AdjustValueMin(newKarma[VIRT_HONESTY], -1, 1);
		AdjustValueMin(newKarma[VIRT_JUSTICE], -1, 1);
		AdjustValueMin(newKarma[VIRT_HONOR], -1, 1);
		break;
	case KA_GAVE_ALL_TO_BEGGAR:
	//  When donating all, you get +3 HONOR in Apple 2, but not in in U4DOS.
	//  TODO: Make this a configuration option.
	//  AdjustValueMax(newKarma[VIRT_HONOR], 3, maxVal[VIRT_HONOR]);
	case KA_GAVE_TO_BEGGAR:
		//  In U4DOS, we only get +2 COMPASSION, no HONOR or SACRIFICE even if
		//  donating all.
		timeLimited = 1;
		AdjustValueMax(newKarma[VIRT_COMPASSION], 2, maxVal[VIRT_COMPASSION]);
		break;
	case KA_BRAGGED:
		AdjustValueMin(newKarma[VIRT_HUMILITY], -5, 1);
		break;
	case KA_HUMBLE:
		timeLimited = 1;
		AdjustValueMax(newKarma[VIRT_HUMILITY], 10, maxVal[VIRT_HUMILITY]);
		break;
	case KA_HAWKWIND:
	case KA_MEDITATION:
		timeLimited = 1;
		AdjustValueMax(newKarma[VIRT_SPIRITUALITY], 3, maxVal[VIRT_SPIRITUALITY]);
		break;
	case KA_BAD_MANTRA:
		AdjustValueMin(newKarma[VIRT_SPIRITUALITY], -3, 1);
		break;
	case KA_ATTACKED_GOOD:
		AdjustValueMin(newKarma[VIRT_COMPASSION], -5, 1);
		AdjustValueMin(newKarma[VIRT_JUSTICE], -5, 1);
		AdjustValueMin(newKarma[VIRT_HONOR], -5, 1);
		break;
	case KA_FLED_EVIL:
		AdjustValueMin(newKarma[VIRT_VALOR], -2, 1);
		break;
	case KA_HEALTHY_FLED_EVIL:
		AdjustValueMin(newKarma[VIRT_VALOR], -2, 1);
		AdjustValueMin(newKarma[VIRT_SACRIFICE], -2, 1);
		break;
	case KA_KILLED_EVIL:
		AdjustValueMax(newKarma[VIRT_VALOR], xu4_random(2), maxVal[VIRT_VALOR]); /* gain one valor half the time, zero the rest */
		break;
	case KA_FLED_GOOD:
		AdjustValueMax(newKarma[VIRT_COMPASSION], 2, maxVal[VIRT_COMPASSION]);
		AdjustValueMax(newKarma[VIRT_JUSTICE], 2, maxVal[VIRT_JUSTICE]);
		break;
	case KA_SPARED_GOOD:
		AdjustValueMax(newKarma[VIRT_COMPASSION], 1, maxVal[VIRT_COMPASSION]);
		AdjustValueMax(newKarma[VIRT_JUSTICE], 1, maxVal[VIRT_JUSTICE]);
		break;
	case KA_DONATED_BLOOD:
		AdjustValueMax(newKarma[VIRT_SACRIFICE], 5, maxVal[VIRT_SACRIFICE]);
		break;
	case KA_DIDNT_DONATE_BLOOD:
		AdjustValueMin(newKarma[VIRT_SACRIFICE], -5, 1);
		break;
	case KA_CHEAT_REAGENTS:
		AdjustValueMin(newKarma[VIRT_HONESTY], -10, 1);
		AdjustValueMin(newKarma[VIRT_JUSTICE], -10, 1);
		AdjustValueMin(newKarma[VIRT_HONOR], -10, 1);
		break;
	case KA_DIDNT_CHEAT_REAGENTS:
		timeLimited = 1;
		AdjustValueMax(newKarma[VIRT_HONESTY], 2, maxVal[VIRT_HONESTY]);
		AdjustValueMax(newKarma[VIRT_JUSTICE], 2, maxVal[VIRT_JUSTICE]);
		AdjustValueMax(newKarma[VIRT_HONOR], 2, maxVal[VIRT_HONOR]);
		break;
	case KA_USED_SKULL:
		/* using the skull is very, very bad... */
		for (v = 0; v < VIRT_MAX; v++)
			AdjustValueMin(newKarma[v], -5, 1);
		break;
	case KA_DESTROYED_SKULL:
		/* ...but destroying it is very, very good */
		for (v = 0; v < VIRT_MAX; v++)
			AdjustValueMax(newKarma[v], 10, maxVal[v]);
		break;
	}

	/*
	 * check if enough time has passed since last virtue award if
	 * action is time limited -- if not, throw away new values
	 */
	if (timeLimited) {
		if (((_saveGame->_moves / 16) >= 0x10000) || (((_saveGame->_moves / 16) & 0xFFFF) != _saveGame->_lastVirtue))
			_saveGame->_lastVirtue = (_saveGame->_moves / 16) & 0xFFFF;
		else
			return;
	}

	/* something changed */
	notifyOfChange();

	/*
	 * return to u4dos compatibility and handle losing of eighths
	 */
	for (v = 0; v < VIRT_MAX; v++) {
		if (maxVal[v] == 100) { /* already an avatar */
			if (newKarma[v] < 100) { /* but lost it */
				_saveGame->_karma[v] = newKarma[v];
				setChanged();
				PartyEvent event(PartyEvent::LOST_EIGHTH, 0);
				notifyObservers(event);
			} else _saveGame->_karma[v] = 0; /* return to u4dos compatibility */
		} else _saveGame->_karma[v] = newKarma[v];
	}
}

/**
 * Apply effects to the entire party
 */
void Party::applyEffect(TileEffect effect) {
	int i;

	for (i = 0; i < size(); i++) {
		switch (effect) {
		case EFFECT_NONE:
		case EFFECT_ELECTRICITY:
			_members[i]->applyEffect(effect);
		case EFFECT_LAVA:
		case EFFECT_FIRE:
		case EFFECT_SLEEP:
			if (xu4_random(2) == 0)
				_members[i]->applyEffect(effect);
		case EFFECT_POISONFIELD:
		case EFFECT_POISON:
			if (xu4_random(5) == 0)
				_members[i]->applyEffect(effect);
		}
	}
}

/**
 * Attempt to elevate in the given virtue
 */
bool Party::attemptElevation(Virtue virtue) {
	if (_saveGame->_karma[virtue] == 99) {
		_saveGame->_karma[virtue] = 0;
		notifyOfChange();
		return true;
	} else
		return false;
}

/**
 * Burns a torch's duration down a certain number of turns
 */
void Party::burnTorch(int turns) {
	_torchDuration -= turns;
	if (_torchDuration <= 0)
		_torchDuration = 0;

	_saveGame->_torchduration = _torchDuration;

	notifyOfChange();
}

/**
 * Returns true if the party can enter the shrine
 */
bool Party::canEnterShrine(Virtue virtue) {
	if (_saveGame->_runes & (1 << (int) virtue))
		return true;
	else
		return false;
}

/**
 * Returns true if the person can join the party
 */
bool Party::canPersonJoin(Common::String name, Virtue *v) {
	int i;

	if (name.empty())
		return 0;

	for (i = 1; i < 8; i++) {
		if (name == _saveGame->_players[i].name) {
			if (v)
				*v = (Virtue) _saveGame->_players[i]._class;
			return true;
		}
	}
	return false;
}

/**
 * Damages the party's ship
 */
void Party::damageShip(unsigned int pts) {
	_saveGame->_shipHull -= pts;
	if ((short)_saveGame->_shipHull < 0)
		_saveGame->_shipHull = 0;

	notifyOfChange();
}

/**
 * Donates 'quantity' gold. Returns true if the donation succeeded,
 * or false if there was not enough gold to make the donation
 */
bool Party::donate(int quantity) {
	if (quantity > _saveGame->_gold)
		return false;

	adjustGold(-quantity);
	if (_saveGame->_gold > 0)
		adjustKarma(KA_GAVE_TO_BEGGAR);
	else adjustKarma(KA_GAVE_ALL_TO_BEGGAR);

	return true;
}

/**
 * Ends the party's turn
 */
void Party::endTurn() {
	int i;

	_saveGame->_moves++;

	for (i = 0; i < size(); i++) {

		/* Handle player status (only for non-combat turns) */
		if ((g_context->_location->_context & CTX_NON_COMBAT) == g_context->_location->_context) {

			/* party members eat food (also non-combat) */
			if (!_members[i]->isDead())
				adjustFood(-1);

			switch (_members[i]->getStatus()) {
			case STAT_SLEEPING:
				if (xu4_random(5) == 0)
					_members[i]->wakeUp();
				break;

			case STAT_POISONED:
				/* SOLUS
				 * shouldn't play poison damage sound in combat,
				 * yet if the PC takes damage just befor combat
				 * begins, the sound is played  after the combat
				 * screen appears
				 */
				soundPlay(SOUND_POISON_DAMAGE, false);
				_members[i]->applyDamage(2);
				break;

			default:
				break;
			}
		}

		/* regenerate magic points */
		if (!_members[i]->isDisabled() && _members[i]->getMp() < _members[i]->getMaxMp())
			_saveGame->_players[i]._mp++;
	}

	/* The party is starving! */
	if ((_saveGame->_food == 0) && ((g_context->_location->_context & CTX_NON_COMBAT) == g_context->_location->_context)) {
		setChanged();
		PartyEvent event(PartyEvent::STARVING, 0);
		notifyObservers(event);
	}

	/* heal ship (25% chance it is healed each turn) */
	if ((g_context->_location->_context == CTX_WORLDMAP) && (_saveGame->_shipHull < 50) && xu4_random(4) == 0)
		healShip(1);
}

/**
 * Adds a chest worth of gold to the party's inventory
 */
int Party::getChest() {
	int gold = xu4_random(50) + xu4_random(8) + 10;
	adjustGold(gold);

	return gold;
}

/**
 * Returns the number of turns a currently lit torch will last (or 0 if no torch lit)
 */
int Party::getTorchDuration() const {
	return _torchDuration;
}

/**
 * Heals the ship's hull strength by 'pts' points
 */
void Party::healShip(unsigned int pts) {
	_saveGame->_shipHull += pts;
	if (_saveGame->_shipHull > 50)
		_saveGame->_shipHull = 50;

	notifyOfChange();
}

/**
 * Returns true if the balloon is currently in the air
 */
bool Party::isFlying() const {
	return (_saveGame->_balloonstate && _torchDuration <= 0);
}

/**
 * Whether or not the party can make an action.
 */
bool Party::isImmobilized() {
	int i;
	bool immobile = true;

	for (i = 0; i < _saveGame->_members; i++) {
		if (!_members[i]->isDisabled())
			immobile = false;
	}

	return immobile;
}

/**
 * Whether or not all the party members are dead.
 */
bool Party::isDead() {
	int i;
	bool dead = true;

	for (i = 0; i < _saveGame->_members; i++) {
		if (!_members[i]->isDead()) {
			dead = false;
		}
	}

	return dead;
}

/**
 * Returns true if the person with that name
 * is already in the party
 */
bool Party::isPersonJoined(Common::String name) {
	int i;

	if (name.empty())
		return false;

	for (i = 1; i < _saveGame->_members; i++) {
		if (name == _saveGame->_players[i].name)
			return true;
	}
	return false;
}

/**
 * Attempts to add the person to the party.
 * Returns JOIN_SUCCEEDED if successful.
 */
CannotJoinError Party::join(Common::String name) {
	int i;
	SaveGamePlayerRecord tmp;

	for (i = _saveGame->_members; i < 8; i++) {
		if (name == _saveGame->_players[i].name) {

			/* ensure avatar is experienced enough */
			if (_saveGame->_members + 1 > (_saveGame->_players[0]._hpMax / 100))
				return JOIN_NOT_EXPERIENCED;

			/* ensure character has enough karma */
			if ((_saveGame->_karma[_saveGame->_players[i]._class] > 0) &&
			        (_saveGame->_karma[_saveGame->_players[i]._class] < 40))
				return JOIN_NOT_VIRTUOUS;

			tmp = _saveGame->_players[_saveGame->_members];
			_saveGame->_players[_saveGame->_members] = _saveGame->_players[i];
			_saveGame->_players[i] = tmp;

			_members.push_back(new PartyMember(this, &_saveGame->_players[_saveGame->_members++]));
			setChanged();
			PartyEvent event(PartyEvent::MEMBER_JOINED, _members.back());
			notifyObservers(event);
			return JOIN_SUCCEEDED;
		}
	}

	return JOIN_NOT_EXPERIENCED;
}

/**
 * Lights a torch with a default duration of 100
 */
bool Party::lightTorch(int duration, bool loseTorch) {
	if (loseTorch) {
		if (g_context->_saveGame->_torches <= 0)
			return false;
		g_context->_saveGame->_torches--;
	}

	_torchDuration += duration;
	_saveGame->_torchduration = _torchDuration;

	notifyOfChange();

	return true;
}

/**
 * Extinguishes a torch
 */
void Party::quenchTorch() {
	_torchDuration = _saveGame->_torchduration = 0;

	notifyOfChange();
}

/**
 * Revives the party after the entire party has been killed
 */
void Party::reviveParty() {
	int i;

	for (i = 0; i < size(); i++) {
		_members[i]->wakeUp();
		_members[i]->setStatus(STAT_GOOD);
		_saveGame->_players[i]._hp = _saveGame->_players[i]._hpMax;
	}

	for (i = ARMR_NONE + 1; i < ARMR_MAX; i++)
		_saveGame->_armor[i] = 0;
	for (i = WEAP_HANDS + 1; i < WEAP_MAX; i++)
		_saveGame->_weapons[i] = 0;
	_saveGame->_food = 20099;
	_saveGame->_gold = 200;
	setTransport(Tileset::findTileByName("avatar")->getId());
	setChanged();
	PartyEvent event(PartyEvent::PARTY_REVIVED, 0);
	notifyObservers(event);
}

MapTile Party::getTransport() const {
	return _transport;
}

void Party::setTransport(MapTile tile) {
	// transport value stored in savegame hardcoded to index into base tilemap
	_saveGame->_transport = TileMap::get("base")->untranslate(tile);
	ASSERT(_saveGame->_transport != 0, "could not generate valid savegame transport for tile with id %d\n", tile._id);

	_transport = tile;

	if (tile.getTileType()->isHorse())
		g_context->_transportContext = TRANSPORT_HORSE;
	else if (tile.getTileType()->isShip())
		g_context->_transportContext = TRANSPORT_SHIP;
	else if (tile.getTileType()->isBalloon())
		g_context->_transportContext = TRANSPORT_BALLOON;
	else g_context->_transportContext = TRANSPORT_FOOT;

	notifyOfChange();
}

void Party::setShipHull(int str) {
	int newStr = str;
	AdjustValue(newStr, 0, 99, 0);

	if (_saveGame->_shipHull != newStr) {
		_saveGame->_shipHull = newStr;
		notifyOfChange();
	}
}

Direction Party::getDirection() const {
	return _transport.getDirection();
}

void Party::setDirection(Direction dir) {
	_transport.setDirection(dir);
}

void Party::adjustReagent(int reagent, int amt) {
	int oldVal = g_context->_saveGame->_reagents[reagent];
	AdjustValue(g_context->_saveGame->_reagents[reagent], amt, 99, 0);

	if (oldVal != g_context->_saveGame->_reagents[reagent]) {
		notifyOfChange();
	}
}

int Party::getReagent(int reagent) const {
	return g_context->_saveGame->_reagents[reagent];
}

short *Party::getReagentPtr(int reagent) const {
	return &g_context->_saveGame->_reagents[reagent];
}

void Party::setActivePlayer(int p) {
	_activePlayer = p;
	setChanged();
	PartyEvent event(PartyEvent::ACTIVE_PLAYER_CHANGED, _activePlayer < 0 ? 0 : _members[_activePlayer]);
	notifyObservers(event);
}

int Party::getActivePlayer() const {
	return _activePlayer;
}

void Party::swapPlayers(int p1, int p2) {
	ASSERT(p1 < _saveGame->_members, "p1 out of range: %d", p1);
	ASSERT(p2 < _saveGame->_members, "p2 out of range: %d", p2);

	SaveGamePlayerRecord tmp = _saveGame->_players[p1];
	_saveGame->_players[p1] = g_context->_saveGame->_players[p2];
	g_context->_saveGame->_players[p2] = tmp;

	syncMembers();

	if (p1 == _activePlayer)
		_activePlayer = p2;
	else if (p2 == _activePlayer)
		_activePlayer = p1;

	notifyOfChange(0);
}

void Party::syncMembers() {
	_members.clear();
	for (int i = 0; i < _saveGame->_members; i++) {
		// add the members to the party
		_members.push_back(new PartyMember(this, &_saveGame->_players[i]));
	}
}

/**
 * Returns the size of the party
 */
int Party::size() const {
	return _members.size();
}

/**
 * Returns a pointer to the party member indicated
 */
PartyMember *Party::member(int index) const {
	return _members[index];
}

} // End of namespace Ultima4
} // End of namespace Ultima
