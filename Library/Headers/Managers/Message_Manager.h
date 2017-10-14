/*
	Message Manager

	- Manages engine related error message and statement outputs
	- Has handy enum and string lists with definitions
	- Can print raw text or a specific error
	- By default, prints via std::cout
	- Holds a log of text in case they need to be access by any external UI
*/

#pragma once
#ifndef MESSAGE_MANAGER
#define MESSAGE_MANAGER
#ifdef	MESSAGE_MANAGER_EXPORT
#define MESSAGE_MANAGER_API __declspec(dllexport)
#else
#define	MESSAGE_MANAGER_API __declspec(dllimport)
#endif

#include <string>

using namespace std;

namespace MSG {
	// Prints a raw string @input to the message log
	MESSAGE_MANAGER_API void Statement(const string &input);
	// Prints a formatted message using the error @error_number
	MESSAGE_MANAGER_API void Error(const int &error_number, const string &input, const std::string &additional_input = "");
}

enum Error_Enum
{
	FILE_MISSING,
	DIRECTORY_MISSING,
	FILE_CORRUPT,

	FBO_INCOMPLETE,
	SHADER_INCOMPLETE,
	PROGRAM_INCOMPLETE,

	ERROR_COUNT
};

static std::string Error_String[ERROR_COUNT] =
{
	"Error (0): The file % does not exist! ",
	"Error (1): The directory % does not exist! ",
	"Error (2): The file % is corrupt! ",

	"Error (3): A Framebuffer in the % is incomplete. ",
	"Error (4): The Shader file % could not compile. ",
	"Error (5): The Shader program % could not compile. "
};

#endif // MESSAGE_MANAGER