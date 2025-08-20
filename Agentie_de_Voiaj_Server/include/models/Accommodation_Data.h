#pragma once

#include <string>

/**
 * Accommodation data structure matching the Accommodations database table
 * Database fields: Accommodation_ID, Name, Destination_ID, Type_of_Accommodation, 
 *                  Category, Address, Facilities, Rating, Description, 
 *                  Date_Created, Date_Modified
 */
struct Accommodation_Data
{
	int id = 0;                           // Accommodation_ID
	std::string name;                     // Name
	int destination_id = 0;               // Destination_ID
	int accommodation_type_id = 0;        // Type_of_Accommodation
	std::string category;                 // Category
	std::string address;                  // Address
	std::string facilities;               // Facilities
	double rating = 0.0;                  // Rating
	std::string description;              // Description
	std::string created_at;               // Date_Created
	std::string updated_at;               // Date_Modified
	
	Accommodation_Data() : id(0), destination_id(0), accommodation_type_id(0), rating(0.0) 
	{
	}
	
	// Clear all data
	void clear()
	{
		id = 0;
		name.clear();
		destination_id = 0;
		accommodation_type_id = 0;
		category.clear();
		address.clear();
		facilities.clear();
		rating = 0.0;
		description.clear();
		created_at.clear();
		updated_at.clear();
	}
};