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

static const uint32 hitRectListOffsets[] = {
	// Scene1001
	1, 0x004B4860,
	// Scene1002
	1, 0x004B4138,
	// Scene1201
	4, 0x004AEBD0,
	// Scene1705
	1, 0x004B69D8,
	// Scene2203
	1, 0x004B8320,
	// Scene2205
	1, 0x004B0620,
	0, 0
};

static const uint32 rectListOffsets[] = {
	// Scene1001
	1, 0x004B49F0,
	1, 0x004B4A00,
	// Scene1002
	3, 0x004B43A0,
	1, 0x004B4418,
	3, 0x004B43A0,
	// Scene1004
	1, 0x004B7C70,
	// Scene1201
	1, 0x004AEE58,
	1, 0x004AEDC8,
	1, 0x004AEE18,
	1, 0x004AED88,
	// Scene1401
	1, 0x004B6758,
	// Scene1402
	1, 0x004B0C48,
	1, 0x004B0C98,
	// Scene1403
	1, 0x004B1FF8,
	1, 0x004B2008,
	// Scene1404
	1, 0x004B8D80,
	// Scene1705
	1, 0x004B6B40,
	1, 0x004B6B30,
	// Scene2001
	1, 0x004B3680,
	1, 0x004B3670,
	// Scene2203
	1, 0x004B8420,
	// Scene2206
	1, 0x004B8AF8,
	1, 0x004B8B58,
	// Scene2207
	3, 0x004B38B8,
	1, 0x004B3948,
	// Scene2242
	1, 0x004B3DC8,
	1, 0x004B3E18,
	0, 0
};

static const uint32 messageListOffsets[] = {
	// Scene1001
	1, 0x004B4888,
	2, 0x004B4898,
	1, 0x004B4970,
	1, 0x004B4890,
	3, 0x004B4910,
	5, 0x004B4938,
	1, 0x004B4960,
	4, 0x004B48A8,
	3, 0x004B48C8,
	// Scene1002
	1, 0x004B4270,
	1, 0x004B4478,
	3, 0x004B4298,
	1, 0x004B4470,
	4, 0x004B4428,
	5, 0x004B4448,
	1, 0x004B44B8,
	2, 0x004B44A8,
	1, 0x004B44A0,
	2, 0x004B43D0,
	4, 0x004B4480,
	2, 0x004B41E0,
	5, 0x004B4148,
	// Scene1004
	3, 0x004B7BF0,
	2, 0x004B7C08,
	1, 0x004B7C18,
	2, 0x004B7C20,
	// Scene1201
	1, 0x004AEC08,
	2, 0x004AEC10,
	2, 0x004AEC20,
	2, 0x004AEC30,
	4, 0x004AEC90,
	2, 0x004AECB0,
	2, 0x004AECC0,
	5, 0x004AECF0,
	2, 0x004AECD0,
	2, 0x004AECE0,
	2, 0x004AED38,
	// Scene1401
	1, 0x004B65C8,
	1, 0x004B65D0,
	1, 0x004B65D8,
	1, 0x004B65E8,
	3, 0x004B6670,
	4, 0x004B6690,
	1, 0x004B66B0,
	3, 0x004B6658,
	2, 0x004B65F0,
	// Scene1402
	1, 0x004B0B48,
	1, 0x004B0B50,
	1, 0x004B0B58,
	1, 0x004B0B60,
	2, 0x004B0B68,
	3, 0x004B0BB8,
	3, 0x004B0BD0,
	// Scene1403
	1, 0x004B1F18,
	1, 0x004B1F20,
	3, 0x004B1F70,
	2, 0x004B1FA8,
	4, 0x004B1F88,
	3, 0x004B1F58,
	2, 0x004B1F28,
	2, 0x004B1FB8,
	// Scene1404
	1, 0x004B8C28,
	1, 0x004B8C30,
	1, 0x004B8C38,
	1, 0x004B8D28,
	3, 0x004B8CB8,
	2, 0x004B8C40,
	6, 0x004B8CE8,
	3, 0x004B8CA0,
	2, 0x004B8CD0,
	2, 0x004B8D18,
	// Scene1705
	1, 0x004B69E8,
	2, 0x004B6A08,
	4, 0x004B6AA0,
	2, 0x004B6A18,
	1, 0x004B69F0,
	2, 0x004B6AC0,
	// Scene2001
	1, 0x004B3538,
	2, 0x004B3540,
	4, 0x004B35F0,
	2, 0x004B3550,
	1, 0x004B3530,
	// Scene2201
	1, 0x004B8118,
	1, 0x004B8130,
	1, 0x004B8178,
	2, 0x004B8120,
	3, 0x004B81A0,
	1, 0x004B81B8,
	2, 0x004B8108,
	5, 0x004B8150,
	4, 0x004B8180,
	3, 0x004B8138,
	2, 0x004B8108,
	2, 0x004B81C8,
	// Scene2203
	1, 0x004B8340,
	1, 0x004B8350,
	1, 0x004B8358,
	1, 0x004B8348,
	3, 0x004B83B0,
	3, 0x004B83C8,
	2, 0x004B8370,
	2, 0x004B8360,
	2, 0x004B83E0,
	2, 0x004B83F0,
	// Scene2205
	1, 0x004B0658,
	2, 0x004B0648,
	1, 0x004B0640,
	4, 0x004B0690,
	2, 0x004B0630,
	// Scene2206
	1, 0x004B88A8,
	2, 0x004B88B8,
	1, 0x004B88C8,
	1, 0x004B8A70,
	1, 0x004B88B0,
	5, 0x004B8948,
	2, 0x004B8970,
	2, 0x004B8988,
	4, 0x004B8998,
	4, 0x004B89B8,
	4, 0x004B89D8,
	5, 0x004B89F8,
	5, 0x004B8A20,
	5, 0x004B8A48,
	// Scene2207
	1, 0x004B38E8,
	4, 0x004B38F0,
	2, 0x004B37D8,
	2, 0x004B3958,
	3, 0x004B3920,
	// Scene2242
	1, 0x004B3C18,
	1, 0x004B3D60,
	1, 0x004B3D48,
	1, 0x004B3C20,
	2, 0x004B3D50,
	5, 0x004B3CF8,
	5, 0x004B3D20,
	4, 0x004B3CB8,
	4, 0x004B3CD8,
	0, 0
};

static const uint32 navigationListOffsets[] = {
	// Module1700
	2, 0x004AE8B8,
	3, 0x004AE8E8,
	// Module1800
	4, 0x004AFD38,
	1, 0x004AFD98,
	2, 0x004AFDB0,
	4, 0x004AFDE0,
	2, 0x004AFE40,
	// Module2300
	2, 0x004B67B8,
	6, 0x004B67E8,
	2, 0x004B6878,
	3, 0x004B68F0,
	3, 0x004B68A8,
	// Module2000
	3, 0x004B7B48,
	3, 0x004B7B00,
	// Module3000
	2, 0x004B7C80,
	2, 0x004B7CE0,
	2, 0x004B7CB0,
	3, 0x004B7D58,
	3, 0x004B7D10,
	4, 0x004B7E60,
	4, 0x004B7DA0,
	4, 0x004B7E00,
	4, 0x004B7F20,
	4, 0x004B7EC0,
	2, 0x004B7F80,
	1, 0x004B7FB0,
	0, 0
};

// Hall of Records scene definitions

static const uint32 sceneInfo140Offsets[] = {
	0x004B7180,
	0x004B7198,
	0x004B71B0,
	0x004B71C8,
	0x004B71E0,
	0x004B71F8,
	0x004B7210,
	0x004B7228,
	0x004B7240,
	0x004B7258,
	0x004B7270,
	0x004B7288,
	0x004B72A0,
	0x004B72B8,
	0x004B72D0,
	0x004B72E8,
	0x004B7300,
	0x004B7318,
	0x004B7330,
	0x004B7348,
	0x004B7360,
	0x004B7378,
	0x004B7390,
	0x004B73A8,
	0x004B73C0,
	0x004B73D8,
	0x004B73F0,
	0x004B7408,
	0x004B7420,
	0x004B7438,
	0x004B7450,
	0x004B7468,
	0x004B7480,
	0x004B7498,
	0x004B74B0,
	0x004B74C8,
	0
};

