#pragma once
#include <QDateTime>
#include <QString>

struct User_Data
    {
        int id = 0;
        QString username;
        QString email;
        QString first_name;
        QString last_name;
        QString phone_number;
        QDateTime last_login;
        bool is_authenticated = false;

        void clear()
        {
            id = 0;
            username.clear();
            email.clear();
            first_name.clear();
            last_name.clear();
            phone_number.clear();
            last_login = QDateTime();
            is_authenticated = false;
        }
    };