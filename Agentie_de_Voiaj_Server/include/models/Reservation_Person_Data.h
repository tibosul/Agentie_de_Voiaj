#pragma once

#include <string>

/**
 * Reservation person data structure matching the Reservation_Persons database table
 * Database fields: Reservation_Person_ID, Reservation_ID, Full_Name, CNP, 
 *                  Birth_Date, Person_Type
 */
struct Reservation_Person_Data
{
	int id = 0;                          // Reservation_Person_ID
	int reservation_id = 0;              // Reservation_ID
	std::string full_name;               // Full_Name
	std::string cnp;                     // CNP
	std::string birth_date;              // Birth_Date
	std::string person_type;             // Person_Type
	
	Reservation_Person_Data() : id(0), reservation_id(0) 
	{
	}
	
	// Clear all data
	void clear()
	{
		id = 0;
		reservation_id = 0;
		full_name.clear();
		cnp.clear();
		birth_date.clear();
		person_type.clear();
	}
	
	// Helper method to check if person is an adult
	bool is_adult() const
	{
		return person_type == "adult";
	}
	
	// Helper method to check if person is a child
	bool is_child() const
	{
		return person_type == "child";
	}
};