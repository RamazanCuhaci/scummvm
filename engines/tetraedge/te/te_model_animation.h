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

#ifndef TETRAEDGE_TE_TE_MODEL_ANIMATION_H
#define TETRAEDGE_TE_TE_MODEL_ANIMATION_H

#include "common/path.h"
#include "common/stream.h"

#include "tetraedge/te/te_animation.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_model.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_trs.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TeModel;

class TeModelAnimation : public TeAnimation, public TeResource {
public:
	struct NMORotation {
		float _f;
		TeQuaternion _rot;
	};
	struct NMOTranslation {
		float _f;
		TeVector3f32 _trans;
	};
	struct NMOScale {
		float _f;
		TeVector3f32 _scale;
	};

	TeModelAnimation();

	~TeModelAnimation() {
		destroy();
	}

	void bind(const TeIntrusivePtr<TeModel> &ptr) {
		_model = ptr;
	};
	int calcCurrentFrame(double millis);
	void cont() override;
	void destroy();
	int findBone(const Common::String &bname);
	int firstFrame() const;
	TeMatrix4x4 getMatrix(const Common::String &name, unsigned long frame, bool param_5);
	TeQuaternion getNMORotation(unsigned long param_3, float param_4) const;
	TeVector3f32 getNMOTranslation(unsigned long param_3, float param_4) const;
	TeTRS getTRS(const Common::String &boneName, unsigned long frame, bool param_5);
	TeTRS getTRS(unsigned long boneNo, unsigned long frame, bool param_5) const;
	int lastFrame() const;
	bool load(const Common::Path &path);
	bool load(Common::SeekableReadStream &stream);
	int nbFrames();
	void reset() override;
	void resizeFBXArrays(unsigned long len);
	void resizeNMOArrays(unsigned long len);
	void save(Common::SeekableWriteStream &stream);
	void saveBone(Common::SeekableWriteStream &stream, uint param_2);
	void setBoneName(uint boneNo, const Common::String &bname);
	void setFrameLimits(int framemin, int framemax) {
		_firstFrame = framemin;
		_lastFrame = framemax;
	}
	void setRotation(unsigned long num, float amount, const TeQuaternion &rot);
	void setScale(unsigned long num, float amount, const TeVector3f32 &scale);
	void setTranslation(unsigned long num, float amount, const TeVector3f32 &trans);
	void unbind();
	void update(double proportion) override;

	int curFrame2() const { return _curFrame2; }
	float speed() const { return _speed; }

	TeIntrusivePtr<TeModel> _model;
	int _firstFrame;
	int _lastFrame;
	Common::Path _loadedPath;

private:
	Common::Array<Common::Array<TeTRS>> _fbxArrays;
	Common::Array<Common::Array<NMOTranslation>> _nmoTransArrays;
	Common::Array<Common::Array<NMORotation>> _nmoRotArrays;
	Common::Array<Common::Array<NMOScale>> _nmoScaleArrays;
	Common::Array<Common::String> _boneNames;
	int _curFrame;
	int _curFrame2;
	bool _curFrameValFresh;
	int _repeatNum;
	bool _finishedSignalPending;
	int _useNMOArrays;
	int _numNMOFrames;
	float _speed;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_MODEL_ANIMATION_H
