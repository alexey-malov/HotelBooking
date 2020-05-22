// HotelBooking.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "HotelBooking.h"

void Run(istream& input, ostream& output, BookingService& service)
{
	string line;
	getline(input, line);
	unsigned size = std::stoul(line);
	for (unsigned i = 0; i < size; ++i)
	{
		getline(input, line);
		istringstream lineStream(line);
		string query;
		lineStream >> query;

		string hotelName;
		if (query == "BOOK"sv)
		{
			Time time;
			ClientId clientId;
			RoomCount roomCount;
			if (!(lineStream >> time >> hotelName >> clientId >> roomCount))
			{
				throw runtime_error("BOOK query syntax error");
			}
			service.Book(time, hotelName, clientId, roomCount);
		}
		else if (query == "CLIENTS"sv)
		{
			if (!(lineStream >> hotelName))
			{
				throw runtime_error("CLIENTS query syntax error");
			}
			output << service.GetDistinctClientCountWithinLastDay(hotelName) << "\n";
		}
		else if (query == "ROOMS"sv)
		{
			if (!(lineStream >> hotelName))
			{
				throw runtime_error("ROOMS query syntax error");
			}
			output << service.GetBookedRoomCountWithinLastDay(hotelName) << "\n";
		}
		else
		{
			throw runtime_error("Unknown query " + query);
		}
	}
}

int main(int /*argc*/, char* /*argv*/[])
{
	try
	{
		BookingService service;
		Run(cin, cout, service);
		return EXIT_SUCCESS;
	}
	catch (const exception& e)
	{
		cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}
}
