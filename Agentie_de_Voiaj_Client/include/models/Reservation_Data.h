#pragma once
#include <QString>

struct Reservation_Data
{
    int id = 0;
    int userId = 0;
    int offerId = 0;
    QString offerName;
    QString destination;
    int numberOfPersons = 0;
    double totalPrice = 0.0;
    QString reservationDate;
    QString status; // pending, confirmed, paid, cancelled
    QString notes;
};