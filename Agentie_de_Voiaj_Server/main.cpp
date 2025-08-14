#include "utils.h"
#include "Database_Manager.h"
#include "Socket_Server.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

using namespace Database;
using namespace SocketNetwork;

// Global server instance for signal handling
Socket_Server* g_server = nullptr;

// Signal handler for graceful shutdown
void signal_handler(int signal)
{
    std::cout << "\nReceived signal " << signal << ". Shutting down server..." << std::endl;
    if (g_server)
    {
        g_server->stop();
    }
    exit(0);
}

int main()
{
    std::cout << "=== AGENTIE DE VOIAJ SERVER ===" << std::endl;
    std::cout << "Version: " << Config::Application::VERSION << std::endl;
    std::cout << "Debug Mode: " << (Config::Application::DEBUG_MODE ? "ON" : "OFF") << std::endl;
    std::cout << "Port: " << Config::Server::PORT << std::endl;
    std::cout << "Max Connections: " << Config::Server::MAX_CONNECTIONS << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try 
    {
        // Create database manager - try multiple server options
        std::cout << "Initializing database manager..." << std::endl;
        
        std::vector<std::string> server_options = {
            "localhost",
            "localhost\\SQLEXPRESS",
            ".\\SQLEXPRESS", 
            "(LocalDB)\\MSSQLLocalDB",
            "."
        };
        
        std::shared_ptr<Database_Manager> db_manager = nullptr;
        bool connected = false;
        
        for (const auto& server : server_options)
        {
            std::cout << "Trying server: " << server << std::endl;
            db_manager = std::make_shared<Database_Manager>(
                server,
                Config::Database::DEFAULT_DATABASE,
                "", // Windows Authentication
                ""
            );
            
            if (db_manager->connect())
            {
                std::cout << "Connected to server: " << server << std::endl;
                connected = true;
                break;
            }
            else
            {
                std::cout << "  Failed: " << db_manager->get_last_error() << std::endl;
            }
        }
        
        if (!connected)
        {
            std::cerr << "\nWARNING: Cannot connect to any SQL Server instance!" << std::endl;
            std::cerr << "\nTried servers:" << std::endl;
            for (const auto& server : server_options)
            {
                std::cerr << "  - " << server << std::endl;
            }
            std::cerr << "\nServer will start WITHOUT database functionality." << std::endl;
            std::cerr << "Database operations will return mock/error responses." << std::endl;
            std::cerr << "\nTo enable database:" << std::endl;
            std::cerr << "1. Install SQL Server LocalDB: https://docs.microsoft.com/en-us/sql/database-engine/configure-windows/sql-server-express-localdb" << std::endl;
            std::cerr << "2. Or install SQL Server Express" << std::endl;
            std::cerr << "3. Restart the server" << std::endl;
            std::cout << "\nPress Enter to continue without database or Ctrl+C to exit..." << std::endl;
            std::cin.get();
            
            // Create a dummy database manager that will handle errors gracefully
            db_manager = std::make_shared<Database_Manager>("dummy", "dummy", "", "");
        }
        if (connected)
        {
            std::cout << "Database connection successful" << std::endl;
            
            // Create database tables if not exist
            std::cout << "Creating database schema..." << std::endl;
            if (!db_manager->create_tables_if_not_exists())
            {
                std::cout << "Warning: Could not create/verify all database tables" << std::endl;
            }
            else
            {
                std::cout << "Database schema ready" << std::endl;
            }
        }
        else
        {
            std::cout << "  Database functionality disabled" << std::endl;
        }
        
        // Create server configuration
        Server_Config config;
        config.ip_address = "0.0.0.0"; // Listen on all interfaces
        config.port = Config::Server::PORT;
        config.max_clients = Config::Server::MAX_CONNECTIONS;
        config.enable_logging = Config::Application::DEBUG_MODE;
        
        // Create and configure server
        std::cout << "Creating server..." << std::endl;
        Socket_Server server(config);
        g_server = &server; // For signal handler
        
        server.set_database_manager(db_manager);
        
        // Initialize server
        if (!server.initialize())
        {
            std::cerr << "ERROR: Server initialization failed!" << std::endl;
            return -1;
        }
        std::cout << "Server initialized" << std::endl;
        
        // Start server
        std::cout << "Starting server on " << config.ip_address << ":" << config.port << "..." << std::endl;
        if (!server.start())
        {
            std::cerr << "ERROR: Server startup failed!" << std::endl;
            return -1;
        }
        
        std::cout << "SERVER STARTED SUCCESSFULLY!" << std::endl;
        std::cout << "\nServer is running. Press Ctrl+C to shutdown." << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        // Main server loop - keep running until interrupted
        while (server.is_server_running())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            // Optional: Print server stats every 60 seconds
            static int stats_counter = 0;
            if (++stats_counter >= 60 && Config::Application::DEBUG_MODE)
            {
                auto stats = server.get_server_stats();
                std::cout << "\n--- Server Stats ---" << std::endl;
                std::cout << "Active clients: " << stats.active_clients << std::endl;
                std::cout << "Total connections: " << stats.total_connections << std::endl;
                std::cout << "Messages received: " << stats.total_messages_received << std::endl;
                std::cout << "Messages sent: " << stats.total_messages_sent << std::endl;
                std::cout << "Uptime: " << stats.uptime << std::endl;
                std::cout << std::string(20, '-') << std::endl;
                stats_counter = 0;
            }
        }
        
    }
    catch (const std::exception& e)
    {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return -1;
    }
    
    std::cout << "Server shutdown complete." << std::endl;
    return 0;
}

