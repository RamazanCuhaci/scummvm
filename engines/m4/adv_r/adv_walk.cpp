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

#include "m4/adv_r/adv_walk.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/vars.h"

namespace M4 {

void ws_demand_location(machine *myWalker, int32 x, int32 y) {
	error("TODO: ws_demand_location");
}

void ws_get_walker_info(machine *myWalker, int32 *x, int32 *y, int32 *s, int32 *layer, int32 *facing) {
	error("TODO: ws_get_walker_info");
}

void set_walker_scaling(SceneDef *rdef) {
	_G(globals)[GLB_MIN_Y] = rdef->back_y << 16;
	_G(globals)[GLB_MAX_Y] = rdef->front_y << 16;
	_G(globals)[GLB_MIN_SCALE] = FixedDiv(rdef->back_scale << 16, 100 << 16);
	_G(globals)[GLB_MAX_SCALE] = FixedDiv(rdef->front_scale << 16, 100 << 16);
	if (_G(globals)[GLB_MIN_Y] == _G(globals)[GLB_MAX_Y])
		_G(globals)[GLB_SCALER] = 0;
	else
		_G(globals)[GLB_SCALER] = FixedDiv(_G(globals)[GLB_MAX_SCALE] - _G(globals)[GLB_MIN_SCALE], _G(globals)[GLB_MAX_Y] - _G(globals)[GLB_MIN_Y]);
}


} // End of namespace M4
