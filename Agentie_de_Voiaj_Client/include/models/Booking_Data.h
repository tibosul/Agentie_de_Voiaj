#pragma once
#include <QString>
#include <QVector>

struct Booking_Data
{
    int offer_id = 0;
    QString offer_name;
    QString destination;
    double price_per_person = 0.0;
    int number_of_persons = 1;
    double total_price = 0.0;
    QString departure_date;
    QString return_date;

    struct Person_Info
    {
        QString full_name;
        QString cnp;
        QString birth_date;
        QString person_type;
    };
    QVector<Person_Info> persons;
};
