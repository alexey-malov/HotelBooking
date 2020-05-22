#include "stdafx.h"
#include "../HotelBooking/HotelBooking.h"

SCENARIO("Client Booking context")
{
	const Time timeSpan = 3;
	ClientBookingContext bookings(timeSpan);
	const ClientId client1 = 1;
	const ClientId client2 = 2;
	const ClientId client3 = 3;
	const ClientId client4 = 4;
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

	WHEN("when an existing booking is out of time span")
	{
		const Time t0 = 1;
		bookings.Book(t0, client1);
		bookings.Book(t0, client2);
		bookings.Book(t0 + timeSpan - 1, client2);
		bookings.Book(t0 + timeSpan - 1, client3);

		// [c1],[c2,c2],[c3]
		const auto clientCount = bookings.GetDistinctClientCountWithingTimespan();
		bookings.Book(t0 + timeSpan, client4);
		THEN("it is removed from statistics")
		{
			// [[c2], [c3], [c4]]
			CHECK(bookings.GetDistinctClientCountWithingTimespan() == clientCount);
			
			bookings.Book(t0 + timeSpan - 1 + (timeSpan - 1), client4);
			// Still [[c2], [c3], [c4]]
			CHECK(bookings.GetDistinctClientCountWithingTimespan() == clientCount);

			bookings.Book(t0 + timeSpan - 1 + timeSpan, client4);
			// Still [c4]]
			CHECK(bookings.GetDistinctClientCountWithingTimespan() == 1);
		}
	}
}
