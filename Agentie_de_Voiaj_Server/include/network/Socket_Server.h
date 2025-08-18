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

#include "network/Network_Types.h"
#include "database/Database_Manager.h"

#pragma comment(lib, "ws2_32.lib")

// Forward declarations
class Protocol_Handler;

namespace SocketNetwork
{
	class Client_Handler;
}

namespace SocketNetwork
{
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