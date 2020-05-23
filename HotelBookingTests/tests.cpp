#include "stdafx.h"
#include "../HotelBooking/HotelBooking.h"

using namespace std;
using namespace std::literals;

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
		auto clientCount = bookings.GetDistinctClientCountWithinTimeSpan();
		bookings.Book(1, client1);
		THEN("client count must be incremented by 1")
		{
			CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == clientCount + 1);
		}
	}

	WHEN("the same client books within time span")
	{
		bookings.Book(1, client1);
		bookings.Book(1, client2);
		auto clientCount = bookings.GetDistinctClientCountWithinTimeSpan();
		bookings.Book(1, client1);
		THEN("client count must stay the same")
		{
			CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == clientCount);
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
		const auto clientCount = bookings.GetDistinctClientCountWithinTimeSpan();
		bookings.Book(t0 + timeSpan, client4);
		THEN("it is removed from statistics")
		{
			// [[c2], [c3], [c4]]
			CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == clientCount);

			bookings.Book(t0 + timeSpan - 1 + (timeSpan - 1), client4);
			// Still [[c2], [c3], [c4]]
			CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == clientCount);

			bookings.Book(t0 + timeSpan - 1 + timeSpan, client4);
			// Still [c4]]
			CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == 1);
		}
	}
}

SCENARIO("Room Booking Context")
{
	const Time timeSpan = 3;
	RoomBookingContext bookingContext(timeSpan);
	WHEN("New booking is registered")
	{
		const auto roomCount = bookingContext.GetBookedRoomCountWithinTimeSpan();
		const unsigned roomsToBook = 3;
		bookingContext.Book(1, roomsToBook);
		THEN("Room count is incremented")
		{
			CHECK(bookingContext.GetBookedRoomCountWithinTimeSpan() == roomCount + roomsToBook);
		}
	}

	WHEN("existing booking exits of time span")
	{
		const Time t0 = 0;
		const unsigned booking1Size = 1;
		const unsigned booking2Size = booking1Size << 2;
		const unsigned booking3Size = booking2Size << 2;
		const unsigned booking4Size = booking3Size << 2;
		const unsigned booking5Size = booking4Size << 2;
		const unsigned booking6Size = booking5Size;

		bookingContext.Book(t0, booking1Size);
		bookingContext.Book(t0, booking2Size);
		bookingContext.Book(t0 + timeSpan - 1, booking3Size);

		const auto bookedRoomsCount = bookingContext.GetBookedRoomCountWithinTimeSpan();

		CHECK(bookedRoomsCount == booking1Size + booking2Size + booking3Size);

		bookingContext.Book(t0 + timeSpan, booking4Size);
		THEN("its room count is subtracted from booked room count")
		{
			CHECK(bookingContext.GetBookedRoomCountWithinTimeSpan()
				== booking3Size + booking4Size);

			bookingContext.Book(t0 + timeSpan - 1 + timeSpan - 1, booking5Size);
			CHECK(bookingContext.GetBookedRoomCountWithinTimeSpan()
				== booking3Size + booking4Size + booking5Size);

			bookingContext.Book(t0 + timeSpan * 4, booking6Size);
			CHECK(bookingContext.GetBookedRoomCountWithinTimeSpan()
				== booking6Size);
		}
	}
}

SCENARIO("Hotel bookings")
{
	const Time timeSpan = 5;
	HotelBookings bookings(timeSpan);
	const ClientId client1 = 34;
	const ClientId client2 = 443;
	const ClientId client3 = 1234;
	const ClientId client4 = 100500;

	CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == 0);
	CHECK(bookings.GetBookedRoomCountWithinTimeSpan() == 0);

	bookings.Book(0, client1, 10);
	CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == 1);
	CHECK(bookings.GetBookedRoomCountWithinTimeSpan() == 10);

	bookings.Book(2, client1, 10);
	CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == 1);
	CHECK(bookings.GetBookedRoomCountWithinTimeSpan() == 20);

	bookings.Book(2, client2, 5);
	CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == 2);
	CHECK(bookings.GetBookedRoomCountWithinTimeSpan() == 25);

	bookings.Book(timeSpan - 1, client3, 20);
	CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == 3);
	CHECK(bookings.GetBookedRoomCountWithinTimeSpan() == 45);

	bookings.Book(timeSpan, client3, 2);
	CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == 3);
	CHECK(bookings.GetBookedRoomCountWithinTimeSpan() == 37);

	bookings.Book(timeSpan + timeSpan - 1, client4, 100);
	CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == 2);
	CHECK(bookings.GetBookedRoomCountWithinTimeSpan() == 102);

	bookings.Book(timeSpan * 10, client4, 3);
	CHECK(bookings.GetDistinctClientCountWithinTimeSpan() == 1);
	CHECK(bookings.GetBookedRoomCountWithinTimeSpan() == 3);
}

SCENARIO("Booking Service tests")
{
	const Time timeSpan = 5;

	const ClientId client1 = 34;
	const ClientId client2 = 443;
	const ClientId client3 = 1234;
	const ClientId client4 = 100500;

	const auto hotel1 = "Hilton"s;
	const auto hotel2 = "Radisoon"s;
	const auto hotel3 = "HolidayInn"s;

	BookingService service;

	CHECK(service.GetBookedRoomCountWithinTimeSpan(hotel1) == 0);
	CHECK(service.GetDistinctClientCountWithinTimeSpan(hotel2) == 0);

	service.Book(0, hotel1, client3, 3);
	CHECK(service.GetBookedRoomCountWithinTimeSpan(hotel1) == 3);
	CHECK(service.GetDistinctClientCountWithinTimeSpan(hotel1) == 1);
	CHECK(service.GetBookedRoomCountWithinTimeSpan(hotel2) == 0);
	CHECK(service.GetDistinctClientCountWithinTimeSpan(hotel2) == 0);
}

SCENARIO("User Interface")
{
	BookingService service(5);
	istringstream input(R"(9
BOOK -3 hilton 1234567890 8
CLIENTS hilton
ROOMS hilton
BOOK 0 hilton 5 1
CLIENTS hilton
ROOMS hilton
BOOK 2 hilton 1234567890 3
CLIENTS hilton
ROOMS hilton
)");
	ostringstream output;

	UserInterface ui(input, output, service);
	ui.Run();
	CHECK(output.str() == "1\n8\n2\n9\n2\n4\n"s);
}