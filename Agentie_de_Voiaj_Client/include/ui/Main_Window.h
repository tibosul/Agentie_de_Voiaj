#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollArea>
#include <QMenuBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QGroupBox>
#include <QGridLayout>
#include <memory>

#include "models/User_Model.h"
#include "models/Destination_Model.h"
#include "models/Offer_Model.h"
#include "models/Reservation_Model.h"
class QPropertyAnimation;

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

class Main_Window : public QMainWindow
{
    Q_OBJECT

public:
    Main_Window(QWidget *parent = nullptr);
    ~Main_Window();

private slots:
    void on_login_action();
    void on_logout_action();
    void on_exit_action();
    void on_about_action();
    void on_settings_action();
    void on_toggle_theme_action();

    void on_tab_changed(int index);

    void on_user_logged_in();
    void on_user_logged_out();
    void on_authentication_status_changed(bool is_authenticated);

    void on_destinations_loaded();
    void on_offers_loaded();
    void on_reservations_loaded();
    void on_user_info_updated();

private:
    void setup_ui();
    void setup_menu_bar();
    void setup_status_bar();
    void setup_tabs();
    void setup_connections();

    void create_destinations_tab();
    void create_offers_tab();
    void create_reservations_tab();
    void create_profile_tab();

    void update_ui_for_authentication_state();
    void show_welcome_message();
    void show_login_prompt();

    void animate_tab_change();
    void show_loading_indicator(bool show);
    
    // UI refresh methods
    void refresh_offers_display();
    void refresh_reservations_display();
    
    // Card creation methods
    QWidget* create_offer_card(const int offer_index);
    QWidget* create_reservation_card(const int reservation_index);
    QWidget* create_offer_card(const Offer_Model::Offer& offer);
    QWidget* create_reservation_card(const Reservation_Model::Reservation& reservation);

    QWidget* m_central_widget;
    QTabWidget* m_tab_widget;

    QWidget* m_destinations_tab;
    QWidget* m_offers_tab;
    QWidget* m_reservations_tab;
    QWidget* m_profile_tab;

    QMenuBar* m_menu_bar;
    QMenu* m_file_menu;
    QMenu* m_view_menu;
    QMenu* m_help_menu;
    QAction* m_login_action;
    QAction* m_logout_action;
    QAction* m_exit_action;
    QAction* m_settings_action;
    QAction* m_toggle_theme_action;
    QAction* m_about_action;

    QStatusBar* m_status_bar;
    QLabel* m_connection_status_label;
    QLabel* m_user_status_label;
    QProgressBar* m_progress_bar;

    QWidget* m_header_widget;
    QLabel* m_title_label;
    QLabel* m_user_info_label;
    QPushButton* m_user_menu_button;
    QPushButton* m_theme_toggle_button;

    // Models
    std::unique_ptr<User_Model> m_user_model;
    std::unique_ptr<Destination_Model> m_destination_model;
    std::unique_ptr<Offer_Model> m_offer_model;
    std::unique_ptr<Reservation_Model> m_reservation_model;

    // Offers tab UI references
    QWidget* m_offers_container;
    QVBoxLayout* m_offers_container_layout;
    QLabel* m_offers_loading_label;
    QLabel* m_offers_no_offers_label;

    // Reservations tab UI references
    QWidget* m_reservations_auth_widget;
    QScrollArea* m_reservations_scroll_area;
    QWidget* m_reservations_container;
    QVBoxLayout* m_reservations_container_layout;
    QLabel* m_reservations_loading_label;
    QLabel* m_reservations_no_reservations_label;

    // Profile tab UI references
    QWidget* m_profile_auth_widget;
    QWidget* m_profile_form_widget;
    QLineEdit* m_profile_username_edit;
    QLineEdit* m_profile_email_edit;
    QLineEdit* m_profile_first_name_edit;
    QLineEdit* m_profile_last_name_edit;
    QLineEdit* m_profile_phone_edit;
    QPushButton* m_profile_edit_button;
    QPushButton* m_profile_save_button;
    QPushButton* m_profile_cancel_button;

    std::unique_ptr<QPropertyAnimation> m_tab_animation;

    bool m_is_authenticated;
    QString m_current_theme;
};