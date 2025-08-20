#pragma once
#include <QString>

/**
 * Reservation data structure for client side (Qt-compatible)
 * Matches the Reservations database table and server Reservation_Data structure
 * Database fields: Reservation_ID, User_ID, Offer_ID, Number_of_Persons, 
 *                  Total_Price, Reservation_Date, Status, Notes
 */
struct Reservation_Data
{
    int id = 0;                         // Reservation_ID
    int userId = 0;                     // User_ID (camelCase for client consistency)
    int offerId = 0;                    // Offer_ID (camelCase for client consistency)
    int numberOfPersons = 0;            // Number_of_Persons (camelCase for client consistency)
    double totalPrice = 0.0;            // Total_Price (camelCase for client consistency)
    QString reservationDate;            // Reservation_Date (camelCase for client consistency)
    QString status;                     // Status (pending, confirmed, paid, cancelled)
    QString notes;                      // Notes
    
    // Client-specific display fields
    QString offerName;                  // For display purposes
    QString destination;                // For display purposes

    Reservation_Data() : id(0), userId(0), offerId(0), numberOfPersons(0), totalPrice(0.0)
    {
    }
    
    // Helper method to check if reservation is active
    bool is_active() const
    {
        return status == "confirmed" || status == "paid" || status == "pending";
    }
    
    // Helper method to check if reservation is cancelled
    bool is_cancelled() const
    {
        return status == "cancelled";
    }
    
    // Clear all data
    void clear()
    {
        id = 0;
        userId = 0;
        offerId = 0;
        numberOfPersons = 0;
        totalPrice = 0.0;
        reservationDate.clear();
        status.clear();
        notes.clear();
        offerName.clear();
        destination.clear();
    }
};