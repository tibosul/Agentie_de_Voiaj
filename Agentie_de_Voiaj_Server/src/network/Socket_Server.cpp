#include "network/Socket_Server.h"
#include "network/Client_Handler.h"
#include "network/Protocol_Handler.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <ws2tcpip.h>

// ============================================================================
// Socket_Server Implementation
// ============================================================================

SocketNetwork::Socket_Server::Socket_Server()
    : server_socket(), is_running(false), is_initialized(false),
      client_count(0), total_connections(0), total_messages_received(0), total_messages_sent(0)
{
    server_start_time = Utils::DateTime::get_current_date_time();
    Utils::Logger::info("Socket_Server created with default configuration");
}

SocketNetwork::Socket_Server::Socket_Server(const Server_Config& config)
    : server_socket(), config(config), is_running(false), is_initialized(false),
      client_count(0), total_connections(0), total_messages_received(0), total_messages_sent(0)
{
    server_start_time = Utils::DateTime::get_current_date_time();
    Utils::Logger::info("Socket_Server created with custom configuration - Port: " + 
                        Utils::Conversion::int_to_string(config.port));
}

SocketNetwork::Socket_Server::Socket_Server(const std::string& ip, int port)
    : server_socket(), is_running(false), is_initialized(false),
      client_count(0), total_connections(0), total_messages_received(0), total_messages_sent(0)
{
    config.ip_address = ip;
    config.port = port;
    server_start_time = Utils::DateTime::get_current_date_time();
    Utils::Logger::info("Socket_Server created - IP: " + ip + ", Port: " + 
                        Utils::Conversion::int_to_string(port));
}

SocketNetwork::Socket_Server::~Socket_Server()
{
    stop();
    cleanup_winsock();
    Utils::Logger::info("Socket_Server destroyed");
}

bool SocketNetwork::Socket_Server::initialize()
{
    if (is_initialized.load())
    {
        Utils::Logger::warning("Socket_Server already initialized");
        return true;
    }

    try 
    {
        if (!initialize_winsock())
        {
            log_server_event("Failed to initialize Winsock");
            return false;
        }

        if (!create_server_socket())
        {
            log_server_event("Failed to create server socket");
            cleanup_winsock();
            return false;
        }

        if (!bind_and_listen())
        {
            log_server_event("Failed to bind and listen on socket");
            server_socket.reset();
            cleanup_winsock();
            return false;
        }

        is_initialized = true;
        log_server_event("Socket_Server initialized successfully on " + 
                        config.ip_address + ":" + Utils::Conversion::int_to_string(config.port));
        return true;
    }
    catch (const std::exception& e)
    {
        log_server_event("Exception during initialization: " + std::string(e.what()));
        return false;
    }
}

bool SocketNetwork::Socket_Server::start()
{
    if (is_running.load())
    {
        Utils::Logger::warning("Socket_Server is already running");
        return true;
    }

    if (!is_initialized.load())
    {
        if (!initialize())
        {
            return false;
        }
    }

    try 
    {
        is_running = true;
        
        // Start accept thread
        accept_thread = std::thread(&Socket_Server::accept_connections_loop, this);
        
        // Start cleanup thread
        cleanup_thread = std::thread([this]() 
        {
            while (is_running.load())
            {
                cleanup_disconnected_clients();
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        });

        log_server_event("Socket_Server started successfully");
        return true;
    }
    catch (const std::exception& e)
    {
        log_server_event("Exception while starting server: " + std::string(e.what()));
        is_running = false;
        return false;
    }
}

void SocketNetwork::Socket_Server::stop()
{
    if (!is_running.load())
    {
        return;
    }

    log_server_event("Stopping Socket_Server...");
    is_running = false;

    // Close server socket to interrupt accept()
    server_socket.reset();

    // Disconnect all clients
    disconnect_all_clients();

    // Wait for threads to finish
    if (accept_thread.joinable())
    {
        accept_thread.join();
    }
    
    if (cleanup_thread.joinable())
    {
        cleanup_thread.join();
    }

    log_server_event("Socket_Server stopped successfully");
}

bool SocketNetwork::Socket_Server::restart()
{
    log_server_event("Restarting Socket_Server...");
    stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return start();
}

void SocketNetwork::Socket_Server::set_config(const Server_Config& new_config)
{
    if (is_running.load())
    {
        Utils::Logger::warning("Cannot change configuration while server is running");
        return;
    }
    
    config = new_config;
    Utils::Logger::info("Server configuration updated");
}

void SocketNetwork::Socket_Server::set_database_manager(std::shared_ptr<Database::Database_Manager> db_mgr)
{
    db_manager = db_mgr;
    if (protocol_handler)
    {
        protocol_handler = std::make_unique<Protocol_Handler>(db_manager);
    }
    Utils::Logger::info("Database manager set for Socket_Server");
}

SocketNetwork::Server_Config SocketNetwork::Socket_Server::get_config() const
{
    return config;
}

bool SocketNetwork::Socket_Server::is_server_running() const
{
    return is_running.load();
}

bool SocketNetwork::Socket_Server::is_server_initialized() const
{
    return is_initialized.load();
}

int SocketNetwork::Socket_Server::get_active_client_count() const
{
    return client_count.load();
}

std::vector<SocketNetwork::Client_Info> SocketNetwork::Socket_Server::get_active_clients() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(clients_mutex));
    std::vector<Client_Info> clients;
    
    for (const auto& pair : active_clients)
    {
        if (pair.second)
        {
            clients.push_back(pair.second->get_client_info());
        }
    }
    
    return clients;
}

void SocketNetwork::Socket_Server::disconnect_client(SOCKET client_socket)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = active_clients.find(client_socket);
    if (it != active_clients.end())
    {
        if (it->second)
        {
            log_client_event(it->second->get_client_info(), "Client disconnected by server");
            it->second->stop_handling();
        }
        active_clients.erase(it);
        client_count--;
    }
}

void SocketNetwork::Socket_Server::disconnect_all_clients()
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    log_server_event("Disconnecting all clients (" + 
                     Utils::Conversion::int_to_string(static_cast<int>(active_clients.size())) + ")");
    
    for (auto& pair : active_clients)
    {
        if (pair.second)
        {
            pair.second->stop_handling();
        }
    }
    
    active_clients.clear();
    client_count = 0;
}

void SocketNetwork::Socket_Server::broadcast_message(const std::string& message)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    int sent_count = 0;
    
    for (auto& pair : active_clients)
    {
        if (pair.second && pair.second->send_message(message))
        {
            sent_count++;
        }
    }
    
    total_messages_sent += sent_count;
    log_server_event("Broadcast message sent to " + Utils::Conversion::int_to_string(sent_count) + " clients");
}

void SocketNetwork::Socket_Server::send_message_to_user(int user_id, const std::string& message)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    for (auto& pair : active_clients)
    {
        if (pair.second && pair.second->get_client_info().user_id == user_id)
        {
            if (pair.second->send_message(message))
            {
                total_messages_sent++;
                log_server_event("Message sent to user " + Utils::Conversion::int_to_string(user_id));
            }
            return;
        }
    }
    
    log_server_event("User " + Utils::Conversion::int_to_string(user_id) + " not found for message delivery");
}

void SocketNetwork::Socket_Server::send_message_to_client(SOCKET client_socket, const std::string& message)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = active_clients.find(client_socket);
    
    if (it != active_clients.end() && it->second)
    {
        if (it->second->send_message(message))
        {
            total_messages_sent++;
        }
    }
}

SocketNetwork::Server_Stats SocketNetwork::Socket_Server::get_server_stats() const
{
    Server_Stats stats;
    stats.active_clients = client_count.load();
    stats.total_connections = total_connections.load();
    stats.total_messages_received = total_messages_received.load();
    stats.total_messages_sent = total_messages_sent.load();
    stats.uptime = get_uptime();
    stats.start_time = server_start_time;
    stats.average_response_time_ms = 0.0; // Response time tracking not implemented in this version
    stats.memory_usage_mb = static_cast<int>(Utils::Memory::get_memory_usage_MB());
    
    return stats;
}

void SocketNetwork::Socket_Server::reset_server_stats()
{
    total_connections = 0;
    total_messages_received = 0;
    total_messages_sent = 0;
    server_start_time = Utils::DateTime::get_current_date_time();
    log_server_event("Server statistics reset");
}

// Private methods implementation

bool SocketNetwork::Socket_Server::initialize_winsock()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    if (result != 0)
    {
        log_server_event("WSAStartup failed with error: " + Utils::Conversion::int_to_string(result));
        return false;
    }
    
    return true;
}

void SocketNetwork::Socket_Server::cleanup_winsock()
{
    WSACleanup();
}

bool SocketNetwork::Socket_Server::create_server_socket()
{
    server_socket.reset(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    
    if (!server_socket.is_valid())
    {
        log_server_event("Socket creation failed with error: " + 
                        Utils::Conversion::int_to_string(WSAGetLastError()));
        return false;
    }
    
    if (!set_socket_options(server_socket))
    {
        server_socket.reset();
        return false;
    }
    
    return true;
}

bool SocketNetwork::Socket_Server::bind_and_listen()
{
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(config.port);
    
    if (inet_pton(AF_INET, config.ip_address.c_str(), &serverAddr.sin_addr) <= 0)
    {
        log_server_event("Invalid IP address: " + config.ip_address);
        return false;
    }
    
    if (bind(server_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        log_server_event("Bind failed with error: " + 
                        Utils::Conversion::int_to_string(WSAGetLastError()));
        return false;
    }
    
    if (listen(server_socket, Config::Server::BACKLOG_SIZE) == SOCKET_ERROR)
    {
        log_server_event("Listen failed with error: " + 
                        Utils::Conversion::int_to_string(WSAGetLastError()));
        return false;
    }
    
    return true;
}

void SocketNetwork::Socket_Server::accept_connections_loop()
{
    log_server_event("Accept connections loop started");
    
    while (is_running.load())
    {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        
        SOCKET clientSocket = accept(server_socket, (sockaddr*)&clientAddr, &clientAddrSize);
        
        if (clientSocket == INVALID_SOCKET)
        {
            if (is_running.load())
            {
                log_server_event("Accept failed with error: " + 
                                Utils::Conversion::int_to_string(WSAGetLastError()));
            }
            continue;
        }
        
        if (client_count.load() >= config.max_clients)
        {
            log_server_event("Maximum clients reached, rejecting connection");
            closesocket(clientSocket);
            continue;
        }
        
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::string clientAddress = std::string(clientIP);
        int clientPort = ntohs(clientAddr.sin_port);
        
        handle_new_client(clientSocket, clientAddress);
        total_connections++;
    }
    
    log_server_event("Accept connections loop ended");
}

void SocketNetwork::Socket_Server::cleanup_disconnected_clients()
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = active_clients.begin();
    
    while (it != active_clients.end())
    {
        if (!it->second || !it->second->is_client_running())
        {
            if (it->second)
            {
                log_client_event(it->second->get_client_info(), "Cleaning up disconnected client");
            }
            it = active_clients.erase(it);
            client_count--;
        }
        else 
        {
            ++it;
        }
    }
}

void SocketNetwork::Socket_Server::handle_new_client(SOCKET client_socket, const std::string& client_address)
{
    try 
    {
        if (!set_socket_options(client_socket))
        {
            closesocket(client_socket);
            log_server_event("Failed to set socket options for new client");
            return;
        }
        
        // Extract port from client socket
        sockaddr_in clientAddr;
        int addrlen = sizeof(clientAddr);
        int client_port = 0;
        
        if (getpeername(client_socket, (sockaddr*)&clientAddr, &addrlen) == 0)
        {
            client_port = ntohs(clientAddr.sin_port);
        }
        
        Client_Info client_info(client_socket, client_address, client_port);
        
        // Thread-safe protocol handler creation
        {
            std::lock_guard<std::mutex> protocol_lock(protocol_handler_mutex);
            if (!protocol_handler)
            {
                try 
                {
                    protocol_handler = std::make_unique<Protocol_Handler>(db_manager);
                }
                catch (const std::exception& e)
                {
                    log_server_event("Failed to create protocol handler: " + std::string(e.what()));
                    closesocket(client_socket);
                    return;
                }
            }
        }
        
        std::shared_ptr<Client_Handler> client_handler;
        try 
        {
            client_handler = std::make_shared<Client_Handler>(
                client_socket, client_info, db_manager, protocol_handler.get(), this);
        }
        catch (const std::exception& e)
        {
            log_server_event("Failed to create client handler: " + std::string(e.what()));
            closesocket(client_socket);
            return;
        }
        
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            active_clients[client_socket] = client_handler;
            client_count++;
        }
        
        try 
        {
            client_handler->start_handling();
            log_client_event(client_info, "New client connected successfully");
            
            if (on_client_connected)
            {
                on_client_connected(client_info);
            }
        }
        catch (const std::exception& e)
        {
            // Remove client from active list if start_handling fails
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                active_clients.erase(client_socket);
                client_count--;
            }
            log_server_event("Failed to start client handling: " + std::string(e.what()));
            closesocket(client_socket);
        }
    }
    catch (const std::exception& e)
    {
        log_server_event("Exception handling new client: " + std::string(e.what()));
        closesocket(client_socket);
    }
    catch (...)
    {
        log_server_event("Unknown exception handling new client");
        closesocket(client_socket);
    }
}

void SocketNetwork::Socket_Server::remove_client(SOCKET client_socket)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = active_clients.find(client_socket);
    
    if (it != active_clients.end())
    {
        if (it->second)
        {
            Client_Info client_info = it->second->get_client_info();
            log_client_event(client_info, "Client removed");
            
            if (on_client_disconnected)
            {
                on_client_disconnected(client_info);
            }
        }
        
        active_clients.erase(it);
        client_count--;
    }
}

SocketNetwork::Client_Info* SocketNetwork::Socket_Server::get_client_info(SOCKET client_socket)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = active_clients.find(client_socket);
    
    if (it != active_clients.end() && it->second)
    {
        return const_cast<Client_Info*>(&it->second->get_client_info());
    }
    
    return nullptr;
}

std::string SocketNetwork::Socket_Server::get_socket_address(SOCKET socket) const
{
    sockaddr_in addr;
    int addrlen = sizeof(addr);
    
    if (getpeername(socket, (sockaddr*)&addr, &addrlen) == 0)
    {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
        return std::string(ip) + ":" + Utils::Conversion::int_to_string(ntohs(addr.sin_port));
    }
    
    return "unknown";
}

bool SocketNetwork::Socket_Server::set_socket_options(SOCKET socket)
{
    // Set receive timeout
    DWORD timeout = config.receive_timeout_ms;
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
    {
        log_server_event("Failed to set receive timeout");
        return false;
    }
    
    // Set send timeout
    timeout = config.send_timeout_ms;
    if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
    {
        log_server_event("Failed to set send timeout");
        return false;
    }
    
    // Enable keep-alive if configured
    if (Config::Server::ENABLE_KEEP_ALIVE)
    {
        BOOL keepalive = TRUE;
        if (setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepalive, sizeof(keepalive)) == SOCKET_ERROR)
        {
            log_server_event("Failed to enable keep-alive");
            return false;
        }
    }
    
    return true;
}

std::string SocketNetwork::Socket_Server::get_uptime() const
{
    // Calculate uptime from server start time
    auto now = std::chrono::system_clock::now();
    return "Running"; // Placeholder
}

void SocketNetwork::Socket_Server::log_server_event(const std::string& message)
{
    if (config.enable_logging)
    {
        Utils::Logger::info("[SERVER] " + message);
    }
}

void SocketNetwork::Socket_Server::log_client_event(const Client_Info& client_info, const std::string& message)
{
    if (config.enable_logging)
    {
        std::string client_desc = client_info.ip_address + ":" + 
                                 Utils::Conversion::int_to_string(client_info.port);
        if (client_info.is_authenticated)
        {
            client_desc += " (User: " + client_info.username + ")";
        }
        Utils::Logger::info("[CLIENT " + client_desc + "] " + message);
    }
}