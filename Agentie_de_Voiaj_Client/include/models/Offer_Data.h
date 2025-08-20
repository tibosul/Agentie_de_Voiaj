#pragma once
#include <QString>

/**
 * Offer data structure for client side (Qt-compatible)
 * Matches the Offers database table and server Offer_Data structure
 * Database fields: Offer_ID, Name, Destination_ID, Accommodation_ID, 
 *                  Types_of_Transport_ID, Price_per_Person, Duration_Days,
 *                  Departure_Date, Return_Date, Total_Seats, Reserved_Seats,
 *                  Included_Services, Description, Status, Date_Created, Date_Modified
 */
struct Offer_Data
{
    int id = 0;                         // Offer_ID
    QString name;                       // Name
    int destination_id = 0;             // Destination_ID
    QString destination;                // Destination name (for display)
    int accommodation_id = 0;           // Accommodation_ID
    int transport_type_id = 0;          // Types_of_Transport_ID
    double price_per_person = 0.0;      // Price_per_Person
    int duration_days = 0;              // Duration_Days
    QString departure_date;             // Departure_Date
    QString return_date;                // Return_Date
    int total_seats = 0;                // Total_Seats
    int reserved_seats = 0;             // Reserved_Seats
    QString included_services;          // Included_Services
    QString description;                // Description
    QString status;                     // Status
    QString created_at;                 // Date_Created
    QString updated_at;                 // Date_Modified
    
    // Client-specific display fields
    QString image_path;
    
    Offer_Data() : id(0), destination_id(0), accommodation_id(0), transport_type_id(0),
        price_per_person(0.0), duration_days(0), total_seats(0), reserved_seats(0)
    {
    }
    
    // Helper method to get available seats
    int available_seats() const
    {
        return total_seats - reserved_seats;
    }
    
    // Helper method to check if offer is available
    bool is_available() const
    {
        return status == "active" && available_seats() > 0;
    }
};
