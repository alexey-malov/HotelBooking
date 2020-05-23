#pragma once
#include <iosfwd>

class BookingService;

class UserInterface
{
public:
	explicit UserInterface(std::istream& input, std::ostream& output, BookingService& service);

	void Run();

private:
	BookingService& m_service;
	std::istream& m_input;
	std::ostream& m_output;
};
