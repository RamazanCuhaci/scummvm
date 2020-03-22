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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/config.h"
#include "ultima/ultima4/person.h"
#include "ultima/ultima4/city.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/conversation.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/event.h"
#include "ultima/ultima4/game.h"   // Included for ReadPlayerController
#include "ultima/ultima4/io.h"
#include "ultima/ultima4/location.h"
#include "ultima/ultima4/music.h"
#include "ultima/ultima4/names.h"
#include "ultima/ultima4/player.h"
#include "ultima/ultima4/savegame.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/stats.h"
#include "ultima/ultima4/types.h"
#include "ultima/ultima4/u4file.h"
#include "ultima/ultima4/utils.h"
#include "ultima/ultima4/script.h"

namespace Ultima {
namespace Ultima4 {

using namespace Std;

int chars_needed(const char *s, int columnmax, int linesdesired, int *real_lines);

/**
 * Returns true of the object that 'punknown' points
 * to is a person object
 */
bool isPerson(Object *punknown) {
	Person *p;
	if ((p = dynamic_cast<Person *>(punknown)) != NULL)
		return true;
	else
		return false;
}

/**
 * Splits a piece of response text into screen-sized chunks.
 */
Common::List<Common::String> replySplit(const Common::String &text) {
	Common::String str = text;
	int pos, real_lines;
	Common::List<Common::String> reply;

	/* skip over any initial newlines */
	if ((pos = str.find("\n\n")) == 0)
		str = str.substr(pos + 1);

	unsigned int num_chars = chars_needed(str.c_str(), TEXT_AREA_W, TEXT_AREA_H, &real_lines);

	/* we only have one chunk, no need to split it up */
	unsigned int len = str.size();
	if (num_chars == len)
		reply.push_back(str);
	else {
		Common::String pre = str.substr(0, num_chars);

		/* add the first chunk to the list */
		reply.push_back(pre);
		/* skip over any initial newlines */
		if ((pos = str.find("\n\n")) == 0)
			str = str.substr(pos + 1);

		while (num_chars != str.size()) {
			/* go to the rest of the text */
			str = str.substr(num_chars);
			/* skip over any initial newlines */
			if ((pos = str.find("\n\n")) == 0)
				str = str.substr(pos + 1);

			/* find the next chunk and add it */
			num_chars = chars_needed(str.c_str(), TEXT_AREA_W, TEXT_AREA_H, &real_lines);
			pre = str.substr(0, num_chars);

			reply.push_back(pre);
		}
	}

	return reply;
}

Person::Person(MapTile tile) : Creature(tile), _start(0, 0) {
	setType(Object::PERSON);
	_dialogue = NULL;
	_npcType = NPC_EMPTY;
}

Person::Person(const Person *p) : Creature(p->_tile) {
	*this = *p;
}

bool Person::canConverse() const {
	return isVendor() || _dialogue != NULL;
}

bool Person::isVendor() const {
	return
	    _npcType >= NPC_VENDOR_WEAPONS &&
	    _npcType <= NPC_VENDOR_STABLE;
}

Common::String Person::getName() const {
	if (_dialogue)
		return _dialogue->getName();
	else if (_npcType == NPC_EMPTY)
		return Creature::getName();
	else
		return "(unnamed person)";
}

void Person::goToStartLocation() {
	setCoords(_start);
}

void Person::setDialogue(Dialogue *d) {
	_dialogue = d;
	if (_tile.getTileType()->getName() == "beggar")
		_npcType = NPC_TALKER_BEGGAR;
	else if (_tile.getTileType()->getName() == "guard")
		_npcType = NPC_TALKER_GUARD;
	else
		_npcType = NPC_TALKER;
}

void Person::setNpcType(PersonNpcType t) {
	_npcType = t;
	ASSERT(!isVendor() || _dialogue == NULL, "vendor has dialogue");
}

Common::List<Common::String> Person::getConversationText(Conversation *cnv, const char *inquiry) {
	Common::String text;

	/*
	 * a convsation with a vendor
	 */
	if (isVendor()) {
		static const Common::String ids[] = {
			"Weapons", "Armor", "Food", "Tavern", "Reagents", "Healer", "Inn", "Guild", "Stable"
		};
		Script *script = cnv->script;

		/**
		 * We aren't currently running a script, load the appropriate one!
		 */
		if (cnv->state == Conversation::INTRO) {
			// unload the previous script if it wasn't already unloaded
			if (script->getState() != Script::STATE_UNLOADED)
				script->unload();
			script->load("vendorScript.xml", ids[_npcType - NPC_VENDOR_WEAPONS], "vendor", g_context->_location->_map->getName());
			script->run("intro");
#ifdef IOS
			U4IOS::IOSConversationChoiceHelper choiceDialog;
#endif
			while (script->getState() != Script::STATE_DONE) {
				// Gather input for the script
				if (script->getState() == Script::STATE_INPUT) {
					switch (script->getInputType()) {
					case Script::INPUT_CHOICE: {
						const Common::String &choices = script->getChoices();
						// Get choice
#ifdef IOS
						choiceDialog.updateChoices(choices, script->getTarget(), npcType);
#endif
						char val = ReadChoiceController::get(choices);
						if (Common::isSpace(val) || val == '\033')
							script->unsetVar(script->getInputName());
						else {
							Common::String s_val;
							s_val = val;
							script->setVar(script->getInputName(), s_val);
						}
					}
					break;

					case Script::INPUT_KEYPRESS:
						ReadChoiceController::get(" \015\033");
						break;

					case Script::INPUT_NUMBER: {
#ifdef IOS
						U4IOS::IOSConversationHelper ipadNumberInput;
						ipadNumberInput.beginConversation(U4IOS::UIKeyboardTypeNumberPad, "Amount?");
#endif
						int val = ReadIntController::get(script->getInputMaxLen(), TEXT_AREA_X + g_context->col, TEXT_AREA_Y + g_context->_line);
						script->setVar(script->getInputName(), val);
					}
					break;

					case Script::INPUT_STRING: {
#ifdef IOS
						U4IOS::IOSConversationHelper ipadNumberInput;
						ipadNumberInput.beginConversation(U4IOS::UIKeyboardTypeDefault);
#endif
						Common::String str = ReadStringController::get(script->getInputMaxLen(), TEXT_AREA_X + g_context->col, TEXT_AREA_Y + g_context->_line);
						if (str.size()) {
							lowercase(str);
							script->setVar(script->getInputName(), str);
						} else script->unsetVar(script->getInputName());
					}
					break;

					case Script::INPUT_PLAYER: {
						ReadPlayerController getPlayerCtrl;
						eventHandler->pushController(&getPlayerCtrl);
						int player = getPlayerCtrl.waitFor();
						if (player != -1) {
							Common::String player_str = xu4_to_string(player + 1);
							script->setVar(script->getInputName(), player_str);
						} else script->unsetVar(script->getInputName());
					}
					break;

					default:
						break;
					} // } switch

					// Continue running the script!
					g_context->_line++;
					script->_continue();
				} // } if
			} // } while
		}

		// Unload the script
		script->unload();
		cnv->state = Conversation::DONE;
	}

	/*
	 * a conversation with a non-vendor
	 */
	else {
		text = "\n\n\n";

		switch (cnv->state) {
		case Conversation::INTRO:
			text = getIntro(cnv);
			break;

		case Conversation::TALK:
			text += getResponse(cnv, inquiry) + "\n";
			break;

		case Conversation::CONFIRMATION:
			ASSERT(_npcType == NPC_LORD_BRITISH, "invalid state: %d", cnv->state);
			text += lordBritishGetQuestionResponse(cnv, inquiry);
			break;

		case Conversation::ASK:
		case Conversation::ASKYESNO:
			ASSERT(_npcType != NPC_HAWKWIND, "invalid state for hawkwind conversation");
			text += talkerGetQuestionResponse(cnv, inquiry) + "\n";
			break;

		case Conversation::GIVEBEGGAR:
			ASSERT(_npcType == NPC_TALKER_BEGGAR, "invalid npc type: %d", _npcType);
			text = beggarGetQuantityResponse(cnv, inquiry);
			break;

		case Conversation::FULLHEAL:
		case Conversation::ADVANCELEVELS:
			/* handled elsewhere */
			break;

		default:
			ASSERT(0, "invalid state: %d", cnv->state);
		}
	}

	return replySplit(text);
}

/**
 * Get the prompt shown after each reply.
 */
Common::String Person::getPrompt(Conversation *cnv) {
	if (isVendor())
		return "";

	Common::String prompt;
	if (cnv->state == Conversation::ASK)
		prompt = getQuestion(cnv);
	else if (cnv->state == Conversation::GIVEBEGGAR)
		prompt = "How much? ";
	else if (cnv->state == Conversation::CONFIRMATION)
		prompt = "\n\nHe asks: Art thou well?";
	else if (cnv->state != Conversation::ASKYESNO)
		prompt = _dialogue->getPrompt();

	return prompt;
}

/**
 * Returns the valid keyboard choices for a given conversation.
 */
const char *Person::getChoices(Conversation *cnv) {
	if (isVendor())
		return cnv->script->getChoices().c_str();

	switch (cnv->state) {
	case Conversation::CONFIRMATION:
	case Conversation::CONTINUEQUESTION:
		return "ny\015 \033";

	case Conversation::PLAYER:
		return "012345678\015 \033";

	default:
		ASSERT(0, "invalid state: %d", cnv->state);
	}

	return NULL;
}

Common::String Person::getIntro(Conversation *cnv) {
	if (_npcType == NPC_EMPTY) {
		cnv->state = Conversation::DONE;
		return Common::String("Funny, no\nresponse!\n");
	}

	// As far as I can tell, about 50% of the time they tell you their
	// name in the introduction
	Response *intro;
	if (xu4_random(2) == 0)
		intro = _dialogue->getIntro();
	else
		intro = _dialogue->getLongIntro();

	cnv->state = Conversation::TALK;
	Common::String text = processResponse(cnv, intro);

	return text;
}

Common::String Person::processResponse(Conversation *cnv, Response *response) {
	Common::String text;
	const vector<ResponsePart> &parts = response->getParts();
	for (vector<ResponsePart>::const_iterator i = parts.begin(); i != parts.end(); i++) {

		// check for command triggers
		if (i->isCommand())
			runCommand(cnv, *i);

		// otherwise, append response part to reply
		else
			text += *i;
	}
	return text;
}

void Person::runCommand(Conversation *cnv, const ResponsePart &command) {
	if (command == ResponsePart::ASK) {
		cnv->question = _dialogue->getQuestion();
		cnv->state = Conversation::ASK;
	} else if (command == ResponsePart::END) {
		cnv->state = Conversation::DONE;
	} else if (command == ResponsePart::ATTACK) {
		cnv->state = Conversation::ATTACK;
	} else if (command == ResponsePart::BRAGGED) {
		g_context->_party->adjustKarma(KA_BRAGGED);
	} else if (command == ResponsePart::HUMBLE) {
		g_context->_party->adjustKarma(KA_HUMBLE);
	} else if (command == ResponsePart::ADVANCELEVELS) {
		cnv->state = Conversation::ADVANCELEVELS;
	} else if (command == ResponsePart::HEALCONFIRM) {
		cnv->state = Conversation::CONFIRMATION;
	} else if (command == ResponsePart::STARTMUSIC_LB) {
		musicMgr->lordBritish();
	} else if (command == ResponsePart::STARTMUSIC_HW) {
		musicMgr->hawkwind();
	} else if (command == ResponsePart::STOPMUSIC) {
		musicMgr->play();
	} else if (command == ResponsePart::HAWKWIND) {
		g_context->_party->adjustKarma(KA_HAWKWIND);
	} else {
		ASSERT(0, "unknown command trigger in dialogue response: %s\n", Common::String(command).c_str());
	}
}

Common::String Person::getResponse(Conversation *cnv, const char *inquiry) {
	Common::String reply;
	Virtue v;
	const ResponsePart &action = _dialogue->getAction();

	reply = "\n";

	/* Does the person take action during the conversation? */
	if (action == ResponsePart::END) {
		runCommand(cnv, action);
		return _dialogue->getPronoun() + " turns away!\n";
	} else if (action == ResponsePart::ATTACK) {
		runCommand(cnv, action);
		return Common::String("\n") + getName() + " says: On guard! Fool!";
	}

	if (_npcType == NPC_TALKER_BEGGAR && scumm_strnicmp(inquiry, "give", 4) == 0) {
		reply.clear();
		cnv->state = Conversation::GIVEBEGGAR;
	}

	else if (scumm_strnicmp(inquiry, "join", 4) == 0 &&
	         g_context->_party->canPersonJoin(getName(), &v)) {
		CannotJoinError join = g_context->_party->join(getName());

		if (join == JOIN_SUCCEEDED) {
			reply += "I am honored to join thee!";
			g_context->_location->_map->removeObject(this);
			cnv->state = Conversation::DONE;
		} else {
			reply += "Thou art not ";
			reply += (join == JOIN_NOT_VIRTUOUS) ? getVirtueAdjective(v) : "experienced";
			reply += " enough for me to join thee.";
		}
	}

	else if ((*_dialogue)[inquiry]) {
		Dialogue::Keyword *kw = (*_dialogue)[inquiry];

		reply = processResponse(cnv, kw->getResponse());
	}

	else if (settings._debug && scumm_strnicmp(inquiry, "dump", 4) == 0) {
		vector<Common::String> words = split(inquiry, " \t");
		if (words.size() <= 1)
			reply = _dialogue->dump("");
		else
			reply = _dialogue->dump(words[1]);
	}

	else
		reply += processResponse(cnv, _dialogue->getDefaultAnswer());

	return reply;
}

Common::String Person::talkerGetQuestionResponse(Conversation *cnv, const char *answer) {
	bool valid = false;
	bool yes = false;
	char ans = tolower(answer[0]);

	if (ans == 'y' || ans == 'n') {
		valid = true;
		yes = ans == 'y';
	}

	if (!valid) {
		cnv->state = Conversation::ASKYESNO;
		return "Yes or no!";
	}

	cnv->state = Conversation::TALK;
	return "\n" + processResponse(cnv, cnv->question->getResponse(yes));
}

Common::String Person::beggarGetQuantityResponse(Conversation *cnv, const char *response) {
	Common::String reply;

	cnv->quant = (int) strtol(response, NULL, 10);
	cnv->state = Conversation::TALK;

	if (cnv->quant > 0) {
		if (g_context->_party->donate(cnv->quant)) {
			reply = "\n";
			reply += _dialogue->getPronoun();
			reply += " says: Oh Thank thee! I shall never forget thy kindness!\n";
		}

		else
			reply = "\n\nThou hast not that much gold!\n";
	} else
		reply = "\n";

	return reply;
}

Common::String Person::lordBritishGetQuestionResponse(Conversation *cnv, const char *answer) {
	Common::String reply;

	cnv->state = Conversation::TALK;

	if (tolower(answer[0]) == 'y') {
		reply = "Y\n\nHe says: That is good.\n";
	}

	else if (tolower(answer[0]) == 'n') {
		reply = "N\n\nHe says: Let me heal thy wounds!\n";
		cnv->state = Conversation::FULLHEAL;
	}

	else
		reply = "\n\nThat I cannot\nhelp thee with.\n";

	return reply;
}

Common::String Person::getQuestion(Conversation *cnv) {
	return "\n" + cnv->question->getText() + "\n\nYou say: ";
}

/**
 * Returns the number of characters needed to get to
 * the next line of text (based on column width).
 */
int chars_to_next_line(const char *s, int columnmax) {
	int chars = -1;

	if (strlen(s) > 0) {
		int lastbreak = columnmax;
		chars = 0;
		for (const char *str = s; *str; str++) {
			if (*str == '\n')
				return (str - s);
			else if (*str == ' ')
				lastbreak = (str - s);
			else if (++chars >= columnmax)
				return lastbreak;
		}
	}

	return chars;
}

/**
 * Counts the number of lines (of the maximum width given by
 * columnmax) in the Common::String.
 */
int linecount(const Common::String &s, int columnmax) {
	int lines = 0;
	unsigned ch = 0;
	while (ch < s.size()) {
		ch += chars_to_next_line(s.c_str() + ch, columnmax);
		if (ch < s.size())
			ch++;
		lines++;
	}
	return lines;
}


/**
 * Returns the number of characters needed to produce a
 * valid screen of text (given a column width and row height)
 */
int chars_needed(const char *s, int columnmax, int linesdesired, int *real_lines) {
	int chars = 0,
	    totalChars = 0;

	Common::String new_str = s;
	const char *str = new_str.c_str();

	// try breaking text into paragraphs first
	Common::String text = s;
	Common::String paragraphs;
	unsigned int pos;
	int lines = 0;
	while ((pos = text.find("\n\n")) < text.size()) {
		Common::String p = text.substr(0, pos);
		lines += linecount(p.c_str(), columnmax);
		if (lines <= linesdesired)
			paragraphs += p + "\n";
		else break;
		text = text.substr(pos + 1);
	}
	// Seems to be some sort of clang compilation bug in this code, that causes this addition
	// to not work correctly.
	int totalPossibleLines = lines + linecount(text.c_str(), columnmax);
	if (totalPossibleLines <= linesdesired)
		paragraphs += text;

	if (!paragraphs.empty()) {
		*real_lines = lines;
		return paragraphs.size();
	} else {
		// reset variables and try another way
		lines = 1;
	}
	// gather all the line breaks
	while ((chars = chars_to_next_line(str, columnmax)) >= 0) {
		if (++lines >= linesdesired)
			break;

		int num_to_move = chars;
		if (*(str + num_to_move) == '\n')
			num_to_move++;

		totalChars += num_to_move;
		str += num_to_move;
	}

	*real_lines = lines;
	return totalChars;
}

} // End of namespace Ultima4
} // End of namespace Ultima
