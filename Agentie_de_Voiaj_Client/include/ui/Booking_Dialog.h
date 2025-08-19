#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDateEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QTableWidget>
#include <QComboBox>
#include <memory>
#include "models/Offer_Data.h"

struct Booking_Data
{
    int offer_id = 0;
    QString offer_name;
    QString destination;
    double price_per_person = 0.0;
    int number_of_persons = 1;
    double total_price = 0.0;
    QString departure_date;
    QString return_date;

    struct Person_Info
    {
        QString full_name;
        QString cnp;
        QString birth_date;
        QString person_type;
    };
    QVector<Person_Info> persons;
};

class User_Model;

class Booking_Dialog : public QDialog
{
    Q_OBJECT

public:
    Booking_Dialog(QWidget* parent = nullptr);
    Booking_Dialog(const Offer_Data& offer, QWidget* parent = nullptr);
    ~Booking_Dialog();

    static bool show_booking_dialog(const Offer_Data& offer, QWidget* parent = nullptr);

    Booking_Data get_booking_data() const;

signals:
    void booking_confirmed(const Booking_Data& booking);
    void booking_canceled();

private slots:
    void on_person_count_changed(int count);
    void on_calculate_total_price();
    void on_confirm_booking();
    void on_cancel_booking();

    void on_booking_success(const QString& message);
    void on_booking_failed(const QString& error);

private:
    void setup_ui();
    void setup_persons_table();
    void setup_connections();
    void update_persons_table();
    void update_price_calculation();
    void validate_and_enable_booking();

    bool validate_persons_data();
    bool validate_required_fields();
    void show_loading_state(bool loading);
    void show_error(const QString& message);
    void show_success(const QString& message);

    QVBoxLayout* m_main_layout;

    QGroupBox* m_offer_group;
    QLabel* m_offer_name_label;
    QLabel* m_destination_label;
    QLabel* m_dates_label;
    QLabel* m_price_per_person_label;

    QGroupBox* m_booking_group;
    QSpinBox* m_person_count_spin;
    QTableWidget* m_persons_table;
    QTextEdit* m_special_requests_edit;

    QGroupBox* m_price_group;
    QLabel* m_subtotal_label;
    QLabel* m_taxes_label;
    QLabel* m_total_price_label;

    QPushButton* m_confirm_button;
    QPushButton* m_cancel_button;
    QProgressBar* m_progress_bar;
    QLabel* m_status_label;

    Offer_Data m_offer;
    Booking_Data m_booking_data;
    std::unique_ptr<User_Model> m_user_model;

    bool m_is_loading;

    static constexpr double TAX_RATE = 0.19; // 19% VAT
};
