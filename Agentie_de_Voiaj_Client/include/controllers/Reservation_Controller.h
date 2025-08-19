#pragma once
#include <QObject>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>
#include <memory>
#include "models/Reservation_Data.h"
#include "models/Booking_Data.h"

class User_Model;
class Api_Client;

class Reservation_Controller : public QObject
{
    Q_OBJECT

public:
    explicit Reservation_Controller(QObject* parent = nullptr);
    ~Reservation_Controller();

    void load_user_reservations(int user_id);
    void create_reservation(const Booking_Data& reservation);
    void cancel_reservation(int reservation_id);
    void update_reservation_status(int reservation_id, const QString& status);

    const QVector<Reservation_Data>& get_reservations() const;
    Reservation_Data get_reservation_by_id(int id) const;
    int get_reservation_count() const;

    double get_total_spent() const;
    int get_total_reservations_count() const;
    QVector<Reservation_Data> get_reservations_by_status(const QString& status) const;

signals:
    void reservations_loaded();
    void reservation_created(int reservation_id);
    void reservation_cancelled(int reservation_id);
    void reservation_updated(int reservation_id);
    void error_occurred(const QString& error);

private slots:
    void on_reservations_received(const QJsonArray& reservations);
    void on_booking_success(const QString& message);
    void on_booking_failed(const QString& error);
    void on_cancellation_success(const QString& message);
    void on_cancellation_failed(const QString& error);

private:
    void connect_api_signals();
    Reservation_Data reservation_from_json(const QJsonObject& json) const;

    QVector<Reservation_Data> m_reservations;
    std::shared_ptr<User_Model> m_user_model;
    bool m_is_loading;
};