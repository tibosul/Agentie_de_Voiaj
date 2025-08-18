#include "utils/utils.h"
#include "database/Database_Manager.h"
#include "network/Socket_Server.h"
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
    // Initialize logging system first
    Utils::Logger::initialize_logging();
    
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
        Utils::Logger::info("Initializing database manager...");
        
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
            Utils::Logger::debug("Trying database server: " + server);
            db_manager = std::make_shared<Database_Manager>(
                server,
                Config::Database::DEFAULT_DATABASE,
                "", // Windows Authentication
                ""
            );
            
            if (db_manager->connect())
            {
                Utils::Logger::info("Connected to database server: " + server);
                connected = true;
                break;
            }
            else
            {
                Utils::Logger::warning("Database connection failed: " + db_manager->get_last_error());
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
            Utils::Logger::info("Database connection successful");
            
            // Create database tables if not exist
            Utils::Logger::info("Creating database schema...");
            if (!db_manager->create_tables_if_not_exists())
            {
                Utils::Logger::warning("Could not create/verify all database tables");
            }
            else
            {
                Utils::Logger::info("Database schema ready");
            }
        }
        else
        {
            Utils::Logger::warning("Database functionality disabled - running in fallback mode");
        }
        
        // Create server configuration
        Server_Config config;
        config.ip_address = "0.0.0.0"; // Listen on all interfaces
        config.port = Config::Server::PORT;
        config.max_clients = Config::Server::MAX_CONNECTIONS;
        config.enable_logging = Config::Application::DEBUG_MODE;
        
        // Create and configure server
        Utils::Logger::info("Creating server...");
        Socket_Server server(config);
        g_server = &server; // For signal handler
        
        server.set_database_manager(db_manager);
        
        // Initialize server
        if (!server.initialize())
        {
            std::cerr << "ERROR: Server initialization failed!" << std::endl;
            return -1;
        }
        Utils::Logger::info("Server initialized successfully");
        
        // Start server
        Utils::Logger::info("Starting server on " + config.ip_address + ":" + std::to_string(config.port) + "...");
        if (!server.start())
        {
            std::cerr << "ERROR: Server startup failed!" << std::endl;
            return -1;
        }
        
        Utils::Logger::info("=== SERVER STARTED SUCCESSFULLY! ===");
        Utils::Logger::info("Server is running. Press Ctrl+C to shutdown.");
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "SERVER RUNNING - Check logs/server_" << Utils::DateTime::get_current_date() << ".log for detailed logs" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        // Main server loop - keep running until interrupted
        while (server.is_server_running())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            // Optional: Print server stats every 30 seconds
            static int stats_counter = 0;
            if (++stats_counter >= 30 && Config::Application::DEBUG_MODE)
            {
                auto stats = server.get_server_stats();
                Utils::Logger::info("=== Server Stats ===");
                Utils::Logger::info("Active clients: " + std::to_string(stats.active_clients));
                Utils::Logger::info("Total connections: " + std::to_string(stats.total_connections));
                Utils::Logger::info("Messages received: " + std::to_string(stats.total_messages_received));
                Utils::Logger::info("Messages sent: " + std::to_string(stats.total_messages_sent));
                Utils::Logger::info("Uptime: " + stats.uptime);
                stats_counter = 0;
            }
        }
        
    }
    catch (const std::exception& e)
    {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return -1;
    }
    
    Utils::Logger::info("=== Server shutdown complete ===");
    std::cout << "Server shutdown complete." << std::endl;
    return 0;
}

