#include "utils.h"
// Windows includes - careful order to avoid conflicts
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <psapi.h> 
#include <rpc.h>

#include <algorithm>
#include <regex>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <filesystem>


// OpenSSL includes
#include <openssl/md5.h>
#include <openssl/evp.h>

// JSON include
#include <nlohmann/json.hpp>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

// using json = nlohmann::json; // Removed to avoid conflicts

namespace Utils
{
	namespace String
	{
		std::string trim(const std::string& str)
		{
			auto start = str.find_first_not_of(" \t\n\r");
			auto end = str.find_last_not_of(" \t\n\r");
			return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
		}

		std::string to_lower(const std::string& str)
		{
			std::string result = str;
			std::transform(result.begin(), result.end(), result.begin(), ::tolower);
			return result;
		}

		std::string to_upper(const std::string& str)
		{
			std::string result = str;
			std::transform(result.begin(), result.end(), result.begin(), ::toupper);
			return result;
		}

		std::vector<std::string> split(const std::string& str, char delimiter)
		{
			std::vector<std::string> tokens;
			std::stringstream ss(str);
			std::string token;
			while (std::getline(ss, token, delimiter))
			{
				if (!token.empty())
					tokens.push_back(token);
			}
			return tokens;
		}

		bool starts_with(const std::string& str, const std::string& prefix)
		{
			return str.compare(0, prefix.size(), prefix) == 0;
		}

		bool ends_with(const std::string& str, const std::string& suffix)
		{
			if (suffix.size() > str.size())
				return false;
			return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
		}

		std::string replace(const std::string& str, const std::string& from, const std::string& to)
		{
			std::string result = str;
			size_t pos = 0;
			while ((pos = result.find(from, pos)) != std::string::npos)
			{
				result.replace(pos, from.length(), to);
				pos += to.length();
			}
			return result;
		}

		bool is_empty(const std::string& str)
		{
			return str.empty() || std::all_of(str.begin(), str.end(), ::isspace);
		}

		std::string escape_SQL(const std::string& input)
		{
			std::string escaped = input;
			size_t pos = 0;
			while ((pos = escaped.find("'", pos)) != std::string::npos)
			{
				escaped.insert(pos, "'");
				pos += 2; // Skip the newly added quote
			}
			return escaped;
		}
	}

	namespace DateTime
	{
		std::string get_current_date_time()
		{
			auto now = std::chrono::system_clock::now();
			std::time_t now_c = std::chrono::system_clock::to_time_t(now);
			char buffer[100];
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));
			return buffer;
		}

		std::string get_current_date()
		{
			auto now = std::chrono::system_clock::now();
			std::time_t now_c = std::chrono::system_clock::to_time_t(now);
			char buffer[11];
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", std::localtime(&now_c));
			return buffer;
		}

		std::string get_current_time()
		{
			auto now = std::chrono::system_clock::now();
			std::time_t now_c = std::chrono::system_clock::to_time_t(now);
			char buffer[9];
			std::strftime(buffer, sizeof(buffer), "%H:%M:%S", std::localtime(&now_c));
			return buffer;
		}

		std::string format_date_time(const std::chrono::system_clock::time_point& tp)
		{
			auto time_t_tp = std::chrono::system_clock::to_time_t(tp);
			char buffer[100];
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t_tp));
			return buffer;
		}

		bool is_valid_date(const std::string& date_str)
		{
			std::regex date_regex(R"(\d{4}-\d{2}-\d{2})");
			return std::regex_match(date_str, date_regex);
		}

		bool is_date_in_future(const std::string& date_str)
		{
			if (!is_valid_date(date_str))
				return false;
			std::tm tm = {};
			std::istringstream ss(date_str);
			ss >> std::get_time(&tm, "%Y-%m-%d");
			auto date_tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
			return date_tp > std::chrono::system_clock::now();
		}

		int days_between(const std::string& date1, const std::string& date2)
		{
			if (!is_valid_date(date1) || !is_valid_date(date2))
				return 0;
			std::tm tm1 = {}, tm2 = {};
			std::istringstream ss1(date1);
			std::istringstream ss2(date2);
			ss1 >> std::get_time(&tm1, "%Y-%m-%d");
			ss2 >> std::get_time(&tm2, "%Y-%m-%d");
			auto tp1 = std::chrono::system_clock::from_time_t(std::mktime(&tm1));
			auto tp2 = std::chrono::system_clock::from_time_t(std::mktime(&tm2));
			return std::chrono::duration_cast<std::chrono::hours>(tp2 - tp1).count() / 24;
		}

		std::string add_days_to_date(const std::string& date_str, int days)
		{
			if (!is_valid_date(date_str))
				return "";
			std::tm tm = {};
			std::istringstream ss(date_str);
			ss >> std::get_time(&tm, "%Y-%m-%d");
			auto date_tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
			date_tp += std::chrono::hours(days * 24);
			
			// Returnează doar data, nu timestamp-ul complet
			auto time_t_tp = std::chrono::system_clock::to_time_t(date_tp);
			char buffer[11];
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", std::localtime(&time_t_tp));
			return buffer;
		}
	}

	namespace Validation
	{
		bool is_valid_username(const std::string& username)
		{
			return !username.empty() && username.length() <= 50 && std::all_of(username.begin(), username.end(), [](char c) {
				return std::isalnum(c) || c == '_' || c == '-';
			});
		}
		bool is_valid_password(const std::string& password)
		{
			return password.length() >= 6 && password.length() <= 100;
		}
		bool is_valid_email(const std::string& email)
		{
			std::regex email_regex(R"((\w+)(\.\w+)*@(\w+)(\.\w+)+)");
			return std::regex_match(email, email_regex);
		}
		bool is_valid_phone_number(const std::string& phone)
		{
			std::regex phone_regex(R"(\+?[0-9\s\-()]{7,15})");
			return std::regex_match(phone, phone_regex);
		}
		bool is_valid_cnp(const std::string& cnp)
		{
			std::regex cnp_regex(R"(\d{13})");
			return std::regex_match(cnp, cnp_regex);
		}
		bool is_positive_number(double value)
		{
			return value > 0;
		}
		bool is_valid_price_range(double min_price, double max_price)
		{
			return min_price >= 0 && max_price >= min_price;
		}
		bool is_valid_person_count(int count)
		{
			return count > 0 && count <= 100; // Assuming a maximum of 100 persons
		}
	}

	namespace Conversion
	{
		std::string double_to_string(double value, int precision)
		{
			std::ostringstream oss;
			oss << std::fixed << std::setprecision(precision) << value;
			return oss.str();
		}
		double string_to_double(const std::string& str)
		{
			try
			{
				return std::stod(str);
			}
			catch (const std::invalid_argument&)
			{
				return 0.0; // or throw an exception
			}
		}
		int string_to_int(const std::string& str)
		{
			try
			{
				return std::stoi(str);
			}
			catch (const std::invalid_argument&)
			{
				return 0; // or throw an exception
			}
		}
		std::string int_to_string(int value)
		{
			return std::to_string(value);
		}
		std::string bool_to_string(bool value)
		{
			return value ? "true" : "false";
		}
		bool string_to_bool(const std::string& str)
		{
			return str == "true" || str == "1";
		}
	}

	namespace Crypto
	{
		std::string hash_password(const std::string& password, const std::string& salt)
		{
			std::string salted_password = password + salt;
			
			EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
			const EVP_MD* md = EVP_md5();
			unsigned char md_value[EVP_MAX_MD_SIZE];
			unsigned int md_len;

			EVP_DigestInit_ex(mdctx, md, NULL);
			EVP_DigestUpdate(mdctx, salted_password.c_str(), salted_password.length());
			EVP_DigestFinal_ex(mdctx, md_value, &md_len);
			EVP_MD_CTX_free(mdctx);

			std::ostringstream oss;
			for (unsigned int i = 0; i < md_len; ++i)
				oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(md_value[i]);
			return oss.str();
		}

		std::string generate_salt()
		{
			return Utils::Random::generate_random_string(16, true, false);
		}

		bool verify_password(const std::string& password, const std::string& hashed_password, const std::string& salt)
		{
			return hash_password(password, salt) == hashed_password;
		}

		std::string generate_session_token()
		{
			return Utils::Random::generate_random_string(32, true, true);
		}

		std::string md5_hash(const std::string& input)
		{
			EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
			const EVP_MD* md = EVP_md5();
			unsigned char md_value[EVP_MAX_MD_SIZE];
			unsigned int md_len;

			EVP_DigestInit_ex(mdctx, md, NULL);
			EVP_DigestUpdate(mdctx, input.c_str(), input.length());
			EVP_DigestFinal_ex(mdctx, md_value, &md_len);
			EVP_MD_CTX_free(mdctx);

			std::ostringstream oss;
			for (unsigned int i = 0; i < md_len; ++i)
				oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(md_value[i]);
			return oss.str();
		}	
	}

	namespace File
	{
		bool file_exists(const std::string& filepath)
		{
			return std::filesystem::exists(filepath);
		}

		std::string read_file(const std::string& filepath)
		{
			if (!file_exists(filepath))
				return "";
			std::ifstream file(filepath);
			if (!file.is_open())
				return "";
			std::stringstream buffer;
			buffer << file.rdbuf();
			file.close(); // Explicit close to avoid warning
			return buffer.str();
		}

		bool write_file(const std::string& filepath, const std::string& content)
		{
			std::ofstream file(filepath);
			if (!file.is_open())
				return false;
			file << content;
			file.close(); // Explicit close
			return file.good(); // Check if write was successful
		}

		bool create_directory(const std::string& directory_path)
		{
			return std::filesystem::create_directories(directory_path);
		}

		std::vector<std::string> list_files(const std::string& directory_path, const std::string& extension)
		{
			std::vector<std::string> files;
			if (!std::filesystem::exists(directory_path) || !std::filesystem::is_directory(directory_path))
				return files;
			for (const auto& entry : std::filesystem::directory_iterator(directory_path))
			{
				if (entry.is_regular_file())
				{
					if (extension.empty() || entry.path().extension() == extension)
						files.push_back(entry.path().filename().string());
				}
			}
			return files;
		}

		std::string get_file_extension(const std::string& filepath)
		{
			auto pos = filepath.find_last_of('.');
			if (pos == std::string::npos)
				return "";
			return filepath.substr(pos + 1);
		}

		std::string get_file_name(const std::string& filepath)
		{
			auto pos = filepath.find_last_of("/\\");
			if (pos == std::string::npos)
				return filepath;
			return filepath.substr(pos + 1);
		}
	}

	namespace Network
	{
		std::string get_local_ip()
		{
			WSADATA wsaData;
			if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
				return "127.0.0.1"; // Failed to initialize
			}

			char hostname[256];
			if (gethostname(hostname, sizeof(hostname)) != 0) {
				WSACleanup();
				return "127.0.0.1";
			}

			struct addrinfo hints = {}, * result;
			hints.ai_family = AF_INET;

			if (getaddrinfo(hostname, NULL, &hints, &result) == 0) {
				char ip_str[INET_ADDRSTRLEN];
				struct sockaddr_in* sockaddr_ipv4 = (struct sockaddr_in*)result->ai_addr;
				inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, ip_str, INET_ADDRSTRLEN);
				freeaddrinfo(result);
				WSACleanup();
				return std::string(ip_str);
			}

			WSACleanup();
			return "127.0.0.1";
		}

		bool is_valid_ip(const std::string& ip)
		{
			struct sockaddr_in sa;
			return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) != 0;
		}

		bool is_valid_port(int port)
		{
			return port > 0 && port <= 65535;
		}

		std::string get_socket_error_message(int error_code)
		{
			switch (error_code)
			{
				case WSAEACCES: return "Permission denied";
				case WSAEADDRINUSE: return "Address already in use";
				case WSAENOTSOCK: return "Socket operation on non-socket";
				case WSAETIMEDOUT: return "Connection timed out";
				default: return "Unknown socket error";
			}
		}

		std::string format_client_address(const std::string& ip, int port)
		{
			return ip + ":" + std::to_string(port);
		}
	}

	namespace Logger
	{
		static Level current_log_level = Level::DEBUG;

		void log(Level level, const std::string& message)
		{
			if (level < current_log_level)
				return;
			std::string level_str;
			switch (level)
			{
				case Level::DEBUG: level_str = "DEBUG"; break;
				case Level::INFO: level_str = "INFO"; break;
				case Level::WARNING: level_str = "WARNING"; break;
				case Level::ERR: level_str = "ERROR"; break;
				case Level::CRITICAL: level_str = "CRITICAL"; break;
			}
			std::cout << "[" << DateTime::get_current_date_time() << "] [" << level_str << "] " << message << std::endl;
		}

		void debug(const std::string& message)
		{
			log(Level::DEBUG, message);
		}

		void info(const std::string& message)
		{
			log(Level::INFO, message);
		}

		void warning(const std::string& message)
		{
			log(Level::WARNING, message);
		}

		void error(const std::string& message)
		{
			log(Level::ERR, message);
		}

		void critical(const std::string& message)
		{
			log(Level::CRITICAL, message);
		}

		void set_log_level(Level level)
		{
			current_log_level = level;
		}

		void log_to_file(const std::string& filename, const std::string& message)
		{
			std::ofstream file(filename, std::ios::app);
			if (file.is_open())
			{
				file << "[" << DateTime::get_current_date_time() << "] " << message << std::endl;
				file.close();
			}
			else
			{
				std::cerr << "Failed to open log file: " << filename << std::endl;
			}
		}

		std::string level_to_string(Level level)
		{
			switch (level)
			{
				case Level::DEBUG: return "DEBUG";
				case Level::INFO: return "INFO";
				case Level::WARNING: return "WARNING";
				case Level::ERR: return "ERROR";
				case Level::CRITICAL: return "CRITICAL";
				default: return "UNKNOWN";
			}
		}
	}

	namespace JSON
	{
		bool is_valid_json(const std::string& json_str)
		{
			try
			{
				nlohmann::json::parse(json_str);
				return true;
			}
			catch (const nlohmann::json::parse_error&)
			{
				return false;
			}
		}

		std::string escape_json(const std::string& input)
		{
			return nlohmann::json(input).dump();
		}

		std::string create_error_response(const std::string& error_message, int error_code)
    	{
			nlohmann::json response;
			response["success"] = false;
			response["message"] = error_message;
			if (error_code != -1)
				response["error_code"] = error_code;
			return response.dump();
    	}

		std::string create_success_response(const std::string& data, const std::string& message)
   		{
			nlohmann::json response;
			response["success"] = true;
			response["message"] = message.empty() ? "Success" : message;
			
			if (!data.empty())
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
			else
			{
				response["data"] = nlohmann::json::object();
			}
			
			return response.dump();
    	}

		std::string format_json(const std::string& json_str)
		{
			auto json_obj = nlohmann::json::parse(json_str);
			return json_obj.dump(4); // Pretty print with 4 spaces
		}
	}

	namespace Random
	{
		int generate_int(int min, int max)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(min, max);
			return dis(gen);
		}

		double generate_double(double min, double max)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dis(min, max);
			return dis(gen);
		}

		std::string generate_random_string(size_t length, bool include_numbers, bool include_symbols)
		{
			std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
			if (include_numbers) chars += "0123456789";
			if (include_symbols) chars += "!@#$%^&*()_+-=";
			
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<int> dis(0, static_cast<int>(chars.size()) - 1);
			
			std::string result;
			result.reserve(length);
			for (size_t i = 0; i < length; ++i) {
				result += chars[static_cast<size_t>(dis(gen))];
			}
			return result;
		}

		std::string generate_uuid()
		{
			UUID uuid;
			RPC_STATUS status = UuidCreate(&uuid);
			if (status != RPC_S_OK) {
				return "00000000-0000-0000-0000-000000000000"; // Return null UUID on error
			}
			
			RPC_CSTR str;
			status = UuidToStringA(&uuid, &str);
			if (status != RPC_S_OK) {
				return "00000000-0000-0000-0000-000000000000";
			}
			
			std::string uuid_str(reinterpret_cast<const char*>(str));
			RpcStringFreeA(&str);
			return uuid_str;
		}
	}

	namespace Memory
	{
		size_t get_memory_usage_MB()
		{
			PROCESS_MEMORY_COUNTERS pmc;
			if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
			{
				return pmc.WorkingSetSize / (1024 * 1024);
			}
			return 0;
		}

		size_t get_available_memory_MB()
		{
			MEMORYSTATUSEX statex;
			statex.dwLength = sizeof(statex);
			if (GlobalMemoryStatusEx(&statex))
			{
				return statex.ullAvailPhys / (1024 * 1024); // Returns available memory in MB
			}
			return 0;
		}

		void log_memory_usage(const std::string& message)
		{
			size_t memory_usage = get_memory_usage_MB();
			Logger::info(message + " - Memory Usage: " + std::to_string(memory_usage) + " MB");
		}
	}

	namespace Performance
	{
		Timer::Timer(const std::string& name)
			: operation_name(name), start_time(std::chrono::high_resolution_clock::now())
		{
			Logger::debug("Timer started for: " + name);
		}

		Timer::~Timer()
		{
			auto end_time = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
			Logger::info("Timer ended for: " + operation_name + " - Duration: " + std::to_string(static_cast<double>(duration.count())) + " ms");
		}

		void Timer::reset()
		{
			start_time = std::chrono::high_resolution_clock::now();
			Logger::debug("Timer reset for: " + operation_name);
		}

		double Timer::elapsed_milliseconds() const
		{
			auto end_time = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
			return static_cast<double>(duration.count());
		}

		void start_operation(const std::string& operation_name)
		{
			Logger::debug("Starting operation: " + operation_name);
			Timer timer(operation_name);
			timer.reset();
		}

		void end_operation(const std::string& operation_name)
		{
			Logger::debug("Ending operation: " + operation_name);
			// Timer will automatically log the duration in its destructor
		}

		void log_performance(const std::string& operation, double duration_ms)
		{
			Logger::info("Performance - Operation: " + operation + " - Duration: " + std::to_string(duration_ms) + " ms");
		}
	}
}
