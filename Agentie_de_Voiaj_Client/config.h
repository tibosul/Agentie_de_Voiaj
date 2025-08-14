#pragma once
#include <QString>

namespace Config
{
    // Application Configuration
    namespace Application
    {
        const QString APP_NAME = "Agentie de Voiaj - Client";
        const QString VERSION = "1.0.0";
        constexpr bool DEBUG_MODE = true;
        constexpr bool LOG_NETWORK_REQUESTS = true;
        constexpr bool LOG_JSON_MESSAGES = true;
        
        // File paths
        const QString CONFIG_DIRECTORY = "config/";
        const QString LOG_DIRECTORY = "logs/";
        const QString CACHE_DIRECTORY = "cache/";
    }

    // Network Configuration
    namespace Network
    {
        const QString DEFAULT_SERVER_HOST = "localhost";
        constexpr int DEFAULT_SERVER_PORT = 8080;
        constexpr int CONNECTION_TIMEOUT_MS = 30000; // 30 seconds
        constexpr int SOCKET_TIMEOUT_MS = 30000;     // 30 seconds
        constexpr int KEEPALIVE_INTERVAL_MS = 60000; // 60 seconds
        constexpr int MAX_RETRY_ATTEMPTS = 3;
        constexpr int RETRY_DELAY_MS = 1000;
        constexpr int BUFFER_SIZE = 4096;
    }

    // UI Configuration
    namespace UI
    {
        constexpr int MIN_WINDOW_WIDTH = 1000;
        constexpr int MIN_WINDOW_HEIGHT = 700;
        constexpr int DEFAULT_WINDOW_WIDTH = 1200;
        constexpr int DEFAULT_WINDOW_HEIGHT = 800;
        
        constexpr int STATUS_MESSAGE_TIMEOUT = 3000; // 3 seconds
        constexpr int PROGRESS_UPDATE_INTERVAL = 100; // milliseconds
        
        // Colors
        const QString PRIMARY_COLOR = "#4CAF50";
        const QString SECONDARY_COLOR = "#2196F3";
        const QString SUCCESS_COLOR = "#4CAF50";
        const QString ERROR_COLOR = "#f44336";
        const QString WARNING_COLOR = "#FF9800";
        const QString INFO_COLOR = "#2196F3";
    }

    // Validation Configuration
    namespace Validation
    {
        constexpr int MIN_USERNAME_LENGTH = 3;
        constexpr int MAX_USERNAME_LENGTH = 50;
        constexpr int MIN_PASSWORD_LENGTH = 6;
        constexpr int MAX_PASSWORD_LENGTH = 100;
        constexpr int MAX_NAME_LENGTH = 50;
        constexpr int MAX_EMAIL_LENGTH = 100;
        constexpr int MAX_PHONE_LENGTH = 20;
        
        // Regex patterns
        const QString EMAIL_PATTERN = "^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$";
        const QString PHONE_PATTERN = "^[0-9+\\-\\s\\(\\)]{8,15}$";
        const QString USERNAME_PATTERN = "^[a-zA-Z0-9_]{3,50}$";
    }

    // Business Logic Configuration
    namespace Business
    {
        constexpr int MAX_PERSONS_PER_RESERVATION = 10;
        constexpr double MIN_PRICE = 0.0;
        constexpr double MAX_PRICE = 50000.0;
        constexpr int MAX_SEARCH_RESULTS = 1000;
        constexpr int ITEMS_PER_PAGE = 50;
    }

    // Cache Configuration
    namespace Cache
    {
        constexpr int CACHE_EXPIRY_HOURS = 24;
        constexpr int MAX_CACHE_SIZE_MB = 50;
        constexpr bool ENABLE_CACHE = true;
    }

    // JSON Configuration
    namespace JSON
    {
        constexpr int MAX_JSON_SIZE = 1024 * 1024; // 1 MB
        constexpr bool PRETTY_PRINT = false; // For network efficiency
        constexpr bool VALIDATE_JSON = true;
    }

    // Error Messages
    namespace ErrorMessages
    {
        const QString CONNECTION_FAILED = "Nu s-a putut conecta la server";
        const QString CONNECTION_TIMEOUT = "Conexiunea a expirat";
        const QString AUTHENTICATION_FAILED = "Autentificare eșuată";
        const QString REGISTRATION_FAILED = "Înregistrare eșuată";
        const QString NETWORK_ERROR = "Eroare de rețea";
        const QString JSON_PARSE_ERROR = "Eroare la parsarea răspunsului serverului";
        const QString INVALID_CREDENTIALS = "Date de autentificare invalide";
        const QString SERVER_ERROR = "Eroare de server";
        const QString OPERATION_CANCELLED = "Operațiune anulată";
        const QString INVALID_INPUT = "Date de intrare invalide";
        const QString BOOKING_FAILED = "Rezervare eșuată";
        const QString CANCELLATION_FAILED = "Anulare eșuată";
        const QString UPDATE_FAILED = "Actualizare eșuată";
    }

    // Success Messages
    namespace SuccessMessages
    {
        const QString CONNECTION_SUCCESSFUL = "Conectat cu succes la server";
        const QString AUTHENTICATION_SUCCESSFUL = "Autentificare reușită";
        const QString REGISTRATION_SUCCESSFUL = "Înregistrare reușită";
        const QString BOOKING_SUCCESSFUL = "Rezervare reușită";
        const QString CANCELLATION_SUCCESSFUL = "Anulare reușită";
        const QString UPDATE_SUCCESSFUL = "Actualizare reușită";
        const QString DATA_LOADED = "Date încărcate cu succes";
        const QString LOGOUT_SUCCESSFUL = "Deconectare reușită";
    }

    // Status Messages
    namespace StatusMessages
    {
        const QString CONNECTING = "Se conectează...";
        const QString AUTHENTICATING = "Se autentifică...";
        const QString REGISTERING = "Se înregistrează...";
        const QString LOADING_DATA = "Se încarcă datele...";
        const QString PROCESSING = "Se procesează...";
        const QString BOOKING = "Se face rezervarea...";
        const QString CANCELLING = "Se anulează...";
        const QString UPDATING = "Se actualizează...";
        const QString DISCONNECTING = "Se deconectează...";
        const QString READY = "Gata";
        const QString CONNECTED = "Conectat";
        const QString DISCONNECTED = "Deconectat";
        const QString AUTHENTICATED = "Autentificat";
        const QString NOT_AUTHENTICATED = "Neautentificat";
    }
}