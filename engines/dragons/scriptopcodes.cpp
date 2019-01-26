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

#include "dragons/dragons.h"
#include "dragons/dragonflg.h"
#include "dragons/dragonini.h"
#include "dragons/dragonobd.h"
#include "dragons/scriptopcodes.h"
#include "dragons/actor.h"
#include "scriptopcodes.h"


namespace Dragons {
// ScriptOpCall

void ScriptOpCall::skip(uint size) {
	_code += size;
}

byte ScriptOpCall::readByte() {
	return *_code++;
}

int16 ScriptOpCall::readSint16() {
	int16 value = READ_LE_UINT16(_code);
	_code += 2;
	return value;
}

uint32 ScriptOpCall::readUint32() {
	uint32 value = READ_LE_UINT32(_code);
	_code += 4;
	return value;
}

// ScriptOpcodes

ScriptOpcodes::ScriptOpcodes(DragonsEngine *vm, DragonFLG *dragonFLG)
	: _vm(vm), _dragonFLG(dragonFLG), _data_80071f5c(0) {
	initOpcodes();
}

ScriptOpcodes::~ScriptOpcodes() {
	freeOpcodes();
}

void ScriptOpcodes::execOpcode(ScriptOpCall &scriptOpCall) {
	if (!_opcodes[scriptOpCall._op])
		error("ScriptOpcodes::execOpcode() Unimplemented opcode %d (0x%X)", scriptOpCall._op, scriptOpCall._op);
	debug(3, "execScriptOpcode(%d) %s", scriptOpCall._op, _opcodeNames[scriptOpCall._op].c_str());
	(*_opcodes[scriptOpCall._op])(scriptOpCall);
}

typedef Common::Functor1Mem<ScriptOpCall&, void, ScriptOpcodes> ScriptOpcodeI;
#define OPCODE(op, func) \
	_opcodes[op] = new ScriptOpcodeI(this, &ScriptOpcodes::func); \
	_opcodeNames[op] = #func;

void ScriptOpcodes::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < DRAGONS_NUM_SCRIPT_OPCODES; ++i) {
		_opcodes[i] = 0;
	}
	// Register opcodes
	// OPCODE(1, opUnk1);

	OPCODE(4,  opExecuteScript);

	OPCODE(7,  opUnk7);

	OPCODE(10, opUnkA);

	OPCODE(15, opUnkF);

	OPCODE(19, opUnk13PropertiesRelated);
	OPCODE(31, opPlayMusic);

}

#undef OPCODE

void ScriptOpcodes::freeOpcodes() {
	for (uint i = 0; i < DRAGONS_NUM_SCRIPT_OPCODES; ++i) {
		delete _opcodes[i];
	}
}

void ScriptOpcodes::updateReturn(ScriptOpCall &scriptOpCall, uint16 size) {
	// scriptOpCall._deltaOfs = size * 2 + 2;
}


void ScriptOpcodes::runScript(ScriptOpCall &scriptOpCall) {
	scriptOpCall._field8 = 0;
	scriptOpCall._result = 0;
	_data_80071f5c = 0;
	executeScriptLoop(scriptOpCall);
}

void ScriptOpcodes::executeScriptLoop(ScriptOpCall &scriptOpCall) {

	if (scriptOpCall._code >= scriptOpCall._codeEnd || scriptOpCall._result & 1) {
		return;
	}

	if (_vm->isFlagSet(Dragons::ENGINE_FLAG_100000)) {
		return;
	}

	if (_vm->isFlagSet(Dragons::ENGINE_FLAG_80000)) {
		//TODO
//		if (IsPressedStart(0)) {
//			Dragons::getEngine()->setFlags(Dragons::ENGINE_FLAG_100000);
//		}
	}

	uint16 opcode = READ_LE_INT16(scriptOpCall._code);

	scriptOpCall._op = (byte) opcode;
	if (opcode < DRAGONS_NUM_SCRIPT_OPCODES) {
		execOpcode(scriptOpCall);
	}

	while (scriptOpCall._code < scriptOpCall._codeEnd && !(scriptOpCall._result & 1) && _data_80071f5c == 0) {

		if (_vm->isFlagSet(Dragons::ENGINE_FLAG_100000)) {
			return;
		}

		if (_vm->isFlagSet(Dragons::ENGINE_FLAG_80000)) {
			//TODO
//		if (IsPressedStart(0)) {
//			Dragons::getEngine()->setFlags(Dragons::ENGINE_FLAG_100000);
//		}
		}

		uint16 opcode = READ_LE_INT16(scriptOpCall._code);

		if (opcode >= DRAGONS_NUM_SCRIPT_OPCODES) {
			return; //TODO should continue here.
		}
		scriptOpCall._op = (byte) opcode;
		execOpcode(scriptOpCall);
	}
}

// Opcodes

void ScriptOpcodes::opUnk1(ScriptOpCall &scriptOpCall) {
//	ARG_INT16(framePointer);
//	debug(3, "set frame pointer %X", framePointer);
//	actor->loadFrame((uint16)framePointer);
//	actor->flags |= Dragons::ACTOR_FLAG_2;
//	actor->sequenceTimer = actor->field_c;
//	updateReturn(scriptOpCall, 1);
}

void ScriptOpcodes::opExecuteScript(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_UINT32(obdOffset);
	ScriptOpCall newScriptOpCall;
	byte *data =_vm->_dragonOBD->getObdAtOffset(obdOffset);

	newScriptOpCall._code = data + 4;
	newScriptOpCall._codeEnd = newScriptOpCall._code + READ_LE_UINT32(data);
	newScriptOpCall._field8 = scriptOpCall._field8;
	newScriptOpCall._result = 0;

	executeScriptLoop(newScriptOpCall);
}

void ScriptOpcodes::opUnk7(ScriptOpCall &scriptOpCall) {
	if (scriptOpCall._field8 == 0) {
		opCode_Unk7(scriptOpCall);
	} else {
		scriptOpCall._code += 6;
	}

}


void ScriptOpcodes::opPlayMusic(ScriptOpCall &scriptOpCall) {
	byte *code = scriptOpCall._code;
	scriptOpCall._code += 4;
	if (scriptOpCall._field8 == 0) {
		//TODO play music here.
	}
}

void ScriptOpcodes::opUnk13PropertiesRelated(ScriptOpCall &scriptOpCall) {
	if (checkPropertyFlag(scriptOpCall)) {
		scriptOpCall._code += 4;
	} else {
		scriptOpCall._code += 4 + READ_LE_INT16(scriptOpCall._code);
	}
}

bool ScriptOpcodes::checkPropertyFlag(ScriptOpCall &scriptOpCall) {
	byte *codePtrOffsetA = scriptOpCall._code + 0xA;
	byte *codePtrOffset2 = scriptOpCall._code + 2;

	uint16 status = 0;
	uint16 result = 0;

	uint16 t2 = 0;
	uint16 t0 = 0;

	for(;;) {
		byte value = 0;
		if (*codePtrOffsetA & 1) {
			uint32 propId = READ_LE_UINT16(codePtrOffset2) * 8 + READ_LE_UINT16(codePtrOffsetA - 6);
			value = _dragonFLG->get(propId) ? 1 : 0;
		} else {
			debug(3, "Op13 get here!!");
			if (*codePtrOffsetA & 2) {
				debug(3, "Op13 get here!! & 2");
				t2 = _vm->getVar(READ_LE_UINT16(codePtrOffset2));
			}

			if (*codePtrOffsetA & 4) {
				t2 = getINIField(READ_LE_INT16(codePtrOffsetA - 6) - 1, READ_LE_INT16(codePtrOffset2));
				debug(3, "Op13 get here!! & 4 %d, %d", READ_LE_INT16(codePtrOffsetA - 6), t2);

			}

			if (!(*codePtrOffsetA & 7)) {
				debug(3, "Op13 get here!! & 7");
				t2 = READ_LE_UINT16(codePtrOffsetA - 6);
			}

			if (*codePtrOffsetA & 8) {
				debug(3, "Op13 get here!! & 8");
				t0 = _vm->getVar(READ_LE_UINT16(codePtrOffsetA - 4));
			}

			if (*codePtrOffsetA & 0x10) {
				debug(3, "Op13 get here!! & 0x10");
				t0 = getINIField(READ_LE_INT16(codePtrOffsetA - 2) - 1, READ_LE_INT16(codePtrOffsetA - 4));
			}

			if (!(*codePtrOffsetA & 0x18)) {
				debug(3, "Op13 get here!! & 0x18");
				t0 = READ_LE_UINT16(codePtrOffsetA - 2);
			}

			if (*(codePtrOffsetA + 1) == 0 && t0 == t2) {
				value = 1;
			}

			if (*(codePtrOffsetA + 1) == 1 && t2 < t0) {
				value = 1;
			}

			if (*(codePtrOffsetA + 1) == 2 && t0 < t2) {
				value = 1;
			}

		}

		if (*codePtrOffsetA & 0x20) {
			value ^= 1;
		}

		if (!(status & 0xffff)) {
			result |= value;
		} else {
			result &= value;
		}

		status = (*codePtrOffsetA >> 6) & 1;

		if (!(*codePtrOffsetA & 0x80)) {
			break;
		}

		codePtrOffset2 += 0xa;
		codePtrOffsetA += 0xa;
	}

	scriptOpCall._code = codePtrOffset2 + 0xa;

	return (result & 0xffff) != 0;
}

void ScriptOpcodes::opUnkA(ScriptOpCall &scriptOpCall) {
	if (scriptOpCall._field8 == 0) {
		opCode_UnkA_setsProperty(scriptOpCall);
	} else {
		scriptOpCall._code += 0xC;
	}
}

void ScriptOpcodes::opUnkF(ScriptOpCall &scriptOpCall) {
	scriptOpCall._result = 1;
}

void ScriptOpcodes::opCode_UnkA_setsProperty(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(field2);
	ARG_INT16(field4);
	ARG_INT16(field6);
	ARG_INT16(field8);
	ARG_INT8(fieldA);
	ARG_INT8(fieldB);

	enum S2Type {
		notSet,
		var,
		ini
	};

	S2Type s2Type = notSet;

	int16 s1 = 0;
	int16 s2 = 0;

	if (fieldA & 8) {
		// VAR
		s1 = _vm->getVar(field6 / 2);
	} else if (fieldA & 0x10) {
		// INI
		s1 = getINIField(field8 - 1, field6);
	} else {
		s1 = field8;
	}

	if (fieldA & 1) {
		debug(3, "fieldA & 1");
		bool flagValue = s1 != 0 ? true : false;
		_dragonFLG->set(field2 * 8 + field4, flagValue);
	}

	if (fieldA & 2) {
		debug(3, "fieldA & 2");
		s2Type = var;
		s2 = _vm->getVar(field2);
	}

	if (fieldA & 4) {
		debug(3, "fieldA & 4");
		s2Type = ini;
		s2 = getINIField(field4 - 1, field2);
		DragonINI *ini = _vm->getINI(field4 - 1);

		if (field2 == 0x1a && ini->field_1a_flags_maybe & 1 && ini->sceneId == _vm->getCurrentSceneId()) {
			if (s1 & 2) {
				ini->actor->flags |= Dragons::ACTOR_FLAG_80;
				ini->actor->field_e = 0x100;
			} else {
				ini->actor->flags &= ~Dragons::ACTOR_FLAG_80;
			}

			if (s1 & 4) {
				ini->actor->flags |= Dragons::ACTOR_FLAG_8000;
			} else {
				ini->actor->flags &= ~Dragons::ACTOR_FLAG_8000;
			}
		}

		if (fieldA & 4 && field2 == 0 && !(ini->field_1a_flags_maybe & 1) && ini->sceneId == _vm->getCurrentSceneId()) {
			if (s1 == -1) {
				if (ini->iptIndex_maybe != -1) {
					// TODO ipt_img_file_related_3(ini->iptIndex_maybe);
				}
			} else {
				// TODO ipt_img_file_related_2(s1);
			}
		}
	}

	if (s2Type != notSet) {
		if (fieldB == 1) {
			s2 += s1;
		} else {
			if (fieldB < 2) {
				if (fieldB == 0) {
					s2 = s1;
				}
			} else {
				if (fieldB == 2) {
					s2 -= s1;
				} else {
					if (fieldB == 3) {
						//TODO s2 = sub_80023830(s1);
					}
				}
			}
		}

		if (s2Type == ini) {
			setINIField(field4 - 1, field2, s2);
		} else  { //var type
			_vm->setVar(field2, s2);
		}
	}
}

uint16 ScriptOpcodes::getINIField(uint32 iniIndex, uint16 fieldOffset) {
	DragonINI *ini = _vm->getINI(iniIndex);

	switch (fieldOffset) {
		case 0x14 : return ini->field_14;
		case 0x1A : return ini->field_1a_flags_maybe;
		case 0x20 : return ini->field_20_actor_field_14;
		default: error("Invalid fieldOffset %d", fieldOffset);
	}
}

void ScriptOpcodes::setINIField(uint32 iniIndex, uint16 fieldOffset, uint16 value) {
	DragonINI *ini = _vm->getINI(iniIndex);

	switch (fieldOffset) {
		case 0x14 : ini->field_14 = value; break;
		case 0x1A : ini->field_1a_flags_maybe = value; break;
		case 0x20 : ini->field_20_actor_field_14 = value; break;
		default: error("Invalid fieldOffset %X", fieldOffset);
	}

}

void ScriptOpcodes::opCode_Unk7(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(field2);
	ARG_INT16(sceneId); //sceneId

	uint16 currentScene = _vm->getCurrentSceneId();
	DragonINI *ini = _vm->getINI(field2 - 1);
	if (!(field0 & 0x8000)) {

		if (ini->field_1a_flags_maybe & 1) {
			if (ini->sceneId == currentScene) {
				assert(ini->actor);
				ini->actor->reset_maybe();
			}
			if (sceneId == currentScene) {
				ini->actor = _vm->_actorManager->loadActor(ini->actorResourceId, ini->frameIndexId_maybe, ini->x, ini->y, 0);
				ini->actor->_sequenceID2 = ini->field_20_actor_field_14;
				if (ini->field_1a_flags_maybe & 2) {
					ini->actor->flags |= Dragons::ACTOR_FLAG_80;
				} else {
					ini->actor->flags &= ~Dragons::ACTOR_FLAG_80;
				}

				if (ini->field_1a_flags_maybe & 0x20) {
					ini->actor->flags |= Dragons::ACTOR_FLAG_100;
				} else {
					ini->actor->flags &= ~Dragons::ACTOR_FLAG_100;
				}

				if (ini->field_1a_flags_maybe & 4) {
					ini->actor->flags |= Dragons::ACTOR_FLAG_8000;
				} else {
					ini->actor->flags &= ~Dragons::ACTOR_FLAG_8000;
				}

				if (ini->field_1a_flags_maybe & 0x100) {
					ini->actor->flags |= Dragons::ACTOR_FLAG_4000;
				} else {
					ini->actor->flags &= ~Dragons::ACTOR_FLAG_4000;
				}
			}
		} else {
			if (ini->sceneId == currentScene && ini->iptIndex_maybe != -1) {
				//TODO ipt_img_file_related_3(ini->iptIndex_maybe);
			}
			if (sceneId == currentScene && ini->iptIndex_maybe != -1) {
				// TODO ipt_img_file_related_2(ini->iptIndex_maybe);
			}
		}

		if (ini->sceneId == 1) {
			//TODO 0x8002d218
		}

		if (sceneId == 1) {
			// TODO 0x8002d2f4
		}
	}
	ini->sceneId = sceneId;
}


} // End of namespace Dragons
