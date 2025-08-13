#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <map>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "utils.h"
#include "Database_Manager.h"
#include "config.h"

#pragma comment(lib, "ws2_32.lib")

class Client_Handler;
class Protocol_Handler;

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

	class Socket_Server
	{
	private:
		SOCKET server_socket;
		Server_Config config;
		std::shared_ptr<Database::Database_Manager> db_manager;
		std::unique_ptr<Protocol_Handler> protocol_handler;

		std::atomic<bool> is_running;
		std::atomic<bool> is_initialized;
		std::thread accept_thread;
		std::thread cleanup_thread;

		std::map<SOCKET, std::unique_ptr<Client_Handler>> active_clients;
		std::mutex clients_mutex;
		std::atomic<int> client_count;

		std::atomic<int> total_connections;
		std::atomic<int> total_messages_received;
		std::atomic<int> total_messages_sent;
		std::string server_start_time;

	public:
		Socket_Server();
		explicit Socket_Server(const Server_Config& config);
		Socket_Server(const std::string& ip, int port);
		~Socket_Server();

		bool initialize();
		bool start();
		void stop();
		bool restart();

		void set_config(const Server_Config& config);
		void set_database_manager(std::shared_ptr<Database::Database_Manager> db_manager);
		Server_Config get_config() const;

		bool is_server_running() const;
		bool is_server_initialized() const;
		int get_active_client_count() const;
		std::vector<Client_Info> get_active_clients() const;

		void disconnect_client(SOCKET client_socket);
		void disconnect_all_clients();
		void broadcast_message(const std::string& message);
		void send_message_to_user(int user_id, const std::string& message);
		void send_message_to_client(SOCKET client_socket, const std::string& message);

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

		Server_Stats get_server_stats() const;
		void reset_server_stats();

		std::function<void(const Client_Info&)> on_client_connected;
		std::function<void(const Client_Info&)> on_client_disconnected;
		std::function<void(const Client_Info&, const std::string&)> on_message_received;
		std::function<void(const std::string&)> on_server_error;

	// Friend declaration to allow Client_Handler to access private members
	friend class Client_Handler;

	private:
		bool initialize_winsock();
		void cleanup_winsock();
		bool create_server_socket();
		bool bind_and_listen();
		void accept_connections_loop();
		void cleanup_disconnected_clients();

		void handle_new_client(SOCKET client_socket, const std::string& client_address);
		void remove_client(SOCKET client_socket);
		Client_Info* get_client_info(SOCKET client_socket);

		std::string get_socket_address(SOCKET socket) const;
		bool set_socket_options(SOCKET socket);
		std::string get_uptime() const;
		void log_server_event(const std::string& message);
		void log_client_event(const Client_Info& client_info, const std::string& message);
	};
}

class Client_Handler
{
private:
	SOCKET client_socket;
	SocketNetwork::Client_Info client_info;
	std::shared_ptr<Database::Database_Manager> db_manager;
	Protocol_Handler* protocol_handler;
	SocketNetwork::Socket_Server* server;

	std::thread handler_thread;
	std::atomic<bool> is_running;
	std::mutex send_mutex;

	std::chrono::steady_clock::time_point last_activity;
	int messages_received = 0;
	int messages_sent = 0;

public:
	Client_Handler(SOCKET socket, const SocketNetwork::Client_Info& info,
		std::shared_ptr<Database::Database_Manager> db_manager,
		Protocol_Handler* protocol_handler, SocketNetwork::Socket_Server* server);
	~Client_Handler();

	void start_handling();
	void stop_handling();
	bool is_client_running() const;

	bool send_message(const std::string& message);
	std::string receive_message();

	const SocketNetwork::Client_Info& get_client_info() const;
	void update_last_activity();
	bool is_authenticated() const;
	void set_authenticated(int user_id, const std::string& username);

	int get_messages_received() const 
	{ 
		return messages_received; 
	}
	
	int get_messages_sent() const 
	{ 
		return messages_sent; 
	}
	std::chrono::milliseconds get_idle_time() const;

private:
	void handle_client_loop();
	bool process_message(const std::string& message);
	void handle_disconnection();
	bool is_socket_valid() const;
	void send_error_response(const std::string& error_message);
	void send_success_response(const std::string& data = "", const std::string& message = "");
};

class Protocol_Handler
{
public:
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
		ADMIN_GET_STATS,
		ADMIN_GET_USERS,
		ADMIN_MANAGE_OFFERS,
		KEEPALIVE,
		ERR,
		UNKNOWN
	};

	struct Parsed_Message
	{
		Message_Type type;
		std::string raw_message;
		std::map<std::string, std::string> parameters;
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

private:
	std::shared_ptr<Database::Database_Manager> db_manager;

public:
	explicit Protocol_Handler(std::shared_ptr<Database::Database_Manager> db_manager);

	Parsed_Message parse_message(const std::string& message);
	Message_Type get_message_type(const std::string& message);
	std::string message_type_to_string(Message_Type type);

	Response process_message(const Parsed_Message& parsed_message, Client_Handler* client_handler);
	std::string create_response(bool success, const std::string& message = "",
		const std::string& data = "", int error_code = 0);
	// Note: Use Utils::JSON::create_error_response and Utils::JSON::create_success_response
	// Note: Use Config::ErrorMessages and Config::SuccessMessages constants in implementation

	Response handle_authentication(const Parsed_Message& message, Client_Handler* client);
	Response handle_registration(const Parsed_Message& message, Client_Handler* client);
	Response handle_get_destinations(const Parsed_Message& message, Client_Handler* client);
	Response handle_get_offers(const Parsed_Message& message, Client_Handler* client);
	Response handle_search_offers(const Parsed_Message& message, Client_Handler* client);
	Response handle_book_offer(const Parsed_Message& message, Client_Handler* client);
	Response handle_get_user_reservations(const Parsed_Message& message, Client_Handler* client);
	Response handle_cancel_reservation(const Parsed_Message& message, Client_Handler* client);
	Response handle_get_user_info(const Parsed_Message& message, Client_Handler* client);
	Response handle_update_user_info(const Parsed_Message& message, Client_Handler* client);
	Response handle_keepalive(const Parsed_Message& message, Client_Handler* client);

	Response handle_admin_get_stats(const Parsed_Message& message, Client_Handler* client);
	Response handle_admin_get_users(const Parsed_Message& message, Client_Handler* client);
	Response handle_admin_manage_offers(const Parsed_Message& message, Client_Handler* client);

	bool validate_required_parameters(const Parsed_Message& message, 
		const std::vector<std::string>& required_params,
		std::string& error_message);
	bool is_user_admin(int user_id);
	// Note: Use Config::Business and Config::Security constants for validation limits

private:
	// JSON conversion functions - consider using a proper JSON library
	std::string vector_to_json(const std::vector<std::map<std::string, std::string>>& data);
	std::string map_to_json(const std::map<std::string, std::string>& data);
};