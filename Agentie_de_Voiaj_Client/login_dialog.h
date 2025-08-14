#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QTabWidget>
#include <QGroupBox>
#include <QProgressBar>
#include "network_manager.h"

class Network_Manager;
struct User_Info;

class Login_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Login_Dialog(Network_Manager* network_manager, QWidget* parent = nullptr);

signals:
    void login_successful();

private slots:
    void on_login_clicked();
    void on_register_clicked();
    void on_authentication_successful(const User_Info& user);
    void on_authentication_failed(const QString& error);
    void on_registration_successful();
    void on_registration_failed(const QString& error);
    void on_connected_to_server();
    void on_connection_error(const QString& error);

private:
    void setup_ui();
    void setup_connections();
    void set_enabled(bool enabled);
    void show_message(const QString& message, bool is_error = false);
    void clear_messages();
    bool validate_login_fields();
    bool validate_register_fields();

    Network_Manager* network_manager;

    // UI elements
    QTabWidget* tab_widget;
    
    // Login tab
    QWidget* login_tab;
    QLineEdit* login_username_edit;
    QLineEdit* login_password_edit;
    QPushButton* login_button;
    QCheckBox* remember_me_checkbox;
    
    // Register tab
    QWidget* register_tab;
    QLineEdit* register_username_edit;
    QLineEdit* register_password_edit;
    QLineEdit* register_confirm_password_edit;
    QLineEdit* register_email_edit;
    QLineEdit* register_first_name_edit;
    QLineEdit* register_last_name_edit;
    QLineEdit* register_phone_edit;
    QPushButton* register_button;
    
    // Common elements
    QLabel* status_label;
    QProgressBar* progress_bar;
    QPushButton* cancel_button;
};