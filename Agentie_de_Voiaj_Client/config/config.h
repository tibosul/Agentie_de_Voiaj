#pragma once
#include <QString>

namespace Config
{
    // Application Configuration
    namespace Application
    {
        const QString APP_NAME = "Agenție de Voiaj Client";
        const QString VERSION = "1.0.0";
        const QString ORGANIZATION = "Travel Agency";
        const QString APP_DOMAIN = "travelagency.local";

        constexpr bool DEBUG_MODE = true;
        constexpr bool ENABLE_LOGGING = true;
        constexpr bool REMEMBER_USER = true;
    }

    // Server Configuration
    namespace Server
    {
        const QString DEFAULT_HOST = "127.0.0.1";
        constexpr int DEFAULT_PORT = 8080;
        constexpr int CONNECTION_TIMEOUT_MS = 30000;  // 30 seconds
        constexpr int REQUEST_TIMEOUT_MS = 15000;     // 15 seconds
        constexpr int MAX_RETRIES = 3;
    }

    // UI Configuration
    namespace UI
    {
        constexpr int WINDOW_MIN_WIDTH = 1024;
        constexpr int WINDOW_MIN_HEIGHT = 768;
        constexpr int CARD_WIDTH = 300;
        constexpr int CARD_HEIGHT = 200;
        constexpr int ANIMATION_DURATION_MS = 300;
        
        const QString DEFAULT_THEME = "light";
        constexpr bool ENABLE_ANIMATIONS = true;
        constexpr bool ENABLE_NOTIFICATIONS = true;
    }

    // Cache Configuration
    namespace Cache
    {
        constexpr int DESTINATIONS_CACHE_HOURS = 24;
        constexpr int OFFERS_CACHE_MINUTES = 30;
        constexpr int MAX_CACHE_SIZE_MB = 50;
    }

    // Validation Configuration
    namespace Validation
    {
        constexpr int MIN_PASSWORD_LENGTH = 6;
        constexpr int MAX_PASSWORD_LENGTH = 100;
        constexpr int MAX_USERNAME_LENGTH = 50;
        constexpr int MAX_EMAIL_LENGTH = 100;
        constexpr int MAX_PERSONS_PER_BOOKING = 10;
    }

    // Messages
    namespace Messages
    {
        const QString CONNECTION_ERROR = "Nu s-a putut conecta la server";
        const QString LOGIN_SUCCESS = "Autentificare reușită";
        const QString LOGIN_FAILED = "Autentificare eșuată";
        const QString BOOKING_SUCCESS = "Rezervare realizată cu succes";
        const QString BOOKING_FAILED = "Rezervarea a eșuat";
        const QString NETWORK_ERROR = "Eroare de rețea";
        const QString INVALID_DATA = "Date invalide";
    }

    namespace Business
    {
        constexpr int MAX_PERSONS_PER_RESERVATION = 10;
        constexpr double MIN_PRICE_PER_PERSON = 50.0; // Minimum price for a booking
        constexpr double MAX_PRICE_PER_PERSON = 10000.0; // Maximum price for a booking
        constexpr int MAX_DAYS_IN_ADVANCE = 365; // Maximum days to book in advance
	}
}
