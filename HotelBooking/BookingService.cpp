#include "BookingService.h"

BookingService::BookingService(Time statisticTimeSpan)
	: m_statisticTimeSpan(statisticTimeSpan)
{
}

void BookingService::Book(Time time, const std::string& hotelName, ClientId clientId, RoomCount roomCount)
{
	GetHotelBookings(hotelName).Book(time, clientId, roomCount);
}

size_t BookingService::GetDistinctClientCountWithinTimeSpan(const std::string& hotelName) const noexcept
{
	auto optHotelBookings = FindHotelBookings(hotelName);
	return optHotelBookings ? optHotelBookings->GetDistinctClientCountWithinTimeSpan() : 0;
}

RoomCount BookingService::GetBookedRoomCountWithinTimeSpan(const std::string& hotelName) const noexcept
{
	auto optHotelBookings = FindHotelBookings(hotelName);
	return optHotelBookings ? optHotelBookings->GetBookedRoomCountWithinTimeSpan() : 0;
}

const HotelBookings* BookingService::FindHotelBookings(const std::string& hotelName) const noexcept
{
	auto it = m_hotelBookings.find(hotelName);
	return it != m_hotelBookings.end() ? &(it->second) : nullptr;
}

HotelBookings& BookingService::GetHotelBookings(const std::string& hotelName)
{
	auto it = m_hotelBookings.find(hotelName);
	if (it == m_hotelBookings.end())
	{
		it = m_hotelBookings.emplace(hotelName, m_statisticTimeSpan).first;
	}
	return it->second;
}
