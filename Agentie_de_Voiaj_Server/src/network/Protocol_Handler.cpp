#include "network/Protocol_Handler.h"
#include "network/Client_Handler.h"
#include <vector>
#include <map>


// ============================================================================
// Protocol_Handler Implementation
// ============================================================================

SocketNetwork::Protocol_Handler::Protocol_Handler(std::shared_ptr<Database::Database_Manager> db_manager)
    : db_manager(db_manager)
{
}

SocketNetwork::Parsed_Message SocketNetwork::Protocol_Handler::parse_message(const std::string& json_message)
{
    SocketNetwork::Parsed_Message parsed;
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
            std::string command = msg_json.contains("type") ? 
                msg_json["type"].get<std::string>() : 
                msg_json["command"].get<std::string>();
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

SocketNetwork::Message_Type SocketNetwork::Protocol_Handler::get_message_type(const nlohmann::json& json_obj)
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

std::string SocketNetwork::Protocol_Handler::message_type_to_string(SocketNetwork::Message_Type type)
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

SocketNetwork::Response SocketNetwork::Protocol_Handler::process_message(const SocketNetwork::Parsed_Message& parsed_message, SocketNetwork::Client_Handler* client_handler)
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

std::string SocketNetwork::Protocol_Handler::create_response(bool success, const std::string& message,
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

SocketNetwork::Response SocketNetwork::Protocol_Handler::handle_authentication(const Parsed_Message& message, Client_Handler* client)
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
        
        Utils::Logger::info("Authentication attempt for user: " + username + " from IP: " + client->get_client_info().ip_address);
        
        auto result = db_manager->authenticate_user(username, password);
        
        if (result.is_success() && result.has_data())
        {
            int user_id = Utils::Conversion::string_to_int(result.data[0]["ID"]);
            client->set_authenticated(user_id, username);
            
            Utils::Logger::info("Authentication SUCCESS: User '" + username + "' (ID:" + std::to_string(user_id) + ") logged in from " + client->get_client_info().ip_address);
            
            nlohmann::json user_data;
            for (const auto& pair : result.data[0])
            {
                user_data[pair.first] = pair.second;
            }
            
            return Response(true, Config::SuccessMessages::LOGIN_SUCCESS, user_data.dump());
        }
        else 
        {
            Utils::Logger::warning("Authentication FAILED: Invalid credentials for user '" + username + "' from " + client->get_client_info().ip_address);
            return Response(false, Config::ErrorMessages::AUTHENTICATION_FAILED);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

SocketNetwork::Response SocketNetwork::Protocol_Handler::handle_registration(const SocketNetwork::Parsed_Message& message, SocketNetwork::Client_Handler* client)
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
        
        Utils::Logger::info("Registration attempt for user: " + user_data.username + " (" + user_data.email + ") from IP: " + client->get_client_info().ip_address);
        
        if (message.json_data.contains("phone_number"))
        {
            user_data.phone_number = message.json_data["phone_number"].get<std::string>();
        }
        
        auto result = db_manager->register_user(user_data);
        
        if (result.is_success())
        {
            Utils::Logger::info("Registration SUCCESS: New user '" + user_data.username + "' created successfully from " + client->get_client_info().ip_address);
            return Response(true, Config::SuccessMessages::USER_CREATED);
        }
        else 
        {
            Utils::Logger::warning("Registration FAILED: " + result.message + " for user '" + user_data.username + "' from " + client->get_client_info().ip_address);
            return Response(false, result.message);
        }
    }
    catch (const std::exception& e)
    {
        return Response(false, Config::ErrorMessages::SERVER_ERROR + ": " + std::string(e.what()));
    }
}

SocketNetwork::Response SocketNetwork::Protocol_Handler::handle_get_destinations(const SocketNetwork::Parsed_Message& message, SocketNetwork::Client_Handler* client)
{
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    try 
    {
        // Check if we're in demo mode and use mock data
        auto result = db_manager->is_running_in_demo_mode() ? 
            db_manager->create_mock_response("get_destinations") : 
            db_manager->get_all_destinations();
        
        if (result.is_success())
        {
            std::string destinations_json = vector_to_json(result.data);
            std::string message = db_manager->is_running_in_demo_mode() ? 
                "Demo destinations retrieved successfully" : 
                Config::SuccessMessages::DATA_RETRIEVED;
            return Response(true, message, destinations_json);
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

SocketNetwork::Response SocketNetwork::Protocol_Handler::handle_get_offers(const SocketNetwork::Parsed_Message& message, SocketNetwork::Client_Handler* client)
{
    if (!db_manager)
    {
        return Response(false, Config::ErrorMessages::DB_CONNECTION_FAILED);
    }
    
    try 
    {
        // Check if we're in demo mode and use mock data
        auto result = db_manager->is_running_in_demo_mode() ? 
            db_manager->create_mock_response("get_offers") : 
            db_manager->get_available_offers();
        
        if (result.is_success())
        {
            std::string offers_json = vector_to_json(result.data);
            std::string message = db_manager->is_running_in_demo_mode() ? 
                "Demo offers retrieved successfully" : 
                Config::SuccessMessages::DATA_RETRIEVED;
            return Response(true, message, offers_json);
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

SocketNetwork::Response SocketNetwork::Protocol_Handler::handle_search_offers(const SocketNetwork::Parsed_Message& message, SocketNetwork::Client_Handler* client)
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

SocketNetwork::Response SocketNetwork::Protocol_Handler::handle_book_offer(const SocketNetwork::Parsed_Message& message, SocketNetwork::Client_Handler* client)
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

SocketNetwork::Response SocketNetwork::Protocol_Handler::handle_get_user_reservations(const SocketNetwork::Parsed_Message& message, SocketNetwork::Client_Handler* client)
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

SocketNetwork::Response SocketNetwork::Protocol_Handler::handle_cancel_reservation(const SocketNetwork::Parsed_Message& message, SocketNetwork::Client_Handler* client)
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

SocketNetwork::Response SocketNetwork::Protocol_Handler::handle_get_user_info(const SocketNetwork::Parsed_Message& message, SocketNetwork::Client_Handler* client)
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

SocketNetwork::Response SocketNetwork::Protocol_Handler::handle_update_user_info(const SocketNetwork::Parsed_Message& message, SocketNetwork::Client_Handler* client)
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

SocketNetwork::Response SocketNetwork::Protocol_Handler::handle_keepalive(const SocketNetwork::Parsed_Message& /*message*/, SocketNetwork::Client_Handler* /*client*/)
{
    return Response(true, "PONG");
}

bool SocketNetwork::Protocol_Handler::is_user_admin(int /*user_id*/)
{
    // Not implemented in college project scope - all users are regular clients
    return false;
}

std::string SocketNetwork::Protocol_Handler::vector_to_json(const std::vector<std::map<std::string, std::string>>& data)
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