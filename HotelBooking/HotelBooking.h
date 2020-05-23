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

#define USE_UNORDERED_MAP_FOR_STORING_HOTELS

using namespace std::literals;
using Time = int64_t;
using ClientId = uint32_t;
using RoomCount = uint32_t;

#ifdef USE_UNORDERED_MAP_FOR_STORING_HOTELS
template <typename Key, typename Value>
using HotelMapType = std::unordered_map<Key, Value>;
#else
template <typename Key, typename Value>
using HotelMapType = std::map<Key, Value>;
#endif

class HotelBookings final
{
public:
	explicit HotelBookings(Time timeSpan)
		: m_timeSpan(timeSpan)
	{
	}

	void Book(Time time, ClientId clientId, RoomCount roomCount)
	{
		AddBooking(time, clientId, roomCount);
		RemoveBookingsDeprecatedBy(time - m_timeSpan);
	}

	size_t GetDistinctClientCountWithinTimeSpan() const noexcept
	{
		return m_clientBookingCount.size();
	}

	RoomCount GetBookedRoomCountWithinTimeSpan() const noexcept
	{
		return m_bookedRoomsWithinTimeSpan;
	}

private:
	struct Booking
	{
		Booking(Time time, ClientId clientId, RoomCount roomCount) noexcept
			: time(time)
			, clientId(clientId)
			, roomCount(roomCount)
		{
		}
		Time time;
		ClientId clientId;
		RoomCount roomCount;
	};

	void AddBooking(Time time, ClientId clientId, RoomCount roomCount)
	{
		m_bookings.emplace_back(time, clientId, roomCount);
		try
		{
			++m_clientBookingCount[clientId];
			m_bookedRoomsWithinTimeSpan += roomCount;
		}
		catch (...)
		{
			// Rollback booking history changes if m_clientBookingCount[] throws
			m_bookings.pop_back();
			throw;
		}
	}

	void RemoveBookingsDeprecatedBy(Time time) noexcept
	{
		auto endOfOutdatedBookings = std::find_if(m_bookings.begin(), m_bookings.end(),
			[time, this](auto&& booking) {
				if (booking.time > time)
				{
					return true;
				}
				else
				{
					DecrementClientBookingCount(booking.clientId);
					m_bookedRoomsWithinTimeSpan -= booking.roomCount;
					return false;
				}
			});
		m_bookings.erase(m_bookings.begin(), endOfOutdatedBookings);
	}

	void DecrementClientBookingCount(ClientId clientId) noexcept
	{
		if (auto it = m_clientBookingCount.find(clientId);
			(it != m_clientBookingCount.end() && (--it->second == 0))) // no client bookings within current time span
		{
			m_clientBookingCount.erase(it);
		}
	}

	Time m_timeSpan;
	RoomCount m_bookedRoomsWithinTimeSpan = 0;

	std::deque<Booking> m_bookings; // Booking history within time span
	std::unordered_map<ClientId, unsigned> m_clientBookingCount;
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

	size_t GetDistinctClientCountWithinTimeSpan(const std::string& hotelName) const noexcept
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
	HotelMapType<std::string, HotelBookings> m_hotelBookings;
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
