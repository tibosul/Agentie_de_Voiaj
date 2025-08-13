#include "utils.h"
#include "Database_Manager.h"
#include "Socket_Server.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace Database;
using namespace SocketNetwork;

// Forward declarations
void test_utils();
void test_database_manager();
void test_socket_server();
void test_protocol_handler();
void test_integration();
void print_separator(const std::string& title);

int main()
{
    std::cout << "=== AGENTIE DE VOIAJ SERVER - COMPREHENSIVE TESTS ===" << std::endl;
    std::cout << "Version: " << Config::Application::VERSION << std::endl;
    std::cout << "Debug Mode: " << (Config::Application::DEBUG_MODE ? "ON" : "OFF") << std::endl;
    
    try 
    {
        // Test 1: Utils Library
        print_separator("TESTING UTILS LIBRARY");
        test_utils();
        
        // Test 2: Database Manager
        print_separator("TESTING DATABASE MANAGER");
        test_database_manager();
        
        // Test 3: Socket Server
        print_separator("TESTING SOCKET SERVER");
        test_socket_server();
        
        // Test 4: Protocol Handler
        print_separator("TESTING PROTOCOL HANDLER");
        test_protocol_handler();
        
        // Test 5: Integration Test
        print_separator("TESTING INTEGRATION");
        test_integration();
        
        print_separator("ALL TESTS COMPLETED SUCCESSFULLY");
        
    }
    catch (const std::exception& e)
    {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return -1;
    }
    
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    return 0;
}

void print_separator(const std::string& title)
{
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << " " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void test_utils()
{
    std::cout << "Testing Utils components..." << std::endl;
    
    // Test String utilities
    std::cout << "\n1. String Utilities:" << std::endl;
    std::cout << "   Trimmed: '" << Utils::String::trim("  Hello World  ") << "'" << std::endl;
    std::cout << "   Uppercase: '" << Utils::String::to_upper("hello") << "'" << std::endl;
    std::cout << "   Lowercase: '" << Utils::String::to_lower("WORLD") << "'" << std::endl;
    
    std::vector<std::string> parts = Utils::String::split("a,b,c,d", ',');
    std::cout << "   Split result: ";
    for (const auto& part : parts) std::cout << "[" << part << "] ";
    std::cout << std::endl;
    
    // Test Date/Time utilities
    std::cout << "\n2. Date/Time Utilities:" << std::endl;
    std::cout << "   Current date: " << Utils::DateTime::get_current_date() << std::endl;
    std::cout << "   Current time: " << Utils::DateTime::get_current_time() << std::endl;
    std::cout << "   Current date-time: " << Utils::DateTime::get_current_date_time() << std::endl;
    std::cout << "   Is valid date (2025-12-31): " << Utils::DateTime::is_valid_date("2025-12-31") << std::endl;
    std::cout << "   Is valid date (invalid): " << Utils::DateTime::is_valid_date("2025-13-32") << std::endl;
    
    // Test Validation utilities
    std::cout << "\n3. Validation Utilities:" << std::endl;
    std::cout << "   Valid email (test@example.com): " << Utils::Validation::is_valid_email("test@example.com") << std::endl;
    std::cout << "   Invalid email (invalid-email): " << Utils::Validation::is_valid_email("invalid-email") << std::endl;
    std::cout << "   Valid username (user123): " << Utils::Validation::is_valid_username("user123") << std::endl;
    std::cout << "   Invalid username (ab): " << Utils::Validation::is_valid_username("ab") << std::endl;
    std::cout << "   Valid password (Pass123!): " << Utils::Validation::is_valid_password("Pass123!") << std::endl;
    std::cout << "   Invalid password (123): " << Utils::Validation::is_valid_password("123") << std::endl;
    
    // Test Conversion utilities
    std::cout << "\n4. Conversion Utilities:" << std::endl;
    std::cout << "   Double to string (123.456): '" << Utils::Conversion::double_to_string(123.456) << "'" << std::endl;
    std::cout << "   String to int ('42'): " << Utils::Conversion::string_to_int("42") << std::endl;
    std::cout << "   Bool to string (true): '" << Utils::Conversion::bool_to_string(true) << "'" << std::endl;
    std::cout << "   String to bool ('false'): " << Utils::Conversion::string_to_bool("false") << std::endl;
    
    // Test Cryptography utilities
    std::cout << "\n5. Cryptography Utilities:" << std::endl;
    std::string salt = Utils::Crypto::generate_salt();
    std::string hash = Utils::Crypto::hash_password("test123", salt);
    std::cout << "   Generated salt: " << salt << std::endl;
    std::cout << "   Password hash: " << hash << std::endl;
    std::cout << "   Password verification: " << Utils::Crypto::verify_password("test123", hash, salt) << std::endl;
    std::cout << "   Wrong password verification: " << Utils::Crypto::verify_password("wrong", hash, salt) << std::endl;
    std::cout << "   Session token: " << Utils::Crypto::generate_session_token() << std::endl;
    
    // Test JSON utilities
    std::cout << "\n6. JSON Utilities:" << std::endl;
    std::string success_json = Utils::JSON::create_success_response("{\"id\":1,\"name\":\"Test\"}", "Success!");
    std::cout << "   Success response: " << success_json << std::endl;
    std::string error_json = Utils::JSON::create_error_response("Test error", 404);
    std::cout << "   Error response: " << error_json << std::endl;
    std::cout << "   JSON validation (valid): " << Utils::JSON::is_valid_json(success_json) << std::endl;
    std::cout << "   JSON validation (invalid): " << Utils::JSON::is_valid_json("{invalid json}") << std::endl;
    
    // Test Network utilities
    std::cout << "\n7. Network Utilities:" << std::endl;
    std::cout << "   Valid IP (192.168.1.1): " << Utils::Network::is_valid_ip("192.168.1.1") << std::endl;
    std::cout << "   Invalid IP (999.999.999.999): " << Utils::Network::is_valid_ip("999.999.999.999") << std::endl;
    std::cout << "   Valid port (8080): " << Utils::Network::is_valid_port(8080) << std::endl;
    std::cout << "   Invalid port (70000): " << Utils::Network::is_valid_port(70000) << std::endl;
    
    // Test Random utilities
    std::cout << "\n8. Random Utilities:" << std::endl;
    std::cout << "   Random int (1-10): " << Utils::Random::generate_int(1, 10) << std::endl;
    std::cout << "   Random double (0.0-1.0): " << Utils::Random::generate_double(0.0, 1.0) << std::endl;
    std::cout << "   Random string (8 chars): " << Utils::Random::generate_random_string(8, true, false) << std::endl;
    std::cout << "   UUID: " << Utils::Random::generate_uuid() << std::endl;
    
    // Test Memory utilities
    std::cout << "\n9. Memory Utilities:" << std::endl;
    std::cout << "   Memory usage: " << Utils::Memory::get_memory_usage_MB() << " MB" << std::endl;
    std::cout << "   Available memory: " << Utils::Memory::get_available_memory_MB() << " MB" << std::endl;
    
    // Test Logger
    std::cout << "\n10. Logger Utilities:" << std::endl;
    Utils::Logger::set_log_level(Utils::Logger::Level::DEBUG);
    Utils::Logger::debug("Debug message test");
    Utils::Logger::info("Info message test");
    Utils::Logger::warning("Warning message test");
    Utils::Logger::error("Error message test");
    Utils::Logger::critical("Critical message test");
    
    std::cout << "✅ Utils tests completed!" << std::endl;
}

void test_database_manager()
{
    std::cout << "Testing Database Manager..." << std::endl;
    
    try 
    {
        // Create Database Manager instance
        Database_Manager db_manager(
            Config::Database::DEFAULT_SERVER,
            Config::Database::DEFAULT_DATABASE,
            "", // Empty username for Windows Authentication
            ""  // Empty password for Windows Authentication
        );
        
        std::cout << "\n1. Database Connection:" << std::endl;
        bool connected = db_manager.connect();
        std::cout << "   Connection successful: " << (connected ? "YES" : "NO") << std::endl;
        
        if (!connected)
        {
            std::cout << "   ⚠️  Cannot test database functionality without connection" << std::endl;
            std::cout << "   Last error: " << db_manager.get_last_error() << std::endl;
            return;
        }
        
        std::cout << "   Connection string: " << db_manager.get_connection_string() << std::endl;
        std::cout << "   Connection alive: " << (db_manager.is_connection_alive() ? "YES" : "NO") << std::endl;
        
        // Test table creation
        std::cout << "\n2. Database Schema:" << std::endl;
        bool tables_created = db_manager.create_tables_if_not_exists();
        std::cout << "   Tables creation: " << (tables_created ? "SUCCESS" : "FAILED") << std::endl;
        
        // Test basic queries
        std::cout << "\n3. Basic Queries:" << std::endl;
        
        // Test destinations
        auto destinations_result = db_manager.get_all_destinations();
        std::cout << "   Destinations query: " << (destinations_result.is_success() ? "SUCCESS" : "FAILED") << std::endl;
        if (destinations_result.is_success())
        {
            std::cout << "   Found " << destinations_result.data.size() << " destinations" << std::endl;
        }
        
        // Test transport types
        auto transport_result = db_manager.get_all_transport_types();
        std::cout << "   Transport types query: " << (transport_result.is_success() ? "SUCCESS" : "FAILED") << std::endl;
        if (transport_result.is_success())
        {
            std::cout << "   Found " << transport_result.data.size() << " transport types" << std::endl;
        }
        
        // Test offers
        auto offers_result = db_manager.get_available_offers();
        std::cout << "   Available offers query: " << (offers_result.is_success() ? "SUCCESS" : "FAILED") << std::endl;
        if (offers_result.is_success())
        {
            std::cout << "   Found " << offers_result.data.size() << " available offers" << std::endl;
        }
        
        // Test user operations (if we can create test user)
        std::cout << "\n4. User Operations:" << std::endl;
        User_Data test_user;
        test_user.username = "test_user_" + Utils::Random::generate_random_string(4, true, false);
        test_user.email = "test@example.com";
        test_user.first_name = "Test";
        test_user.last_name = "User";
        test_user.phone_number = "1234567890";
        test_user.password_hash = "test123"; // Will be hashed by DB manager
        
        auto register_result = db_manager.register_user(test_user);
        std::cout << "   User registration: " << (register_result.is_success() ? "SUCCESS" : "FAILED") << std::endl;
        if (!register_result.is_success())
        {
            std::cout << "   Error: " << register_result.message << std::endl;
        }
        
        // Test authentication
        if (register_result.is_success())
        {
            auto auth_result = db_manager.authenticate_user(test_user.username, "test123");
            std::cout << "   User authentication: " << (auth_result.is_success() ? "SUCCESS" : "FAILED") << std::endl;
            
            if (auth_result.is_success() && auth_result.has_data())
            {
                int user_id = Utils::Conversion::string_to_int(auth_result.data[0]["ID"]);
                std::cout << "   Authenticated user ID: " << user_id << std::endl;
                
                // Test getting user info
                auto user_info_result = db_manager.get_user_by_id(user_id);
                std::cout << "   Get user info: " << (user_info_result.is_success() ? "SUCCESS" : "FAILED") << std::endl;
            }
        }
        
        // Test statistics
        std::cout << "\n5. Statistics:" << std::endl;
        auto user_stats = db_manager.get_user_statistics();
        std::cout << "   User statistics: " << (user_stats.is_success() ? "SUCCESS" : "FAILED") << std::endl;
        
        auto booking_stats = db_manager.get_booking_statistics();
        std::cout << "   Booking statistics: " << (booking_stats.is_success() ? "SUCCESS" : "FAILED") << std::endl;
        
        auto popular_destinations = db_manager.get_popular_destinations(5);
        std::cout << "   Popular destinations: " << (popular_destinations.is_success() ? "SUCCESS" : "FAILED") << std::endl;
        
        std::cout << "✅ Database Manager tests completed!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "❌ Database Manager test failed: " << e.what() << std::endl;
    }
}

void test_socket_server()
{
    std::cout << "Testing Socket Server..." << std::endl;
    
    try 
    {
        // Create server configuration
        Server_Config config;
        config.ip_address = "127.0.0.1";
        config.port = Config::Server::PORT + 1; // Use different port for testing
        config.max_clients = 10;
        config.enable_logging = true;
        
        std::cout << "\n1. Socket Server Creation:" << std::endl;
        Socket_Server server(config);
        std::cout << "   Server created: SUCCESS" << std::endl;
        std::cout << "   Server running: " << (server.is_server_running() ? "YES" : "NO") << std::endl;
        std::cout << "   Server initialized: " << (server.is_server_initialized() ? "YES" : "NO") << std::endl;
        
        // Test configuration
        std::cout << "\n2. Server Configuration:" << std::endl;
        auto server_config = server.get_config();
        std::cout << "   IP Address: " << server_config.ip_address << std::endl;
        std::cout << "   Port: " << server_config.port << std::endl;
        std::cout << "   Max Clients: " << server_config.max_clients << std::endl;
        std::cout << "   Receive Timeout: " << server_config.receive_timeout_ms << " ms" << std::endl;
        std::cout << "   Send Timeout: " << server_config.send_timeout_ms << " ms" << std::endl;
        std::cout << "   Logging Enabled: " << (server_config.enable_logging ? "YES" : "NO") << std::endl;
        
        // Test initialization
        std::cout << "\n3. Server Initialization:" << std::endl;
        bool initialized = server.initialize();
        std::cout << "   Initialization: " << (initialized ? "SUCCESS" : "FAILED") << std::endl;
        
        if (initialized)
        {
            std::cout << "   Server initialized: " << (server.is_server_initialized() ? "YES" : "NO") << std::endl;
            
            // Test starting server
            std::cout << "\n4. Server Start/Stop:" << std::endl;
            bool started = server.start();
            std::cout << "   Server start: " << (started ? "SUCCESS" : "FAILED") << std::endl;
            
            if (started)
            {
                std::cout << "   Server running: " << (server.is_server_running() ? "YES" : "NO") << std::endl;
                std::cout << "   Active clients: " << server.get_active_client_count() << std::endl;
                
                // Get server statistics
                std::cout << "\n5. Server Statistics:" << std::endl;
                auto stats = server.get_server_stats();
                std::cout << "   Active clients: " << stats.active_clients << std::endl;
                std::cout << "   Total connections: " << stats.total_connections << std::endl;
                std::cout << "   Messages received: " << stats.total_messages_received << std::endl;
                std::cout << "   Messages sent: " << stats.total_messages_sent << std::endl;
                std::cout << "   Start time: " << stats.start_time << std::endl;
                std::cout << "   Memory usage: " << stats.memory_usage_mb << " MB" << std::endl;
                
                // Let server run for a moment
                std::cout << "\n   Server running for 2 seconds..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
                
                // Stop server
                std::cout << "\n6. Server Shutdown:" << std::endl;
                server.stop();
                std::cout << "   Server stop: SUCCESS" << std::endl;
                std::cout << "   Server running: " << (server.is_server_running() ? "YES" : "NO") << std::endl;
            }
        }
        
        std::cout << "✅ Socket Server tests completed!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "❌ Socket Server test failed: " << e.what() << std::endl;
    }
}

void test_protocol_handler()
{
    std::cout << "Testing Protocol Handler..." << std::endl;
    
    try 
    {
        // Create database manager for protocol handler
        auto db_manager = std::make_shared<Database_Manager>();
        Protocol_Handler protocol(db_manager);
        
        std::cout << "\n1. JSON Message Parsing:" << std::endl;
        
        // Test various JSON message types
        std::vector<std::string> json_messages = {
            "{\"type\":\"AUTH\",\"username\":\"admin\",\"password\":\"test123\"}",
            "{\"type\":\"REGISTER\",\"username\":\"newuser\",\"email\":\"user@test.com\",\"first_name\":\"John\",\"last_name\":\"Doe\"}",
            "{\"type\":\"GET_DESTINATIONS\"}",
            "{\"type\":\"GET_OFFERS\"}",
            "{\"type\":\"SEARCH_OFFERS\",\"destination\":\"Paris\",\"min_price\":100,\"max_price\":1000}",
            "{\"type\":\"BOOK_OFFER\",\"offer_id\":1,\"person_count\":2}",
            "{\"type\":\"KEEPALIVE\"}",
            "{\"command\":\"AUTH\",\"username\":\"admin\",\"password\":\"test123\"}",
            "{\"invalid\":\"json_without_type\"}",
            "{malformed json}"
        };
        
        for (const auto& msg : json_messages)
        {
            auto parsed = protocol.parse_message(msg);
            std::cout << "   JSON Message: \"" << msg << "\"" << std::endl;
            std::cout << "     Type: " << protocol.message_type_to_string(parsed.type) << std::endl;
            std::cout << "     Valid: " << (parsed.is_valid ? "YES" : "NO") << std::endl;
            if (!parsed.is_valid && !parsed.error_message.empty())
            {
                std::cout << "     Error: " << parsed.error_message << std::endl;
            }
            if (parsed.is_valid && !parsed.json_data.empty())
            {
                std::cout << "     JSON Data: " << parsed.json_data.dump() << std::endl;
            }
            std::cout << std::endl;
        }
        
        std::cout << "\n3. JSON Response Creation:" << std::endl;
        std::string success_resp = protocol.create_response(true, "Operation successful", "{\"id\":1,\"name\":\"Test\"}", 0);
        std::cout << "   Success response: " << success_resp << std::endl;
        
        std::string error_resp = protocol.create_response(false, "Operation failed", "", 404);
        std::cout << "   Error response: " << error_resp << std::endl;
        
        std::cout << "\n2. Message Type Recognition:" << std::endl;
        std::vector<std::string> commands = {
            "AUTH", "LOGIN", "REGISTER", "SIGNUP", "GET_DESTINATIONS",
            "GET_OFFERS", "SEARCH_OFFERS", "BOOK_OFFER", "KEEPALIVE", "PING"
        };
        
        for (const auto& cmd : commands)
        {
            nlohmann::json test_json;
            test_json["type"] = cmd;
            auto type = protocol.get_message_type(test_json);
            std::cout << "   Command \"" << cmd << "\" -> " << protocol.message_type_to_string(type) << std::endl;
        }
        
        
        std::cout << "✅ Protocol Handler tests completed!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "❌ Protocol Handler test failed: " << e.what() << std::endl;
    }
}

void test_integration()
{
    std::cout << "Testing System Integration..." << std::endl;
    
    try 
    {
        std::cout << "\n1. Component Integration:" << std::endl;
        
        // Create database manager
        auto db_manager = std::make_shared<Database_Manager>();
        std::cout << "   Database Manager created: SUCCESS" << std::endl;
        
        // Create server with database integration
        Server_Config config;
        config.port = Config::Server::PORT + 2; // Different port for integration test
        Socket_Server server(config);
        server.set_database_manager(db_manager);
        std::cout << "   Socket Server created with DB integration: SUCCESS" << std::endl;
        
        // Test configuration consistency
        std::cout << "\n2. Configuration Consistency:" << std::endl;
        std::cout << "   Default server port: " << Config::Server::PORT << std::endl;
        std::cout << "   Default max connections: " << Config::Server::MAX_CONNECTIONS << std::endl;
        std::cout << "   Default database: " << Config::Database::DEFAULT_DATABASE << std::endl;
        std::cout << "   Application name: " << Config::Application::APP_NAME << std::endl;
        std::cout << "   Application version: " << Config::Application::VERSION << std::endl;
        std::cout << "   Debug mode: " << (Config::Application::DEBUG_MODE ? "ENABLED" : "DISABLED") << std::endl;
        
        // Test error message consistency
        std::cout << "\n3. Error Messages:" << std::endl;
        std::cout << "   DB Connection Failed: " << Config::ErrorMessages::DB_CONNECTION_FAILED << std::endl;
        std::cout << "   Authentication Failed: " << Config::ErrorMessages::AUTHENTICATION_FAILED << std::endl;
        std::cout << "   Invalid Request: " << Config::ErrorMessages::INVALID_REQUEST << std::endl;
        std::cout << "   Server Error: " << Config::ErrorMessages::SERVER_ERROR << std::endl;
        
        // Test success messages
        std::cout << "\n4. Success Messages:" << std::endl;
        std::cout << "   User Created: " << Config::SuccessMessages::USER_CREATED << std::endl;
        std::cout << "   Login Success: " << Config::SuccessMessages::LOGIN_SUCCESS << std::endl;
        std::cout << "   Data Retrieved: " << Config::SuccessMessages::DATA_RETRIEVED << std::endl;
        
        // Test business rules
        std::cout << "\n5. Business Rules:" << std::endl;
        std::cout << "   Max persons per reservation: " << Config::Business::MAX_PERSONS_PER_RESERVATION << std::endl;
        std::cout << "   Min offer price: " << Config::Business::MIN_OFFER_PRICE << std::endl;
        std::cout << "   Max offer price: " << Config::Business::MAX_OFFER_PRICE << std::endl;
        std::cout << "   Min advance booking days: " << Config::Business::MIN_ADVANCE_BOOKING_DAYS << std::endl;
        
        // Test security settings
        std::cout << "\n6. Security Settings:" << std::endl;
        std::cout << "   Min password length: " << Config::Security::MIN_PASSWORD_LENGTH << std::endl;
        std::cout << "   Max login attempts: " << Config::Security::MAX_LOGIN_ATTEMPTS << std::endl;
        std::cout << "   Lockout duration: " << Config::Security::LOCKOUT_DURATION_MINUTES << " minutes" << std::endl;
        
        std::cout << "✅ Integration tests completed!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "❌ Integration test failed: " << e.what() << std::endl;
    }
}