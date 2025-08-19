#include "network/Api_Client.h"
#include "config/config.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QSslError>
#include <QDebug>

// Static instance
Api_Client* Api_Client::s_instance = nullptr;

Api_Client::Api_Client(QObject* parent)
    : QObject(parent)
    , m_network_manager(std::make_unique<QNetworkAccessManager>(this))
    , m_timeout_timer(std::make_unique<QTimer>(this))
    , m_server_host(Config::Server::DEFAULT_HOST)
    , m_server_port(Config::Server::DEFAULT_PORT)
    , m_timeout_ms(DEFAULT_TIMEOUT_MS)
    , m_is_connected(false)
    , m_current_reply(nullptr)
    , m_current_request_type(Request_Type::Login)
{
    // Setup server URL
    m_server_url = QString("http://%1:%2").arg(m_server_host).arg(m_server_port);
    
    // Setup timeout timer
    m_timeout_timer->setSingleShot(true);
    connect(m_timeout_timer.get(), &QTimer::timeout, 
            this, &Api_Client::on_request_timeout);
    
    // Setup network manager signals
    connect(m_network_manager.get(), &QNetworkAccessManager::finished,
            this, &Api_Client::on_network_reply_finished);
}

Api_Client::~Api_Client()
{
    if (m_current_reply)
    {
        m_current_reply->abort();
        m_current_reply->deleteLater();
    }
    s_instance = nullptr;
}

Api_Client& Api_Client::instance()
{
    if (!s_instance)
    {
        s_instance = new Api_Client();
    }
    return *s_instance;
}

void Api_Client::set_server_url(const QString& host, int port)
{
    m_server_host = host;
    m_server_port = port;
    m_server_url = QString("http://%1:%2").arg(host).arg(port);
    
    qDebug() << "Api_Client server URL set to:" << m_server_url;
}

void Api_Client::set_auth_token(const QString& token)
{
    m_auth_token = token;
}

void Api_Client::set_timeout(int timeout_ms)
{
    m_timeout_ms = timeout_ms;
}

void Api_Client::test_connection()
{
    QJsonObject testData;
    testData["type"] = "KEEPALIVE";
    
    send_request(Request_Type::Login, "", testData, "POST");
}

void Api_Client::login(const QString& username, const QString& password)
{
    QJsonObject loginData;
    loginData["type"] = "AUTH";
    loginData["username"] = username;
    loginData["password"] = password;
    
    send_request(Request_Type::Login, "", loginData, "POST");
}

void Api_Client::register_user(const QJsonObject& user_data)
{
    QJsonObject registerData = user_data;
    registerData["type"] = "REGISTER";
    
    send_request(Request_Type::Register, "", registerData, "POST");
}

void Api_Client::logout()
{
    m_auth_token.clear();
    m_is_connected = false;
    emit logged_out();
    emit connection_status_changed(false);
}

void Api_Client::get_destinations()
{
    QJsonObject requestData;
    requestData["type"] = "GET_DESTINATIONS";
    
    send_request(Request_Type::Get_Destinations, "", requestData, "POST");
}

void Api_Client::get_offers()
{
    QJsonObject requestData;
    requestData["type"] = "GET_OFFERS";
    
    send_request(Request_Type::Get_Offers, "", requestData, "POST");
}

void Api_Client::search_offers(const QJsonObject& search_params)
{
    QJsonObject requestData = search_params;
    requestData["type"] = "SEARCH_OFFERS";
    
    send_request(Request_Type::Search_Offers, "", requestData, "POST");
}

void Api_Client::get_user_info()
{
    QJsonObject requestData;
    requestData["type"] = "GET_USER_INFO";
    
    send_request(Request_Type::Get_User_Info, "", requestData, "POST");
}

void Api_Client::update_user_info(const QJsonObject& user_info)
{
    QJsonObject requestData = user_info;
    requestData["type"] = "UPDATE_USER_INFO";
    
    send_request(Request_Type::Update_User_Info, "", requestData, "POST");
}

void Api_Client::get_user_reservations()
{
    QJsonObject requestData;
    requestData["type"] = "GET_USER_RESERVATIONS";
    
    send_request(Request_Type::Get_User_Reservations, "", requestData, "POST");
}

void Api_Client::book_offer(int offer_id, int person_count, const QJsonObject& additional_info)
{
    QJsonObject requestData = additional_info;
    requestData["type"] = "BOOK_OFFER";
    requestData["offer_id"] = offer_id;
    requestData["person_count"] = person_count;
    
    send_request(Request_Type::Book_Offer, "", requestData, "POST");
}

void Api_Client::cancel_reservation(int reservation_id)
{
    QJsonObject requestData;
    requestData["type"] = "CANCEL_RESERVATION";
    requestData["reservation_id"] = reservation_id;
    
    send_request(Request_Type::Cancel_Reservation, "", requestData, "POST");
}

bool Api_Client::is_connected() const
{
    return m_is_connected;
}

QString Api_Client::get_server_url() const
{
    return m_server_url;
}

QString Api_Client::get_last_error() const
{
    return m_last_error;
}

QNetworkRequest Api_Client::create_request(const QString& endpoint) const
{
    QUrl url(m_server_url + endpoint);
    QNetworkRequest request(url);
    
    // Set headers
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, 
                     Config::Application::APP_NAME + " " + Config::Application::VERSION);
    
    // Set authentication if available
    if (!m_auth_token.isEmpty())
    {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth_token).toUtf8());
    }
    
    return request;
}

void Api_Client::send_request(Request_Type type, const QString& endpoint, 
                           const QJsonObject& data, const QString& method)
{
    // Cancel previous request if still running
    if (m_current_reply)
    {
        m_current_reply->abort();
        m_current_reply->deleteLater();
        m_current_reply = nullptr;
    }
    
    m_current_request_type = type;
    
    QNetworkRequest request = create_request(endpoint);
    QByteArray jsonData = QJsonDocument(data).toJson(QJsonDocument::Compact);
    
    qDebug() << "Sending request:" << request_type_to_string(type);
    qDebug() << "URL:" << request.url().toString();
    qDebug() << "Data:" << jsonData;
    
    // Send request based on method
    if (method.toUpper() == "POST")
    {
        m_current_reply = m_network_manager->post(request, jsonData);
    }
    else if (method.toUpper() == "PUT")
    {
        m_current_reply = m_network_manager->put(request, jsonData);
    }
    else if (method.toUpper() == "DELETE")
    {
        m_current_reply = m_network_manager->deleteResource(request);
    }
    else
    {
        m_current_reply = m_network_manager->get(request);
    }
    
    // Connect SSL error signal for this specific reply
    if (m_current_reply) {
        connect(m_current_reply, &QNetworkReply::sslErrors,
                [this](const QList<QSslError>& errors) {
                    this->on_ssl_errors(m_current_reply, errors);
                });
    }
    
    // Start timeout timer
    m_timeout_timer->start(m_timeout_ms);
}

void Api_Client::on_network_reply_finished()
{
    m_timeout_timer->stop();
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || reply != m_current_reply)
    {
        return;
    }
    
    handle_response(m_current_request_type, reply);
    
    reply->deleteLater();
    m_current_reply = nullptr;
}

void Api_Client::on_request_timeout()
{
    if (m_current_reply)
    {
        m_current_reply->abort();
        emit_error("Request timeout");
    }
}

void Api_Client::on_ssl_errors(QNetworkReply* reply, const QList<QSslError>& errors)
{
    Q_UNUSED(reply)
    QString errorString = "SSL Errors: ";
    for (const auto& error : errors)
    {
        errorString += error.errorString() + "; ";
    }
    emit_error(errorString);
}

void Api_Client::handle_response(Request_Type type, QNetworkReply* reply)
{
    Api_Response response = parse_response(reply);
    
    qDebug() << "Response received for:" << request_type_to_string(type);
    qDebug() << "Success:" << response.success;
    qDebug() << "Message:" << response.message;
    
    if (response.success)
    {
        m_is_connected = true;
        emit connection_status_changed(true);
        
        if (type == Request_Type::Login || type == Request_Type::Register)
        {
            process_authentification_response(response);
        } else
        {
            process_data_response(type, response);
        }
    }
    else
    {
        m_last_error = response.message;
        handle_network_error(reply->error(), response.error_details);
    }
    
    emit request_completed(type, response);
}

Api_Client::Api_Response Api_Client::parse_response(QNetworkReply* reply) const
{
    Api_Response response;
    response.status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    if (reply->error() != QNetworkReply::NoError)
    {
        response.success = false;
        response.message = reply->errorString();
        response.error_details = reply->readAll();
        return response;
    }
    
    QByteArray responseData = reply->readAll();
    
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError)
    {
        response.success = false;
        response.message = "JSON Parse Error: " + parseError.errorString();
        response.error_details = responseData;
        return response;
    }
    
    QJsonObject jsonObj = jsonDoc.object();
    response.success = jsonObj["success"].toBool();
    response.message = jsonObj["message"].toString();
    response.data = jsonObj["data"].toObject();
    
    return response;
}

void Api_Client::process_authentification_response(const Api_Response& response)
{
    if (response.success)
    {
        // Extract user data and potentially auth token
        QJsonObject userData = response.data;
        
        if (m_current_request_type == Request_Type::Login)
        {
            emit login_success(userData);
        }
        else if (m_current_request_type == Request_Type::Register)
        {
            emit register_success();
        }
    }
    else
    {
        if (m_current_request_type == Request_Type::Login)
        {
            emit login_failed(response.message);
        }
        else if (m_current_request_type == Request_Type::Register)
        {
            emit register_failed(response.message);
        }
    }
}

void Api_Client::process_data_response(Request_Type type, const Api_Response& response)
{
    QJsonArray dataArray;
    
    // Check if data is an array or convert object to array
    if (response.data.contains("data") && response.data["data"].isArray())
    {
        dataArray = response.data["data"].toArray();
    }
    else if (response.data.isEmpty())
    {
        // Empty response is OK for some operations
    }
    else
    {
        // Single object response, wrap in array
        dataArray.append(response.data);
    }
    
    switch (type)
    {
        case Request_Type::Get_Destinations:
            emit destinations_received(dataArray);
            break;
            
        case Request_Type::Get_Offers:
        case Request_Type::Search_Offers:
            emit offers_received(dataArray);
            break;
            
        case Request_Type::Get_User_Info:
            emit user_info_received(response.data);
            break;
            
        case Request_Type::Get_User_Reservations:
            emit reservations_received(dataArray);
            break;
            
        case Request_Type::Book_Offer:
            emit booking_success(response.message);
            break;
            
        case Request_Type::Cancel_Reservation:
            emit cancellation_success(response.message);
            break;
            
        default:
            break;
    }
}

void Api_Client::handle_network_error(QNetworkReply::NetworkError error, const QString& error_string)
{
    m_is_connected = false;
    emit connection_status_changed(false);
    
    QString errorMsg;
    switch (error)
    {
        case QNetworkReply::ConnectionRefusedError:
            errorMsg = "Connection refused - Server might be down";
            break;
        case QNetworkReply::HostNotFoundError:
            errorMsg = "Host not found - Check server address";
            break;
        case QNetworkReply::TimeoutError:
            errorMsg = "Request timeout";
            break;
        case QNetworkReply::ContentNotFoundError:
            errorMsg = "Content not found (404)";
            break;
        default:
            errorMsg = error_string.isEmpty() ? "Unknown network error" : error_string;
            break;
    }
    
    emit_error(errorMsg);
}

void Api_Client::emit_error(const QString& error_message)
{
    m_last_error = error_message;
    qWarning() << "Api_Client error:" << error_message;
    emit network_error(error_message);
    
    // Emit specific error signals based on current request type
    switch (m_current_request_type)
    {
        case Request_Type::Login:
            emit login_failed(error_message);
            break;
        case Request_Type::Register:
            emit register_failed(error_message);
            break;
        case Request_Type::Book_Offer:
            emit booking_failed(error_message);
            break;
        case Request_Type::Cancel_Reservation:
            emit cancellation_failed(error_message);
            break;
        default:
            break;
    }
}

QString Api_Client::request_type_to_string(Request_Type type) const
{
    switch (type)
    {
        case Request_Type::Login: return "Login";
        case Request_Type::Register: return "Register";
        case Request_Type::Get_Destinations: return "Get_Destinations";
        case Request_Type::Get_Offers: return "Get_Offers";
        case Request_Type::Search_Offers: return "Search_Offers";
        case Request_Type::Book_Offer: return "Book_Offer";
        case Request_Type::Get_User_Reservations: return "Get_User_Reservations";
        case Request_Type::Cancel_Reservation: return "Cancel_Reservation";
        case Request_Type::Get_User_Info: return "Get_User_Info";
        case Request_Type::Update_User_Info: return "Update_User_Info";
        default: return "Unknown";
    }
}

bool Api_Client::is_authentification_required(Request_Type type) const
{
    switch (type)
    {
        case Request_Type::Login:
        case Request_Type::Register:
        case Request_Type::Get_Destinations:
        case Request_Type::Get_Offers:
        case Request_Type::Search_Offers:
            return false;
        default:
            return true;
    }
}