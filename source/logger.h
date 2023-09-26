#ifndef LOGGER_H
#define LOGGER_H

#include <string>



namespace Logger {
	void Status(std::string message);
	void Warning(std::string message);
	void Error(std::string message);
}

#endif