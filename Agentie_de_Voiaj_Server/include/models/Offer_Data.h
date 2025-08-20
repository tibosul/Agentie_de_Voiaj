#pragma once

#include <string>

/**
 * Offer data structure matching the Offers database table
 * Database fields: Offer_ID, Name, Destination_ID, Accommodation_ID, 
 *                  Types_of_Transport_ID, Price_per_Person, Duration_Days,
 *                  Departure_Date, Return_Date, Total_Seats, Reserved_Seats,
 *                  Included_Services, Description, Status, Date_Created, Date_Modified
 */
struct Offer_Data
{
	int id = 0;                           // Offer_ID
	std::string name;                     // Name
	int destination_id = 0;               // Destination_ID
	int accommodation_id = 0;             // Accommodation_ID
	int transport_type_id = 0;            // Types_of_Transport_ID
	double price_per_person = 0.0;        // Price_per_Person
	int duration_days = 0;                // Duration_Days
	std::string departure_date;           // Departure_Date
	std::string return_date;              // Return_Date
	int total_seats = 0;                  // Total_Seats
	int reserved_seats = 0;               // Reserved_Seats
	std::string included_services;        // Included_Services
	std::string description;              // Description
	std::string status;                   // Status
	std::string created_at;               // Date_Created
	std::string updated_at;               // Date_Modified
	
	Offer_Data() : id(0), destination_id(0), accommodation_id(0), transport_type_id(0), 
		price_per_person(0.0), duration_days(0), total_seats(0), reserved_seats(0) 
	{
	}
	
	// Clear all data
	void clear()
	{
		id = 0;
		name.clear();
		destination_id = 0;
		accommodation_id = 0;
		transport_type_id = 0;
		price_per_person = 0.0;
		duration_days = 0;
		departure_date.clear();
		return_date.clear();
		total_seats = 0;
		reserved_seats = 0;
		included_services.clear();
		description.clear();
		status.clear();
		created_at.clear();
		updated_at.clear();
	}
	
	// Helper method to get available seats
	int get_available_seats() const
	{
		return total_seats - reserved_seats;
	}
};