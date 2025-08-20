#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>
#include <map>

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QObject>
#include <QtCore/QTimer>

#include "network/Network_Types.h"
#include "database/Database_Manager.h"

// Forward declarations
class Protocol_Handler;

namespace SocketNetwork
{
	class Client_Handler;
}

namespace SocketNetwork
{
	class Socket_Server : public QObject
	{
		Q_OBJECT
		
	private:
		QTcpServer* tcp_server;
		Server_Config config;
		std::shared_ptr<Database::Database_Manager> db_manager;
		std::unique_ptr<Protocol_Handler> protocol_handler;

		std::atomic<bool> is_running;
		std::atomic<bool> is_initialized;
		QTimer* cleanup_timer;

		std::map<int, std::shared_ptr<Client_Handler>> active_clients;
		std::mutex clients_mutex;
		std::mutex protocol_handler_mutex;
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

		void disconnect_client(int client_socket_descriptor);
		void disconnect_all_clients();
		void broadcast_message(const std::string& message);
		void send_message_to_user(int user_id, const std::string& message);
		void send_message_to_client(int client_socket_descriptor, const std::string& message);

		Server_Stats get_server_stats() const;
		void reset_server_stats();

		std::function<void(const Client_Info&)> on_client_connected;
		std::function<void(const Client_Info&)> on_client_disconnected;
		std::function<void(const Client_Info&, const std::string&)> on_message_received;
		std::function<void(const std::string&)> on_server_error;

	// Friend declaration to allow Client_Handler to access private members
	friend class Client_Handler;

	private:
		bool initialize_qt_network();
		void cleanup_qt_network();
		bool create_server();
		bool bind_and_listen();
		void cleanup_disconnected_clients();

		void handle_new_client(QTcpSocket* client_socket);
		void remove_client(int client_socket_descriptor);
		Client_Info* get_client_info(int client_socket_descriptor);

		std::string get_socket_address(QTcpSocket* socket) const;
		std::string get_uptime() const;
		void log_server_event(const std::string& message);
		void log_client_event(const Client_Info& client_info, const std::string& message);

	private slots:
		void on_new_connection();
		void on_cleanup_timer();
	};
}