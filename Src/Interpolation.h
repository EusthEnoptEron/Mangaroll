#pragma once

#include <math.h>
#include "Kernel\OVR_Alg.h"

namespace OvrMangaroll {
	
	typedef float(*InterpolationMethod)(float progress);


	class Interpol {
	public:
		static float Linear(float progress) { return progress; }
		static float QuadraticEaseIn(float progress) { return EaseInPower(progress, 2); }
		static float QuadraticEaseOut(float progress) { return EaseOutPower(progress, 2); }
		static float QuadraticEaseInOut(float progress) { return EaseInOutPower(progress, 2); }
		static float CubicEaseIn(float progress) { return EaseInPower(progress, 3); }
		static float CubicEaseOut(float progress) { return EaseOutPower(progress, 3); }
		static float CubicEaseInOut(float progress) { return EaseInOutPower(progress, 3); }
		static float QuarticEaseIn(float progress) { return EaseInPower(progress, 4); }
		static float QuarticEaseOut(float progress) { return EaseOutPower(progress, 4); }
		static float QuarticEaseInOut(float progress) { return EaseInOutPower(progress, 4); }
		static float QuinticEaseIn(float progress) { return EaseInPower(progress, 5); }
		static float QuinticEaseOut(float progress) { return EaseOutPower(progress, 5); }
		static float QuinticEaseInOut(float progress) { return EaseInOutPower(progress, 5); }

	private:
		static const float Pi;
		static const float HalfPi;

		static float EaseInPower(float progress, int power)
		{
			return powf(progress, power);
		}

		static float EaseOutPower(float progress, int power)
		{
			int sign = power % 2 == 0 ? -1 : 1;
			return (float)(sign * (powf(progress - 1, power) + sign));
		}

		static float EaseInOutPower(float progress, int power)
		{
			progress *= 2;
			if (progress < 1)
			{
				return powf(progress, power) / 2.0f;
			}
			else
			{
				int sign = power % 2 == 0 ? -1 : 1;
				return (sign / 2.0 * (powf(progress - 2, power) + sign * 2));
			}
		}

		static float SineEaseInImpl(float progress)
		{
			return sin(progress * HalfPi - HalfPi) + 1;
		}

		static float SineEaseOutImpl(float progress)
		{
			return sin(progress * HalfPi);
		}

		static float SineEaseInOutImpl(float progress)
		{
			return (sin(progress * Pi - HalfPi) + 1) / 2;
		}
	};

	class Interpolator {
	public:
		Interpolator(InterpolationMethod method = Interpol::Linear) {
			interpolate = method;
		}

		void	Set(double startDomain_, double startValue_, double endDomain_, double endValue_)
		{
			startDomain = startDomain_;
			endDomain = endDomain_;
			startValue = startValue_;
			endValue = endValue_;
		}

		double	Value(double domain) const
		{
			double f = OVR::Alg::Clamp((domain - startDomain) / (endDomain - startDomain), 0.0, 1.0);
			f = interpolate(f);
			return startValue * (1.0 - f) + endValue * f;
		}

		double	startDomain;
		double	endDomain;
		double	startValue;
		double	endValue;
		float(*interpolate)(float progress);
	};


	enum TweenState {
		TWEEN_INACTIVE,
		TWEEN_ACTIVE,
		TWEEN_PAUSED
	};
	class Tween {
	public:
		Tween(InterpolationMethod method = Interpol::Linear)
			: _Interpolator(method)
			, _State(TWEEN_INACTIVE)
			, _StartTime(0)
			, _Delta(0)
			, _Duration(1.0f)
		{
		}

		void Start(float t, float duration, float from, float to) {
			_StartTime = t;
			_Delta = 0;
			_State = TWEEN_ACTIVE;

			_Interpolator.Set(0, from, duration, to);
		}

		void Update(float t) {
			_Delta = t - _StartTime;
			_Value = _Interpolator.Value(_Delta);

			if (_Delta >= _Duration) {
				_State = TWEEN_INACTIVE;
			}
		}

		float Value() {
			return _Value;
		}

		bool IsActive() {
			return _State == TWEEN_ACTIVE;
		}

	private:
		Interpolator _Interpolator;
		TweenState _State;
		float _StartTime;
		float _Delta;
		float _Duration;
		float _Value;
	};
}