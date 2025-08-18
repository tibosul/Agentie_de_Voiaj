#pragma once
#include <string>

namespace Config
{
	// Server Configuration
	namespace Server
	{
		constexpr int PORT = 8080;
		constexpr int MAX_CONNECTIONS = 100;
		constexpr int BACKLOG_SIZE = 10;
		constexpr int BUFFER_SIZE = 4096;
		constexpr int SOCKET_TIMEOUT_MS = 30000; // 30 seconds
		constexpr bool ENABLE_KEEP_ALIVE = true; // Enable TCP keep-alive
	}

	// Database Configuration
	namespace Database
	{
		const std::string DEFAULT_SERVER = "localhost";
		const std::string DEFAULT_DATABASE = "Agentie_de_Voiaj";
		const std::string DRIVER = "ODBC Driver 17 for SQL Server";
		constexpr int CONNECTION_TIMEOUT = 30; // seconds
		constexpr int QUERY_TIMEOUT = 15; // seconds
		constexpr bool AUTO_COMMIT = true; // Auto-commit transactions

		// Connection string template - not used, build_connection_string() used instead
		const std::string CONNECTION_TEMPLATE =
			"DRIVER={" + DRIVER + "};"
			"SERVER=" + DEFAULT_SERVER + ";"
			"DATABASE=" + DEFAULT_DATABASE + ";"
			"Trusted_Connection=yes;"
			"Connection Timeout=" + std::to_string(CONNECTION_TIMEOUT) + ";";
	}

	// Application Configuration
	namespace Application
	{
		const std::string APP_NAME = "Agentie de Voiaj Server";
		const std::string VERSION = "1.0.0";
		constexpr bool DEBUG_MODE = true; // Enable debug mode for logging
		constexpr bool LOG_SQL_QUERIES = true; // Log SQL queries
		constexpr bool LOG_CLIENT_REQUESTS = true; // Log client requests

		// File paths
		const std::string LOG_DIRECTORY = "logs/";
		const std::string CONFIG_DIRECTORY = "config/";
		const std::string SQL_SCRIPTS_DIRECTORY = "sql/";
	}

	// JSON Message Configuration
	namespace JSON
	{
		constexpr int MAX_JSON_SIZE = 1024 * 1024; // 1 MB
		constexpr bool PRETTY_PRINT = true; // Enable pretty printing for JSON responses
		constexpr bool VALIDATE_SCHEMA = true; // Validate JSON against schema
	}

	// Security Configuration
	namespace Security
	{
		constexpr int MIN_PASSWORD_LENGTH = 6;
		constexpr int MAX_LOGIN_ATTEMPTS = 5;
		constexpr int LOCKOUT_DURATION_MINUTES = 15;
		constexpr bool REQUIRE_EMAIL_VALIDATION = false;
		// Salt is now per-user (username) for better security
	}

	// Business Logic Configuration
	namespace Business
	{
		constexpr int MAX_PERSONS_PER_RESERVATION = 10;
		constexpr double MIN_OFFER_PRICE = 1.0;
		constexpr double MAX_OFFER_PRICE = 50000.0;
		constexpr int MAX_RESERVATION_DURATION_DAYS = 365;
		constexpr int MIN_ADVANCE_BOOKING_DAYS = 1;
	}

	// Error Messages
	namespace ErrorMessages
	{
		const std::string DB_CONNECTION_FAILED = "Failed to connect to database";
		const std::string DB_QUERY_FAILED = "Database query failed";
		const std::string INVALID_JSON = "Invalid JSON format";
		const std::string AUTHENTICATION_FAILED = "Authentication failed";
		const std::string USER_NOT_FOUND = "User not found";
		const std::string OFFER_NOT_FOUND = "Offer not found";
		const std::string INSUFFICIENT_SEATS = "Not enough available seats";
		const std::string INVALID_REQUEST = "Invalid request format";
		const std::string SERVER_ERROR = "Internal server error";
		const std::string SOCKET_COMM_ERROR = "Socket communication error";
	}

	// Success Messages
	namespace SuccessMessages
	{
		const std::string USER_CREATED = "User created successfully";
		const std::string LOGIN_SUCCESS = "Login successful";
		const std::string RESERVATION_CREATED = "Reservation created successfully";
		const std::string RESERVATION_CANCELLED = "Reservation cancelled successfully";
		const std::string DATA_RETRIEVED = "Data retrieved successfully";
	}
}