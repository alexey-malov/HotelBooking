#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>

using Time = std::int64_t;
using ClientId = std::uint32_t;
using RoomCount = std::uint32_t;

class HotelBookings final
{
public:
	explicit HotelBookings(Time timeSpan);

	void Book(Time time, ClientId clientId, RoomCount roomCount);

	size_t GetDistinctClientCountWithinTimeSpan() const noexcept;

	RoomCount GetBookedRoomCountWithinTimeSpan() const noexcept;

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

	void AddBooking(Time time, ClientId clientId, RoomCount roomCount);
	void RemoveBookingsDeprecatedBy(Time time) noexcept;
	void DecrementClientBookingCount(ClientId clientId) noexcept;

	Time m_timeSpan;
	RoomCount m_bookedRoomsWithinTimeSpan = 0;

	std::deque<Booking> m_bookings; // Booking history within time span
	std::unordered_map<ClientId, unsigned> m_clientBookingCount;
};
