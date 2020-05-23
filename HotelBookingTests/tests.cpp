#include "stdafx.h"
#include "../HotelBooking/HotelBooking.h"
#include <chrono>
#include <random>

using namespace std;
using namespace std::literals;

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

vector<string> GenerateHotels(unsigned count)
{
	const auto alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890"s;
	vector<string> hotels;
	hotels.reserve(count);
	uniform_int_distribution<size_t> dist(0, alphabet.size() - 1);
	mt19937 gen;
	for (unsigned i = 0; i < count; ++i)
	{
		string name;
		name.reserve(12);
		for (unsigned j = 0; j < 12; ++j)
		{
			name += alphabet[dist(gen)];
		}
		hotels.push_back(name);
	}
	return hotels;
}

vector<ClientId> GenerateClientIds(unsigned count)
{
	vector<ClientId> clients;
	clients.reserve(count);
	uniform_int_distribution<unsigned> dist(1, 999'999'999);
	mt19937 gen;
	for (unsigned i = 0; i < count; ++i)
	{
		clients.push_back(dist(gen));
	}
	return clients;
}

SCENARIO("Benchmark")
{
	auto hotels = GenerateHotels(1'000);
	auto clients = GenerateClientIds(20'000);
	mt19937 gen;
	uniform_int_distribution<size_t> randHotel(0, hotels.size() - 1);
	uniform_int_distribution<size_t> randClient(0, clients.size() - 1);
	uniform_int_distribution<RoomCount> randRoomCount(1, 1000);
	uniform_int_distribution<Time> randTimeDelta(0, 1000);
	Time time = 0;

	BookingService service;

	const auto beginTime = chrono::steady_clock::now();
	for (unsigned i = 0; i < 1000'000; ++i)
	{
		auto hotel = hotels[randHotel(gen)];
		auto client = clients[randClient(gen)];
		auto roomCount = randRoomCount(gen);
		time += randTimeDelta(gen);
		service.Book(time, hotel, client, roomCount);
	}
	const auto endTime = chrono::steady_clock::now();
	const auto duration = endTime - beginTime;
	std::cout << "Duration: " << std::chrono::duration_cast<chrono::milliseconds>(duration).count() << "\n";
}
