#include "StarSystemSim/utilities/load_text_file.h"
#include "StarSystemSim/utilities/error.h"

#include <fstream>

namespace utils {

	std::string loadTextFile(const char* path)
	{
		std::string text = "";

		std::fstream file(path);
		if (file.fail())
		{
			printError("Failed to load text file (\"%s\")!\n", path);
		}
		else
		{
			std::string line = "";

			while (std::getline(file, line))
			{
				text = text + line + '\n';
			}
		}

		return text;
	}

}