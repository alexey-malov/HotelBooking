#include "BookingService.h"
#include "UserInterface.h"
#include <iostream>

int main(int /*argc*/, char* /*argv*/[])
{
	using namespace std;

	try
	{
		BookingService service;
		UserInterface ui(cin, cout, service);
		ui.Run();
		return EXIT_SUCCESS;
	}
	catch (const exception& e)
	{
		cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}
}
