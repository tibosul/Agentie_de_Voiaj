#include "controllers/Reservation_Controller.h"
#include "models/User_Model.h"
#include "network/Api_Client.h"
#include "ui/Booking_Dialog.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QTimer>
#include <algorithm>

Reservation_Controller::Reservation_Controller(QObject* parent)
    : QObject(parent)
    , m_user_model(std::make_shared<User_Model>(this))
    , m_is_loading(false)
{
    connect_api_signals();
}

Reservation_Controller::~Reservation_Controller() = default;

void Reservation_Controller::load_user_reservations(int user_id)
{
    if (m_is_loading) {
        return;
    }
    
    m_is_loading = true;
    qDebug() << "Reservation_Controller: Loading reservations for user" << user_id;
    
    // In real implementation, this would call:
    // Api_Client::instance().get_user_reservations();
    
    // For now, simulate with mock data
    QTimer::singleShot(1000, [this]() {
        QJsonArray mockReservations;
        
        // Mock reservation 1
        QJsonObject res1;
        res1["Reservation_ID"] = "1";
        res1["User_ID"] = "1";
        res1["Offer_ID"] = "1";
        res1["Offer_Name"] = "Paris Weekend";
        res1["Destination_Name"] = "Paris";
        res1["Number_of_Persons"] = "2";
        res1["Total_Price"] = "598.00";
        res1["Reservation_Date"] = "2025-08-15";
        res1["Status"] = "confirmed";
        res1["Notes"] = "Anniversary trip";
        mockReservations.append(res1);
        
        // Mock reservation 2
        QJsonObject res2;
        res2["Reservation_ID"] = "2";
        res2["User_ID"] = "1";
        res2["Offer_ID"] = "2";
        res2["Offer_Name"] = "Rome Adventure";
        res2["Destination_Name"] = "Rome";
        res2["Number_of_Persons"] = "1";
        res2["Total_Price"] = "449.00";
        res2["Reservation_Date"] = "2025-08-10";
        res2["Status"] = "pending";
        res2["Notes"] = "";
        mockReservations.append(res2);
        
        on_reservations_received(mockReservations);
    });
}

void Reservation_Controller::create_reservation(const Booking_Data& booking_data)
{
    qDebug() << "Reservation_Controller: Creating reservation for offer" << booking_data.offer_id;
    
    // In real implementation, call API:
    // Api_Client::instance().book_offer(booking_data.offer_id, booking_data.number_of_persons, /* additional data */);
    
    // Simulate success
    QTimer::singleShot(2000, [this]() {
        on_booking_success("Rezervarea a fost creat cu succes!");
    });
}

void Reservation_Controller::cancel_reservation(int reservation_id)
{
    qDebug() << "Reservation_Controller: Cancelling reservation" << reservation_id;
    
    // In real implementation, call API:
    // Api_Client::instance().cancel_reservation(reservation_id);
    
    // Simulate success
    QTimer::singleShot(1000, [this, reservation_id]() {
        // Update local data
        for (auto& reservation : m_reservations) {
            if (reservation.id == reservation_id) {
                reservation.status = "cancelled";
                break;
            }
        }
        
        on_cancellation_success("Rezervarea a fost anulat cu succes!");
        emit reservation_cancelled(reservation_id);
    });
}

void Reservation_Controller::update_reservation_status(int reservation_id, const QString& status)
{
    qDebug() << "Reservation_Controller: Updating reservation" << reservation_id << "status to" << status;
    
    // Update local data
    for (auto& reservation : m_reservations) {
        if (reservation.id == reservation_id) {
            reservation.status = status;
            emit reservation_updated(reservation_id);
            break;
        }
    }
}

const QVector<ReservationData>& Reservation_Controller::get_reservations() const
{
    return m_reservations;
}

ReservationData Reservation_Controller::get_reservation_by_id(int id) const
{
    auto it = std::find_if(m_reservations.begin(), m_reservations.end(),
                          [id](const ReservationData& res) { return res.id == id; });
    
    if (it != m_reservations.end()) {
        return *it;
    }
    return ReservationData();
}

int Reservation_Controller::get_reservation_count() const
{
    return m_reservations.size();
}

double Reservation_Controller::get_total_spent() const
{
    double total = 0.0;
    for (const auto& reservation : m_reservations) {
        if (reservation.status != "cancelled") {
            total += reservation.totalPrice;
        }
    }
    return total;
}

int Reservation_Controller::get_total_reservations_count() const
{
    return std::count_if(m_reservations.begin(), m_reservations.end(),
                        [](const ReservationData& res) {
                            return res.status == "confirmed" || res.status == "paid";
                        });
}

QVector<ReservationData> Reservation_Controller::get_reservations_by_status(const QString& status) const
{
    QVector<ReservationData> filtered;
    for (const auto& reservation : m_reservations) {
        if (reservation.status == status) {
            filtered.append(reservation);
        }
    }
    return filtered;
}

void Reservation_Controller::connect_api_signals()
{
    Api_Client& api = Api_Client::instance();
    
    connect(&api, &Api_Client::reservations_received,
            this, &Reservation_Controller::on_reservations_received);
    connect(&api, &Api_Client::booking_success,
            this, &Reservation_Controller::on_booking_success);
    connect(&api, &Api_Client::booking_failed,
            this, &Reservation_Controller::on_booking_failed);
    connect(&api, &Api_Client::cancellation_success,
            this, &Reservation_Controller::on_cancellation_success);
    connect(&api, &Api_Client::cancellation_failed,
            this, &Reservation_Controller::on_cancellation_failed);
}

void Reservation_Controller::on_reservations_received(const QJsonArray& reservations)
{
    qDebug() << "Reservation_Controller: Received" << reservations.size() << "reservations";
    
    m_reservations.clear();
    
    for (const auto& value : reservations) {
        if (value.isObject()) {
            ReservationData reservation = reservation_from_json(value.toObject());
            if (reservation.id > 0) {
                m_reservations.append(reservation);
            }
        }
    }
    
    m_is_loading = false;
    emit reservations_loaded();
}

void Reservation_Controller::on_booking_success(const QString& message)
{
    qDebug() << "Reservation_Controller: Booking success:" << message;
    emit reservation_created(0); // In real implementation, get actual ID from response
}

void Reservation_Controller::on_booking_failed(const QString& error)
{
    qDebug() << "Reservation_Controller: Booking failed:" << error;
    emit error_occurred("Rezervarea a euat: " + error);
}

void Reservation_Controller::on_cancellation_success(const QString& message)
{
    qDebug() << "Reservation_Controller: Cancellation success:" << message;
}

void Reservation_Controller::on_cancellation_failed(const QString& error)
{
    qDebug() << "Reservation_Controller: Cancellation failed:" << error;
    emit error_occurred("Anularea a euat: " + error);
}

ReservationData Reservation_Controller::reservation_from_json(const QJsonObject& jsonObj) const
{
    ReservationData reservation;
    
    reservation.id = jsonObj["Reservation_ID"].toString().toInt();
    reservation.userId = jsonObj["User_ID"].toString().toInt();
    reservation.offerId = jsonObj["Offer_ID"].toString().toInt();
    reservation.offerName = jsonObj["Offer_Name"].toString();
    reservation.destination = jsonObj["Destination_Name"].toString();
    reservation.numberOfPersons = jsonObj["Number_of_Persons"].toString().toInt();
    reservation.totalPrice = jsonObj["Total_Price"].toString().toDouble();
    reservation.reservationDate = jsonObj["Reservation_Date"].toString();
    reservation.status = jsonObj["Status"].toString();
    reservation.notes = jsonObj["Notes"].toString();
    
    return reservation;
}