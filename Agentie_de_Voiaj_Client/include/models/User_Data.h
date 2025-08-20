#pragma once
#include <QDateTime>
#include <QString>

/**
 * User data structure for client side (Qt-compatible)
 * Matches the Users database table and server User_Data structure
 * Database fields: User_ID, Username, Password_Hash, Password_Salt, Email, 
 *                  First_Name, Last_Name, Phone, Date_Created, Date_Modified
 */
struct User_Data
{
    int id = 0;                         // User_ID
    QString username;                   // Username
    QString password_hash;              // Password_Hash (client usually doesn't store this)
    QString password_salt;              // Password_Salt (client usually doesn't store this)
    QString email;                      // Email
    QString first_name;                 // First_Name
    QString last_name;                  // Last_Name
    QString phone_number;               // Phone
    QString created_at;                 // Date_Created
    QString updated_at;                 // Date_Modified
    
    // Client-specific fields
    QDateTime last_login;
    bool is_authenticated = false;

    User_Data() : id(0), is_authenticated(false)
    {
    }

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
        last_login = QDateTime();
        is_authenticated = false;
    }
};