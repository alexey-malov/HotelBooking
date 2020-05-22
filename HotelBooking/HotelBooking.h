#pragma once

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using namespace std::literals;
using Time = int64_t;
using ClientId = uint32_t;
using RoomCount = unsigned;

class ClientBookingContext final
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
	struct ClientBooking
	{
		ClientBooking(Time time, ClientId clientId)
			: time(time)
			, clientId(clientId)
		{
		}
		Time time;
		ClientId clientId;
	};

	void RegisterNewBooking(Time time, ClientId clientId)
	{
		auto& clientBookingCounter = m_clientBookingCount[clientId];
		if (++clientBookingCounter == 1) // new client registered
		{
			++m_distinctClientCountWithinTimeSpan;
		}
		m_bookingHistory.emplace_back(time, clientId);
	}

	void UnregisterBookinsUpTo(Time time)
	{
		while (m_historyPointer < m_bookingHistory.size())
		{
			const auto& booking = m_bookingHistory[m_historyPointer];
			if (booking.time > time)
			{
				break;
			}
			UnregisterClientBooking(booking.clientId);
			++m_historyPointer;
		}
	}

	void UnregisterClientBooking(ClientId clientId)
	{
		auto& clientBookingCount = m_clientBookingCount[clientId];
		assert(clientBookingCount > 0);
		if (--clientBookingCount == 0) // no client bookings within current time span
		{
			--m_distinctClientCountWithinTimeSpan;
		}
	}

	std::unordered_map<ClientId, unsigned> m_clientBookingCount;
	Time m_statisticsTimeSpan;
	unsigned m_distinctClientCountWithinTimeSpan = 0;

	std::vector<ClientBooking> m_bookingHistory;
	size_t m_historyPointer = 0;
};

class HotelBookings final
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

class BookingService final
{
public:
	void Book(Time time, const std::string& hotelName, ClientId clientId, RoomCount roomCount)
	{
		m_hotelBookings[hotelName].Book(time, clientId, roomCount);
	}

	unsigned GetDistinctClientCountWithinLastDay(const std::string& hotelName) const
	{
		if (auto it = m_hotelBookings.find(hotelName); it != m_hotelBookings.end())
		{
			return it->second.GetDistinctClientCountWithinLastDay();
		}
		return 0; // Not bookings in this hotel yet
	}

	unsigned GetBookedRoomCountWithinLastDay(const std::string& hotelName) const
	{
		if (auto it = m_hotelBookings.find(hotelName); it != m_hotelBookings.end())
		{
			return it->second.GetBookedRoomCountWithinLastDay();
		}
		return 0; // Not bookings in this hotel yet
	}

private:
	std::unordered_map<std::string, HotelBookings> m_hotelBookings;
};
