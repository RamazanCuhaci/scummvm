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

#ifndef AGS_ENGINE_AC_SYS_EVENTS_H
#define AGS_ENGINE_AC_SYS_EVENTS_H

#include "common/events.h"
#include "ags/shared/ac/keycode.h"

namespace AGS3 {

// AGS own mouse button codes
// TODO: these were internal button codes, but AGS script uses different ones,
// which start with Left=1, and make more sense (0 is easier to use as "no value").
// Must research if there are any dependencies to these internal values, and if not,
// then just replace these matching script ones!
// UPD: even plugin API seem to match script codes and require remap to internals.
// UPD: or use SDL constants in the engine, but make conversion more visible by using a function.
enum eAGSMouseButton {
	MouseNone = -1,
	MouseLeft = 0,
	MouseRight = 1,
	MouseMiddle = 2
};


// Keyboard input handling
//
// avoid including SDL.h here, at least for now, because that leads to conflicts with allegro
union SDL_Event;

// Converts SDL key data to eAGSKeyCode, which may be also directly used as an ASCII char
// if it is in proper range, see comments to eAGSKeyCode for details.
eAGSKeyCode ags_keycode_from_sdl(const SDL_Event &event);
// Converts eAGSKeyCode to SDL key scans (up to 3 values, because this is not a 1:1 match);
// NOTE: fails at Ctrl+ or Alt+ AGS keys, or any unknown key codes.

bool ags_key_to_sdl_scan(eAGSKeyCode key, Common::KeyState(&scan)[3]);

// Tells if key event refers to one of the mod-keys
inline bool is_mod_key(const Common::KeyState &ks) {
	return ks.keycode == Common::KEYCODE_LCTRL || ks.keycode == Common::KEYCODE_RCTRL ||
	       ks.keycode == Common::KEYCODE_LALT || ks.keycode == Common::KEYCODE_RALT ||
	       ks.keycode == Common::KEYCODE_LSHIFT || ks.keycode == Common::KEYCODE_RSHIFT ||
	       ks.keycode == Common::KEYCODE_MODE;
}

// Converts mod key into merged mod (left & right) for easier handling
inline int make_merged_mod(int mod) {
	int m_mod = 0;
	if ((mod & Common::KBD_CTRL) != 0) m_mod |= Common::KBD_CTRL;
	if ((mod & Common::KBD_SHIFT) != 0) m_mod |= Common::KBD_SHIFT;
	if ((mod & Common::KBD_ALT) != 0) m_mod |= Common::KBD_ALT;
	// what about Common::KBD_GUI, and there's also some Common::KEYCODE_MODE?
	return m_mod;
}

// Tells if there are any buffered key events
bool ags_keyevent_ready();
// Queries for the next key event in buffer; returns uninitialized data if none was queued
SDL_Event ags_get_next_keyevent();
// Tells if the key is currently down, provided AGS key;
// Returns positive value if it's down, 0 if it's not, negative value if the key code is not supported.
// NOTE: for particular script codes this function returns positive if either of two keys are down.
int ags_iskeydown(eAGSKeyCode ags_key);
// Simulates key press with the given AGS key
void ags_simulate_keypress(eAGSKeyCode ags_key);


// Mouse input handling
//
// Tells if the mouse button is currently down
bool ags_misbuttondown(int but);
// Returns mouse button code
int  ags_mgetbutton();
// Returns recent relative mouse movement
void ags_mouse_get_relxy(int &x, int &y);
// Updates mouse cursor position in game
void ags_domouse(int what);
// Returns -1 for wheel down and +1 for wheel up
// TODO: introduce constants for this
int  ags_check_mouse_wheel();

// Other input utilities
//
// Clears buffered keypresses and mouse clicks, if any
void ags_clear_input_buffer();
// Halts execution until any user input
// TODO: seriously not a good design, replace with event listening
void ags_wait_until_keypress();


// Events.
//
union SDL_Event;
// Set engine callback for when quit event is received by the backend.
void sys_evt_set_quit_callback(void(*proc)(void));
// Set engine callback for when input focus is received or lost by the window.
void sys_evt_set_focus_callbacks(void(*switch_in)(void), void(*switch_out)(void));

// Process single event.
void sys_evt_process_one(const SDL_Event &event);
// Process all events in the backend's queue.
void sys_evt_process_pending(void);

} // namespace AGS3

#endif
