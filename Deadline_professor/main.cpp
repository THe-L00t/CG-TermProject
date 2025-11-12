#include "Engine.h"

int main(int argc, char** argv) {
	std::unique_ptr<Engine> en{};
	en->Initialize();
	en->Run();
}