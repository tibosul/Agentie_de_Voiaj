#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QMutex>
#include <memory>
#include <optional>

class Api_Client : public QObject
{
    Q_OBJECT

public:
    enum class Request_Type
    {
        Login,
        Register,
        Get_Destinations,
        Get_Offers,
        Search_Offers,
        Book_Offer,
        Get_User_Reservations,
        Cancel_Reservation,
        Get_User_Info,
        Update_User_Info
    };

    struct Api_Response
    {
        bool success = false;
        QString message;
        QJsonObject data;
        int status_code = 0;
        QString error_details;
    };

    static Api_Client& instance();
    static void shutdown();

    void set_server_url(const QString& host, int port);
    void set_auth_token(const QString& token);
    void set_timeout(int timeout_ms);

    void initialize_connection();
    void test_connection();

    void login(const QString& username, const QString& password);
    void register_user(const QJsonObject& user_data);
    void logout();

    void get_destinations();
    void get_offers();
    void search_offers(const QJsonObject& search_params);

    void get_user_info();
    void update_user_info(const QJsonObject& user_data);
    void get_user_reservations();

    void book_offer(int offer_id, int person_count, const QJsonObject& additional_info);
    void cancel_reservation(int reservation_id);

    bool is_connected() const;
    QString get_server_url() const;
    QString get_last_error() const;

signals:
    void connection_status_changed(bool is_connected);
    void network_error(const QString& error_message);

    void login_success(const QJsonObject& user_data);
    void login_failed(const QString& error_message);
    void register_success();
    void register_failed(const QString& error_message);
    void logged_out();

    void destinations_received(const QJsonArray& destinations);
    void offers_received(const QJsonArray& offers);
    void user_info_received(const QJsonObject& user_info);
    void reservations_received(const QJsonArray& reservations);

    void booking_success(const QString& message);
    void booking_failed(const QString& error_message);
    void cancellation_success(const QString& message);
    void cancellation_failed(const QString& error_message);

    void request_completed(Request_Type type, const Api_Response& response);

private slots:
    void on_socket_connected();
    void on_socket_disconnected();
    void on_socket_ready_read();
    void on_socket_error(QAbstractSocket::SocketError error);
    void on_request_timeout();
    void attempt_reconnection();

private:
    explicit Api_Client(QObject* parent = nullptr);
    ~Api_Client();

    void connect_to_server();
    void disconnect_from_server();
    void send_json_message(const QJsonObject& message);
    void send_request(Request_Type type, const QJsonObject& data);
    void handle_response(const QJsonObject& response);
    
    Api_Response parse_json_response(const QJsonObject& json_response) const;
    void process_authentication_response(const Api_Response& response);
    void process_data_response(Request_Type type, const Api_Response& response);

    void handle_socket_error(QAbstractSocket::SocketError error);
    void emit_error(const QString& error_message);

    QString request_type_to_string(Request_Type type) const;
    bool is_authentication_required(Request_Type type) const;

    // Constants
    static constexpr int CONNECTION_TIMEOUT_MS = 5000;
    static constexpr int REQUEST_TIMEOUT_MS = 10000;

    static Api_Client* s_instance;

    std::unique_ptr<QTcpSocket> m_socket;
    std::unique_ptr<QTimer> m_timeout_timer;
    std::unique_ptr<QTimer> m_reconnect_timer;
    mutable QMutex m_mutex;

    QString m_server_host;
    int m_server_port;
    QString m_auth_token;
    int m_timeout_ms;

    bool m_is_connected;
    QString m_last_error;
    Request_Type m_current_request_type;
    QByteArray m_receive_buffer;
    
    // Store pending request when not connected
    struct Pending_Request {
        Request_Type type;
        QJsonObject data;
    };
    std::optional<Pending_Request> m_pending_request;

    static constexpr int DEFAULT_TIMEOUT_MS = 30000;
    static constexpr int DEFAULT_PORT = 8080;
    static constexpr int MAX_BUFFER_SIZE = 1024 * 1024; // 1MB limit
};

Q_DECLARE_METATYPE(Api_Client::Request_Type)
Q_DECLARE_METATYPE(Api_Client::Api_Response)