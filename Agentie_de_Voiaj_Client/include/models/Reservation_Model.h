#pragma once
#include <QAbstractListModel>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QSettings>

class Api_Client;

class Reservation_Model : public QAbstractListModel
{
    Q_OBJECT

public:
    struct Reservation
    {
        int id = 0;
        int user_id = 0;
        int offer_id = 0;
        QString offer_name;
        QString destination;
        int person_count = 0;
        double total_price = 0.0;
        QDateTime reservation_date;
        QDateTime travel_start_date;
        QDateTime travel_end_date;
        QString status;
        QString special_requests;
        QString created_at;
        QString modified_at;
    };

    enum Roles
    {
        Id_Role = Qt::UserRole + 1,
        User_Id_Role,
        Offer_Id_Role,
        Offer_Name_Role,
        Destination_Role,
        Person_Count_Role,
        Total_Price_Role,
        Reservation_Date_Role,
        Travel_Start_Date_Role,
        Travel_End_Date_Role,
        Status_Role,
        Special_Requests_Role,
        Created_At_Role,
        Modified_At_Role
    };

    explicit Reservation_Model(QObject* parent = nullptr);
    ~Reservation_Model();

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Public methods
    void refresh_reservations();
    void clear_reservations();
    
    const QVector<Reservation>& get_reservations() const;
    Reservation get_reservation(int index) const;
    Reservation get_reservation_by_id(int id) const;
    int get_reservation_count() const;
    
    QVector<Reservation> get_reservations_by_status(const QString& status) const;
    QVector<Reservation> get_active_reservations() const;
    QVector<Reservation> get_past_reservations() const;
    QVector<Reservation> get_upcoming_reservations() const;

    // Reservation management
    void cancel_reservation(int reservation_id);
    void update_reservation_status(int reservation_id, const QString& new_status);

    // Getters
    bool is_loading() const { return m_is_loading; }
    QString get_last_error() const { return m_last_error; }

signals:
    void reservations_loaded();
    void reservations_cleared();
    void data_refreshed();
    void error_occurred(const QString& error_message);
    void cancellation_success(const QString& message);
    void cancellation_failed(const QString& error_message);
    void status_updated(int reservation_id, const QString& new_status);

private slots:
    void on_reservations_received(const QJsonArray& reservations);
    void on_cancellation_success(const QString& message);
    void on_cancellation_failed(const QString& error_message);
    void on_network_error(const QString& error_message);

private:
    void connect_api_signals();
    Reservation reservation_from_json(const QJsonObject& jsonObj) const;
    void set_loading(bool loading);
    void set_error(const QString& error);
    void save_cached_reservations();
    void load_cached_reservations();

    QVector<Reservation> m_reservations;
    bool m_is_loading = false;
    QString m_last_error;
    QSettings* m_settings = nullptr;
};
