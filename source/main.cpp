#include <cstdlib>
#include <exception>

#include "logger.h"
#include "app.h"



int main(const int argc, const char ** argv)
{
	App app{"Vulkan Tests", 800, 600};

	try {
		app.Run();
	} catch (const std::exception &e) {
		Logger::Error("Error while running.");
	}

	Logger::Status("Exiting programm...");
	return EXIT_SUCCESS;
}