#pragma once

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QAbstractSocket>
#include "config.h"
#include "utils.h"

struct User_Info
{
    int id = 0;
    QString username;
    QString email;
    QString first_name;
    QString last_name;
    QString phone_number;
    
    bool is_valid() const { return id > 0 && !username.isEmpty(); }
};

struct Destination_Info
{
    int id = 0;
    QString name;
    QString country;
    QString description;
    QString image_path;
};

struct Offer_Info
{
    int id = 0;
    QString name;
    QString destination_name;
    QString accommodation_name;
    QString transport_type;
    double price_per_person = 0.0;
    int duration_days = 0;
    QString departure_date;
    QString return_date;
    int available_seats = 0;
    QString description;
    QString status;
};

struct Reservation_Info
{
    int id = 0;
    int offer_id = 0;
    QString offer_name;
    int number_of_persons = 0;
    double total_price = 0.0;
    QString reservation_date;
    QString status;
    QString notes;
};

class Network_Manager : public QObject
{
    Q_OBJECT

public:
    explicit Network_Manager(QObject* parent = nullptr);
    ~Network_Manager();

    enum class Connection_Status
    {
        Disconnected,
        Connecting,
        Connected,
        Error
    };

    // Connection management
    bool connect_to_server(const QString& host = Config::Network::DEFAULT_SERVER_HOST, 
                          quint16 port = Config::Network::DEFAULT_SERVER_PORT);
    void disconnect_from_server();
    Connection_Status get_connection_status() const;
    QString get_last_error() const;

    // User authentication
    void authenticate_user(const QString& username, const QString& password);
    void register_user(const QString& username, const QString& password, 
                      const QString& email, const QString& first_name, 
                      const QString& last_name, const QString& phone_number);
    void logout_user();
    
    // Data retrieval
    void get_destinations();
    void get_offers();
    void search_offers(const QString& destination = "", double min_price = 0.0, 
                      double max_price = 0.0, const QString& start_date = "", 
                      const QString& end_date = "");
    
    // Booking operations
    void book_offer(int offer_id, int person_count);
    void get_user_reservations();
    void cancel_reservation(int reservation_id);
    
    // User management
    void get_user_info();
    void update_user_info(const QString& email, const QString& first_name,
                         const QString& last_name, const QString& phone_number);

    // Current user info
    const User_Info& get_current_user() const { return current_user; }
    bool is_user_logged_in() const { return current_user.is_valid(); }

signals:
    // Connection signals
    void connected_to_server();
    void disconnected_from_server();
    void connection_error(const QString& error);

    // Authentication signals
    void authentication_successful(const User_Info& user);
    void authentication_failed(const QString& error);
    void registration_successful();
    void registration_failed(const QString& error);
    void user_logged_out();

    // Data signals
    void destinations_received(const QList<Destination_Info>& destinations);
    void offers_received(const QList<Offer_Info>& offers);
    void reservations_received(const QList<Reservation_Info>& reservations);
    void user_info_received(const User_Info& user);

    // Operation signals
    void booking_successful(int reservation_id);
    void booking_failed(const QString& error);
    void cancellation_successful();
    void cancellation_failed(const QString& error);
    void user_info_updated();
    void user_info_update_failed(const QString& error);

    // General signals
    void operation_completed(const QString& operation, bool success, const QString& message);

private slots:
    void on_connected();
    void on_disconnected();
    void on_socket_error(QAbstractSocket::SocketError error);
    void on_ready_read();
    void on_keepalive_timer();

private:
    QTcpSocket* tcp_socket;
    Connection_Status connection_status;
    QString last_error;
    User_Info current_user;
    QTimer* keepalive_timer;
    QByteArray receive_buffer;
    
    // Configuration and utilities
    QString server_host;
    quint16 server_port;
    int connection_attempts;
    QDateTime last_activity;
    
    // Qt TCP helpers
    void setup_socket_connections();
    void process_incoming_data();
    
    // Message handling
    void send_json_message(const QJsonObject& message);
    void process_server_response(const QJsonObject& response);
    
    // Response processors
    void process_authentication_response(const QJsonObject& response);
    void process_registration_response(const QJsonObject& response);
    void process_destinations_response(const QJsonObject& response);
    void process_offers_response(const QJsonObject& response);
    void process_reservations_response(const QJsonObject& response);
    void process_booking_response(const QJsonObject& response);
    void process_user_info_response(const QJsonObject& response);
    
    // Utility functions
    QList<Destination_Info> parse_destinations(const QJsonArray& destinations_array);
    QList<Offer_Info> parse_offers(const QJsonArray& offers_array);
    QList<Reservation_Info> parse_reservations(const QJsonArray& reservations_array);
    User_Info parse_user_info(const QJsonObject& user_object);
    
    void reset_connection();
    void set_connection_status(Connection_Status status);
    
    // Utility functions
    void log_network_activity(const QString& message, const QString& type = "Info");
    void update_last_activity();
    QString format_json_message(const QJsonObject& message);
    bool validate_server_response(const QJsonObject& response);
    void handle_connection_timeout();
    void handle_network_error(const QString& error_message);
};