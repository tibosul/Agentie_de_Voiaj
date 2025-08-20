#include "models/Offer_Model.h"
#include "network/Api_Client.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

Offer_Model::Offer_Model(QObject* parent)
    : QAbstractListModel(parent)
    , m_settings(new QSettings(QSettings::IniFormat, QSettings::UserScope, "Agentie_de_Voiaj", "Offer_Model", this))
{
    connect_api_signals();
    
    // Load cached offers if available
    load_cached_offers();
}

Offer_Model::~Offer_Model()
{
    // Save current offers to cache
    save_cached_offers();
}

int Offer_Model::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    
    return m_offers.size();
}

QVariant Offer_Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_offers.size())
        return QVariant();
    
    const Offer& offer = m_offers[index.row()];
    
    switch (role)
    {
        case Id_Role:
            return offer.id;
        case Name_Role:
            return offer.name;
        case Destination_Role:
            return offer.destination;
        case Price_Role:
            return offer.price_per_person;
        case Duration_Role:
            return offer.duration_days;
        case Available_Seats_Role:
            return offer.available_seats;
        case Description_Role:
            return offer.description;
        case Image_Role:
            return offer.image_path;
        case Status_Role:
            return offer.status;
        case Start_Date_Role:
            return offer.start_date;
        case End_Date_Role:
            return offer.end_date;
        case Created_At_Role:
            return offer.created_at;
        case Modified_At_Role:
            return offer.modified_at;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> Offer_Model::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Id_Role] = "id";
    roles[Name_Role] = "name";
    roles[Destination_Role] = "destination";
    roles[Price_Role] = "price";
    roles[Duration_Role] = "duration";
    roles[Available_Seats_Role] = "available_seats";
    roles[Description_Role] = "description";
    roles[Image_Role] = "image";
    roles[Status_Role] = "status";
    roles[Start_Date_Role] = "start_date";
    roles[End_Date_Role] = "end_date";
    roles[Created_At_Role] = "created_at";
    roles[Modified_At_Role] = "modified_at";
    return roles;
}

void Offer_Model::refresh_offers()
{
    if (m_is_loading)
        return;
    
    set_loading(true);
    qDebug() << "Offer_Model: Refreshing offers...";
    
    Api_Client::instance().get_offers();
}

void Offer_Model::search_offers(const QString& destination, double min_price, double max_price)
{
    if (m_is_loading)
        return;
    
    set_loading(true);
    qDebug() << "Offer_Model: Searching offers...";
    
    QJsonObject search_params;
    if (!destination.isEmpty())
        search_params["destination"] = destination;
    if (min_price > 0.0)
        search_params["min_price"] = min_price;
    if (max_price > 0.0)
        search_params["max_price"] = max_price;
    
    Api_Client::instance().search_offers(search_params);
}

void Offer_Model::clear_offers()
{
    beginResetModel();
    m_offers.clear();
    endResetModel();
    
    emit offers_cleared();
}

const QVector<Offer_Model::Offer>& Offer_Model::get_offers() const
{
    return m_offers;
}

Offer_Model::Offer Offer_Model::get_offer(int index) const
{
    if (index >= 0 && index < m_offers.size())
        return m_offers.at(index);
    return Offer();
}

Offer_Model::Offer Offer_Model::get_offer_by_id(int id) const
{
    auto it = std::find_if(m_offers.begin(), m_offers.end(),
                          [id](const Offer& offer) { return offer.id == id; });
    
    if (it != m_offers.end())
        return *it;
    return Offer();
}

int Offer_Model::get_offer_count() const
{
    return m_offers.size();
}

QVector<Offer_Model::Offer> Offer_Model::get_offers_by_destination(const QString& destination) const
{
    QVector<Offer> results;
    
    if (destination.isEmpty())
        return m_offers;
    
    for (const auto& offer : m_offers)
    {
        if (offer.destination.contains(destination, Qt::CaseInsensitive))
            results.append(offer);
    }
    
    return results;
}

QVector<Offer_Model::Offer> Offer_Model::get_offers_by_price_range(double min_price, double max_price) const
{
    QVector<Offer> results;
    
    for (const auto& offer : m_offers)
    {
        if (offer.price_per_person >= min_price && offer.price_per_person <= max_price)
            results.append(offer);
    }
    
    return results;
}

QVector<Offer_Model::Offer> Offer_Model::get_available_offers() const
{
    QVector<Offer> results;
    
    for (const auto& offer : m_offers)
    {
        if (offer.available_seats > 0 && offer.status == "Available")
            results.append(offer);
    }
    
    return results;
}

void Offer_Model::book_offer(int offer_id, int person_count, const QJsonObject& additional_info)
{
    qDebug() << "Offer_Model: Booking offer" << offer_id << "for" << person_count << "persons";
    
    QJsonObject booking_data = additional_info;
    booking_data["offer_id"] = offer_id;
    booking_data["person_count"] = person_count;
    
    Api_Client::instance().book_offer(offer_id, person_count, booking_data);
}

void Offer_Model::cancel_reservation(int reservation_id)
{
    qDebug() << "Offer_Model: Cancelling reservation" << reservation_id;
    Api_Client::instance().cancel_reservation(reservation_id);
}

void Offer_Model::connect_api_signals()
{
    Api_Client& api = Api_Client::instance();
    
    connect(&api, &Api_Client::offers_received,
            this, &Offer_Model::on_offers_received);
    connect(&api, &Api_Client::booking_success,
            this, &Offer_Model::on_booking_success);
    connect(&api, &Api_Client::booking_failed,
            this, &Offer_Model::on_booking_failed);
    connect(&api, &Api_Client::cancellation_success,
            this, &Offer_Model::on_cancellation_success);
    connect(&api, &Api_Client::cancellation_failed,
            this, &Offer_Model::on_cancellation_failed);
    connect(&api, &Api_Client::network_error,
            this, &Offer_Model::on_network_error);
}

void Offer_Model::on_offers_received(const QJsonArray& offers)
{
    qDebug() << "Offer_Model: Received" << offers.size() << "offers";
    
    beginResetModel();
    m_offers.clear();
    
    for (const auto& value : offers)
    {
        if (value.isObject())
        {
            Offer offer = offer_from_json(value.toObject());
            if (offer.id > 0)  // Valid offer
            {
                m_offers.append(offer);
            }
        }
    }
    
    endResetModel();
    
    set_loading(false);
    emit offers_loaded();
    emit data_refreshed();
    
    // Save to cache
    save_cached_offers();
}

void Offer_Model::on_booking_success(const QString& message)
{
    qDebug() << "Offer_Model: Booking successful:" << message;
    emit booking_success(message);
}

void Offer_Model::on_booking_failed(const QString& error_message)
{
    qDebug() << "Offer_Model: Booking failed:" << error_message;
    set_error(error_message);
    emit booking_failed(error_message);
}

void Offer_Model::on_cancellation_success(const QString& message)
{
    qDebug() << "Offer_Model: Cancellation successful:" << message;
    emit cancellation_success(message);
}

void Offer_Model::on_cancellation_failed(const QString& error_message)
{
    qDebug() << "Offer_Model: Cancellation failed:" << error_message;
    set_error(error_message);
    emit cancellation_failed(error_message);
}

void Offer_Model::on_network_error(const QString& error_message)
{
    set_loading(false);
    qWarning() << "Offer_Model: Network error:" << error_message;
    set_error(error_message);
    emit error_occurred(error_message);
}

Offer_Model::Offer Offer_Model::offer_from_json(const QJsonObject& jsonObj) const
{
    Offer offer;
    
    offer.id = jsonObj["Offer_ID"].toString().toInt();
    offer.name = jsonObj["Name"].toString();
    offer.destination = jsonObj["Destination"].toString();
    offer.price_per_person = jsonObj["Price_per_Person"].toString().toDouble();
    offer.duration_days = jsonObj["Duration_Days"].toString().toInt();
    offer.available_seats = jsonObj["Available_Seats"].toString().toInt();
    offer.description = jsonObj["Description"].toString();
    offer.image_path = jsonObj["Image_Path"].toString();
    offer.status = jsonObj["Status"].toString();
    
    // Parse dates if available
    if (jsonObj.contains("Start_Date"))
        offer.start_date = QDateTime::fromString(jsonObj["Start_Date"].toString(), Qt::ISODate);
    if (jsonObj.contains("End_Date"))
        offer.end_date = QDateTime::fromString(jsonObj["End_Date"].toString(), Qt::ISODate);
    
    offer.created_at = jsonObj["Date_Created"].toString();
    offer.modified_at = jsonObj["Date_Modified"].toString();
    
    return offer;
}

void Offer_Model::set_loading(bool loading)
{
    if (m_is_loading != loading)
    {
        m_is_loading = loading;
        // Emit loading state change if needed
    }
}

void Offer_Model::set_error(const QString& error)
{
    m_last_error = error;
}

void Offer_Model::save_cached_offers()
{
    if (m_offers.isEmpty())
        return;
    
    QJsonArray offers_array;
    for (const auto& offer : m_offers)
    {
        QJsonObject offer_obj;
        offer_obj["Offer_ID"] = offer.id;
        offer_obj["Name"] = offer.name;
        offer_obj["Destination"] = offer.destination;
        offer_obj["Price_per_Person"] = offer.price_per_person;
        offer_obj["Duration_Days"] = offer.duration_days;
        offer_obj["Available_Seats"] = offer.available_seats;
        offer_obj["Description"] = offer.description;
        offer_obj["Image_Path"] = offer.image_path;
        offer_obj["Status"] = offer.status;
        offer_obj["Date_Created"] = offer.created_at;
        offer_obj["Date_Modified"] = offer.modified_at;
        
        offers_array.append(offer_obj);
    }
    
    m_settings->setValue("cached_offers", offers_array);
    m_settings->setValue("cache_timestamp", QDateTime::currentDateTime().toString(Qt::ISODate));
}

void Offer_Model::load_cached_offers()
{
    if (!m_settings->contains("cached_offers"))
        return;
    
    QJsonArray offers_array = m_settings->value("cached_offers").toJsonArray();
    if (offers_array.isEmpty())
        return;
    
    // Check if cache is not too old (24 hours)
    QString cache_timestamp = m_settings->value("cache_timestamp").toString();
    QDateTime cache_time = QDateTime::fromString(cache_timestamp, Qt::ISODate);
    if (cache_time.addDays(1) < QDateTime::currentDateTime())
        return;
    
    beginResetModel();
    m_offers.clear();
    
    for (const auto& value : offers_array)
    {
        if (value.isObject())
        {
            Offer offer = offer_from_json(value.toObject());
            if (offer.id > 0)
            {
                m_offers.append(offer);
            }
        }
    }
    
    endResetModel();
    
    qDebug() << "Offer_Model: Loaded" << m_offers.size() << "cached offers";
}

