#pragma once

#include <string>

/**
 * Reservation data structure matching the Reservations database table
 * Database fields: Reservation_ID, User_ID, Offer_ID, Number_of_Persons, 
 *                  Total_Price, Reservation_Date, Status, Notes
 */
struct Reservation_Data
{
	int id = 0;                          // Reservation_ID
	int user_id = 0;                     // User_ID
	int offer_id = 0;                    // Offer_ID
	int number_of_persons = 0;           // Number_of_Persons
	double total_price = 0.0;            // Total_Price
	std::string reservation_date;        // Reservation_Date
	std::string status;                  // Status
	std::string notes;                   // Notes
	
	Reservation_Data() : id(0), user_id(0), offer_id(0), number_of_persons(0), total_price(0.0) 
	{
	}
	
	// Clear all data
	void clear()
	{
		id = 0;
		user_id = 0;
		offer_id = 0;
		number_of_persons = 0;
		total_price = 0.0;
		reservation_date.clear();
		status.clear();
		notes.clear();
	}
	
	// Helper method to check if reservation is active
	bool is_active() const
	{
		return status == "confirmed" || status == "paid" || status == "pending";
	}
	
	// Helper method to check if reservation is cancelled
	bool is_cancelled() const
	{
		return status == "cancelled";
	}
};