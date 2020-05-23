#pragma once
#include "HotelBookings.h"

/*
Determines whether to use unordered_map for storing hotels.
Search, insertion, and deletion in unordered_map have average complexity O(1), and O(N) in worst case
If there is a chance that hotel names often have hash collisions, then comment this macro to use std::map.
In this case O(Log(N)) complexity will be guaranteed when searching for hotel
*/
#define USE_UNORDERED_MAP_FOR_STORING_HOTELS

#ifdef USE_UNORDERED_MAP_FOR_STORING_HOTELS
template <typename Key, typename Value>
using HotelMapType = std::unordered_map<Key, Value>;
#else
#include <map>
template <typename Key, typename Value>
using HotelMapType = std::map<Key, Value>;
#endif

class BookingService final
{
public:
	explicit BookingService(Time statisticTimeSpan = 24 * 60 * 60);

	void Book(Time time, const std::string& hotelName, ClientId clientId, RoomCount roomCount);

	size_t GetDistinctClientCount(const std::string& hotelName) const noexcept;

	RoomCount GetBookedRoomCount(const std::string& hotelName) const noexcept;

private:
	const HotelBookings* FindHotelBookings(const std::string& hotelName) const noexcept;

	HotelBookings& GetHotelBookings(const std::string& hotelName);

	Time m_statisticTimeSpan;
	HotelMapType<std::string, HotelBookings> m_hotelBookings;
};
