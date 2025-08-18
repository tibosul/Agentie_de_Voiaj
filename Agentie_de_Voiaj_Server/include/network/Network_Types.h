#pragma once

#include <string>
#include <atomic>
#include <functional>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#include "config.h"
#include "utils/utils.h"

namespace SocketNetwork
{
	struct Server_Config
	{
		std::string ip_address = "127.0.0.1";
		int port = Config::Server::PORT;
		int max_clients = Config::Server::MAX_CONNECTIONS;
		int receive_timeout_ms = Config::Server::SOCKET_TIMEOUT_MS;
		int send_timeout_ms = Config::Server::SOCKET_TIMEOUT_MS;
		int keep_alive_interval_ms = 60000;
		bool enable_logging = Config::Application::DEBUG_MODE;

		Server_Config() = default;
		Server_Config(const std::string& ip, int p)
			: ip_address(ip), port(p) 
		{
		}
	};

	struct Client_Info
	{
		SOCKET socket;
		std::string ip_address;
		int port;
		std::string connection_time;
		std::string last_activity_time;
		bool is_authenticated = false;
		int user_id = 0;
		std::string username;

		Client_Info(SOCKET s, const std::string& ip, int p)
			: socket(s), ip_address(ip), port(p)
		{
			connection_time = Utils::DateTime::get_current_date_time();
			last_activity_time = connection_time;
		}
	};

	struct Server_Stats
	{
		int active_clients;
		int total_connections;
		int total_messages_received;
		int total_messages_sent;
		std::string uptime;
		std::string start_time;
		double average_response_time_ms;
		int memory_usage_mb;
	};

	enum class Message_Type
	{
		AUTHENTICATION,
		REGISTRATION,
		GET_DESTINATIONS,
		GET_OFFERS,
		SEARCH_OFFERS,
		BOOK_OFFER,
		GET_USER_RESERVATIONS,
		CANCEL_RESERVATION,
		GET_USER_INFO,
		UPDATE_USER_INFO,
		// Admin message types reserved for future implementation
		KEEPALIVE,
		ERR,
		UNKNOWN
	};

	struct Parsed_Message
	{
		Message_Type type;
		std::string raw_message;
		nlohmann::json json_data; // Store parsed JSON data
		bool is_valid = false;
		std::string error_message;

		Parsed_Message() : type(Message_Type::UNKNOWN), is_valid(false)
		{
		}
	};

	struct Response
	{
		bool success = false;
		std::string message;
		std::string data; // JSON data
		int error_code = 0;

		Response(bool s = false, const std::string& msg = "", const std::string& d = "")
			: success(s), message(msg), data(d)
		{
		}
	};

	// Forward declarations
	class Socket_Server;
	class Client_Handler;
	class Protocol_Handler;
}