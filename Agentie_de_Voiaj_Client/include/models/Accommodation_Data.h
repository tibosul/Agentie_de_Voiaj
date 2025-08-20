#pragma once
#include <QString>

/**
 * Accommodation data structure for client side (Qt-compatible)
 * Matches the Accommodations database table and server Accommodation_Data structure
 * Database fields: Accommodation_ID, Name, Destination_ID, Type_of_Accommodation, 
 *                  Category, Address, Facilities, Rating, Description, 
 *                  Date_Created, Date_Modified
 */
struct Accommodation_Data
{
    int id = 0;                         // Accommodation_ID
    QString name;                       // Name
    int destination_id = 0;             // Destination_ID
    int accommodation_type_id = 0;      // Type_of_Accommodation
    QString category;                   // Category
    QString address;                    // Address
    QString facilities;                 // Facilities
    double rating = 0.0;                // Rating
    QString description;                // Description
    QString created_at;                 // Date_Created
    QString updated_at;                 // Date_Modified
    
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