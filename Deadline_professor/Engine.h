#pragma once
#include "TotalHeader.h"
#include "window.h"
class Engine
{
public:
	void Initialize();
	void Run();
	void Update();

private:
	std::unique_ptr<window> w;
};

