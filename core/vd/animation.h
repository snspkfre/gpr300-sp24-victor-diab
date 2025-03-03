#include <glm/glm.hpp>
#include <imgui.h>
#include <algorithm>
#include <vector>

namespace vd
{
	template <typename T>
	class KeyFrame
	{
	public:
		float time; //Should be between 0 and animation clip maxTime
		T value;

		KeyFrame(float time, T value)
		{
			this->time = time;
			this->value = value;
		}

		KeyFrame()
		{
			time = 0;
		}
	};

	class AnimationClip
	{
	public:
		float duration;
		std::vector<KeyFrame<glm::vec3>> positionKeys;
		std::vector<KeyFrame<glm::vec3>> rotationKeys;
		std::vector<KeyFrame<glm::vec3>> scaleKeys;
	};

	class Animator
	{
	public:
		AnimationClip* clip = new AnimationClip;
		bool isPlaying = false;
		float playbackSpeed = 1;
		bool isLooping = false;
		float playbackTime = 0.0f;

		void Update(float dt)
		{
			if (isPlaying)
			{
				playbackTime += dt * playbackSpeed;
				if (playbackTime > clip->duration)
				{
					playbackTime = isLooping ? 0.0f : clip->duration;
					isPlaying = isLooping;
				}
				if (playbackTime < 0.0f)
				{
					playbackTime = isLooping ? clip->duration : 0.0f;
					isPlaying = isLooping;
				}
			}
		}

		template <typename T>
		T GetValue(std::vector<KeyFrame<T>> collection, T fallbackValue)
		{
			KeyFrame<T> next, prev;
			if (collection.size() <= 1)
				return fallbackValue;
			for (int i = 1; i < collection.size(); i++)
			{
				if (collection[i].time >= playbackTime)
				{
					next = collection[i];
					prev = collection[i - 1];
					break;
				}
			}
			
			return Lerp(prev.value, next.value, InvLerp(prev.time, next.time, playbackTime));
		}

		template<typename T>
		T Lerp(T a, T b, float t)
		{
			return (1.0f - t) * a + t * b;
		}

		template<typename T>
		T InvLerp(T a, T b, T t)
		{
			return (t - a) / (b - a);
		}
	};
}