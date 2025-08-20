#pragma once
#include <QAbstractListModel>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QSettings>

class Api_Client;

class Offer_Model : public QAbstractListModel
{
    Q_OBJECT

public:
    struct Offer
    {
        int id = 0;
        QString name;
        QString destination;
        double price_per_person = 0.0;
        int duration_days = 0;
        int available_seats = 0;
        QString description;
        QString image_path;
        QString status;
        QDateTime start_date;
        QDateTime end_date;
        QString created_at;
        QString modified_at;
    };

    enum Roles
    {
        Id_Role = Qt::UserRole + 1,
        Name_Role,
        Destination_Role,
        Price_Role,
        Duration_Role,
        Available_Seats_Role,
        Description_Role,
        Image_Role,
        Status_Role,
        Start_Date_Role,
        End_Date_Role,
        Created_At_Role,
        Modified_At_Role
    };

    explicit Offer_Model(QObject* parent = nullptr);
    ~Offer_Model();

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Public methods
    void refresh_offers();
    void search_offers(const QString& destination = "", double min_price = 0.0, double max_price = 0.0);
    void clear_offers();
    
    const QVector<Offer>& get_offers() const;
    Offer get_offer(int index) const;
    Offer get_offer_by_id(int id) const;
    int get_offer_count() const;
    
    QVector<Offer> get_offers_by_destination(const QString& destination) const;
    QVector<Offer> get_offers_by_price_range(double min_price, double max_price) const;
    QVector<Offer> get_available_offers() const;

    // Booking functionality
    void book_offer(int offer_id, int person_count, const QJsonObject& additional_info = QJsonObject());
    void cancel_reservation(int reservation_id);

    // Getters
    bool is_loading() const { return m_is_loading; }
    QString get_last_error() const { return m_last_error; }

signals:
    void offers_loaded();
    void offers_cleared();
    void data_refreshed();
    void error_occurred(const QString& error_message);
    void booking_success(const QString& message);
    void booking_failed(const QString& error_message);
    void cancellation_success(const QString& message);
    void cancellation_failed(const QString& error_message);

private slots:
    void on_offers_received(const QJsonArray& offers);
    void on_booking_success(const QString& message);
    void on_booking_failed(const QString& error_message);
    void on_cancellation_success(const QString& message);
    void on_cancellation_failed(const QString& error_message);
    void on_network_error(const QString& error_message);

private:
    void connect_api_signals();
    Offer offer_from_json(const QJsonObject& jsonObj) const;
    void set_loading(bool loading);
    void set_error(const QString& error);
    void load_cached_offers();
    void save_cached_offers();

    QVector<Offer> m_offers;
    bool m_is_loading = false;
    QString m_last_error;
    QSettings* m_settings = nullptr;
};

