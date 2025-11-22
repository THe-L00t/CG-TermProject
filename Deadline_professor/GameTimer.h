#pragma once
#include <chrono>

class GameTimer
{
public:
	static GameTimer* Instance;

public:
	GameTimer();
	void Update();

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> prevTime;
public:
	float timer = 0;
	float elapsedTime = 0;
};