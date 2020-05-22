#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

using namespace std;
using namespace std::literals;
using Time = int64_t;
using ClientId = uint32_t;
using RoomCount = unsigned;

class ClientBookingContext
{
public:
	explicit ClientBookingContext(Time statisticsTimeSpan)
		: m_statisticsTimeSpan(statisticsTimeSpan)
	{
	}

	void Book(Time time, ClientId clientId)
	{
		UnregisterBookinsUpTo(time - m_statisticsTimeSpan);
		RegisterNewBooking(time, clientId);
	}

	unsigned GetDistinctClientCountWithingTimespan() const
	{
		return m_distinctClientCountWithinTimeSpan;
	}

private:
	void RegisterNewBooking(Time /*time*/, ClientId clientId)
	{
		auto& clientBookingCounter = m_clientBookingCount[clientId];
		if (++clientBookingCounter == 1) // new client registered
		{
			++m_distinctClientCountWithinTimeSpan;
		}
	}
	void UnregisterBookinsUpTo(Time /*time*/)
	{
	}

	std::unordered_map<ClientId, unsigned> m_clientBookingCount;
	Time m_statisticsTimeSpan;
	unsigned m_distinctClientCountWithinTimeSpan = 0;
};

class HotelBookings
{
public:
	void Book(Time /*time*/, ClientId /*clientId*/, RoomCount /*roomCount*/)
	{
	}

	unsigned GetDistinctClientCountWithinLastDay() const
	{
		return 0;
	}

	unsigned GetBookedRoomCountWithinLastDay() const
	{
		return 0;
	}
};

class BookingService
{
public:
	void Book(Time time, const string& hotelName, ClientId clientId, RoomCount roomCount)
	{
		m_hotelBookings[hotelName].Book(time, clientId, roomCount);
	}

	unsigned GetDistinctClientCountWithinLastDay(const string& hotelName) const
	{
		if (auto it = m_hotelBookings.find(hotelName); it != m_hotelBookings.end())
		{
			return it->second.GetDistinctClientCountWithinLastDay();
		}
		return 0; // Not bookings in this hotel yet
	}

	unsigned GetBookedRoomCountWithinLastDay(const string& hotelName) const
	{
		if (auto it = m_hotelBookings.find(hotelName); it != m_hotelBookings.end())
		{
			return it->second.GetBookedRoomCountWithinLastDay();
		}
		return 0; // Not bookings in this hotel yet
	}

private:
	unordered_map<string, HotelBookings> m_hotelBookings;
};
