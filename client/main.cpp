#include <iostream>
#include <exception>

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>

#include "client.h"

using namespace SDL2pp;

int main(int argc, char* argv[])
{

	try {

		if (argc != 3)
		{
			std::cerr << "Usage: " << argv[0] << " <hostname or IP> <servicename or port>" << std::endl;
			return 1;
		}

		const char* hostname = argv[1];
		const char* servicename = argv[2];

		client client(hostname, servicename);
		client.run();

		return 0;
	} catch (std::exception& e) {
		// If case of error, print it and exit with error
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
