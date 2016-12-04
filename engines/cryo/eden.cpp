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

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "gui/EventRecorder.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/fs.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/palette.h"
#include "common/timer.h"

//#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "cryo/defs.h"
#include "cryo/cryo.h"
#include "cryo/platdefs.h"
#include "cryo/cryolib.h"
#include "cryo/eden.h"
#include "cryo/sound.h"

namespace Cryo {

int16 _torchTick = 0;
int16 _glowIndex = 0;
int16 _torchCurIndex = 0;

bool  _allowDoubled = true;
int   _cursCenter = 11;

EdenGame::EdenGame(CryoEngine *vm) : _vm(vm) {
	_adamMapMarkPos = Common::Point(-1, -1);

	_scrollPos = _oldScrollPos = 0;
	_frescoTalk = false;
	torchCursor = false;
	_curBankNum = 0;
	glow_h = glow_w = glow_y = glow_x = 0;
	needPaletteUpdate = false;
	curs_saved = false;
	showBlackBars = false;
	fond_saved = false;
	_bankData = nullptr;
	tyranPtr = nullptr;
	_lastAnimFrameNumb = _curAnimFrameNumb = 0;
	_lastAnimTicks = 0;
	cur_perso_rect = nullptr;
	_numAnimFrames = max_perso_desc = num_img_desc = 0;
	_restartAnimation = animationActive = false;
	_animationDelay = animationIndex = lastAnimationIndex = 0;
	dword_30724 = dword_30728 = _mouthAnimations = animationTable = nullptr;
	_characterBankData = nullptr;
	savedUnderSubtitles = false;
	num_text_lines = 0;
	text_ptr = nullptr;
	textoutptr = textout = nullptr;
	_curSpecialObject = nullptr;
	_lastDialogChoice = false;
	parlemoiNormalFlag = false;
	_closeCharacterDialog = false;
	dword_30B04 = 0;
	lastPhrasesFile = 0;
	dialogSkipFlags = 0;
	voiceSamplesBuffer = nullptr;
	needToFade = false;
	lastMusicNum = 0;
	_mainBankBuf = nullptr;
	_musicBuf = nullptr;
	gameLipsync = nullptr;
	gamePhrases = nullptr;
	gameDialogs = nullptr;
	gameConditions = nullptr;
	sal_buf = nullptr;
	bank_data_buf = nullptr;
	_gameIcons = nullptr;
	gameRooms = nullptr;
	bigfile_header = nullptr;
	glow_buffer = nullptr;
	gameFont = nullptr;
	p_global = nullptr;
	mouse_y_center = mouse_x_center = 0;
	bufferAllocationErrorFl = quit_flag2 = quit_flag3 = false;
	gameStarted = false;
	_soundAllocated = false;
	_musicChannel = _voiceChannel = nullptr;
	_hnmSoundChannel = nullptr;
	voiceSound = nullptr;
	p_view2 = p_underSubtitlesView = p_subtitlesview = p_underBarsView = p_mainview = p_hnmview = nullptr;
	_hnmContext = nullptr;
	_doubledScreen = false;
	curs_x_pan = 0;
	_inventoryScrollDelay = 0;
	curs_y = curs_x = 0;
	_currCursor = 0;
	current_spot = current_spot2 = nullptr;
	pomme_q = false;
	keybd_held = false;
	mouse_held = false;
	normalCursor = false;
	showVideoSubtitle = false;
	specialTextMode = false;
	voiceSamplesSize = 0;
	_animateTalking = false;
	_personTalking = false;
	_musicFadeFlag = 0;
	musicPlaying = false;
	mus_samples_ptr = mus_patterns_ptr = _musSequencePtr = nullptr;
	mus_enabled = false;
	pCurrentObjectLocation = nullptr;
	byte_31D64 = false;
	_noPalette = false;
	_gameLoaded = false;
	memset(tapes, 0, sizeof(tapes));
	confirmMode = 0;
	cur_slider_value_ptr = nullptr;
	lastMenuItemIdLo = 0;
	lastTapeRoomNum = 0;
	cur_slider_x = cur_slider_y = 0;
	destinationRoom = 0;
	word_31E7A = 0;
	word_378CC = 0; //TODO: set by CLComputer_Init to 0
	word_378CE = 0;

	word_32448 = word_3244A = word_3244C = 0;
	flt_32450 = flt_32454 = 0.0;	//TODO: never changed, make consts?
	curs_old_tick = 0;

	invIconsBase = 19;
//	invIconsCount = (_vm->getPlatform() == Common::kPlatformMacintosh) ? 9 : 11;
	invIconsCount = 11;
	roomIconsBase = invIconsBase + invIconsCount;

}

void EdenGame::removeConsole() {
}

void EdenGame::scroll() {
	restoreFriezes();
	p_mainview->_normal._srcLeft = _scrollPos;
	p_mainview->_zoom._srcLeft = _scrollPos;
}

void EdenGame::resetScroll() {
	_oldScrollPos = _scrollPos;
	_scrollPos = 0;
	restoreFriezes();   //TODO: inlined scroll() ?
	p_mainview->_normal._srcLeft = 0;
	p_mainview->_zoom._srcLeft = 0;
}

void EdenGame::scrollFrescoes() {
	if (curs_y > 16 && curs_y < 176) {
		if (curs_x >= 0 && curs_x < 32 && _scrollPos > 3)
			_scrollPos -= 4;
		else if (curs_x > 288 && curs_x < 320 && _scrollPos < p_global->fresqWidth)
			_scrollPos += 4;
	}
	scroll();
}

// Original name: afffresques
void EdenGame::displayFrescoes() {
	useBank(p_global->fresqImgBank);
	noclipax(0, 0, 16);
	useBank(p_global->fresqImgBank + 1);
	noclipax(0, 320, 16);
	needPaletteUpdate = true;
}

void EdenGame::gametofresques() {
	_frescoTalk = false;
	rundcurs();
	saveFriezes();
	displayFrescoes();
	p_global->_displayFlags = DisplayFlags::dfFrescoes;
}

// Original name: dofresques
void EdenGame::doFrescoes() {
	curs_saved = false;
	torchCursor = true;
	glow_x = -1;
	glow_y = -1;
	p_global->_gameFlags |= GameFlags::gfFlag20;
	p_global->ff_D4 = 0;
	p_global->_curObjectId = 0;
	p_global->_iconsIndex = 13;
	p_global->_autoDialog = false;
	gametofresques();
	p_global->frescoNumber = 3;
}

// Original name: finfresques
void EdenGame::endFrescoes() {
	torchCursor = false;
	curs_saved = true;
	p_global->_displayFlags = DisplayFlags::dfFlag1;
	resetScroll();
	p_global->ff_100 = 0xFF;
	maj_salle(p_global->roomNum);
	if (p_global->_phaseNum == 114)
		p_global->narratorSequence = 1;
	p_global->_eventType = EventType::etEvent8;
	showEvents();
}

void EdenGame::scrollMirror() {
	if (curs_y > 16 && curs_y < 165) {
		if (curs_x >= 0 && curs_x < 16) {
			if (_scrollPos > 3) {
				if (_doubledScreen)
					_scrollPos -= 2;
				else
					_scrollPos -= 1;
				scroll();
			}
		} else if (curs_x > 290 && curs_x < 320) {
			if (_scrollPos < 320) {
				if (_doubledScreen)
					_scrollPos += 2;
				else
					_scrollPos += 1;
				scroll();
			}
		}
	}
}

void EdenGame::scrollpano() {
	if (curs_y > 16 && curs_y < 165) {
		if (curs_x >= 0 && curs_x < 16) {
			if (_scrollPos > 3) {
				if (_doubledScreen)
					_scrollPos -= 2;
				else
					_scrollPos -= 1;
			}
		} else if (curs_x > 290 && curs_x < 320) {
			if (_scrollPos < 320) {
				if (_doubledScreen)
					_scrollPos += 2;
				else
					_scrollPos += 1;
			}
		}
	}
	scroll();
}

// Original name: affsuiveur
void EdenGame::displayFollower(Follower *follower, int16 x, int16 y) {
	useBank(follower->_spriteBank);
	noclipax(follower->_spriteNum, x, y + 16);
}

// Original name: persoinmiroir
void EdenGame::characterInMirror() {
	icon_t  *icon1 = &_gameIcons[3];
	icon_t  *icon = &_gameIcons[roomIconsBase];
	Follower *suiveur = followerList;
	int16 num = 1;
	for (int i = 0; i < 16; i++) {
		if (p_global->party & (1 << i))
			num++;
	}
	icon += num;
	icon->sx = -1;
	icon--;
	icon->sx = icon1->sx;
	icon->sy = icon1->sy;
	icon->ex = icon1->ex;
	icon->ey = 170;
	icon->_cursorId = icon1->_cursorId;
	icon->_actionId = icon1->_actionId;
	icon->_objectId = icon1->_objectId;
	icon--;
	displayFollower(suiveur, suiveur->sx, suiveur->sy);
	for (; suiveur->_id != -1; suiveur++) {
		perso_t *perso;
		for (perso = kPersons; perso != &kPersons[PER_UNKN_156]; perso++) {
			if (perso->_id != suiveur->_id)
				continue;

			if (perso->_flags & PersonFlags::pf80)
				continue;

			if ((perso->_flags & PersonFlags::pfInParty) == 0)
				continue;

			if (perso->_roomNum != p_global->roomNum)
				continue;

			icon->sx = suiveur->sx;
			icon->sy = suiveur->sy;
			icon->ex = suiveur->ex;
			icon->ey = suiveur->ey;
			icon->_cursorId = 8;
			icon->_actionId = perso->_actionId;
			icon--;
			displayFollower(suiveur, suiveur->sx, suiveur->sy);
			break;
		}
	}
}

void EdenGame::gametomiroir(byte arg1) {
	if (p_global->_displayFlags != DisplayFlags::dfFlag2) {
		rundcurs();
		restoreFriezes();
		drawTopScreen();
		showObjects();
		saveFriezes();
	}
	int16 bank = p_global->roomBgBankNum;
	unsigned int resNum = bank + 326;
	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		if (bank == 76 || bank == 128)
			resNum = 2487;				// PCIMG.HSQ
	}
	useBank(resNum);
	noclipax(0, 0, 16);
	useBank(resNum + 1);
	noclipax(0, 320, 16);
	characterInMirror();
	needPaletteUpdate = true;
	p_global->_iconsIndex = 16;
	p_global->_autoDialog = false;
	p_global->_displayFlags = DisplayFlags::dfMirror;
	p_global->ff_102 = arg1;
}

void EdenGame::flipMode() {
	if (_personTalking) {
		endpersovox();
		if (p_global->_displayFlags == DisplayFlags::dfPerson) {
			if (p_global->_characterPtr == &kPersons[PER_THOO] && p_global->_phaseNum >= 80)
				af_subtitle();
			else {
				getdatasync();
				load_perso_cour();
				addanim();
				_restartAnimation = true;
				anim_perso();
			}
		} else
			af_subtitle();
		persovox();
	} else {
		if (p_global->_displayFlags != DisplayFlags::dfFrescoes && p_global->_displayFlags != DisplayFlags::dfFlag2) {
			closesalle();
			if (p_global->_displayFlags & DisplayFlags::dfFlag1)
				gametomiroir(1);
			else {
				quitMirror();
				maj_salle(p_global->roomNum);
				if (byte_31D64) {
					dialautoon();
					parle_moi();
				}
				byte_31D64 = false;
			}
		}
	}
}

// Original name: quitmiroir
void EdenGame::quitMirror() {
	rundcurs();
	afficher();
	resetScroll();
	saveFriezes();
	p_global->_displayFlags = DisplayFlags::dfFlag1;
	p_global->ff_100 = 0xFF;
	p_global->_eventType = EventType::etEventC;
	p_global->ff_102 = 1;
}

void EdenGame::clictimbre() {
	flipMode();
}

void EdenGame::clicplanval() {
	if ((p_global->_partyOutside & PersonMask::pmDina) && p_global->_phaseNum == 371) {
		quitMirror();
		maj_salle(p_global->roomNum);
		return;
	}
	if (p_global->roomNum == 8 || p_global->roomNum < 16)
		return;
	rundcurs();
	afficher();
	if (p_global->_displayFlags == DisplayFlags::dfMirror)
		quitMirror();
	deplaval((p_global->roomNum & 0xFF00) | 1); //TODO: check me
}

void EdenGame::gotolieu(goto_t *go) {
	p_global->valleyVidNum = go->arriveVid;
	p_global->travelTime = go->travelTime * 256;
	p_global->stepsToFindAppleFast = 0;
	p_global->_eventType = EventType::etEvent2;
	setChoiceYes();
	showEvents();
	if (!isAnswerYes())
		return;
	if (p_global->ff_113) {
		waitEndSpeak();
		if (!pomme_q)
			close_perso();
	}
	if (go->departVid) {
		bars_out();
		playHNM(go->departVid);
		needToFade = true;
	}
	initPlace(p_global->newRoomNum);
	specialoutside();
	faire_suivre(p_global->newRoomNum);
	closesalle();
	_adamMapMarkPos.x = -1;
	_adamMapMarkPos.y = -1;
	temps_passe(p_global->travelTime);
	p_global->ff_100 = p_global->room_ptr->ff_0;
	p_global->roomNum = p_global->newRoomNum;
	p_global->_areaNum = p_global->roomNum >> 8;
	p_global->_eventType = EventType::etEvent5;
	p_global->newMusicType = MusicType::mt2;
	setpersohere();
	musique();
	majsalle1(p_global->roomNum);
	drawTopScreen();
	_adamMapMarkPos.x = -1;
	_adamMapMarkPos.y = -1;
}

void EdenGame::deplaval(uint16 roomNum) {
	p_global->newLocation = roomNum & 0xFF;
	p_global->valleyVidNum = 0;
	p_global->phaseActionsCount++;
	closesalle();
	endpersovox();
	byte c1 = roomNum & 0xFF;
	if (c1 == 0)
		return;
	if (c1 < 0x80) {
		p_global->_displayFlags = DisplayFlags::dfFlag1;
		setChoiceYes();
		p_global->_eventType = EventType::etEvent1;
		showEvents();
		if (!isAnswerYes())
			return;
		if (p_global->ff_113) {
			waitEndSpeak();
			if (!pomme_q)
				close_perso();
		}
		specialout();
		if (p_global->_areaPtr->type == AreaType::atValley) {
			temps_passe(32);
			p_global->stepsToFindAppleFast++;
			p_global->stepsToFindAppleNormal++;
		}
		faire_suivre((roomNum & 0xFF00) | p_global->newLocation);
		p_global->ff_100 = p_global->room_ptr->ff_0;
		p_global->roomNum = roomNum;
		p_global->_areaNum = roomNum >> 8;
		p_global->_eventType = EventType::etEvent5;
		setpersohere();
		p_global->newMusicType = MusicType::mtNormal;
		musique();
		majsalle1(roomNum);
		p_global->_chronoFlag = 0;
		p_global->chrono = 0;
		p_global->ff_54 = 0;
		if (p_global->_roomCharacterType == PersonFlags::pftTyrann)
			chronoon(3000);
		return;
	}
	if (c1 == 0xFF) {
		p_global->_eventType = EventType::etEventE;
		showEvents();
		if (!kPersons[PER_MESSAGER]._roomNum && eloirevientq())
			chronoon(800);
		return;
	}
	p_global->stepsToFindAppleFast = 0;
	byte newAreaNum = c1 & 0x7F;
	byte curAreaNum = p_global->roomNum >> 8;
	int16 newRoomNum = newAreaNum << 8;
	if (curAreaNum == Areas::arTausCave && newAreaNum == Areas::arMo)
		newRoomNum |= 0x16;
	else if (curAreaNum == Areas::arMoorkusLair)
		newRoomNum |= 4;
	else
		newRoomNum |= 1;
	p_global->newRoomNum = newRoomNum;
	if (newAreaNum == Areas::arTausCave)
		gotolieu(&gotos[0]);
	else {
		for (goto_t *go = gotos + 1; go->curAreaNum != 0xFF; go++) {
			if (go->curAreaNum == curAreaNum) {
				gotolieu(go);
				break;
			}
		}
	}
}

// Original name: deplacement
void EdenGame::move(Direction dir) {
	room_t *room = p_global->room_ptr;
	int16 roomNum = p_global->roomNum;
	debug("move: from room %4X", roomNum);
	char newLoc = 0;
	rundcurs();
	afficher();
	p_global->prevLocation = roomNum & 0xFF;
	switch (dir) {
	case kCryoNorth:
		newLoc = room->exits[0];
		break;
	case kCryoEast:
		newLoc = room->exits[1];
		break;
	case kCryoSouth:
		newLoc = room->exits[2];
		break;
	case kCryoWest:
		newLoc = room->exits[3];
		break;
	}
	deplaval((roomNum & 0xFF00) | newLoc);
}

// Original name: deplacement2
void EdenGame::move2(Direction dir) {
	room_t *room = p_global->room_ptr;
	int16 roomNum = p_global->roomNum;
	char newLoc = 0;
	p_global->prevLocation = roomNum & 0xFF;
	switch (dir) {
	case kCryoNorth:
		newLoc = room->exits[0];
		break;
	case kCryoEast:
		newLoc = room->exits[1];
		break;
	case kCryoSouth:
		newLoc = room->exits[2];
		break;
	case kCryoWest:
		newLoc = room->exits[3];
		break;
	}
	deplaval((roomNum & 0xFF00) | newLoc);
}

void EdenGame::dinosoufle() {
	if (p_global->_curObjectId == 0) {
		bars_out();
		playHNM(148);
		maj2();
	}
}

void EdenGame::plaquemonk() {
	if (p_global->_curObjectId != 0) {
		if (p_global->_curObjectId == Objects::obPrism) {
			loseObject(Objects::obPrism);
			bars_out();
			specialTextMode = true;
			playHNM(89);
			// CHECKME: Unused code
			// word_2F514 |= 0x8000;
			maj2();
			p_global->_eventType = EventType::etEventB;
			showEvents();
		}
	} else {
		bars_out();
		playHNM(7);
		maj2();
		p_global->_eventType = EventType::etEvent4;
		showEvents();
	}
}

void EdenGame::fresquesgraa() {
	if (p_global->_curObjectId == 0) {
		p_global->fresqWidth = 320;
		p_global->fresqImgBank = 113;
		doFrescoes();
		dinaparle();
	}
}

void EdenGame::fresqueslasc() {
	if (p_global->_curObjectId == 0) {
		p_global->fresqWidth = 112;
		p_global->fresqImgBank = 315;
		doFrescoes();
	}
}

void EdenGame::pushpierre() {
	if (p_global->_curObjectId == 0) {
		gameRooms[22].exits[0] = 17;
		gameRooms[26].exits[2] = 9;
		move(kCryoNorth);
	}
}

void EdenGame::tetemomie() {
	if (p_global->_curObjectId == Objects::obTooth) {
		p_global->_gameFlags |= GameFlags::gfMummyOpened;
		move(kCryoNorth);
	} else if (p_global->_curObjectId == 0) {
		if (p_global->_gameFlags & GameFlags::gfMummyOpened)
			move(kCryoNorth);
		else {
			p_global->_eventType = EventType::etEvent6;
			persoparle(PersonId::pidMonk);
			p_global->_eventType = 0;
		}
	}
}

void EdenGame::tetesquel() {
	if (p_global->_curObjectId == Objects::obTooth) {
		gameRooms[22].exits[0] = 16;
		gameRooms[26].exits[2] = 13;
		_gameIcons[16]._cursorId |= 0x8000;
		loseObject(Objects::obTooth);
		move(kCryoNorth);
	}
}

void EdenGame::squelmoorkong() {
	p_global->_eventType = EventType::etEvent9;
	showEvents();
}

void EdenGame::choisir() {
	byte objid = current_spot2->_objectId;
	byte obj;
	switch (objid) {
	case 0:
		obj = p_global->giveobj1;
		break;
	case 1:
		obj = p_global->giveobj2;
		break;
	case 2:
		obj = p_global->giveobj3;
		break;
	default:
		warning("Unexpected object_id in choisir()");
		return;
	}
	objectmain(obj);
	winObject(obj);
	p_global->_iconsIndex = 16;
	p_global->_autoDialog = false;
	p_global->ff_60 = false;
	parle_moi();
}

void EdenGame::dinaparle() {
	int16 num;
	perso_t *perso = &kPersons[PER_DINA];
	if (perso->_partyMask & (p_global->party | p_global->_partyOutside)) {
		if (p_global->frescoNumber < 3)
			p_global->frescoNumber = 3;
		p_global->frescoNumber++;
		if (p_global->frescoNumber < 15) {
			endpersovox();
			if (p_global->frescoNumber == 7 && p_global->_phaseNum == 113)
				incPhase1();
			p_global->_characterPtr = perso;
			p_global->_dialogType = DialogType::dtInspect;
			num = (perso->_id << 3) | DialogType::dtInspect; //TODO: combine
			bool res = dialoscansvmas((dial_t *)getElem(gameDialogs, num));
			_frescoTalk = false;
			if (res) {
				restaurefondbulle();
				_frescoTalk = true;
				persovox();
			}
			p_global->ff_CA = 0;
			p_global->_dialogType = DialogType::dtTalk;
		} else
			endFrescoes();
	}
}

void EdenGame::roiparle() {
	if (p_global->_phaseNum <= 400)
		persoparle(0);
}

void EdenGame::roiparle1() {
	if (p_global->_curObjectId == Objects::obSword) {
		p_global->_gameFlags |= GameFlags::gfFlag80;
		bars_out();
		playHNM(76);
		move2(kCryoNorth);
	} else {
		p_global->frescoNumber = 1;
		roiparle();
	}
}

void EdenGame::roiparle2() {
	p_global->frescoNumber = 2;
	roiparle();
}

void EdenGame::roiparle3() {
	p_global->frescoNumber = 3;
	roiparle();
}

void EdenGame::getcouteau() {
	if (p_global->_phaseNum >= 80) {
		gameRooms[113].video = 0;
		getobject(Objects::obKnife);
	}
	p_global->_eventType = EventType::etEvent7;
	showEvents();
}

void EdenGame::getprisme() {
	getobject(Objects::obPrism);
	p_global->_eventType = EventType::etEvent7;
	showEvents();
}

void EdenGame::getchampb() {
	getobject(Objects::obShroom);
}

void EdenGame::getchampm() {
	getobject(Objects::obBadShroom);
}

void EdenGame::getor() {
	getobject(Objects::obGold);
}

void EdenGame::getnido() {
	if (p_global->_curObjectId != 0)
		return;
	p_global->room_ptr->bank = 282; //TODO: fix me
	p_global->room_ptr--;
	p_global->room_ptr->bank = 281; //TODO: fix me
	p_global->room_ptr->ff_0 = 3;
	getobject(Objects::obFullNest);
}

void EdenGame::getnidv() {
	if (p_global->_curObjectId != 0)
		return;
	p_global->room_ptr->bank = 282; //TODO: fix me
	p_global->room_ptr--;
	p_global->room_ptr->bank = 281; //TODO: fix me
	p_global->room_ptr->ff_0 = 3;
	getobject(Objects::obNest);
}

void EdenGame::getcorne() {
	if (p_global->_curObjectId != 0)
		return;
	getobject(Objects::obHorn);
	p_global->_eventType = EventType::etEvent7;
	showEvents();
	bigphase1();
	setpersohere();
	p_global->room_ptr = getsalle(p_global->roomNum);
}

void EdenGame::getsoleil() {
	if (p_global->_curObjectId != 0)
		return;
	gameRooms[238].video = 0;
	gameRooms[238].flags = RoomFlags::rf80;
	getobject(Objects::obSunStone);
}

void EdenGame::getoeuf() {
	if (p_global->_curObjectId != 0)
		return;
	p_global->room_ptr->flags = 0;
	p_global->room_ptr->video = 0;
	getobject(Objects::obEgg);
}

void EdenGame::getplaque() {
	if (p_global->_curObjectId != 0 && p_global->_curObjectId < Objects::obTablet1)
		return;
	p_global->_curObjectId = 0;
	getobject(Objects::obTablet2);
	putObject();
	for (int i = 0; i < 6; i++)
		_objects[Objects::obTablet1 - 1 + i]._count = 0;
	p_global->curObjectFlags = 0;
	p_global->inventoryScrollPos = 0;
	p_global->curObjectCursor = 9;
	_gameIcons[16]._cursorId |= 0x8000;
	showObjects();
	gameRooms[131].video = 0;
	bars_out();
	playHNM(149);
	p_global->ff_F1 = RoomFlags::rf04;
	p_global->_drawFlags = DrawFlags::drDrawFlag20;
	normalCursor = true;
	maj2();
}

void EdenGame::voirlac() {
	perso_t *perso = &kPersons[PER_MORKUS];
	room_t *room = p_global->room_ptr;
	area_t *area = p_global->_areaPtr;
	int16 vid = p_global->_curObjectId == Objects::obApple ? 81 : 54;
	for (++perso; perso->_roomNum != 0xFFFF; perso++) {
		if (perso->_roomNum != p_global->roomNum)
			continue;
		vid++;
		if (p_global->_curObjectId != Objects::obApple)
			continue;                   //TODO: pc breaks here
		if ((perso->_flags & PersonFlags::pfTypeMask) != PersonFlags::pftMosasaurus)
			continue;
		if (!(perso->_flags & PersonFlags::pf80))
			return;
		perso->_flags &= ~PersonFlags::pf80; //TODO: useless? see above
		area->_flags |= AreaFlags::afFlag8;
		p_global->_curAreaFlags |= AreaFlags::afFlag8;
		room->ff_0 = 3;
	}
	debug("sea monster: room = %X, d0 = %X\n", p_global->roomNum, p_global->roomImgBank);
	bars_out();
	playHNM(vid);
	maj_salle(p_global->roomNum);           //TODO: getting memory trashed here?
	if (p_global->_curObjectId == Objects::obApple)
		loseObject(Objects::obApple);
	p_global->_eventType = EventType::etEventF;
	showEvents();
}

void EdenGame::gotohall() {
	p_global->prevLocation = p_global->roomNum & 0xFF;
	deplaval((p_global->roomNum & 0xFF00) | 6);
}

void EdenGame::demitourlabi() {
	uint16 target;
	p_global->prevLocation = p_global->roomNum & 0xFF;
	p_global->ff_100 = 0xFF;
	target = (p_global->roomNum & 0xFF00) | p_global->room_ptr->exits[2];
	faire_suivre(target);
	p_global->roomNum = target;
	p_global->_eventType = EventType::etEvent5;
	maj_salle(p_global->roomNum);
}

void EdenGame::gotonido() {
	p_global->room_ptr++;
	p_global->_eventType = 0;
	p_global->roomImgBank = p_global->room_ptr->bank;
	p_global->roomVidNum = p_global->room_ptr->video;
	p_global->curRoomFlags = p_global->room_ptr->flags;
	p_global->ff_F1 = p_global->room_ptr->flags;
	animpiece();
	p_global->ff_100 = 0;
	maj2();
}

void EdenGame::gotoval() {
	uint16 target = p_global->roomNum;
	char obj;
	rundcurs();
	afficher();
	_scrollPos = 0;
	obj = current_spot2->_objectId - 14;    //TODO
	p_global->prevLocation = target & 0xFF;
	deplaval((target & 0xFF00) | obj);  //TODO careful!
}

void EdenGame::visiter() {
	bars_out();
	playHNM(144);
	p_global->ff_F1 = RoomFlags::rf04;
	maj2();
}

void EdenGame::final() {
	if (p_global->_curObjectId != 0)
		return;
	bars_out();
	*(int16 *)(gameRooms + 0x6DC) = 319; //TODO
	p_global->roomImgBank = 319;
	playHNM(97);
	maj2();
	p_global->_eventType = EventType::etEvent12;
	showEvents();
	p_global->narratorSequence = 54;
}

// Original name: goto_nord
void EdenGame::moveNorth() {
	if (p_global->_curObjectId == 0)
		move(kCryoNorth);
}

// Original name: goto_est
void EdenGame::moveEast() {
	if (p_global->_curObjectId == 0)
		move(kCryoEast);
}

// Original name: goto_sud
void EdenGame::moveSouth() {
	if (p_global->_curObjectId == 0)
		move(kCryoSouth);
}

// Original name: goto_ouest
void EdenGame::moveWest() {
	if (p_global->_curObjectId == 0)
		move(kCryoWest);
}

void EdenGame::afficher() {
	if (!p_global->ff_102 && !p_global->ff_103) {
		if (needPaletteUpdate) {
			needPaletteUpdate = false;
			CLPalette_Send2Screen(global_palette, 0, 256);
		}
		CLBlitter_CopyView2Screen(p_mainview);
	} else {
		if (p_global->ff_102)
			effet3();
		else
			effet2();

		p_global->ff_103 = 0;
		p_global->ff_102 = 0;
	}
}

void EdenGame::afficher128() {
	if (p_global->updatePaletteFlag == 16) {
		CLPalette_Send2Screen(global_palette, 0, 129);
		CLBlitter_CopyView2Screen(p_mainview);
		p_global->updatePaletteFlag = 0;
	} else {
		ClearScreen();
		fadetoblack128(1);
		if (showBlackBars)
			drawBlackBars();
		CLBlitter_CopyView2Screen(p_mainview);
		fadefromblack128(1);
	}
}

// Original name: sauvefrises
void EdenGame::saveFriezes() {
	saveTopFrieze(0);
	saveBottomFrieze();
}

// Original name: sauvefriseshaut
void EdenGame::saveTopFrieze(int16 x) { // Save top bar
	_underTopBarScreenRect = Common::Rect(x, 0, x + 320 - 1, 15);
	_underTopBarBackupRect = Common::Rect(0, 0, 320 - 1, 15);
	CLBlitter_CopyViewRect(p_mainview, p_underBarsView, &_underTopBarScreenRect, &_underTopBarBackupRect);
}

// Original name: sauvefrisesbas
void EdenGame::saveBottomFrieze() {         // Save bottom bar
	_underBottomBarScreenRect.left = 0;
	_underBottomBarScreenRect.right = 320 - 1;
	CLBlitter_CopyViewRect(p_mainview, p_underBarsView, &_underBottomBarScreenRect, &_underBottomBarBackupRect);
}

// Original name: restaurefrises
void EdenGame::restoreFriezes() {
	restoreTopFrieze();
	restoreBottomFrieze();
}

// Original name: restaurefriseshaut
void EdenGame::restoreTopFrieze() {
	_underTopBarScreenRect.left = _scrollPos;
	_underTopBarScreenRect.right = _scrollPos + 320 - 1;
	CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &_underTopBarBackupRect, &_underTopBarScreenRect);
}

// Original name: restaurefrisesbas
void EdenGame::restoreBottomFrieze() {
	_underBottomBarScreenRect.left = _scrollPos;
	_underBottomBarScreenRect.right = _scrollPos + 320 - 1;
	CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &_underBottomBarBackupRect, &_underBottomBarScreenRect);
}

void EdenGame::use_main_bank() {
	_bankData = _mainBankBuf;
}

// Original name: use_bank
void EdenGame::useBank(int16 bank) {
	if (bank > 2500)
		error("attempt to load bad bank %d", bank);

	_bankData = bank_data_buf;
	if (_curBankNum != bank) {
		loadFile(bank, bank_data_buf);
		verifh(bank_data_buf);
		_curBankNum = bank;
	}
}

void EdenGame::sundcurs(int16 x, int16 y) {
	byte *keep = _cursKeepBuf;
	_cursKeepPos = Common::Point(x - 4, y - 4);
	byte *scr = p_mainview_buf + _cursKeepPos.x + _cursKeepPos.y * 640;
	for (int16 h = 48; h--;) {
		for (int16 w = 48; w--;)
			*keep++ = *scr++;
		scr += 640 - 48;
	}
	curs_saved = true;
}

void EdenGame::rundcurs() {
	byte *keep = _cursKeepBuf;
	byte *scr = p_mainview_buf + _cursKeepPos.x + _cursKeepPos.y * 640;
	if (!curs_saved || (_cursKeepPos == Common::Point(-1, -1)))  //TODO ...
		return;

	for (int16 h = 48; h--;) {
		for (int16 w = 48; w--;)
			*scr++ = *keep++;
		scr += 640 - 48;
	}

}

void EdenGame::noclipax(int16 index, int16 x, int16 y) {
	byte *pix = _bankData;
	byte *scr = p_mainview_buf + x + y * 640;
	byte h0, h1, mode;
	int16 w, h;
	if (_curBankNum != 117 && !_noPalette) {
		if (READ_LE_UINT16(pix) > 2)
			readPalette(pix + 2);
	}
	pix += READ_LE_UINT16(pix);
	pix += READ_LE_UINT16(pix + index * 2);
	//  int16   height:9
	//  int16   pad:6;
	//  int16   flag:1;
	h0 = *pix++;
	h1 = *pix++;
	w = ((h1 & 1) << 8) | h0;
	h = *pix++;
	mode = *pix++;
	debug("- draw sprite %d at %d:%d, %dx%d", index, x, y, w, h);
	if (mode != 0xFF && mode != 0xFE)
		return;
	if (y + h > 200)
		h -= (y + h - 200);
	if (h1 & 0x80) {
		// compressed
		for (; h-- > 0;) {
			int16 ww;
			for (ww = w; ww > 0;) {
				byte c = *pix++;
				if (c >= 0x80) {
					if (c == 0x80) {
						byte fill = *pix++;
						if (fill == 0) {
							scr += 128 + 1;
							ww -= 128 + 1;
						} else {
							byte run;
							*scr++ = fill;  //TODO: wha?
							*scr++ = fill;
							ww -= 128 + 1;
							for (run = 127; run--;)
								*scr++ = fill;
						}
					} else {
						byte fill = *pix++;
						byte run = 255 - c + 2;
						ww -= run;
						if (fill == 0)
							scr += run;
						else
							for (; run--;)
								*scr++ = fill;
					}
				} else {
					byte run = c + 1;
					ww -= run;
					for (; run--;) {
						byte p = *pix++;
						if (p == 0)
							scr++;
						else
							*scr++ = p;
					}
				}
			}
			scr += 640 - w;
		}
	} else {
		// uncompressed
		for (; h--;) {
			int16 ww;
			for (ww = w; ww--;) {
				byte p = *pix++;
				if (p == 0)
					scr++;
				else
					*scr++ = p;
			}
			scr += 640 - w;
		}
	}
}

void EdenGame::noclipax_avecnoir(int16 index, int16 x, int16 y) {
	byte *pix = _bankData;
	byte *scr = p_mainview_buf + x + y * 640;
	byte h0, h1, mode;
	int16 w, h;
	if (_curBankNum != 117) {
		if (READ_LE_UINT16(pix) > 2)
			readPalette(pix + 2);
	}
	pix += READ_LE_UINT16(pix);
	pix += READ_LE_UINT16(pix + index * 2);
	//  int16   height:9
	//  int16   pad:6;
	//  int16   flag:1;
	h0 = *pix++;
	h1 = *pix++;
	w = ((h1 & 1) << 8) | h0;
	h = *pix++;
	mode = *pix++;
	if (mode != 0xFF && mode != 0xFE)
		return;
	if (y + h > 200)
		h -= (y + h - 200);
	if (h1 & 0x80) {
		// compressed
		for (; h-- > 0;) {
			int16 ww;
			for (ww = w; ww > 0;) {
				byte c = *pix++;
				if (c >= 0x80) {
					if (c == 0x80) {
						byte fill = *pix++;
						byte run;
						*scr++ = fill;  //TODO: wha?
						*scr++ = fill;
						ww -= 128 + 1;
						for (run = 127; run--;)
							*scr++ = fill;
					} else {
						byte fill = *pix++;
						byte run = 255 - c + 2;
						ww -= run;
						for (; run--;)
							*scr++ = fill;
					}
				} else {
					byte run = c + 1;
					ww -= run;
					for (; run--;) {
						byte p = *pix++;
						*scr++ = p;
					}
				}
			}
			scr += 640 - w;
		}
	} else {
		// uncompressed
		for (; h--;) {
			int16 ww;
			for (ww = w; ww--;) {
				byte p = *pix++;
				*scr++ = p;
			}
			scr += 640 - w;
		}
	}
}

void EdenGame::getglow(int16 x, int16 y, int16 w, int16 h) {
	byte *scr = p_mainview_buf + x + y * 640;
	byte *gl = glow_buffer;
	glow_x = x;
	glow_y = y;
	glow_w = w;
	glow_h = h;
	for (; h--;) {
		int16 ww;
		for (ww = w; ww--;)
			*gl++ = *scr++;
		scr += 640 - w;
	}
}

void EdenGame::unglow() {
	byte *gl = glow_buffer;
	byte *scr = p_mainview_buf + glow_x + glow_y * 640;
	if (glow_x < 0 || glow_y < 0)   //TODO: move it up
		return;
	for (; glow_h--;) {
		int16 ww;
		for (ww = glow_w; ww--;)
			*scr++ = *gl++;
		scr += 640 - glow_w;
	}
}

void EdenGame::glow(int16 index) {
	// byte pixbase;
	byte *pix = _bankData;

	index += 9;
	pix += READ_LE_UINT16(pix);
	pix += READ_LE_UINT16(pix + index * 2);
	//  int16   height:9
	//  int16   pad:6;
	//  int16   flag:1;
	byte h0 = *pix++;
	byte h1 = *pix++;
	int16 w = ((h1 & 1) << 8) | h0;
	int16 h = *pix++;
	byte mode = *pix++;
	if (mode != 0xFF && mode != 0xFE)
		return;

	int16 x = curs_x + _scrollPos - 38;
	int16 y = curs_y - 28;
	int16 ex = p_global->fresqWidth + 320;

	if (x + w <= 0 || x >= ex || y + h <= 0 || y >= 176)
		return;

	int16 dx;
	if (x < 0) {
		dx = -x;
		x = 0;
	} else if (x + w > ex)
		dx = x + w - ex;
	else
		dx = 0;

	int16 dy;
	if (y < 16) {
		dy = 16 - y;
		y = 16;
	} else if (y + h > 175)
		dy = y + h - 175;
	else
		dy = 0;

	int16 pstride = dx;
	int16 sstride = 640 - (w - dx);
	if (y == 16)
		pix += w * dy;
	if (x == 0)
		pix += dx;

	byte *scr = p_mainview_buf + x + y * 640;

	w -= dx;
	h -= dy;

	getglow(x, y, w, h);

	for (; h--;) {
		for (int16 ww = w; ww--;) {
			byte p = *pix++;
			if (p == 0)
				scr++;
			else
				*scr++ += p << 4;
		}
		pix += pstride;
		scr += sstride;
	}
}

void EdenGame::readPalette(byte *ptr) {
	bool doit = true;
	while (doit) {
		uint16 idx = *ptr++;
		if (idx != 0xFF) {
			uint16 cnt = *ptr++;
			while (cnt--) {
				if (idx == 0) {
					pal_entry.r = 0;
					pal_entry.g = 0;
					pal_entry.b = 0;
					ptr += 3;
				} else {
					pal_entry.r = *ptr++ << 10;
					pal_entry.g = *ptr++ << 10;
					pal_entry.b = *ptr++ << 10;
				}
				CLPalette_SetRGBColor(global_palette, idx, &pal_entry);
				idx++;
			}
		} else
			doit = false;
	}
}

// Original name: spritesurbulle
void EdenGame::spriteOnSubtitle(int16 index, int16 x, int16 y) {
	byte *pix = _bankData;
	byte *scr = p_subtitlesview_buf + x + y * subtitles_x_width;
	if ((_curBankNum != 117) && (READ_LE_UINT16(pix) > 2))
		readPalette(pix + 2);

	pix += READ_LE_UINT16(pix);
	pix += READ_LE_UINT16(pix + index * 2);
	//  int16   height:9
	//  int16   pad:6;
	//  int16   flag:1;
	byte h0 = *pix++;
	byte h1 = *pix++;
	int16 w = ((h1 & 1) << 8) | h0;
	int16 h = *pix++;
	byte mode = *pix++;
	if (mode != 0xFF && mode != 0xFE)
		return;
	if (h1 & 0x80) {
		// compressed
		for (; h-- > 0;) {
			for (int16 ww = w; ww > 0;) {
				byte c = *pix++;
				if (c >= 0x80) {
					if (c == 0x80) {
						byte fill = *pix++;
						if (fill == 0) {
							scr += 128 + 1;
							ww -= 128 + 1;
						} else {
							byte run;
							*scr++ = fill;  //TODO: wha?
							*scr++ = fill;
							ww -= 128 + 1;
							for (run = 127; run--;)
								*scr++ = fill;
						}
					} else {
						byte fill = *pix++;
						byte run = 255 - c + 2;
						ww -= run;
						if (fill == 0)
							scr += run;
						else {
							for (; run--;)
								*scr++ = fill;
						}
					}
				} else {
					byte run = c + 1;
					ww -= run;
					for (; run--;) {
						byte p = *pix++;
						if (p == 0)
							scr++;
						else
							*scr++ = p;
					}
				}
			}
			scr += subtitles_x_width - w;
		}
	} else {
		// uncompressed
		for (; h--;) {
			for (int16 ww = w; ww--;) {
				byte p = *pix++;
				if (p == 0)
					scr++;
				else
					*scr++ = p;
			}
			scr += subtitles_x_width - w;
		}
	}
}

void EdenGame::bars_out() {
	if (showBlackBars)
		return;

	afficher();
	_underTopBarScreenRect.left = _scrollPos;
	_underTopBarScreenRect.right = _scrollPos + 320 - 1;
	CLBlitter_CopyViewRect(p_mainview, p_underBarsView, &_underTopBarScreenRect, &_underTopBarBackupRect);
	_underBottomBarScreenRect.left = _underTopBarScreenRect.left;
	_underBottomBarScreenRect.right = _underTopBarScreenRect.right;
	CLBlitter_CopyViewRect(p_mainview, p_underBarsView, &_underBottomBarScreenRect, &_underBottomBarBackupRect);
	int16 r19 = 14;   // TODO - init in decl?
	int16 r20 = 176;
	int16 r25 = 14;
	int16 r24 = 21;
	_underTopBarScreenRect.left = 0;
	_underTopBarScreenRect.right = 320 - 1;
	_underTopBarBackupRect.left = _scrollPos;
	_underTopBarBackupRect.right = _scrollPos + 320 - 1;
	unsigned int *scr40, *scr41, *scr42;
	while (r24 > 0) {
		if (r25 > 0) {
			_underTopBarScreenRect.top = 16 - r25;
			_underTopBarScreenRect.bottom = 16 - 1;
			_underTopBarBackupRect.top = 0;
			_underTopBarBackupRect.bottom = r25 - 1;
			CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &_underTopBarScreenRect, &_underTopBarBackupRect);
			scr40 = ((unsigned int *)p_mainview_buf) + r19 * 640 / 4;
			scr41 = scr40 + 640 / 4;
			for (int i = 0; i < 320; i += 4) {
				*scr40++ = 0;
				*scr41++ = 0;
			}
		}
		_underTopBarScreenRect.top = 16;
		_underTopBarScreenRect.bottom = r24 + 16 - 1;
		_underTopBarBackupRect.top = 200 - r24;
		_underTopBarBackupRect.bottom = 200 - 1;
		CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &_underTopBarScreenRect, &_underTopBarBackupRect);
		scr40 = ((unsigned int *)p_mainview_buf) + r20 * 640 / 4;
		scr41 = scr40 + 640 / 4;
		scr42 = scr41 + 640 / 4;
		for (int i = 0; i < 320; i += 4) {
			*scr40++ = 0;
			*scr41++ = 0;
			*scr42++ = 0;
		}
		r19 -= 2;
		r20 += 3;
		r25 -= 2;
		r24 -= 3;
		afficher();
	}
	scr40 = (unsigned int *)p_mainview_buf;
	scr41 = scr40 + 640 / 4;
	for (int i = 0; i < 320; i += 4) {
		*scr40++ = 0;
		*scr41++ = 0;
	}
	scr40 = ((unsigned int *)p_mainview_buf) + r20 * 640 / 4;
	scr41 = scr40 + 640 / 4;
	scr42 = scr41 + 640 / 4;
	for (int i = 0; i < 320; i += 4) {
		*scr40++ = 0;
		*scr41++ = 0;
		*scr42++ = 0;
	}
	afficher();
	initRects();
	showBlackBars = true;
}

// Original name: bars_in
void EdenGame::showBars() {
	if (!showBlackBars)
		return;

	drawBlackBars();
	int16 r29 = 2;
	int16 r28 = 2;
	_underTopBarScreenRect.left = 0;
	_underTopBarScreenRect.right = 320 - 1;
	_underTopBarBackupRect.left = _scrollPos;
	_underTopBarBackupRect.right = _scrollPos + 320 - 1;
	while (r28 < 24) {
		if (r29 <= 16) {
			_underTopBarScreenRect.top = 16 - r29;
			_underTopBarScreenRect.bottom = 16 - 1;
			_underTopBarBackupRect.top = 0;
			_underTopBarBackupRect.bottom = r29 - 1;
			CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &_underTopBarScreenRect, &_underTopBarBackupRect);
		}
		_underTopBarScreenRect.top = 16;
		_underTopBarScreenRect.bottom = 16 + r28;
		_underTopBarBackupRect.top = 200 - 1 - r28;
		_underTopBarBackupRect.bottom = 200 - 1;
		CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &_underTopBarScreenRect, &_underTopBarBackupRect);
		r29 += 2;
		r28 += 3;
		afficher();
	}
	initRects();
	showBlackBars = false;
}

void EdenGame::sauvefondbouche() {
	rect_src.left = cur_perso_rect->sx;
	rect_src.top = cur_perso_rect->sy;
	rect_src.right = cur_perso_rect->ex;
	rect_src.bottom = cur_perso_rect->ey;
	rect_dst.left = cur_perso_rect->sx + 320;
	rect_dst.top = cur_perso_rect->sy;
	rect_dst.right = cur_perso_rect->ex + 320;
	rect_dst.bottom = cur_perso_rect->ey;
	CLBlitter_CopyViewRect(p_mainview, p_mainview, &rect_src, &rect_dst);
	fond_saved = true;
}

void EdenGame::restaurefondbouche() {
	rect_src.left = cur_perso_rect->sx;
	rect_src.top = cur_perso_rect->sy;
	rect_src.right = cur_perso_rect->ex;
	rect_src.bottom = cur_perso_rect->ey;
	rect_dst.left = cur_perso_rect->sx + 320;
	rect_dst.top = cur_perso_rect->sy;
	rect_dst.right = cur_perso_rect->ex + 320;
	rect_dst.bottom = cur_perso_rect->ey;
	CLBlitter_CopyViewRect(p_mainview, p_mainview, &rect_dst, &rect_src);
}

// Original name : blackbars
void EdenGame::drawBlackBars() {
	byte *scr = p_mainview_buf;
	for (int16 y = 0; y < 16; y++) {
		for (int16 x = 0; x < 640; x++)
			*scr++ = 0;
	}

	scr += 640 * (200 - 16 - 24);
	for (int16 y = 0; y < 24; y++) {
		for (int16 x = 0; x < 640; x++)
			*scr++ = 0;
	}
}

void EdenGame::drawTopScreen() {  // Draw  top bar (location / party / map)
	p_global->_drawFlags &= ~DrawFlags::drDrawTopScreen;
	useBank(314);
	noclipax(36, 83, 0);
	noclipax(p_global->_areaPtr->num - 1, 0, 0);
	noclipax(23, 145, 0);
	for (perso_t *perso = &kPersons[PER_DINA]; perso != &kPersons[PER_UNKN_156]; perso++) {
		if ((perso->_flags & PersonFlags::pfInParty) && !(perso->_flags & PersonFlags::pf80))
			noclipax(perso->_targetLoc + 18, perso->_lastLoc + 120, 0);
	}
	_adamMapMarkPos.x = -1;
	_adamMapMarkPos.y = -1;
	displayValleyMap();
	needPaletteUpdate = true;
}

// Original name: affplanval
void EdenGame::displayValleyMap() { // Draw mini-map
	if (p_global->_areaPtr->type == AreaType::atValley) {
		noclipax(p_global->_areaPtr->num + 9, 266, 1);
		for (perso_t *perso = &kPersons[PER_UNKN_18C]; perso->_roomNum != 0xFFFF; perso++) {
			if (((perso->_roomNum >> 8) == p_global->_areaNum)
			        && !(perso->_flags & PersonFlags::pf80) && (perso->_flags & PersonFlags::pf20))
				displayMapMark(33, perso->_roomNum & 0xFF);
		}
		if (p_global->_areaPtr->citadelLevel)
			displayMapMark(34, p_global->_areaPtr->citadelRoom->location);
		saveTopFrieze(0);
		int16 loc = p_global->roomNum & 0xFF;
		if (loc >= 16)
			displayAdamMapMark(loc);
		restoreTopFrieze();
	} else {
		saveTopFrieze(0);
		restoreTopFrieze();
	}
}

// Original name: affrepere
void EdenGame::displayMapMark(int16 index, int16 location) {
	noclipax(index, 269 + location % 16 * 4, 2 + (location - 16) / 16 * 3);
}

// Original name: affrepereadam
void EdenGame::displayAdamMapMark(int16 location) {
	int16 x = 269;
	int16 y = 2;
	restoreAdamMapMark();
	if (location > 15 && location < 76) {
		x += (location & 15) * 4;
		y += ((location - 16) >> 4) * 3;
		saveAdamMapMark(x, y);
		byte *pix = p_underBarsView->_bufferPtr;
		int16 w = p_underBarsView->_width;
		pix += x + w * y;
		pix[1] = 0xC3;
		pix[2] = 0xC3;
		pix += w;
		pix[0] = 0xC3;
		pix[1] = 0xC3;
		pix[2] = 0xC3;
		pix[3] = 0xC3;
		pix += w;
		pix[1] = 0xC3;
		pix[2] = 0xC3;
	}
}

// Original name: rest_repadam
void EdenGame::restoreAdamMapMark() {
	if (_adamMapMarkPos.x == -1 && _adamMapMarkPos.y == -1)
		return;

	int16 x = _adamMapMarkPos.x;
	int16 y = _adamMapMarkPos.y;
	byte *pix = p_underBarsView->_bufferPtr;
	int16 w = p_underBarsView->_width;
	pix += x + w * y;
	pix[1] = _oldPix[0];
	pix[2] = _oldPix[1];
	pix += w;
	pix[0] = _oldPix[2];
	pix[1] = _oldPix[3];
	pix[2] = _oldPix[4];
	pix[3] = _oldPix[5];
	pix += w;
	pix[1] = _oldPix[6];
	pix[2] = _oldPix[7];
}

// Original name: save_repadam
void EdenGame::saveAdamMapMark(int16 x, int16 y) {
	_adamMapMarkPos.x = x;
	_adamMapMarkPos.y = y;
	byte *pix = p_underBarsView->_bufferPtr;
	int16 w = p_underBarsView->_width;
	pix += x + w * y;
	_oldPix[0] = pix[1];
	_oldPix[1] = pix[2];
	pix += w;
	_oldPix[2] = pix[0];
	_oldPix[3] = pix[1];
	_oldPix[4] = pix[2];
	_oldPix[5] = pix[3];
	pix += w;
	_oldPix[6] = pix[1];
	_oldPix[7] = pix[2];
}

bool EdenGame::istrice(int16 roomNum) {
	char loc = roomNum & 0xFF;
	int16 area = roomNum & 0xFF00;
	for (perso_t *perso = &kPersons[PER_UNKN_18C]; perso != &kPersons[PER_UNKN_372]; perso++) {
		if ((perso->_flags & PersonFlags::pf80) || (perso->_flags & PersonFlags::pfTypeMask) != PersonFlags::pftTriceraptor)
			continue;
		if (perso->_roomNum == (area | (loc - 16)))
			return true;
		if (perso->_roomNum == (area | (loc + 16)))
			return true;
		if (perso->_roomNum == (area | (loc - 1)))
			return true;
		if (perso->_roomNum == (area | (loc + 1)))
			return true;
	}
	return false;
}

bool EdenGame::istyran(int16 roomNum) {
	char loc = roomNum & 0xFF;
	int16 area = roomNum & 0xFF00;
	// TODO: orig bug: this ptr is not initialized when first called from getsalle
	// PC version scans kPersons[] directly and is not affected
	if (!tyranPtr)
		return false;

	for (; tyranPtr->_roomNum != 0xFFFF; tyranPtr++) {
		if (tyranPtr->_flags & PersonFlags::pf80)
			continue;
		if (tyranPtr->_roomNum == (area | (loc - 16)))
			return true;
		if (tyranPtr->_roomNum == (area | (loc + 16)))
			return true;
		if (tyranPtr->_roomNum == (area | (loc - 1)))
			return true;
		if (tyranPtr->_roomNum == (area | (loc + 1)))
			return true;
	}
	return false;
}

void EdenGame::istyranval(area_t *area) {
	byte areaNum = area->num;
	area->_flags &= ~AreaFlags::HasTyrann;
	for (perso_t *perso = &kPersons[PER_UNKN_372]; perso->_roomNum != 0xFFFF; perso++) {
		if (perso->_flags & PersonFlags::pf80)
			continue;

		if ((perso->_roomNum >> 8) == areaNum) {
			area->_flags |= AreaFlags::HasTyrann;
			break;
		}
	}
}

char EdenGame::getDirection(perso_t *perso) {
	char dir = -1;
	byte trgLoc = perso->_targetLoc;
	byte curLoc = perso->_roomNum & 0xFF;   //TODO name
	if (curLoc != trgLoc) {
		curLoc &= 0xF;
		trgLoc &= 0xF;
		if (curLoc != trgLoc) {
			dir = 2;
			if (curLoc > trgLoc)
				dir = 5;
		}
		trgLoc = perso->_targetLoc;
		curLoc = perso->_roomNum & 0xFF;
		curLoc &= 0xF0;
		trgLoc &= 0xF0;
		if (curLoc != trgLoc) {
			if (curLoc > trgLoc)
				dir++;
			dir++;
		}
	}
	return dir;
}

// Original name: caselibre
bool EdenGame::canMoveThere(char loc, perso_t *perso) {
	room_t *room = p_global->cita_area_firstRoom;
	if (loc <= 0x10 || loc > 76 || (loc & 0xF) >= 12 || loc == perso->_lastLoc)
		return false;

	int16 roomNum = (perso->_roomNum & ~0xFF) | loc;   //TODO: danger! signed
	if (roomNum == p_global->roomNum)
		return false;

	for (; room->ff_0 != 0xFF; room++) {
		if (room->location != loc)
			continue;
		if (!(room->flags & RoomFlags::rf01))
			return false;
		for (perso = &kPersons[PER_UNKN_18C]; perso->_roomNum != 0xFFFF; perso++) {
			if (perso->_flags & PersonFlags::pf80)
				continue;
			if (perso->_roomNum == roomNum)
				break;
		}
		if (perso->_roomNum != 0xFFFF)
			return false;
		return true;
	}
	return false;
}

// Original name: melange1
void EdenGame::scramble1(uint8 elem[4]) {
	if (_vm->_rnd->getRandomNumber(1) & 1)
		SWAP(elem[1], elem[2]);
}

// Original name melange2
void EdenGame::scramble2(uint8 elem[4]) {
	if (_vm->_rnd->getRandomNumber(1) & 1)
		SWAP(elem[0], elem[1]);

	if (_vm->_rnd->getRandomNumber(1) & 1)
		SWAP(elem[2], elem[3]);
}

void EdenGame::melangedir() {
	scramble1(tab_2CB1E[0]);
	scramble1(tab_2CB1E[1]);
	scramble1(tab_2CB1E[2]);
	scramble2(tab_2CB1E[3]);
	scramble2(tab_2CB1E[4]);
	scramble1(tab_2CB1E[5]);
	scramble2(tab_2CB1E[6]);
	scramble2(tab_2CB1E[7]);
}

bool EdenGame::naitredino(char persoType) {
	for (perso_t *perso = &kPersons[PER_MORKUS]; (++perso)->_roomNum != 0xFFFF;) {
		char areaNum = perso->_roomNum >> 8;
		if (areaNum != p_global->cita_area_num)
			continue;
		if ((perso->_flags & PersonFlags::pf80) && (perso->_flags & PersonFlags::pfTypeMask) == persoType) {
			perso->_flags &= ~PersonFlags::pf80;
			return true;
		}
	}
	return false;
}

void EdenGame::newcita(char arg1, int16 arg2, room_t *room) {
	cita_t *cita = cita_list;
	while (cita->ff_0 < arg2)
		cita++;

	uint16 index = ((room->flags & 0xC0) >> 6);    //TODO: this is very wrong
	int16 *ptr = cita->ff_2 + index * 2;
	if (arg1 == 4 || arg1 == 6)
		ptr++;

	room->bank = ptr[0];
	room->video = ptr[8];
	room->flags |= RoomFlags::rf02;
}

void EdenGame::citaevol(int16 level) {
	room_t *room = p_global->cur_area_ptr->citadelRoom;
	perso_t *perso = &kPersons[PER_UNKN_372];
	byte loc = room->location;
	if (level >= 80 && !istrice((p_global->cita_area_num << 8) | loc)) {
		room->level = 79;
		return;
	}

	if (level > 160)
		level = 160;

	if (room->level < 64 && level >= 64 && naitredino(PersonFlags::pftTriceraptor)) {
		p_global->cur_area_ptr->_flags |= AreaFlags::HasTriceraptors;
		ajouinfo(p_global->cita_area_num + ValleyNews::vnTriceraptorsIn);
	}
	if (room->level < 40 && level >= 40 && naitredino(PersonFlags::pftVelociraptor)) {
		p_global->cur_area_ptr->_flags |= AreaFlags::HasVelociraptors;
		ajouinfo(p_global->cita_area_num + ValleyNews::vnVelociraptorsIn);
	}
	room->level = level;
	newcita(p_global->cita_area_num, level, room);
	byte speed = kDinoSpeedForCitaLevel[room->level >> 4];
	for (; perso->_roomNum != 0xFFFF; perso++) {
		if (perso->_flags & PersonFlags::pf80)
			continue;
		if ((perso->_roomNum >> 8) == p_global->cita_area_num && perso->_targetLoc == loc)
			perso->_speed = speed;
	}
}

void EdenGame::citacapoute(int16 roomNum) {
	perso_t *perso = &kPersons[PER_UNKN_18C];
	room_t *room = p_global->cur_area_ptr->citadelRoom;
	room->flags |= RoomFlags::rf01;
	room->flags &= ~RoomFlags::rfHasCitadel;
	room->bank = 193;
	room->video = 0;
	room->level = 0;
	p_global->cur_area_ptr->citadelLevel = 0;
	p_global->cur_area_ptr->citadelRoom = 0;
	roomNum = (roomNum & ~0xFF) | room->location;
	for (; perso->_roomNum != 0xFFFF; perso++) {
		if (perso->_roomNum == roomNum) {
			perso->_flags &= ~PersonFlags::pf80;
			delinfo((roomNum >> 8) + ValleyNews::vnTyrannIn);
			break;
		}
	}
}

void EdenGame::buildcita() {
	area_t *area = p_global->_areaPtr;
	p_global->cur_area_ptr = p_global->_areaPtr;
	if (area->citadelRoom)
		citacapoute(p_global->roomNum);
	p_global->ff_6A = p_global->ff_69;
	p_global->narratorSequence = p_global->ff_69 | 0x80;
	area->citadelRoom = p_global->room_ptr;
	p_global->room_ptr->flags &= ~RoomFlags::rf01;
	p_global->room_ptr->flags |= RoomFlags::rfHasCitadel;
	p_global->room_ptr->level = 32;
	newcita(p_global->_areaNum, 32, p_global->room_ptr);
	area->_flags &= ~AreaFlags::TyrannSighted;
	if (!(area->_flags & AreaFlags::afFlag8000)) {
		if (p_global->_phaseNum == 304 || p_global->_phaseNum != 384) //TODO: wha
			eloirevient();
		area->_flags |= AreaFlags::afFlag8000;
	}
	p_global->_roomCharacter->_flags |= PersonFlags::pf80;
	p_global->cita_area_num = p_global->_areaNum;
	naitredino(1);
	delinfo(p_global->_areaNum + ValleyNews::vnCitadelLost);
	delinfo(p_global->_areaNum + ValleyNews::vnTyrannLost);
	if (p_global->_phaseNum == 193 && p_global->_areaNum == Areas::arUluru)
		bigphase1();
}

void EdenGame::citatombe(char level) {
	if (level)
		newcita(p_global->cita_area_num, level, p_global->cur_area_ptr->citadelRoom);
	else {
		citacapoute(p_global->cita_area_num << 8);
		ajouinfo(p_global->cita_area_num + ValleyNews::vnCitadelLost);
	}
}

void EdenGame::constcita() {
	//	room_t *room = p_global->cur_area_ptr->room_ptr; //TODO: wrong? chk below
	//	byte id = room->ff_C;
	if (!p_global->cur_area_ptr->citadelLevel || !p_global->cur_area_ptr->citadelRoom)
		return;

	room_t *room = p_global->cur_area_ptr->citadelRoom; //TODO: copied here by me
	byte loc = room->location;
	tyranPtr = &kPersons[PER_UNKN_372];
	if (istyran((p_global->cita_area_num << 8) | loc)) {
		if (!(p_global->cur_area_ptr->_flags & AreaFlags::TyrannSighted)) {
			ajouinfo(p_global->cita_area_num + ValleyNews::vnTyrannIn);
			p_global->cur_area_ptr->_flags |= AreaFlags::TyrannSighted;
		}
		byte level = room->level - 1;
		if (level < 32)
			level = 32;
		room->level = level;
		citatombe(level);
	} else {
		p_global->cur_area_ptr->_flags &= ~AreaFlags::TyrannSighted;
		citaevol(room->level + 1);
	}
}

void EdenGame::depladino(perso_t *perso) {
	int dir = getDirection(perso);
	if (dir != -1) {
		melangedir();
		uint8 *dirs = tab_2CB1E[dir];
		byte loc = perso->_roomNum & 0xFF;
		uint8 dir2 = *dirs++;
		if (dir2 & 0x80)
			dir2 = -(dir2 & ~0x80);
		dir2 += loc;
		if (canMoveThere(dir2, perso))
			goto ok;
		dir2 = *dirs++;
		if (dir2 & 0x80)
			dir2 = -(dir2 & ~0x80);
		dir2 += loc;
		if (canMoveThere(dir2, perso))
			goto ok;
		dir2 = *dirs++;
		if (dir2 & 0x80)
			dir2 = -(dir2 & ~0x80);
		dir2 += loc;
		if (canMoveThere(dir2, perso))
			goto ok;
		dir2 = *dirs++;
		if (dir2 & 0x80)
			dir2 = -(dir2 & ~0x80);
		dir2 += loc;
		if (canMoveThere(dir2, perso))
			goto ok;
		dir2 = perso->_lastLoc;
		perso->_lastLoc = 0;
		if (!canMoveThere(dir2, perso))
			return;
	ok:
		;
		perso->_lastLoc = perso->_roomNum & 0xFF;
		perso->_roomNum &= ~0xFF;
		perso->_roomNum |= dir2 & 0xFF;
		if (perso->_targetLoc - 16 == (perso->_roomNum & 0xFF))
			perso->_targetLoc = 0;
		if (perso->_targetLoc + 16 == (perso->_roomNum & 0xFF))
			perso->_targetLoc = 0;
		if (perso->_targetLoc - 1 == (perso->_roomNum & 0xFF))
			perso->_targetLoc = 0;
		if (perso->_targetLoc + 1 == (perso->_roomNum & 0xFF))
			perso->_targetLoc = 0;
	} else
		perso->_targetLoc = 0;
}

void EdenGame::deplaalldino() {
	perso_t *perso = &kPersons[PER_UNKN_18C - 1];   //TODO fix this
	while ((++perso)->_roomNum != 0xFFFF) {
		if (((perso->_roomNum >> 8) & 0xFF) != p_global->cita_area_num)
			continue;
		if (perso->_flags & PersonFlags::pf80)
			continue;
		if (!perso->_targetLoc)
			continue;
		if (--perso->_steps)
			continue;
		perso->_steps = 1;
		if (perso->_roomNum == p_global->roomNum)
			continue;
		perso->_steps = perso->_speed;
		depladino(perso);
	}
}

void EdenGame::newvallee() {
	perso_t *perso = &kPersons[PER_UNKN_372];
	int16 *ptr = tab_2CB16;
	int16 roomNum;
	while ((roomNum = *ptr++) != -1) {
		perso->_roomNum = roomNum;
		perso->_flags &= ~PersonFlags::pf80;
		perso->_flags &= ~PersonFlags::pf20; //TODO: combine?
		perso++;
	}
	perso->_roomNum = 0xFFFF;
	kAreasTable[7]._flags |= AreaFlags::HasTyrann;
	p_global->worldHasTyrann = 32;
}

char EdenGame::whereiscita() {
	char res = -1;
	for (room_t *room = p_global->cita_area_firstRoom; room->ff_0 != 0xFF; room++) {
		if (!(room->flags & RoomFlags::rfHasCitadel))
			continue;
		res = room->location;
		break;
	}
	return res;
}

bool EdenGame::iscita(int16 loc) {
	loc &= 0xFF;
	for (room_t *room = p_global->cita_area_firstRoom; room->ff_0 != 0xFF; room++) {
		if (!(room->flags & RoomFlags::rfHasCitadel))
			continue;
		if (room->location == loc + 16)
			return true;
		if (room->location == loc - 16)
			return true;
		if (room->location == loc - 1)
			return true;
		if (room->location == loc + 1)
			return true;
	}
	return false;
}

void EdenGame::lieuvava(area_t *area) {
	if (area->type == AreaType::atValley) {
		istyranval(area);
		area->citadelLevel = 0;
		if (area->citadelRoom)
			area->citadelLevel = p_global->cita_area_firstRoom->level;  //TODO: no search?
		byte mask = ~(1 << (area->num - Areas::arChamaar));
		p_global->worldTyrannSighted &= mask;
		p_global->ff_4E &= mask;
		p_global->worldGaveGold &= mask;
		p_global->worldHasVelociraptors &= mask;
		p_global->worldHasTriceraptors &= mask;
		p_global->worldHasTyrann &= mask;
		p_global->ff_53 &= mask;
		mask = ~mask;
		if (area->_flags & AreaFlags::TyrannSighted)
			p_global->worldTyrannSighted |= mask;
		if (area->_flags & AreaFlags::afFlag4)
			p_global->ff_4E |= mask;
		if (area->_flags & AreaFlags::HasTriceraptors)
			p_global->worldHasTriceraptors |= mask;
		if (area->_flags & AreaFlags::afGaveGold)
			p_global->worldGaveGold |= mask;
		if (area->_flags & AreaFlags::HasVelociraptors)
			p_global->worldHasVelociraptors |= mask;
		if (area->_flags & AreaFlags::HasTyrann)
			p_global->worldHasTyrann |= mask;
		if (area->_flags & AreaFlags::afFlag20)
			p_global->ff_53 |= mask;
		if (area == p_global->_areaPtr) {
			p_global->_curAreaFlags = area->_flags;
			p_global->curCitadelLevel = area->citadelLevel;
		}
	}
	p_global->ff_4D &= p_global->worldTyrannSighted;
}

void EdenGame::vivredino() {
	for (perso_t *perso = &kPersons[PER_UNKN_18C]; perso->_roomNum != 0xFFFF; perso++) {
		if (((perso->_roomNum >> 8) & 0xFF) != p_global->cita_area_num)
			continue;
		if (perso->_flags & PersonFlags::pf80)
			continue;
		switch (perso->_flags & PersonFlags::pfTypeMask) {
		case PersonFlags::pftTyrann:
			if (iscita(perso->_roomNum))
				perso->_targetLoc = 0;
			else if (!perso->_targetLoc) {
				char cita = whereiscita();
				if (cita != -1) {
					perso->_targetLoc = cita;
					perso->_speed = 2;
					perso->_steps = 1;
				}
			}
			break;
		case PersonFlags::pftTriceraptor:
			if (perso->_flags & PersonFlags::pfInParty) {
				if (iscita(perso->_roomNum))
					perso->_targetLoc = 0;
				else if (!perso->_targetLoc) {
					char cita = whereiscita();
					if (cita != -1) {
						perso->_targetLoc = cita;
						perso->_speed = 3;
						perso->_steps = 1;
					}
				}
			}
			break;
		case PersonFlags::pftVelociraptor:
			if (perso->_flags & PersonFlags::pf10) {
				if (perso->_roomNum == p_global->roomNum) {
					perso_t *perso2 = &kPersons[PER_UNKN_372];
					bool found = false;
					for (; perso2->_roomNum != 0xFFFF; perso2++) {
						if ((perso->_roomNum & ~0xFF) == (perso2->_roomNum & ~0xFF)) {
							if (perso2->_flags & PersonFlags::pf80)
								continue;
							perso->_targetLoc = perso2->_roomNum & 0xFF;
							perso->_steps = 1;
							found = true;
							break;
						}
					}
					if (found)
						continue;
				} else {
					tyranPtr = &kPersons[PER_UNKN_372];
					if (istyran(perso->_roomNum)) {
						if (p_global->_phaseNum < 481 && (perso->_powers & (1 << (p_global->cita_area_num - 3)))) {
							tyranPtr->_flags |= PersonFlags::pf80;
							tyranPtr->_roomNum = 0;
							perso->_flags &= ~PersonFlags::pf10;
							perso->_flags |= PersonFlags::pfInParty;
							ajouinfo(p_global->cita_area_num + ValleyNews::vnTyrannLost);
							delinfo(p_global->cita_area_num + ValleyNews::vnTyrannIn);
							if (naitredino(PersonFlags::pftTriceraptor))
								ajouinfo(p_global->cita_area_num + ValleyNews::vnTriceraptorsIn);
							constcita();
							p_global->cur_area_ptr->_flags &= ~AreaFlags::TyrannSighted;
						} else {
							perso->_flags &= ~PersonFlags::pf10;
							perso->_flags &= ~PersonFlags::pfInParty;
							ajouinfo(p_global->cita_area_num + ValleyNews::vnVelociraptorsLost);
						}
						continue;
					}
				}
			}
			if (!perso->_targetLoc) {
				int16 loc;
				perso->_lastLoc = 0;
				do {
					loc = (_vm->_rnd->getRandomNumber(63) & 63) + 16;
					if ((loc & 0xF) >= 12)
						loc &= ~4;  //TODO: ??? same as -= 4
				} while (!canMoveThere(loc, perso));
				perso->_targetLoc = loc;
				perso->_steps = 1;
			}
			break;
		}
	}
}

void EdenGame::vivreval(int16 areaNum) {
	p_global->cita_area_num = areaNum;
	p_global->cur_area_ptr = &kAreasTable[areaNum - 1];
	p_global->cita_area_firstRoom = &gameRooms[p_global->cur_area_ptr->firstRoomIndex];
	deplaalldino();
	constcita();
	vivredino();
	newchampi();
	newnido();
	newnidv();
	if (p_global->_phaseNum >= 226)
		newor();
	lieuvava(p_global->cur_area_ptr);
}

void EdenGame::chaquejour() {
	vivreval(3);
	vivreval(4);
	vivreval(5);
	vivreval(6);
	vivreval(7);
	vivreval(8);
	p_global->_drawFlags |= DrawFlags::drDrawTopScreen;
}

void EdenGame::temps_passe(int16 t) {
	int16 days = p_global->_gameDays;
	int16 lo = p_global->_gameHours + t;
	if (lo > 255) {
		days++;
		lo &= 0xFF;
	}

	p_global->_gameHours = lo;
	t = ((t >> 8) & 0xFF) + days;
	t -= p_global->_gameDays;
	if (t) {
		p_global->_gameDays += t;
		while (t--)
			chaquejour();
	}
}

void EdenGame::heurepasse() {
	temps_passe(5);
}

void EdenGame::anim_perso() {
	if (_curBankNum != p_global->_characterImageBank)
		loadCharacter(p_global->_characterPtr);
	restaurefondbulle();
	if (_restartAnimation) {
		_lastAnimTicks = _vm->_timerTicks;
		_restartAnimation = false;
	}
	_curAnimFrameNumb = (_vm->_timerTicks - _lastAnimTicks) >> 2;   // TODO: check me!!!
	if (_curAnimFrameNumb > _numAnimFrames)               // TODO: bug?
		_animateTalking = false;
	if (p_global->_curCharacterAnimPtr && !p_global->_animationFlags && _curAnimFrameNumb != _lastAnimFrameNumb) {
		_lastAnimFrameNumb = _curAnimFrameNumb;
		if (*p_global->_curCharacterAnimPtr == 0xFF)
			getanimrnd();
		_bankData = _characterBankData;
		num_img_desc = 0;
		perso_spr(p_global->_curCharacterAnimPtr);
		p_global->_curCharacterAnimPtr += num_img_desc + 1;
		_mouthAnimations = _imageDesc + 200;
		removeMouthSprite();
		if (*_mouthAnimations)
			displayImage();
		_animationDelay--;
		if (!_animationDelay) { //TODO: combine
			p_global->_animationFlags = 1;
			_animationDelay = 8;
		}
	}

	_animationDelay--;
	if (!_animationDelay) { //TODO: combine
		getanimrnd();
		//TODO: no reload?
	}
	if (_animateTalking) {
		if (!animationTable) {
			animationTable = gameLipsync + 7262;    //TODO: fix me
			if (!fond_saved)
				sauvefondbouche();
		}
		if (!_personTalking)
			_curAnimFrameNumb = _numAnimFrames - 1;
		animationIndex = animationTable[_curAnimFrameNumb];
		if (animationIndex == 0xFF)
			_animateTalking = false;
		else if (animationIndex != lastAnimationIndex) {
			_bankData = _characterBankData;
			restaurefondbouche();
//			debug("perso spr %d", animationIndex);
			perso_spr(p_global->persoSpritePtr2 + animationIndex * 2);  //TODO: int16s?
			_mouthAnimations = _imageDesc + 200;
			if (*_mouthAnimations)
				displayImage();
			lastAnimationIndex = animationIndex;
		}
	}
	af_subtitle();
}

void EdenGame::getanimrnd() {
	_animationDelay = 8;
	int16 rnd = _vm->_rnd->getRandomNumber(65535) & (byte)~0x18;    //TODO
	dword_30724 = p_global->persoSpritePtr + 16;    //TODO
	p_global->_curCharacterAnimPtr = p_global->persoSpritePtr + ((dword_30724[1] << 8) + dword_30724[0]);
	p_global->_animationFlags = 1;
	if (rnd >= 8)
		return;
	p_global->_animationFlags = 0;
	if (rnd <= 0)
		return;
	for (rnd *= 8; rnd > 0; rnd--) {
		while (*p_global->_curCharacterAnimPtr)
			p_global->_curCharacterAnimPtr++;
		p_global->_curCharacterAnimPtr++;
	}
}

void EdenGame::addanim() {
	lastAnimationIndex = 0xFF;
	_lastAnimTicks = 0;
	p_global->_animationFlags = 0xC0;
	p_global->_curCharacterAnimPtr = p_global->persoSpritePtr;
	getanimrnd();
	animationActive = true;
	if (p_global->_characterPtr == &kPersons[PER_ROI])
		return;
	perso_spr(p_global->persoSpritePtr + READ_LE_UINT16(p_global->persoSpritePtr));  //TODO: GetElem(0)
	_mouthAnimations = _imageDesc + 200;
	if (p_global->_characterPtr->_id != PersonId::pidCabukaOfCantura && p_global->_characterPtr->_targetLoc != 7) //TODO: targetLoc is minisprite idx
		removeMouthSprite();
	if (*_mouthAnimations)
		displayImage();
}

// Original name: virespritebouche
void EdenGame::removeMouthSprite() {
	byte *src = _mouthAnimations + 2;
	byte *dst = src;
	char cnt = _mouthAnimations[0];
	while (cnt--) {
		byte a = *src++;
		byte b = *src++;
		byte c = *src++;
		dst[0] = a;
		dst[1] = b;
		dst[2] = c;
		if (dword_30728[0] != 0xFF) {
			if ((a < dword_30728[0] || a > dword_30728[1])
			        && (a < dword_30728[2] || a > dword_30728[3]))
				dst += 3;
			else
				_mouthAnimations[0]--;
		} else
			dst += 3;
	}
}

void EdenGame::anim_perfin() {
	p_global->_animationFlags &= ~0x80;
	_animationDelay = 0;
	animationActive = false;
}

void EdenGame::perso_spr(byte *spr) {
	byte *img = _imageDesc + 200 + 2;
	int16 count = 0;
	byte c;
	while ((c = *spr++)) {
		byte *src;
		int16 index = 0;
		byte cc = 0;
		if (c == 1) {
			cc = index;
			c = *spr++;
		}
		num_img_desc++;
		index = (cc << 8) | c;
		index -= 2;
		//	debug("anim sprite %d", index);

		if (index > max_perso_desc)
			index = max_perso_desc;
		index *= 2;         //TODO: src = GetElem(ff_C2, index)
		src = p_global->ff_C2;
		src += READ_LE_UINT16(src + index);
		while ((c = *src++)) {
			*img++ = c;
			*img++ = *src++;
			*img++ = *src++;
			count++;
		}
	}
	_imageDesc[200] = count & 0xFF;
	_imageDesc[201] = count >> 8;
}

// Original name: af_image
void EdenGame::displayImage() {
	byte *img = _imageDesc + 200;

	int16 count = READ_LE_UINT16(img);
	if (!count)
		return;

	byte *img_start = img;
	byte *curimg = _imageDesc;

	img += 2;
	count *= 3;
	while (count--)
		*curimg++ = *img++;
	img = img_start;
	count = READ_LE_UINT16(img);
	img += 2;
	/////// draw it
	while (count--) {
		uint16 index = *img++;
		uint16 x = *img++ + _gameIcons[0].sx;
		uint16 y = *img++ + _gameIcons[0].sy;
		byte *pix = _bankData;
		byte *scr = p_mainview_buf + x + y * 640;
		index--;
		if (READ_LE_UINT16(pix) > 2)
			readPalette(pix + 2);
		pix += READ_LE_UINT16(pix);
		pix += READ_LE_UINT16(pix + index * 2);
		//  int16   height:9
		//  int16   pad:6;
		//  int16   flag:1;
		byte h0 = *pix++;
		byte h1 = *pix++;
		int16 w = ((h1 & 1) << 8) | h0;
		int16 h = *pix++;
		byte mode = *pix++;
		if (mode != 0xFF && mode != 0xFE)
			continue;   //TODO: enclosing block?
		if (h1 & 0x80) {
			// compressed
			for (; h-- > 0;) {
				for (int16 ww = w; ww > 0;) {
					byte c = *pix++;
					if (c >= 0x80) {
						if (c == 0x80) {
							byte fill = *pix++;
							if (fill == 0) {
								scr += 128 + 1;
								ww -= 128 + 1;
							} else {
								byte run;
								*scr++ = fill;  //TODO: wha?
								*scr++ = fill;
								ww -= 128 + 1;
								for (run = 127; run--;)
									*scr++ = fill;
							}
						} else {
							byte fill = *pix++;
							byte run = 255 - c + 2;
							ww -= run;
							if (fill == 0)
								scr += run;
							else {
								for (; run--;)
									*scr++ = fill;
							}
						}
					} else {
						byte run = c + 1;
						ww -= run;
						for (; run--;) {
							byte p = *pix++;
							if (p == 0)
								scr++;
							else
								*scr++ = p;
						}
					}
				}
				scr += 640 - w;
			}
		} else {
			// uncompressed
			for (; h--;) {
				for (int16 ww = w; ww--;) {
					byte p = *pix++;
					if (p == 0)
						scr++;
					else
						*scr++ = p;
				}
				scr += 640 - w;
			}
		}
	}
}

void EdenGame::af_perso1() {
	perso_spr(p_global->persoSpritePtr + READ_LE_UINT16(p_global->persoSpritePtr));
	displayImage();
}

void EdenGame::af_perso() {
	load_perso_cour();
	af_perso1();
}

void EdenGame::ef_perso() {
	p_global->_animationFlags &= 0x3F;
}

// Original name: load_perso
void EdenGame::loadCharacter(perso_t *perso) {
	_characterBankData = nullptr;
	if (!perso->_spriteBank)
		return;

	if (perso->_spriteBank != p_global->_characterImageBank) {
		cur_perso_rect = &perso_rects[perso->_id];   //TODO: array of int16?
		dword_30728 = tab_persxx[perso->_id];
		ef_perso();
		p_global->_characterImageBank = perso->_spriteBank;
		useBank(p_global->_characterImageBank);
		_characterBankData = _bankData;
		byte *ptr = _bankData;
		ptr += READ_LE_UINT16(ptr);
		byte *baseptr = ptr;
		ptr += READ_LE_UINT16(ptr) - 2;
		ptr = baseptr + READ_LE_UINT16(ptr) + 4;
		_gameIcons[0].sx = READ_LE_UINT16(ptr);
		_gameIcons[0].sy = READ_LE_UINT16(ptr + 2);
		_gameIcons[0].ex = READ_LE_UINT16(ptr + 4);
		_gameIcons[0].ey = READ_LE_UINT16(ptr + 6);
		ptr += 8;
		p_global->ff_C2 = ptr + 2;
		max_perso_desc = READ_LE_UINT16(ptr) / 2;
		ptr += READ_LE_UINT16(ptr);
		baseptr = ptr;
		ptr += READ_LE_UINT16(ptr) - 2;
		p_global->persoSpritePtr = baseptr;
		p_global->persoSpritePtr2 = baseptr + READ_LE_UINT16(ptr);
		debug("load perso: b6 len is %ld", p_global->persoSpritePtr2 - p_global->persoSpritePtr);
	} else {
		useBank(p_global->_characterImageBank);
		_characterBankData = _bankData;
	}
}

void EdenGame::load_perso_cour() {
	loadCharacter(p_global->_characterPtr);
}

void EdenGame::fin_perso() {
	p_global->_animationFlags &= 0x3F;
	p_global->_curCharacterAnimPtr = nullptr;
	p_global->ff_CA = 0;
	p_global->_characterImageBank = -1;
	anim_perfin();
}

void EdenGame::no_perso() {
	if (p_global->_displayFlags == DisplayFlags::dfPerson) {
		p_global->_displayFlags = p_global->oldDisplayFlags;
		fin_perso();
	}
	endpersovox();
}

void EdenGame::close_perso() {
	endpersovox();
	if (p_global->_displayFlags == DisplayFlags::dfPerson && p_global->_characterPtr->_id != PersonId::pidNarrator && p_global->_eventType != EventType::etEventE) {
		rundcurs();
		savedUnderSubtitles = true;
		restaurefondbulle();
		afficher();
		p_global->ff_103 = 16;
	}
	if (p_global->_characterPtr->_id == PersonId::pidNarrator)
		p_global->ff_103 = 69;
	p_global->eloiHaveNews &= 1;
	p_global->ff_CA = 0;
	p_global->ff_F6 = 0;
	if (p_global->_displayFlags == DisplayFlags::dfPerson) {
		p_global->_displayFlags = p_global->oldDisplayFlags;
		p_global->_animationFlags &= 0x3F;
		p_global->_curCharacterAnimPtr = nullptr;
		anim_perfin();
		if (p_global->_displayFlags & DisplayFlags::dfMirror) {
			gametomiroir(1);
			_scrollPos = _oldScrollPos;
			scroll();
			return;
		}
		if (p_global->numGiveObjs) {
			if (!(p_global->_displayFlags & DisplayFlags::dfFlag2))
				showObjects();
			p_global->numGiveObjs = 0;
		}
		if (p_global->ff_F2 & 1) {
			p_global->ff_102 = 6;
			p_global->ff_F2 &= ~1;
		}
		char oldLoc = p_global->newLocation;
		p_global->newLocation = 0;
		if (!(p_global->narratorSequence & 0x80))
			p_global->ff_100 = 0xFF;
		maj_salle(p_global->roomNum);
		p_global->newLocation = oldLoc;
	}

	if (p_global->chrono)
		p_global->_chronoFlag = 1;
}

// Original name: af_fondsuiveur
void EdenGame::displayBackgroundFollower() {
	char id = p_global->_characterPtr->_id;
	for (Follower *follower = followerList; follower->_id != -1; follower++) {
		if (follower->_id == id) {
			int bank = 326;
			if (follower->sx >= 320)
				bank = 327;
			useBank(bank + p_global->roomBgBankNum);
			noclipax_avecnoir(0, 0, 16);
			break;
		}
	}
}

void EdenGame::af_fondperso1() {
	byte bank;
	char *ptab;
	if (p_global->_characterPtr == &kPersons[PER_MESSAGER]) {
		_gameIcons[0].sx = 0;
		perso_rects[PER_MESSAGER].sx = 2;
		bank = p_global->_characterBackgroundBankIdx;
		if (p_global->_eventType == EventType::etEventE) {
			p_global->ff_103 = 1;
			goto no_suiveur;
		}
		_gameIcons[0].sx = 60;
		perso_rects[PER_MESSAGER].sx = 62;
	}
	if (p_global->_characterPtr == &kPersons[PER_THOO]) {
		bank = 37;
		if (p_global->_curObjectId == Objects::obShell)
			goto no_suiveur;
	}
	ptab = kPersoRoomBankTable + p_global->_characterPtr->_roomBankId;
	bank = *ptab++;
	if (!(p_global->_characterPtr->_partyMask & p_global->party)) {
		while ((bank = *ptab++) != 0xFF) {
			if (bank == (p_global->roomNum & 0xFF)) { //TODO: signed vs unsigned - chg r31 to uns?
				bank = *ptab;
				break;
			}
			ptab++;
		}
		if (bank != 0xFF)
			goto no_suiveur;
		ptab = kPersoRoomBankTable + p_global->_characterPtr->_roomBankId;
		bank = *ptab++;
	}
	displayBackgroundFollower();
no_suiveur:
	;
	if (!bank)
		return;
	useBank(bank);
	if (p_global->_characterPtr == &kPersons[PER_UNKN_156])
		noclipax_avecnoir(0, 0, 16);
	else
		noclipax(0, 0, 16);
}

void EdenGame::af_fondperso() {
	if (p_global->_characterPtr->_spriteBank) {
		fond_saved = false;
		af_fondperso1();
	}
}

// Original name: setpersoicon
void EdenGame::setCharacterIcon() {
	if (p_global->_iconsIndex == 4)
		return;

	if (p_global->_characterPtr == &kPersons[PER_MESSAGER] && p_global->_eventType == EventType::etEventE) {
		p_global->_iconsIndex = 123;
		return;
	}
	icon_t *icon = _gameIcons;
	icon_t *icon2 = &_gameIcons[roomIconsBase];

	*icon2++ = *icon++; //TODO: is this ok?
	*icon2++ = *icon++;
	icon2->sx = -1;
}

// Original name: show_perso
void EdenGame::showCharacter() {
	perso_t *perso = p_global->_characterPtr;
	if (perso->_spriteBank) {
		closesalle();
		if (p_global->_displayFlags != DisplayFlags::dfPerson) {
			if (p_global->_displayFlags & DisplayFlags::dfMirror)
				resetScroll();
			p_global->oldDisplayFlags = p_global->_displayFlags;
			p_global->_displayFlags = DisplayFlags::dfPerson;
			loadCharacter(perso);
			setCharacterIcon();
			af_fondperso();
			if (perso == &kPersons[PER_THOO] && p_global->_curObjectId == Objects::obShell) {
				af_subtitle();
				update_cursor();
				needPaletteUpdate = true;
				afficher();
				rundcurs();
				return;
			}
		}
		load_perso_cour();
		addanim();
		if (!p_global->_curCharacterAnimPtr) {
			af_perso();
			af_subtitle();
		}
		_restartAnimation = true;
		anim_perso();
		if (perso != &kPersons[PER_UNKN_156])
			update_cursor();
		needPaletteUpdate = true;
		if (perso != &kPersons[PER_UNKN_156])
			rundcurs();
		afficher();
	} else {
		displayPlace();
		af_subtitle();
	}
}

void EdenGame::showpersopanel() {
	perso_t *perso = p_global->_characterPtr;
	load_perso_cour();
	addanim();
	if (!p_global->_curCharacterAnimPtr) {
		af_perso();
		af_subtitle();
	}
	_restartAnimation = true;
	needPaletteUpdate = true;
	if (p_global->_drawFlags & DrawFlags::drDrawFlag8)
		return;
	anim_perso();
	if (perso != &kPersons[PER_UNKN_156])
		update_cursor();
	afficher();
	if (perso != &kPersons[PER_UNKN_156])
		rundcurs();
	p_global->_drawFlags |= DrawFlags::drDrawFlag8;
	p_global->_iconsIndex = 112;
}

void EdenGame::getdatasync() {
	int16 num = p_global->textNum;
	if (p_global->textBankIndex != 1)
		num += 565;
	if (p_global->textBankIndex == 3)
		num += 707;
	if (num == 144)
		num = 142;
	_animateTalking = ReadDataSync(num - 1);
	if (_animateTalking)
		_numAnimFrames = ReadNombreFrames();
	else
		_numAnimFrames = 0;
	if (p_global->textNum == 144)
		_numAnimFrames = 48;
	animationTable = 0;
}

int16 EdenGame::ReadNombreFrames() {
	int16 num = 0;
	animationTable = gameLipsync + 7260 + 2;    //TODO: fix me
	while (*animationTable++ != 0xFF)
		num++;
	return num;
}

void EdenGame::waitEndSpeak() {
	for (;;) {
		if (animationActive)
			anim_perso();
		musicspy();
		afficher();
		CLKeyboard_Read();
		testPommeQ();
		if (pomme_q) {
			close_perso();
			PommeQ();
			break;
		}
		if (!mouse_held)
			if (CLMouse_IsDown())
				break;
		if (mouse_held)
			if (!CLMouse_IsDown())
				mouse_held = false;
	}
	mouse_held = true;
}

void EdenGame::my_bulle() {
	if (!p_global->textNum)
		return;

	byte *icons = phraseIconsBuffer;
	byte *linesp = phraseCoordsBuffer;
	byte *phrasePtr = phraseBuffer;
	p_global->numGiveObjs = 0;
	p_global->giveobj1 = 0;
	p_global->giveobj2 = 0;
	p_global->giveobj3 = 0;
	p_global->textWidthLimit = subtitles_x_width;
	text_ptr = gettxtad(p_global->textNum);
	num_text_lines = 0;
	int16 words_on_line = 0;
	int16 word_width = 0;
	int16 line_width = 0;
	byte c;
	while ((c = *text_ptr++) != 0xFF) {
		if (c == 0x11 || c == 0x13) {
			if (p_global->_phaseNum <= 272 || p_global->_phaseNum == 386) {
				p_global->eloiHaveNews = c & 0xF;
				p_global->ff_4D = p_global->worldTyrannSighted;
			}
		} else if (c >= 0x80 && c < 0x90)
			SysBeep(1);
		else if (c >= 0x90 && c < 0xA0) {
			while (*text_ptr++ != 0xFF) ;
			text_ptr--;
		} else if (c >= 0xA0 && c < 0xC0)
			p_global->textToken1 = c & 0xF;
		else if (c >= 0xC0 && c < 0xD0)
			p_global->textToken2 = c & 0xF;
		else if (c >= 0xD0 && c < 0xE0) {
			byte c1 = *text_ptr++;
			if (c == 0xD2)
#ifdef FAKE_DOS_VERSION
				p_global->textWidthLimit = c1 + 160;
#else
				p_global->textWidthLimit = c1 + subtitles_x_center; //TODO: signed? 160 in pc ver
#endif
			else {
				byte c2 = *text_ptr++;
				switch (p_global->numGiveObjs) {
				case 0:
					p_global->giveobj1 = c2;
					break;
				case 1:
					p_global->giveobj2 = c2;
					break;
				case 2:
					p_global->giveobj3 = c2;
					break;
				}
				p_global->numGiveObjs++;
				*icons++ = *text_ptr++;
				*icons++ = *text_ptr++;
				*icons++ = c2;
			}
		} else if (c >= 0xE0 && c < 0xFF)
			SysBeep(1);
		else if (c != '\r') {
			byte width;
			int16 overrun;
			*phrasePtr++ = c;
			width = gameFont[c];
#ifdef FAKE_DOS_VERSION
			if (c == ' ')
				width = space_width;
#endif
			word_width += width;
			line_width += width;
			overrun = line_width - p_global->textWidthLimit;
			if (overrun > 0) {
				num_text_lines++;
				if (c != ' ') {
					*linesp++ = words_on_line;
					*linesp++ = word_width + space_width - overrun;
					line_width = word_width;
				} else {
					*linesp++ = words_on_line + 1;
					*linesp++ = space_width - overrun;   //TODO: checkme
					line_width = 0;
				}
				word_width = 0;
				words_on_line = 0;
			} else {
				if (c == ' ') {
					words_on_line++;
					word_width = 0;
				}
			}
		}
	}
	num_text_lines++;
	*linesp++ = words_on_line + 1;
	*linesp++ = word_width;
	*phrasePtr = c;
	if (p_global->textBankIndex == 2 && p_global->textNum == 101 && p_global->pref_language == 1)
		patchPhrase();
	my_pr_bulle();
	if (!p_global->numGiveObjs)
		return;
	use_main_bank();
	if (num_text_lines < 3)
		num_text_lines = 3;
	icons = phraseIconsBuffer;
	for (byte i = 0; i < p_global->numGiveObjs; i++) {
		byte x = *icons++;
		byte y = *icons++;
		byte s = *icons++;
		spriteOnSubtitle(52, x + subtitles_x_center, y - 1);
		spriteOnSubtitle(s + 9, x + subtitles_x_center + 1, y);
	}
}

void EdenGame::my_pr_bulle() {
	CLBlitter_FillView(p_subtitlesview, 0);
	if (p_global->pref_language == 0)
		return;

	byte *coo = phraseCoordsBuffer;
	bool done = false;
	textout = p_subtitlesview_buf;
	text_ptr = phraseBuffer;
	int16 lines = 1;
	while (!done) {
		int16 num_words = *coo++;       // num words on line
		int16 pad_size = *coo++;        // amount of extra spacing
		byte *cur_out = textout;
		int16 extraSpacing = num_words > 1 ? pad_size / (num_words - 1) + 1 : 0;
		if (lines == num_text_lines)
			extraSpacing = 0;
		byte c = *text_ptr++;
		while (!done & (num_words > 0)) { //TODO: bug - missed & ?
			if (c < 0x80 && c != '\r') {
				if (c == ' ') {
					num_words--;
					if (pad_size >= extraSpacing) {
						textout += extraSpacing + space_width;
						pad_size -= extraSpacing;
					} else {
						textout += pad_size + space_width;
						pad_size = 0;
					}
				} else {
					int16 char_width = gameFont[c];
					if (!(p_global->_drawFlags & DrawFlags::drDrawMenu)) {
						textout += subtitles_x_width;
						if (!specialTextMode)
							charsurbulle(c, 195, char_width);
						textout++;
						if (!specialTextMode)
							charsurbulle(c, 195, char_width);
						textout -= subtitles_x_width + 1;
					}
					if (specialTextMode)
						charsurbulle(c, 250, char_width);
					else
						charsurbulle(c, 230, char_width);
					textout += char_width;
				}
			} else
				error("my_pr_bulle: Unexpected format");

			c = *text_ptr++;
			if (c == 0xFF)
				done = true;
		}
		textout = cur_out + subtitles_x_width * FONT_HEIGHT;
		lines++;
		text_ptr--;
	}
}

void EdenGame::charsurbulle(byte c, byte color, int16 width) {
	byte *glyph = gameFont + 256 + c * FONT_HEIGHT;
	textoutptr = textout;
	for (int16 h = 0; h < FONT_HEIGHT; h++) {
		byte bits = *glyph++;
		int16 mask = 0x80;
		for (int16 w = 0; w < width; w++) {
			if (bits & mask)
				*textoutptr = color;
			textoutptr++;
			mask >>= 1;
		}
		textoutptr += subtitles_x_width - width;
	}
}

void EdenGame::af_subtitle() {
	byte *src = p_subtitlesview_buf;
	byte *dst = p_mainview_buf;
	int16 y;
	if (p_global->_displayFlags & DisplayFlags::dfFlag2) {
		y = 174;
		if ((p_global->_drawFlags & DrawFlags::drDrawMenu) && num_text_lines == 1)
			y = 167;
		dst += 640 * (y - num_text_lines * FONT_HEIGHT) + subtitles_x_scr_margin;
	} else {
		y = 174;
		dst += 640 * (y - num_text_lines * FONT_HEIGHT) + _scrollPos + subtitles_x_scr_margin;
	}
	if (animationActive && !_personTalking)
		return;
	sauvefondbulle(y);
	for (int16 h = 0; h < num_text_lines * FONT_HEIGHT + 1; h++) {
		for (int16 w = 0; w < subtitles_x_width; w++) {
			byte c = *src++;
			if (c)
				*dst = c;
			dst++;
		}
		dst += 640 - subtitles_x_width;
	}
}

void EdenGame::sauvefondbulle(int16 y) {
	_underSubtitlesScreenRect.top = y - num_text_lines * FONT_HEIGHT;
	_underSubtitlesScreenRect.left = _scrollPos + subtitles_x_scr_margin;
	_underSubtitlesScreenRect.right = _scrollPos + subtitles_x_scr_margin + subtitles_x_width - 1;
	_underSubtitlesScreenRect.bottom = y;
	_underSubtitlesBackupRect.top = 0;
	_underSubtitlesBackupRect.bottom = num_text_lines * FONT_HEIGHT;
	CLBlitter_CopyViewRect(p_mainview, p_underSubtitlesView, &_underSubtitlesScreenRect, &_underSubtitlesBackupRect);
	savedUnderSubtitles = true;
}

void EdenGame::restaurefondbulle() {
	if (!savedUnderSubtitles)
		return;
	CLBlitter_CopyViewRect(p_underSubtitlesView, p_mainview, &_underSubtitlesBackupRect, &_underSubtitlesScreenRect);
	savedUnderSubtitles = false;
}

void EdenGame::af_subtitlehnm() {
	byte *src = p_subtitlesview_buf;
	byte *dst = p_hnmview_buf + subtitles_x_scr_margin + (158 - num_text_lines * FONT_HEIGHT) * 320;
	for (int16 y = 0; y < num_text_lines * FONT_HEIGHT; y++) {
		for (int16 x = 0; x < subtitles_x_width; x++) {
			char c = *src++;
			if (c)
				*dst = c;
			dst++;
		}
		dst += 320 - subtitles_x_width;
	}
}

void EdenGame::patchPhrase() {
	phraseBuffer[36] = 'c';
}

void EdenGame::vavapers() {
	perso_t *perso = p_global->_characterPtr;
	p_global->curPersoFlags = perso->_flags;
	p_global->curPersoItems = perso->_items;
	p_global->_curCharacterPowers = perso->_powers;
}

void EdenGame::citadelle() {
	p_global->ff_69++;
	p_global->ff_F6++;
	parlemoiNormalFlag = true;
	_closeCharacterDialog = true;
}

void EdenGame::choixzone() {
	if (p_global->giveobj3)
		p_global->_iconsIndex = 6;
	else
		p_global->_iconsIndex = 10;
	p_global->_autoDialog = false;
	putObject();
}

void EdenGame::showevents1() {
	p_global->ff_113 = 0;
	perso_ici(3);
}

void EdenGame::showEvents() {
	if (p_global->_eventType && p_global->_displayFlags != DisplayFlags::dfPerson)
		showevents1();
}

void EdenGame::parle_mfin() {
	perso_t *perso = p_global->_characterPtr;
	if (p_global->_curObjectId) {
		char curobj = p_global->_curObjectId;
		if (p_global->_dialogType == DialogType::dtHint)
			return;
		object_t *obj = getobjaddr(p_global->_curObjectId);
		if (p_global->_dialogType == DialogType::dtDinoItem)
			perso = p_global->_roomCharacter;
		if (isAnswerYes()) {
			loseObject(p_global->_curObjectId);
			perso->_powers |= obj->_powerMask;
		}
		perso->_items |= obj->_itemMask;
		SpecialObjets(perso, curobj);
		return;
	}
	if (!isAnswerYes())
		return;
	nextinfo();
	if (!p_global->_lastInfo)
		_closeCharacterDialog = true;
	else {
		p_global->_nextDialogPtr = nullptr;
		_closeCharacterDialog = false;
	}
}

void EdenGame::parlemoi_normal() {
	dial_t *dial;
	if (!p_global->_nextDialogPtr) {
		perso_t *perso = p_global->_characterPtr;
		if (perso) {
			int16 num = (perso->_id << 3) | p_global->_dialogType;
			dial = (dial_t *)getElem(gameDialogs, num);
		} else {
			close_perso();
			return;
		}
	} else {
		if (_closeCharacterDialog) {
			close_perso();
			return;
		}
		dial = p_global->_nextDialogPtr;
	}
	char ok = dial_scan(dial);
	p_global->_nextDialogPtr = p_global->_dialogPtr;
	_closeCharacterDialog = false;
	if (!ok)
		close_perso();
	else
		parle_mfin();
}

void EdenGame::parle_moi() {
	byte r28;
	char ok;
	dial_t *dial;
	endpersovox();
	r28 = p_global->ff_F6;
	p_global->ff_F6 = 0;
	if (!r28) {
		setChoiceNo();
		if (p_global->_drawFlags & DrawFlags::drDrawInventory)
			showObjects();
		if (p_global->_drawFlags & DrawFlags::drDrawTopScreen)
			drawTopScreen();
		if (p_global->_curObjectId) {
			if (p_global->_dialogType == DialogType::dtTalk) {
				p_global->_dialogType = DialogType::dtItem;
				p_global->_nextDialogPtr = nullptr;
				_closeCharacterDialog = false;
			}
			parlemoi_normal();
			return;
		}
		if (p_global->_dialogType == DialogType::dtItem) {
			p_global->_dialogType = DialogType::dtTalk;
			if (!_closeCharacterDialog)
				p_global->_nextDialogPtr = nullptr;
		}
		if (parlemoiNormalFlag) {
			parlemoi_normal();
			return;
		}
		if (!p_global->last_dialog_ptr) {
			int16 num = 160;
			if (p_global->_phaseNum >= 400)
				num++;
			dial = (dial_t *)getElem(gameDialogs, num);
		} else
			dial = p_global->last_dialog_ptr;
		ok = dial_scan(dial);
		p_global->last_dialog_ptr = p_global->_dialogPtr;
		parlemoiNormalFlag = false;
		if (!ok) {
			parlemoiNormalFlag = true;
			if (p_global->ff_60) {
				if (p_global->_characterPtr == &kPersons[PER_MESSAGER]) {
					p_global->_dialogType = DialogType::dtTalk;
					if (p_global->eloiHaveNews)
						parlemoi_normal();
					else
						close_perso();
				} else
					close_perso();
			} else
				parlemoi_normal();
		} else
			parle_mfin();
	} else
		close_perso();
}

void EdenGame::init_perso_ptr(perso_t *perso) {
	p_global->metPersonsMask1 |= perso->_partyMask;
	p_global->metPersonsMask2 |= perso->_partyMask;
	p_global->_nextDialogPtr = nullptr;
	_closeCharacterDialog = false;
	dialogSkipFlags = DialogFlags::dfSpoken;
	p_global->ff_60 = false;
	p_global->textToken1 = 0;
}

void EdenGame::perso1(perso_t *perso) {
	p_global->phaseActionsCount++;
	if (perso == &kPersons[PER_THOO])
		p_global->phaseActionsCount--;
	p_global->_characterPtr = perso;
	init_perso_ptr(perso);
	parle_moi();
}

void EdenGame::perso_normal(perso_t *perso) {
	p_global->last_dialog_ptr = nullptr;
	p_global->_dialogType = DialogType::dtTalk;
	parlemoiNormalFlag = false;
	perso1(perso);
}

void EdenGame::persoparle(int16 pers) {
	perso_t *perso = &kPersons[pers];
	p_global->_characterPtr = perso;
	p_global->_dialogType = DialogType::dtInspect;
	uint16 idx = perso->_id * 8 | p_global->_dialogType;
	dialoscansvmas((dial_t *)getElem(gameDialogs, idx));
	displayPlace();
	af_subtitle();
	persovox();
	p_global->ff_CA = 0;
	p_global->_dialogType = DialogType::dtTalk;
}

void EdenGame::roi()  {
	perso_normal(&kPersons[PER_ROI]);
}

void EdenGame::dina() {
	perso_normal(&kPersons[PER_DINA]);
}

void EdenGame::thoo() {
	perso_normal(&kPersons[PER_THOO]);
}

void EdenGame::monk() {
	perso_normal(&kPersons[PER_MONK]);
}

void EdenGame::bourreau() {
	perso_normal(&kPersons[PER_BOURREAU]);
}

void EdenGame::messager() {
	perso_normal(&kPersons[PER_MESSAGER]);
}

void EdenGame::mango()    {
	perso_normal(&kPersons[PER_MANGO]);
}

void EdenGame::eve()  {
	perso_normal(&kPersons[PER_EVE]);
}

void EdenGame::azia() {
	perso_normal(&kPersons[PER_AZIA]);
}

void EdenGame::mammi() {
	perso_t *perso;
	for (perso = &kPersons[PER_MAMMI]; perso->_partyMask == PersonMask::pmLeader; perso++) {
		if (perso->_roomNum == p_global->roomNum) {
			perso_normal(perso);
			break;
		}
	}
}

void EdenGame::gardes()   {
	perso_normal(&kPersons[PER_GARDES]);
}

void EdenGame::bambou()   {
	perso_normal(&kPersons[PER_BAMBOO]);
}

void EdenGame::kabuka()   {
	if (p_global->roomNum == 0x711) perso_normal(&kPersons[PER_KABUKA]);
	else bambou();
}

void EdenGame::fisher()   {
	if (p_global->roomNum == 0x902) perso_normal(&kPersons[PER_FISHER]);
	else kabuka();
}

void EdenGame::dino() {
	perso_t *perso = p_global->_roomCharacter;
	if (!perso)
		return;
	parlemoiNormalFlag = true;
	p_global->_dialogType = DialogType::dtTalk;
	p_global->_roomCharacterFlags = perso->_flags;
	p_global->_roomPersoItems = perso->_items;
	p_global->_roomCharacterPowers = perso->_powers;
	p_global->_characterPtr = perso;
	init_perso_ptr(perso);
	debug("beg dino talk");
	parle_moi();
	debug("end dino talk");
	if (p_global->_areaNum == Areas::arWhiteArch)
		return;
	if (p_global->ff_60)
		waitEndSpeak();
	if (pomme_q)
		return;
	perso = &kPersons[PER_MANGO];
	if (!(p_global->party & PersonMask::pmMungo)) {
		perso = &kPersons[PER_DINA];
		if (!(p_global->party & PersonMask::pmDina)) {
			perso = &kPersons[PER_EVE];
			if (!(p_global->party & PersonMask::pmEve)) {
				perso = &kPersons[PER_GARDES];
			}
		}
	}
	p_global->_dialogType = DialogType::dtDinoAction;
	if (p_global->_curObjectId)
		p_global->_dialogType = DialogType::dtDinoItem;
	perso1(perso);
	if (p_global->_roomCharacterType == PersonFlags::pftMosasaurus && !p_global->_curObjectId) {
		p_global->_areaPtr->_flags |= AreaFlags::afFlag20;
		lieuvava(p_global->_areaPtr);
	}
}

void EdenGame::tyran() {
	perso_t *perso = p_global->_roomCharacter;
	if (!perso)
		return;

	parlemoiNormalFlag = true;
	p_global->_dialogType = DialogType::dtTalk;
	p_global->_roomCharacterFlags = perso->_flags;
	p_global->_characterPtr = perso;
	init_perso_ptr(perso);
	perso = &kPersons[PER_MANGO];
	if (!(p_global->party & PersonMask::pmMungo)) {
		perso = &kPersons[PER_DINA];
		if (!(p_global->party & PersonMask::pmDina)) {
			perso = &kPersons[PER_EVE];
			if (!(p_global->party & PersonMask::pmEve)) {
				perso = &kPersons[PER_GARDES];
			}
		}
	}
	p_global->_dialogType = DialogType::dtDinoAction;
	if (p_global->_curObjectId)
		p_global->_dialogType = DialogType::dtDinoItem;
	perso1(perso);
}

void EdenGame::morkus()   {
	perso_normal(&kPersons[PER_MORKUS]);
}

void EdenGame::comment() {
	perso_t *perso = &kPersons[PER_DINA];
	if (!(p_global->party & PersonMask::pmDina)) {
		perso = &kPersons[PER_EVE];
		if (!(p_global->party & PersonMask::pmEve)) {
			perso = &kPersons[PER_GARDES];
			if (!(p_global->party & PersonMask::pmThugg))
				return;
		}
	}
	p_global->_dialogType = DialogType::dtHint;
	perso1(perso);
}

void EdenGame::adam() {
	resetScroll();
	switch (p_global->_curObjectId) {
	case Objects::obNone:
		gotopanel();
		break;
	case Objects::obRoot:
		if (p_global->roomNum == 2817
		        && p_global->_phaseNum > 496 && p_global->_phaseNum < 512) {
			bigphase1();
			loseObject(Objects::obRoot);
			p_global->ff_100 = 0xFF;
			quitMirror();
			maj_salle(p_global->roomNum);
			reste_ici(5);
			p_global->_eventType = EventType::etEvent3;
			showEvents();
			waitEndSpeak();
			if (pomme_q)
				return;
			close_perso();
			reste_ici(5);
			p_global->roomNum = 2818;
			p_global->_areaNum = Areas::arWhiteArch;
			p_global->_eventType = EventType::etEvent5;
			p_global->valleyVidNum = 0;
			p_global->ff_102 = 6;
			p_global->newMusicType = MusicType::mtNormal;
			maj_salle(p_global->roomNum);
		} else {
			p_global->_dialogType = DialogType::dtHint;
			perso1(&kPersons[PER_EVE]);
		}
		break;
	case Objects::obShell:
		p_global->_dialogType = DialogType::dtHint;
		perso1(&kPersons[PER_THOO]);
		break;
	case Objects::obFlute:
	case Objects::obTrumpet:
		if (p_global->_roomCharacterType) {
			quitMirror();
			maj_salle(p_global->roomNum);
			dino();
		} else
			comment();
		break;
	case Objects::obTablet1:
	case Objects::obTablet2:
	case Objects::obTablet3:
	case Objects::obTablet4:
	case Objects::obTablet5:
	case Objects::obTablet6: {
		if ((p_global->_partyOutside & PersonMask::pmDina)
		        && p_global->_curObjectId == Objects::obTablet1 && p_global->_phaseNum == 370)
			incPhase1();
		char *objvid = &kTabletView[(p_global->_curObjectId - Objects::obTablet1) * 2];
		object_t *object = getobjaddr(*objvid++);
		int16 vid = 84;
		if (!object->_count)
			vid = *objvid;
		bars_out();
		specialTextMode = true;
		playHNM(vid);
		needPaletteUpdate = true;
		p_global->ff_102 = 16;
		showBars();
		gametomiroir(0);
		}
		break;
	case Objects::obApple:
	case Objects::obShroom:
	case Objects::obBadShroom:
	case Objects::obNest:
	case Objects::obFullNest:
	case Objects::obDrum:
		if (p_global->party & PersonMask::pmThugg) {
			p_global->_dialogType = DialogType::dtHint;
			perso1(&kPersons[PER_GARDES]);
		}
		break;
	default:
		comment();
	}
}

// Original name: oui and init_oui
void EdenGame::setChoiceYes()  {
	_lastDialogChoice = true;
}

// Original name: non and init_non
void EdenGame::setChoiceNo()  {
	_lastDialogChoice =  false;
}

// Original name: verif_oui
bool EdenGame::isAnswerYes() {
	return _lastDialogChoice;
}

// Original name: SpcChampi
void EdenGame::specialMushroom(perso_t *perso) {
	perso->_flags |= PersonFlags::pf10;
	p_global->_areaPtr->_flags |= AreaFlags::afFlag2;
	p_global->_curAreaFlags |= AreaFlags::afFlag2;
}

// Original name: SpcNidv
void EdenGame::specialNidv(perso_t *perso) {
	if (!isAnswerYes())
		return;
	perso->_flags |= PersonFlags::pf10;
	p_global->_roomCharacterFlags |= PersonFlags::pf10;
	p_global->_gameFlags |= GameFlags::gfFlag400;
	if (p_global->_characterPtr == &kPersons[PER_EVE]) {
		p_global->_areaPtr->_flags |= AreaFlags::afFlag4;
		p_global->_curAreaFlags |= AreaFlags::afFlag4;
		perso->_flags |= PersonFlags::pfInParty;
		p_global->_roomCharacterFlags |= PersonFlags::pfInParty;
		lieuvava(p_global->_areaPtr);
	} else {
		perso->_flags &= ~PersonFlags::pf10;
		p_global->_roomCharacterFlags &= ~PersonFlags::pf10;
	}
}

// Original name: SpcNido
void EdenGame::specialNido(perso_t *perso) {
	if (perso == &kPersons[PER_GARDES])
		giveObject();
}

// Original name: SpcPomme
void EdenGame::specialApple(perso_t *perso) {
	perso->_flags |= PersonFlags::pf10;
	p_global->_areaPtr->_flags |= AreaFlags::afFlag8;
	p_global->_curAreaFlags |= AreaFlags::afFlag8;
	p_global->_gameFlags |= GameFlags::gfFlag200;
}

// Original name: SpcOr
void EdenGame::specialGold(perso_t *perso) {
	if (!isAnswerYes())
		return;
	perso->_items = _curSpecialObject->_itemMask;
	p_global->_roomPersoItems = _curSpecialObject->_itemMask;
	perso->_flags |= PersonFlags::pf10;
	perso->_flags &= ~PersonFlags::pfInParty;
	perso->_targetLoc = 0;
	p_global->_areaPtr->_flags |= AreaFlags::afGaveGold;
	p_global->_curAreaFlags |= AreaFlags::afGaveGold;
	if (p_global->_phaseNum == 226)
		incPhase1();
}

// Original name: SpcPrisme
void EdenGame::specialPrism(perso_t *perso) {
	if (perso == &kPersons[PER_DINA]) {
		if (p_global->_partyOutside & PersonMask::pmMonk)
			p_global->_gameFlags |= GameFlags::gfPrismAndMonk;
	}
}

// Original name: SpcTalisman
void EdenGame::specialTalisman(perso_t *perso) {
	if (perso == &kPersons[PER_DINA])
		suis_moi(1);
}

// Original name: SpcMasque
void EdenGame::specialMask(perso_t *perso) {
	if (perso == &kPersons[PER_BAMBOO]) {
		dialautoon();
		parlemoiNormalFlag = true;
	}
}

// Original name: SpcSac
void EdenGame::specialBag(perso_t *perso) {
	if (p_global->textToken1 != 3)
		return;
	if (perso == &kPersons[PER_KABUKA] || perso == &kPersons[PER_MAMMI_3])
		loseObject(_curSpecialObject->_id);
}

// Original name: SpcTrompet
void EdenGame::specialTrumpet(perso_t *perso) {
	if (!isAnswerYes())
		return;
	p_global->ff_54 = 4;
	winObject(Objects::obTrumpet);
	p_global->_drawFlags |= DrawFlags::drDrawInventory;
	_closeCharacterDialog = true;
	tyranDies(p_global->_roomCharacter);
}

// Original name: SpcArmes
void EdenGame::specialWeapons(perso_t *perso) {
	if (!isAnswerYes())
		return;
	perso->_powers = _curSpecialObject->_powerMask;
	p_global->_roomCharacterPowers = _curSpecialObject->_powerMask;
	giveObject();
}

// Original name: SpcInstru
void EdenGame::specialInstrument(perso_t *perso) {
	if (!isAnswerYes())
		return;
	if (perso == &kPersons[PER_MONK]) {
		p_global->_partyInstruments &= ~1;   //TODO: check me
		if (_curSpecialObject->_id == Objects::obRing) {
			p_global->_partyInstruments |= 1;
			p_global->_monkGotRing++;                //TODO: |= 1 ?
		}
	}
	if (perso == &kPersons[PER_GARDES]) {
		p_global->_partyInstruments &= ~2;
		if (_curSpecialObject->_id == Objects::obDrum)
			p_global->_partyInstruments |= 2;
	}
	perso->_powers = _curSpecialObject->_powerMask;
	p_global->_curCharacterPowers = _curSpecialObject->_powerMask;
	giveObject();
}

// Original name: SpcOeuf
void EdenGame::specialEgg(perso_t *perso) {
	if (!isAnswerYes())
		return;
	_gameIcons[131]._cursorId &= ~0x8000;
	p_global->_characterBackgroundBankIdx = 62;
	dialautoon();
}

// Original name: TyranMeurt
void EdenGame::tyranDies(perso_t *perso) {
	perso->_flags |= PersonFlags::pf80;
	perso->_roomNum = 0;
	delinfo(p_global->_areaNum + ValleyNews::vnTyrannIn);
	p_global->_roomCharacterType = 0;
	p_global->_roomCharacterFlags = 0;
	p_global->_chronoFlag = 0;
}

void EdenGame::SpecialObjets(perso_t *perso, char objid) {
	struct SpecialObject {
		int8  _characterType;
		int8  _objectId;
		void  (EdenGame::*dispFct)(perso_t *perso);
	};

	static SpecialObject kSpecialObjectActions[] = {
		//    persoType, objectId, dispFct
		{ PersonFlags::pfType8, Objects::obShroom, &EdenGame::specialMushroom },
		{ PersonFlags::pftTriceraptor, Objects::obNest, &EdenGame::specialNidv },
		{ PersonFlags::pfType0, Objects::obFullNest, &EdenGame::specialNido },
		{ PersonFlags::pftMosasaurus, Objects::obApple, &EdenGame::specialApple },
		{ PersonFlags::pftVelociraptor, Objects::obGold, &EdenGame::specialGold },
		{ PersonFlags::pfType0, Objects::obPrism, &EdenGame::specialPrism },
		{ PersonFlags::pfType0, Objects::obTalisman, &EdenGame::specialTalisman },
		{ PersonFlags::pfType2, Objects::obMaskOfDeath, &EdenGame::specialMask },
		{ PersonFlags::pfType2, Objects::obMaskOfBonding, &EdenGame::specialMask },
		{ PersonFlags::pfType2, Objects::obMaskOfBirth, &EdenGame::specialMask },
		{ PersonFlags::pfType0, Objects::obBag, &EdenGame::specialBag },
		{ PersonFlags::pfType2, Objects::obBag, &EdenGame::specialBag },
		{ PersonFlags::pftTyrann, Objects::obTrumpet, &EdenGame::specialTrumpet },
		{ PersonFlags::pftVelociraptor, Objects::obEyeInTheStorm, &EdenGame::specialWeapons },
		{ PersonFlags::pftVelociraptor, Objects::obSkyHammer, &EdenGame::specialWeapons },
		{ PersonFlags::pftVelociraptor, Objects::obFireInTheClouds, &EdenGame::specialWeapons },
		{ PersonFlags::pftVelociraptor, Objects::obWithinAndWithout, &EdenGame::specialWeapons },
		{ PersonFlags::pftVelociraptor, Objects::obEyeInTheCyclone, &EdenGame::specialWeapons },
		{ PersonFlags::pftVelociraptor, Objects::obRiverThatWinds, &EdenGame::specialWeapons },
		{ PersonFlags::pfType0, Objects::obTrumpet, &EdenGame::specialInstrument },
		{ PersonFlags::pfType0, Objects::obDrum, &EdenGame::specialInstrument },
		{ PersonFlags::pfType0, Objects::obRing, &EdenGame::specialInstrument },
		{ PersonFlags::pfType0, Objects::obEgg, &EdenGame::specialEgg },
		{ -1, -1, nullptr }
	};
	SpecialObject *spcobj = kSpecialObjectActions;
	char characterType = perso->_flags & PersonFlags::pfTypeMask;
	_curSpecialObject = &_objects[objid - 1];
	for (; spcobj->_characterType != -1; spcobj++) {
		if (spcobj->_objectId == objid && spcobj->_characterType == characterType) {
			(this->*spcobj->dispFct)(perso);
			break;
		}
	}
}

void EdenGame::dialautoon() {
	p_global->_iconsIndex = 4;
	p_global->_autoDialog = true;
	putObject();
}

void EdenGame::dialautooff() {
	p_global->_iconsIndex = 0x10;
	p_global->_autoDialog = false;
}

void EdenGame::follow() {
	if (p_global->_roomCharacterType == PersonFlags::pfType12) {
		debug("follow: hiding person %ld", p_global->_roomCharacter - kPersons);
		p_global->_roomCharacter->_flags |= PersonFlags::pf80;
		p_global->_roomCharacter->_roomNum = 0;
		p_global->_gameFlags |= GameFlags::gfFlag8;
		_gameIcons[123]._objectId = 18;
		_gameIcons[124]._objectId = 35;
		_gameIcons[125]._cursorId &= ~0x8000;
		p_global->_characterBackgroundBankIdx = 56;
	} else
		suis_moi5();
}

void EdenGame::dialonfollow() {
	p_global->_iconsIndex = 4;
	p_global->_autoDialog = true;
	follow();
}

void EdenGame::abortdial() {
	p_global->ff_F6++;
	if (p_global->_roomCharacterType != PersonFlags::pftTriceraptor || p_global->_characterPtr != &kPersons[PER_EVE])
		return;
	p_global->_areaPtr->_flags |= AreaFlags::afFlag4;
	p_global->_curAreaFlags |= AreaFlags::afFlag4;
	p_global->_roomCharacter->_flags |= PersonFlags::pfInParty;
	p_global->_roomCharacterFlags |= PersonFlags::pfInParty;
	lieuvava(p_global->_areaPtr);
}

void EdenGame::narrateur() {
	if (!(p_global->_displayFlags & DisplayFlags::dfFlag1))
		return;
	if (!p_global->narratorSequence) {
		if (p_global->ff_6A == p_global->ff_69)
			goto skip;
		buildcita();
	}
	p_global->ff_F5 |= 0x80;
	p_global->ff_F2 &= ~1;  //TODO: check me
	p_global->_characterPtr = &kPersons[PER_UNKN_156];
	p_global->ff_60 = false;
	p_global->_eventType = 0;
	p_global->ff_103 = 69;
	if (dialo_even(&kPersons[PER_UNKN_156])) {
		p_global->narrator_dialog_ptr = p_global->_dialogPtr;
		dialautoon();
		p_global->ff_F2 |= 1;
		waitEndSpeak();
		if (pomme_q)
			return;
		endpersovox();
		while (dialoscansvmas(p_global->narrator_dialog_ptr)) {
			p_global->narrator_dialog_ptr = p_global->_dialogPtr;
			waitEndSpeak();
			if (pomme_q)
				return;
			endpersovox();
		}
		p_global->narrator_dialog_ptr = p_global->_dialogPtr;
		p_global->ff_102 = 0;
		p_global->ff_103 = 0;
		close_perso();
		lieuvava(p_global->_areaPtr);
		if (p_global->narratorSequence == 8)
			deplaval(134);
	}
	p_global->ff_103 = 0;
	if (p_global->narratorSequence == 10) {
		suis_moi(5);
		suis_moi(7);
		suis_moi(3);
		suis_moi(18);
		reste_ici(6);
		p_global->eloiHaveNews = 0;
		deplaval(139);
	}
	p_global->_eventType = EventType::etEventD;
	showEvents();
	p_global->ff_F5 &= ~0x80;
skip:
	;
	p_global->ff_F2 &= ~1;  //TODO: check me
	if (p_global->narratorSequence > 50 && p_global->narratorSequence <= 80)
		p_global->endGameFlag = 50;
	if (p_global->narratorSequence == 3)
		chronoon(1200);
	p_global->narratorSequence = 0;

}

void EdenGame::vrf_phrases_file() {
	int16 num = 3;
	if (p_global->_dialogPtr < (dial_t *)getElem(gameDialogs, 48))
		num = 1;
	else if (p_global->_dialogPtr < (dial_t *)getElem(gameDialogs, 128))
		num = 2;
	p_global->textBankIndex = num;
	if (p_global->pref_language)
		num += (p_global->pref_language - 1) * 3;
	if (num == lastPhrasesFile)
		return;
	lastPhrasesFile = num;
	num += 404;
	loadFile(num, gamePhrases);
	verifh(gamePhrases);
}

byte *EdenGame::gettxtad(int16 id) {
	vrf_phrases_file();
	return (byte *)getElem(gamePhrases, id - 1);
}

void EdenGame::gotocarte() {
	goto_t *go = &gotos[current_spot2->_objectId];
	endpersovox();
	byte newArea = go->areaNum;
	p_global->newRoomNum = (go->areaNum << 8) | 1;
	p_global->newLocation = 1;
	p_global->prevLocation = p_global->roomNum & 0xFF;
	char curArea = p_global->roomNum >> 8;
	if (curArea == go->areaNum)
		newArea = 0;
	else {
		for (; go->curAreaNum != 0xFF; go++) {
			if (go->curAreaNum == curArea)
				break;
		}

		if (go->areaNum == 0xFF)
			return;
	}
	p_global->_eventType = EventType::etGotoArea | newArea;
	setChoiceYes();
	showevents1();
	waitEndSpeak();
	if (pomme_q)
		return;

	close_perso();
	if (isAnswerYes())
		gotolieu(go);
}

void EdenGame::record() {
	if (p_global->_curObjectId)
		return;

	if (p_global->_characterPtr >= &kPersons[PER_UNKN_18C])
		return;

	if (p_global->_eventType == EventType::etEventE || p_global->_eventType >= EventType::etGotoArea)
		return;

	for (tape_t *tape = tapes; tape != tapes + MAX_TAPES; tape++) {
		if (tape->_textNum == p_global->textNum)
			return;
	}

	tape_t *tape = tapes;
	for (int16 i = 0; i < MAX_TAPES - 1; i++) {
		tape->_textNum = tape[+1]._textNum;
		tape->_perso = tape[+1]._perso;
		tape->_party = tape[+1]._party;
		tape->_roomNum = tape[+1]._roomNum;
		tape->_bgBankNum = tape[+1]._bgBankNum;
		tape->_dialog = tape[+1]._dialog;
		tape++;
	}

	perso_t *perso = p_global->_characterPtr;
	if (perso == &kPersons[PER_EVE])
		perso = p_global->_phaseNum >= 352 ? &kPersons[PER_UNKN_372]
		        : &kPersons[PER_UNKN_402];
	tape->_textNum = p_global->textNum;
	tape->_perso = perso;
	tape->_party = p_global->party;
	tape->_roomNum = p_global->roomNum;
	tape->_bgBankNum = p_global->roomBgBankNum;
	tape->_dialog = p_global->_dialogPtr;
}

bool EdenGame::dial_scan(dial_t *dial) {
	if (p_global->numGiveObjs) {
		if (!(p_global->_displayFlags & DisplayFlags::dfFlag2))
			showObjects();
		p_global->numGiveObjs = 0;
	}
	p_global->_dialogPtr = dial;
	vavapers();
	p_global->phraseBufferPtr = phraseBuffer;
	byte hidx, lidx;
	uint16 mask;
	for (;; p_global->_dialogPtr++) {
		for (;; p_global->_dialogPtr++) {
			if (p_global->_dialogPtr->_flags == -1 && p_global->_dialogPtr->_condNumLow == -1)
				return false;
			byte flags = p_global->_dialogPtr->_flags;
			p_global->dialogFlags = flags;
			if (!(flags & DialogFlags::dfSpoken) || (flags & DialogFlags::dfRepeatable)) {
				hidx = (p_global->_dialogPtr->_textCondHiMask >> 6) & 3;
				lidx = p_global->_dialogPtr->_condNumLow;
				if (flags & 0x10)
					hidx |= 4;
				if (testcondition(((hidx << 8) | lidx) & 0x7FF))
					break;
			} else {
				if (flags & dialogSkipFlags)
					continue;
				hidx = (p_global->_dialogPtr->_textCondHiMask >> 6) & 3;
				lidx = p_global->_dialogPtr->_condNumLow;
				if (flags & 0x10)
					hidx |= 4;
				if (testcondition(((hidx << 8) | lidx) & 0x7FF))
					break;
			}
		}
		char bidx = (p_global->_dialogPtr->_textCondHiMask >> 2) & 0xF;
		if (!bidx)
			goto no_perso;  //TODO: rearrange
		mask = (p_global->party | p_global->_partyOutside) & (1 << (bidx - 1));
		if (mask)
			break;
	}
	perso_t *perso;
	for (perso = kPersons; !(perso->_partyMask == mask && perso->_roomNum == p_global->roomNum); perso++)
		; //Find matching

	p_global->_characterPtr = perso;
	init_perso_ptr(perso);
	no_perso();
no_perso:
	;
	hidx = p_global->_dialogPtr->_textCondHiMask;
	lidx = p_global->_dialogPtr->_textNumLow;
	p_global->textNum = ((hidx << 8) | lidx) & 0x3FF;
	if (p_global->phraseBufferPtr != phraseBuffer) {
		for (int16 i = 0; i < 32; i++)
			SysBeep(1);
	} else
		my_bulle();
	if (!dword_30B04) {
		static void (EdenGame::*talk_subject[])() = {
			&EdenGame::setChoiceYes,
			&EdenGame::setChoiceNo,
			&EdenGame::eloipart,
			&EdenGame::dialautoon,
			&EdenGame::dialautooff,
			&EdenGame::stay_here,
			&EdenGame::follow,
			&EdenGame::citadelle,
			&EdenGame::dialonfollow,
			&EdenGame::abortdial,
			&EdenGame::incphase,
			&EdenGame::bigphase,
			&EdenGame::giveObject,
			&EdenGame::choixzone,
			&EdenGame::lostobject
		};
		char pnum = p_global->_dialogPtr->_flags & 0xF;
		if (pnum)
			(this->*talk_subject[pnum - 1])();
		p_global->ff_60 = true;
		p_global->_dialogPtr->_flags |= DialogFlags::dfSpoken;
		p_global->_dialogPtr++;
	}
	if (p_global->_dialogType != DialogType::dtInspect) {
		record();
		getdatasync();
		showCharacter();
		persovox();
	}
	return true;
}

bool EdenGame::dialoscansvmas(dial_t *dial) {
	byte oldFlag = dialogSkipFlags;
	dialogSkipFlags = DialogFlags::df20;
	bool res = dial_scan(dial);
	dialogSkipFlags = oldFlag;
	return res;
}

bool EdenGame::dialo_even(perso_t *perso) {
	p_global->_characterPtr = perso;
	int num = (perso->_id << 3) | DialogType::dtEvent;
	dial_t *dial = (dial_t *)getElem(gameDialogs, num);
	bool res = dialoscansvmas(dial);
	p_global->last_dialog_ptr = nullptr;
	parlemoiNormalFlag = false;
	return res;
}

void EdenGame::stay_here() {
	if (p_global->_characterPtr == &kPersons[PER_DINA] && p_global->roomNum == 260)
		p_global->_gameFlags |= GameFlags::gfFlag1000;
	reste_ici5();
}

void EdenGame::mort(int16 vid) {
	bars_out();
	playHNM(vid);
	fadetoblack(2);
	CLBlitter_FillScreenView(0);
	CLBlitter_FillView(p_mainview, 0);
	showBars();
	p_global->narratorSequence = 51;
	p_global->newMusicType = MusicType::mtNormal;
	musique();
	musicspy();
}

void EdenGame::evenchrono() {
	if (!(p_global->_displayFlags & DisplayFlags::dfFlag1))
		return;

	uint16 oldGameTime = p_global->gameTime;
	currentTime = _vm->_timerTicks / 100;
	p_global->gameTime = currentTime;
	if (p_global->gameTime <= oldGameTime)
		return;
	heurepasse();
	if (!(p_global->_chronoFlag & 1))
		return;
	p_global->chrono -= 200;
	if (p_global->chrono == 0)
		p_global->_chronoFlag |= 2;
	if (!(p_global->_chronoFlag & 2))
		return;
	p_global->_chronoFlag = 0;
	p_global->chrono = 0;
	if (p_global->_roomCharacterType == PersonFlags::pftTyrann) {
		int16 vid = 272;
		if (p_global->curRoomFlags & 0xC0) {
			vid += 2;
			if ((p_global->curRoomFlags & 0xC0) != 0x80) {
				vid += 2;
				mort(vid);
				return;
			}
		}
		if (p_global->_areaNum == Areas::arUluru || p_global->_areaNum == Areas::arTamara) {
			mort(vid);
			return;
		}
		vid++;
		mort(vid);
		return;
	}
	if (p_global->roomNum == 2817) {
		suis_moi(5);
		p_global->_gameFlags |= GameFlags::gfFlag40;
		dialautoon();
	} else
		eloirevient();
	p_global->_eventType = EventType::etEvent10;
	showEvents();
}

void EdenGame::chronoon(int16 t) {
	p_global->chrono = t;
	p_global->_chronoFlag = 1;
}

void EdenGame::prechargephrases(int16 vid) {
	perso_t *perso = &kPersons[PER_MORKUS];
	if (vid == 170)
		perso = &kPersons[PER_UNKN_156];
	p_global->_characterPtr = perso;
	p_global->_dialogType = DialogType::dtInspect;
	int num = (perso->_id << 3) | p_global->_dialogType;
	dial_t *dial = (dial_t *)getElem(gameDialogs, num);
	dialoscansvmas(dial);
}

void EdenGame::effet1() {
	rectanglenoir32();
	if (!_doubledScreen) {
		setRS1(0, 0, 16 - 1, 4 - 1);
		int y = p_mainview->_normal._dstTop;
		for (int16 i = 16; i <= 96; i += 4) {
			for (int x = p_mainview->_normal._dstLeft; x < p_mainview->_normal._dstLeft + 320; x += 16) {
				setRD1(x, y + i, x + 16 - 1, y + i + 4 - 1);
				CLBlitter_CopyViewRect(p_view2, _vm->ScreenView, &rect_src, &rect_dst);
				setRD1(x, y + 192 - i, x + 16 - 1, y + 192 - i + 4 - 1);
				CLBlitter_CopyViewRect(p_view2, _vm->ScreenView, &rect_src, &rect_dst);
			}
			CLBlitter_UpdateScreen();
			wait(1);
		}
	} else {
		setRS1(0, 0, 16 * 2 - 1, 4 * 2 - 1);
		int y = p_mainview->_zoom._dstTop;
		for (int16 i = 16 * 2; i <= 96 * 2; i += 4 * 2) {
			for (int x = p_mainview->_zoom._dstLeft; x < p_mainview->_zoom._dstLeft + 320 * 2; x += 16 * 2) {
				setRD1(x, y + i, x + 16 * 2 - 1, y + i + 4 * 2 - 1);
				CLBlitter_CopyViewRect(p_view2, _vm->ScreenView, &rect_src, &rect_dst);
				setRD1(x, y + 192 * 2 - i, x + 16 * 2 - 1, y + 192 * 2 - i + 4 * 2 - 1);
				CLBlitter_CopyViewRect(p_view2, _vm->ScreenView, &rect_src, &rect_dst);
			}
			wait(1);
		}
	}
	CLPalette_Send2Screen(global_palette, 0, 256);
	p_mainview->_normal._height = 2;
	p_mainview->_zoom._height = 4;
	int16 ny = p_mainview->_normal._dstTop;
	int16 dy = p_mainview->_zoom._dstTop;
	for (int16 i = 0; i < 100; i += 2) {
		p_mainview->_normal._srcTop = 99 - i;
		p_mainview->_zoom._srcTop = 99 - i;
		p_mainview->_normal._dstTop = 99 - i + ny;
		p_mainview->_zoom._dstTop = (99 - i) * 2 + dy;
		CLBlitter_CopyView2Screen(p_mainview);
		p_mainview->_normal._srcTop = 100 + i;
		p_mainview->_zoom._srcTop = 100 + i;
		p_mainview->_normal._dstTop = 100 + i + ny;
		p_mainview->_zoom._dstTop = (100 + i) * 2 + dy;
		CLBlitter_CopyView2Screen(p_mainview);
		CLBlitter_UpdateScreen();
		wait(1);
	}
	p_mainview->_normal._height = 200;
	p_mainview->_zoom._height = 400;
	p_mainview->_normal._srcTop = 0;
	p_mainview->_zoom._srcTop = 0;
	p_mainview->_normal._dstTop = ny;
	p_mainview->_zoom._dstTop = dy;
	p_global->ff_F1 = 0;
}

void EdenGame::effet2() {
	static int16 pattern1[] = {0, 1, 2, 3, 7, 11, 15, 14, 13, 12, 8, 4, 5, 6, 10, 9};
	static int16 pattern2[] = {0, 15, 1, 14, 2, 13, 3, 12, 7, 8, 11, 4, 5, 10, 6, 9};
	static int16 pattern3[] = {0, 2, 5, 7, 8, 10, 13, 15, 1, 3, 4, 6, 9, 11, 12, 14};
	static int16 pattern4[] = {0, 3, 15, 12, 1, 7, 14, 8, 2, 11, 13, 4, 5, 6, 10, 9};

	static int eff2pat = 0;
	if (p_global->ff_103 == 69) {
		effet4();
		return;
	}
	switch (++eff2pat) {
	case 1:
		colimacon(pattern1);
		break;
	case 2:
		colimacon(pattern2);
		break;
	case 3:
		colimacon(pattern3);
		break;
	case 4:
		colimacon(pattern4);
		eff2pat = 0;
		break;
	}
}

void EdenGame::effet3() {
	CLPalette_GetLastPalette(oldPalette);
	for (uint16 i = 0; i < 6; i++) {
		for (uint16 c = 0; c < 256; c++) {
			newColor.r = oldPalette[c].r >> i;
			newColor.g = oldPalette[c].g >> i;
			newColor.b = oldPalette[c].b >> i;
			CLPalette_SetRGBColor(newPalette, c, &newColor);
		}
		CLPalette_Send2Screen(newPalette, 0, 256);
		wait(1);
	}
	CLBlitter_CopyView2Screen(p_mainview);
	for (uint16 i = 0; i < 6; i++) {
		for (uint16 c = 0; c < 256; c++) {
			newColor.r = global_palette[c].r >> (5 - i);
			newColor.g = global_palette[c].g >> (5 - i);
			newColor.b = global_palette[c].b >> (5 - i);
			CLPalette_SetRGBColor(newPalette, c, &newColor);
		}
		CLPalette_Send2Screen(newPalette, 0, 256);
		wait(1);
	}
}

void EdenGame::effet4() {
	byte *scr, *pix, *r24, *r25, *r30, c;
	int16 x, y;
	int16 r17, r23, r16, r18, r19, r22, r27, r31;
	CLPalette_Send2Screen(global_palette, 0, 256);

//	Unused
//	int16 w = _vm->ScreenView->_width;
//	int16 h = _vm->ScreenView->_height;
	int16 ww = _vm->ScreenView->_pitch;
	if (!_doubledScreen) {
		x = p_mainview->_normal._dstLeft;
		y = p_mainview->_normal._dstTop;
		for (int16 i = 32; i > 0; i -= 2) {
			scr = _vm->ScreenView->_bufferPtr;
			scr += (y + 16) * ww + x;
			pix = p_mainview->_bufferPtr + 16 * 640;
			r17 = 320 / i;
			r23 = 320 - 320 / i * i;  //TODO: 320 % i ?
			r16 = 160 / i;
			r18 = 160 - 160 / i * i;  //TODO: 160 % i ?
			for (r19 = r16; r19 > 0; r19--) {
				r24 = scr;
				r25 = pix;
				for (r22 = r17; r22 > 0; r22--) {
					c = *r25;
					r25 += i;
					r30 = r24;
					for (r27 = i; r27 > 0; r27--) {
						for (r31 = i; r31 > 0; r31--)
							*r30++ = c;
						r30 += ww - i;
					}
					r24 += i;
				}
				if (r23) {
					c = *r25;
					r30 = r24;
					for (r27 = i; r27 > 0; r27--) {
						for (r31 = r23; r31 > 0; r31--)
							*r30++ = c;
						r30 += ww - r23;
					}
				}
				scr += i * ww;
				pix += i * 640;
			}
			if (r18) {
				r24 = scr;
				r25 = pix;
				for (r22 = r17; r22 > 0; r22--) {
					c = *r25;
					r25 += i;
					r30 = r24;
					for (r27 = r18; r27 > 0; r27--) {
						for (r31 = i; r31 > 0; r31--)
							*r30++ = c;
						r30 += ww - i;
					}
					r24 += i;
				}
				if (r23) {
					c = *r25;
					r30 = r24;
					for (r27 = r18; r27 > 0; r27--) {
						for (r31 = r23; r31 > 0; r31--)
							*r30++ = c;
						r30 += ww - r23;
					}
				}
			}
			CLBlitter_UpdateScreen();
			wait(3);
		}
	} else {
		x = p_mainview->_zoom._dstLeft;
		y = p_mainview->_zoom._dstTop;
		for (int16 i = 32; i > 0; i -= 2) {
			scr = _vm->ScreenView->_bufferPtr;
			scr += (y + 16 * 2) * ww + x;
			pix = p_mainview->_bufferPtr + 16 * 640;
			r17 = 320 / i;
			r23 = 320 % i;
			r16 = 160 / i;
			r18 = 160 % i;
			for (r19 = r16; r19 > 0; r19--) {
				r24 = scr;
				r25 = pix;
				for (r22 = r17; r22 > 0; r22--) {
					c = *r25;
					r25 += i;
					r30 = r24;
					for (r27 = i * 2; r27 > 0; r27--) {
						for (r31 = i * 2; r31 > 0; r31--)
							*r30++ = c;
						r30 += ww - i * 2;
					}
					r24 += i * 2;
				}
				if (r23) {
					c = *r25;
					r30 = r24;
					for (r27 = i * 2; r27 > 0; r27--) {
						for (r31 = r23 * 2; r31 > 0; r31--)
							*r30++ = c;
						r30 += ww - r23 * 2;
					}
				}
				scr += i * ww * 2;
				pix += i * 640;
			}
			if (r18) {
				r24 = scr;
				r25 = pix;
				for (r22 = r17; r22 > 0; r22--) {
					c = *r25;
					r25 += i;
					r30 = r24;
					for (r27 = r18 * 2; r27 > 0; r27--) {
						for (r31 = i * 2; r31 > 0; r31--)
							*r30++ = c;
						r30 += ww - i * 2;
					}
					r24 += i * 2;
				}
				if (r23) {
					c = *r25;
					r30 = r24;
					for (r27 = i * 2; r27 > 0; r27--) {
						for (r31 = r23 * 2; r31 > 0; r31--)
							*r30++ = c;
						r30 += ww - r23 * 2;
					}
				}
			}
			wait(3);
		}
	}
	CLBlitter_CopyView2Screen(p_mainview);
}

void EdenGame::ClearScreen() {
	byte *scr;
	int16 x, y;

//	Unused
//	int16 w = _vm->ScreenView->_width;
//	int16 h = _vm->ScreenView->_height;
	int16 ww = _vm->ScreenView->_pitch;
	if (!_doubledScreen) {
		x = p_mainview->_normal._dstLeft;
		y = p_mainview->_normal._dstTop;
		scr = _vm->ScreenView->_bufferPtr;
		scr += (y + 16) * ww + x;
		for (int16 yy = 0; yy < 160; yy++) {
			for (int16 xx = 0; xx < 320; xx++)
				*scr++ = 0;
			scr += ww - 320;
		}
	} else {
		x = p_mainview->_zoom._dstLeft;
		y = p_mainview->_zoom._dstTop;
		scr = _vm->ScreenView->_bufferPtr;
		scr += (y + 32) * ww + x;
		for (int16 yy = 0; yy < 160; yy++) {
			for (int16 xx = 0; xx < 320; xx++) {
				scr[0] = 0;
				scr[1] = 0;
				scr[ww] = 0;
				scr[ww + 1] = 0;
				scr += 2;
			}
			scr += (ww - 320) * 2;
		}
	}
	CLBlitter_UpdateScreen();
}

void EdenGame::colimacon(int16 pattern[16]) {
	byte *scr, *pix;
	int16 x, y;
	int16 p, r27, r25;

//	Unused
//	int16 w = _vm->ScreenView->_width;
//	int16 h = _vm->ScreenView->_height;
	int16 ww = _vm->ScreenView->_pitch;
	if (!_doubledScreen) {
		x = p_mainview->_normal._dstLeft;
		y = p_mainview->_normal._dstTop;
		scr = _vm->ScreenView->_bufferPtr;
		scr += (y + 16) * ww + x;
		for (int16 i = 0; i < 16; i++) {
			p = pattern[i];
			r27 = p % 4 + p / 4 * ww;
			for (int16 j = 0; j < 320 * 160 / 16; j++)
				scr[j / (320 / 4) * ww * 4 + j % (320 / 4) * 4 + r27] = 0;
			CLBlitter_UpdateScreen();
			wait(1);
		}
	} else {
		x = p_mainview->_zoom._dstLeft;
		y = p_mainview->_zoom._dstTop;
		scr = _vm->ScreenView->_bufferPtr;
		scr += (y + 16 * 2) * ww + x;
		for (int16 i = 0; i < 16; i++) {
			p = pattern[i];
			r27 = p % 4 * 2 + p / 4 * ww * 2;
			for (int16 j = 0; j < 320 * 160 / 16; j++) {
				byte *sc = &scr[j / (320 / 4) * ww * 4 * 2 + j % (320 / 4) * 4 * 2 + r27];
				sc[0] = 0;
				sc[1] = 0;
				sc[ww] = 0;
				sc[ww + 1] = 0;
			}
			wait(1);
		}
	}
	CLPalette_Send2Screen(global_palette, 0, 256);
	if (!_doubledScreen) {
		pix = p_mainview->_bufferPtr;
		x = p_mainview->_normal._dstLeft;
		y = p_mainview->_normal._dstTop;
		pix += 640 * 16;
		scr = _vm->ScreenView->_bufferPtr;
		scr += (y + 16) * ww + x;
		for (int16 i = 0; i < 16; i++) {
			p = pattern[i];
			r25 = p % 4 + p / 4 * 640;
			r27 = p % 4 + p / 4 * ww;
			for (int16 j = 0; j < 320 * 160 / 16; j++)
				scr[j / (320 / 4) * ww * 4 + j % (320 / 4) * 4 + r27] =
				    pix[j / (320 / 4) * 640 * 4 + j % (320 / 4) * 4 + r25];
			CLBlitter_UpdateScreen();
			wait(1);
		}
	} else {
		pix = p_mainview->_bufferPtr;
		x = p_mainview->_zoom._dstLeft;
		y = p_mainview->_zoom._dstTop;
		pix += 640 * 16;
		scr = _vm->ScreenView->_bufferPtr;
		scr += (y + 16 * 2) * ww + x;
		for (int16 i = 0; i < 16; i++) {
			p = pattern[i];
			r25 = p % 4 + p / 4 * 640;
			r27 = p % 4 * 2 + p / 4 * ww * 2;
			for (int16 j = 0; j < 320 * 160 / 16; j++) {
				byte c = pix[j / (320 / 4) * 640 * 4 + j % (320 / 4) * 4 + r25];
				byte *sc = &scr[j / (320 / 4) * ww * 4 * 2 + j % (320 / 4) * 4 * 2 + r27];
				sc[0] = c;
				sc[1] = c;
				sc[ww] = c;
				sc[ww + 1] = c;
			}
			wait(1);
		}
	}
}

void EdenGame::fadetoblack(int delay) {
	CLPalette_GetLastPalette(oldPalette);
	for (int16 i = 0; i < 6; i++) {
		for (int16 j = 0; j < 256; j++) {
			newColor.r = oldPalette[j].r >> i;
			newColor.g = oldPalette[j].g >> i;
			newColor.b = oldPalette[j].b >> i;
			CLPalette_SetRGBColor(newPalette, j, &newColor);
		}
		CLPalette_Send2Screen(newPalette, 0, 256);
		wait(delay);
	}
}

void EdenGame::fadetoblack128(int delay) {
	CLPalette_GetLastPalette(oldPalette);
	for (int16 i = 0; i < 6; i++) {
		for (int16 j = 0; j < 129; j++) { //TODO: wha?
			newColor.r = oldPalette[j].r >> i;
			newColor.g = oldPalette[j].g >> i;
			newColor.b = oldPalette[j].b >> i;
			CLPalette_SetRGBColor(newPalette, j, &newColor);
		}
		CLPalette_Send2Screen(newPalette, 0, 128);
		wait(delay);
	}
}

void EdenGame::fadefromblack128(int delay) {
	for (int16 i = 0; i < 6; i++) {
		for (int16 j = 0; j < 129; j++) { //TODO: wha?
			newColor.r = global_palette[j].r >> (5 - i);
			newColor.g = global_palette[j].g >> (5 - i);
			newColor.b = global_palette[j].b >> (5 - i);
			CLPalette_SetRGBColor(newPalette, j, &newColor);
		}
		CLPalette_Send2Screen(newPalette, 0, 128);
		wait(delay);
	}
}

void EdenGame::rectanglenoir32() {
	// blacken 32x32 rectangle
	int *pix = (int *)p_view2_buf;
	for (int16 i = 0; i < 32; i++) {
		pix[0] = 0;
		pix[1] = 0;
		pix[2] = 0;
		pix[3] = 0;
		pix[4] = 0;
		pix[5] = 0;
		pix[6] = 0;
		pix[7] = 0;
		pix += 32 / 4;
	}
}

void EdenGame::setRS1(int16 sx, int16 sy, int16 ex, int16 ey) {
	rect_src.left = sx;
	rect_src.top = sy;
	rect_src.right = ex;
	rect_src.bottom = ey;
}

void EdenGame::setRD1(int16 sx, int16 sy, int16 ex, int16 ey) {
	rect_dst.left = sx;
	rect_dst.top = sy;
	rect_dst.right = ex;
	rect_dst.bottom = ey;
}

void EdenGame::wait(int howlong) {
	int t = TickCount();
#ifdef EDEN_DEBUG
	howlong *= 10;
#endif
	for (int t2 = t; t2 - t < howlong; t2 = TickCount())
		g_system->delayMillis(10); // waste time
}

void EdenGame::effetpix() {
	byte *scr;
	uint16 r25, r18, r31, r30;  //TODO: change to xx/yy

	uint16 ww = _vm->ScreenView->_pitch;
	r25 = ww * 80;
	r18 = 640 * 80;
	byte *pix = p_mainview->_bufferPtr + 16 * 640;
	if (!_doubledScreen) {
		int x = p_mainview->_normal._dstLeft;
		int y = p_mainview->_normal._dstTop;
		scr = _vm->ScreenView->_bufferPtr;
		scr += (y + 16) * ww + x;
	} else {
		int x = p_mainview->_zoom._dstLeft;
		int y = p_mainview->_zoom._dstTop;
		scr = _vm->ScreenView->_bufferPtr;
		scr += (y + 16 * 2) * ww + x;
		r25 *= 2;
	}
	int16 r20 = 0x4400;   //TODO
	int16 r27 = 1;
	int16 r26 = 0;
	do {
		char r8 = r27 & 1;
		r27 >>= 1;
		if (r8)
			r27 ^= r20;
		if (r27 < 320 * 80) {
			r31 = r27 / 320;
			r30 = r27 % 320;
			if (_doubledScreen) {
				r31 *= 2;
				r30 *= 2;
				scr[r31 * ww + r30] = 0;
				scr[r31 * ww + r30 + 1] = 0;
				scr[r31 * ww + r25 + r30] = 0;
				scr[r31 * ww + r25 + r30 + 1] = 0;
				r31++;
				scr[r31 * ww + r30] = 0;
				scr[r31 * ww + r30 + 1] = 0;
				scr[r31 * ww + r25 + r30] = 0;
				scr[r31 * ww + r25 + r30 + 1] = 0;
				if (++r26 == 960) {
					wait(1);
					r26 = 0;
				}
			} else {
				scr[r31 * ww + r30] = 0;
				scr[r31 * ww + r25 + r30] = 0;
				if (++r26 == 960) {
					CLBlitter_UpdateScreen();
					wait(1);
					r26 = 0;
				}
			}
		}
	} while (r27 != 1);
	CLPalette_Send2Screen(global_palette, 0, 256);
	r20 = 0x4400;
	r27 = 1;
	r26 = 0;
	do {
		char r8 = r27 & 1;
		r27 >>= 1;
		if (r8)
			r27 ^= r20;
		if (r27 < 320 * 80) {
			r31 = r27 / 320;
			r30 = r27 % 320;
			byte p0 = pix[r31 * 640 + r30];
			byte p1 = pix[r31 * 640 + r18 + r30];
			if (_doubledScreen) {
				r31 *= 2;
				r30 *= 2;
				scr[r31 * ww + r30] = p0;
				scr[r31 * ww + r30 + 1] = p0;
				scr[r31 * ww + r25 + r30] = p1;
				scr[r31 * ww + r25 + r30 + 1] = p1;
				r31++;
				scr[r31 * ww + r30] = p0;
				scr[r31 * ww + r30 + 1] = p0;
				scr[r31 * ww + r25 + r30] = p1;
				scr[r31 * ww + r25 + r30 + 1] = p1;
				if (++r26 == 960) {
					wait(1);
					r26 = 0;
				}
			} else {
				scr[r31 * ww + r30] = p0;
				scr[r31 * ww + r25 + r30] = p1;
				if (++r26 == 960) {
					CLBlitter_UpdateScreen();
					wait(1);
					r26 = 0;
				}
			}
		}
	} while (r27 != 1);
	assert(_vm->ScreenView->_pitch == 320);
}

////// datfile.c
void EdenGame::verifh(void *ptr) {
	byte sum = 0;
	byte *head = (byte *)ptr;

	for (int8 i = 0; i < 6; i++)
		sum += *head++;

	if (sum != 0xAB)
		return;

	debug("* Begin unpacking resource");
	head -= 6;
	uint16 h0 = READ_LE_UINT16(head);
	head += 2;
	/*char unused = * */head++;
	uint16 h3 = READ_LE_UINT16(head);
	head += 2;
	byte *data = h0 + head + 26;
	h3 -= 6;
	head += h3;
	for (; h3; h3--)
		*data-- = *head--;
	head = data + 1;
	data = (byte *)ptr;
	Expand_hsq(head, data);
}

void EdenGame::openbigfile() {
	assert(sizeof(pakfile_t) == 25);
	int32 size = 0x10000;
	h_bigfile.open("EDEN.DAT");
	h_bigfile.read(bigfile_header, size);

	_hnmContext = _vm->_video->resetInternals();
	_vm->_video->setFile(_hnmContext, &h_bigfile);
}

void EdenGame::closebigfile() {
	h_bigfile.close();
}

void EdenGame::loadFile(uint16 num, void *buffer) {
	if (_vm->getPlatform() == Common::kPlatformDOS) {
		if ((_vm->isDemo() && num > 2204) || num > 2472)
			error("Trying to read invalid game resource");
	}

	assert(num < bigfile_header->count);
	pakfile_t *file = &bigfile_header->files[num];
	int32 size = READ_LE_UINT32(&file->size);
	int32 offs = READ_LE_UINT32(&file->offs);
	debug("* Loading resource %d (%s) at 0x%X, %d bytes", num, file->name, offs, size);
	h_bigfile.seek(offs, SEEK_SET);
	h_bigfile.read(buffer, size);
}

void EdenGame::shnmfl(uint16 num) {
	unsigned int resNum = num - 1 + 485;
	assert(resNum < bigfile_header->count);
	pakfile_t *file = &bigfile_header->files[resNum];
	int size = READ_LE_UINT32(&file->size);
	int offs = READ_LE_UINT32(&file->offs);
	debug("* Loading movie %d (%s) at 0x%X, %d bytes", num, file->name, (uint)offs, size);
	_hnmContext->_file->seek(offs, SEEK_SET);
}

int EdenGame::ssndfl(uint16 num) {
	unsigned int resNum = num - 1 + ((_vm->getPlatform() == Common::kPlatformDOS && _vm->isDemo()) ? 656 : 661);
	assert(resNum < bigfile_header->count);
	pakfile_t *file = &bigfile_header->files[resNum];
	int32 size = READ_LE_UINT32(&file->size);
	int32 offs = READ_LE_UINT32(&file->offs);
	debug("* Loading sound %d (%s) at 0x%X, %d bytes", num, file->name, (uint)offs, size);
	if (_soundAllocated) {
		free(voiceSamplesBuffer);
		voiceSamplesBuffer = nullptr;
		_soundAllocated = false; //TODO: bug??? no alloc
	} else {
		voiceSamplesBuffer = malloc(size);
		_soundAllocated = true;
	}

	h_bigfile.seek(offs, SEEK_SET);
	//For PC loaded data is a VOC file, on Mac version this is a raw samples
	if (_vm->getPlatform() == Common::kPlatformMacintosh)
		h_bigfile.read(voiceSamplesBuffer, size);
	else {
		// VOC files also include extra information for lipsync
		// 1. Standard VOC header
		h_bigfile.read(voiceSamplesBuffer, 0x1A);

		// 2. Lipsync?
		unsigned char chunkType = h_bigfile.readByte();

		uint32 val;
		h_bigfile.read(&val, 3);
		unsigned int chunkLen = LE32(val);

		if (chunkType == 5) {
			h_bigfile.read(gameLipsync + 7260, chunkLen);
//			anim_buffer_ptr = gameLipsync + 7260 + 2;

			chunkType = h_bigfile.readByte();
			h_bigfile.read(&val, 3);
			chunkLen = LE32(val);
		}

		// 3. Normal sound data
		if (chunkType == 1) {
			/*unsigned short freq = */h_bigfile.readUint16LE();
			size = chunkLen - 2;
			h_bigfile.read(voiceSamplesBuffer, size);
		}
	}

	return size;
}

void EdenGame::ConvertIcons(icon_t *icon, int count) {
	for (int i = 0; i < count; i++, icon++) {
		icon->sx = BE16(icon->sx);
		icon->sy = BE16(icon->sy);
		icon->ex = BE16(icon->ex);
		icon->ey = BE16(icon->ey);
		icon->_cursorId = BE16(icon->_cursorId);
		icon->_objectId = BE32(icon->_objectId);
		icon->_actionId = BE32(icon->_actionId);
	}
}

void EdenGame::ConvertLinks(room_t *room, int count) {
	for (int i = 0; i < count; i++, room++) {
		room->bank = BE16(room->bank);
		room->party = BE16(room->party);
	}
}

void EdenGame::ConvertMacToPC() {
	// Convert all mac (big-endian) resources to native format
	ConvertIcons(_gameIcons, 136);
	ConvertLinks(gameRooms, 424);
	// Array of longs
	int *p = (int *)gameLipsync;
	for (int i = 0; i < 7240 / 4; i++)
		p[i] = BE32(p[i]);
}

void EdenGame::loadpermfiles() {
	switch (_vm->getPlatform()) {
	case Common::kPlatformDOS:
	{
		// Since PC version stores hotspots and rooms info in the executable, load them from premade resource file
		Common::File f;

		if (f.open("led.dat")) {
			const int kNumIcons = 136;
			const int kNumRooms = 424;
			if (f.size() != kNumIcons * sizeof(icon_t) + kNumRooms * sizeof(room_t))
				error("Mismatching aux data");
			f.read(_gameIcons, kNumIcons * sizeof(icon_t));
			f.read(gameRooms, kNumRooms * sizeof(room_t));
			f.close();
		} else
			error("Can not load aux data");
	}
		break;
	case Common::kPlatformMacintosh:
		loadFile(2498, _gameIcons);
		loadFile(2497, gameRooms);
		loadFile(2486, gameLipsync);
		ConvertMacToPC();
		break;
	default:
		error("Unsupported platform");
	}

	loadFile(0, _mainBankBuf);
	loadFile(402, gameFont);
	loadFile(404, gameDialogs);
	loadFile(403, gameConditions);
}

bool EdenGame::ReadDataSyncVOC(unsigned int num) {
	unsigned int resNum = num - 1 + ((_vm->getPlatform() == Common::kPlatformDOS && _vm->isDemo()) ? 656 : 661);
	unsigned char vocHeader[0x1A];
	int filePos = 0;
	loadpartoffile(resNum, vocHeader, filePos, sizeof(vocHeader));
	filePos += sizeof(vocHeader);
	unsigned char chunkType = 0;
	loadpartoffile(resNum, &chunkType, sizeof(vocHeader), 1);
	filePos++;
	if (chunkType == 5) {
		unsigned int chunkLen = 0;
		loadpartoffile(resNum, &chunkLen, filePos, 3);
		filePos += 3;
		chunkLen = LE32(chunkLen);
		loadpartoffile(resNum, gameLipsync + 7260, filePos, chunkLen);
		return true;
	}
	return false;
}

bool EdenGame::ReadDataSync(uint16 num) {
	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		long pos = READ_LE_UINT32(gameLipsync + num * 4);
		long len = 1024;
		if (pos != -1) {
			loadpartoffile(1936, gameLipsync + 7260, pos, len);
			return true;
		}
	} else
		return ReadDataSyncVOC(num + 1);	//TODO: remove -1 in caller
	return false;
}

void EdenGame::loadpartoffile(uint16 num, void *buffer, int32 pos, int32 len) {
	assert(num < bigfile_header->count);
	pakfile_t *file = &bigfile_header->files[num];
	int32 offs = READ_LE_UINT32(&file->offs);
	debug("* Loading partial resource %d (%s) at 0x%X(+0x%X), %d bytes", num, file->name, offs, pos, len);
	h_bigfile.seek(offs + pos, SEEK_SET);
	h_bigfile.read(buffer, len);
}

void EdenGame::Expand_hsq(void *input, void *output) {
	byte   *src = (byte *)input;
	byte   *dst = (byte *)output;
	byte   *ptr;
	uint16  bit;        // bit
	uint16  queue = 0;  // queue
	uint16  len = 0;
	int16       ofs;
#define GetBit										\
	bit = queue & 1;								\
	queue >>= 1;									\
	if (!queue) {									\
		queue = (src[1] << 8) | src[0]; src += 2;	\
		bit = queue & 1;							\
		queue = (queue >> 1) | 0x8000;				\
	}

	for (;;) {
		GetBit;
		if (bit)
			*dst++ = *src++;
		else {
			len = 0;
			GetBit;
			if (!bit) {
				GetBit;
				len = (len << 1) | bit;
				GetBit;
				len = (len << 1) | bit;
				ofs = 0xFF00 | *src++;      //TODO: -256
			} else {
				ofs = (src[1] << 8) | src[0];
				src += 2;
				len = ofs & 7;
				ofs = (ofs >> 3) | 0xE000;
				if (!len) {
					len = *src++;
					if (!len)
						break;
				}
			}
			ptr = dst + ofs;
			len += 2;
			while (len--)
				*dst++ = *ptr++;
		}
	}
}

//////
void EdenGame::ajouinfo(byte info) {
	byte idx = p_global->next_info_idx;
	if (kPersons[PER_MESSAGER]._roomNum)
		info |= 0x80;
	info_list[idx] = info;
	if (idx == p_global->last_info_idx)
		p_global->_lastInfo = info;
	idx++;
	if (idx == 16)
		idx = 0;
	p_global->next_info_idx = idx;
}

void EdenGame::unlockinfo() {
	for (byte idx = 0; idx < 16; idx++) {
		if (info_list[idx] != 0xFF)
			info_list[idx] &= ~0x80;
	}
	p_global->_lastInfo &= ~0x80;
}

void EdenGame::nextinfo() {
	do {
		byte idx = p_global->last_info_idx;
		info_list[idx] = 0;
		idx++;
		if (idx == 16)
			idx = 0;
		p_global->last_info_idx = idx;
		p_global->_lastInfo = info_list[idx];
	} while (p_global->_lastInfo == 0xFF);
}

void EdenGame::delinfo(byte info) {
	for (byte idx = 0; idx < 16; idx++) {
		if ((info_list[idx] & ~0x80) == info) {
			info_list[idx] = 0xFF;
			if (idx == p_global->last_info_idx)
				nextinfo();
			break;
		}
	}
}

void EdenGame::updateinfolist() {
	for (int idx = 0; idx < 16; idx++)
		info_list[idx] = 0;
}

void EdenGame::init_globals() {
	_gameIcons[16]._cursorId |= 0x8000;

	p_global->_areaNum = Areas::arMo;
	p_global->areaVisitCount = 1;
	p_global->menuItemIdLo = 0;
	p_global->menuItemIdHi = 0;
	p_global->randomNumber = 0;
	p_global->gameTime = 0;
	p_global->_gameDays = 0;
	p_global->chrono = 0;
	p_global->eloiDepartureDay = 0;
	p_global->roomNum = 259;
	p_global->newRoomNum = 0;
	p_global->_phaseNum = 0;
	p_global->metPersonsMask1 = 0;
	p_global->party = 0;
	p_global->_partyOutside = 0;
	p_global->metPersonsMask2 = 0;
	p_global->__UNUSED_1C = 0;
	p_global->phaseActionsCount = 0;
	p_global->_curAreaFlags = 0;
	p_global->curItemsMask = 0;
	p_global->curPowersMask = 0;
	p_global->curPersoItems = 0;
	p_global->_curCharacterPowers = 0;
	p_global->wonItemsMask = 0;
	p_global->wonPowersMask = 0;
	p_global->stepsToFindAppleFast = 0;
	p_global->stepsToFindAppleNormal = 0;
	p_global->_roomPersoItems = 0;
	p_global->_roomCharacterPowers = 0;
	p_global->_gameFlags = GameFlags::gfNone;
	p_global->morkusSpyVideoNum1 = 89;
	p_global->morkusSpyVideoNum2 = 88;
	p_global->morkusSpyVideoNum3 = 83;
	p_global->morkusSpyVideoNum4 = 94;
	p_global->newMusicType = MusicType::mtDontChange;
	p_global->ff_43 = 0;
	p_global->videoSubtitleIndex = 0;
	p_global->_partyInstruments = 0;
	p_global->_monkGotRing = 0;
	p_global->_chronoFlag = 0;
	p_global->curRoomFlags = 0;
	p_global->endGameFlag = 0;
	p_global->_lastInfo = 0;
	p_global->_autoDialog = false;
	p_global->worldTyrannSighted = 0;
	p_global->ff_4D = 0;
	p_global->ff_4E = 0;
	p_global->worldGaveGold = 0;
	p_global->worldHasTriceraptors = 0;
	p_global->worldHasVelociraptors = 0;
	p_global->worldHasTyrann = 0;
	p_global->ff_53 = 0;
	p_global->ff_54 = 0;
	p_global->ff_55 = 0;
	p_global->_gameHours = 0;
	p_global->textToken1 = 0;
	p_global->textToken2 = 0;
	p_global->eloiHaveNews = 0;
	p_global->dialogFlags = 0;
	p_global->curAreaType = 0;
	p_global->curCitadelLevel = 0;
	p_global->newLocation = 0;
	p_global->prevLocation = 0;
	p_global->curPersoFlags = 0;
	p_global->ff_60 = false;
	p_global->_eventType = EventType::etEvent5;
	p_global->ff_62 = 0;
	p_global->_curObjectId = 0;
	p_global->curObjectFlags = 0;
	p_global->ff_65 = 1;
	p_global->_roomCharacterType = 0;
	p_global->_roomCharacterFlags = 0;
	p_global->narratorSequence = 0;
	p_global->ff_69 = 0;
	p_global->ff_6A = 0;
	p_global->frescoNumber = 0;
	p_global->ff_6C = 0;
	p_global->ff_6D = 0;
	p_global->labyrinthDirections = 0;
	p_global->labyrinthRoom = 0;
	p_global->_curCharacterAnimPtr = nullptr;
	p_global->_characterImageBank = 0;
	p_global->roomImgBank = 0;
	p_global->_characterBackgroundBankIdx = 55;
	p_global->ff_D4 = 0;
	p_global->fresqWidth = 0;
	p_global->fresqImgBank = 0;
	p_global->ff_DA = 0;
	p_global->ff_DC = 0;
	p_global->room_x_base = 0;
	p_global->ff_E0 = 0;
	p_global->_dialogType = DialogType::dtTalk;
	p_global->ff_E4 = 0;
	p_global->currentMusicNum = 0;
	p_global->textNum = 0;
	p_global->travelTime = 0;
	p_global->ff_EC = 0;
	p_global->_displayFlags = DisplayFlags::dfFlag1;
	p_global->oldDisplayFlags = 1;
	p_global->_drawFlags = 0;
	p_global->ff_F1 = 0;
	p_global->ff_F2 = 0;
	p_global->menuFlags = 0;
	p_global->ff_F5 = 0;
	p_global->ff_F6 = 0;
	p_global->ff_F7 = 0;
	p_global->ff_F8 = 0;
	p_global->ff_F9 = 0;
	p_global->ff_FA = 0;
	p_global->_animationFlags = 0;
	p_global->giveobj1 = 0;
	p_global->giveobj2 = 0;
	p_global->giveobj3 = 0;
	p_global->ff_100 = 0;
	p_global->roomVidNum = 0;
	p_global->ff_102 = 0;
	p_global->ff_103 = 0;
	p_global->roomBgBankNum = 0;
	p_global->valleyVidNum = 0;
	p_global->updatePaletteFlag = 0;
	p_global->inventoryScrollPos = 0;
	p_global->obj_count = 0;
	p_global->textBankIndex = 69;
	p_global->cita_area_num = 0;
	p_global->ff_113 = 0;
	p_global->lastSalNum = 0;
	p_global->_dialogPtr = nullptr;
	p_global->tape_ptr = tapes;
	p_global->_nextDialogPtr = nullptr;
	p_global->narrator_dialog_ptr = nullptr;
	p_global->last_dialog_ptr = nullptr;
	p_global->nextRoomIcon = nullptr;
	p_global->phraseBufferPtr = nullptr;
	p_global->room_ptr = nullptr;
	p_global->_areaPtr = nullptr;
	p_global->last_area_ptr = nullptr;
	p_global->cur_area_ptr = nullptr;
	p_global->cita_area_firstRoom = 0;
	p_global->_characterPtr = nullptr;
	p_global->_roomCharacter = 0;
	p_global->last_info_idx = 0;
	p_global->next_info_idx = 0;
	p_global->_iconsIndex = 16;
	p_global->persoSpritePtr = nullptr;
	p_global->numGiveObjs = 0;

	initRects();

	_underSubtitlesScreenRect.top = 0;
	_underSubtitlesScreenRect.left = subtitles_x_scr_margin;
	_underSubtitlesScreenRect.right = subtitles_x_scr_margin + subtitles_x_width - 1;
	_underSubtitlesScreenRect.bottom = 176 - 1;

	_underSubtitlesBackupRect.top = 0;
	_underSubtitlesBackupRect.left = subtitles_x_scr_margin;
	_underSubtitlesBackupRect.right = subtitles_x_scr_margin + subtitles_x_width - 1;
	_underSubtitlesBackupRect.bottom = 60 - 1;
}

void EdenGame::initRects() {
	_underTopBarScreenRect = Common::Rect(0, 0, 320 - 1, 16 - 1);
	_underTopBarBackupRect = Common::Rect(0, 0, 320 - 1, 16 - 1);
	_underBottomBarScreenRect = Common::Rect(0, 176, 320 - 1, 200 - 1);  //TODO: original bug? this cause crash in copyrect (this, underBottomBarBackupRect)
	_underBottomBarBackupRect = Common::Rect(0, 16, 320 - 1, 40 - 1);
}

void EdenGame::closesalle() {
	if (p_global->_displayFlags & DisplayFlags::dfPanable) {
		p_global->_displayFlags &= ~DisplayFlags::dfPanable;
		resetScroll();
	}
}

// Original name afsalle1
void EdenGame::displaySingleRoom(room_t *room) {
	byte *ptr = (byte *)getElem(sal_buf, room->ff_0 - 1);
	ptr++;
	for (;;) {
		byte b0 = *ptr++;
		byte b1 = *ptr++;
		int16 index = (b1 << 8) | b0;
		if (index == -1)
			break;
		if (index > 0) {
			int16 x = *ptr++ | (((b1 & 0x2) >> 1) << 8);      //TODO: check me
			int16 y = *ptr++;
			ptr++;
			index &= 0x1FF;
			if (!(p_global->_displayFlags & 0x80)) {
				if (index == 1 || p_global->ff_F7)
					noclipax_avecnoir(index - 1, x, y);
			}
			p_global->ff_F7 = 0;
			continue;
		}
		if (b1 & 0x40) {
			if (b1 & 0x20) {
				char addIcon = 0;
				icon_t *icon = p_global->nextRoomIcon;
				if (b0 < 4) {
					if (p_global->room_ptr->exits[b0])
						addIcon = 1;
				} else if (b0 > 229) {
					if (p_global->_partyOutside & (1 << (b0 - 230)))
						addIcon = 1;
				} else if (b0 >= 100) {
					debug("add object %d", b0 - 100);
					if (objecthere(b0 - 100)) {
						addIcon = 1;
						p_global->ff_F7 = 0xFF;
					}
				} else
					addIcon = 1;
				if (addIcon) {
					icon->_actionId = b0;
					icon->_objectId = b0;
					icon->_cursorId = kActionCursors[b0];
					int16 x = READ_LE_UINT16(ptr);
					ptr += 2;
					int16 y = READ_LE_UINT16(ptr);
					ptr += 2;
					int16 ex = READ_LE_UINT16(ptr);
					ptr += 2;
					int16 ey = READ_LE_UINT16(ptr);
					ptr += 2;
					x += p_global->room_x_base;
					ex += p_global->room_x_base;
					debug("add hotspot at %3d:%3d - %3d:%3d, action = %d", x, y, ex, ey, b0);
#ifdef EDEN_DEBUG
					for (int iii = x; iii < ex; iii++)
						p_mainview_buf[y * 640 + iii] = p_mainview_buf[ey * 640 + iii] = (iii % 2) ? 0 : 255;
					for (int iii = y; iii < ey; iii++)
						p_mainview_buf[iii * 640 + x] = p_mainview_buf[iii * 640 + ex] = (iii % 2) ? 0 : 255;
#endif
					icon->sx = x;
					icon->sy = y;
					icon->ex = ex;
					icon->ey = ey;
					p_global->nextRoomIcon = ++icon;
					icon->sx = -1;
				} else
					ptr += 8;
			} else
				ptr += 8;
		} else
			ptr += 8;
	}
}

// Original name: afsalle
void EdenGame::displayRoom() {
	room_t *room = p_global->room_ptr;
	p_global->_displayFlags = DisplayFlags::dfFlag1;
	p_global->room_x_base = 0;
	p_global->roomBgBankNum = room->background;
	if (room->flags & RoomFlags::rf08) {
		p_global->_displayFlags |= DisplayFlags::dfFlag80;
		if (room->flags & RoomFlags::rfPanable) {
		// Scrollable room on 2 screens
			p_global->_displayFlags |= DisplayFlags::dfPanable;
			p_global->ff_F4 = 0;
			rundcurs();
			saveFriezes();
			useBank(room->bank - 1);
			noclipax_avecnoir(0, 0, 16);
			useBank(room->bank);
			noclipax_avecnoir(0, 320, 16);
			displaySingleRoom(room);
			p_global->room_x_base = 320;
			displaySingleRoom(room + 1);
		} else
			displaySingleRoom(room);
	} else {
		//TODO: roomImgBank is garbage here!
		debug("drawroom: room 0x%X using bank %d", p_global->roomNum, p_global->roomImgBank);
		useBank(p_global->roomImgBank);
		displaySingleRoom(room);
		assert(_vm->ScreenView->_pitch == 320);
	}
}

// Original name: aflieu
void EdenGame::displayPlace() {
	no_perso();
	if (!pomme_q) {
		p_global->_iconsIndex = 16;
		p_global->_autoDialog = false;
	}
	p_global->nextRoomIcon = &_gameIcons[roomIconsBase];
	displayRoom();
	needPaletteUpdate = true;
}

// Original name: loadsal
void EdenGame::loadPlace(int16 num) {
	if (num == p_global->lastSalNum)
		return;
	p_global->lastSalNum = num;
	loadFile(num + 419, sal_buf);
}

void EdenGame::specialoutside() {
	if (p_global->last_area_ptr->type == AreaType::atValley && (p_global->party & PersonMask::pmLeader))
		perso_ici(5);
}

void EdenGame::specialout() {
	if (p_global->_gameDays - p_global->eloiDepartureDay > 2) {
		if (eloirevientq())
			eloirevient();
	}

	if (p_global->_phaseNum >= 32 && p_global->_phaseNum < 48) {
		if (p_global->newLocation == 9 || p_global->newLocation == 4 || p_global->newLocation == 24) {
			kPersons[PER_MESSAGER]._roomNum = 263;
			return;
		}
	}

	if ((p_global->_phaseNum == 434) && (p_global->newLocation == 5)) {
		reste_ici(4);
		kPersons[PER_BOURREAU]._roomNum = 264;
		return;
	}

	if (p_global->_phaseNum < 400) {
		if ((p_global->_gameFlags & GameFlags::gfFlag4000) && p_global->prevLocation == 1
		        && (p_global->party & PersonMask::pmEloi) && p_global->curAreaType == AreaType::atValley)
			eloipart();
	}

	if (p_global->_phaseNum == 386) {
		if (p_global->prevLocation == 1
		        && (p_global->party & PersonMask::pmEloi) && p_global->_areaNum == Areas::arCantura)
			eloipart();
	}
}

void EdenGame::specialin() {
	if (!(p_global->party & PersonMask::pmEloi) && (p_global->_partyOutside & PersonMask::pmEloi) && (p_global->roomNum & 0xFF) == 1) {
		suis_moi(5);
		p_global->eloiHaveNews = 1;
	}
	if (p_global->roomNum == 288)
		p_global->_gameFlags |= GameFlags::gfFlag100 | GameFlags::gfFlag2000;
	if (p_global->roomNum == 3075 && p_global->_phaseNum == 546) {
		incPhase1();
		if (p_global->curItemsMask & 0x2000) { // Morkus' tablet
			bars_out();
			playHNM(92);
			gameRooms[129].exits[0] = 0;
			gameRooms[129].exits[2] = 1;
			p_global->roomNum = 3074;
			kPersons[PER_MANGO]._roomNum = 3074;
			p_global->_eventType = EventType::etEvent5;
			maj_salle(p_global->roomNum);
			return;
		}
		p_global->narratorSequence = 53;
	}
	if (p_global->roomNum == 1793 && p_global->_phaseNum == 336)
		eloipart();
	if (p_global->roomNum == 259 && p_global->_phaseNum == 129)
		p_global->narratorSequence = 12;
	if (p_global->roomNum >= 289 && p_global->roomNum < 359)
		p_global->labyrinthDirections = kLabyrinthPath[(p_global->roomNum & 0xFF) - 33];
	if (p_global->roomNum == 305 && p_global->prevLocation == 103)
		p_global->_gameFlags &= ~GameFlags::gfFlag2000;
	if (p_global->roomNum == 304 && p_global->prevLocation == 105)
		p_global->_gameFlags &= ~GameFlags::gfFlag2000;
	if (p_global->_phaseNum < 226) {
		if (p_global->roomNum == 842)
			p_global->_gameFlags |= GameFlags::gfFlag2;
		if (p_global->roomNum == 1072)
			p_global->_gameFlags |= GameFlags::gfFlag4;
		if (p_global->roomNum == 1329)
			p_global->_gameFlags |= GameFlags::gfFlag8000;
	}
}

void EdenGame::animpiece() {
	room_t *room = p_global->room_ptr;
	if (p_global->roomVidNum && p_global->ff_100 != 0xFF) {
		if (p_global->valleyVidNum || !room->level || (room->flags & RoomFlags::rfHasCitadel)
		        || room->level == p_global->ff_100) {
			bars_out();
			p_global->updatePaletteFlag = 16;
			if (!(p_global->narratorSequence & 0x80)) //TODO: bug? !() @ 100DC
				p_global->ff_102 = 0;
			if (!needToFade)
				needToFade = room->flags & RoomFlags::rf02;
			playHNM(p_global->roomVidNum);
			return;
		}
	}
	p_global->ff_F1 &= ~RoomFlags::rf04;
}

void EdenGame::getdino(room_t *room) {
	assert(tab_2CEF0[4] == 0x25);

	room->flags &= ~0xC;
	for (perso_t *perso = &kPersons[PER_UNKN_18C]; perso->_roomNum != 0xFFFF; perso++) {
		if (perso->_flags & PersonFlags::pf80)
			continue;
		if (perso->_roomNum != p_global->roomNum)
			continue;
		byte persoType = perso->_flags & PersonFlags::pfTypeMask;
		if (persoType == PersonFlags::pftVelociraptor)
			delinfo(p_global->cita_area_num + ValleyNews::vnVelociraptorsIn);
		if (persoType == PersonFlags::pftTriceraptor)
			delinfo(p_global->cita_area_num + ValleyNews::vnTriceraptorsIn);
		perso->_flags |= PersonFlags::pf20;
		int16 *tab = tab_2CF70;
		if (p_global->_areaNum != Areas::arUluru && p_global->_areaNum != Areas::arTamara)
			tab = tab_2CEF0;
		byte r27 = (room->flags & 0xC0) >> 2;    //TODO: check me (like pc)
		persoType = perso->_flags & PersonFlags::pfTypeMask;
		if (persoType == PersonFlags::pftTyrann)
			persoType = 13;
		r27 |= (persoType & 7) << 1;    //TODO: check me 13 & 7 = ???
		tab += r27;
		p_global->roomVidNum = *tab++;
		int16 bank = *tab;
		if (bank & 0x8000) {
			bank &= ~0x8000;
			room->flags |= RoomFlags::rf08;
		}
		room->flags |= RoomFlags::rf04 | RoomFlags::rf02;
		p_global->roomImgBank = bank;
		break;
	}
}

room_t *EdenGame::getsalle(int16 loc) { //TODO: byte?
	debug("get room for %X, starting from %d, looking for %X", loc, p_global->_areaPtr->firstRoomIndex, p_global->_partyOutside);
	room_t *room = &gameRooms[p_global->_areaPtr->firstRoomIndex];
	loc &= 0xFF;
	for (;; room++) {
		for (; room->location != loc; room++) {
			if (room->ff_0 == 0xFF)
				return 0;
		}
		if (p_global->_partyOutside == room->party || room->party == 0xFFFF)
			break;
	}
	debug("found room: party = %X, bank = %X", room->party, room->bank);
	p_global->roomImgBank = room->bank;
	p_global->labyrinthRoom = 0;
	if (p_global->roomImgBank > 104 && p_global->roomImgBank <= 112)
		p_global->labyrinthRoom = p_global->roomImgBank - 103;
	if (p_global->valleyVidNum)
		p_global->roomVidNum = p_global->valleyVidNum;
	else
		p_global->roomVidNum = room->video;
	if ((room->flags & 0xC0) == RoomFlags::rf40 || (room->flags & RoomFlags::rf01))
		getdino(room);
	if (room->flags & RoomFlags::rfHasCitadel) {
		delinfo(p_global->_areaNum + ValleyNews::vnCitadelLost);
		delinfo(p_global->_areaNum + ValleyNews::vnTyrannIn);
		delinfo(p_global->_areaNum + ValleyNews::vnTyrannLost);
		delinfo(p_global->_areaNum + ValleyNews::vnVelociraptorsLost);
	}
	if (istyran(p_global->roomNum))
		p_global->_gameFlags |= GameFlags::gfFlag10;
	else
		p_global->_gameFlags &= ~GameFlags::gfFlag10;
	return room;
}

// Original name: initlieu
void EdenGame::initPlace(int16 roomNum) {
	p_global->_gameFlags |= GameFlags::gfFlag4000;
	_gameIcons[18]._cursorId |= 0x8000;
	p_global->last_area_ptr = p_global->_areaPtr;
	p_global->_areaPtr = &kAreasTable[((roomNum >> 8) & 0xFF) - 1];
	area_t *area = p_global->_areaPtr;
	area->visitCount++;
	p_global->areaVisitCount = area->visitCount;
	p_global->_curAreaFlags = area->_flags;
	p_global->curAreaType = area->type;
	p_global->curCitadelLevel = area->citadelLevel;
	if (p_global->curAreaType == AreaType::atValley)
		_gameIcons[18]._cursorId &= ~0x8000;
	loadPlace(area->salNum);
}

void EdenGame::maj2() {
	displayPlace();
	assert(_vm->ScreenView->_pitch == 320);
	if (p_global->roomNum == 273 && p_global->prevLocation == 18)
		p_global->ff_102 = 1;
	if (p_global->_eventType == EventType::etEventC) {
		drawTopScreen();
		showObjects();
	}
	FRDevents();
	assert(_vm->ScreenView->_pitch == 320);
	bool r30 = false;
	if (p_global->curAreaType == AreaType::atValley && !(p_global->_displayFlags & DisplayFlags::dfPanable))
		r30 = true;
	//TODO: ^^ inlined func?

	if (p_global->ff_102 || p_global->ff_103)
		afficher();
	else if (p_global->ff_F1 == (RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01)) {
		drawBlackBars();
		effet1();
	} else if (p_global->ff_F1 && !(p_global->ff_F1 & RoomFlags::rf04) && !r30) {
		if (!(p_global->_displayFlags & DisplayFlags::dfPanable))
			drawBlackBars();
		else if (p_global->valleyVidNum)
			drawBlackBars();
		effet1();
	} else if (r30 && !(p_global->ff_F1 & RoomFlags::rf04))
		effetpix();
	else
		afficher128();
	musique();
	if (p_global->_eventType != EventType::etEventC) {
		drawTopScreen();
		showObjects();
	}
	showBars();
	showEvents();
	p_global->labyrinthDirections = 0;
	specialin();
}

void EdenGame::majsalle1(int16 roomNum) {
	room_t *room = getsalle(roomNum & 0xFF);
	p_global->room_ptr = room;
	debug("DrawRoom: room 0x%X, arg = 0x%X", p_global->roomNum, roomNum);
	p_global->curRoomFlags = room->flags;
	p_global->ff_F1 = room->flags;
	animpiece();
	p_global->ff_100 = 0;
	maj2();
}

// Original name: updateRoom
void EdenGame::maj_salle(uint16 roomNum) {
	setpersohere();
	majsalle1(roomNum);
}

// Original name: initbuf
void EdenGame::allocateBuffers() {
#define ALLOC(ptr, size, typ) if (!((ptr) = (typ*)malloc(size))) bufferAllocationErrorFl = true;
	ALLOC(bigfile_header, 0x10000, pak_t);
	ALLOC(gameRooms, 0x4000, room_t);
	ALLOC(_gameIcons, 0x4000, icon_t);
	ALLOC(bank_data_buf, 0x10000, byte);
	ALLOC(p_global, sizeof(*p_global), global_t);
	ALLOC(sal_buf, 2048, void);
	ALLOC(gameConditions, 0x4800, byte);
	ALLOC(gameDialogs, 0x2800, byte);
	ALLOC(gamePhrases, 0x10000, byte);
	ALLOC(_mainBankBuf, 0x9400, byte);
	ALLOC(glow_buffer, 0x2800, byte);
	ALLOC(gameFont, 0x900, byte);
	ALLOC(gameLipsync, 0x205C, byte);
	ALLOC(_musicBuf, 0x140000, byte);
#undef ALLOC
}

void EdenGame::freebuf() {
	free(bigfile_header);
	free(gameRooms);
	free(_gameIcons);
	free(bank_data_buf);
	free(p_global);
	free(sal_buf);
	free(gameConditions);
	free(gameDialogs);
	free(gamePhrases);
	free(_mainBankBuf);
	free(glow_buffer);
	free(gameFont);
	free(gameLipsync);
	free(_musicBuf);
}

void EdenGame::openwindow() {
	p_underBarsView = new View(_vm, 320, 40);
	p_underBarsView->_normal._width = 320;

	p_view2 = new View(_vm, 32, 32);
	p_view2_buf = p_view2->_bufferPtr;

	p_subtitlesview = new View(_vm, subtitles_x_width, 60);
	p_subtitlesview_buf = p_subtitlesview->_bufferPtr;

	p_underSubtitlesView = new View(_vm, subtitles_x_width, 60);
	p_underSubtitlesView_buf = p_underSubtitlesView->_bufferPtr;

	p_mainview = new View(_vm, 640, 200);
	p_mainview->_normal._width = 320;
	CLBlitter_FillView(p_mainview, 0xFFFFFFFF);
	p_mainview->CLView_SetSrcZoomValues(0, 0);
	p_mainview->CLView_SetDisplayZoomValues(640, 400);
	p_mainview->CLView_CenterIn(_vm->ScreenView);
	p_mainview_buf = p_mainview->_bufferPtr;

	mouse_x_center = p_mainview->_normal._dstLeft + p_mainview->_normal._width / 2;
	mouse_y_center = p_mainview->_normal._dstTop + p_mainview->_normal._height / 2;
	CLMouse_SetPosition(mouse_x_center, mouse_y_center);
	CLMouse_Hide();

	curs_x = 320 / 2;
	curs_y = 200 / 2;
}

void EdenGame::EmergencyExit() {
	SysBeep(1);
}

void EdenGame::run() {
	invIconsCount = (_vm->getPlatform() == Common::kPlatformMacintosh) ? 9 : 11;
	roomIconsBase = invIconsBase + invIconsCount;

	word_378CE = 0;
	CRYOLib_ManagersInit();
	_vm->_video->setupSound(5, 0x2000, 8, 11025 * 65536.0 , 0);
	_vm->_video->setForceZero2Black(true);
	_vm->_video->setupTimer(12.5);
	voiceSound = CLSoundRaw_New(0, 11025 * 65536.0, 8, 0);
	_hnmSoundChannel = _vm->_video->getSoundChannel();
	CLSound_SetWantsDesigned(1); // CHECKME: Used?

	_musicChannel = new CSoundChannel(_vm->_mixer, 11025, false);
	_voiceChannel = new CSoundChannel(_vm->_mixer, 11025, false);

	allocateBuffers();
	openbigfile();
	openwindow();
	loadpermfiles();

	if (!bufferAllocationErrorFl) {
		LostEdenMac_InitPrefs();
		if (_vm->getPlatform() == Common::kPlatformMacintosh)
			initCubeMac();
		else
			initCubePC();

		p_mainview->_doubled = _doubledScreen;
		while (!quit_flag2) {
			init_globals();
			quit_flag3 = false;
			normalCursor = true;
			torchCursor = false;
			_cursKeepPos = Common::Point(-1, -1);
			if (!_gameLoaded)
				intro();
			edmain();
			startmusique(1);
			drawBlackBars();
			afficher();
			fadetoblack(3);
			ClearScreen();
			playHNM(95);
			if (p_global->endGameFlag == 50) {
				loadrestart();
				_gameLoaded = false;
			}
			fademusica0(2);
			_musicChannel->stop();
			musicPlaying = false;
			mus_enabled = false;
		}
		// LostEdenMac_SavePrefs();
	}

	delete _voiceChannel;
	delete _musicChannel;

	fadetoblack(4);
	closebigfile();
	freebuf();
	CRYOLib_ManagersDone();
}

void EdenGame::edmain() {
	//TODO
	entergame();
	while (!bufferAllocationErrorFl && !quit_flag3 && p_global->endGameFlag != 50) {
		if (!gameStarted) {
			// if in demo mode, reset game after a while
			demoCurrentTicks = _vm->_timerTicks;
			if (demoCurrentTicks - demoStartTicks > 3000) {
				rundcurs();
				afficher();
				fademusica0(2);
				fadetoblack(3);
				CLBlitter_FillScreenView(0);
				CLBlitter_FillView(p_mainview, 0);
				_musicChannel->stop();
				musicPlaying = false;
				mus_enabled = false;
				intro();
				entergame();
			}
		}
		rundcurs();
		musicspy();
		FRDevents();
		narrateur();
		evenchrono();
		if (p_global->_drawFlags & DrawFlags::drDrawInventory)
			showObjects();
		if (p_global->_drawFlags & DrawFlags::drDrawTopScreen)
			drawTopScreen();
		if ((p_global->_displayFlags & DisplayFlags::dfPanable) && (p_global->_displayFlags != DisplayFlags::dfPerson))
			scrollpano();
		if ((p_global->_displayFlags & DisplayFlags::dfMirror) && (p_global->_displayFlags != DisplayFlags::dfPerson))
			scrollMirror();
		if ((p_global->_displayFlags & DisplayFlags::dfFrescoes) && (p_global->_displayFlags != DisplayFlags::dfPerson))
			scrollFrescoes();
		if (p_global->_displayFlags & DisplayFlags::dfFlag2)
			noclicpanel();
		if (animationActive)
			anim_perso();
		update_cursor();
		afficher();
	}
}

void EdenGame::intro() {
	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		// Play intro videos in HQ
		CLSoundChannel_Stop(_hnmSoundChannel);
		_vm->_video->closeSound();
		_vm->_video->setupSound(5, 0x2000, 16, 22050 * 65536.0, 0);
		_hnmSoundChannel = _vm->_video->getSoundChannel();
		playHNM(2012);
		playHNM(171);
		CLBlitter_FillScreenView(0);
		specialTextMode = false;
		playHNM(2001);
		CLSoundChannel_Stop(_hnmSoundChannel);
		_vm->_video->closeSound();
		_vm->_video->setupSound(5, 0x2000, 8, 11025 * 65536.0, 0);
		_hnmSoundChannel = _vm->_video->getSoundChannel();
	} else {
		playHNM(98);	// Cryo logo
		playHNM(171);	// Virgin logo
		CLBlitter_FillScreenView(0);
		specialTextMode = false;
		playHNM(170);	// Intro video
	}
}

void EdenGame::entergame() {
	char flag = 0;
	currentTime = _vm->_timerTicks / 100;
	p_global->gameTime = currentTime;
	demoStartTicks = _vm->_timerTicks;
	gameStarted = false;
	if (!_gameLoaded) {
		p_global->roomNum = 279;
		p_global->_areaNum = Areas::arMo;
		p_global->ff_100 = 0xFF;
		initPlace(p_global->roomNum);
		p_global->currentMusicNum = 0;
		startmusique(1);
	} else {
		flag = p_global->_autoDialog;    //TODO
		initafterload();
		lastMusicNum = p_global->currentMusicNum;   //TODO: ???
		p_global->currentMusicNum = 0;
		startmusique(lastMusicNum);
		p_global->inventoryScrollPos = 0;
		gameStarted = true;
	}
	showObjects();
	drawTopScreen();
	saveFriezes();
	showBlackBars = true;
	p_global->ff_102 = 1;
	maj_salle(p_global->roomNum);
	if (flag) {
		p_global->_iconsIndex = 4;
		p_global->_autoDialog = true;
		parle_moi();
	}
}

void EdenGame::signon(const char *s) {
}

void EdenGame::testPommeQ() {
	if (!CLKeyboard_HasCmdDown())
		return;

	char key = CLKeyboard_GetLastASCII();
	if (key == 'Q' || key == 'q') {
		if (!pomme_q)
			pomme_q = true;
	}
}

void EdenGame::FRDevents() {
	CLKeyboard_Read();
	if (_allowDoubled) {
		if (CLKeyboard_IsScanCodeDown(0x30)) { //TODO: const
			if (!keybd_held) {
				_doubledScreen = !_doubledScreen;
				p_mainview->_doubled = _doubledScreen;
				CLBlitter_FillScreenView(0);
				keybd_held = true;
			}
		} else
			keybd_held = false;
	}
	int16  mouseY;
	int16  mouseX;
	CLMouse_GetPosition(&mouseX, &mouseY);
	mouseX -= mouse_x_center;
	mouseY -= mouse_y_center;
	CLMouse_SetPosition(mouse_x_center, mouse_y_center);
	curs_x += mouseX;
	if (curs_x < 4)
		curs_x = 4;
	if (curs_x > 292)
		curs_x = 292;
	curs_y += mouseY;

	int16 max_y = p_global->_displayFlags == DisplayFlags::dfFlag2 ? 190 : 170;
	if (curs_y < 4)
		curs_y = 4;
	if (curs_y > max_y)
		curs_y = max_y;
	curs_x_pan = curs_x;
	if (curs_y >= 10 && curs_y <= 164 && !(p_global->_displayFlags & DisplayFlags::dfFrescoes))
		curs_x_pan += _scrollPos;
	if (normalCursor) {
		_currCursor = 0;
		current_spot = scan_icon_list(curs_x_pan + _cursCenter, curs_y + _cursCenter, p_global->_iconsIndex);
		if (current_spot)
			_currCursor = current_spot->_cursorId;
	}
	if (_cursCenter == 0 && _currCursor != 53) {
		_cursCenter = 11;
		curs_x -= 11;
	}
	if (_cursCenter == 11 && _currCursor == 53) {
		_cursCenter = 0;
		curs_x += 11;
	}
	if (p_global->_displayFlags & DisplayFlags::dfPanable) {
		//TODO: current_spot may be zero (due to scan_icon_list failure) if cursor slips between hot areas.
		//fix me here or above?
		if (current_spot) { // ok, plug it here
			current_spot2 = current_spot;
			displayAdamMapMark(current_spot2->_actionId - 14);
		}
	}
	if (p_global->_displayFlags == DisplayFlags::dfFlag2 && current_spot)
		current_spot2 = current_spot;
	if (p_global->_displayFlags & DisplayFlags::dfFrescoes) {
		if (_frescoTalk)
			restaurefondbulle();
		if (_currCursor == 9 && !torchCursor) {
			rundcurs();
			torchCursor = true;
			glow_x = -1;
		}
		if (_currCursor != 9 && torchCursor) {
			unglow();
			torchCursor = false;
			curs_saved = false;
		}
	}
	if (CLMouse_IsDown()) {
		if (!mouse_held) {
			mouse_held = true;
			gameStarted = true;
			mouse();
		}
	} else
		mouse_held = false;
	if (p_global->_displayFlags != DisplayFlags::dfFlag2) {
		if (--_inventoryScrollDelay <= 0) {
			if (p_global->obj_count > invIconsCount && curs_y > 164) {
				if (curs_x > 284 && p_global->inventoryScrollPos + invIconsCount < p_global->obj_count) {
					p_global->inventoryScrollPos++;
					_inventoryScrollDelay = 20;
					showObjects();
				}

				if (curs_x < 30 && p_global->inventoryScrollPos != 0) {
					p_global->inventoryScrollPos--;
					_inventoryScrollDelay = 20;
					showObjects();
				}
			}
		}
	}
	if (_inventoryScrollDelay < 0)
		_inventoryScrollDelay = 0;
	if (!pomme_q) {
		testPommeQ();
		if (pomme_q) {
			PommeQ();
			return;     //TODO: useless
		}
	}
}

icon_t *EdenGame::scan_icon_list(int16 x, int16 y, int16 index) {
	icon_t *icon;
	for (icon = &_gameIcons[index]; icon->sx >= 0; icon++) {
		if (icon->_cursorId & 0x8000)
			continue;
#if 0
		// MAC version use this check. Same check is present in PC version, but never used
		// Because of x >= clause two adjacent rooms has 1-pixel wide dead zone between them
		// On valley view screens if cursor slips in this zone a crash in FRDevents occurs
		// due to lack of proper checks
		if (x < icon->ff_0 || x >= icon->ff_4
		        || y < icon->ff_2 || y >= icon->ff_6)
#else
		// PC version has this check inlined in FRDevents
		// Should we keep it or fix edge coordinates in afroom() instead?
		if (x < icon->sx || x > icon->ex
		        || y < icon->sy || y > icon->ey)
#endif
			continue;
		return icon;
	}
	return nullptr;
}

void EdenGame::update_cursor() {
	if (++_torchTick > 3)
		_torchTick = 0;
	if (!_torchTick) {
		_torchCurIndex++;
		_glowIndex++;
	}
	if (_torchCurIndex > 8)
		_torchCurIndex = 0;
	if (_glowIndex > 4)
		_glowIndex = 0;

	if (!torchCursor) {
		use_main_bank();
		sundcurs(curs_x + _scrollPos, curs_y);
		if (_currCursor != 53 && _currCursor < 10) { //TODO: cond
			if (_vm->getPlatform() == Common::kPlatformMacintosh)
				moteur();
			else
				pc_moteur();
		} else
			noclipax(_currCursor, curs_x + _scrollPos, curs_y);
		glow_x = 1;
	} else {
		useBank(117);
		if (curs_x > 294)
			curs_x = 294;
		unglow();
		glow(_glowIndex);
		noclipax(_torchCurIndex, curs_x + _scrollPos, curs_y);
		if (_frescoTalk)
			af_subtitle();
	}
}

void EdenGame::mouse() {
	static void (EdenGame::*mouse_actions[])() = {
		&EdenGame::moveNorth,
		&EdenGame::moveEast,
		&EdenGame::moveSouth,
		&EdenGame::moveWest,
		&EdenGame::plaquemonk,
		&EdenGame::fresquesgraa,
		&EdenGame::pushpierre,
		&EdenGame::tetesquel,
		&EdenGame::tetemomie,
		&EdenGame::moveNorth,
		&EdenGame::roiparle1,
		&EdenGame::roiparle2,
		&EdenGame::roiparle3,
		&EdenGame::gotohall,
		&EdenGame::demitourlabi,
		&EdenGame::squelmoorkong,
		&EdenGame::gotonido,
		&EdenGame::voirlac,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::final,
		&EdenGame::moveNorth,
		&EdenGame::moveSouth,
		&EdenGame::visiter,
		&EdenGame::dinosoufle,
		&EdenGame::fresqueslasc,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		&EdenGame::gotoval,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::getprisme,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::getoeuf,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::getchampb,
		&EdenGame::getchampm,
		&EdenGame::getcouteau,
		&EdenGame::getnidv,
		&EdenGame::getnido,
		&EdenGame::getor,
		nullptr,
		&EdenGame::ret,
		&EdenGame::getsoleil,
		&EdenGame::getcorne,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::ret,
		&EdenGame::ret,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::getplaque,
		&EdenGame::clicplanval,
		&EdenGame::endFrescoes,
		&EdenGame::choisir,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::roi,
		&EdenGame::dina,
		&EdenGame::thoo,
		&EdenGame::monk,
		&EdenGame::bourreau,
		&EdenGame::messager,
		&EdenGame::mango,
		&EdenGame::eve,
		&EdenGame::azia,
		&EdenGame::mammi,
		&EdenGame::gardes,
		&EdenGame::fisher,
		&EdenGame::dino,
		&EdenGame::tyran,
		&EdenGame::morkus,
		&EdenGame::ret,
		&EdenGame::parle_moi,
		&EdenGame::adam,
		&EdenGame::takeobject,
		&EdenGame::putObject,
		&EdenGame::clictimbre,
		&EdenGame::dinaparle,
		&EdenGame::close_perso,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::generique,
		&EdenGame::choixsubtitle,
		&EdenGame::EdenQuit,
		&EdenGame::restart,
		&EdenGame::cancel2,
		&EdenGame::testvoice,
		&EdenGame::reglervol,
		&EdenGame::load,
		&EdenGame::save,
		&EdenGame::cliccurstape,
		&EdenGame::playtape,
		&EdenGame::stoptape,
		&EdenGame::rewindtape,
		&EdenGame::forwardtape,
		&EdenGame::confirmyes,
		&EdenGame::confirmno,
		&EdenGame::gotocarte
	};

	if (!(current_spot = scan_icon_list(curs_x_pan + _cursCenter,
	                                    curs_y + _cursCenter, p_global->_iconsIndex)))
		return;
	current_spot2 = current_spot;
	debug("invoking mouse action %d", current_spot->_actionId);
	if (mouse_actions[current_spot->_actionId])
		(this->*mouse_actions[current_spot->_actionId])();
}

////// film.c
// Original name: showfilm
void EdenGame::showMovie(char arg1) {
	_vm->_video->readHeader(_hnmContext);
	if (_vm->_video->_curVideoNum == 92) {
		// _hnmContext->_header._unusedFlag2 = 0; CHECKME: Useless?
		CLSoundChannel_SetVolumeLeft(_hnmSoundChannel, 0);
		CLSoundChannel_SetVolumeRight(_hnmSoundChannel, 0);
	}

	if (_vm->_video->getVersion(_hnmContext) != 4)
		return;

	bool playing = true;
	_vm->_video->allocMemory(_hnmContext);
	p_hnmview = new View(_vm, _hnmContext->_header._width, _hnmContext->_header._height);
	p_hnmview->CLView_SetSrcZoomValues(0, 0);
	p_hnmview->CLView_SetDisplayZoomValues(_hnmContext->_header._width * 2, _hnmContext->_header._height * 2);
	p_hnmview->CLView_CenterIn(_vm->ScreenView);
	p_hnmview_buf = p_hnmview->_bufferPtr;
	if (arg1) {
		p_hnmview->_normal._height = 160;
		p_hnmview->_zoom._height = 320;   //TODO: width??
		p_hnmview->_normal._dstTop = p_mainview->_normal._dstTop + 16;
		p_hnmview->_zoom._dstTop = p_mainview->_zoom._dstTop + 32;
	}
	_vm->_video->setFinalBuffer(_hnmContext, p_hnmview->_bufferPtr);
	p_hnmview->_doubled = _doubledScreen;
	do {
		hnm_position = _vm->_video->getFrameNum(_hnmContext);
		_vm->_video->waitLoop(_hnmContext);
		playing = _vm->_video->nextElement(_hnmContext);
		if (specialTextMode)
			displayHNMSubtitles();
		else
			musicspy();
		CLBlitter_CopyView2Screen(p_hnmview);
		assert(_vm->ScreenView->_pitch == 320);
		CLKeyboard_Read();
		if (_allowDoubled) {
			if (CLKeyboard_IsScanCodeDown(0x30)) { //TODO: const
				if (!keybd_held) {
					_doubledScreen = !_doubledScreen;
					p_hnmview->_doubled = _doubledScreen;   //TODO: but mainview ?
					CLBlitter_FillScreenView(0);
					keybd_held = true;
				}
			} else
				keybd_held = false;
		}
		if (arg1) {
			if (CLMouse_IsDown()) {
				if (!mouse_held) {
					mouse_held = true;
					videoCanceled = 1;
				}
			} else
				mouse_held = false;
		}
	} while (playing && !videoCanceled);
	delete p_hnmview;
	_vm->_video->deallocMemory(_hnmContext);
}

void EdenGame::playHNM(int16 num) {
	perso_t *perso = nullptr;
	int16 oldDialogType = -1;
	_vm->_video->_curVideoNum = num;
	if (num != 2001 && num != 2012 && num != 98 && num != 171) {
		byte oldMusicType = p_global->newMusicType;
		p_global->newMusicType = MusicType::mtEvent;
		musique();
		musicspy();
		p_global->newMusicType = oldMusicType;
	}
	p_global->videoSubtitleIndex = 1;
	if (specialTextMode) {
		perso = p_global->_characterPtr;
		oldDialogType = p_global->_dialogType;
		prechargephrases(num);
		fademusica0(1);
		_musicChannel->stop();
	}
	showVideoSubtitle = false;
	videoCanceled = 0;
	shnmfl(num);
	_vm->_video->reset(_hnmContext);
	_vm->_video->flushPreloadBuffer(_hnmContext);
	if (needToFade) {
		fadetoblack(4);
		ClearScreen();
		needToFade = false;
	}
	if (num == 2012 || num == 98 || num == 171)
		showMovie(0);
	else
		showMovie(1);
	_cursKeepPos = Common::Point(-1, -1);
	p_mainview->_doubled = _doubledScreen;
	if (specialTextMode) {
		_musicFadeFlag = 3;
		musicspy();
		p_global->_characterPtr = perso;
		p_global->_dialogType = oldDialogType;
		specialTextMode = false;
	}
	if (videoCanceled)
		p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_vm->_video->_curVideoNum == 167)
		p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_vm->_video->_curVideoNum == 104)
		p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_vm->_video->_curVideoNum == 102)
		p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_vm->_video->_curVideoNum == 77)
		p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_vm->_video->_curVideoNum == 149)
		p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
}

// Original name bullehnm
void EdenGame::displayHNMSubtitles() {
	uint16 *frames;
	perso_t *perso;
	switch (_vm->_video->_curVideoNum) {
	case 170:
		frames = kFramesVid170;
		perso = &kPersons[PER_UNKN_156];
		break;
	case 83:
		frames = kFramesVid83;
		perso = &kPersons[PER_MORKUS];
		break;
	case 88:
		frames = kFramesVid88;
		perso = &kPersons[PER_MORKUS];
		break;
	case 89:
		frames = kFramesVid89;
		perso = &kPersons[PER_MORKUS];
		break;
	case 94:
		frames = kFramesVid94;
		perso = &kPersons[PER_MORKUS];
		break;
	default:
		return;
	}
	uint16 *frames_start = frames;
	uint16 frame;
	while ((frame = *frames++) != 0xFFFF) {
		if ((frame & ~0x8000) == hnm_position)
			break;
	}
	if (frame == 0xFFFF) {
		if (showVideoSubtitle)
			af_subtitlehnm();
		return;
	}
	if (frame & 0x8000)
		showVideoSubtitle = false;
	else {
		p_global->videoSubtitleIndex = (frames - frames_start) / 2 + 1;
		p_global->_characterPtr = perso;
		p_global->_dialogType = DialogType::dtInspect;
		int16 num = (perso->_id << 3) | p_global->_dialogType;
		dialoscansvmas((dial_t *)getElem(gameDialogs, num));
		showVideoSubtitle = true;
	}
	if (showVideoSubtitle)
		af_subtitlehnm();
}

////// sound.c
void EdenGame::musique() {
	if (p_global->newMusicType == MusicType::mtDontChange)
		return;

	dial_t *dial = (dial_t *)getElem(gameDialogs, 128);
	for (;;dial++) {
		if (dial->_flags == -1 && dial->_condNumLow == -1)
			return;
		byte flag = dial->_flags;
		byte hidx = (dial->_textCondHiMask & 0xC0) >> 6;
		byte lidx = dial->_condNumLow;            //TODO: fixme - unsigned = signed
		if (flag & 0x10)
			hidx |= 4;
		if (testcondition(((hidx << 8) | lidx) & 0x7FF))
			break;
	}
	byte mus = dial->_textNumLow;
	p_global->newMusicType = MusicType::mtDontChange;
	if (mus != 0 && mus != 2 && mus < 50)
		startmusique(mus);
}

void EdenGame::startmusique(byte num) {
	if (num == p_global->currentMusicNum)
		return;

	if (musicPlaying) {
		fademusica0(1);
		_musicChannel->stop();
	}
	loadmusicfile(num);
	p_global->currentMusicNum = num;
	_musSequencePtr = _musicBuf + 32;  //TODO: rewrite it properly
	int16 seq_size = READ_LE_UINT16(_musicBuf + 30);
	mus_patterns_ptr = _musicBuf + 30 + seq_size;
	int16 pat_size = READ_LE_UINT16(_musicBuf + 27);
	mus_samples_ptr = _musicBuf + 32 + 4 + pat_size;
	int16 freq = READ_LE_UINT16(mus_samples_ptr - 2);

	delete _musicChannel;
	_musicChannel = new CSoundChannel(_vm->_mixer, freq == 166 ? 11025 : 22050, false);
	mus_enabled = true;

	musicSequencePos = 0;
	mus_vol_left = p_global->pref_10C[0];
	mus_vol_right = p_global->pref_10C[1];
	_musicChannel->setVolume(mus_vol_left, mus_vol_right);
}

void EdenGame::musicspy() {
	if (!mus_enabled)
		return;
	mus_vol_left = p_global->pref_10C[0];
	mus_vol_right = p_global->pref_10C[1];
	if (_musicFadeFlag & 3)
		fademusicup();
	if (_personTalking && !_voiceChannel->numQueued())
		_musicFadeFlag = 3;
	if (_musicChannel->numQueued() < 3) {
		byte patnum = _musSequencePtr[(int)musicSequencePos];
		if (patnum == 0xFF) {
			// rewind
			musicSequencePos = 0;
			patnum = _musSequencePtr[(int)musicSequencePos];
		}
		musicSequencePos++;
		byte *patptr = mus_patterns_ptr + patnum * 6;
		int ofs = patptr[0] + (patptr[1] << 8) + (patptr[2] << 16);
		int len = patptr[3] + (patptr[4] << 8) + (patptr[5] << 16);
		_musicChannel->queueBuffer(mus_samples_ptr + ofs, len);
		musicPlaying = true;
	}
}

int EdenGame::loadmusicfile(int16 num) {
	pakfile_t *file = &bigfile_header->files[num + 435];
	int32 size = READ_LE_UINT32(&file->size);
	int32 offs = READ_LE_UINT32(&file->offs);
	h_bigfile.seek(offs, SEEK_SET);
	int32 numread = size;
	if (numread > 0x140000)     //TODO: const
		numread = 0x140000;
	h_bigfile.read(_musicBuf, numread);
	return size;
}

void EdenGame::persovox() {
	int16 num = p_global->textNum;
	if (p_global->textBankIndex != 1)
		num += 565;
	if (p_global->textBankIndex == 3)
		num += 707;
	voiceSamplesSize = ssndfl(num);
	int16 volumeLeft = p_global->pref_110[0];
	int16 volumeRight = p_global->pref_110[1];
	int16 stepLeft = _musicChannel->_volumeLeft < volumeLeft ? stepLeft = 1 : -1;
	int16 stepRight = _musicChannel->_volumeRight < volumeRight ? stepRight = 1 : -1;
	do {
		if (volumeLeft != _musicChannel->_volumeLeft)
			_musicChannel->setVolumeLeft(_musicChannel->_volumeLeft + stepLeft);
		if (volumeRight != _musicChannel->_volumeRight)
			_musicChannel->setVolumeRight(_musicChannel->_volumeRight + stepRight);
	} while (_musicChannel->_volumeLeft != volumeLeft || _musicChannel->_volumeRight != volumeRight);
	volumeLeft = p_global->pref_10E[0];
	volumeRight = p_global->pref_10E[1];
	_voiceChannel->setVolume(volumeLeft, volumeRight);
	_voiceChannel->queueBuffer((byte*)voiceSamplesBuffer, voiceSamplesSize, true);
	_personTalking = true;
	_musicFadeFlag = 0;
	_lastAnimTicks = _vm->_timerTicks;
}

void EdenGame::endpersovox() {
	restaurefondbulle();
	if (_personTalking) {
		_voiceChannel->stop();
		_personTalking = false;
		_musicFadeFlag = 3;
	}

	if (_soundAllocated) {
		free(voiceSamplesBuffer);
		voiceSamplesBuffer = nullptr;
		_soundAllocated = false;
	}
}

void EdenGame::fademusicup() {
	if (_musicFadeFlag & 2) {
		int16 vol = _musicChannel->_volumeLeft;
		if (vol < mus_vol_left) {
			vol += 8;
			if (vol > mus_vol_left)
				vol = mus_vol_left;
		} else {
			vol -= 8;
			if (vol < mus_vol_left)
				vol = mus_vol_left;
		}
		_musicChannel->setVolumeLeft(vol);
		if (vol == mus_vol_left)
			_musicFadeFlag &= ~2;
	}
	if (_musicFadeFlag & 1) {
		int16 vol = _musicChannel->_volumeRight;
		if (vol < mus_vol_right) {
			vol += 8;
			if (vol > mus_vol_right)
				vol = mus_vol_right;
		} else {
			vol -= 8;
			if (vol < mus_vol_right)
				vol = mus_vol_right;
		}
		_musicChannel->setVolumeRight(vol);
		if (vol == mus_vol_right)
			_musicFadeFlag &= ~1;
	}
}

void EdenGame::fademusica0(int16 delay) {
	int16 volume;
	while ((volume = _musicChannel->getVolume()) > 2) {
		volume -= 2;
		if (volume < 2)
			volume = 2;
		_musicChannel->setVolume(volume, volume);
		wait(delay);
	}
}

//// obj.c
object_t *EdenGame::getobjaddr(int16 id) {
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		if (_objects[i]._id == id)
			break;
	}

	return _objects + i;
}

void EdenGame::countobjects() {
	int16 index = 0;
	byte total = 0;
	for (int i = 0; i < MAX_OBJECTS; i++) {
		int16 count = _objects[i]._count;
#ifdef EDEN_DEBUG
		count = 1;
		goto show_all_objects;  //DEBUG
#endif
		if (count == 0)
			continue;
		if (_objects[i]._flags & ObjectFlags::ofInHands)
			count--;
show_all_objects:
		;
		if (count) {
			total += count;
			while (count--)
				own_objects[index++] = _objects[i]._id;
		}
	}
	p_global->obj_count = total;
}

void EdenGame::showObjects() {
	icon_t *icon = &_gameIcons[invIconsBase];
	p_global->_drawFlags &= ~(DrawFlags::drDrawInventory | DrawFlags::drDrawFlag2);
	countobjects();
	int16 total = p_global->obj_count;
	for (int16 i = invIconsCount; i--; icon++) {
		if (total) {
			icon->_cursorId &= ~0x8000;
			total--;
		} else
			icon->_cursorId |= 0x8000;
	}
	use_main_bank();
	noclipax(55, 0, 176);
	icon = &_gameIcons[invIconsBase];
	total = p_global->obj_count;
	int16 index = p_global->inventoryScrollPos;
	for (int16 i = invIconsCount; total-- && i--; icon++) {
		char obj = own_objects[index++];
		icon->_objectId = obj;
		noclipax(obj + 9, icon->sx, 178);
	}
	needPaletteUpdate = true;
	if ((p_global->_displayFlags & DisplayFlags::dfMirror) || (p_global->_displayFlags & DisplayFlags::dfPanable)) {
		saveBottomFrieze();
		scroll();
	}
}

void EdenGame::winObject(int16 id) {
	object_t *object = getobjaddr(id);
	object->_flags |= ObjectFlags::ofFlag1;
	object->_count++;
	p_global->curItemsMask |= object->_itemMask;
	p_global->wonItemsMask |= object->_itemMask;
	p_global->curPowersMask |= object->_powerMask;
	p_global->wonPowersMask |= object->_powerMask;
}

void EdenGame::loseObject(int16 id) {
	object_t *object = getobjaddr(id);
	if (object->_count > 0)
		object->_count--;
	if (!object->_count) {
		object->_flags &= ~ObjectFlags::ofFlag1;
		p_global->curItemsMask &= ~object->_itemMask;
		p_global->curPowersMask &= ~object->_powerMask;
	}
	p_global->_curObjectId = 0;
	p_global->curObjectFlags = 0;
	p_global->curObjectCursor = 9;
	_gameIcons[16]._cursorId |= 0x8000;
	object->_flags &= ~ObjectFlags::ofInHands;
	normalCursor = true;
	_currCursor = 0;
	torchCursor = false;
}

void EdenGame::lostobject() {
	parlemoiNormalFlag = true;
	if (p_global->_curObjectId)
		loseObject(p_global->_curObjectId);
}

bool EdenGame::objecthere(int16 id) {
	object_t *object = getobjaddr(id);
	for (pCurrentObjectLocation = &kObjectLocations[object->_locations]; *pCurrentObjectLocation != 0xFFFF; pCurrentObjectLocation++) {
		if (*pCurrentObjectLocation == p_global->roomNum)
			return true;
	}
	return false;
}

void EdenGame::objectmain(int16 id) {
	object_t *object = getobjaddr(id);
	_gameIcons[16]._cursorId &= ~0x8000;
	p_global->_curObjectId = object->_id;
	p_global->curObjectCursor = p_global->_curObjectId + 9;
	object->_flags |= ObjectFlags::ofInHands;
	p_global->curObjectFlags = object->_flags;
	_currCursor = p_global->_curObjectId + 9;
	normalCursor = false;
}

void EdenGame::getobject(int16 id) {
	room_t *room = p_global->room_ptr;
	if (p_global->_curObjectId)
		return;
	if (!objecthere(id))
		return;
	*pCurrentObjectLocation |= 0x8000;
	objectmain(id);
	winObject(id);
	showObjects();
	p_global->roomImgBank = room->bank;
	p_global->roomVidNum = room->video;
	displayPlace();
}

void EdenGame::putObject() {
	if (!p_global->_curObjectId)
		return;
	_gameIcons[16]._cursorId |= 0x8000;
	object_t *object = getobjaddr(p_global->_curObjectId);
	p_global->curObjectCursor = 9;
	p_global->_curObjectId = 0;
	p_global->curObjectFlags = 0;
	object->_flags &= ~ObjectFlags::ofInHands;
	p_global->_nextDialogPtr = nullptr;
	_closeCharacterDialog = false;
	p_global->_dialogType = DialogType::dtTalk;
	showObjects();
	normalCursor = true;
}

void EdenGame::newobject(int16 id, int16 arg2) {
	object_t *object = getobjaddr(id);
	uint16 e, *t = &kObjectLocations[object->_locations];
	while ((e = *t) != 0xFFFF) {
		e &= ~0x8000;
		if ((e >> 8) == arg2)
			*t = e;
		t++;
	}
}

void EdenGame::giveobjectal(int16 id) {
	if (id == Objects::obKnife)
		kObjectLocations[2] = 0;
	if (id == Objects::obApple)
		p_global->stepsToFindAppleNormal = 0;
	if (id >= Objects::obEyeInTheStorm && id < (Objects::obRiverThatWinds + 1) && p_global->_roomCharacterType == PersonFlags::pftVelociraptor) {
		//TODO: fix that cond above
		object_t *object = getobjaddr(id);
		p_global->_roomCharacter->_powers &= ~object->_powerMask;
	}
	winObject(id);
}

void EdenGame::giveObject() {
	byte id = p_global->giveobj1;
	if (id) {
		p_global->giveobj1 = 0;
		giveobjectal(id);
	}
	id = p_global->giveobj2;
	if (id) {
		p_global->giveobj2 = 0;
		giveobjectal(id);
	}
	id = p_global->giveobj3;
	if (id) {
		p_global->giveobj3 = 0;
		giveobjectal(id);
	}
}

void EdenGame::takeobject() {
	objectmain(current_spot2->_objectId);
	p_global->_nextDialogPtr = nullptr;
	_closeCharacterDialog = false;
	p_global->_dialogType = DialogType::dtTalk;
	if (p_global->inventoryScrollPos)
		p_global->inventoryScrollPos--;
	showObjects();
}
////
void EdenGame::newchampi() {
	if (_objects[Objects::obShroom - 1]._count == 0) {
		newobject(Objects::obShroom, p_global->cita_area_num);
		newobject(Objects::obBadShroom, p_global->cita_area_num);
	}
}

void EdenGame::newnidv() {
	room_t *room = p_global->cita_area_firstRoom;
	if (_objects[Objects::obNest - 1]._count)
		return;
	object_t *obj = getobjaddr(Objects::obNest);
	for (uint16 *ptr = kObjectLocations + obj->_locations; *ptr != 0xFFFF; ptr++) {
		if ((*ptr & ~0x8000) >> 8 != p_global->cita_area_num)
			continue;
		*ptr &= ~0x8000;
		for (; room->ff_0 != 0xFF; room++) {
			if (room->location == (*ptr & 0xFF)) {
				room->bank = 279;
				room->ff_0 = 9;
				room++;
				room->bank = 280;
				return;
			}
		}
	}
}

void EdenGame::newnido() {
	room_t *room = p_global->cita_area_firstRoom;
	if (_objects[Objects::obFullNest - 1]._count)
		return;
	if (_objects[Objects::obNest - 1]._count)
		return;
	object_t *obj = getobjaddr(Objects::obFullNest);
	for (uint16 *ptr = kObjectLocations + obj->_locations; *ptr != 0xFFFF; ptr++) {
		if ((*ptr & ~0x8000) >> 8 != p_global->cita_area_num)
			continue;
		*ptr &= ~0x8000;
		for (; room->ff_0 != 0xFF; room++) {
			if (room->location == (*ptr & 0xFF)) {
				room->bank = 277;
				room->ff_0 = 9;
				room++;
				room->bank = 278;
				return;
			}
		}
	}
}

void EdenGame::newor() {
	if (_objects[Objects::obGold - 1]._count == 0) {
		newobject(Objects::obGold, p_global->cita_area_num);
	}
}

void EdenGame::gotopanel() {
	if (pomme_q)
		byte_31D64 = p_global->_autoDialog;  //TODO: check me
	_noPalette = false;
	p_global->_iconsIndex = 85;
	p_global->_characterPtr = nullptr;
	p_global->_drawFlags |= DrawFlags::drDrawMenu;
	p_global->_displayFlags = DisplayFlags::dfFlag2;
	p_global->menuFlags = 0;
	affpanel();
	fadetoblack(3);
	afftoppano();
	CLBlitter_CopyView2Screen(p_mainview);
	CLPalette_Send2Screen(global_palette, 0, 256);
	curs_x = 320 / 2;
	curs_y = 200 / 2;
	CLMouse_SetPosition(mouse_x_center, mouse_y_center);
}

void EdenGame::noclicpanel() {
	if (p_global->menuFlags & MenuFlags::mfFlag4) {
		depcurstape();
		return;
	}
	if (p_global->_drawFlags & DrawFlags::drDrawFlag8)
		return;
	if (p_global->menuFlags & MenuFlags::mfFlag1) {
		changervol();
		return;
	}
	byte num;
	if (current_spot2 >= &_gameIcons[119]) {
		debug("noclic: objid = %p, glob3,2 = %2X %2X", (void *)current_spot2, p_global->menuItemIdHi, p_global->menuItemIdLo);
		if (current_spot2->_objectId == (uint16)((p_global->menuItemIdLo + p_global->menuItemIdHi) << 8)) //TODO: check me
			return;
	} else {
		int idx = current_spot2 - &_gameIcons[105];
		if (idx == 0) {
			p_global->menuItemIdLo = 1;
			num = 1;
			goto skip;
		}
		num = idx & 0x7F + 1;
		if (num >= 5)
			num = 1;
		if (num == p_global->ff_43)
			return;
		p_global->ff_43 = 0;
	}
	num = p_global->menuItemIdLo;
	p_global->menuItemIdLo = current_spot2->_objectId & 0xFF;
skip:
	;
	p_global->menuItemIdHi = (current_spot2->_objectId & 0xFF00) >> 8;
	debug("noclic: new glob3,2 = %2X %2X", p_global->menuItemIdHi, p_global->menuItemIdLo);
	affresult();
	num &= 0xF0;
	if (num != 0x30)
		num = p_global->menuItemIdLo & 0xF0;
	if (num == 0x30)
		affcurseurs();
}

void EdenGame::generique() {
	drawBlackBars();
	afficher();
	fadetoblack(3);
	ClearScreen();
	int oldmusic = p_global->currentMusicNum;
	playHNM(95);
	affpanel();
	afftoppano();
	needPaletteUpdate = true;
	startmusique(oldmusic);
}

void EdenGame::cancel2() {
	drawTopScreen();
	showObjects();
	p_global->_iconsIndex = 16;
	p_global->_drawFlags &= ~DrawFlags::drDrawMenu;
	gametomiroir(1);
}

void EdenGame::testvoice() {
	p_global->frescoNumber = 0;
	p_global->_characterPtr = kPersons;
	p_global->_dialogType = DialogType::dtInspect;
	int16 num = (kPersons[0]._id << 3) | p_global->_dialogType;
	dialoscansvmas((dial_t *)getElem(gameDialogs, num));
	restaurefondbulle();
	af_subtitle();
	persovox();
	waitEndSpeak();
	endpersovox();
	p_global->ff_CA = 0;
	p_global->_dialogType = DialogType::dtTalk;
}

void EdenGame::load() {
	char name[132];
	_gameLoaded = false;
	byte oldMusic = p_global->currentMusicNum;   //TODO: from ush to byte?!
	fademusica0(1);
	desktopcolors();
	FlushEvents(-1, 0);
//	if(OpenDialog(0, 0)) //TODO: write me
	{
		// TODO
		strcpy(name, "edsave1.000");
		loadgame(name);
	}
	CLMouse_Hide();
	CLBlitter_FillScreenView(0xFFFFFFFF);
	fadetoblack(3);
	CLBlitter_FillScreenView(0);
	if (!_gameLoaded) {
		_musicFadeFlag = 3;
		musicspy();
		needPaletteUpdate = true;
		return;
	}
	if ((oldMusic & 0xFF) != p_global->currentMusicNum) { //TODO: r30 is uns char/bug???
		oldMusic = p_global->currentMusicNum;
		p_global->currentMusicNum = 0;
		startmusique(oldMusic);
	} else {
		_musicFadeFlag = 3;
		musicspy();
	}
	bool talk = p_global->_autoDialog;    //TODO check me
	initafterload();
	fadetoblack(3);
	CLBlitter_FillScreenView(0);
	CLBlitter_FillView(p_mainview, 0);
	drawTopScreen();
	p_global->inventoryScrollPos = 0;
	showObjects();
	maj_salle(p_global->roomNum);
	if (talk) {
		p_global->_iconsIndex = 4;
		p_global->_autoDialog = true;
		parle_moi();
	}

}

void EdenGame::initafterload() {
	p_global->_characterImageBank = 0;
	p_global->lastSalNum = 0;
	loadPlace(p_global->_areaPtr->salNum);
	_gameIcons[18]._cursorId |= 0x8000;
	if (p_global->curAreaType == AreaType::atValley)
		_gameIcons[18]._cursorId &= ~0x8000;
	kPersoRoomBankTable[30] = 27;
	if (p_global->_phaseNum >= 352)
		kPersoRoomBankTable[30] = 26;
	_animateTalking = false;
	animationActive = false;
	p_global->ff_100 = 0;
	p_global->_eventType = EventType::etEventC;
	p_global->valleyVidNum = 0;
	p_global->_drawFlags &= ~DrawFlags::drDrawMenu;
	currentTime = _vm->_timerTicks / 100;
	p_global->gameTime = currentTime;
	if (p_global->_roomCharacterType == PersonFlags::pftTyrann)
		chronoon(3000);
	_adamMapMarkPos.x = -1;
	_adamMapMarkPos.y = -1;
}

void EdenGame::save() {
	char name[260];
	fademusica0(1);
	desktopcolors();
	FlushEvents(-1, 0);
	//SaveDialog(byte_37150, byte_37196->ff_A);
	//TODO
	strcpy(name, "edsave1.000");
	savegame(name);
	CLMouse_Hide();
	CLBlitter_FillScreenView(0xFFFFFFFF);
	fadetoblack(3);
	CLBlitter_FillScreenView(0);
	_musicFadeFlag = 3;
	musicspy();
	needPaletteUpdate = true;
}

void EdenGame::desktopcolors() {
	fadetoblack(3);
	CLBlitter_FillScreenView(0xFFFFFFFF);
	CLPalette_BeSystem();
	CLMouse_Show();
}

void EdenGame::panelrestart() {
	_gameLoaded = false;
	byte curmus = p_global->currentMusicNum;
	byte curlng = p_global->pref_language;
	loadrestart();
	p_global->pref_language = curlng;
	if (!_gameLoaded) //TODO always?
		return;
	p_global->_characterImageBank = 0;
	p_global->lastSalNum = 0;
	loadPlace(p_global->_areaPtr->salNum);
	p_global->_displayFlags = DisplayFlags::dfFlag1;
	_gameIcons[18]._cursorId |= 0x8000;
	if (p_global->curAreaType == AreaType::atValley)
		_gameIcons[18]._cursorId &= ~0x8000;
	kPersoRoomBankTable[30] = 27;
	if (p_global->_phaseNum >= 352)
		kPersoRoomBankTable[30] = 26;
	_animateTalking = false;
	animationActive = false;
	p_global->ff_100 = 0;
	p_global->_eventType = 0;
	p_global->valleyVidNum = 0;
	p_global->_drawFlags &= ~DrawFlags::drDrawMenu;
	p_global->inventoryScrollPos = 0;
	_adamMapMarkPos.x = -1;
	_adamMapMarkPos.y = -1;
	if (curmus != p_global->currentMusicNum) {
		curmus = p_global->currentMusicNum;
		p_global->currentMusicNum = 0;
		startmusique(curmus);
	}
	fadetoblack(3);
	CLBlitter_FillScreenView(0);
	CLBlitter_FillView(p_mainview, 0);
	drawTopScreen();
	showObjects();
	saveFriezes();
	showBlackBars = true;
	maj_salle(p_global->roomNum);
}

void EdenGame::reallyquit() {
	quit_flag3 = true;
	quit_flag2 = true;
}

void EdenGame::confirmer(char mode, char yesId) {
	p_global->_iconsIndex = 119;
	_gameIcons[119]._objectId = yesId;
	confirmMode = mode;
	useBank(65);
	noclipax(12, 117, 74);
	curs_x = 156;
	if (pomme_q)
		curs_x = 136;
	curs_y = 88;
}

void EdenGame::confirmyes() {
	affpanel();
	p_global->_iconsIndex = 85;
	switch (confirmMode) {
	case 1:
		panelrestart();
		break;
	case 2:
		reallyquit();
		break;
	}
}

void EdenGame::confirmno() {
	affpanel();
	p_global->_iconsIndex = 85;
	pomme_q = false;
}

void EdenGame::restart() {
	confirmer(1, current_spot2->_objectId);
}

void EdenGame::EdenQuit() {
	confirmer(2, current_spot2->_objectId);
}

void EdenGame::choixsubtitle() {
	byte lang = current_spot2->_objectId & 0xF;
	if (lang == p_global->pref_language)
		return;
	if (lang > 5)
		return;
	p_global->pref_language = lang;
	langbuftopanel();
	afflangue();
}

void EdenGame::reglervol() {
	byte *valptr = &p_global->pref_10C[current_spot2->_objectId & 7];
	curs_y = 104 - ((*valptr >> 2) & 0x3F); // TODO: check me
	cur_slider_value_ptr = valptr;
	p_global->menuFlags |= MenuFlags::mfFlag1;
	if (current_spot2->_objectId & 8)
		p_global->menuFlags |= MenuFlags::mfFlag2;
	cur_slider_x = current_spot2->sx;
	cur_slider_y = curs_y;
}

void EdenGame::changervol() {
	if (mouse_held) {
		limitezonecurs(cur_slider_x - 1, cur_slider_x + 3, 40, 110);
		int16 delta = cur_slider_y - curs_y;
		if (delta == 0)
			return;
		newvol(cur_slider_value_ptr, delta);
		if (p_global->menuFlags & MenuFlags::mfFlag2)
			newvol(cur_slider_value_ptr + 1, delta);
		cursbuftopanel();
		affcurseurs();
		cur_slider_y = curs_y;
	} else
		p_global->menuFlags &= ~(MenuFlags::mfFlag1 | MenuFlags::mfFlag2);
}

void EdenGame::newvol(byte *volptr, int16 delta) {
	int16 vol = *volptr / 4;
	vol += delta;
	if (vol < 0)
		vol = 0;
	if (vol > 63)
		vol = 63;
	*volptr = vol * 4;
	_musicChannel->setVolume(p_global->pref_10C[0], p_global->pref_10C[1]);
}

void EdenGame::playtape() {
	if (p_global->menuItemIdHi & 8)
		p_global->tape_ptr++;
	for (;; p_global->tape_ptr++) {
		if (p_global->tape_ptr == &tapes[MAX_TAPES]) {
			p_global->tape_ptr--;
			stoptape();
			return;
		}
		if (p_global->tape_ptr->_textNum)
			break;
	}
	p_global->menuFlags |= MenuFlags::mfFlag8;
	p_global->_drawFlags &= ~DrawFlags::drDrawMenu;
	uint16 oldRoomNum = p_global->roomNum;
	uint16 oldParty = p_global->party;
	byte oldBack = p_global->roomBgBankNum;
	perso_t *oldPerso = p_global->_characterPtr;
	p_global->party = p_global->tape_ptr->_party;
	p_global->roomNum = p_global->tape_ptr->_roomNum;
	p_global->roomBgBankNum = p_global->tape_ptr->_bgBankNum;
	p_global->_dialogPtr = p_global->tape_ptr->_dialog;
	p_global->_characterPtr = p_global->tape_ptr->_perso;
	endpersovox();
	affcurstape();
	if (p_global->_characterPtr != oldPerso
	        || p_global->roomNum != lastTapeRoomNum) {
		lastTapeRoomNum = p_global->roomNum;
		p_global->_curCharacterAnimPtr = nullptr;
		p_global->ff_CA = 0;
		p_global->_characterImageBank = -1;
		anim_perfin();
		load_perso_cour();
	}
	af_fondperso();
	p_global->textNum = p_global->tape_ptr->_textNum;
	my_bulle();
	getdatasync();
	showpersopanel();
	persovox();
	p_global->roomBgBankNum = oldBack;
	p_global->party = oldParty;
	p_global->roomNum = oldRoomNum;
}

void EdenGame::rewindtape() {
	if (p_global->tape_ptr > tapes) {
		p_global->tape_ptr--;
		p_global->menuFlags &= ~MenuFlags::mfFlag8;
		affcurstape();
	}
}

void EdenGame::depcurstape() {
	if (mouse_held) {
		limitezonecurs(95, 217, 179, 183);
		int idx = (curs_x - 97);
		if (idx < 0)
			idx = 0;

		idx /= 8;
		tape_t *tape = tapes + idx;
		if (tape >= tapes + 16)
			tape = tapes + 16 - 1;

		if (tape != p_global->tape_ptr) {
			p_global->tape_ptr = tape;
			affcurstape();
			p_global->menuFlags &= ~MenuFlags::mfFlag8;
		}
	} else
		p_global->menuFlags &= ~MenuFlags::mfFlag4;
}

void EdenGame::affcurstape() {
	if (p_global->_drawFlags & DrawFlags::drDrawFlag8)
		_noPalette = true;
	useBank(65);
	noclipax(2, 0, 176);
	int x = (p_global->tape_ptr - tapes) * 8 + 97;
	_gameIcons[112].sx = x - 3;
	_gameIcons[112].ex = x + 3;
	noclipax(5, x, 179);
	_noPalette = false;
}

void EdenGame::forwardtape() {
	if (p_global->tape_ptr < tapes + 16) {
		p_global->tape_ptr++;
		p_global->menuFlags &= ~MenuFlags::mfFlag8;
		affcurstape();
	}
}

void EdenGame::stoptape() {
	if (!(p_global->_drawFlags & DrawFlags::drDrawFlag8))
		return;
	p_global->menuFlags &= ~MenuFlags::mfFlag8;
	p_global->_drawFlags &= ~DrawFlags::drDrawFlag8;
	p_global->menuFlags |= MenuFlags::mfFlag10;
	p_global->_iconsIndex = 85;
	p_global->_characterPtr = nullptr;
	lastTapeRoomNum = 0;
	endpersovox();
	fin_perso();
	affpanel();
	afftoppano();
	needPaletteUpdate = true;
}

void EdenGame::cliccurstape() {
	p_global->menuFlags |= MenuFlags::mfFlag4;
}

void EdenGame::paneltobuf() {
	setRS1(0, 16, 320 - 1, 169 - 1);
	setRD1(320, 16, 640 - 1, 169 - 1);
	CLBlitter_CopyViewRect(p_mainview, p_mainview, &rect_src, &rect_dst);
}

void EdenGame::cursbuftopanel() {
	setRS1(434, 40, 525 - 1, 111 - 1);
	setRD1(114, 40, 205 - 1, 111 - 1);
	CLBlitter_CopyViewRect(p_mainview, p_mainview, &rect_src, &rect_dst);
}

void EdenGame::langbuftopanel() {
	setRS1(328, 42, 407 - 1, 97 - 1);
	setRD1(8, 42,  87 - 1, 97 - 1);
	CLBlitter_CopyViewRect(p_mainview, p_mainview, &rect_src, &rect_dst);
}

void EdenGame::affpanel() {
	useBank(65);
	noclipax(0, 0, 16);
	paneltobuf();
	afflangue();
	affcurseurs();
	affcurstape();
}

void EdenGame::afflangue() {
	useBank(65);
	if (p_global->pref_language < 0 //TODO: never happens
	        || p_global->pref_language > 5)
		return;
	noclipax(6,  8, p_global->pref_language * 9 + 43);  //TODO: * FONT_HEIGHT
	noclipax(7, 77, p_global->pref_language * 9 + 44);
}

void EdenGame::affcursvol(int16 x, int16 vol1, int16 vol2) {
	int16 slider = 3;
	if (lastMenuItemIdLo && (lastMenuItemIdLo & 9) != 1) //TODO check me
		slider = 4;
	noclipax(slider, x, 104 - vol1);
	slider = 3;
	if ((lastMenuItemIdLo & 9) != 0)
		slider = 4;
	noclipax(slider, x + 12, 104 - vol2);
}

void EdenGame::affcurseurs() {
	useBank(65);
	if (p_global->_drawFlags & DrawFlags::drDrawFlag8)
		return;
	curseurselect(48);
	affcursvol(114, p_global->pref_10C[0] / 4, p_global->pref_10C[1] / 4);
	curseurselect(50);
	affcursvol(147, p_global->pref_10E[0] / 4, p_global->pref_10E[1] / 4);
	curseurselect(52);
	affcursvol(179, p_global->pref_110[0] / 4, p_global->pref_110[1] / 4);
}

void EdenGame::curseurselect(int itemId) {
	lastMenuItemIdLo = p_global->menuItemIdLo;
	if ((lastMenuItemIdLo & ~9) != itemId)
		lastMenuItemIdLo = 0;
}

void EdenGame::afftoppano() {
	noclipax(1, 0, 0);
}

void EdenGame::affresult() {
	restaurefondbulle();
	p_global->_characterPtr = &kPersons[19];
	p_global->_dialogType = DialogType::dtInspect;
	int16 num = (kPersons[19]._id << 3) | p_global->_dialogType;
	if (dialoscansvmas((dial_t *)getElem(gameDialogs, num)))
		af_subtitle();
	p_global->ff_CA = 0;
	p_global->_dialogType = DialogType::dtTalk;
	p_global->_characterPtr = nullptr;
}

void EdenGame::limitezonecurs(int16 xmin, int16 xmax, int16 ymin, int16 ymax) {
	curs_x = CLIP(curs_x, xmin, xmax);
	curs_y = CLIP(curs_y, ymin, ymax);
}

void EdenGame::PommeQ() {
	icon_t *icon = &_gameIcons[85];
	if (p_global->_displayFlags & DisplayFlags::dfFrescoes) {
		torchCursor = false;
		curs_saved = true;
		if (p_global->_displayFlags & DisplayFlags::dfPerson)
			close_perso();
		p_global->_displayFlags = DisplayFlags::dfFlag1;
		resetScroll();
		p_global->ff_100 = 0xFF;
		maj_salle(p_global->roomNum);
	}
	if (p_global->_displayFlags & DisplayFlags::dfPerson)
		close_perso();
	if (p_global->_displayFlags & DisplayFlags::dfPanable)
		resetScroll();
	if (p_global->_displayFlags & DisplayFlags::dfMirror)
		resetScroll();
	if (p_global->_drawFlags & DrawFlags::drDrawFlag8)
		stoptape();
	if (_personTalking)
		endpersovox();
	p_global->ff_103 = 0;
	p_global->ff_102 = 0;
	putObject();
	_currCursor = 53;
	if (p_global->_displayFlags != DisplayFlags::dfFlag2)
		gotopanel();
	current_spot2 = icon + 7;   //TODO
	EdenQuit();
}

void EdenGame::habitants(perso_t *perso) {
	char persType = perso->_flags & PersonFlags::pfTypeMask; //TODO rename
	if (persType && persType != PersonFlags::pfType2) {
		p_global->_roomCharacter = perso;
		p_global->_roomCharacterType = persType;
		p_global->_roomCharacterFlags = perso->_flags;
		p_global->_roomPersoItems = perso->_items;
		p_global->_roomCharacterPowers = perso->_powers;
		p_global->_partyOutside |= perso->_partyMask;
		if (p_global->_roomCharacterType == PersonFlags::pftTriceraptor)
			delinfo(p_global->_areaNum + ValleyNews::vnTriceraptorsIn);
		else if (p_global->_roomCharacterType == PersonFlags::pftVelociraptor)
			delinfo(p_global->_areaNum + ValleyNews::vnVelociraptorsIn);
	} else if (!(perso->_flags & PersonFlags::pfInParty))
		p_global->_partyOutside |= perso->_partyMask;
}

void EdenGame::suiveurs(perso_t *perso) {
	char persType = perso->_flags & PersonFlags::pfTypeMask;
	if (persType == 0 || persType == PersonFlags::pfType2) {
		if (perso->_flags & PersonFlags::pfInParty)
			p_global->party |= perso->_partyMask;
	}
}

void EdenGame::evenements(perso_t *perso) {
	if (p_global->ff_113)
		return;

	if (perso >= &kPersons[PER_UNKN_18C])
		return;

	if (!dialo_even(perso))
		return;

	p_global->ff_113++;
	p_global->oldDisplayFlags = 1;
	perso = p_global->_characterPtr;
	init_perso_ptr(perso);
	if (!(perso->_partyMask & PersonMask::pmLeader))
		p_global->ff_60 = true;
	p_global->_eventType = 0;
}

void EdenGame::followme(perso_t *perso) {
	if (perso->_flags & PersonFlags::pfTypeMask)
		return;
	if (perso->_flags & PersonFlags::pfInParty)
		perso->_roomNum = destinationRoom;
}

void EdenGame::rangermammi(perso_t *perso, room_t *room) {
	room_t *found_room = nullptr;
	if (!(perso->_partyMask & PersonMask::pmLeader))
		return;
	for (; room->ff_0 != 0xFF; room++) {
		if (room->flags & RoomFlags::rfHasCitadel) {
			found_room = room;
			break;
		}
		if (room->party != 0xFFFF && (room->party & PersonMask::pmLeader))
			found_room = room;  //TODO: no brk?
	}
	if (!found_room)
		return;
	perso->_roomNum &= ~0xFF;
	perso->_roomNum |= found_room->location;
	perso->_flags &= ~PersonFlags::pfInParty;
	p_global->party &= ~perso->_partyMask;
}

void EdenGame::perso_ici(int16 action) {
	perso_t *perso = &kPersons[PER_UNKN_156];
//	room_t *room = p_global->last_area_ptr->room_ptr;    //TODO: compiler opt bug? causes access to zero ptr??? last_area_ptr == 0
	switch (action) {
	case 0:
		suiveurs(perso);
		break;
	case 1:
		habitants(perso);
		break;
	case 3:
		evenements(perso);
		break;
	case 4:
		followme(perso);
		break;
	case 5:
		rangermammi(perso, p_global->last_area_ptr->citadelRoom);
		break;
	}
	perso = kPersons;
	do {
		if (perso->_roomNum == p_global->roomNum && !(perso->_flags & PersonFlags::pf80)) {
			switch (action) {
			case 0:
				suiveurs(perso);
				break;
			case 1:
				habitants(perso);
				break;
			case 3:
				evenements(perso);
				break;
			case 4:
				followme(perso);
				break;
			case 5:
				rangermammi(perso, p_global->last_area_ptr->citadelRoom);
				break;
			}
		}
		perso++;
	} while (perso->_roomNum != 0xFFFF);
}

void EdenGame::setpersohere() {
	debug("setpersohere, perso is %ld", p_global->_characterPtr - kPersons);
	p_global->_partyOutside = 0;
	p_global->party = 0;
	p_global->_roomCharacter = 0;
	p_global->_roomCharacterType = 0;
	p_global->_roomCharacterFlags = 0;
	perso_ici(1);
	perso_ici(0);
	if (p_global->_roomCharacterType == PersonFlags::pftTyrann) delinfo(p_global->_areaNum + ValleyNews::vnTyrannIn);
	if (p_global->_roomCharacterType == PersonFlags::pftTriceraptor) delinfo(p_global->_areaNum + ValleyNews::vnTriceraptorsIn);
	if (p_global->_roomCharacterType == PersonFlags::pftVelociraptor) {
		delinfo(p_global->_areaNum + ValleyNews::vnTyrannIn);
		delinfo(p_global->_areaNum + ValleyNews::vnTyrannLost);
		delinfo(p_global->_areaNum + ValleyNews::vnVelociraptorsLost);
	}
}

void EdenGame::faire_suivre(int16 roomNum) {
	destinationRoom = roomNum;
	perso_ici(4);
}

void EdenGame::suis_moi5() {
	debug("adding person %ld to party", p_global->_characterPtr - kPersons);
	p_global->_characterPtr->_flags |= PersonFlags::pfInParty;
	p_global->_characterPtr->_roomNum = p_global->roomNum;
	p_global->party |= p_global->_characterPtr->_partyMask;
	p_global->_drawFlags |= DrawFlags::drDrawTopScreen;
}

void EdenGame::suis_moi(int16 index) {
	perso_t *old_perso = p_global->_characterPtr;
	p_global->_characterPtr = &kPersons[index];
	suis_moi5();
	p_global->_characterPtr = old_perso;
}

void EdenGame::reste_ici5() {
	debug("removing person %ld from party", p_global->_characterPtr - kPersons);
	p_global->_characterPtr->_flags &= ~PersonFlags::pfInParty;
	p_global->_partyOutside |= p_global->_characterPtr->_partyMask;
	p_global->party &= ~p_global->_characterPtr->_partyMask;
	p_global->_drawFlags |= DrawFlags::drDrawTopScreen;
}

void EdenGame::reste_ici(int16 index) {
	perso_t *old_perso = p_global->_characterPtr;
	p_global->_characterPtr = &kPersons[index];
	reste_ici5();
	p_global->_characterPtr = old_perso;
}

void EdenGame::eloipart() {
	reste_ici(5);
	p_global->_gameFlags &= ~GameFlags::gfFlag4000;
	kPersons[PER_MESSAGER]._roomNum = 0;
	p_global->_partyOutside &= ~kPersons[PER_MESSAGER]._partyMask;
	if (p_global->roomNum == 2817)
		chronoon(3000);
	p_global->eloiDepartureDay = p_global->_gameDays;
	p_global->eloiHaveNews = 0;
	unlockinfo();
}

bool EdenGame::eloirevientq() {
	if (p_global->_phaseNum < 304)
		return true;
	if ((p_global->_phaseNum <= 353) || (p_global->_phaseNum == 370) || (p_global->_phaseNum == 384))
		return false;
	if (p_global->_areaNum != Areas::arShandovra)
		return true;
	if (p_global->_phaseNum < 480)
		return false;
	return true;
}

void EdenGame::eloirevient() {
	if (p_global->_areaPtr->type == AreaType::atValley && !kPersons[PER_MESSAGER]._roomNum)
		kPersons[PER_MESSAGER]._roomNum = (p_global->roomNum & 0xFF00) + 1;
}
//// phase.c
void EdenGame::incPhase1() {
	static phase_t phases[] = {
		{ 65, &EdenGame::dialautoon },
		{ 113, &EdenGame::phase113 },
		{ 129, &EdenGame::dialautoon },
		{ 130, &EdenGame::phase130 },
		{ 161, &EdenGame::phase161 },
		{ 211, &EdenGame::dialautoon },
		{ 226, &EdenGame::phase226 },
		{ 257, &EdenGame::phase257 },
		{ 353, &EdenGame::phase353 },
		{ 369, &EdenGame::phase369 },
		{ 371, &EdenGame::phase371 },
		{ 385, &EdenGame::phase385 },
		{ 386, &EdenGame::dialonfollow },
		{ 418, &EdenGame::phase418 },
		{ 433, &EdenGame::phase433 },
		{ 434, &EdenGame::phase434 },
		{ 449, &EdenGame::dialautoon },
		{ 497, &EdenGame::dialautoon },
		{ 513, &EdenGame::phase513 },
		{ 514, &EdenGame::phase514 },
		{ 529, &EdenGame::phase529 },
		{ 545, &EdenGame::phase545 },
		{ 561, &EdenGame::phase561 },
		{ -1, nullptr }
	};

	p_global->_phaseNum++;
	debug("!!! next phase - %4X , room %4X", p_global->_phaseNum, p_global->roomNum);
	p_global->phaseActionsCount = 0;
	for (phase_t *phase = phases; phase->_id != -1; phase++) {
		if (p_global->_phaseNum == phase->_id) {
			(this->*phase->disp)();
			break;
		}
	}
}

void EdenGame::incphase() {
	incPhase1();
}

void EdenGame::phase113() {
	reste_ici(1);
	kPersons[PER_DINA]._roomNum = 274;
}

void EdenGame::phase130() {
	dialautoon();
	reste_ici(3);
}

void EdenGame::phase161() {
	area_t *area = p_global->_areaPtr;
	suis_moi(9);
	kPersons[PER_MAMMI]._flags |= PersonFlags::pf10;
	area->_flags |= AreaFlags::afFlag1;
	p_global->_curAreaFlags |= AreaFlags::afFlag1;
}

void EdenGame::phase226() {
	newobject(16, 3);
	newobject(16, 4);
	newobject(16, 5);
}

void EdenGame::phase257() {
	_gameIcons[127]._cursorId &= ~0x8000;
	p_global->_characterBackgroundBankIdx = 58;
	dialautooff();
}

void EdenGame::phase353() {
	reste_ici(1);
	kPersons[PER_DINA]._roomNum = 0;
	kTabletView[1] = 88;
}

void EdenGame::phase369() {
	suis_moi(5);
	p_global->narratorSequence = 2;
	gameRooms[334].exits[0] = 134;
	gameRooms[335].exits[0] = 134;
}

void EdenGame::phase371() {
	eloirevient();
	_gameIcons[128]._cursorId &= ~0x8000;
	_gameIcons[129]._cursorId &= ~0x8000;
	_gameIcons[127]._cursorId |= 0x8000;
	p_global->_characterBackgroundBankIdx = 59;
	gameRooms[334].exits[0] = 0xFF;
	gameRooms[335].exits[0] = 0xFF;
	_gameIcons[123]._objectId = 9;
	_gameIcons[124]._objectId = 26;
	_gameIcons[125]._objectId = 42;
	_gameIcons[126]._objectId = 56;
}

void EdenGame::phase385() {
	dialautooff();
	eloirevient();
	p_global->next_info_idx = 0;
	p_global->last_info_idx = 0;
	updateinfolist();
	p_global->_lastInfo = 0;
}

void EdenGame::phase418() {
	loseObject(Objects::obHorn);
	dialautoon();
	suis_moi(4);
}

void EdenGame::phase433() {
	dialautoon();
	kPersons[PER_MAMMI_4]._flags &= ~PersonFlags::pf80;
	kPersons[PER_BOURREAU]._flags &= ~PersonFlags::pf80;
	setpersohere();
	p_global->_chronoFlag = 0;
	p_global->chrono = 0;
}

void EdenGame::phase434() {
	p_global->roomNum = 275;
	gameRooms[16].bank = 44;
	gameRooms[18].bank = 44;
	_gameIcons[132]._cursorId &= ~0x8000;
	p_global->_characterBackgroundBankIdx = 61;
	gameRooms[118].exits[2] = 0xFF;
	abortdial();
	gameRooms[7].bank = 322;
	reste_ici(7);
	reste_ici(3);
	reste_ici(5);
	reste_ici(18);
	reste_ici(4);
	p_global->_drawFlags |= DrawFlags::drDrawTopScreen;
}

void EdenGame::phase513() {
	p_global->last_dialog_ptr = nullptr;
	parlemoiNormalFlag = false;
	dialautoon();
}

void EdenGame::phase514() {
	gameRooms[123].exits[2] = 1;
}

void EdenGame::phase529() {
	_gameIcons[133]._cursorId &= ~0x8000;
	p_global->_characterBackgroundBankIdx = 63;
}

void EdenGame::phase545() {
}

void EdenGame::phase561() {
	p_global->narratorSequence = 10;
}

void EdenGame::bigphase1() {
	static void (EdenGame::*bigphases[])() = {
		&EdenGame::phase16,
		&EdenGame::phase32,
		&EdenGame::phase48,
		&EdenGame::phase64,
		&EdenGame::phase80,
		&EdenGame::phase96,
		&EdenGame::phase112,
		&EdenGame::phase128,
		&EdenGame::phase144,
		&EdenGame::phase160,
		&EdenGame::phase176,
		&EdenGame::phase192,
		&EdenGame::phase208,
		&EdenGame::phase224,
		&EdenGame::phase240,
		&EdenGame::phase256,
		&EdenGame::phase272,
		&EdenGame::phase288,
		&EdenGame::phase304,
		&EdenGame::phase320,
		&EdenGame::phase336,
		&EdenGame::phase352,
		&EdenGame::phase368,
		&EdenGame::phase384,
		&EdenGame::phase400,
		&EdenGame::phase416,
		&EdenGame::phase432,
		&EdenGame::phase448,
		&EdenGame::phase464,
		&EdenGame::phase480,
		&EdenGame::phase496,
		&EdenGame::phase512,
		&EdenGame::phase528,
		&EdenGame::phase544,
		&EdenGame::phase560
	};

	int16 phase = (p_global->_phaseNum & ~3) + 0x10;   //TODO: check me
	debug("!!! big phase - %4X", phase);
	p_global->phaseActionsCount = 0;
	p_global->_phaseNum = phase;
	if (phase > 560)
		return;
	phase >>= 4;
	(this->*bigphases[phase - 1])();
}

void EdenGame::bigphase() {
	if (!(p_global->_dialogPtr->_flags & DialogFlags::dfSpoken))
		bigphase1();
}

void EdenGame::phase16() {
	dialautoon();
}

void EdenGame::phase32() {
	word_31E7A &= ~0x8000;
}

void EdenGame::phase48() {
	gameRooms[8].exits[1] = 22;
	dialautoon();
}

void EdenGame::phase64() {
	suis_moi(1);
	kPersons[PER_MESSAGER]._roomNum = 259;
}

void EdenGame::phase80() {
	kPersons[PER_THOO]._roomNum = 0;
}

void EdenGame::phase96() {
}

void EdenGame::phase112() {
	giveObject();
}

void EdenGame::phase128() {
	suis_moi(1);
	giveObject();
}

void EdenGame::phase144() {
	suis_moi(5);
	gameRooms[113].video = 0;
	gameRooms[113].bank = 317;
}

void EdenGame::phase160() {
}

void EdenGame::phase176() {
	dialonfollow();
}

void EdenGame::phase192() {
	area_t *area = p_global->_areaPtr;
	suis_moi(10);
	kPersons[PER_MAMMI_1]._flags |= PersonFlags::pf10;
	dialautoon();
	area->_flags |= AreaFlags::afFlag1;
	p_global->_curAreaFlags |= AreaFlags::afFlag1;
}

void EdenGame::phase208() {
	eloirevient();
}

void EdenGame::phase224() {
	_gameIcons[126]._cursorId &= ~0x8000;
	p_global->_characterBackgroundBankIdx = 57;
	dialautooff();
}

void EdenGame::phase240() {
	area_t *area = p_global->_areaPtr;
	suis_moi(11);
	kPersons[PER_MAMMI_2]._flags |= PersonFlags::pf10;
	area->_flags |= AreaFlags::afFlag1;
	p_global->_curAreaFlags |= AreaFlags::afFlag1;
}

void EdenGame::phase256() {
	dialautoon();
}

void EdenGame::phase272() {
	dialautoon();
	p_global->eloiHaveNews = 0;
}

void EdenGame::phase288() {
	setChoiceYes();
	kPersons[PER_MANGO]._roomNum = 0;
	reste_ici(6);
	suis_moi(5);
	p_global->narratorSequence = 8;
}

void EdenGame::phase304() {
	area_t *area = p_global->_areaPtr;
	suis_moi(7);
	suis_moi(14);
	kPersons[PER_MAMMI_5]._flags |= PersonFlags::pf10;
	dialautoon();
	area->_flags |= AreaFlags::afFlag1;
	p_global->_curAreaFlags |= AreaFlags::afFlag1;
}

void EdenGame::phase320() {
	dialonfollow();
}

void EdenGame::phase336() {
	gameRooms[288].exits[0] = 135;
	gameRooms[289].exits[0] = 135;
	loseObject(p_global->_curObjectId);
	dialautoon();
}

void EdenGame::phase352() {
	kPersoRoomBankTable[30] = 26;
	kPersons[PER_EVE]._spriteBank = 9;
	kPersons[PER_EVE]._targetLoc = 8;
	followerList[13]._spriteNum = 2;
	dialautoon();
	gameRooms[288].exits[0] = 0xFF;
	gameRooms[289].exits[0] = 0xFF;
	gameRooms[288].flags &= ~RoomFlags::rf02;
	gameRooms[289].flags &= ~RoomFlags::rf02;
}

void EdenGame::phase368() {
	reste_ici(7);
	dialautoon();
	kPersons[PER_MESSAGER]._roomNum = 1811;
	kPersons[PER_DINA]._roomNum = 1607;
}

void EdenGame::phase384() {
	area_t *area = p_global->_areaPtr;
	suis_moi(7);
	reste_ici(1);
	dialautoon();
	area->_flags |= AreaFlags::afFlag1;
	p_global->_curAreaFlags |= AreaFlags::afFlag1;
	eloipart();
}

void EdenGame::phase400() {
	dialonfollow();
	kPersons[PER_ROI]._roomNum = 0;
	kPersons[PER_MONK]._roomNum = 259;
	p_global->eloiHaveNews = 0;
	kObjectLocations[20] = 259;
}

void EdenGame::phase416() {
	suis_moi(3);
	_gameIcons[130]._cursorId &= ~0x8000;
	p_global->_characterBackgroundBankIdx = 60;
	gameRooms[0].exits[0] = 138;
}

void EdenGame::phase432() {
	p_global->narratorSequence = 3;
	kPersons[PER_MAMMI_4]._flags |= PersonFlags::pf80;
	kPersons[PER_BOURREAU]._flags |= PersonFlags::pf80;
	kPersons[PER_MESSAGER]._roomNum = 257;
	gameRooms[0].exits[0] = 0xFF;
	p_global->_drawFlags |= DrawFlags::drDrawTopScreen;
}

void EdenGame::phase448() {
	dialautoon();
	eloipart();
}

void EdenGame::phase464() {
	p_global->_areaPtr->_flags |= AreaFlags::afFlag1;
	p_global->_curAreaFlags |= AreaFlags::afFlag1;
	kPersons[PER_MAMMI_6]._flags |= PersonFlags::pf10;
	suis_moi(8);
	p_global->cita_area_num = p_global->_areaNum;
	naitredino(8);
}

void EdenGame::phase480() {
	giveObject();
	newvallee();
	eloirevient();
	kTabletView[1] = 94;
}

void EdenGame::phase496() {
	dialautoon();
	p_global->last_dialog_ptr = nullptr;
	parlemoiNormalFlag = false;
}

void EdenGame::phase512() {
	reste_ici(3);
	reste_ici(7);
	reste_ici(8);
	reste_ici(18);
}

void EdenGame::phase528() {
	p_global->narratorSequence = 11;
	suis_moi(3);
	suis_moi(5);
	suis_moi(7);
	suis_moi(8);
	suis_moi(18);
}

void EdenGame::phase544() {
	eloipart();
	dialautoon();
	reste_ici(8);
	reste_ici(18);
}

void EdenGame::phase560() {
	kPersons[PER_MESSAGER]._roomNum = 3073;
	gameRooms[127].exits[1] = 0;
}

//// saveload.c
void EdenGame::savegame(char *name) {
//	filespec_t fs;
//	file_t handle;
	int32 size;
//	CLFile_MakeStruct(0, 0, name, &fs);
//	CLFile_Create(&fs);
//	CLFile_SetFinderInfos(&fs, 'EDNS', 'LEDN');
//	CLFile_Open(&fs, 3, handle);

	Common::OutSaveFile *handle = g_system->getSavefileManager()->openForSaving(name);
	if (!handle)
		return;

#define CLFile_Write(h, ptr, size) \
debug("writing 0x%X bytes", *size); \
h->write(ptr, *size);

	vavaoffsetout();
	size = (char *)(&p_global->save_end) - (char *)(p_global);
	CLFile_Write(handle, p_global, &size);
	size = (char *)(&_gameIcons[134]) - (char *)(&_gameIcons[123]);
	CLFile_Write(handle, &_gameIcons[123], &size);
	lieuoffsetout();
	size = (char *)(&kAreasTable[12]) - (char *)(&kAreasTable[0]);
	CLFile_Write(handle, &kAreasTable[0], &size);
	size = (char *)(&gameRooms[423]) - (char *)(&gameRooms[0]);
	CLFile_Write(handle, &gameRooms[0], &size);
	size = (char *)(&_objects[42]) - (char *)(&_objects[0]);
	CLFile_Write(handle, &_objects[0], &size);
	size = (char *)(&kObjectLocations[45]) - (char *)(&kObjectLocations[0]);
	CLFile_Write(handle, &kObjectLocations[0], &size);
	size = (char *)(&followerList[14]) - (char *)(&followerList[13]);
	CLFile_Write(handle, &followerList[13], &size);
	size = (char *)(&kPersons[55]) - (char *)(&kPersons[0]);
	CLFile_Write(handle, &kPersons[0], &size);
	bandeoffsetout();
	size = (char *)(&tapes[16]) - (char *)(&tapes[0]);
	CLFile_Write(handle, &tapes[0], &size);
	size = (char *)(&kTabletView[6]) - (char *)(&kTabletView[0]);
	CLFile_Write(handle, &kTabletView[0], &size);
	size = (char *)(&gameDialogs[10240]) - (char *)(&gameDialogs[0]); //TODO: const size 10240
	CLFile_Write(handle, &gameDialogs[0], &size);

	delete handle;

#undef CLFile_Write

//	CLFile_Close(handle);

	vavaoffsetin();
	lieuoffsetin();
	bandeoffsetin();

	debug("* Game saved to %s", name);
}

void EdenGame::loadrestart() {
	assert(0);  //TODO: this won't work atm - all snapshots are BE
	int32 offs = 0;
	int32 size;
	size = (char *)(&p_global->save_end) - (char *)(p_global);
	loadpartoffile(2495, p_global, offs, size);
	offs += size;
	vavaoffsetin();
	size = (char *)(&_gameIcons[134]) - (char *)(&_gameIcons[123]);
	loadpartoffile(2495, &_gameIcons[123], offs, size);
	offs += size;
	size = (char *)(&kAreasTable[12]) - (char *)(&kAreasTable[0]);
	loadpartoffile(2495, &kAreasTable[0], offs, size);
	offs += size;
	lieuoffsetin();
	size = (char *)(&gameRooms[423]) - (char *)(&gameRooms[0]);
	loadpartoffile(2495, &gameRooms[0], offs, size);
	offs += size;
	size = (char *)(&_objects[42]) - (char *)(&_objects[0]);
	loadpartoffile(2495,  &_objects[0], offs, size);
	offs += size;
	size = (char *)(&kObjectLocations[45]) - (char *)(&kObjectLocations[0]);
	loadpartoffile(2495,  &kObjectLocations[0], offs, size);
	offs += size;
	size = (char *)(&followerList[14]) - (char *)(&followerList[13]);
	loadpartoffile(2495,  &followerList[13], offs, size);
	offs += size;
	size = (char *)(&kPersons[55]) - (char *)(&kPersons[0]);
	loadpartoffile(2495,  &kPersons[0], offs, size);
	offs += size;
	size = (char *)(&tapes[16]) - (char *)(&tapes[0]);
	loadpartoffile(2495,  &tapes[0], offs, size);
	offs += size;
	bandeoffsetin();
	size = (char *)(&kTabletView[6]) - (char *)(&kTabletView[0]);
	loadpartoffile(2495, &kTabletView[0], offs, size);
	offs += size;
	size = (char *)(&gameDialogs[10240]) - (char *)(&gameDialogs[0]); //TODO: const size 10240
	loadpartoffile(2495,  &gameDialogs[0], offs, size);
	_gameLoaded = true;
}

void EdenGame::loadgame(char *name) {
//	filespec_t fs;
//	file_t handle;
//	CLFile_MakeStruct(0, 0, name, &fs);
//	CLFile_Open(&fs, 3, handle);

	Common::InSaveFile *handle = g_system->getSavefileManager()->openForLoading(name);
	if (!handle)
		return;

#define CLFile_Read(h, ptr, size) \
	h->read(ptr, *size);

	int32 size = (char *)(&p_global->save_end) - (char *)(p_global);
	CLFile_Read(handle, p_global, &size);
	vavaoffsetin();
	size = (char *)(&_gameIcons[134]) - (char *)(&_gameIcons[123]);
	CLFile_Read(handle, &_gameIcons[123], &size);
	size = (char *)(&kAreasTable[12]) - (char *)(&kAreasTable[0]);
	CLFile_Read(handle, &kAreasTable[0], &size);
	lieuoffsetin();
	size = (char *)(&gameRooms[423]) - (char *)(&gameRooms[0]);
	CLFile_Read(handle, &gameRooms[0], &size);
	size = (char *)(&_objects[42]) - (char *)(&_objects[0]);
	CLFile_Read(handle, &_objects[0], &size);
	size = (char *)(&kObjectLocations[45]) - (char *)(&kObjectLocations[0]);
	CLFile_Read(handle, &kObjectLocations[0], &size);
	size = (char *)(&followerList[14]) - (char *)(&followerList[13]);
	CLFile_Read(handle, &followerList[13], &size);
	size = (char *)(&kPersons[55]) - (char *)(&kPersons[0]);
	CLFile_Read(handle, &kPersons[0], &size);
	size = (char *)(&tapes[16]) - (char *)(&tapes[0]);
	CLFile_Read(handle, &tapes[0], &size);
	bandeoffsetin();
	size = (char *)(&kTabletView[6]) - (char *)(&kTabletView[0]);
	CLFile_Read(handle, &kTabletView[0], &size);
	size = (char *)(&gameDialogs[10240]) - (char *)(&gameDialogs[0]); //TODO: const size 10240
	CLFile_Read(handle, &gameDialogs[0], &size);

	delete handle;
#undef CLFile_Read

//	CLFile_Close(handle);
	_gameLoaded = true;
	debug("* Game loaded from %s", name);
}

#define NULLPTR (void*)0xFFFFFF
#define OFSOUT(val, base, typ) if (val)      (val) = (typ*)((char*)(val) - (size_t)(base)); else (val) = (typ*)NULLPTR;
#define OFSIN(val, base, typ) if ((void*)(val) != NULLPTR)   (val) = (typ*)((char*)(val) + (size_t)(base)); else (val) = 0;

void EdenGame::vavaoffsetout() {
	OFSOUT(p_global->_dialogPtr, gameDialogs, dial_t);
	OFSOUT(p_global->_nextDialogPtr, gameDialogs, dial_t);
	OFSOUT(p_global->narrator_dialog_ptr, gameDialogs, dial_t);
	OFSOUT(p_global->last_dialog_ptr, gameDialogs, dial_t);
	OFSOUT(p_global->tape_ptr, tapes, tape_t);
	OFSOUT(p_global->nextRoomIcon, _gameIcons, icon_t);
	OFSOUT(p_global->room_ptr, gameRooms, room_t);
	OFSOUT(p_global->cita_area_firstRoom, gameRooms, room_t);
	OFSOUT(p_global->_areaPtr, kAreasTable, area_t);
	OFSOUT(p_global->last_area_ptr, kAreasTable, area_t);
	OFSOUT(p_global->cur_area_ptr, kAreasTable, area_t);
	OFSOUT(p_global->_characterPtr, kPersons, perso_t);
	OFSOUT(p_global->_roomCharacter, kPersons, perso_t);
}

void EdenGame::vavaoffsetin() {
	OFSIN(p_global->_dialogPtr, gameDialogs, dial_t);
	OFSIN(p_global->_nextDialogPtr, gameDialogs, dial_t);
	OFSIN(p_global->narrator_dialog_ptr, gameDialogs, dial_t);
	OFSIN(p_global->last_dialog_ptr, gameDialogs, dial_t);
	OFSIN(p_global->tape_ptr, tapes, tape_t);
	OFSIN(p_global->nextRoomIcon, _gameIcons, icon_t);
	OFSIN(p_global->room_ptr, gameRooms, room_t);
	OFSIN(p_global->cita_area_firstRoom, gameRooms, room_t);
	OFSIN(p_global->_areaPtr, kAreasTable, area_t);
	OFSIN(p_global->last_area_ptr, kAreasTable, area_t);
	OFSIN(p_global->cur_area_ptr, kAreasTable, area_t);
	OFSIN(p_global->_characterPtr, kPersons, perso_t);
	OFSIN(p_global->_roomCharacter, kPersons, perso_t);
}

void EdenGame::lieuoffsetout() {
	for (int i = 0; i < 12; i++)
		OFSOUT(kAreasTable[i].citadelRoom, gameRooms, room_t);
}

void EdenGame::lieuoffsetin() {
	for (int i = 0; i < 12; i++)
		OFSIN(kAreasTable[i].citadelRoom, gameRooms, room_t);
}

void EdenGame::bandeoffsetout() {
	for (int i = 0; i < 16; i++) {
		OFSOUT(tapes[i]._perso, kPersons, perso_t);
		OFSOUT(tapes[i]._dialog, gameDialogs, dial_t);
	}
}

void EdenGame::bandeoffsetin() {
	for (int i = 0; i < 16; i++) {
		OFSIN(tapes[i]._perso, kPersons, perso_t);
		OFSIN(tapes[i]._dialog, gameDialogs, dial_t);
	}
}

//// cond.c

byte *code_ptr;

char EdenGame::testcondition(int16 index) {
	char end = 0;
	byte op;
	uint16 value, value2;
	uint16 stack[32], *sp = stack, *sp2;
	assert(index > 0);
	code_ptr = (byte *)getElem(gameConditions, (index - 1));
	do {
		value = cher_valeur();
		for (;;) {
			op = *code_ptr++;
			if (op == 0xFF) {
				end = 1;
				break;
			}
			if ((op & 0x80) == 0) {
				value2 = cher_valeur();
				value = operation(op, value, value2);
			} else {
				assert(sp < stack + 32);
				*sp++ = value;
				*sp++ = op;
				break;
			}
		}
	} while (!end);

	if (sp != stack) {
		*sp++ = value;
		sp2 = stack;
		value = *sp2++;
		do {
			op = *sp2++;
			value2 = *sp2++;
			value = operation(op, value, value2);
		} while (sp2 != sp);
	}
//	if (value)
	debug("cond %d(-1) returns %s", index, value ? "TRUE" : "false");
//	if (index == 402) debug("(glob_61.b == %X) & (glob_12.w == %X) & (glob_4C.b == %X) & (glob_4E.b == %X)", p_global->eventType, p_global->phaseNum, p_global->worldTyrannSighted, p_global->ff_4E);
	return value != 0;
}

uint16 EdenGame::opera_add(uint16 v1, uint16 v2)  {
	return v1 + v2;
}

uint16 EdenGame::opera_sub(uint16 v1, uint16 v2)  {
	return v1 - v2;
}

uint16 EdenGame::opera_and(uint16 v1, uint16 v2)  {
	return v1 & v2;
}
uint16 EdenGame::opera_or(uint16 v1, uint16 v2)   {
	return v1 | v2;
}

uint16 EdenGame::opera_egal(uint16 v1, uint16 v2)     {
	return v1 == v2 ? -1 : 0;
}

uint16 EdenGame::opera_petit(uint16 v1, uint16 v2)    {
	return v1 < v2 ? -1 : 0;    //TODO: all comparisons are unsigned!
}

uint16 EdenGame::opera_grand(uint16 v1, uint16 v2)    {
	return v1 > v2 ? -1 : 0;
}

uint16 EdenGame::opera_diff(uint16 v1, uint16 v2)     {
	return v1 != v2 ? -1 : 0;
}

uint16 EdenGame::opera_petega(uint16 v1, uint16 v2)   {
	return v1 <= v2 ? -1 : 0;
}

uint16 EdenGame::opera_graega(uint16 v1, uint16 v2)   {
	return v1 >= v2 ? -1 : 0;
}

uint16 EdenGame::opera_faux(uint16 v1, uint16 v2)     {
	return 0;
}

uint16 EdenGame::operation(byte op, uint16 v1, uint16 v2) {
	static uint16(EdenGame::*operations[16])(uint16, uint16) = {
		&EdenGame::opera_egal,
		&EdenGame::opera_petit,
		&EdenGame::opera_grand,
		&EdenGame::opera_diff,
		&EdenGame::opera_petega,
		&EdenGame::opera_graega,
		&EdenGame::opera_add,
		&EdenGame::opera_sub,
		&EdenGame::opera_and,
		&EdenGame::opera_or,
		&EdenGame::opera_faux,
		&EdenGame::opera_faux,
		&EdenGame::opera_faux,
		&EdenGame::opera_faux,
		&EdenGame::opera_faux,
		&EdenGame::opera_faux
	};
	return (this->*operations[(op & 0x1F) >> 1])(v1, v2);
}

uint16 EdenGame::cher_valeur() {
	uint16 val;
	byte typ = *code_ptr++;
	if (typ < 0x80) {
		byte ofs = *code_ptr++;
		if (typ == 1)
			val = *(byte *)(ofs + (byte *)p_global);
		else
			val = *(uint16 *)(ofs + (byte *)p_global);
	} else if (typ == 0x80)
		val = *code_ptr++;
	else {
		val = READ_LE_UINT16(code_ptr);
		code_ptr += 2;
	}
	return val;
}

void EdenGame::ret() {
}

//// cube.c
void EdenGame::make_tabcos() {
	for (int i = 0; i < 361; i++) {
		tabcos[i * 2] = (int)(cos(3.1416 * i / 180.0) * 255.0);
		tabcos[i * 2 + 1] = (int)(sin(3.1416 * i / 180.0) * 255.0);
	}
}

void EdenGame::make_matrice_fix() {
	int16 r30 = word_3244C;
	int16 r28 = word_3244A;
	int16 r29 = word_32448;

	dword_32424 = (tabcos[r29 * 2] * tabcos[r28 * 2]) >> 8;
	dword_32430 = (tabcos[r29 * 2 + 1] * tabcos[r28 * 2]) >> 8;
	dword_3243C = -tabcos[r28 * 2 + 1];
	dword_32428 = ((-tabcos[r29 * 2 + 1] * tabcos[r30 * 2]) >> 8)
	              + ((tabcos[r30 * 2 + 1] * ((tabcos[r29 * 2] * tabcos[r28 * 2 + 1]) >> 8)) >> 8);
	dword_32434 = ((tabcos[r29 * 2] * tabcos[r30 * 2]) >> 8)
	              + ((tabcos[r30 * 2 + 1] * ((tabcos[r29 * 2 + 1] * tabcos[r28 * 2 + 1]) >> 8)) >> 8);
	dword_32440 = (tabcos[r28 * 2] * tabcos[r30 * 2 + 1]) >> 8;
	dword_3242C = ((tabcos[r29 * 2 + 1] * tabcos[r30 * 2 + 1]) >> 8)
	              + ((tabcos[r30 * 2] * ((tabcos[r29 * 2] * tabcos[r28 * 2 + 1]) >> 8)) >> 8);
	dword_32438 = ((-tabcos[r29 * 2] * tabcos[r30 * 2 + 1]) >> 8)
	              + ((tabcos[r30 * 2] * ((tabcos[r29 * 2 + 1] * tabcos[r28 * 2 + 1]) >> 8)) >> 8);
	dword_32444 = (tabcos[r28 * 2] * tabcos[r30 * 2]) >> 8;
}

void EdenGame::projection_fix(cube_t *cubep, int n) {
	for (int i = 0; i < n; i++) {
		int r28 = cubep->vertices[i * 4];
		int r27 = cubep->vertices[i * 4 + 1];
		int r26 = cubep->vertices[i * 4 + 2];

		int r25 = dword_32424 * r28 + dword_32428 * r27 + dword_3242C * r26 + (int)(flt_32454 * 256.0f);
		int r24 = dword_32430 * r28 + dword_32434 * r27 + dword_32438 * r26 + (int)(flt_32450 * 256.0f);
		int r29 = dword_3243C * r28 + dword_32440 * r27 + dword_32444 * r26 + (int)(flt_2DF7C * 256.0f);

		r29 >>= 8;
		if (r29 == -256)
			r29++;
		cubep->projection[i * 4    ] = r25 / (r29 + 256) + curs_x + 14 + _scrollPos;
		cubep->projection[i * 4 + 1] = r24 / (r29 + 256) + curs_y + 14;
		cubep->projection[i * 4 + 2] = r29;

//		assert(cube->projection[i * 4] < 640);
//		assert(cube->projection[i * 4 + 1] < 200);
	}
}

// Original name init_cube
void EdenGame::initCubeMac() {
	NEWcharge_map(2493, cube_texture);
	NEWcharge_objet_mob(&cube, 2494, cube_texture);
	make_tabcos();
}

void EdenGame::moteur() {
	Eden_dep_and_rot();
	make_matrice_fix();
	projection_fix(&cube, cube_faces);
	affiche_objet(&cube);
}

void EdenGame::affiche_objet(cube_t *cubep) {
	for (int i = 0; i < cubep->num; i++)
		affiche_polygone_mapping(cubep, cubep->faces[i]);
}

void EdenGame::NEWcharge_map(int file_id, byte *buffer) {
	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		loadpartoffile(file_id, buffer, 32, 256 * 3);

		for (int i = 0; i < 256; i++) {
			color3_t color;
			color.r = buffer[i * 3] << 8;
			color.g = buffer[i * 3 + 1] << 8;
			color.b = buffer[i * 3 + 2] << 8;
			CLPalette_SetRGBColor(global_palette, i, &color);
		}
		CLPalette_Send2Screen(global_palette, 0, 256);

		loadpartoffile(file_id, buffer, 32 + 256 * 3, 0x4000);
	} else {
#if 0
// Fake Mac cursor on PC
			Common::File f;
			if (f.open("curs.raw")) {
				f.seek(32);
				f.read(buffer, 256 * 3);

				for (i = 0; i < 256; i++) {
					color3_t color;
					color.r = buffer[i * 3] << 8;
					color.g = buffer[i * 3 + 1] << 8;
					color.b = buffer[i * 3 + 2] << 8;
					CLPalette_SetRGBColor(global_palette, i, &color);
				}
				CLPalette_Send2Screen(global_palette, 0, 256);

				f.read(buffer, 0x4000);

				f.close();
			}
			else
				error("can not load cursor texture");
#endif
	}
}

void EdenGame::NEWcharge_objet_mob(cube_t *cubep, int file_id, byte *texptr) {
	char *tmp1 = (char *)malloc(454);
	if (_vm->getPlatform() == Common::kPlatformMacintosh)
		loadpartoffile(file_id, tmp1, 0, 454);
	else {
#if 0
// Fake Mac cursor on PC
		Common::File f;
		if (f.open("curseden.mob")) {
			f.read(tmp1, 454);
			f.close();
		}
		else
			::error("can not load cursor model");
#endif
	}

	char *next = tmp1;
	char error;
	cube_faces = next_val(&next, &error);
	int16 *vertices = (int16 *)malloc(cube_faces * 4 * sizeof(*vertices));
	int16 *projection = (int16 *)malloc(cube_faces * 4 * sizeof(*projection));
	for (int i = 0; i < cube_faces; i++) {
		vertices[i * 4] = next_val(&next, &error);
		vertices[i * 4 + 1] = next_val(&next, &error);
		vertices[i * 4 + 2] = next_val(&next, &error);
	}
	int count2 = next_val(&next, &error);
	cubeface_t **tmp4 = (cubeface_t **)malloc(count2 * sizeof(*tmp4));
	for (int i = 0; i < count2; i++) {
		char textured;
		tmp4[i] = (cubeface_t *)malloc(sizeof(cubeface_t));
		tmp4[i]->tri = 3;
		textured = next_val(&next, &error);
		tmp4[i]->ff_5 = next_val(&next, &error);
		tmp4[i]->indices = (uint16 *)malloc(3 * sizeof(*tmp4[i]->indices));
		tmp4[i]->uv = (int16 *)malloc(3 * 2 * sizeof(*tmp4[i]->uv));
		for (int j = 0; j < 3; j++) {
			tmp4[i]->indices[j] = next_val(&next, &error);
			if (textured) {
				tmp4[i]->uv[j * 2] = next_val(&next, &error);
				tmp4[i]->uv[j * 2 + 1] = next_val(&next, &error);
			}
		}
		if (textured) {
			tmp4[i]->ff_4 = 3;
			tmp4[i]->texptr = texptr;
		} else
			tmp4[i]->ff_4 = 0;
	}
	free(tmp1);
	cubep->num = count2;
	cubep->faces = tmp4;
	cubep->projection = projection;
	cubep->vertices = vertices;
}

int EdenGame::next_val(char **ptr, char *error) {
	char c = 0;
	char *p = *ptr;
	int val = strtol(p, 0, 10);
	while ((*p >= '0' && *p <= '9' && *p != 0) || *p == '-')
		p++;
	while ((*p == 13 || *p == 10 || *p == ',' || *p == ' ') && *p)
		c = *p++;
	*error = c == 10;
	*ptr = p;
	return val;
}

void EdenGame::selectmap(int16 num) {
	static const char mapMode[12] = { 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 2, 0 };
	// Cube faces to texture coords mapping
	// each entry is num_polys(6) * num_faces_per_poly(2) * vertex_per_face(3) * uv(2)

	static const int16 cube_texcoords[3][6 * 2 * 3 * 2] = {
		{
			32, 32,  0, 32,  0,  0,
			32, 32,  0,  0, 32,  0,

			0, 32,  0,  0, 32,  0,
			0, 32, 32,  0, 32, 32,

			32, 32,  0, 32,  0,  0,
			32, 32,  0,  0, 32,  0,

			32,  0, 32, 32,  0, 32,
			32,  0,  0, 32,  0,  0,

			0,  0, 32,  0, 32, 32,
			0,  0, 32, 32,  0, 32,

			0, 32,  0,  0, 32,  0,
			0, 32, 32,  0, 32, 32
		}, {
			32, 32,  0, 32,  0,  0,
			32, 32,  0,  0, 32,  0,

			32,  0, 32, 32,  0, 32,
			32,  0,  0, 32,  0,  0,

			32,  0, 32, 32,  0, 32,
			32,  0,  0, 32,  0,  0,

			0, 32,  0,  0, 32,  0,
			0, 32, 32,  0, 32, 32,

			32,  0, 32, 32,  0, 32,
			32,  0,  0, 32,  0,  0,

			32,  0, 32, 32,  0, 32,
			32,  0,  0, 32,  0,  0
		}, {
			30, 30,  2, 30,  2,  2,
			30, 30,  2,  2, 30,  2,

			2, 30,  2,  2, 30,  2,
			2, 30, 30,  2, 30, 30,

			30, 30,  2, 30,  2,  2,
			30, 30,  2,  2, 30,  2,

			30,  2, 30, 30,  2, 30,
			30,  2,  2, 30,  2,  2,

			2,  2, 30,  2, 30, 30,
			2,  2, 30, 30,  2, 30,

			2, 30,  2,  2, 30,  2,
			2, 30, 30,  2, 30, 30
		}
	};

	curs_cur_map = num;
	int16 k = 0;
	int mode = mapMode[num];
	int16 x = (num & 7) * 32;
	int16 y = (num & 0x18) * 4;
	for (int i = 0; i < 6 * 2; i++) {
		for (int j = 0; j < 3; j++) {
			cube.faces[i]->uv[j * 2    ] = x + cube_texcoords[mode][k++];
			cube.faces[i]->uv[j * 2 + 1] = y + cube_texcoords[mode][k++];
		}
	}
}

void EdenGame::Eden_dep_and_rot() {
	int16 curs = _currCursor;
	if (normalCursor && (p_global->_drawFlags & DrawFlags::drDrawFlag20))
		curs = 10;
	selectmap(curs);
	curs_new_tick = TickCount();
	if (curs_new_tick - curs_old_tick < 1)
		return;
	curs_old_tick = curs_new_tick;
	switch (_currCursor) {
	case 0:
		word_3244C = (word_3244C + 2) % 360;
		word_3244A = (word_3244A + 2) % 360;
		restoreZDEP();
		break;
	case 1:
		word_3244C = 0;
		word_3244A -= 2;
		if (word_3244A < 0)
			word_3244A += 360;
		restoreZDEP();
		break;
	case 2:
		word_3244C = (word_3244C + 2) % 360;
		word_3244A = 0;
		restoreZDEP();
		break;
	case 3:
		word_3244C -= 2;
		if (word_3244C < 0)
			word_3244C += 360;
		word_3244A = 0;
		restoreZDEP();
		break;
	case 4:
		word_3244C = 0;
		word_3244A = (word_3244A + 2) % 360;
		restoreZDEP();
		break;
	case 5:
		word_3244C = 0;
		word_3244A = 0;
		flt_2DF7C += flt_2DF84;
		if ((flt_2DF7C < -3600.0 + flt_2DF80) || flt_2DF7C > flt_2DF80)
			flt_2DF84 = -flt_2DF84;
		break;
	case 6:
		word_3244C = 0;
		word_3244A = 0;
		flt_2DF7C = flt_2DF80;
		break;
	case 7:
		word_3244C -= 2;
		if (word_3244C < 0)
			word_3244C += 360;
		word_3244A = 0;
		restoreZDEP();
		break;
	case 8:
		word_3244C = 0;
		word_3244A = 0;
		flt_2DF7C = flt_2DF80;
		break;
	case 9:
		word_3244C = 0;
		word_3244A = 0;
		flt_2DF7C = flt_2DF80;
		break;
	}
}

void EdenGame::restoreZDEP() {
	flt_2DF84 = 200.0;
	if (flt_2DF7C < flt_2DF80)
		flt_2DF7C += flt_2DF84;
	if (flt_2DF7C > flt_2DF80)
		flt_2DF7C -= flt_2DF84;
}

void EdenGame::affiche_polygone_mapping(cube_t *cubep, cubeface_t *face) {
	uint16 *indices = face->indices;
	int idx = indices[0] * 4;
	int16 v46 = cubep->projection[idx];
	int16 v48 = cubep->projection[idx + 1];

	idx = indices[1] * 4;
	int16 v4A = cubep->projection[idx];
	int16 v4C = cubep->projection[idx + 1];

	idx = indices[2] * 4;
	int16 v4E = cubep->projection[idx];
	int16 v50 = cubep->projection[idx + 1];

	if ((v4C - v48) * (v4E - v46) - (v50 - v48) * (v4A - v46) > 0)
		return;

	int16 *uv = face->uv;
	int16 ymin = 200; // min y
	int16 ymax = 0;   // max y
	idx = indices[0] * 4;
	int16 r20 = cubep->projection[idx];
	int16 r30 = cubep->projection[idx + 1];
	int16 r19 = *uv++;
	int16 r18 = *uv++;
	indices++;
	for (int r17 = 0; r17 < face->tri - 1; r17++, indices++) {
		idx = indices[0] * 4;
		int16 r26 = cubep->projection[idx];
		int16 r31 = cubep->projection[idx + 1];
		uint16 r25 = *uv++;    //TODO: unsigned
		int16 r24 = *uv++;    //TODO: unsigned
		if (r30 < ymin)
			ymin = r30;
		if (r30 > ymax)
			ymax = r30;
		if (r31 < ymin)
			ymin = r31;
		if (r31 > ymax)
			ymax = r31;
		trace_ligne_mapping(r20, r30, r26, r31, r19, r18, r25, r24, _lines);
		r20 = r26;
		r30 = r31;
		r19 = r25;
		r18 = r24;
	}
	idx = face->indices[0] * 4;
	int16 r26 = cubep->projection[idx];
	int16 r31 = cubep->projection[idx + 1];
	uv = face->uv;
	uint16 r25 = *uv++;   //TODO: this is unsigned
	int16 r24 = *uv;      //TODO: this is signed
	if (r30 < ymin)
		ymin = r30;
	if (r30 > ymax)
		ymax = r30;
	if (r31 < ymin)
		ymin = r31;
	if (r31 > ymax)
		ymax = r31;
	trace_ligne_mapping(r20, r30, r26, r31, r19, r18, r25, r24, _lines);
	affiche_ligne_mapping(ymin, ymax, p_mainview->_bufferPtr, face->texptr);
}

void EdenGame::trace_ligne_mapping(int16 r3, int16 r4, int16 r5, int16 r6, int16 r7, int16 r8, int16 r9, int16 r10, int16 *linesp) {
	int16 r26 = r6 - r4;
	if (r26 <= 0) {
		if (r26 == 0) {
			linesp += r4 * 8;
			if (r5 - r3 > 0) {
				linesp[0] = r3;
				linesp[1] = r5;
				linesp[4] = r7;
				linesp[5] = r9;
				linesp[6] = r8;
				linesp[7] = r10;
			} else {
				linesp[0] = r5;
				linesp[1] = r3;
				linesp[4] = r9;
				linesp[5] = r7;
				linesp[6] = r10;
				linesp[7] = r8;
			}
			return;
		}
		int16 t = r3;
		r3 = r5;
		r5 = t;
		t = r7;
		r7 = r9;
		r9 = t;
		t = r8;
		r8 = r10;
		r10 = t;
		linesp += r6 * 8;
		r26 = -r26;
	} else
		linesp += r4 * 8 + 1;    //TODO wha???

	int r30 = r3 << 16;
	int r29 = r7 << 16;
	int r28 = r8 << 16;

	int r25 = ((r5 - r3) << 16) / r26;
	int r24 = ((r9 - r7) << 16) / r26;
	int r23 = ((r10 - r8) << 16) / r26;

	for (int i = 0; i < r26; i++) {
		linesp[0] = r30 >> 16;
		linesp[4] = r29 >> 16;
		linesp[6] = r28 >> 16;

		r30 += r25;
		r29 += r24;
		r28 += r23;
		linesp += 8;
	}
}

void EdenGame::affiche_ligne_mapping(int16 r3, int16 r4, byte *target, byte *texture) {
	int16 height = r4 - r3;
	byte *trg_line = p_mainview->_bufferPtr + r3 * 640;    //TODO: target??
	int16 *line = &_lines[r3 * 8];
	//	debug("curs: beg draw %d - %d", r3, r4);
	for (int r22 = height; r22; r22--, line += 8, trg_line += 640) {
		int16 r29 = line[0];
		int16 r28 = line[1];
		int16 len = r28 - r29;
		if (len < 0)
			break;
		if (len == 0)
			continue;

		//	debug("curs: lin draw %d", r4 - height);
		uint16 r31 = line[4] << 8;
		uint16 r30 = line[6] << 8;

		int16 r21 = line[5] - line[4];
		int16 r20 = line[7] - line[6];

		int16 r26 = (r21 << 8) / len;
		int16 r25 = (r20 << 8) / len;
		byte *trg = trg_line + r29;
#if 1
		while (r29++ < r28) {
			*trg++ = texture[(r30 & 0xFF00) | (r31 >> 8)];
			r31 += r26;
			r30 += r25;
		}
#endif
	}
}

// PC cursor
cubeCursor pc_cursors[9] = {
		{ { 0, 0, 0, 0, 0, 0 }, 3, 2 },
		{ { 1, 1, 0, 1, 1, 0 }, 2, -2 },
		{ { 2, 2, 2, 2, 2, 2 }, 1, 2 },
		{ { 3, 3, 3, 3, 3, 3 }, 1, -2 },
		{ { 4, 4, 4, 4, 4, 4 }, 2, 2 },
		{ { 5, 5, 5, 5, 5, 5 }, 4, 0 },
		{ { 6, 6, 6, 6, 6, 6 }, 1, 2 },
		{ { 7, 7, 7, 7, 7, 7 }, 1, -2 },
//		{ { 0, 8, 0, 0, 8, 8 }, 2, 2 },
		{ { 0, 8, 0, 0, 8, 8 }, 2, 2 }
};

XYZ pc_cube[6][3] = {
		{ { -15, -15, -15 }, { -15, 15, -15 }, { 15, 15, -15 } },
		{ { -15, -15, 15 }, { -15, 15, 15 }, { -15, 15, -15 } },
		{ { -15, -15, 15 }, { -15, -15, -15 }, { 15, -15, -15 } },
		{ { 15, -15, 15 }, { 15, 15, 15 }, { -15, 15, 15 } },
		{ { 15, -15, -15 }, { 15, 15, -15 }, { 15, 15, 15 } },
		{ { 15, 15, 15 }, { 15, 15, -15 }, { -15, 15, -15 } }
};

signed short cosine[] = {
	// = cos(n) << 7; n += 10;
	128, 126, 120, 111, 98, 82, 64, 44, 22, 0, -22, -44, -64, -82, -98, -111, -120, -126,
	-128, -126, -120, -111, -98, -82, -64, -44, -22, 0, 22, 44, 64, 82, 98, 111, 120, 126,
	128, 126, 120, 111, 98, 82, 64, 44, 22, 0
};

void EdenGame::MakeTables() {
	for (int i = -15; i < 15; i++) {
		int v = (i * 11) / 15 + 11;
		tab1[i + 15] = v;
		tab2[i + 15] = v * 22;
	}

	for (int i = 0; i < 36; i++) {
		for (int j = -35; j < 36; j++)
			tab3[i][j + 35] = (cosine[i] * j) >> 7;
	}
}

void EdenGame::GetSinCosTables(unsigned short angle, signed char **cos_table, signed char **sin_table) {
	angle /= 2;
	*cos_table = tab3[angle] + 35;

	angle += 9;
	if (angle >= 36)
		angle -= 36;

	*sin_table = tab3[angle] + 35;
}


void EdenGame::RotatePoint(XYZ *point, XYZ *rpoint) {
	// see http://www.cprogramming.com/tutorial/3d/rotation.html
	XYZ xrot;

	xrot.x = point->x;
	xrot.y = cos_x[point->y] + sin_x[point->z];
	xrot.z = sin_x[-point->y] + cos_x[point->z];

	rpoint->x = cos_y[xrot.x] + sin_y[-xrot.z];
	rpoint->y = xrot.y;
	rpoint->z = sin_y[xrot.x] + cos_y[xrot.z];

	rpoint->z += zoom;
}

void EdenGame::MapPoint(XYZ *point, short *x, short *y) {
	*y = ((12800 / point->z) * point->y) >> 7;
	*x = ((12800 / point->z) * point->x) >> 7;
}

short EdenGame::CalcFaceArea(XYZ *face) {
	XYZ rpoint;
	short x[3], y[3];

	for (int i = 0; i < 3; i++) {
		RotatePoint(&face[i], &rpoint);
		MapPoint(&rpoint, &x[i], &y[i]);
	}

	short area = (y[1] - y[0]) * (x[2] - x[0]) - (y[2] - y[0]) * (x[1] - x[0]);

	return area;
}

void EdenGame::PaintPixel(XYZ *point, unsigned char pixel) {
	short x, y;
	MapPoint(point, &x, &y);
	cursorcenter[y * 40 + x] = pixel;
}

void EdenGame::PaintFace0(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->x = x;
			point->y = y;
			RotatePoint(point, &rpoint);
			PaintPixel(&rpoint, _face[0][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::PaintFace1(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->y = y;
			point->z = -x;
			RotatePoint(point, &rpoint);
			PaintPixel(&rpoint, _face[1][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::PaintFace2(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->x = x;
			point->z = -y;
			RotatePoint(point, &rpoint);
			PaintPixel(&rpoint, _face[2][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::PaintFace3(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->x = -x;
			point->y = -y;
			RotatePoint(point, &rpoint);
			PaintPixel(&rpoint, _face[3][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::PaintFace4(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->y = y;
			point->z = x;
			RotatePoint(point, &rpoint);
			PaintPixel(&rpoint, _face[4][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::PaintFace5(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->x = x;
			point->z = y;
			RotatePoint(point, &rpoint);
			PaintPixel(&rpoint, _face[5][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::PaintFaces() {
	XYZ point;
	if (!(faceskip & 1)) {
		point.z = -15;
		PaintFace0(&point);
	}
	if (!(faceskip & 2)) {
		point.x = -15;
		PaintFace1(&point);
	}
	if (!(faceskip & 4)) {
		point.y = -15;
		PaintFace2(&point);
	}
	if (!(faceskip & 8)) {
		point.z = 15;
		PaintFace3(&point);
	}
	if (!(faceskip & 16)) {
		point.x = 15;
		PaintFace4(&point);
	}
	if (!(faceskip & 32)) {
		point.y = 15;
		PaintFace5(&point);
	}
}

void EdenGame::RenderCube() {
	for (int i = 0; i < sizeof(cursor); i++)
		cursor[i] = 0;
	cursorcenter = &cursor[40 * 20 + 20];

	GetSinCosTables(angle_x, &cos_x, &sin_x);
	GetSinCosTables(angle_y, &cos_y, &sin_y);
	GetSinCosTables(angle_z, &cos_z, &sin_z);

	for (int i = 0; i < 6; i++) {
		int area = CalcFaceArea(pc_cube[i]);
		if (area <= 0) {
			_face[i] = _newface[i];	// set new texture for invisible area,
			faceskip |= 1 << i;	// but don't draw it just yet
		} else
			faceskip &= ~(1 << i);
	}

	PaintFaces();

	const int xshift = -5;		// TODO: temporary fix to decrease left margin
	unsigned char *cur = cursor;
	unsigned char *scr = p_mainview->_bufferPtr + curs_x + _scrollPos  + xshift + curs_y * p_mainview->_pitch;

	for (int y = 0; y < 40; y++) {
		for (int x = 0; x < 40; x++) {
			if (x + curs_x + _scrollPos + xshift < p_mainview->_pitch && y + curs_y < p_mainview->_height)
				if (*cur)
					*scr = *cur;
			scr++;
			cur++;
		}
		scr += p_mainview->_pitch - 40;
	}
}


void EdenGame::IncAngleX(int step) {
	angle_x += step;
	if (angle_x == 70 + 2)
		angle_x = 0;
	if (angle_x == 0 - 2)
		angle_x = 70;
}

void EdenGame::DecAngleX() {
	if (angle_x != 0)
		angle_x -= (angle_x > 4) ? 4 : 2;
}

void EdenGame::IncAngleY(int step) {
	angle_y += step;
	if (angle_y == 70 + 2)
		angle_y = 0;
	if (angle_y == 0 - 2)
		angle_y = 70;
}

void EdenGame::DecAngleY() {
	if (angle_y != 0)
		angle_y -= (angle_y > 4) ? 4 : 2;
}

void EdenGame::IncZoom() {
	if (zoom == 170)
		zoom_step = 40;
	if (zoom == 570)
		zoom_step = -40;
	zoom += zoom_step;
}

void EdenGame::DecZoom() {
	if (zoom != 170) {
		if (zoom < 170)
			zoom = 170;
		else
			zoom -= 40;
	}
}

void EdenGame::initCubePC() {
	zoom = 170;
	zoom_step = 40;
	angle_x = angle_y = angle_z = 0;
	pc_cursor = &pc_cursors[0];
	curs_cur_map = -1;
	MakeTables();
}

void EdenGame::pc_selectmap(int16 num) {
	if (num != curs_cur_map) {
		pc_cursor = &pc_cursors[num];
		unsigned char *bank = _mainBankBuf + READ_LE_UINT16(_mainBankBuf);
		for (int i = 0; i < 6; i++) {
			_newface[i] = 4 + (unsigned char*)getElem(bank, pc_cursor->_sides[i]);
			if (curs_cur_map == -1)
				_face[i] = _newface[i];
		}
		curs_cur_map = num;
	}
}

void EdenGame::pc_moteur() {
	int16 curs = _currCursor;
	if (normalCursor && (p_global->_drawFlags & DrawFlags::drDrawFlag20))
		curs = 9;
	pc_selectmap(curs);
	curs_new_tick = TickCount();
	if (curs_new_tick - curs_old_tick < 1)
		return;
	curs_old_tick = curs_new_tick;
	int step = pc_cursor->speed;
	switch (pc_cursor->kind) {
	case 0:
		break;
	case 1:	// rot up-down
		DecAngleY();
		DecZoom();
		IncAngleX(step);
		break;
	case 2: // rot left-right
		DecAngleX();
		DecZoom();
		IncAngleY(step);
		break;
	case 3: // rotate random
		DecZoom();
		IncAngleX(step);
		IncAngleY(step);
		break;
	case 4: // zoom in-out
		_face[0] = _newface[0];
		DecAngleY();
		DecAngleX();
		IncZoom();
		break;
	}
	RenderCube();
}

////// macgame.c
//void MyDlgHook()  {  }
//void PrepareReply()  {  }
int16 EdenGame::OpenDialog(void *arg1, void *arg2) {
	//TODO
	return 0;
}

//void SaveDialog()  {  }
//void LostEdenMac_SavePrefs()  {  }
//void LostEdenMac_LoadPrefs()  {  }

void EdenGame::LostEdenMac_InitPrefs() {
	p_global->pref_language = 1;
	_doubledScreen = false;    // TODO: set to true
	p_global->pref_10C[0] = 192;
	p_global->pref_10C[1] = 192;
	p_global->pref_10E[0] = 255;
	p_global->pref_10E[1] = 255;
	p_global->pref_110[0] = 32;
	p_global->pref_110[1] = 32;
}

//void MacGame_DoAbout()  {  }
//void MacGame_DoAdjustMenus()  {  }
//void LostEdenMac_DoPreferences()  {  }
//void MacGame_DoSave()  {  }
//void MacGame_DoMenuCommand()  {  }
//void MacGame_DoOpen()  {  }
//void MacGame_DoSaveAs()  {  }

}   // namespace Cryo
