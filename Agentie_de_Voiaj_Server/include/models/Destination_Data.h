#pragma once

#include <string>

/**
 * Destination data structure matching the Destinations database table
 * Database fields: Destination_ID, Name, Country, Description, Image_Path, 
 *                  Date_Created, Date_Modified
 */
struct Destination_Data
{
	int id = 0;                    // Destination_ID
	std::string name;              // Name
	std::string country;           // Country
	std::string description;       // Description
	std::string image_path;        // Image_Path
	std::string created_at;        // Date_Created
	std::string updated_at;        // Date_Modified
	
	Destination_Data() : id(0) 
	{
	}
	
	// Clear all data
	void clear()
	{
		id = 0;
		name.clear();
		country.clear();
		description.clear();
		image_path.clear();
		created_at.clear();
		updated_at.clear();
	}
};