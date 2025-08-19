#include "models/Destination_Model.h"
#include "network/Api_Client.h"
#include "config.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <algorithm>

Destination_Model::Destination_Model(QObject* parent)
    : QAbstractListModel(parent)
    , m_refresh_timer(std::make_unique<QTimer>(this))
    , m_is_loading(false)
{
    connect_api_signals();
    
    // Setup auto-refresh timer (optional)
    m_refresh_timer->setSingleShot(true);
    connect(m_refresh_timer.get(), &QTimer::timeout,
            this, &Destination_Model::refresh_destinations);
}

Destination_Model::~Destination_Model() = default;

int Destination_Model::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_destinations.size();
}

QVariant Destination_Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_destinations.size())
    {
        return QVariant();
    }

    const Destination& destination = m_destinations.at(index.row());

    switch (role)
    {
        case Id_Role:
            return destination.id;
        case Name_Role:
        case Qt::DisplayRole:
            return destination.name;
        case Country_Role:
            return destination.country;
        case Description_Role:
            return destination.description;
        case Image_Path_Role:
            return destination.image_path;
        case Created_At_Role:
            return destination.created_at;
        case Modified_At_Role:
            return destination.modified_at;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> Destination_Model::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Id_Role] = "id";
    roles[Name_Role] = "name";
    roles[Country_Role] = "country";
    roles[Description_Role] = "description";
    roles[Image_Path_Role] = "image_path";
    roles[Created_At_Role] = "created_at";
    roles[Modified_At_Role] = "modified_at";
    return roles;
}

void Destination_Model::refresh_destinations()
{
    if (m_is_loading)
    {
        return;
    }
    
    m_is_loading = true;
    qDebug() << "Destination_Model: Refreshing destinations...";
    
    Api_Client::instance().get_destinations();
}

void Destination_Model::clear_destinations()
{
    beginResetModel();
    m_destinations.clear();
    endResetModel();
    
    emit destinations_cleared();
}

const QVector<Destination_Model::Destination>& Destination_Model::get_destinations() const
{
    return m_destinations;
}

Destination_Model::Destination Destination_Model::get_destination(int index) const
{
    if (index >= 0 && index < m_destinations.size())
    {
        return m_destinations.at(index);
    }
    return Destination();
}

Destination_Model::Destination Destination_Model::get_destination_by_id(int id) const
{
    auto it = std::find_if(m_destinations.begin(), m_destinations.end(),
                          [id](const Destination& dest) { return dest.id == id; });
    
    if (it != m_destinations.end())
    {
        return *it;
    }
    return Destination();
}

int Destination_Model::get_destination_count() const
{
    return m_destinations.size();
}

QVector<Destination_Model::Destination> Destination_Model::search_destinations(const QString& search_text) const
{
    QVector<Destination> results;
    
    if (search_text.isEmpty())
    {
        return m_destinations;
    }
    
    QString lowerSearchText = search_text.toLower();
    
    for (const auto& destination : m_destinations)
    {
        if (destination.name.toLower().contains(lowerSearchText) ||
            destination.country.toLower().contains(lowerSearchText) ||
            destination.description.toLower().contains(lowerSearchText)) {
            results.append(destination);
        }
    }
    
    return results;
}

QVector<Destination_Model::Destination> Destination_Model::get_destinations_by_country(const QString& country) const
{
    QVector<Destination> results;
    
    for (const auto& destination : m_destinations)
    {
        if (destination.country == country)
        {
            results.append(destination);
        }
    }
    
    return results;
}

QStringList Destination_Model::get_unique_countries() const
{
    QStringList countries;
    
    for (const auto& destination : m_destinations)
    {
        if (!countries.contains(destination.country))
        {
            countries.append(destination.country);
        }
    }
    
    countries.sort();
    return countries;
}

void Destination_Model::connect_api_signals()
{
    Api_Client& api = Api_Client::instance();
    
    connect(&api, &Api_Client::destinations_received,
            this, &Destination_Model::on_destinations_received);
    connect(&api, &Api_Client::network_error,
            this, &Destination_Model::on_network_error);
}

void Destination_Model::on_destinations_received(const QJsonArray& destinations)
{
    qDebug() << "Destination_Model: Received" << destinations.size() << "destinations";
    
    beginResetModel();
    m_destinations.clear();
    
    for (const auto& value : destinations)
    {
        if (value.isObject())
        {
            Destination dest = destination_from_json(value.toObject());
            if (dest.id > 0)  // Valid destination
            {
                m_destinations.append(dest);
            }
        }
    }
    
    endResetModel();
    
    m_is_loading = false;
    emit destinations_loaded();
    emit data_refreshed();
}

void Destination_Model::on_network_error(const QString& error_message)
{
    m_is_loading = false;
    qWarning() << "Destination_Model: Network error:" << error_message;
    emit error_occurred(error_message);
}

void Destination_Model::add_destination(const Destination& destination)
{
    beginInsertRows(QModelIndex(), m_destinations.size(), m_destinations.size());
    m_destinations.append(destination);
    endInsertRows();
}

void Destination_Model::update_destination(const Destination& destination)
{
    for (int i = 0; i < m_destinations.size(); ++i)
    {
        if (m_destinations[i].id == destination.id)
        {
            m_destinations[i] = destination;
            QModelIndex index = createIndex(i, 0);
            emit dataChanged(index, index);
            break;
        }
    }
}

Destination_Model::Destination Destination_Model::destination_from_json(const QJsonObject& jsonObj) const
{
    Destination dest;
    
    dest.id = jsonObj["Destination_ID"].toString().toInt();
    dest.name = jsonObj["Name"].toString();
    dest.country = jsonObj["Country"].toString();
    dest.description = jsonObj["Description"].toString();
    dest.image_path = jsonObj["Image_Path"].toString();
    dest.created_at = jsonObj["Date_Created"].toString();
    dest.modified_at = jsonObj["Date_Modified"].toString();
    
    return dest;
}

