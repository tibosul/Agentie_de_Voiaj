#pragma once
#include <QString>
#include <QVector>

/**
 * Booking data structure for client side (Qt-compatible)
 * Used during the booking process before creating a reservation
 * Contains offer information and person details for booking
 */
struct Booking_Data
{
    int offer_id = 0;                   // Offer_ID
    QString offer_name;                 // Offer name for display
    QString destination;                // Destination name for display
    double price_per_person = 0.0;      // Price per person
    int number_of_persons = 1;          // Number of persons
    double total_price = 0.0;           // Total calculated price
    QString departure_date;             // Departure date
    QString return_date;                // Return date
    
    /**
     * Person information structure
     * Matches Reservation_Person_Data fields for consistency
     */
    struct Person_Info
    {
        QString full_name;              // Full_Name
        QString cnp;                    // CNP
        QString birth_date;             // Birth_Date
        QString person_type;            // Person_Type (adult/child)
        
        Person_Info() = default;
        
        // Helper methods
        bool is_adult() const { return person_type == "adult"; }
        bool is_child() const { return person_type == "child"; }
    };
    
    QVector<Person_Info> persons;       // List of persons for this booking
    
    Booking_Data() : offer_id(0), price_per_person(0.0), number_of_persons(1), total_price(0.0)
    {
    }
    
    // Calculate total price based on persons and price per person
    void calculate_total_price()
    {
        total_price = price_per_person * number_of_persons;
    }
    
    // Clear all data
    void clear()
    {
        offer_id = 0;
        offer_name.clear();
        destination.clear();
        price_per_person = 0.0;
        number_of_persons = 1;
        total_price = 0.0;
        departure_date.clear();
        return_date.clear();
        persons.clear();
    }
};
