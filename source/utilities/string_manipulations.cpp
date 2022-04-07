#include "StarSystemSim/utilities/string_manipulations.h"

#include <cstring>

namespace utils {

	int findWord(const char* text, const char* word)
	{
		std::string spring = text;
		std::string theWord = word;

		for (int64_t i = 0; i < spring.size(); i++)
		{
			if (spring[i] == theWord[0])
			{
				for (int64_t j = 0; j < theWord.size() || theWord[j] == ' '; j++)
				{
					if (spring[i + j] != theWord[j])
						break;

					if (j == theWord.size() - 1 || theWord[j + 1] == ' ')
						return i;
				}
			}
		}

		return -1;
	}

	std::string getLine(const char* text, unsigned int lineNo)
	{
		std::string finalLine = "";
		unsigned int currLineNo = 0;

		for (unsigned int i = 0; i < std::strlen(text) + 1; ++i)
		{
			if (text[i] == '\0')
				break;
			else if (text[i] == '\n')
			{
				currLineNo++;
			}
			else if (currLineNo == lineNo)
			{
				finalLine = finalLine + text[i];
			}
		}

		return finalLine;
	}

	unsigned int howManyLines(const char* text)
	{
		unsigned int lines = 1;

		for (unsigned int i = 0; i < std::strlen(text) + 1; ++i)
		{
			if (text[i] == '\0')
				break;
			else if (text[i] == '\n')
				lines++;
		}

		return lines;
	}

	const char* changeFileExtention(const char* filename, const char* new_extention)
	{
		std::string text = filename;
		std::string newName = "";

		for (int i = text.size() - 1; i >= 0; i--)
		{
			if (text[i] == '.')
			{
				for (int j = 0; j <= i; j++)
				{
					newName += text[j];
				}
				newName += new_extention;
				break;
			}
		}

		return newName.c_str();
	}

	FileExtention getFileExtention(const char* filename)
	{
		FileExtention extention = FileExtention::NONE;
		std::string wholeName = filename;
		std::string extentionString = "";

		for (int i = wholeName.size() - 1; i >= 0; i--)
		{
			if (wholeName[i] == '.')
			{
				for (int j = i + 1; j < wholeName.size(); j++)
					extentionString += wholeName[j];

				break;
			}
		}

		if (extentionString == "jpg" || extentionString == "jpeg")
			extention = FileExtention::JPEG;
		else if (extentionString == "png")
			extention = FileExtention::PNG;
		else if (extentionString == "bmp")
			extention = FileExtention::BMP;
		else if (extentionString == "ttf")
			extention = FileExtention::TTF;
		else if (extentionString == "texture")
			extention = FileExtention::TEXTURE;
		else
			extention = FileExtention::NONE;

		return extention;
	}

}