#include "network/Api_Client.h"
#include "config/config.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QMutexLocker>
#include <QDebug>
#include <mutex>

// Static instance
Api_Client* Api_Client::s_instance = nullptr;

Api_Client::Api_Client(QObject* parent)
    : QObject(parent)
    , m_socket(std::make_unique<QTcpSocket>(this))
    , m_timeout_timer(std::make_unique<QTimer>(this))
    , m_server_host(Config::Server::DEFAULT_HOST)
    , m_server_port(Config::Server::DEFAULT_PORT)
    , m_timeout_ms(DEFAULT_TIMEOUT_MS)
    , m_is_connected(false)
    , m_current_request_type(Request_Type::Login)
{
    // Setup timeout timer
    m_timeout_timer->setSingleShot(true);
    connect(m_timeout_timer.get(), &QTimer::timeout, 
            this, &Api_Client::on_request_timeout);
    
    // Setup socket signals
    connect(m_socket.get(), &QTcpSocket::connected,
            this, &Api_Client::on_socket_connected);
    connect(m_socket.get(), &QTcpSocket::disconnected,
            this, &Api_Client::on_socket_disconnected);
    connect(m_socket.get(), &QTcpSocket::readyRead,
            this, &Api_Client::on_socket_ready_read);
    connect(m_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &Api_Client::on_socket_error);
}

Api_Client::~Api_Client()
{
    disconnect_from_server();
    s_instance = nullptr;
}

Api_Client& Api_Client::instance()
{
    // Thread-safe singleton using call_once
    static std::once_flag once_flag;
    std::call_once(once_flag, []() {
        s_instance = new Api_Client();
    });
    return *s_instance;
}

void Api_Client::set_server_url(const QString& host, int port)
{
    QMutexLocker locker(&m_mutex);
    m_server_host = host;
    m_server_port = port;
    
    qDebug() << "Api_Client server set to:" << host << ":" << port;
}

void Api_Client::set_auth_token(const QString& token)
{
    QMutexLocker locker(&m_mutex);
    m_auth_token = token;
}

void Api_Client::set_timeout(int timeout_ms)
{
    QMutexLocker locker(&m_mutex);
    m_timeout_ms = timeout_ms;
}

void Api_Client::connect_to_server()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState)
    {
        return; // Already connected
    }
    
    if (m_socket->state() == QAbstractSocket::ConnectingState)
    {
        return; // Already connecting
    }
    
    qDebug() << "Connecting to server:" << m_server_host << ":" << m_server_port;
    m_socket->connectToHost(m_server_host, m_server_port);
}

void Api_Client::disconnect_from_server()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState)
    {
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState)
        {
            m_socket->waitForDisconnected(3000);
        }
    }
}

void Api_Client::test_connection()
{
    QJsonObject testData;
    testData["type"] = "KEEPALIVE";
    
    send_request(Request_Type::Login, testData);
}

void Api_Client::login(const QString& username, const QString& password)
{
    QJsonObject loginData;
    loginData["type"] = "AUTH";
    loginData["username"] = username;
    loginData["password"] = password;
    
    send_request(Request_Type::Login, loginData);
}

void Api_Client::register_user(const QJsonObject& user_data)
{
    QJsonObject registerData = user_data;
    registerData["type"] = "REGISTER";
    
    send_request(Request_Type::Register, registerData);
}

void Api_Client::logout()
{
    QMutexLocker locker(&m_mutex);
    m_auth_token.clear();
    m_is_connected = false;
    disconnect_from_server();
    
    emit logged_out();
    emit connection_status_changed(false);
}

void Api_Client::get_destinations()
{
    QJsonObject requestData;
    requestData["type"] = "GET_DESTINATIONS";
    
    send_request(Request_Type::Get_Destinations, requestData);
}

void Api_Client::get_offers()
{
    QJsonObject requestData;
    requestData["type"] = "GET_OFFERS";
    
    send_request(Request_Type::Get_Offers, requestData);
}

void Api_Client::search_offers(const QJsonObject& search_params)
{
    QJsonObject requestData = search_params;
    requestData["type"] = "SEARCH_OFFERS";
    
    send_request(Request_Type::Search_Offers, requestData);
}

void Api_Client::get_user_info()
{
    QJsonObject requestData;
    requestData["type"] = "GET_USER_INFO";
    
    send_request(Request_Type::Get_User_Info, requestData);
}

void Api_Client::update_user_info(const QJsonObject& user_info)
{
    QJsonObject requestData = user_info;
    requestData["type"] = "UPDATE_USER_INFO";
    
    send_request(Request_Type::Update_User_Info, requestData);
}

void Api_Client::get_user_reservations()
{
    QJsonObject requestData;
    requestData["type"] = "GET_USER_RESERVATIONS";
    
    send_request(Request_Type::Get_User_Reservations, requestData);
}

void Api_Client::book_offer(int offer_id, int person_count, const QJsonObject& additional_info)
{
    QJsonObject requestData = additional_info;
    requestData["type"] = "BOOK_OFFER";
    requestData["offer_id"] = offer_id;
    requestData["person_count"] = person_count;
    
    send_request(Request_Type::Book_Offer, requestData);
}

void Api_Client::cancel_reservation(int reservation_id)
{
    QJsonObject requestData;
    requestData["type"] = "CANCEL_RESERVATION";
    requestData["reservation_id"] = reservation_id;
    
    send_request(Request_Type::Cancel_Reservation, requestData);
}

bool Api_Client::is_connected() const
{
    QMutexLocker locker(&m_mutex);
    return m_is_connected && m_socket->state() == QAbstractSocket::ConnectedState;
}

QString Api_Client::get_server_url() const
{
    QMutexLocker locker(&m_mutex);
    return QString("%1:%2").arg(m_server_host).arg(m_server_port);
}

QString Api_Client::get_last_error() const
{
    QMutexLocker locker(&m_mutex);
    return m_last_error;
}

void Api_Client::send_request(Request_Type type, const QJsonObject& data)
{
    m_current_request_type = type;
    
    if (!is_connected())
    {
        // Store the request for sending after connection is established
        {
            QMutexLocker locker(&m_mutex);
            m_pending_request = Pending_Request{type, data};
        }
        connect_to_server();
        return;
    }
    
    send_json_message(data);
}

void Api_Client::send_json_message(const QJsonObject& message)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState)
    {
        emit_error("Not connected to server");
        return;
    }
    
    QJsonDocument doc(message);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    
    // Add carriage return and newline delimiter to match server expectation
    jsonData.append("\r\n");
    
    qDebug() << "Sending JSON message:" << jsonData;
    
    qint64 bytesWritten = m_socket->write(jsonData);
    if (bytesWritten != jsonData.size())
    {
        emit_error("Failed to send complete message");
        return;
    }
    
    m_socket->flush();
    
    // Start timeout timer
    m_timeout_timer->start(m_timeout_ms);
}

void Api_Client::on_socket_connected()
{
    qDebug() << "Socket connected to server";
    
    std::optional<Pending_Request> pending;
    {
        QMutexLocker locker(&m_mutex);
        m_is_connected = true;
        pending = m_pending_request;
        m_pending_request.reset();
    }
    
    emit connection_status_changed(true);
    
    // Send pending request if exists
    if (pending.has_value())
    {
        qDebug() << "Sending pending request:" << request_type_to_string(pending->type);
        m_current_request_type = pending->type;
        send_json_message(pending->data);
    }
}

void Api_Client::on_socket_disconnected()
{
    qDebug() << "Socket disconnected from server";
    
    {
        QMutexLocker locker(&m_mutex);
        m_is_connected = false;
        m_receive_buffer.clear();
    }
    
    m_timeout_timer->stop();
    emit connection_status_changed(false);
}

void Api_Client::on_socket_ready_read()
{
    m_timeout_timer->stop();
    
    QByteArray data = m_socket->readAll();
    
    // Check buffer size limit to prevent memory exhaustion
    if (m_receive_buffer.size() + data.size() > MAX_BUFFER_SIZE)
    {
        qWarning() << "Buffer size limit exceeded, clearing buffer";
        emit_error("Receive buffer overflow - connection reset");
        disconnect_from_server();
        return;
    }
    
    m_receive_buffer.append(data);
    
    // Process complete JSON messages (delimited by newlines)
    int processed_messages = 0;
    const int MAX_MESSAGES_PER_READ = 100; // Prevent infinite loop
    
    while (processed_messages < MAX_MESSAGES_PER_READ)
    {
        int newlineIndex = m_receive_buffer.indexOf('\n');
        if (newlineIndex == -1)
        {
            break; // No complete message yet
        }
        
        QByteArray messageData = m_receive_buffer.left(newlineIndex);
        // Remove \r if present before \n
        if (!messageData.isEmpty() && messageData.endsWith('\r'))
        {
            messageData.chop(1);
        }
        m_receive_buffer.remove(0, newlineIndex + 1);
        
        if (messageData.isEmpty())
        {
            processed_messages++;
            continue;
        }
        
        qDebug() << "Received JSON message:" << messageData;
        
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(messageData, &parseError);
        
        if (parseError.error != QJsonParseError::NoError)
        {
            qWarning() << "JSON parse error:" << parseError.errorString();
            processed_messages++;
            continue;
        }
        
        if (doc.isObject())
        {
            handle_response(doc.object());
        }
        
        processed_messages++;
    }
    
    if (processed_messages >= MAX_MESSAGES_PER_READ)
    {
        qWarning() << "Maximum messages per read exceeded, possible flooding attack";
    }
}

void Api_Client::on_socket_error(QAbstractSocket::SocketError error)
{
    handle_socket_error(error);
}

void Api_Client::on_request_timeout()
{
    emit_error("Request timeout");
    disconnect_from_server();
}

void Api_Client::handle_response(const QJsonObject& response)
{
    Api_Response api_response = parse_json_response(response);
    
    qDebug() << "Response received for:" << request_type_to_string(m_current_request_type);
    qDebug() << "Success:" << api_response.success;
    qDebug() << "Message:" << api_response.message;
    
    if (api_response.success)
    {
        if (m_current_request_type == Request_Type::Login || m_current_request_type == Request_Type::Register)
        {
            process_authentification_response(api_response);
        }
        else
        {
            process_data_response(m_current_request_type, api_response);
        }
    }
    else
    {
        QMutexLocker locker(&m_mutex);
        m_last_error = api_response.message;
        emit_error(api_response.message);
    }
    
    emit request_completed(m_current_request_type, api_response);
}

Api_Client::Api_Response Api_Client::parse_json_response(const QJsonObject& json_response) const
{
    Api_Response response;
    response.success = json_response["success"].toBool();
    response.message = json_response["message"].toString();
    response.data = json_response["data"].toObject();
    response.status_code = 200; // TCP doesn't have HTTP status codes
    
    return response;
}

void Api_Client::process_authentification_response(const Api_Response& response)
{
    if (response.success)
    {
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

void Api_Client::handle_socket_error(QAbstractSocket::SocketError error)
{
    {
        QMutexLocker locker(&m_mutex);
        m_is_connected = false;
    }
    
    emit connection_status_changed(false);
    
    QString errorMsg;
    switch (error)
    {
        case QAbstractSocket::ConnectionRefusedError:
            errorMsg = "Connection refused - Server might be down";
            break;
        case QAbstractSocket::HostNotFoundError:
            errorMsg = "Host not found - Check server address";
            break;
        case QAbstractSocket::SocketTimeoutError:
            errorMsg = "Socket timeout";
            break;
        case QAbstractSocket::NetworkError:
            errorMsg = "Network error";
            break;
        default:
            errorMsg = m_socket->errorString();
            break;
    }
    
    emit_error(errorMsg);
}

void Api_Client::emit_error(const QString& error_message)
{
    {
        QMutexLocker locker(&m_mutex);
        m_last_error = error_message;
    }
    
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