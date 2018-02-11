#include "Managers\Message_Manager.h"
#include <shared_mutex>
#include <deque>
#include <iostream>

static shared_mutex message_log_mutex;
static deque<string> message_log;


/** A helper function that writes the message to a log and to the console. */
void text_output(const string & output)
{
	unique_lock<shared_mutex> write_guard(message_log_mutex);
	message_log.push_back(output);
	cout << output + "\n";
}

namespace MSG {
	void Statement(const string & input)
	{
		text_output(input);
	}

	void Error(const int & error_number, const string & input, const std::string & additional_input)
	{
		string error_message = Error_String[error_number];
		const int &spot = error_message.find('%');
		error_message.erase(error_message.begin() + spot, error_message.begin() + spot + 1);
		error_message.insert(spot, input);
		text_output(error_message + additional_input);
	}
}