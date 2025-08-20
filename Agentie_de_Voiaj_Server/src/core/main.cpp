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
        
        // More intelligent server detection - try most common configurations first
        std::vector<std::string> server_options = {
            "localhost",                    // Standard localhost
            ".",                           // Local instance
            ".\\SQLEXPRESS",               // SQL Server Express (most common)
            "localhost\\SQLEXPRESS",       // SQL Server Express on localhost
            "(LocalDB)\\MSSQLLocalDB",     // SQL Server LocalDB
            "localhost\\MSSQLSERVER",      // SQL Server default instance
            ".\\MSSQLSERVER"               // SQL Server default instance local
        };
        
        std::shared_ptr<Database_Manager> db_manager = nullptr;
        bool connected = false;
        std::string successful_server;
        
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
                Utils::Logger::info("✅ Connected to database server: " + server);
                
                // Check if the database exists
                if (db_manager->database_exists())
                {
                    Utils::Logger::info("✅ Database '" + Config::Database::DEFAULT_DATABASE + "' exists and is accessible");
                    successful_server = server;
                    connected = true;
                    break;
                }
                else
                {
                    Utils::Logger::warning("⚠️  Connected to server but database '" + Config::Database::DEFAULT_DATABASE + "' doesn't exist");
                    Utils::Logger::info("💡 Run 'setup_database.bat' to create the database and tables");
                }
            }
            else
            {
                Utils::Logger::warning("❌ Database connection failed for " + server + ": " + db_manager->get_last_error());
            }
        }
        
        if (!connected)
        {
            Utils::Logger::error("Cannot connect to any SQL Server instance!");
            Utils::Logger::info("Tried servers: localhost, localhost\\SQLEXPRESS, .\\SQLEXPRESS, (LocalDB)\\MSSQLLocalDB, .");
            
            std::cerr << "\n" << std::string(60, '=') << std::endl;
            std::cerr << "🔶 DATABASE CONNECTION FAILED - STARTING IN DEMO MODE 🔶" << std::endl;
            std::cerr << std::string(60, '=') << std::endl;
            std::cerr << "\n📋 DEMO MODE FEATURES:" << std::endl;
            std::cerr << "  ✅ User authentication (demo/demo123, admin/admin123, test/test123)" << std::endl;
            std::cerr << "  ✅ User registration (mock responses)" << std::endl;
            std::cerr << "  ✅ View destinations (Paris, Rome)" << std::endl;
            std::cerr << "  ✅ View offers (Paris Weekend, Rome Adventure)" << std::endl;
            std::cerr << "  ✅ All server functionality for testing" << std::endl;
            std::cerr << "\n📢 To enable REAL database:" << std::endl;
            std::cerr << "  1. Install SQL Server LocalDB or Express" << std::endl;
            std::cerr << "  2. Run 'setup_database.bat' to create database and tables" << std::endl;
                            std::cerr << "  3. Or manually create 'Agentie_de_Voiaj' database" << std::endl;
            std::cerr << "  4. Restart the server" << std::endl;
            std::cerr << "\n" << std::string(60, '-') << std::endl;
            std::cout << "Press Enter to continue in DEMO MODE or Ctrl+C to exit..." << std::endl;
            std::cin.get();
            
            // Create a dummy database manager that will handle errors gracefully
            db_manager = std::make_shared<Database_Manager>("dummy", "dummy", "", "");
            Utils::Logger::warning("Server starting in DEMO MODE with mock data");
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
        if (db_manager && db_manager->is_running_in_demo_mode())
        {
            Utils::Logger::warning("⚠️  RUNNING IN DEMO MODE - Using mock data ⚠️");
        }
        Utils::Logger::info("Server is running. Press Ctrl+C to shutdown.");
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "SERVER RUNNING";
        if (db_manager && db_manager->is_running_in_demo_mode())
        {
            std::cout << " (DEMO MODE)";
        }
        std::cout << " - Check logs/server_" << Utils::DateTime::get_current_date() << ".log for detailed logs" << std::endl;
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

