#include "Engine.h"

int main(int argc, char** argv) {
	std::unique_ptr<Engine> en = std::make_unique<Engine>();
	en->Initialize(argc, argv);
	en->Run();
	return 0;
}