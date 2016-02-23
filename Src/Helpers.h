#pragma once

#include "App.h"

using namespace OVR;
namespace OvrMangaroll {

	// Holder for time information
	class Time {
	public:
		// Time in seconds that has passed since the last update
		static double Delta;
		// Time in seconds that has passed from the start of the application
		// until the last update
		static double Elapsed;
	};


	// Holder for input information
	class Frame {
	public:
		static const VrFrame *Current;
	};

	class HMD {
	public:
		static Vector3f Direction;
	/*	static Quatf Orientation;
		static Vector3f Position;*/
	};
}