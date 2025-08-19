#pragma once
#include <QAbstractListModel>
#include <QTimer>
#include <memory>

class Destination_Model : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Destination_Roles
    {
        Id_Role = Qt::UserRole + 1,
        Name_Role,
        Country_Role,
        Description_Role,
        Image_Path_Role,
        Created_At_Role,
        Modified_At_Role
    };

    struct Destination
    {
        int id = 0;
        QString name;
        QString country;
        QString description;
        QString image_path;
        QString created_at;
        QString modified_at;
    };

    explicit Destination_Model(QObject *parent = nullptr);
    ~Destination_Model();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void refresh_destinations();
    void clear_destinations();

    const QVector<Destination>& get_destinations() const;
    Destination get_destination(int index) const;
    Destination get_destination_by_id(int id) const;
    int get_destination_count() const;

    QVector<Destination> search_destinations(const QString& search_text) const;
    QVector<Destination> get_destinations_by_country(const QString& country) const;
    QStringList get_unique_countries() const;

signals:
    void destinations_loaded();
    void destinations_cleared();
    void data_refreshed();
    void error_occurred(const QString& error_message);

private slots:
    void on_destinations_received(const QJsonArray& destinations);
    void on_network_error(const QString& error_message);

private:
    void connect_api_signals();
    void add_destination(const Destination& destination);
    void update_destination(const Destination& destination);
    Destination destination_from_json(const QJsonObject& json) const;

    QVector<Destination> m_destinations;
    std::unique_ptr<QTimer> m_refresh_timer;
    bool m_is_loading = false;
};