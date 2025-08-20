#pragma once
#include <QString>

/**
 * Reservation person data structure for client side (Qt-compatible)
 * Matches the Reservation_Persons database table and server Reservation_Person_Data structure
 * Database fields: Reservation_Person_ID, Reservation_ID, Full_Name, CNP, 
 *                  Birth_Date, Person_Type
 */
struct Reservation_Person_Data
{
    int id = 0;                         // Reservation_Person_ID
    int reservation_id = 0;             // Reservation_ID
    QString full_name;                  // Full_Name
    QString cnp;                        // CNP
    QString birth_date;                 // Birth_Date
    QString person_type;                // Person_Type
    
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