#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>

// Qt SQL headers
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QtCore/QString>
#include <QtCore/QVariant>

// Utils header
#include "utils/utils.h"

// Data structures - included from separate header files
#include "models/All_Data_Structures.h"

namespace Database
{
	enum class Result_Type
	{
		SUCCESS,
		ERROR_CONNECTION,
		ERROR_EXECUTION,
		DB_ERROR_NO_DATA,
		ERROR_CONSTRAINT,
		DB_ERROR_TIMEOUT
	};

	struct Query_Result
	{
		Result_Type type;
		std::string message;
		std::vector<std::map<std::string, std::string>> data;
		int affected_rows = 0;
		
		Query_Result(Result_Type t = Result_Type::SUCCESS, const std::string& msg = "")
			: type(t), message(msg), affected_rows(0) 
		{
		}

		bool is_success() const 
		{ 
			return type == Result_Type::SUCCESS; 
		}
		
		bool has_data() const 
		{ 
			return !data.empty(); 
		}
	};

	class Database_Manager
	{
	private:
		QSqlDatabase db;
		QString connection_name;

		std::string server;
		std::string database;
		std::string username;
		std::string password;
		std::string connection_string;

		bool is_connected;
		bool is_demo_mode; // When true, returns mock data instead of real DB operations
		std::mutex db_mutex;

		static constexpr int MAX_RETRIES_ATTEMPTS = 3;
		static constexpr int RETRY_DELAY_MS = 1000;

	public:
		Database_Manager();
		explicit Database_Manager(const std::string& server, const std::string& database, 
			const std::string& username, const std::string& password);
		~Database_Manager();

		// Connection methods
		bool connect();
		bool connect(const std::string& server, const std::string& database,
			const std::string& username, const std::string& password);
		bool disconnect();
		bool is_connection_alive() const;
		bool database_exists() const;
		bool reconnect();

		// Configuration
		void set_configuration_params(const std::string& server, const std::string& database,
			const std::string& username, const std::string& password);
		std::string get_connection_string() const;

		// Core query methods
		Query_Result execute_query(const std::string& query);
		Query_Result execute_select(const std::string& query);
		Query_Result execute_insert(const std::string& query);
		Query_Result execute_update(const std::string& query);
		Query_Result execute_delete(const std::string& query);

		// Advanced features
		Query_Result execute_prepared(const std::string& query, const std::vector<std::pair<std::string, std::string>>& params);
		
		// Transaction support
		bool begin_transaction();
		bool commit_transaction();
		bool rollback_transaction();
		Query_Result execute_transaction(const std::vector<std::string>& queries);

		// Stored procedures
		Query_Result execute_stored_procedure(const std::string& procedure_name, const std::vector<std::string>& params);
		
		// Schema operations
		bool table_exists(const std::string& table_name);
		std::vector<std::string> get_table_columns(const std::string& table_name);
		bool create_tables_if_not_exists();

		// User management
		Query_Result authenticate_user(const std::string& username, const std::string& password);
		Query_Result register_user(const User_Data& user_data);
		Query_Result get_user_by_id(int user_id);
		Query_Result get_user_by_username(const std::string& username);
		Query_Result update_user(const User_Data& user);
		Query_Result delete_user(int user_id);
		Query_Result change_password(int user_id, const std::string& old_password,
			const std::string& new_password);

		// Destination management
		Query_Result get_all_destinations();
		Query_Result get_destination_by_id(int destination_id);
		Query_Result add_destination(const Destination_Data& destination);
		Query_Result update_destination(const Destination_Data& destination);
		Query_Result delete_destination(int destination_id);

		// Transport types management
		Query_Result get_all_transport_types();
		Query_Result get_transport_type_by_id(int transport_type_id);
		Query_Result add_transport_type(const Transport_Type_Data& transport_type);
		Query_Result update_transport_type(const Transport_Type_Data& transport_type);
		Query_Result delete_transport_type(int transport_type_id);

		// Accommodation types management
		Query_Result get_all_accommodation_types();
		Query_Result get_accommodation_type_by_id(int accommodation_type_id);
		Query_Result add_accommodation_type(const Accommodation_Type_Data& accommodation_type);
		Query_Result update_accommodation_type(const Accommodation_Type_Data& accommodation_type);
		Query_Result delete_accommodation_type(int accommodation_type_id);

		// Accommodation management
		Query_Result get_accommodations_by_destination(int destination_id);
		Query_Result get_accommodation_by_id(int accommodation_id);
		Query_Result add_accommodation(const Accommodation_Data& accommodation);
		Query_Result update_accommodation(const Accommodation_Data& accommodation);
		Query_Result delete_accommodation(int accommodation_id);

		// Offer management
		Query_Result get_all_offers();
		Query_Result get_available_offers();
		Query_Result get_offer_by_id(int offer_id);
		Query_Result search_offers(const std::string& destination = "", 
			double min_price = 0, double max_price = 0,
			const std::string& start_date = "", 
			const std::string& end_date = "");
		Query_Result add_offer(const Offer_Data& offer);
		Query_Result update_offer(const Offer_Data& offer);
		Query_Result delete_offer(int offer_id);

		// Reservation management
		Query_Result book_offer(int user_id, int offer_id, int person_count = 1);
		Query_Result get_user_reservations(int user_id);
		Query_Result get_offer_reservations(int offer_id);
		Query_Result get_reservation_by_id(int reservation_id);
		Query_Result cancel_reservation(int reservation_id);
		Query_Result update_reservation_status(int reservation_id, const std::string& status);

		// Reservation persons
		Query_Result add_reservation_person(const Reservation_Person_Data& person_data);
		Query_Result get_reservation_persons(int reservation_id);
		Query_Result update_reservation_person(const Reservation_Person_Data& person_data);
		Query_Result delete_reservation_person(int person_id);

		// Statistics
		Query_Result get_popular_destinations(int limit = 10);
		Query_Result get_revenue_report(const std::string& start_date, const std::string& end_date);
		Query_Result get_user_statistics();
		Query_Result get_booking_statistics();

		// Utilities
		std::string escape_string(const std::string& input);
		std::string format_date_for_sql(const std::string& date);
		bool validate_connection_params();
		std::string get_last_error();
		void log_error(const std::string& operation, const std::string& error);
		
		// Demo mode utilities
		void enable_demo_mode();
		bool is_running_in_demo_mode() const;
		Query_Result create_mock_response(const std::string& operation);

		// Static utilities
		static std::string hash_password(const std::string& password, const std::string& salt);
		static std::string generate_salt();
		static bool validate_email(const std::string& email);
		static bool validate_cnp(const std::string& cnp);

	private:
		// Private helpers
		bool initialize_handles();
		void cleanup_handles();
		std::string build_connection_string() const;
		Query_Result process_select_result(QSqlQuery& query);
		Query_Result process_execution_result(QSqlQuery& query);
		bool handle_sql_error(const QSqlError& error);
		std::string get_sql_error(const QSqlError& error);
		bool retry_operation(std::function<bool()> operation, int max_attempts = MAX_RETRIES_ATTEMPTS);
		
		// Table creation SQL
		std::string get_create_users_table_sql();
		std::string get_create_destinations_table_sql();
		std::string get_create_transport_types_table_sql();
		std::string get_create_accommodation_types_table_sql();
		std::string get_create_accommodations_table_sql();
		std::string get_create_offers_table_sql();
		std::string get_create_reservations_table_sql();
		std::string get_create_reservation_persons_table_sql();
		std::string get_create_indexes_sql();
	};
}