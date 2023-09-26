#include "logger.h"

#include <ctime>



namespace Logger {
	void Status(std::string message)
	{
		std::time_t result = std::time(nullptr);
		std::string timestamp = std::asctime(std::localtime(&result));
		timestamp.pop_back();
		printf("[%s][STATUS] %s\n", timestamp.c_str(), message.c_str());
	}



	void Warning(std::string message)
	{
		std::time_t result = std::time(nullptr);
		std::string timestamp = std::asctime(std::localtime(&result));
		timestamp.pop_back();
		printf("[%s][WARNING] %s\n", timestamp.c_str(), message.c_str());
	}



	void Error(std::string message)
	{
		std::time_t result = std::time(nullptr);
		std::string timestamp = std::asctime(std::localtime(&result));
		timestamp.pop_back();
		printf("[%s][ERROR] %s\n", timestamp.c_str(), message.c_str());
	}
}