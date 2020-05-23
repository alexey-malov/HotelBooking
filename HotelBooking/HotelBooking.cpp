// HotelBooking.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "HotelBooking.h"

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
