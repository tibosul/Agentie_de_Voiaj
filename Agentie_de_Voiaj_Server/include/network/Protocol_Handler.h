#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

#include "network/Network_Types.h"
#include "database/Database_Manager.h"

// Forward declarations
namespace SocketNetwork
{
	class Client_Handler;
}

namespace SocketNetwork
{
	class Protocol_Handler
	{
	private:
		std::shared_ptr<Database::Database_Manager> db_manager;

	public:
		explicit Protocol_Handler(std::shared_ptr<Database::Database_Manager> db_manager);

		Parsed_Message parse_message(const std::string& json_message);
		Message_Type get_message_type(const nlohmann::json& json_obj);
		std::string message_type_to_string(Message_Type type);

		Response process_message(const Parsed_Message& parsed_message, SocketNetwork::Client_Handler* client_handler);
		std::string create_response(bool success, const std::string& message = "",
			const std::string& data = "", int error_code = 0);
		// Note: Use Utils::JSON::create_error_response and Utils::JSON::create_success_response
		// Note: Use Config::ErrorMessages and Config::SuccessMessages constants in implementation

		Response handle_authentication(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		Response handle_registration(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		Response handle_get_destinations(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		Response handle_get_offers(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		Response handle_search_offers(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		Response handle_book_offer(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		Response handle_get_user_reservations(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		Response handle_cancel_reservation(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		Response handle_get_user_info(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		Response handle_update_user_info(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		Response handle_keepalive(const Parsed_Message& message, SocketNetwork::Client_Handler* client);

		// Admin functions not implemented for college project scope
		// Response handle_admin_get_stats(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		// Response handle_admin_get_users(const Parsed_Message& message, SocketNetwork::Client_Handler* client);
		// Response handle_admin_manage_offers(const Parsed_Message& message, SocketNetwork::Client_Handler* client);

		// bool validate_required_parameters(const Parsed_Message& message,  // Removed - not needed with JSON
		//	const std::vector<std::string>& required_params,
		//	std::string& error_message);
		bool is_user_admin(int user_id);  // Keep for potential future use
		// Note: Use Config::Business and Config::Security constants for validation limits


	private:
		// JSON utilities
		std::string vector_to_json(const std::vector<std::map<std::string, std::string>>& data);
		// std::string create_json_response(bool success, const std::string& message = "", const nlohmann::json& data = nlohmann::json::object(), int error_code = 0);  // Not used
	};
}