#include "stdafx.h"
#include "../HotelBooking/HotelBooking.h"

SCENARIO("Client Booking context")
{
	ClientBookingContext bookings(3);
	ClientId client1 = 1;
	ClientId client2 = 2;
	ClientId client3 = 3;
	WHEN("New client books")
	{
		auto clientCount = bookings.GetDistinctClientCountWithingTimespan();
		bookings.Book(1, client1);
		THEN("client count must be incremented by 1")
		{
			CHECK(bookings.GetDistinctClientCountWithingTimespan() == clientCount + 1);
		}
	}

	WHEN("the same client books within time span")
	{
		bookings.Book(1, client1);
		bookings.Book(1, client2);
		auto clientCount = bookings.GetDistinctClientCountWithingTimespan();
		bookings.Book(1, client1);
		THEN("client count must stay the same")
		{
			CHECK(bookings.GetDistinctClientCountWithingTimespan() == clientCount);
		}
	}
}
