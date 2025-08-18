#include "network/Client_Handler.h"
#include "network/Protocol_Handler.h"
#include "network/Socket_Server.h"
#include <iostream>
#include <chrono>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>


// ============================================================================
// Client_Handler Implementation
// ============================================================================

SocketNetwork::Client_Handler::Client_Handler(SOCKET socket, const SocketNetwork::Client_Info& info,
    std::shared_ptr<Database::Database_Manager> db_manager,
    Protocol_Handler* protocol_handler, SocketNetwork::Socket_Server* server)
    : client_socket(socket), client_info(info), db_manager(db_manager),
      protocol_handler(protocol_handler), server(server), is_running(false)
{
    last_activity = std::chrono::steady_clock::now();
}

SocketNetwork::Client_Handler::~Client_Handler()
{
    stop_handling();
    if (client_socket != INVALID_SOCKET)
    {
        closesocket(client_socket);
    }
}

void SocketNetwork::Client_Handler::start_handling()
{
    if (is_running.load())
    {
        return;
    }
    
    is_running = true;
    handler_thread = std::thread(&Client_Handler::handle_client_loop, this);
}

void SocketNetwork::Client_Handler::stop_handling()
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

bool SocketNetwork::Client_Handler::is_client_running() const
{
    return is_running.load();
}

bool SocketNetwork::Client_Handler::send_message(const std::string& message)
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

std::string SocketNetwork::Client_Handler::receive_message()
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

const SocketNetwork::Client_Info& SocketNetwork::Client_Handler::get_client_info() const
{
    return client_info;
}

void SocketNetwork::Client_Handler::update_last_activity()
{
    last_activity = std::chrono::steady_clock::now();
    client_info.last_activity_time = Utils::DateTime::get_current_date_time();
}

bool SocketNetwork::Client_Handler::is_authenticated() const
{
    return client_info.is_authenticated;
}

void SocketNetwork::Client_Handler::set_authenticated(int user_id, const std::string& username)
{
    client_info.is_authenticated = true;
    client_info.user_id = user_id;
    client_info.username = username;
}

std::chrono::milliseconds SocketNetwork::Client_Handler::get_idle_time() const
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - last_activity);
}

void SocketNetwork::Client_Handler::handle_client_loop()
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

bool SocketNetwork::Client_Handler::process_message(const std::string& message)
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

void SocketNetwork::Client_Handler::handle_disconnection()
{
    is_running = false;
    
    if (server)
    {
        server->remove_client(client_socket);
    }
}

bool SocketNetwork::Client_Handler::is_socket_valid() const
{
    return client_socket != INVALID_SOCKET && is_running.load();
}

void SocketNetwork::Client_Handler::send_error_response(const std::string& error_message)
{
    std::string response = Utils::JSON::create_error_response(error_message);
    send_message(response);
}

void SocketNetwork::Client_Handler::send_success_response(const std::string& data, const std::string& message)
{
    std::string response = Utils::JSON::create_success_response(data, message);
    send_message(response);
}