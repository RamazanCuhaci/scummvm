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

#ifndef MTROPOLIS_PLUGIN_OBSIDIAN_H
#define MTROPOLIS_PLUGIN_OBSIDIAN_H

#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/runtime.h"
#include "mtropolis/plugin/obsidian_data.h"

namespace MTropolis {

namespace Obsidian {

class MovementModifier : public Modifier {
public:
	bool load(const PlugInModifierLoaderContext &context, const Data::Obsidian::MovementModifier &data);

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
};

class RectShiftModifier : public Modifier {
public:
	bool load(const PlugInModifierLoaderContext &context, const Data::Obsidian::RectShiftModifier &data);

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
};

class TextWorkModifier : public Modifier {
public:
	bool load(const PlugInModifierLoaderContext &context, const Data::Obsidian::TextWorkModifier &data);

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
};

class ObsidianPlugIn : public MTropolis::PlugIn {
public:
	ObsidianPlugIn();

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

private:
	PlugInModifierFactory<MovementModifier, Data::Obsidian::MovementModifier> _movementModifierFactory;
	PlugInModifierFactory<RectShiftModifier, Data::Obsidian::RectShiftModifier> _rectShiftModifierFactory;
	PlugInModifierFactory<TextWorkModifier, Data::Obsidian::TextWorkModifier> _textWorkModifierFactory;
};

} // End of namespace Obsidian

} // End of namespace MTropolis

#endif
