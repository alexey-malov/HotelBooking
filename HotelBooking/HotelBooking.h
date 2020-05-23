#pragma once

#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#define USE_UNORDERED_MAP_AS_DICTIONARY

using namespace std::literals;
using Time = int64_t;
using ClientId = uint32_t;
using RoomCount = uint32_t;

#ifdef USE_UNORDERED_MAP_AS_DICTIONARY
template <typename Key, typename Value>
using Dictionary = std::unordered_map<Key, Value>;
#else
template <typename Key, typename Value>
using Dictionary = std::map<Key, Value>;
#endif

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
		UnregisterBookingsUpTo(time - m_statisticsTimeSpan);
	}

	unsigned GetDistinctClientCountWithinTimeSpan() const noexcept
	{
		return m_distinctClientCountWithinTimeSpan;
	}

private:
	struct ClientBooking
	{
		ClientBooking(Time time, ClientId clientId) noexcept
			: time(time)
			, clientId(clientId)
		{
		}
		Time time;
		ClientId clientId;
	};

	void RegisterNewBooking(Time time, ClientId clientId)
	{
		m_bookingHistory.emplace_back(time, clientId);

		try
		{
			auto& clientBookingCounter = m_clientBookingCount[clientId];
			if (++clientBookingCounter == 1) // new client registered
			{
				++m_distinctClientCountWithinTimeSpan;
			}
		}
		catch (...)
		{
			// Rollback booking history changes if m_clientBookingCount[] throws
			m_bookingHistory.pop_back();
			throw;
		}
	}

	void UnregisterBookingsUpTo(Time time) noexcept
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

	void UnregisterClientBooking(ClientId clientId) noexcept
	{
		auto& clientBookingCount = m_clientBookingCount[clientId];
		assert(clientBookingCount > 0);
		if (--clientBookingCount == 0) // no client bookings within current time span
		{
			--m_distinctClientCountWithinTimeSpan;
		}
	}

	Dictionary<ClientId, unsigned> m_clientBookingCount;
	Time m_statisticsTimeSpan;
	unsigned m_distinctClientCountWithinTimeSpan = 0;

	std::vector<ClientBooking> m_bookingHistory;
	size_t m_historyPointer = 0;
};

class RoomBookingContext final
{
public:
	explicit RoomBookingContext(Time timeSpan) noexcept
		: m_timeSpan(timeSpan)
	{
	}

	void Book(Time time, RoomCount roomCount)
	{
		AddRoomBookings(time, roomCount);
		RemoveBookingsStatsUpTo(time - m_timeSpan);
	}

	RoomCount GetBookedRoomCountWithinTimeSpan() const noexcept
	{
		return m_bookedRoomsWithinTimeSpan;
	}

private:
	struct RoomBooking
	{
		explicit RoomBooking(Time time, RoomCount roomCount = 0) noexcept
			: time(time)
			, roomCount(roomCount)
		{
		}
		Time time;
		RoomCount roomCount;
	};

	void RemoveBookingsStatsUpTo(Time time) noexcept
	{
		auto endOfOutdatedBookings = std::find_if(m_bookings.begin(), m_bookings.end(),
			[time, this](auto&& booking) {
				if (booking.time > time)
				{
					return true;
				}
				else
				{
					m_bookedRoomsWithinTimeSpan -= booking.roomCount;
					return false;
				}
			});
		m_bookings.erase(m_bookings.begin(), endOfOutdatedBookings);
	}

	void AddRoomBookings(Time time, RoomCount roomCount)
	{
		m_bookings.emplace_back(time, roomCount);
		m_bookedRoomsWithinTimeSpan += roomCount;
	}

	Time m_timeSpan;
	std::deque<RoomBooking> m_bookings;
	RoomCount m_bookedRoomsWithinTimeSpan = 0;
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

	unsigned GetDistinctClientCountWithinTimeSpan() const noexcept
	{
		return m_clientBookings.GetDistinctClientCountWithinTimeSpan();
	}

	RoomCount GetBookedRoomCountWithinTimeSpan() const noexcept
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
		GetHotelBookings(hotelName).Book(time, clientId, roomCount);
	}

	unsigned GetDistinctClientCountWithinTimeSpan(const std::string& hotelName) const noexcept
	{
		auto optHotelBookings = FindHotelBookings(hotelName);
		return optHotelBookings ? optHotelBookings->GetDistinctClientCountWithinTimeSpan() : 0;
	}

	RoomCount GetBookedRoomCountWithinTimeSpan(const std::string& hotelName) const noexcept
	{
		auto optHotelBookings = FindHotelBookings(hotelName);
		return optHotelBookings ? optHotelBookings->GetBookedRoomCountWithinTimeSpan() : 0;
	}

private:
	const HotelBookings* FindHotelBookings(const std::string& hotelName) const noexcept
	{
		auto it = m_hotelBookings.find(hotelName);
		return it != m_hotelBookings.end() ? &(it->second) : nullptr;
	}

	HotelBookings& GetHotelBookings(const std::string& hotelName)
	{
		auto it = m_hotelBookings.find(hotelName);
		if (it == m_hotelBookings.end())
		{
			it = m_hotelBookings.emplace(hotelName, m_statisticTimeSpan).first;
		}
		return it->second;
	}

	Time m_statisticTimeSpan;
	Dictionary<std::string, HotelBookings> m_hotelBookings;
};

class UserInterface
{
public:
	explicit UserInterface(std::istream& input, std::ostream& output, BookingService& service)
		: m_input(input)
		, m_output(output)
		, m_service(service)
	{
	}

	void Run()
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

private:
	BookingService& m_service;
	std::istream& m_input;
	std::ostream& m_output;
};
