#pragma once
#include "HotelBookings.h"

#define USE_UNORDERED_MAP_FOR_STORING_HOTELS

#ifdef USE_UNORDERED_MAP_FOR_STORING_HOTELS
template <typename Key, typename Value>
using HotelMapType = std::unordered_map<Key, Value>;
#else
template <typename Key, typename Value>
using HotelMapType = std::map<Key, Value>;
#endif

class BookingService final
{
public:
	explicit BookingService(Time statisticTimeSpan = 24 * 60 * 60);

	void Book(Time time, const std::string& hotelName, ClientId clientId, RoomCount roomCount);

	size_t GetDistinctClientCountWithinTimeSpan(const std::string& hotelName) const noexcept;

	RoomCount GetBookedRoomCountWithinTimeSpan(const std::string& hotelName) const noexcept;

private:
	const HotelBookings* FindHotelBookings(const std::string& hotelName) const noexcept;

	HotelBookings& GetHotelBookings(const std::string& hotelName);

	Time m_statisticTimeSpan;
	HotelMapType<std::string, HotelBookings> m_hotelBookings;
};
