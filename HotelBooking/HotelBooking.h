#pragma once

#include <algorithm>
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
		RegisterNewBooking(time, clientId);
		UnregisterBookinsUpTo(time - m_statisticsTimeSpan);
	}

	unsigned GetDistinctClientCountWithinTimespan() const
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

class RoomBookingContext final
{
public:
	explicit RoomBookingContext(Time timeSpan)
		: m_timeSpan(timeSpan)
	{
	}

	void Book(Time time, unsigned roomCount)
	{
		AddRoomBookings(time, roomCount);
		RemoveBookingsStatsUpTo(time - m_timeSpan);
	}

	unsigned GetBookedRoomCountWithinTimeSpan() const
	{
		return GetBookedRoomCountWithinTimeSpanImpl();
	}

private:
	struct AccumulatedRoomBooking
	{
		explicit AccumulatedRoomBooking(Time time, unsigned accumulatedRoomCount = 0)
			: time(time)
			, accumulatedRoomCount(accumulatedRoomCount)
		{
		}
		Time time;
		unsigned accumulatedRoomCount;
	};

	void RemoveBookingsStatsUpTo(Time time)
	{
		auto it = std::find_if(m_accumulatedRoomBookings.begin() + m_statistingStartPos,
			m_accumulatedRoomBookings.end(), [time](auto&& booking) {
				return booking.time > time;
			});
		m_statistingStartPos = it - m_accumulatedRoomBookings.begin();
	}

	unsigned GetBookedRoomCountWithinTimeSpanImpl() const
	{
		if (m_accumulatedRoomBookings.empty())
		{
			return 0;
		}
		assert(m_statistingStartPos < m_accumulatedRoomBookings.size());
		auto roomsToIgnore = (m_statistingStartPos > 0)
			? m_accumulatedRoomBookings[m_statistingStartPos - 1].accumulatedRoomCount
			: 0;
		return m_accumulatedRoomBookings.back().accumulatedRoomCount - roomsToIgnore;
	}

	void AddRoomBookings(Time time, unsigned roomCount)
	{
		GetAccumulatedBooking(time).accumulatedRoomCount += roomCount;
	}

	AccumulatedRoomBooking& GetAccumulatedBooking(Time time)
	{
		if (!m_accumulatedRoomBookings.empty() && m_accumulatedRoomBookings.back().time == time)
		{
			return m_accumulatedRoomBookings.back();
		}

		// According to specification new events MUST not preceed existing ones
		assert(m_accumulatedRoomBookings.empty() || m_accumulatedRoomBookings.back().time < time);
		auto currentRoomCount = !m_accumulatedRoomBookings.empty()
			? m_accumulatedRoomBookings.back().accumulatedRoomCount
			: 0;
		return m_accumulatedRoomBookings.emplace_back(time, currentRoomCount);
	}

	Time m_timeSpan;
	std::vector<AccumulatedRoomBooking> m_accumulatedRoomBookings;
	size_t m_statistingStartPos = 0;
};

class HotelBookings final
{
public:
	explicit HotelBookings(Time statisticTimeSpan)
		: m_clientBookings(statisticTimeSpan)
		, m_roomBookings(statisticTimeSpan)
	{
	}

	void Book(Time time, ClientId clientId, RoomCount roomCount)
	{
		m_clientBookings.Book(time, clientId);
		m_roomBookings.Book(time, roomCount);
	}

	unsigned GetDistinctClientCountWithinTimeSpan() const
	{
		return m_clientBookings.GetDistinctClientCountWithinTimespan();
	}

	unsigned GetBookedRoomCountWithinTimeSpan() const
	{
		return m_roomBookings.GetBookedRoomCountWithinTimeSpan();
	}

private:
	ClientBookingContext m_clientBookings;
	RoomBookingContext m_roomBookings;
};

class BookingService final
{
public:
	explicit BookingService(Time statisticTimeSpan = 24 * 60 * 60)
		: m_statisticTimeSpan(statisticTimeSpan)
	{
	}

	void Book(Time time, const std::string& hotelName, ClientId clientId, RoomCount roomCount)
	{
		auto it = m_hotelBookings.find(hotelName);
		if (it == m_hotelBookings.end())
		{
			it = m_hotelBookings.emplace(hotelName, m_statisticTimeSpan).first;
		}
		it->second.Book(time, clientId, roomCount);
	}

	unsigned GetDistinctClientCountWithinTimeSpan(const std::string& hotelName) const
	{
		if (auto it = m_hotelBookings.find(hotelName); it != m_hotelBookings.end())
		{
			return it->second.GetDistinctClientCountWithinTimeSpan();
		}
		return 0; // Not bookings in this hotel yet
	}

	unsigned GetBookedRoomCountWithinTimeSpan(const std::string& hotelName) const
	{
		if (auto it = m_hotelBookings.find(hotelName); it != m_hotelBookings.end())
		{
			return it->second.GetBookedRoomCountWithinTimeSpan();
		}
		return 0; // Not bookings in this hotel yet
	}

private:
	Time m_statisticTimeSpan;
	std::unordered_map<std::string, HotelBookings> m_hotelBookings;
};
