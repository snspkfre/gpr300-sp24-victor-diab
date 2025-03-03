#include <cmath>

namespace vd
{
	enum IntMethod
	{
		Linear,
		Cubic,
		Cosine,
		Exponential
	};

	template<typename T>
	T PickInterpolation(T a, T b, float t, IntMethod interpolation)
	{
		switch (interpolation)
		{
		case vd::Linear:
			return Lerp(a, b, t);
			break;
		case vd::Cubic:
			return CubicInterpolate(a, b, t);
			break;
		case vd::Cosine:
			return CosineInterpolate(a, b, t);
		case vd::Exponential:
			return ExponentialInterpolate(a, b, t);
		default:
			break;
		}
	}

	template<typename T>
	T Lerp(T a, T b, float t)
	{
		return (1.0f - t) * a + t * b;
	}

	template<typename T>
	T CubicInterpolate(T a, T b, float t)
	{
		float t2 = t * t;
		float t3 = t2 * t;
		return a + (b - a) * (3 * t2 - 2 * t3);
	}

	template<typename T>
	T CosineInterpolate(T a, T b, float t) {
		float t2 = (1 - cos(t * 3.14159265358979323846f)) / 2;
		return a + (b - a) * t2;
	}

	template<typename T>
	T ExponentialInterpolate(T a, T b, float t, float k = 5.0f) {
		float s = (exp(k * t) - 1) / (exp(k) - 1);
		return a + (b - a) * s;
	}

	template<typename T>
	T InvLerp(T a, T b, T t)
	{
		return (t - a) / (b - a);
	}
}