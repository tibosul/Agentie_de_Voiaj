#include "Socket_Server.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace SocketNetwork;
using namespace Database;

// ============================================================================
// Socket_Server Implementation
// ============================================================================

Socket_Server::Socket_Server()
    : server_socket(INVALID_SOCKET), is_running(false), is_initialized(false),
      client_count(0), total_connections(0), total_messages_received(0), total_messages_sent(0)
{
    server_start_time = Utils::DateTime::get_current_date_time();
    Utils::Logger::info("Socket_Server created with default configuration");
}

Socket_Server::Socket_Server(const Server_Config& config)
    : server_socket(INVALID_SOCKET), config(config), is_running(false), is_initialized(false),
      client_count(0), total_connections(0), total_messages_received(0), total_messages_sent(0)
{
    server_start_time = Utils::DateTime::get_current_date_time();
    Utils::Logger::info("Socket_Server created with custom configuration - Port: " + 
                        Utils::Conversion::int_to_string(config.port));
}

Socket_Server::Socket_Server(const std::string& ip, int port)
    : server_socket(INVALID_SOCKET), is_running(false), is_initialized(false),
      client_count(0), total_connections(0), total_messages_received(0), total_messages_sent(0)
{
    config.ip_address = ip;
    config.port = port;
    server_start_time = Utils::DateTime::get_current_date_time();
    Utils::Logger::info("Socket_Server created - IP: " + ip + ", Port: " + 
                        Utils::Conversion::int_to_string(port));
}

Socket_Server::~Socket_Server()
{
    stop();
    cleanup_winsock();
    Utils::Logger::info("Socket_Server destroyed");
}

bool Socket_Server::initialize()
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
            closesocket(server_socket);
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

bool Socket_Server::start()
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

void Socket_Server::stop()
{
    if (!is_running.load())
    {
        return;
    }

    log_server_event("Stopping Socket_Server...");
    is_running = false;

    // Close server socket to interrupt accept()
    if (server_socket != INVALID_SOCKET)
    {
        closesocket(server_socket);
        server_socket = INVALID_SOCKET;
    }

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

bool Socket_Server::restart()
{
    log_server_event("Restarting Socket_Server...");
    stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return start();
}

void Socket_Server::set_config(const Server_Config& new_config)
{
    if (is_running.load())
    {
        Utils::Logger::warning("Cannot change configuration while server is running");
        return;
    }
    
    config = new_config;
    Utils::Logger::info("Server configuration updated");
}

void Socket_Server::set_database_manager(std::shared_ptr<Database_Manager> db_mgr)
{
    db_manager = db_mgr;
    if (protocol_handler)
    {
        protocol_handler = std::make_unique<Protocol_Handler>(db_manager);
    }
    Utils::Logger::info("Database manager set for Socket_Server");
}

Server_Config Socket_Server::get_config() const
{
    return config;
}

bool Socket_Server::is_server_running() const
{
    return is_running.load();
}

bool Socket_Server::is_server_initialized() const
{
    return is_initialized.load();
}

int Socket_Server::get_active_client_count() const
{
    return client_count.load();
}

std::vector<Client_Info> Socket_Server::get_active_clients() const
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

void Socket_Server::disconnect_client(SOCKET client_socket)
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

void Socket_Server::disconnect_all_clients()
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

void Socket_Server::broadcast_message(const std::string& message)
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

void Socket_Server::send_message_to_user(int user_id, const std::string& message)
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

void Socket_Server::send_message_to_client(SOCKET client_socket, const std::string& message)
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

Socket_Server::Server_Stats Socket_Server::get_server_stats() const
{
    Server_Stats stats;
    stats.active_clients = client_count.load();
    stats.total_connections = total_connections.load();
    stats.total_messages_received = total_messages_received.load();
    stats.total_messages_sent = total_messages_sent.load();
    stats.uptime = get_uptime();
    stats.start_time = server_start_time;
    stats.average_response_time_ms = 0.0; // TODO: Implement response time tracking
    stats.memory_usage_mb = static_cast<int>(Utils::Memory::get_memory_usage_MB());
    
    return stats;
}

void Socket_Server::reset_server_stats()
{
    total_connections = 0;
    total_messages_received = 0;
    total_messages_sent = 0;
    server_start_time = Utils::DateTime::get_current_date_time();
    log_server_event("Server statistics reset");
}

// Private methods implementation

bool Socket_Server::initialize_winsock()
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

void Socket_Server::cleanup_winsock()
{
    WSACleanup();
}

bool Socket_Server::create_server_socket()
{
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (server_socket == INVALID_SOCKET)
    {
        log_server_event("Socket creation failed with error: " + 
                        Utils::Conversion::int_to_string(WSAGetLastError()));
        return false;
    }
    
    if (!set_socket_options(server_socket))
    {
        closesocket(server_socket);
        return false;
    }
    
    return true;
}

bool Socket_Server::bind_and_listen()
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

void Socket_Server::accept_connections_loop()
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

void Socket_Server::cleanup_disconnected_clients()
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

void Socket_Server::handle_new_client(SOCKET client_socket, const std::string& client_address)
{
    try 
    {
        if (!set_socket_options(client_socket))
        {
            closesocket(client_socket);
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
        
        if (!protocol_handler)
        {
            protocol_handler = std::make_unique<Protocol_Handler>(db_manager);
        }
        
        auto client_handler = std::make_unique<Client_Handler>(
            client_socket, client_info, db_manager, protocol_handler.get(), this);
        
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            active_clients[client_socket] = std::move(client_handler);
            client_count++;
        }
        
        active_clients[client_socket]->start_handling();
        
        log_client_event(client_info, "New client connected");
        
        if (on_client_connected)
        {
            on_client_connected(client_info);
        }
    }
    catch (const std::exception& e)
    {
        log_server_event("Exception handling new client: " + std::string(e.what()));
        closesocket(client_socket);
    }
}

void Socket_Server::remove_client(SOCKET client_socket)
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

Client_Info* Socket_Server::get_client_info(SOCKET client_socket)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = active_clients.find(client_socket);
    
    if (it != active_clients.end() && it->second)
    {
        return const_cast<Client_Info*>(&it->second->get_client_info());
    }
    
    return nullptr;
}

std::string Socket_Server::get_socket_address(SOCKET socket) const
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

bool Socket_Server::set_socket_options(SOCKET socket)
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

std::string Socket_Server::get_uptime() const
{
    // TODO: Implement proper uptime calculation
    auto now = std::chrono::system_clock::now();
    return "Running"; // Placeholder
}

void Socket_Server::log_server_event(const std::string& message)
{
    if (config.enable_logging)
    {
        Utils::Logger::info("[SERVER] " + message);
    }
}

void Socket_Server::log_client_event(const Client_Info& client_info, const std::string& message)
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

// ============================================================================
// Client_Handler Implementation
// ============================================================================

Client_Handler::Client_Handler(SOCKET socket, const SocketNetwork::Client_Info& info,
    std::shared_ptr<Database::Database_Manager> db_manager,
    Protocol_Handler* protocol_handler, SocketNetwork::Socket_Server* server)
    : client_socket(socket), client_info(info), db_manager(db_manager),
      protocol_handler(protocol_handler), server(server), is_running(false)
{
    last_activity = std::chrono::steady_clock::now();
}

Client_Handler::~Client_Handler()
{
    stop_handling();
    if (client_socket != INVALID_SOCKET)
    {
        closesocket(client_socket);
    }
}

void Client_Handler::start_handling()
{
    if (is_running.load())
    {
        return;
    }
    
    is_running = true;
    handler_thread = std::thread(&Client_Handler::handle_client_loop, this);
}

void Client_Handler::stop_handling()
{
    if (!is_running.load())
    {
        return;
    }
    
    is_running = false;
    
    if (client_socket != INVALID_SOCKET)
    {
        shutdown(client_socket, SD_BOTH);
        closesocket(client_socket);
        client_socket = INVALID_SOCKET;
    }
    
    if (handler_thread.joinable())
    {
        handler_thread.join();
    }
}

bool Client_Handler::is_client_running() const
{
    return is_running.load();
}

bool Client_Handler::send_message(const std::string& message)
{
    if (!is_socket_valid())
    {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(send_mutex);
    
    try 
    {
        std::string full_message = message + "\r\n";
        int bytes_sent = send(client_socket, full_message.c_str(), 
                             static_cast<int>(full_message.length()), 0);
        
        if (bytes_sent == SOCKET_ERROR)
        {
            return false;
        }
        
        messages_sent++;
        update_last_activity();
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::string Client_Handler::receive_message()
{
    if (!is_socket_valid())
    {
        return "";
    }
    
    char buffer[Config::Server::BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0)
    {
        return "";
    }
    
    buffer[bytes_received] = '\0';
    std::string message(buffer);
    
    // Remove trailing \r\n
    while (!message.empty() && (message.back() == '\r' || message.back() == '\n'))
    {
        message.pop_back();
    }
    
    messages_received++;
    update_last_activity();
    
    return message;
}

const SocketNetwork::Client_Info& Client_Handler::get_client_info() const
{
    return client_info;
}

void Client_Handler::update_last_activity()
{
    last_activity = std::chrono::steady_clock::now();
    client_info.last_activity_time = Utils::DateTime::get_current_date_time();
}

bool Client_Handler::is_authenticated() const
{
    return client_info.is_authenticated;
}

void Client_Handler::set_authenticated(int user_id, const std::string& username)
{
    client_info.is_authenticated = true;
    client_info.user_id = user_id;
    client_info.username = username;
}

std::chrono::milliseconds Client_Handler::get_idle_time() const
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - last_activity);
}

void Client_Handler::handle_client_loop()
{
    while (is_running.load())
    {
        try 
        {
            std::string message = receive_message();
            
            if (message.empty())
            {
                if (WSAGetLastError() == WSAETIMEDOUT)
                {
                    continue; // Timeout is normal, continue listening
                }
                break; // Client disconnected or error
            }
            
            if (!process_message(message))
            {
                break; // Error processing message
            }
        }
        catch (const std::exception& e)
        {
            send_error_response("Internal server error: " + std::string(e.what()));
            break;
        }
    }
    
    handle_disconnection();
}

bool Client_Handler::process_message(const std::string& message)
{
    if (!protocol_handler)
    {
        send_error_response(Config::ErrorMessages::SERVER_ERROR);
        return false;
    }
    
    try 
    {
        auto parsed_message = protocol_handler->parse_message(message);
        
        if (!parsed_message.is_valid)
        {
            send_error_response(parsed_message.error_message);
            return true; // Continue handling other messages
        }
        
        auto response = protocol_handler->process_message(parsed_message, this);
        
        std::string response_str;
        if (response.success)
        {
            response_str = Utils::JSON::create_success_response(response.data, response.message);
        }
        else 
        {
            response_str = Utils::JSON::create_error_response(response.message, response.error_code);
        }
        
        return send_message(response_str);
    }
    catch (const std::exception& e)
    {
        send_error_response("Message processing error: " + std::string(e.what()));
        return true;
    }
}

void Client_Handler::handle_disconnection()
{
    is_running = false;
    
    if (server)
    {
        server->remove_client(client_socket);
    }
}

bool Client_Handler::is_socket_valid() const
{
    return client_socket != INVALID_SOCKET && is_running.load();
}

void Client_Handler::send_error_response(const std::string& error_message)
{
    std::string response = Utils::JSON::create_error_response(error_message);
    send_message(response);
}

void Client_Handler::send_success_response(const std::string& data, const std::string& message)
{
    std::string response = Utils::JSON::create_success_response(data, message);
    send_message(response);
}

// ============================================================================
// Protocol_Handler Implementation
// ============================================================================

Protocol_Handler::Protocol_Handler(std::shared_ptr<Database::Database_Manager> db_manager)
    : db_manager(db_manager)
{
}

Protocol_Handler::Parsed_Message Protocol_Handler::parse_message(const std::string& json_message)
{
    Parsed_Message parsed;
    parsed.raw_message = json_message;
    
    if (json_message.empty())
    {
        parsed.error_message = Config::ErrorMessages::INVALID_REQUEST;
        return parsed;
    }
    
    try 
    {
        nlohmann::json msg_json = nlohmann::json::parse(json_message);
        parsed.json_data = msg_json;
        
        // Extract command/type from JSON
        if (!msg_json.contains("type") && !msg_json.contains("command"))
        {
            parsed.error_message = "Missing 'type' or 'command' field in JSON message";
            return parsed;
        }
        
        parsed.type = get_message_type(msg_json);
        
        if (parsed.type == Message_Type::UNKNOWN)
        {
            std::string command;
            if (msg_json.contains("type"))
            {
                command = msg_json["type"].get<std::string>();
            }
            else
            {
                command = msg_json["command"].get<std::string>();
            }
            parsed.error_message = "Unknown command: " + command;
            return parsed;
        }
        
        parsed.is_valid = true;
        return parsed;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        parsed.error_message = "JSON parse error: " + std::string(e.what());
        return parsed;
    }
    catch (const std::exception& e)
    {
        parsed.error_message = "Parse error: " + std::string(e.what());
        return parsed;
    }
}

Protocol_Handler::Message_Type Protocol_Handler::get_message_type(const nlohmann::json& json_obj)
{
    try
    {
        std::string command;
        if (json_obj.contains("type"))
        {
            command = json_obj["type"].get<std::string>();
        }
        else if (json_obj.contains("command"))
        {
            command = json_obj["command"].get<std::string>();
        }
        else
        {
            return Message_Type::UNKNOWN;
        }
        
        // Convert to uppercase for comparison
        std::string cmd = Utils::String::to_upper(Utils::String::trim(command));
        
        if (cmd == "AUTH" || cmd == "LOGIN") return Message_Type::AUTHENTICATION;
        if (cmd == "REGISTER" || cmd == "SIGNUP") return Message_Type::REGISTRATION;
        if (cmd == "GET_DESTINATIONS") return Message_Type::GET_DESTINATIONS;
        if (cmd == "GET_OFFERS") return Message_Type::GET_OFFERS;
        if (cmd == "SEARCH_OFFERS") return Message_Type::SEARCH_OFFERS;
        if (cmd == "BOOK_OFFER") return Message_Type::BOOK_OFFER;
        if (cmd == "GET_USER_RESERVATIONS") return Message_Type::GET_USER_RESERVATIONS;
        if (cmd == "CANCEL_RESERVATION") return Message_Type::CANCEL_RESERVATION;
        if (cmd == "GET_USER_INFO") return Message_Type::GET_USER_INFO;
        if (cmd == "UPDATE_USER_INFO") return Message_Type::UPDATE_USER_INFO;
        // Admin commands not supported in college project scope
        // if (cmd == "ADMIN_GET_STATS") return Message_Type::ADMIN_GET_STATS;
        // if (cmd == "ADMIN_GET_USERS") return Message_Type::ADMIN_GET_USERS;
        // if (cmd == "ADMIN_MANAGE_OFFERS") return Message_Type::ADMIN_MANAGE_OFFERS;
        if (cmd == "KEEPALIVE" || cmd == "PING") return Message_Type::KEEPALIVE;
        if (cmd == "ERROR") return Message_Type::ERR;
        
        return Message_Type::UNKNOWN;
    }
    catch (const std::exception&)
    {
        return Message_Type::UNKNOWN;
    }
}

std::string Protocol_Handler::message_type_to_string(Message_Type type)
{
    switch (type)
    {
        case Message_Type::AUTHENTICATION: return "AUTHENTICATION";
        case Message_Type::REGISTRATION: return "REGISTRATION";
        case Message_Type::GET_DESTINATIONS: return "GET_DESTINATIONS";
        case Message_Type::GET_OFFERS: return "GET_OFFERS";
        case Message_Type::SEARCH_OFFERS: return "SEARCH_OFFERS";
        case Message_Type::BOOK_OFFER: return "BOOK_OFFER";
        case Message_Type::GET_USER_RESERVATIONS: return "GET_USER_RESERVATIONS";
        case Message_Type::CANCEL_RESERVATION: return "CANCEL_RESERVATION";
        case Message_Type::GET_USER_INFO: return "GET_USER_INFO";
        case Message_Type::UPDATE_USER_INFO: return "UPDATE_USER_INFO";
        // Admin message types not supported in college project scope
        // case Message_Type::ADMIN_GET_STATS: return "ADMIN_GET_STATS";
        // case Message_Type::ADMIN_GET_USERS: return "ADMIN_GET_USERS"; 
        // case Message_Type::ADMIN_MANAGE_OFFERS: return "ADMIN_MANAGE_OFFERS";
        case Message_Type::KEEPALIVE: return "KEEPALIVE";
        case Message_Type::ERR: return "ERROR";
        case Message_Type::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

Protocol_Handler::Response Protocol_Handler::process_message(const Parsed_Message& parsed_message, Client_Handler* client_handler)
{
    if (!client_handler)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR);
    }
    
    try 
    {
        switch (parsed_message.type)
        {
            case Message_Type::AUTHENTICATION:
                return handle_authentication(parsed_message, client_handler);
            
            case Message_Type::REGISTRATION:
                return handle_registration(parsed_message, client_handler);
            
            case Message_Type::GET_DESTINATIONS:
                return handle_get_destinations(parsed_message, client_handler);
            
            case Message_Type::GET_OFFERS:
                return handle_get_offers(parsed_message, client_handler);
            
            case Message_Type::SEARCH_OFFERS:
                return handle_search_offers(parsed_message, client_handler);
            
            case Message_Type::BOOK_OFFER:
                return handle_book_offer(parsed_message, client_handler);
            
            case Message_Type::GET_USER_RESERVATIONS:
                return handle_get_user_reservations(parsed_message, client_handler);
            
            case Message_Type::CANCEL_RESERVATION:
                return handle_cancel_reservation(parsed_message, client_handler);
            
            case Message_Type::GET_USER_INFO:
                return handle_get_user_info(parsed_message, client_handler);
            
            case Message_Type::UPDATE_USER_INFO:
                return handle_update_user_info(parsed_message, client_handler);
            
            case Message_Type::KEEPALIVE:
                return handle_keepalive(parsed_message, client_handler);
            
            // Admin functions not implemented for college project scope
            // case Message_Type::ADMIN_GET_STATS:
            //     return handle_admin_get_stats(parsed_message, client_handler);
            // case Message_Type::ADMIN_GET_USERS:
            //     return handle_admin_get_users(parsed_message, client_handler);
            // case Message_Type::ADMIN_MANAGE_OFFERS:
            //     return handle_admin_manage_offers(parsed_message, client_handler);
            
            default:
                return Response(false, "Unsupported message type");
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

std::string Protocol_Handler::create_response(bool success, const std::string& message,
    const std::string& data, int error_code)
{
    nlohmann::json response;
    response["success"] = success;
    response["message"] = message;
    
    if (success && !data.empty())
    {
        try
        {
            // Try to parse data as JSON, if it fails treat as string
            response["data"] = nlohmann::json::parse(data);
        }
        catch (const nlohmann::json::parse_error&)
        {
            response["data"] = data;
        }
    }
    else if (success)
    {
        response["data"] = nlohmann::json::object();
    }
    
    if (!success && error_code != 0)
    {
        response["error_code"] = error_code;
    }
    
    return response.dump();
}

Protocol_Handler::Response Protocol_Handler::handle_authentication(const Parsed_Message& message, Client_Handler* client)
{
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    // Check required fields in JSON
    if (!message.json_data.contains("username") || !message.json_data.contains("password"))
    {
        return Response(false, "Missing required fields: username, password");
    }
    
    try 
    {
        std::string username = message.json_data["username"].get<std::string>();
        std::string password = message.json_data["password"].get<std::string>();
        
        auto result = db_manager->authenticate_user(username, password);
        
        if (result.is_success() && result.has_data())
        {
            int user_id = Utils::Conversion::string_to_int(result.data[0]["ID"]);
            client->set_authenticated(user_id, username);
            
            nlohmann::json user_data;
            for (const auto& pair : result.data[0])
            {
                user_data[pair.first] = pair.second;
            }
            
            return Response(true, Config::SuccessMessages::LOGIN_SUCCESS, user_data.dump());
        }
        else 
        {
            return Response(false, Config::ErrorMessages::AUTHENTICATION_FAILED);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

Protocol_Handler::Response Protocol_Handler::handle_registration(const Parsed_Message& message, Client_Handler* client)
{
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    // Check required fields in JSON
    const std::vector<std::string> required_fields = {"username", "password", "email", "first_name", "last_name"};
    for (const auto& field : required_fields)
    {
        if (!message.json_data.contains(field))
        {
            return Response(false, "Missing required field: " + field);
        }
    }
    
    try 
    {
        User_Data user_data;
        user_data.username = message.json_data["username"].get<std::string>();
        user_data.password_hash = message.json_data["password"].get<std::string>(); // Will be hashed in DB layer
        user_data.email = message.json_data["email"].get<std::string>();
        user_data.first_name = message.json_data["first_name"].get<std::string>();
        user_data.last_name = message.json_data["last_name"].get<std::string>();
        
        if (message.json_data.contains("phone_number"))
        {
            user_data.phone_number = message.json_data["phone_number"].get<std::string>();
        }
        
        auto result = db_manager->register_user(user_data);
        
        if (result.is_success())
        {
            return Response(true, Config::SuccessMessages::USER_CREATED);
        }
        else 
        {
            return Response(false, result.message);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

Protocol_Handler::Response Protocol_Handler::handle_get_destinations(const Parsed_Message& message, Client_Handler* client)
{
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    try 
    {
        auto result = db_manager->get_all_destinations();
        
        if (result.is_success())
        {
            std::string destinations_json = vector_to_json(result.data);
            return Response(true, Config::SuccessMessages::DATA_RETRIEVED, destinations_json);
        }
        else 
        {
            return Response(false, result.message);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

Protocol_Handler::Response Protocol_Handler::handle_get_offers(const Parsed_Message& message, Client_Handler* client)
{
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    try 
    {
        auto result = db_manager->get_available_offers();
        
        if (result.is_success())
        {
            std::string offers_json = vector_to_json(result.data);
            return Response(true, Config::SuccessMessages::DATA_RETRIEVED, offers_json);
        }
        else 
        {
            return Response(false, result.message);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

Protocol_Handler::Response Protocol_Handler::handle_search_offers(const Parsed_Message& message, Client_Handler* client)
{
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    try 
    {
        std::string destination = message.json_data.contains("destination") ? message.json_data["destination"].get<std::string>() : "";
        double min_price = message.json_data.contains("min_price") ? message.json_data["min_price"].get<double>() : 0.0;
        double max_price = message.json_data.contains("max_price") ? message.json_data["max_price"].get<double>() : 0.0;
        std::string start_date = message.json_data.contains("start_date") ? message.json_data["start_date"].get<std::string>() : "";
        std::string end_date = message.json_data.contains("end_date") ? message.json_data["end_date"].get<std::string>() : "";
        
        auto result = db_manager->search_offers(destination, min_price, max_price, start_date, end_date);
        
        if (result.is_success())
        {
            std::string offers_json = vector_to_json(result.data);
            return Response(true, Config::SuccessMessages::DATA_RETRIEVED, offers_json);
        }
        else 
        {
            return Response(false, result.message);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

Protocol_Handler::Response Protocol_Handler::handle_book_offer(const Parsed_Message& message, Client_Handler* client)
{
    if (!client->is_authenticated())
    {
        return Response(false, Config::ErrorMessages::AUTHENTICATION_FAILED);
    }
    
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    if (!message.json_data.contains("offer_id"))
    {
        return Response(false, "Missing required field: offer_id");
    }
    
    try 
    {
        int offer_id = message.json_data["offer_id"].get<int>();
        int person_count = message.json_data.contains("person_count") ? message.json_data["person_count"].get<int>() : 1;
        
        // Validate person count
        if (person_count < 1 || person_count > Config::Business::MAX_PERSONS_PER_RESERVATION)
        {
            return Response(false, "Invalid person count");
        }
        
        auto result = db_manager->book_offer(client->get_client_info().user_id, offer_id, person_count);
        
        if (result.is_success())
        {
            return Response(true, Config::SuccessMessages::RESERVATION_CREATED);
        }
        else 
        {
            return Response(false, result.message);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

Protocol_Handler::Response Protocol_Handler::handle_get_user_reservations(const Parsed_Message& message, Client_Handler* client)
{
    if (!client->is_authenticated())
    {
        return Response(false, Config::ErrorMessages::AUTHENTICATION_FAILED);
    }
    
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    try 
    {
        auto result = db_manager->get_user_reservations(client->get_client_info().user_id);
        
        if (result.is_success())
        {
            std::string reservations_json = vector_to_json(result.data);
            return Response(true, Config::SuccessMessages::DATA_RETRIEVED, reservations_json);
        }
        else 
        {
            return Response(false, result.message);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

Protocol_Handler::Response Protocol_Handler::handle_cancel_reservation(const Parsed_Message& message, Client_Handler* client)
{
    if (!client->is_authenticated())
    {
        return Response(false, Config::ErrorMessages::AUTHENTICATION_FAILED);
    }
    
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    if (!message.json_data.contains("reservation_id"))
    {
        return Response(false, "Missing required field: reservation_id");
    }
    
    try 
    {
        int reservation_id = message.json_data["reservation_id"].get<int>();
        
        auto result = db_manager->cancel_reservation(reservation_id);
        
        if (result.is_success())
        {
            return Response(true, Config::SuccessMessages::RESERVATION_CANCELLED);
        }
        else 
        {
            return Response(false, result.message);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

Protocol_Handler::Response Protocol_Handler::handle_get_user_info(const Parsed_Message& message, Client_Handler* client)
{
    if (!client->is_authenticated())
    {
        return Response(false, Config::ErrorMessages::AUTHENTICATION_FAILED);
    }
    
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    try 
    {
        auto result = db_manager->get_user_by_id(client->get_client_info().user_id);
        
        if (result.is_success() && result.has_data())
        {
            nlohmann::json user_data;
            for (const auto& pair : result.data[0])
            {
                user_data[pair.first] = pair.second;
            }
            return Response(true, Config::SuccessMessages::DATA_RETRIEVED, user_data.dump());
        }
        else 
        {
            return Response(false, Config::ErrorMessages::USER_NOT_FOUND);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

Protocol_Handler::Response Protocol_Handler::handle_update_user_info(const Parsed_Message& message, Client_Handler* client)
{
    if (!client->is_authenticated())
    {
        return Response(false, Config::ErrorMessages::AUTHENTICATION_FAILED);
    }
    
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    try 
    {
        // Get current user data
        auto current_result = db_manager->get_user_by_id(client->get_client_info().user_id);
        if (!current_result.is_success() || !current_result.has_data())
        {
            return Response(false, Config::ErrorMessages::USER_NOT_FOUND);
        }
        
        User_Data user_data;
        user_data.id = client->get_client_info().user_id;
        user_data.username = current_result.data[0]["Username"];
        user_data.password_hash = current_result.data[0]["Password_Hash"];
        
        // Update only provided fields
        user_data.email = message.json_data.contains("email") ? 
                          message.json_data["email"].get<std::string>() : current_result.data[0]["Email"];
        user_data.first_name = message.json_data.contains("first_name") ? 
                              message.json_data["first_name"].get<std::string>() : current_result.data[0]["First_Name"];
        user_data.last_name = message.json_data.contains("last_name") ? 
                             message.json_data["last_name"].get<std::string>() : current_result.data[0]["Last_Name"];
        user_data.phone_number = message.json_data.contains("phone_number") ? 
                                message.json_data["phone_number"].get<std::string>() : current_result.data[0]["Phone_Number"];
        
        auto result = db_manager->update_user(user_data);
        
        if (result.is_success())
        {
            return Response(true, "User information updated successfully");
        }
        else 
        {
            return Response(false, result.message);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

Protocol_Handler::Response Protocol_Handler::handle_keepalive(const Parsed_Message& message, Client_Handler* client)
{
    return Response(true, "PONG");
}

// Admin functions commented out - not implemented for college project scope
/*
Protocol_Handler::Response Protocol_Handler::handle_admin_get_stats(const Parsed_Message& message, Client_Handler* client)
{
    // Not implemented - college project focuses on client functionality only
    return Response(false, "Admin functions not available in this version");
}

Protocol_Handler::Response Protocol_Handler::handle_admin_get_users(const Parsed_Message& message, Client_Handler* client)
{
    // Not implemented - college project focuses on client functionality only
    return Response(false, "Admin functions not available in this version");
}

Protocol_Handler::Response Protocol_Handler::handle_admin_manage_offers(const Parsed_Message& message, Client_Handler* client)
{
    // Not implemented - college project focuses on client functionality only
    return Response(false, "Admin functions not available in this version");
}
*/


bool Protocol_Handler::is_user_admin(int user_id)
{
    // TODO: Implement proper admin check - for now, assume user_id 1 is admin
    return user_id == 1;
}

std::string Protocol_Handler::vector_to_json(const std::vector<std::map<std::string, std::string>>& data)
{
    if (data.empty())
    {
        return "[]";
    }
    
    nlohmann::json json_array = nlohmann::json::array();
    for (const auto& row : data)
    {
        nlohmann::json json_obj;
        for (const auto& pair : row)
        {
            json_obj[pair.first] = pair.second;
        }
        json_array.push_back(json_obj);
    }
    
    return json_array.dump();
}


std::string Protocol_Handler::create_json_response(bool success, const std::string& message, const nlohmann::json& data, int error_code)
{
    nlohmann::json response;
    response["success"] = success;
    response["message"] = message;
    response["data"] = data;
    
    if (!success && error_code != 0)
    {
        response["error_code"] = error_code;
    }
    
    return response.dump();
}