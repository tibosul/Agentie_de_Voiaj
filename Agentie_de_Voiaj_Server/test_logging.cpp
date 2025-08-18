#include "utils/utils.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    std::cout << "=== TESTING LOGGING SYSTEM ===" << std::endl;
    
    // Initialize logging
    Utils::Logger::initialize_logging();
    
    // Test different log levels
    Utils::Logger::debug("This is a DEBUG message");
    Utils::Logger::info("This is an INFO message");
    Utils::Logger::warning("This is a WARNING message");
    Utils::Logger::error("This is an ERROR message");
    Utils::Logger::critical("This is a CRITICAL message");
    
    // Test authentication-like logging
    Utils::Logger::info("Authentication attempt for user: test_user from IP: 192.168.1.100");
    Utils::Logger::info("Authentication SUCCESS: User 'test_user' (ID:123) logged in from 192.168.1.100");
    
    // Test registration-like logging
    Utils::Logger::info("Registration attempt for user: new_user (test@email.com) from IP: 192.168.1.101");
    Utils::Logger::info("Registration SUCCESS: New user 'new_user' created successfully from 192.168.1.101");
    
    // Test server operations
    Utils::Logger::info("=== Server startup sequence ===");
    Utils::Logger::info("Initializing database manager...");
    Utils::Logger::info("Database connection successful");
    Utils::Logger::info("Creating server...");
    Utils::Logger::info("=== SERVER STARTED SUCCESSFULLY! ===");
    
    std::cout << "\n=== Logging test completed! Check logs/ folder ===" << std::endl;
    std::cout << "Log file should be: logs/server_" << Utils::DateTime::get_current_date() << ".log" << std::endl;
    
    return 0;
}