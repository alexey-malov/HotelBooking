#include "HotelBookings.h"

HotelBookings::HotelBookings(Time timeSpan)
	: m_timeSpan(timeSpan)
{
}

void HotelBookings::Book(Time time, ClientId clientId, RoomCount roomCount)
{
	AddBooking(time, clientId, roomCount);
	RemoveBookingsDeprecatedBy(time - m_timeSpan);
}

size_t HotelBookings::GetDistinctClientCountWithinTimeSpan() const noexcept
{
	return m_clientBookingCount.size();
}

RoomCount HotelBookings::GetBookedRoomCountWithinTimeSpan() const noexcept
{
	return m_bookedRoomsWithinTimeSpan;
}

void HotelBookings::AddBooking(Time time, ClientId clientId, RoomCount roomCount)
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

void HotelBookings::RemoveBookingsDeprecatedBy(Time time) noexcept
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

void HotelBookings::DecrementClientBookingCount(ClientId clientId) noexcept
{
	if (auto it = m_clientBookingCount.find(clientId);
		(it != m_clientBookingCount.end() && (--it->second == 0))) // no client bookings within current time span
	{
		m_clientBookingCount.erase(it);
	}
}
