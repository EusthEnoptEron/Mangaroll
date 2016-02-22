#pragma once

#include "Kernel/OVR_Math.h"


using namespace OVR;

namespace OvrMangaroll {

	class GlObject
	{
	public:
		GlObject(void) : Position(0.0f,0.0f,0.0f), Rotation(), Scale(1.0f, 1.0f, 1.0f), Mat(), _dirty(true) { }
		virtual ~GlObject(void) { }
	
		// Set dirty
		void Touch() { _dirty = true; }
		virtual void UpdateModel(void) {
			if(_dirty) {
				_dirty = false;
				
				Mat = Matrix4f::Scaling(Scale) *
					  Matrix4f(Rotation) *
					  Matrix4f::Translation(Position);
			}
		};

		virtual void Draw(const Matrix4f &m) = 0;

		Vector3f Position;
		Quatf Rotation;
		Vector3f Scale;

		Matrix4f Mat;

	private:
		bool _dirty;

	};
}
