#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QSslError>
#include <memory>

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

    void set_server_url(const QString& host, int port);
    void set_auth_token(const QString& token);
    void set_timeout(int timeout_ms);

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
    void on_network_reply_finished();
    void on_request_timeout();
    void on_ssl_errors(QNetworkReply* reply, const QList<QSslError>& errors);

private:
    explicit Api_Client(QObject* parent = nullptr);
    ~Api_Client();

    QNetworkRequest create_request(const QString& endpoint) const;
    void send_request(Request_Type type, const QString& endpoint,
                      const QJsonObject& data = {}, const QString& method = "GET");
    void handle_response(Request_Type type, QNetworkReply* reply);
    
    Api_Response parse_response(QNetworkReply* reply) const;
    void process_authentification_response(const Api_Response& response);
    void process_data_response(Request_Type type, const Api_Response& response);

    void handle_network_error(QNetworkReply::NetworkError error, const QString& error_message);
    void emit_error(const QString& error_message);

    QString request_type_to_string(Request_Type type) const;
    bool is_authentification_required(Request_Type type) const;

    static Api_Client* s_instance;

    std::unique_ptr<QNetworkAccessManager> m_network_manager;
    std::unique_ptr<QTimer> m_timeout_timer;

    QString m_server_host;
    int m_server_port;
    QString m_server_url;
    QString m_auth_token;
    int m_timeout_ms;

    bool m_is_connected;
    QString m_last_error;
    QNetworkReply* m_current_reply;
    Request_Type m_current_request_type;

    static constexpr int DEFAULT_TIMEOUT_MS = 30000;
    static constexpr int DEFAULT_PORT = 8080;
};

Q_DECLARE_METATYPE(Api_Client::Request_Type)
Q_DECLARE_METATYPE(Api_Client::Api_Response)