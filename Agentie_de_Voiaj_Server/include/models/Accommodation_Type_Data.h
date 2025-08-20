#pragma once

#include <string>

/**
 * Accommodation type data structure matching the Types_of_Accommodation database table
 * Database fields: Accommodation_Type_ID, Name, Description, Date_Created, Date_Modified
 */
struct Accommodation_Type_Data
{
	int id = 0;                    // Accommodation_Type_ID
	std::string name;              // Name
	std::string description;       // Description
	std::string created_at;        // Date_Created
	std::string updated_at;        // Date_Modified
	
	Accommodation_Type_Data() : id(0) 
	{
	}
	
	// Clear all data
	void clear()
	{
		id = 0;
		name.clear();
		description.clear();
		created_at.clear();
		updated_at.clear();
	}
};