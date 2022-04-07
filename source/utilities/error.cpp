#include "StarSystemSim/utilities/error.h"

#include <iostream>
#include <cstdarg>

namespace utils {

	void fatalError(const char* format, ...)
	{
		char message[256];

		va_list ap;
		va_start(ap, format);
		vsprintf_s(message, sizeof(message), format, ap);
		va_end(ap);

		std::cerr << "\nFatalError: \n";
		std::cerr << message << std::endl;

		exit(1);
	}

	void printError(const char* format, ...)
	{
		char message[256];
		va_list ap;
		va_start(ap, format);
		vsprintf_s(message, sizeof(message), format, ap);
		va_end(ap);

		std::cerr << "Pear::ERROR: ";
		std::cerr << message << std::endl;
	}

}