#pragma once
#include <QString>

struct Offer_Data
{
    int id = 0;
    QString name;
    QString destination;
    QString description;
    double price_per_person = 0.0;
    int duration_days = 0;
    QString departure_date;
    QString return_date;
    int available_seats = 0;
    QString image_path;
    QString included_services;
};
