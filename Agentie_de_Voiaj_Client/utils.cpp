#include "utils.h"
#include "config.h"
#include <QApplication>
#include <QScreen>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QUuid>
#include <QRandomGenerator>
#include <QWidget>

namespace Utils
{
    // String utilities implementation
    namespace String
    {
        bool is_empty(const QString& str)
        {
            return str.trimmed().isEmpty();
        }

        QString trim(const QString& str)
        {
            return str.trimmed();
        }

        QString to_lower(const QString& str)
        {
            return str.toLower();
        }

        QString to_upper(const QString& str)
        {
            return str.toUpper();
        }

        bool contains_only_digits(const QString& str)
        {
            if (str.isEmpty()) return false;
            return str.contains(QRegularExpression("^\\d+$"));
        }

        QString escape_html(const QString& str)
        {
            QString result = str;
            result.replace("&", "&amp;");
            result.replace("<", "&lt;");
            result.replace(">", "&gt;");
            result.replace("\"", "&quot;");
            result.replace("'", "&#39;");
            return result;
        }

        QString truncate(const QString& str, int max_length, const QString& suffix)
        {
            if (str.length() <= max_length)
                return str;
            return str.left(max_length - suffix.length()) + suffix;
        }
    }

    // Validation utilities implementation
    namespace Validation
    {
        bool is_valid_email(const QString& email)
        {
            if (email.length() > Config::Validation::MAX_EMAIL_LENGTH) return false;
            QRegularExpression regex(Config::Validation::EMAIL_PATTERN);
            return regex.match(email).hasMatch();
        }

        bool is_valid_phone(const QString& phone)
        {
            if (phone.length() > Config::Validation::MAX_PHONE_LENGTH) return false;
            QRegularExpression regex(Config::Validation::PHONE_PATTERN);
            return regex.match(phone).hasMatch();
        }

        bool is_valid_username(const QString& username)
        {
            if (username.length() < Config::Validation::MIN_USERNAME_LENGTH ||
                username.length() > Config::Validation::MAX_USERNAME_LENGTH) return false;
            QRegularExpression regex(Config::Validation::USERNAME_PATTERN);
            return regex.match(username).hasMatch();
        }

        bool is_valid_password(const QString& password)
        {
            return password.length() >= Config::Validation::MIN_PASSWORD_LENGTH &&
                   password.length() <= Config::Validation::MAX_PASSWORD_LENGTH;
        }

        bool is_valid_name(const QString& name)
        {
            return !String::is_empty(name) && name.length() <= Config::Validation::MAX_NAME_LENGTH;
        }

        bool is_valid_price(double price)
        {
            return price >= Config::Business::MIN_PRICE && price <= Config::Business::MAX_PRICE;
        }

        bool is_valid_person_count(int count)
        {
            return count > 0 && count <= Config::Business::MAX_PERSONS_PER_RESERVATION;
        }

        QString get_validation_error(const QString& field, const QString& value)
        {
            if (field == "email" && !is_valid_email(value))
                return "Adresa de email nu este validă";
            if (field == "phone" && !is_valid_phone(value))
                return "Numărul de telefon nu este valid";
            if (field == "username" && !is_valid_username(value))
                return QString("Numele de utilizator trebuie să aibă între %1 și %2 caractere")
                       .arg(Config::Validation::MIN_USERNAME_LENGTH)
                       .arg(Config::Validation::MAX_USERNAME_LENGTH);
            if (field == "password" && !is_valid_password(value))
                return QString("Parola trebuie să aibă cel puțin %1 caractere")
                       .arg(Config::Validation::MIN_PASSWORD_LENGTH);
            if (field == "name" && !is_valid_name(value))
                return "Numele este obligatoriu și nu poate depăși " + 
                       QString::number(Config::Validation::MAX_NAME_LENGTH) + " caractere";
            return "Valoare invalidă";
        }
    }

    // JSON utilities implementation
    namespace JSON
    {
        bool is_valid_json(const QString& json_string)
        {
            QJsonParseError error;
            QJsonDocument::fromJson(json_string.toUtf8(), &error);
            return error.error == QJsonParseError::NoError;
        }

        QString to_json_string(const QJsonObject& obj, bool pretty_print)
        {
            QJsonDocument doc(obj);
            return QString::fromUtf8(doc.toJson(pretty_print ? QJsonDocument::Indented : QJsonDocument::Compact));
        }

        QString to_json_string(const QJsonArray& array, bool pretty_print)
        {
            QJsonDocument doc(array);
            return QString::fromUtf8(doc.toJson(pretty_print ? QJsonDocument::Indented : QJsonDocument::Compact));
        }

        QJsonObject parse_object(const QString& json_string, bool* ok)
        {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(json_string.toUtf8(), &error);
            if (ok) *ok = (error.error == QJsonParseError::NoError && doc.isObject());
            return doc.object();
        }

        QJsonArray parse_array(const QString& json_string, bool* ok)
        {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(json_string.toUtf8(), &error);
            if (ok) *ok = (error.error == QJsonParseError::NoError && doc.isArray());
            return doc.array();
        }

        QString create_success_response(const QString& message, const QJsonValue& data)
        {
            QJsonObject response;
            response["success"] = true;
            if (!message.isEmpty())
                response["message"] = message;
            if (!data.isNull() && !data.isUndefined())
                response["data"] = data;
            return to_json_string(response);
        }

        QString create_error_response(const QString& message, int error_code)
        {
            QJsonObject response;
            response["success"] = false;
            response["message"] = message;
            if (error_code != 0)
                response["error_code"] = error_code;
            return to_json_string(response);
        }

        bool is_success_response(const QJsonObject& response)
        {
            return response["success"].toBool(false);
        }

        QString get_response_message(const QJsonObject& response)
        {
            return response["message"].toString();
        }

        QJsonValue get_response_data(const QJsonObject& response)
        {
            return response["data"];
        }
    }

    // DateTime utilities implementation
    namespace DateTime
    {
        QString get_current_date_time()
        {
            return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        }

        QString get_current_date()
        {
            return QDate::currentDate().toString("yyyy-MM-dd");
        }

        QString get_current_time()
        {
            return QTime::currentTime().toString("hh:mm:ss");
        }

        QString format_date_time(const QDateTime& dateTime, const QString& format)
        {
            return dateTime.toString(format);
        }

        QString format_date(const QDate& date, const QString& format)
        {
            return date.toString(format);
        }

        QDateTime parse_date_time(const QString& dateTimeString, const QString& format)
        {
            return QDateTime::fromString(dateTimeString, format);
        }

        QDate parse_date(const QString& dateString, const QString& format)
        {
            return QDate::fromString(dateString, format);
        }

        QString time_ago(const QDateTime& dateTime)
        {
            QDateTime now = QDateTime::currentDateTime();
            qint64 seconds = dateTime.secsTo(now);
            
            if (seconds < 60)
                return "acum câteva secunde";
            else if (seconds < 3600)
                return QString("acum %1 minute").arg(seconds / 60);
            else if (seconds < 86400)
                return QString("acum %1 ore").arg(seconds / 3600);
            else if (seconds < 604800)
                return QString("acum %1 zile").arg(seconds / 86400);
            else
                return dateTime.toString("yyyy-MM-dd");
        }

        bool is_valid_date_format(const QString& dateString, const QString& format)
        {
            QDate date = QDate::fromString(dateString, format);
            return date.isValid();
        }
    }

    // Conversion utilities implementation
    namespace Conversion
    {
        QString int_to_string(int value)
        {
            return QString::number(value);
        }

        QString double_to_string(double value, int precision)
        {
            return QString::number(value, 'f', precision);
        }

        int string_to_int(const QString& str, bool* ok)
        {
            return str.toInt(ok);
        }

        double string_to_double(const QString& str, bool* ok)
        {
            return str.toDouble(ok);
        }

        QString bool_to_string(bool value)
        {
            return value ? "true" : "false";
        }

        bool string_to_bool(const QString& str)
        {
            QString lower = str.toLower();
            return lower == "true" || lower == "1" || lower == "yes" || lower == "on";
        }

        QString bytes_to_human_readable(qint64 bytes)
        {
            const QStringList units = {"B", "KB", "MB", "GB", "TB"};
            double size = bytes;
            int unit = 0;
            
            while (size >= 1024 && unit < units.size() - 1)
            {
                size /= 1024;
                unit++;
            }
            
            return QString("%1 %2").arg(size, 0, 'f', unit == 0 ? 0 : 2).arg(units[unit]);
        }
    }

    // Cryptography utilities implementation
    namespace Crypto
    {
        QString hash_string(const QString& input, const QString& salt)
        {
            QCryptographicHash hash(QCryptographicHash::Sha256);
            hash.addData((input + salt).toUtf8());
            return QString::fromUtf8(hash.result().toHex());
        }

        QString generate_uuid()
        {
            return QUuid::createUuid().toString(QUuid::WithoutBraces);
        }

        QString generate_random_string(int length)
        {
            const QString charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
            QString result;
            result.reserve(length);
            
            for (int i = 0; i < length; ++i)
            {
                int index = QRandomGenerator::global()->bounded(charset.length());
                result.append(charset[index]);
            }
            
            return result;
        }

        bool verify_hash(const QString& input, const QString& hash, const QString& salt)
        {
            return hash_string(input, salt) == hash;
        }
    }

    // UI utilities implementation
    namespace UI
    {
        QString get_style_sheet(const QString& component, const QString& color)
        {
            QString base_color = color.isEmpty() ? Config::UI::PRIMARY_COLOR : color;
            
            if (component == "button")
            {
                return QString("QPushButton { background-color: %1; color: white; padding: 8px 16px; "
                              "border: none; border-radius: 4px; font-weight: bold; } "
                              "QPushButton:hover { background-color: %2; } "
                              "QPushButton:disabled { background-color: #CCCCCC; }")
                       .arg(base_color, base_color + "CC");
            }
            else if (component == "success_button")
            {
                return get_style_sheet("button", Config::UI::SUCCESS_COLOR);
            }
            else if (component == "error_button")
            {
                return get_style_sheet("button", Config::UI::ERROR_COLOR);
            }
            
            return "";
        }

        QString format_currency(double amount, const QString& currency)
        {
            return QString("%1 %2").arg(amount, 0, 'f', 2).arg(currency);
        }

        QString format_duration(int days)
        {
            if (days == 1)
                return "1 zi";
            else if (days < 7)
                return QString("%1 zile").arg(days);
            else if (days == 7)
                return "1 săptămână";
            else if (days < 30)
                return QString("%1 săptămâni").arg(days / 7);
            else if (days < 365)
                return QString("%1 luni").arg(days / 30);
            else
                return QString("%1 ani").arg(days / 365);
        }

        QString get_status_color(const QString& status)
        {
            QString lower_status = status.toLower();
            if (lower_status.contains("activ") || lower_status.contains("confirmat"))
                return Config::UI::SUCCESS_COLOR;
            else if (lower_status.contains("anulat") || lower_status.contains("expirat"))
                return Config::UI::ERROR_COLOR;
            else if (lower_status.contains("asteptare") || lower_status.contains("pending"))
                return Config::UI::WARNING_COLOR;
            else
                return Config::UI::INFO_COLOR;
        }

        QString get_icon_path(const QString& icon_name)
        {
            return QString(":/icons/%1.png").arg(icon_name);
        }

        void center_widget_on_screen(QWidget* widget)
        {
            if (!widget) return;
            
            QScreen* screen = QApplication::primaryScreen();
            if (screen)
            {
                QRect screen_geometry = screen->availableGeometry();
                int x = (screen_geometry.width() - widget->width()) / 2;
                int y = (screen_geometry.height() - widget->height()) / 2;
                widget->move(x, y);
            }
        }

        void apply_theme(QWidget* widget, const QString& theme)
        {
            if (!widget) return;
            
            if (theme == "default")
            {
                widget->setStyleSheet(
                    "QWidget { background-color: #F5F5F5; color: #333333; } "
                    "QTabWidget::pane { border: 1px solid #C0C0C0; } "
                    "QTabBar::tab { background: #E0E0E0; padding: 8px 16px; margin-right: 2px; } "
                    "QTabBar::tab:selected { background: " + Config::UI::PRIMARY_COLOR + "; color: white; } "
                    "QTabBar::tab:hover { background: #F0F0F0; }"
                );
            }
        }
    }

    // Network utilities implementation
    namespace Network
    {
        QString get_local_ip_address()
        {
            foreach (const QHostAddress& address, QNetworkInterface::allAddresses())
            {
                if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost)
                    return address.toString();
            }
            return "127.0.0.1";
        }

        bool is_port_available(int port)
        {
            // This is a simplified check - in reality you'd try to bind to the port
            return port > 0 && port <= 65535;
        }

        QString get_connection_status_string(int status)
        {
            switch (status)
            {
                case 0: return Config::StatusMessages::DISCONNECTED;
                case 1: return Config::StatusMessages::CONNECTING;
                case 2: return Config::StatusMessages::CONNECTED;
                case 3: return "Eroare";
                default: return "Necunoscut";
            }
        }

        QString format_network_error(const QString& error)
        {
            return QString("Eroare de rețea: %1").arg(error);
        }

        bool is_valid_ip_address(const QString& ip)
        {
            QHostAddress address(ip);
            return !address.isNull();
        }

        bool is_valid_port(int port)
        {
            return port > 0 && port <= 65535;
        }
    }

    // File utilities implementation
    namespace File
    {
        QString get_app_data_path()
        {
            return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        }

        QString get_config_file_path(const QString& filename)
        {
            return QDir(get_app_data_path()).filePath(Config::Application::CONFIG_DIRECTORY + filename);
        }

        QString get_log_file_path(const QString& filename)
        {
            return QDir(get_app_data_path()).filePath(Config::Application::LOG_DIRECTORY + filename);
        }

        QString get_cache_file_path(const QString& filename)
        {
            return QDir(get_app_data_path()).filePath(Config::Application::CACHE_DIRECTORY + filename);
        }

        bool ensure_directory_exists(const QString& path)
        {
            return QDir().mkpath(path);
        }

        bool write_text_file(const QString& filepath, const QString& content)
        {
            QFile file(filepath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream stream(&file);
                stream << content;
                return true;
            }
            return false;
        }

        QString read_text_file(const QString& filepath)
        {
            QFile file(filepath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream stream(&file);
                return stream.readAll();
            }
            return QString();
        }

        bool file_exists(const QString& filepath)
        {
            return QFile::exists(filepath);
        }

        qint64 get_file_size(const QString& filepath)
        {
            QFileInfo info(filepath);
            return info.size();
        }

        QString get_file_extension(const QString& filepath)
        {
            QFileInfo info(filepath);
            return info.suffix();
        }
    }

    // Logging utilities implementation
    namespace Logger
    {
        static Level current_level = Level::Info;
        static bool file_logging_enabled = true;
        static bool console_logging_enabled = true;

        QString level_to_string(Level level)
        {
            switch (level)
            {
                case Level::Debug: return "DEBUG";
                case Level::Info: return "INFO";
                case Level::Warning: return "WARNING";
                case Level::Error: return "ERROR";
                case Level::Critical: return "CRITICAL";
                default: return "UNKNOWN";
            }
        }

        void log(Level level, const QString& message, const QString& category)
        {
            if (level < current_level) return;

            QString timestamp = DateTime::get_current_date_time();
            QString log_entry = QString("[%1] [%2] [%3] %4")
                               .arg(timestamp, level_to_string(level), category, message);

            if (console_logging_enabled && Config::Application::DEBUG_MODE)
            {
                qDebug().noquote() << log_entry;
            }

            if (file_logging_enabled)
            {
                QString log_file = File::get_log_file_path("app.log");
                File::ensure_directory_exists(QFileInfo(log_file).path());
                
                QFile file(log_file);
                if (file.open(QIODevice::WriteOnly | QIODevice::Append))
                {
                    QTextStream stream(&file);
                    stream << log_entry << Qt::endl;
                }
            }
        }

        void debug(const QString& message, const QString& category)
        {
            log(Level::Debug, message, category);
        }

        void info(const QString& message, const QString& category)
        {
            log(Level::Info, message, category);
        }

        void warning(const QString& message, const QString& category)
        {
            log(Level::Warning, message, category);
        }

        void error(const QString& message, const QString& category)
        {
            log(Level::Error, message, category);
        }

        void critical(const QString& message, const QString& category)
        {
            log(Level::Critical, message, category);
        }

        void log_network_request(const QString& request, const QString& endpoint)
        {
            if (Config::Application::LOG_NETWORK_REQUESTS)
            {
                debug(QString("Request to %1: %2").arg(endpoint, request), "Network");
            }
        }

        void log_network_response(const QString& response, const QString& endpoint)
        {
            if (Config::Application::LOG_NETWORK_REQUESTS)
            {
                debug(QString("Response from %1: %2").arg(endpoint, response), "Network");
            }
        }

        void log_json_data(const QJsonObject& data, const QString& operation)
        {
            if (Config::Application::LOG_JSON_MESSAGES)
            {
                debug(QString("%1: %2").arg(operation, JSON::to_json_string(data, true)), "JSON");
            }
        }

        void set_log_level(Level level)
        {
            current_level = level;
        }

        void enable_file_logging(bool enable)
        {
            file_logging_enabled = enable;
        }

        void enable_console_logging(bool enable)
        {
            console_logging_enabled = enable;
        }
    }

    // Error handling utilities implementation
    namespace Error
    {
        QString get_error_type_string(Type type)
        {
            switch (type)
            {
                case Type::NetworkError: return "Eroare de rețea";
                case Type::ValidationError: return "Eroare de validare";
                case Type::JSONError: return "Eroare JSON";
                case Type::AuthenticationError: return "Eroare de autentificare";
                case Type::ServerError: return "Eroare de server";
                default: return "Eroare necunoscută";
            }
        }

        QString format_error_message(const Error_Info& error)
        {
            QString formatted = QString("[%1] %2")
                               .arg(get_error_type_string(error.type), error.message);
            
            if (!error.details.isEmpty())
                formatted += QString(" - %1").arg(error.details);
            
            if (error.code != 0)
                formatted += QString(" (Cod: %1)").arg(error.code);
            
            return formatted;
        }

        void log_error(const Error_Info& error)
        {
            Logger::error(format_error_message(error), "Error");
        }

        Error_Info create_network_error(const QString& message, int code)
        {
            return Error_Info(Type::NetworkError, message, code);
        }

        Error_Info create_validation_error(const QString& field, const QString& message)
        {
            return Error_Info(Type::ValidationError, message, 0, field);
        }

        Error_Info create_json_error(const QString& message)
        {
            return Error_Info(Type::JSONError, message);
        }

        Error_Info create_auth_error(const QString& message)
        {
            return Error_Info(Type::AuthenticationError, message);
        }

        Error_Info create_server_error(const QString& message, int code)
        {
            return Error_Info(Type::ServerError, message, code);
        }
    }
}