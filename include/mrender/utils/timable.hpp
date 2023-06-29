#pragma once

#include <unordered_map>
#include <chrono>

namespace mrender {

template<typename T>
class Timer
{
public:
	Timer(std::string_view name, T&& function)
		: mName(name), mFunction(function), mStopped(false)
	{
		mStartTime = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		if (!mStopped)
		{
			auto endTime = std::chrono::high_resolution_clock::now();

			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(mStartTime).time_since_epoch().count();
			long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

			mStopped = true;

			float duration = (end - start) * 0.001f;
			mFunction(duration);
		}
	}

private:
	std::string_view mName;
	T mFunction;
	bool mStopped;

	std::chrono::time_point<std::chrono::steady_clock> mStartTime;
};

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](float time) { mProfileResults[name] = time; })

class Timable 
{
public:
	std::unordered_map<std::string_view, float> mProfileResults;
};

}	// namespace mrender