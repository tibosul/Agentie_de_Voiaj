#include "network_manager.h"
#include <QJsonParseError>
#include <QDebug>

Network_Manager::Network_Manager(QObject* parent)
    : QObject(parent), tcp_socket(nullptr), connection_status(Connection_Status::Disconnected),
      server_host(Config::Network::DEFAULT_SERVER_HOST), server_port(Config::Network::DEFAULT_SERVER_PORT),
      connection_attempts(0)
{
    // Create TCP socket
    tcp_socket = new QTcpSocket(this);
    setup_socket_connections();
    
    // Setup keepalive timer
    keepalive_timer = new QTimer(this);
    connect(keepalive_timer, &QTimer::timeout, this, &Network_Manager::on_keepalive_timer);
    keepalive_timer->setInterval(Config::Network::KEEPALIVE_INTERVAL_MS);
    
    // Initialize logging
    Utils::Logger::info("Network Manager initialized", "Network");
    update_last_activity();
}

Network_Manager::~Network_Manager()
{
    disconnect_from_server();
}

bool Network_Manager::connect_to_server(const QString& host, quint16 port)
{
    if (connection_status == Connection_Status::Connected)
    {
        Utils::Logger::warning("Already connected to server", "Network");
        return true;
    }
    
    server_host = host;
    server_port = port;
    connection_attempts++;
    
    log_network_activity(QString("Attempting to connect to %1:%2 (attempt %3)")
                        .arg(host).arg(port).arg(connection_attempts), "Info");
    
    set_connection_status(Connection_Status::Connecting);
    
    // Set socket timeouts
    tcp_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    tcp_socket->connectToHost(host, port);
    
    // Set connection timeout
    QTimer::singleShot(Config::Network::CONNECTION_TIMEOUT_MS, this, [this]() {
        if (connection_status == Connection_Status::Connecting) {
            handle_connection_timeout();
        }
    });
    
    return true; // Connection is asynchronous - result will come via signals
}

void Network_Manager::disconnect_from_server()
{
    keepalive_timer->stop();
    
    if (tcp_socket->state() != QAbstractSocket::UnconnectedState)
    {
        tcp_socket->disconnectFromHost();
        if (tcp_socket->state() != QAbstractSocket::UnconnectedState)
        {
            tcp_socket->waitForDisconnected(3000); // Wait up to 3 seconds
        }
    }
    
    reset_connection();
}

Network_Manager::Connection_Status Network_Manager::get_connection_status() const
{
    return connection_status;
}

QString Network_Manager::get_last_error() const
{
    return last_error;
}

void Network_Manager::authenticate_user(const QString& username, const QString& password)
{
    if (connection_status != Connection_Status::Connected)
    {
        emit authentication_failed("Not connected to server");
        return;
    }
    
    QJsonObject message;
    message["type"] = "AUTH";
    message["username"] = username;
    message["password"] = password;
    
    send_json_message(message);
}

void Network_Manager::register_user(const QString& username, const QString& password,
                                   const QString& email, const QString& first_name,
                                   const QString& last_name, const QString& phone_number)
{
    if (connection_status != Connection_Status::Connected)
    {
        emit registration_failed("Not connected to server");
        return;
    }
    
    QJsonObject message;
    message["type"] = "REGISTER";
    message["username"] = username;
    message["password"] = password;
    message["email"] = email;
    message["first_name"] = first_name;
    message["last_name"] = last_name;
    message["phone_number"] = phone_number;
    
    send_json_message(message);
}

void Network_Manager::logout_user()
{
    current_user = User_Info(); // Reset user info
    disconnect_from_server();
    emit user_logged_out();
}

void Network_Manager::get_destinations()
{
    if (connection_status != Connection_Status::Connected)
    {
        return;
    }
    
    QJsonObject message;
    message["type"] = "GET_DESTINATIONS";
    
    send_json_message(message);
}

void Network_Manager::get_offers()
{
    if (connection_status != Connection_Status::Connected)
    {
        return;
    }
    
    QJsonObject message;
    message["type"] = "GET_OFFERS";
    
    send_json_message(message);
}

void Network_Manager::search_offers(const QString& destination, double min_price,
                                   double max_price, const QString& start_date,
                                   const QString& end_date)
{
    if (connection_status != Connection_Status::Connected)
    {
        return;
    }
    
    QJsonObject message;
    message["type"] = "SEARCH_OFFERS";
    
    if (!destination.isEmpty())
        message["destination"] = destination;
    if (min_price > 0)
        message["min_price"] = min_price;
    if (max_price > 0)
        message["max_price"] = max_price;
    if (!start_date.isEmpty())
        message["start_date"] = start_date;
    if (!end_date.isEmpty())
        message["end_date"] = end_date;
    
    send_json_message(message);
}

void Network_Manager::book_offer(int offer_id, int person_count)
{
    if (connection_status != Connection_Status::Connected)
    {
        emit booking_failed("Not connected to server");
        return;
    }
    
    if (!is_user_logged_in())
    {
        emit booking_failed("User not logged in");
        return;
    }
    
    QJsonObject message;
    message["type"] = "BOOK_OFFER";
    message["offer_id"] = offer_id;
    message["person_count"] = person_count;
    
    send_json_message(message);
}

void Network_Manager::get_user_reservations()
{
    if (connection_status != Connection_Status::Connected)
    {
        return;
    }
    
    if (!is_user_logged_in())
    {
        return;
    }
    
    QJsonObject message;
    message["type"] = "GET_USER_RESERVATIONS";
    
    send_json_message(message);
}

void Network_Manager::cancel_reservation(int reservation_id)
{
    if (connection_status != Connection_Status::Connected)
    {
        emit cancellation_failed("Not connected to server");
        return;
    }
    
    QJsonObject message;
    message["type"] = "CANCEL_RESERVATION";
    message["reservation_id"] = reservation_id;
    
    send_json_message(message);
}

void Network_Manager::get_user_info()
{
    if (connection_status != Connection_Status::Connected)
    {
        return;
    }
    
    QJsonObject message;
    message["type"] = "GET_USER_INFO";
    
    send_json_message(message);
}

void Network_Manager::update_user_info(const QString& email, const QString& first_name,
                                      const QString& last_name, const QString& phone_number)
{
    if (connection_status != Connection_Status::Connected)
    {
        emit user_info_update_failed("Not connected to server");
        return;
    }
    
    QJsonObject message;
    message["type"] = "UPDATE_USER_INFO";
    message["email"] = email;
    message["first_name"] = first_name;
    message["last_name"] = last_name;
    message["phone_number"] = phone_number;
    
    send_json_message(message);
}

void Network_Manager::on_connected()
{
    log_network_activity(QString("Successfully connected to %1:%2").arg(server_host).arg(server_port), "Info");
    connection_attempts = 0; // Reset attempts counter on successful connection
    set_connection_status(Connection_Status::Connected);
    keepalive_timer->start();
    emit connected_to_server();
}

void Network_Manager::on_disconnected()
{
    log_network_activity("Disconnected from server", "Info");
    keepalive_timer->stop();
    set_connection_status(Connection_Status::Disconnected);
    emit disconnected_from_server();
}

void Network_Manager::on_socket_error(QAbstractSocket::SocketError error)
{
    last_error = tcp_socket->errorString();
    log_network_activity(QString("Socket error %1: %2").arg(error).arg(last_error), "Error");
    set_connection_status(Connection_Status::Error);
    handle_network_error(last_error);
}

void Network_Manager::on_ready_read()
{
    // Read all available data
    receive_buffer.append(tcp_socket->readAll());
    
    // Process complete JSON messages
    process_incoming_data();
}

void Network_Manager::setup_socket_connections()
{
    // Connect Qt TCP socket signals to our slots
    connect(tcp_socket, &QTcpSocket::connected, this, &Network_Manager::on_connected);
    connect(tcp_socket, &QTcpSocket::disconnected, this, &Network_Manager::on_disconnected);
    connect(tcp_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &Network_Manager::on_socket_error);
    connect(tcp_socket, &QTcpSocket::readyRead, this, &Network_Manager::on_ready_read);
}

void Network_Manager::process_incoming_data()
{
    // Look for complete JSON messages in the buffer, terminated by \r\n
    while (!receive_buffer.isEmpty())
    {
        // Look for message terminator \r\n
        int terminator_pos = receive_buffer.indexOf("\r\n");
        if (terminator_pos == -1)
        {
            // No complete message yet, wait for more data
            break;
        }
        
        // Extract complete message
        QByteArray message_data = receive_buffer.left(terminator_pos);
        receive_buffer.remove(0, terminator_pos + 2); // Remove message + \r\n
        
        // Parse JSON message
        QJsonParseError parse_error;
        QJsonDocument doc = QJsonDocument::fromJson(message_data, &parse_error);
        
        if (parse_error.error == QJsonParseError::NoError)
        {
            if (doc.isObject())
            {
                process_server_response(doc.object());
            }
            else
            {
                qDebug() << "Received non-object JSON:" << message_data;
            }
        }
        else
        {
            qDebug() << "JSON parse error:" << parse_error.errorString() << "Data:" << message_data;
        }
    }
}

void Network_Manager::on_keepalive_timer()
{
    if (connection_status == Connection_Status::Connected)
    {
        QJsonObject message;
        message["type"] = "KEEPALIVE";
        send_json_message(message);
    }
}

void Network_Manager::send_json_message(const QJsonObject& message)
{
    if (connection_status != Connection_Status::Connected)
    {
        Utils::Logger::warning("Cannot send message - not connected", "Network");
        return;
    }
    
    QString json_string = format_json_message(message);
    QByteArray data = json_string.toUtf8();
    // Add \r\n terminator as expected by server
    data.append("\r\n");
    
    Utils::Logger::log_network_request(json_string, server_host + ":" + QString::number(server_port));
    
    tcp_socket->write(data);
    tcp_socket->flush();
    update_last_activity();
}

void Network_Manager::process_server_response(const QJsonObject& response)
{
    update_last_activity();
    
    Utils::Logger::log_network_response(Utils::JSON::to_json_string(response), 
                                       server_host + ":" + QString::number(server_port));
    
    if (!validate_server_response(response))
    {
        handle_network_error(Config::ErrorMessages::JSON_PARSE_ERROR);
        return;
    }
    
    bool success = Utils::JSON::is_success_response(response);
    QString message = Utils::JSON::get_response_message(response);
    
    // Determine response type based on content
    if (response.contains("data"))
    {
        QJsonValue data = response["data"];
        
        // Check if it's user authentication response
        if (data.isObject() && data.toObject().contains("username"))
        {
            process_authentication_response(response);
        }
        // Check if it's destinations data
        else if (data.isArray() && !data.toArray().isEmpty() && 
                 data.toArray().first().toObject().contains("name") &&
                 data.toArray().first().toObject().contains("country"))
        {
            process_destinations_response(response);
        }
        // Check if it's offers data
        else if (data.isArray() && !data.toArray().isEmpty() && 
                 data.toArray().first().toObject().contains("price_per_person"))
        {
            process_offers_response(response);
        }
        // Check if it's reservations data
        else if (data.isArray() && !data.toArray().isEmpty() && 
                 data.toArray().first().toObject().contains("reservation_date"))
        {
            process_reservations_response(response);
        }
        // Check if it's booking response
        else if (data.isObject() && data.toObject().contains("reservation_id"))
        {
            process_booking_response(response);
        }
        // Check if it's user info response
        else if (data.isObject() && data.toObject().contains("email"))
        {
            process_user_info_response(response);
        }
    }
    // Handle registration response
    else if (message.contains("registered", Qt::CaseInsensitive) || 
             message.contains("created", Qt::CaseInsensitive))
    {
        process_registration_response(response);
    }
    
    emit operation_completed("server_response", success, message);
}

void Network_Manager::process_authentication_response(const QJsonObject& response)
{
    bool success = response["success"].toBool();
    
    if (success)
    {
        QJsonObject user_data = response["data"].toObject();
        current_user = parse_user_info(user_data);
        emit authentication_successful(current_user);
    }
    else
    {
        emit authentication_failed(response["message"].toString());
    }
}

void Network_Manager::process_registration_response(const QJsonObject& response)
{
    bool success = response["success"].toBool();
    
    if (success)
    {
        emit registration_successful();
    }
    else
    {
        emit registration_failed(response["message"].toString());
    }
}

void Network_Manager::process_destinations_response(const QJsonObject& response)
{
    if (response["success"].toBool())
    {
        QJsonArray destinations_array = response["data"].toArray();
        QList<Destination_Info> destinations = parse_destinations(destinations_array);
        emit destinations_received(destinations);
    }
}

void Network_Manager::process_offers_response(const QJsonObject& response)
{
    if (response["success"].toBool())
    {
        QJsonArray offers_array = response["data"].toArray();
        QList<Offer_Info> offers = parse_offers(offers_array);
        emit offers_received(offers);
    }
}

void Network_Manager::process_reservations_response(const QJsonObject& response)
{
    if (response["success"].toBool())
    {
        QJsonArray reservations_array = response["data"].toArray();
        QList<Reservation_Info> reservations = parse_reservations(reservations_array);
        emit reservations_received(reservations);
    }
}

void Network_Manager::process_booking_response(const QJsonObject& response)
{
    bool success = response["success"].toBool();
    
    if (success)
    {
        int reservation_id = response["data"].toObject()["reservation_id"].toInt();
        emit booking_successful(reservation_id);
    }
    else
    {
        emit booking_failed(response["message"].toString());
    }
}

void Network_Manager::process_user_info_response(const QJsonObject& response)
{
    if (response["success"].toBool())
    {
        QJsonObject user_data = response["data"].toObject();
        User_Info user = parse_user_info(user_data);
        emit user_info_received(user);
    }
}

QList<Destination_Info> Network_Manager::parse_destinations(const QJsonArray& destinations_array)
{
    QList<Destination_Info> destinations;
    
    for (const QJsonValue& value : destinations_array)
    {
        QJsonObject obj = value.toObject();
        Destination_Info dest;
        // Handle both server format (Destination_ID, Name, etc.) and standard format
        dest.id = obj.contains("Destination_ID") ? obj["Destination_ID"].toInt() : obj["id"].toInt();
        dest.name = obj.contains("Name") ? obj["Name"].toString() : obj["name"].toString();
        dest.country = obj.contains("Country") ? obj["Country"].toString() : obj["country"].toString();
        dest.description = obj.contains("Description") ? obj["Description"].toString() : obj["description"].toString();
        dest.image_path = obj.contains("Image_Path") ? obj["Image_Path"].toString() : obj["image_path"].toString();
        destinations.append(dest);
    }
    
    return destinations;
}

QList<Offer_Info> Network_Manager::parse_offers(const QJsonArray& offers_array)
{
    QList<Offer_Info> offers;
    
    for (const QJsonValue& value : offers_array)
    {
        QJsonObject obj = value.toObject();
        Offer_Info offer;
        // Handle both server format (Offer_ID, Name, etc.) and standard format
        offer.id = obj.contains("Offer_ID") ? obj["Offer_ID"].toInt() : obj["id"].toInt();
        offer.name = obj.contains("Name") ? obj["Name"].toString() : obj["name"].toString();
        offer.destination_name = obj.contains("Destination_Name") ? obj["Destination_Name"].toString() : obj["destination_name"].toString();
        offer.accommodation_name = obj.contains("Accommodation_Name") ? obj["Accommodation_Name"].toString() : obj["accommodation_name"].toString();
        offer.transport_type = obj.contains("Transport_Name") ? obj["Transport_Name"].toString() : obj["transport_type"].toString();
        offer.price_per_person = obj.contains("Price_per_Person") ? obj["Price_per_Person"].toDouble() : obj["price_per_person"].toDouble();
        offer.duration_days = obj.contains("Duration_Days") ? obj["Duration_Days"].toInt() : obj["duration_days"].toInt();
        offer.departure_date = obj.contains("Departure_Date") ? obj["Departure_Date"].toString() : obj["departure_date"].toString();
        offer.return_date = obj.contains("Return_Date") ? obj["Return_Date"].toString() : obj["return_date"].toString();
        // Calculate available seats: Total_Seats - Reserved_Seats
        int total_seats = obj.contains("Total_Seats") ? obj["Total_Seats"].toInt() : obj["total_seats"].toInt();
        int reserved_seats = obj.contains("Reserved_Seats") ? obj["Reserved_Seats"].toInt() : obj["reserved_seats"].toInt();
        offer.available_seats = total_seats - reserved_seats;
        offer.description = obj.contains("Description") ? obj["Description"].toString() : obj["description"].toString();
        offer.status = obj.contains("Status") ? obj["Status"].toString() : obj["status"].toString();
        offers.append(offer);
    }
    
    return offers;
}

QList<Reservation_Info> Network_Manager::parse_reservations(const QJsonArray& reservations_array)
{
    QList<Reservation_Info> reservations;
    
    for (const QJsonValue& value : reservations_array)
    {
        QJsonObject obj = value.toObject();
        Reservation_Info reservation;
        // Handle both server format (Reservation_ID, Offer_ID, etc.) and standard format
        reservation.id = obj.contains("Reservation_ID") ? obj["Reservation_ID"].toInt() : obj["id"].toInt();
        reservation.offer_id = obj.contains("Offer_ID") ? obj["Offer_ID"].toInt() : obj["offer_id"].toInt();
        reservation.offer_name = obj.contains("Offer_Name") ? obj["Offer_Name"].toString() : obj["offer_name"].toString();
        reservation.number_of_persons = obj.contains("Number_of_Persons") ? obj["Number_of_Persons"].toInt() : obj["number_of_persons"].toInt();
        reservation.total_price = obj.contains("Total_Price") ? obj["Total_Price"].toDouble() : obj["total_price"].toDouble();
        reservation.reservation_date = obj.contains("Reservation_Date") ? obj["Reservation_Date"].toString() : obj["reservation_date"].toString();
        reservation.status = obj.contains("Status") ? obj["Status"].toString() : obj["status"].toString();
        reservation.notes = obj.contains("Notes") ? obj["Notes"].toString() : obj["notes"].toString();
        reservations.append(reservation);
    }
    
    return reservations;
}

User_Info Network_Manager::parse_user_info(const QJsonObject& user_object)
{
    User_Info user;
    // Handle both server format (User_ID, Username, etc.) and standard format (id, username, etc.)
    user.id = user_object.contains("User_ID") ? user_object["User_ID"].toInt() : user_object["id"].toInt();
    user.username = user_object.contains("Username") ? user_object["Username"].toString() : user_object["username"].toString();
    user.email = user_object.contains("Email") ? user_object["Email"].toString() : user_object["email"].toString();
    user.first_name = user_object.contains("First_Name") ? user_object["First_Name"].toString() : user_object["first_name"].toString();
    user.last_name = user_object.contains("Last_Name") ? user_object["Last_Name"].toString() : user_object["last_name"].toString();
    user.phone_number = user_object.contains("Phone") ? user_object["Phone"].toString() : user_object["phone_number"].toString();
    return user;
}

void Network_Manager::reset_connection()
{
    current_user = User_Info();
    set_connection_status(Connection_Status::Disconnected);
}

void Network_Manager::set_connection_status(Connection_Status status)
{
    connection_status = status;
    log_network_activity(Utils::Network::get_connection_status_string(static_cast<int>(status)), "Status");
}

// Utility functions implementation
void Network_Manager::log_network_activity(const QString& message, const QString& type)
{
    if (Config::Application::LOG_NETWORK_REQUESTS)
    {
        if (type == "Error")
            Utils::Logger::error(message, "Network");
        else if (type == "Warning")
            Utils::Logger::warning(message, "Network");
        else
            Utils::Logger::info(message, "Network");
    }
}

void Network_Manager::update_last_activity()
{
    last_activity = QDateTime::currentDateTime();
}

QString Network_Manager::format_json_message(const QJsonObject& message)
{
    return Utils::JSON::to_json_string(message, Config::JSON::PRETTY_PRINT);
}

bool Network_Manager::validate_server_response(const QJsonObject& response)
{
    if (!response.contains("success"))
    {
        Utils::Logger::error("Server response missing 'success' field", "Network");
        return false;
    }
    return true;
}

void Network_Manager::handle_connection_timeout()
{
    Utils::Logger::error(QString("Connection timeout after %1ms").arg(Config::Network::CONNECTION_TIMEOUT_MS), "Network");
    tcp_socket->abort();
    set_connection_status(Connection_Status::Error);
    emit connection_error(Config::ErrorMessages::CONNECTION_TIMEOUT);
}

void Network_Manager::handle_network_error(const QString& error_message)
{
    Utils::Logger::error(Utils::Network::format_network_error(error_message), "Network");
    emit connection_error(error_message);
}

// Qt TCP implementation - all socket operations are now handled by QTcpSocket