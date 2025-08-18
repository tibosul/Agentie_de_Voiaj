#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#include "network/Network_Types.h"
#include "database/Database_Manager.h"

// Forward declarations

namespace SocketNetwork
{
	class Socket_Server;
	class Protocol_Handler;
}

namespace SocketNetwork
{
	class Client_Handler
	{
	private:
		SocketRAII client_socket;
		Client_Info client_info;
		std::shared_ptr<Database::Database_Manager> db_manager;
		Protocol_Handler* protocol_handler;
		Socket_Server* server;

		std::thread handler_thread;
		std::atomic<bool> is_running;
		std::mutex send_mutex;

		std::chrono::steady_clock::time_point last_activity;
		int messages_received = 0;
		int messages_sent = 0;

	public:
		Client_Handler(SOCKET socket, const Client_Info& info,
			std::shared_ptr<Database::Database_Manager> db_manager,
			Protocol_Handler* protocol_handler, Socket_Server* server);
		~Client_Handler();

		void start_handling();
		void stop_handling();
		bool is_client_running() const;

		bool send_message(const std::string& message);
		std::string receive_message();

		const Client_Info& get_client_info() const;
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
}