#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QDebug>

namespace Utils
{
    // String utilities
    namespace String
    {
        bool is_empty(const QString& str);
        QString trim(const QString& str);
        QString to_lower(const QString& str);
        QString to_upper(const QString& str);
        bool contains_only_digits(const QString& str);
        QString escape_html(const QString& str);
        QString truncate(const QString& str, int max_length, const QString& suffix = "...");
    }

    // Validation utilities
    namespace Validation
    {
        bool is_valid_email(const QString& email);
        bool is_valid_phone(const QString& phone);
        bool is_valid_username(const QString& username);
        bool is_valid_password(const QString& password);
        bool is_valid_name(const QString& name);
        bool is_valid_price(double price);
        bool is_valid_person_count(int count);
        QString get_validation_error(const QString& field, const QString& value);
    }

    // JSON utilities
    namespace JSON
    {
        bool is_valid_json(const QString& json_string);
        QString to_json_string(const QJsonObject& obj, bool pretty_print = false);
        QString to_json_string(const QJsonArray& array, bool pretty_print = false);
        QJsonObject parse_object(const QString& json_string, bool* ok = nullptr);
        QJsonArray parse_array(const QString& json_string, bool* ok = nullptr);
        
        // Response helpers
        QString create_success_response(const QString& message = "", const QJsonValue& data = QJsonValue());
        QString create_error_response(const QString& message, int error_code = 0);
        bool is_success_response(const QJsonObject& response);
        QString get_response_message(const QJsonObject& response);
        QJsonValue get_response_data(const QJsonObject& response);
    }

    // DateTime utilities
    namespace DateTime
    {
        QString get_current_date_time();
        QString get_current_date();
        QString get_current_time();
        QString format_date_time(const QDateTime& dateTime, const QString& format = "yyyy-MM-dd hh:mm:ss");
        QString format_date(const QDate& date, const QString& format = "yyyy-MM-dd");
        QDateTime parse_date_time(const QString& dateTimeString, const QString& format = "yyyy-MM-dd hh:mm:ss");
        QDate parse_date(const QString& dateString, const QString& format = "yyyy-MM-dd");
        QString time_ago(const QDateTime& dateTime);
        bool is_valid_date_format(const QString& dateString, const QString& format = "yyyy-MM-dd");
    }

    // Conversion utilities
    namespace Conversion
    {
        QString int_to_string(int value);
        QString double_to_string(double value, int precision = 2);
        int string_to_int(const QString& str, bool* ok = nullptr);
        double string_to_double(const QString& str, bool* ok = nullptr);
        QString bool_to_string(bool value);
        bool string_to_bool(const QString& str);
        QString bytes_to_human_readable(qint64 bytes);
    }

    // Cryptography utilities
    namespace Crypto
    {
        QString hash_string(const QString& input, const QString& salt = "");
        QString generate_uuid();
        QString generate_random_string(int length);
        bool verify_hash(const QString& input, const QString& hash, const QString& salt = "");
    }

    // UI utilities
    namespace UI
    {
        QString get_style_sheet(const QString& component, const QString& color = "");
        QString format_currency(double amount, const QString& currency = "RON");
        QString format_duration(int days);
        QString get_status_color(const QString& status);
        QString get_icon_path(const QString& icon_name);
        void center_widget_on_screen(QWidget* widget);
        void apply_theme(QWidget* widget, const QString& theme = "default");
    }

    // Network utilities
    namespace Network
    {
        QString get_local_ip_address();
        bool is_port_available(int port);
        QString get_connection_status_string(int status);
        QString format_network_error(const QString& error);
        bool is_valid_ip_address(const QString& ip);
        bool is_valid_port(int port);
    }

    // File utilities
    namespace File
    {
        QString get_app_data_path();
        QString get_config_file_path(const QString& filename);
        QString get_log_file_path(const QString& filename);
        QString get_cache_file_path(const QString& filename);
        bool ensure_directory_exists(const QString& path);
        bool write_text_file(const QString& filepath, const QString& content);
        QString read_text_file(const QString& filepath);
        bool file_exists(const QString& filepath);
        qint64 get_file_size(const QString& filepath);
        QString get_file_extension(const QString& filepath);
    }

    // Logging utilities
    namespace Logger
    {
        enum class Level
        {
            Debug,
            Info, 
            Warning,
            Error,
            Critical
        };

        void log(Level level, const QString& message, const QString& category = "General");
        void debug(const QString& message, const QString& category = "General");
        void info(const QString& message, const QString& category = "General");
        void warning(const QString& message, const QString& category = "General");
        void error(const QString& message, const QString& category = "General");
        void critical(const QString& message, const QString& category = "General");
        
        void log_network_request(const QString& request, const QString& endpoint = "");
        void log_network_response(const QString& response, const QString& endpoint = "");
        void log_json_data(const QJsonObject& data, const QString& operation = "");
        
        void set_log_level(Level level);
        void enable_file_logging(bool enable);
        void enable_console_logging(bool enable);
    }

    // Error handling utilities
    namespace Error
    {
        enum class Type
        {
            NetworkError,
            ValidationError,
            JSONError,
            AuthenticationError,
            ServerError,
            UnknownError
        };

        struct Error_Info
        {
            Type type;
            QString message;
            QString details;
            int code;
            QDateTime timestamp;
            
            Error_Info(Type t = Type::UnknownError, const QString& msg = "", int c = 0, const QString& det = "")
                : type(t), message(msg), code(c), details(det), timestamp(QDateTime::currentDateTime()) {}
        };

        QString get_error_type_string(Type type);
        QString format_error_message(const Error_Info& error);
        void log_error(const Error_Info& error);
        Error_Info create_network_error(const QString& message, int code = 0);
        Error_Info create_validation_error(const QString& field, const QString& message);
        Error_Info create_json_error(const QString& message);
        Error_Info create_auth_error(const QString& message);
        Error_Info create_server_error(const QString& message, int code = 0);
    }
}