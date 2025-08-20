#pragma once

#include <string>

/**
 * User data structure matching the Users database table
 * Database fields: User_ID, Username, Password_Hash, Password_Salt, Email, 
 *                  First_Name, Last_Name, Phone, Date_Created, Date_Modified
 */
struct User_Data
{
	int id = 0;                    // User_ID
	std::string username;          // Username
	std::string password_hash;     // Password_Hash
	std::string password_salt;     // Password_Salt
	std::string email;             // Email
	std::string first_name;        // First_Name
	std::string last_name;         // Last_Name
	std::string phone_number;      // Phone
	std::string created_at;        // Date_Created
	std::string updated_at;        // Date_Modified
	
	User_Data() : id(0) 
	{
	}
	
	// Clear all data
	void clear()
	{
		id = 0;
		username.clear();
		password_hash.clear();
		password_salt.clear();
		email.clear();
		first_name.clear();
		last_name.clear();
		phone_number.clear();
		created_at.clear();
		updated_at.clear();
	}
};