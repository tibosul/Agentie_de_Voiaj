#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include <chrono>
#include <thread>

// Include test headers
#include "network/Protocol_Handler.h"
#include "database/Database_Manager.h"
#include "network/Socket_Server.h"
#include "network/Network_Types.h"
#include "utils/Logger.h"

// Test utilities
class TestUtils {
public:
    static void run_test(const std::string& test_name, std::function<bool()> test_func) {
        std::cout << "Running test: " << test_name << "... ";
        try {
            bool result = test_func();
            if (result) {
                std::cout << "PASSED" << std::endl;
                passed_tests++;
            } else {
                std::cout << "FAILED" << std::endl;
                failed_tests++;
            }
        } catch (const std::exception& e) {
            std::cout << "FAILED with exception: " << e.what() << std::endl;
            failed_tests++;
        }
        total_tests++;
    }
    
    static void print_summary() {
        std::cout << "\n=== TEST SUMMARY ===" << std::endl;
        std::cout << "Total tests: " << total_tests << std::endl;
        std::cout << "Passed: " << passed_tests << std::endl;
        std::cout << "Failed: " << failed_tests << std::endl;
        std::cout << "Success rate: " << (total_tests > 0 ? (passed_tests * 100.0 / total_tests) : 0) << "%" << std::endl;
    }
    
private:
    static int total_tests;
    static int passed_tests;
    static int failed_tests;
};

int TestUtils::total_tests = 0;
int TestUtils::passed_tests = 0;
int TestUtils::failed_tests = 0;

// Mock database for testing
class MockDatabaseManager : public Database::Database_Manager {
public:
    MockDatabaseManager() : Database::Database_Manager() {}
    
    bool is_connected() const override { return true; }
    
    Query_Result search_offers(const std::string& destination, double min_price, double max_price) override {
        Query_Result result;
        result.success = true;
        result.message = "Mock offers found";
        
        // Create mock offers
        std::vector<std::map<std::string, std::string>> mock_data;
        std::map<std::string, std::string> offer1;
        offer1["id"] = "1";
        offer1["destination"] = "Paris";
        offer1["price"] = "500.0";
        offer1["duration"] = "7";
        offer1["description"] = "Beautiful Paris vacation";
        mock_data.push_back(offer1);
        
        std::map<std::string, std::string> offer2;
        offer2["id"] = "2";
        offer2["destination"] = "Rome";
        offer2["price"] = "600.0";
        offer2["duration"] = "5";
        offer2["description"] = "Amazing Rome experience";
        mock_data.push_back(offer2);
        
        result.data = mock_data;
        return result;
    }
    
    Query_Result book_offer(int offer_id, int user_id, int person_count, 
                           const std::string& additional_info) override {
        Query_Result result;
        result.success = true;
        result.message = "Mock reservation created";
        result.data = {{"reservation_id", "123"}};
        return result;
    }
    
    Query_Result get_user_reservations(int user_id) override {
        Query_Result result;
        result.success = true;
        result.message = "Mock reservations found";
        
        std::vector<std::map<std::string, std::string>> mock_data;
        std::map<std::string, std::string> reservation;
        reservation["id"] = "123";
        reservation["offer_id"] = "1";
        reservation["destination"] = "Paris";
        reservation["status"] = "Confirmed";
        reservation["person_count"] = "2";
        mock_data.push_back(reservation);
        
        result.data = mock_data;
        return result;
    }
    
    Query_Result cancel_reservation(int reservation_id) override {
        Query_Result result;
        result.success = true;
        result.message = "Mock reservation cancelled";
        return result;
    }
    
    Query_Result update_user_info(int user_id, const std::string& field, 
                                 const std::string& value) override {
        Query_Result result;
        result.success = true;
        result.message = "Mock user info updated";
        return result;
    }
};

// Protocol Handler Tests
void test_protocol_handler() {
    std::cout << "\n=== PROTOCOL HANDLER TESTS ===" << std::endl;
    
    auto mock_db = std::make_shared<MockDatabaseManager>();
    auto protocol_handler = std::make_unique<Protocol_Handler>(mock_db);
    
    // Test search offers
    TestUtils::run_test("Search Offers - Valid Request", [&]() {
        Request request;
        request.type = Message_Type::SEARCH_OFFERS;
        request.data = {{"destination", "Paris"}, {"min_price", "100"}, {"max_price", "1000"}};
        
        Response response = protocol_handler->process_message(request);
        return response.success && response.message == "Mock offers found";
    });
    
    // Test book offer
    TestUtils::run_test("Book Offer - Valid Request", [&]() {
        Request request;
        request.type = Message_Type::BOOK_OFFER;
        request.data = {{"offer_id", "1"}, {"person_count", "2"}, {"additional_info", "Window seat"}};
        
        Response response = protocol_handler->process_message(request);
        return response.success && response.message == "Mock reservation created";
    });
    
    // Test get user reservations
    TestUtils::run_test("Get User Reservations - Valid Request", [&]() {
        Request request;
        request.type = Message_Type::GET_USER_RESERVATIONS;
        request.data = {{"user_id", "1"}};
        
        Response response = protocol_handler->process_message(request);
        return response.success && response.message == "Mock reservations found";
    });
    
    // Test cancel reservation
    TestUtils::run_test("Cancel Reservation - Valid Request", [&]() {
        Request request;
        request.type = Message_Type::CANCEL_RESERVATION;
        request.data = {{"reservation_id", "123"}};
        
        Response response = protocol_handler->process_message(request);
        return response.success && response.message == "Mock reservation cancelled";
    });
    
    // Test invalid message type
    TestUtils::run_test("Invalid Message Type", [&]() {
        Request request;
        request.type = static_cast<Message_Type>(999); // Invalid type
        request.data = {{"test", "data"}};
        
        Response response = protocol_handler->process_message(request);
        return !response.success && response.message.find("Unknown message type") != std::string::npos;
    });
}

// Database Manager Tests
void test_database_manager() {
    std::cout << "\n=== DATABASE MANAGER TESTS ===" << std::endl;
    
    auto mock_db = std::make_shared<MockDatabaseManager>();
    
    // Test search offers
    TestUtils::run_test("Database Search Offers", [&]() {
        auto result = mock_db->search_offers("Paris", 100.0, 1000.0);
        return result.success && result.data.size() == 2;
    });
    
    // Test book offer
    TestUtils::run_test("Database Book Offer", [&]() {
        auto result = mock_db->book_offer(1, 1, 2, "Window seat");
        return result.success && result.data.size() == 1;
    });
    
    // Test get user reservations
    TestUtils::run_test("Database Get User Reservations", [&]() {
        auto result = mock_db->get_user_reservations(1);
        return result.success && result.data.size() == 1;
    });
    
    // Test cancel reservation
    TestUtils::run_test("Database Cancel Reservation", [&]() {
        auto result = mock_db->cancel_reservation(123);
        return result.success;
    });
    
    // Test update user info
    TestUtils::run_test("Database Update User Info", [&]() {
        auto result = mock_db->update_user_info(1, "phone", "+1234567890");
        return result.success;
    });
}

// Socket Server Tests
void test_socket_server() {
    std::cout << "\n=== SOCKET SERVER TESTS ===" << std::endl;
    
    // Test server creation
    TestUtils::run_test("Server Creation - Default Config", [&]() {
        auto server = std::make_unique<SocketNetwork::Socket_Server>();
        return server != nullptr && !server->is_server_running();
    });
    
    // Test server creation with custom config
    TestUtils::run_test("Server Creation - Custom Config", [&]() {
        SocketNetwork::Server_Config config;
        config.ip_address = "127.0.0.1";
        config.port = 8080;
        config.max_clients = 10;
        
        auto server = std::make_unique<SocketNetwork::Socket_Server>(config);
        return server != nullptr && !server->is_server_running();
    });
    
    // Test server configuration
    TestUtils::run_test("Server Configuration", [&]() {
        auto server = std::make_unique<SocketNetwork::Socket_Server>();
        SocketNetwork::Server_Config config;
        config.ip_address = "127.0.0.1";
        config.port = 8080;
        
        server->set_config(config);
        auto retrieved_config = server->get_config();
        return retrieved_config.ip_address == "127.0.0.1" && retrieved_config.port == 8080;
    });
    
    // Test server statistics
    TestUtils::run_test("Server Statistics", [&]() {
        auto server = std::make_unique<SocketNetwork::Socket_Server>();
        auto stats = server->get_server_stats();
        return stats.active_clients == 0 && stats.total_connections == 0;
    });
}

// Integration Tests
void test_integration() {
    std::cout << "\n=== INTEGRATION TESTS ===" << std::endl;
    
    // Test full request-response cycle
    TestUtils::run_test("Full Request-Response Cycle", [&]() {
        auto mock_db = std::make_shared<MockDatabaseManager>();
        auto protocol_handler = std::make_unique<Protocol_Handler>(mock_db);
        
        // Create a complete request
        Request request;
        request.type = Message_Type::SEARCH_OFFERS;
        request.data = {{"destination", "Paris"}, {"min_price", "100"}, {"max_price", "1000"}};
        
        // Process the request
        Response response = protocol_handler->process_message(request);
        
        // Verify response
        if (!response.success) return false;
        
        // Verify data structure
        return response.data.size() == 2 && 
               response.data[0]["destination"] == "Paris" &&
               response.data[1]["destination"] == "Rome";
    });
    
    // Test error handling
    TestUtils::run_test("Error Handling", [&]() {
        auto mock_db = std::make_shared<MockDatabaseManager>();
        auto protocol_handler = std::make_unique<Protocol_Handler>(mock_db);
        
        // Create invalid request
        Request request;
        request.type = static_cast<Message_Type>(999);
        
        Response response = protocol_handler->process_message(request);
        return !response.success && !response.message.empty();
    });
}

// Performance Tests
void test_performance() {
    std::cout << "\n=== PERFORMANCE TESTS ===" << std::endl;
    
    // Test message processing speed
    TestUtils::run_test("Message Processing Performance", [&]() {
        auto mock_db = std::make_shared<MockDatabaseManager>();
        auto protocol_handler = std::make_unique<Protocol_Handler>(mock_db);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Process multiple requests
        for (int i = 0; i < 1000; i++) {
            Request request;
            request.type = Message_Type::SEARCH_OFFERS;
            request.data = {{"destination", "Test"}, {"min_price", "0"}, {"max_price", "1000"}};
            
            Response response = protocol_handler->process_message(request);
            if (!response.success) return false;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Should process 1000 requests in less than 100ms
        return duration.count() < 100;
    });
}

// Main test runner
int main() {
    std::cout << "Starting Agentie de Voiaj Server Unit Tests..." << std::endl;
    std::cout << "=============================================" << std::endl;
    
    try {
        // Initialize logger for tests
        Utils::Logger::set_log_level(Utils::LogLevel::INFO);
        
        // Run all test suites
        test_protocol_handler();
        test_database_manager();
        test_socket_server();
        test_integration();
        test_performance();
        
        // Print test summary
        TestUtils::print_summary();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Test suite failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
