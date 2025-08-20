#pragma once
#include <QString>

/**
 * Destination data structure for client side (Qt-compatible)
 * Matches the Destinations database table and server Destination_Data structure
 * Database fields: Destination_ID, Name, Country, Description, Image_Path, 
 *                  Date_Created, Date_Modified
 */
struct Destination_Data
{
    int id = 0;                         // Destination_ID
    QString name;                       // Name
    QString country;                    // Country
    QString description;                // Description
    QString image_path;                 // Image_Path
    QString created_at;                 // Date_Created
    QString updated_at;                 // Date_Modified
    
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