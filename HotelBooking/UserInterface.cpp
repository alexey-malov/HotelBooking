#include "UserInterface.h"
#include "BookingService.h"
#include <sstream>

UserInterface::UserInterface(std::istream& input, std::ostream& output, BookingService& service)
	: m_input(input)
	, m_output(output)
	, m_service(service)
{
}

void UserInterface::Run()
{
	using namespace std;

	string line;
	getline(m_input, line);
	unsigned size = std::stoul(line);
	for (unsigned i = 0; i < size; ++i)
	{
		getline(m_input, line);
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

			m_service.Book(time, hotelName, clientId, roomCount);
		}
		else if (query == "CLIENTS"sv)
		{
			if (!(lineStream >> hotelName))
			{
				throw runtime_error("CLIENTS query syntax error");
			}

			m_output << m_service.GetDistinctClientCountWithinTimeSpan(hotelName) << "\n";
		}
		else if (query == "ROOMS"sv)
		{
			if (!(lineStream >> hotelName))
			{
				throw runtime_error("ROOMS query syntax error");
			}

			m_output << m_service.GetBookedRoomCountWithinTimeSpan(hotelName) << "\n";
		}
		else
		{
			throw runtime_error("Unknown query " + query);
		}
	}
}
