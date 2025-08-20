#include "models/Reservation_Model.h"
#include "network/Api_Client.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

Reservation_Model::Reservation_Model(QObject* parent)
    : QAbstractListModel(parent)
    , m_settings(new QSettings(QSettings::IniFormat, QSettings::UserScope, "Agentie_de_Voiaj", "Reservation_Model", this))
{
    connect_api_signals();
    
    // Load cached reservations if available
    load_cached_reservations();
}

Reservation_Model::~Reservation_Model()
{
    // Save current reservations to cache
    save_cached_reservations();
}

int Reservation_Model::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    
    return m_reservations.size();
}

QVariant Reservation_Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_reservations.size())
        return QVariant();
    
    const Reservation& reservation = m_reservations[index.row()];
    
    switch (role)
    {
        case Id_Role:
            return reservation.id;
        case User_Id_Role:
            return reservation.user_id;
        case Offer_Id_Role:
            return reservation.offer_id;
        case Offer_Name_Role:
            return reservation.offer_name;
        case Destination_Role:
            return reservation.destination;
        case Person_Count_Role:
            return reservation.person_count;
        case Total_Price_Role:
            return reservation.total_price;
        case Reservation_Date_Role:
            return reservation.reservation_date;
        case Travel_Start_Date_Role:
            return reservation.travel_start_date;
        case Travel_End_Date_Role:
            return reservation.travel_end_date;
        case Status_Role:
            return reservation.status;
        case Special_Requests_Role:
            return reservation.special_requests;
        case Created_At_Role:
            return reservation.created_at;
        case Modified_At_Role:
            return reservation.modified_at;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> Reservation_Model::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Id_Role] = "id";
    roles[User_Id_Role] = "user_id";
    roles[Offer_Id_Role] = "offer_id";
    roles[Offer_Name_Role] = "offer_name";
    roles[Destination_Role] = "destination";
    roles[Person_Count_Role] = "person_count";
    roles[Total_Price_Role] = "total_price";
    roles[Reservation_Date_Role] = "reservation_date";
    roles[Travel_Start_Date_Role] = "travel_start_date";
    roles[Travel_End_Date_Role] = "travel_end_date";
    roles[Status_Role] = "status";
    roles[Special_Requests_Role] = "special_requests";
    roles[Created_At_Role] = "created_at";
    roles[Modified_At_Role] = "modified_at";
    return roles;
}

void Reservation_Model::refresh_reservations()
{
    if (m_is_loading)
        return;
    
    set_loading(true);
    qDebug() << "Reservation_Model: Refreshing reservations...";
    
    Api_Client::instance().get_user_reservations();
}

void Reservation_Model::clear_reservations()
{
    beginResetModel();
    m_reservations.clear();
    endResetModel();
    
    emit reservations_cleared();
}

const QVector<Reservation_Model::Reservation>& Reservation_Model::get_reservations() const
{
    return m_reservations;
}

Reservation_Model::Reservation Reservation_Model::get_reservation(int index) const
{
    if (index >= 0 && index < m_reservations.size())
        return m_reservations.at(index);
    return Reservation();
}

Reservation_Model::Reservation Reservation_Model::get_reservation_by_id(int id) const
{
    auto it = std::find_if(m_reservations.begin(), m_reservations.end(),
                          [id](const Reservation& reservation) { return reservation.id == id; });
    
    if (it != m_reservations.end())
        return *it;
    return Reservation();
}

int Reservation_Model::get_reservation_count() const
{
    return m_reservations.size();
}

QVector<Reservation_Model::Reservation> Reservation_Model::get_reservations_by_status(const QString& status) const
{
    QVector<Reservation> results;
    
    if (status.isEmpty())
        return m_reservations;
    
    for (const auto& reservation : m_reservations)
    {
        if (reservation.status == status)
            results.append(reservation);
    }
    
    return results;
}

QVector<Reservation_Model::Reservation> Reservation_Model::get_active_reservations() const
{
    QVector<Reservation> results;
    QDateTime now = QDateTime::currentDateTime();
    
    for (const auto& reservation : m_reservations)
    {
        if (reservation.status == "Confirmed" && 
            reservation.travel_end_date > now)
        {
            results.append(reservation);
        }
    }
    
    return results;
}

QVector<Reservation_Model::Reservation> Reservation_Model::get_past_reservations() const
{
    QVector<Reservation> results;
    QDateTime now = QDateTime::currentDateTime();
    
    for (const auto& reservation : m_reservations)
    {
        if (reservation.travel_end_date < now)
        {
            results.append(reservation);
        }
    }
    
    return results;
}

QVector<Reservation_Model::Reservation> Reservation_Model::get_upcoming_reservations() const
{
    QVector<Reservation> results;
    QDateTime now = QDateTime::currentDateTime();
    
    for (const auto& reservation : m_reservations)
    {
        if (reservation.status == "Confirmed" && 
            reservation.travel_start_date > now)
        {
            results.append(reservation);
        }
    }
    
    return results;
}

void Reservation_Model::cancel_reservation(int reservation_id)
{
    qDebug() << "Reservation_Model: Cancelling reservation" << reservation_id;
    Api_Client::instance().cancel_reservation(reservation_id);
}

void Reservation_Model::update_reservation_status(int reservation_id, const QString& new_status)
{
    // This would typically call an API endpoint to update status
    // For now, we'll just emit the signal
    emit status_updated(reservation_id, new_status);
}

void Reservation_Model::connect_api_signals()
{
    Api_Client& api = Api_Client::instance();
    
    connect(&api, &Api_Client::reservations_received,
            this, &Reservation_Model::on_reservations_received);
    connect(&api, &Api_Client::cancellation_success,
            this, &Reservation_Model::on_cancellation_success);
    connect(&api, &Api_Client::cancellation_failed,
            this, &Reservation_Model::on_cancellation_failed);
    connect(&api, &Api_Client::network_error,
            this, &Reservation_Model::on_network_error);
}

void Reservation_Model::on_reservations_received(const QJsonArray& reservations)
{
    qDebug() << "Reservation_Model: Received" << reservations.size() << "reservations";
    
    beginResetModel();
    m_reservations.clear();
    
    for (const auto& value : reservations)
    {
        if (value.isObject())
        {
            Reservation reservation = reservation_from_json(value.toObject());
            if (reservation.id > 0)  // Valid reservation
            {
                m_reservations.append(reservation);
            }
        }
    }
    
    endResetModel();
    
    set_loading(false);
    emit reservations_loaded();
    emit data_refreshed();
    
    // Save to cache
    save_cached_reservations();
}

void Reservation_Model::on_cancellation_success(const QString& message)
{
    qDebug() << "Reservation_Model: Cancellation successful:" << message;
    emit cancellation_success(message);
    
    // Refresh reservations to get updated data
    refresh_reservations();
}

void Reservation_Model::on_cancellation_failed(const QString& error_message)
{
    qDebug() << "Reservation_Model: Cancellation failed:" << error_message;
    set_error(error_message);
    emit cancellation_failed(error_message);
}

void Reservation_Model::on_network_error(const QString& error_message)
{
    set_loading(false);
    qWarning() << "Reservation_Model: Network error:" << error_message;
    set_error(error_message);
    emit error_occurred(error_message);
}

Reservation_Model::Reservation Reservation_Model::reservation_from_json(const QJsonObject& jsonObj) const
{
    Reservation reservation;
    
    reservation.id = jsonObj["Reservation_ID"].toString().toInt();
    reservation.user_id = jsonObj["User_ID"].toString().toInt();
    reservation.offer_id = jsonObj["Offer_ID"].toString().toInt();
    reservation.offer_name = jsonObj["Offer_Name"].toString();
    reservation.destination = jsonObj["Destination"].toString();
    reservation.person_count = jsonObj["Person_Count"].toString().toInt();
    reservation.total_price = jsonObj["Total_Price"].toString().toDouble();
    
    // Parse dates if available
    if (jsonObj.contains("Reservation_Date"))
        reservation.reservation_date = QDateTime::fromString(jsonObj["Reservation_Date"].toString(), Qt::ISODate);
    if (jsonObj.contains("Travel_Start_Date"))
        reservation.travel_start_date = QDateTime::fromString(jsonObj["Travel_Start_Date"].toString(), Qt::ISODate);
    if (jsonObj.contains("Travel_End_Date"))
        reservation.travel_end_date = QDateTime::fromString(jsonObj["Travel_End_Date"].toString(), Qt::ISODate);
    
    reservation.status = jsonObj["Status"].toString();
    reservation.special_requests = jsonObj["Special_Requests"].toString();
    reservation.created_at = jsonObj["Date_Created"].toString();
    reservation.modified_at = jsonObj["Date_Modified"].toString();
    
    return reservation;
}

void Reservation_Model::set_loading(bool loading)
{
    if (m_is_loading != loading)
    {
        m_is_loading = loading;
        // Emit loading state change if needed
    }
}

void Reservation_Model::set_error(const QString& error)
{
    m_last_error = error;
}

void Reservation_Model::save_cached_reservations()
{
    if (m_reservations.isEmpty())
        return;
    
    QJsonArray reservations_array;
    for (const auto& reservation : m_reservations)
    {
        QJsonObject reservation_obj;
        reservation_obj["Reservation_ID"] = reservation.id;
        reservation_obj["User_ID"] = reservation.user_id;
        reservation_obj["Offer_ID"] = reservation.offer_id;
        reservation_obj["Offer_Name"] = reservation.offer_name;
        reservation_obj["Destination"] = reservation.destination;
        reservation_obj["Person_Count"] = reservation.person_count;
        reservation_obj["Total_Price"] = reservation.total_price;
        reservation_obj["Reservation_Date"] = reservation.reservation_date.toString(Qt::ISODate);
        reservation_obj["Travel_Start_Date"] = reservation.travel_start_date.toString(Qt::ISODate);
        reservation_obj["Travel_End_Date"] = reservation.travel_end_date.toString(Qt::ISODate);
        reservation_obj["Status"] = reservation.status;
        reservation_obj["Special_Requests"] = reservation.special_requests;
        reservation_obj["Date_Created"] = reservation.created_at;
        reservation_obj["Date_Modified"] = reservation.modified_at;
        
        reservations_array.append(reservation_obj);
    }
    
    m_settings->setValue("cached_reservations", reservations_array);
    m_settings->setValue("cache_timestamp", QDateTime::currentDateTime().toString(Qt::ISODate));
}

void Reservation_Model::load_cached_reservations()
{
    if (!m_settings->contains("cached_reservations"))
        return;
    
    QJsonArray reservations_array = m_settings->value("cached_reservations").toJsonArray();
    if (reservations_array.isEmpty())
        return;
    
    // Check if cache is not too old (24 hours)
    QString cache_timestamp = m_settings->value("cache_timestamp").toString();
    QDateTime cache_time = QDateTime::fromString(cache_timestamp, Qt::ISODate);
    if (cache_time.addDays(1) < QDateTime::currentDateTime())
        return;
    
    beginResetModel();
    m_reservations.clear();
    
    for (const auto& value : reservations_array)
    {
        if (value.isObject())
        {
            Reservation reservation = reservation_from_json(value.toObject());
            if (reservation.id > 0)
            {
                m_reservations.append(reservation);
            }
        }
    }
    
    endResetModel();
    
    qDebug() << "Reservation_Model: Loaded" << m_reservations.size() << "cached reservations";
}

