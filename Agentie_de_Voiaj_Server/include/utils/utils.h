#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <exception>
#include <nlohmann/json.hpp>

namespace Utils
{
	// String Utilities
	namespace String
	{
		std::string trim(const std::string& str);
		std::string to_lower(const std::string& str);
		std::string to_upper(const std::string& str);
		std::vector<std::string> split(const std::string& str, char delimiter);
		bool starts_with(const std::string& str, const std::string& prefix);
		bool ends_with(const std::string& str, const std::string& suffix);
		std::string replace(const std::string& str, const std::string& from, const std::string& to);
		bool is_empty(const std::string& str);
		std::string escape_SQL(const std::string& input);
	}

	// Date and Time Utilities
	namespace DateTime
	{
		std::string get_current_date_time();
		std::string get_current_date();
		std::string get_current_time();
		std::string format_date_time(const std::chrono::system_clock::time_point& tp);
		bool is_valid_date(const std::string& date_str); // format: YYYY-MM-DD
		bool is_date_in_future(const std::string& date_str); // format: YYYY-MM-DD
		int days_between(const std::string& date1, const std::string& date2); // format: YYYY-MM-DD
		std::string add_days_to_date(const std::string& date_str, int days); // format: YYYY-MM-DD
	}

	// Validation Utilities
	namespace Validation
	{
		bool is_valid_username(const std::string& username);
		bool is_valid_password(const std::string& password);
		bool is_valid_email(const std::string& email);
		bool is_valid_phone_number(const std::string& phone);
		bool is_valid_cnp(const std::string& cnp);
		bool is_positive_number(double value);
		bool is_valid_price_range(double min_price, double max_price);
		bool is_valid_person_count(int count);
	}

	// Conversion Utilities
	namespace Conversion
	{
		std::string double_to_string(double value, int precision = 2);
		double string_to_double(const std::string& str);
		int string_to_int(const std::string& str);
		std::string int_to_string(int value);
		std::string bool_to_string(bool value);
		bool string_to_bool(const std::string& str);
	}

	// Cryptography Utilities
	namespace Crypto
	{
		std::string hash_password(const std::string& password, const std::string & salt = "");
		std::string generate_salt();
		bool verify_password(const std::string& password, const std::string& hashed_password, const std::string& salt = "");
		std::string generate_session_token();
		std::string md5_hash(const std::string& input);
	}

	// File Utilities
	namespace File
	{
		bool file_exists(const std::string& filepath);
		std::string read_file(const std::string& filepath);
		bool write_file(const std::string& filepath, const std::string& content);
		bool create_directory(const std::string& directory_path);
		std::vector<std::string> list_files(const std::string& directory_path, const std::string& extension = "");
		std::string get_file_extension(const std::string& filepath);
		std::string get_file_name(const std::string& filepath);
	}

	// Network Utilities
	namespace Network
	{
		std::string get_local_ip();
		bool is_valid_ip(const std::string& ip);
		bool is_valid_port(int port);
		std::string get_socket_error_message(int error_code);
		std::string format_client_address(const std::string& ip, int port);
	}

	// Logging Utilities
	namespace Logger
	{
		enum class Level
		{
			DEBUG = 0,
			INFO = 1,
			WARNING = 2,
			ERR = 3,
			CRITICAL = 4
		};

		void log(Level level, const std::string& message);
		void debug(const std::string& message);
		void info(const std::string& message);
		void warning(const std::string& message);
		void error(const std::string& message);
		void critical(const std::string& message);
		void log_to_file(const std::string& filename, const std::string& message);
		void set_log_level(Level min_level);
		std::string level_to_string(Level level);
		void initialize_logging();
		void enable_file_logging(bool enabled);
		std::string get_log_filename();
	}

	// JSON Utilities
	namespace JSON
	{
		bool is_valid_json(const std::string& json_str);
		std::string escape_json(const std::string& input);
		std::string create_error_response(const std::string& error_message, int error_code = -1);
		std::string create_success_response(const std::string& data = "", const std::string& message = "");
		std::string format_json(const std::string& json_str);
	}

	// Random Utilities
	namespace Random
	{
		int generate_int(int min, int max);
		double generate_double(double min, double max);
		std::string generate_random_string(size_t length, bool include_numbers = true, bool include_symbols = false);
		std::string generate_uuid();
	}

	// Memory Utilities
	namespace Memory
	{
		size_t get_memory_usage_MB();
		size_t get_available_memory_MB();
		void log_memory_usage(const std::string& context = "");
	}

	// Performance Utilities
	namespace Performance
	{
		class Timer
		{
		private:
			std::chrono::high_resolution_clock::time_point start_time;
			std::string operation_name;

		public:
			Timer(const std::string& name);
			~Timer();
			void reset();
			double elapsed_milliseconds() const;
		};

		void start_operation(const std::string& operation_name);
		void end_operation(const std::string& operation_name);
		void log_performance(const std::string& operation, double duration_ms);
	}

	namespace Exceptions
	{
		class BaseException : public std::exception
		{
		protected:
			std::string message_;
			int error_code_;

		public:
			BaseException(const std::string& message, int code = -1)
				: message_(message), error_code_(code) {
			}

			const char* what() const noexcept override { return message_.c_str(); }
			int error_code() const { return error_code_; }
			const std::string& message() const { return message_; }
		};

		class DatabaseException : public BaseException
		{
		public:
			DatabaseException(const std::string& message, int code = -1)
				: BaseException("[DATABASE] " + message, code) {
			}
		};

		class NetworkException : public BaseException
		{
		public:
			NetworkException(const std::string& message, int code = -1)
				: BaseException("[NETWORK] " + message, code) {
			}
		};

		class ValidationException : public BaseException
		{
		public:
			ValidationException(const std::string& message, int code = -1)
				: BaseException("[VALIDATION] " + message, code) {
			}
		};

		class ConfigurationException : public BaseException
		{
		public:
			ConfigurationException(const std::string& message, int code = -1)
				: BaseException("[CONFIG] " + message, code) {
			}
		};
	}
}
