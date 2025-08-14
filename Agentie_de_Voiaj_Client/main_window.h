#pragma once

#include <QtWidgets/QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QGroupBox>
#include <QSplitter>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QProgressBar>
#include <QTextEdit>
#include <QScrollArea>
#include "ui_main_window.h"
#include "network_manager.h"

class Login_Dialog;

class Main_Window : public QMainWindow
{
    Q_OBJECT

public:
    Main_Window(QWidget *parent = nullptr);
    ~Main_Window();

private slots:
    // Authentication
    void show_login_dialog();
    void on_login_successful();
    void on_user_logged_out();
    
    // Navigation
    void on_offers_tab_clicked();
    void on_search_tab_clicked();
    void on_reservations_tab_clicked();
    void on_profile_tab_clicked();
    
    // Data loading
    void on_destinations_received(const QList<Destination_Info>& destinations);
    void on_offers_received(const QList<Offer_Info>& offers);
    void on_reservations_received(const QList<Reservation_Info>& reservations);
    void on_user_info_received(const User_Info& user);
    
    // Operations
    void on_search_offers();
    void on_book_offer();
    void on_cancel_reservation();
    void on_refresh_data();
    void on_update_profile();
    
    // Network status
    void on_connected_to_server();
    void on_disconnected_from_server();
    void on_connection_error(const QString& error);
    void on_operation_completed(const QString& operation, bool success, const QString& message);

private:
    void setup_ui();
    void setup_menu_bar();
    void setup_tool_bar();
    void setup_status_bar();
    void setup_connections();
    void setup_offers_tab();
    void setup_search_tab();
    void setup_reservations_tab();
    void setup_profile_tab();
    
    void update_ui_for_user_state();
    void populate_destinations_combo();
    void populate_offers_table(const QList<Offer_Info>& offers);
    void populate_offers_table(const QList<Offer_Info>& offers, QTableWidget* table);
    void populate_reservations_table(const QList<Reservation_Info>& reservations);
    void update_profile_fields(const User_Info& user);
    
    void show_status_message(const QString& message, int timeout = 3000);
    void set_connection_status(const QString& status, bool is_connected);
    
    Ui::Main_WindowClass ui;
    Network_Manager* network_manager;
    Login_Dialog* login_dialog;
    
    // Current data
    QList<Destination_Info> current_destinations;
    QList<Offer_Info> current_offers;
    QList<Reservation_Info> current_reservations;
    
    // UI Components
    QStackedWidget* main_stack;
    QTabWidget* main_tabs;
    
    // Offers tab
    QWidget* offers_tab;
    QTableWidget* offers_table;
    QPushButton* refresh_offers_button;
    QPushButton* book_selected_offer_button;
    QLabel* offers_info_label;
    
    // Search tab
    QWidget* search_tab;
    QComboBox* destination_combo;
    QLineEdit* min_price_edit;
    QLineEdit* max_price_edit;
    QDateEdit* start_date_edit;
    QDateEdit* end_date_edit;
    QPushButton* search_button;
    QPushButton* clear_search_button;
    QTableWidget* search_results_table;
    QLabel* search_status_label;
    
    // Reservations tab
    QWidget* reservations_tab;
    QTableWidget* reservations_table;
    QPushButton* refresh_reservations_button;
    QPushButton* cancel_selected_reservation_button;
    QLabel* reservations_info_label;
    
    // Profile tab
    QWidget* profile_tab;
    QLineEdit* profile_username_edit;
    QLineEdit* profile_email_edit;
    QLineEdit* profile_first_name_edit;
    QLineEdit* profile_last_name_edit;
    QLineEdit* profile_phone_edit;
    QPushButton* update_profile_button;
    QPushButton* logout_button;
    
    // Menu and toolbar
    QAction* connect_action;
    QAction* disconnect_action;
    QAction* login_action;
    QAction* logout_action;
    QAction* refresh_action;
    QAction* about_action;
    
    // Status bar
    QLabel* connection_status_label;
    QLabel* user_status_label;
    QProgressBar* operation_progress;
};

