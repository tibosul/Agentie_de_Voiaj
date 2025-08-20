#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QProgressBar>
#include <memory>

class User_Model;

class Login_Window : public QDialog
{
    Q_OBJECT

public:
    explicit Login_Window(QWidget* parent = nullptr);
    ~Login_Window();

    static bool show_login_dialog(QWidget* parent = nullptr);

private slots:
    void on_login_clicked();
    void on_register_clicked();
    void on_forgot_password_clicked();
    void on_remember_me_toggled(bool checked);

    void on_login_success();
    void on_login_failed(const QString& error_message);
    void on_register_success();
    void on_register_failed(const QString& error_message);
    
    void on_network_error(const QString& error_message);

private:
    void setup_ui();
    void setup_connections();
    void setup_validation();
    void load_saved_credentials();
    void save_credentials();
    void clear_form();

    void set_login_mode();
    void set_register_mode();
    void set_loading_state(bool loading);

    bool validate_login_form();
    bool validate_register_form();
    void show_error(const QString& message);
    void show_success(const QString& message);

    QTabWidget* m_tab_widget;

    QWidget* m_login_tab;
    QLineEdit* m_username_input;
    QLineEdit* m_password_input;
    QPushButton* m_login_button;
    QCheckBox* m_remember_me_checkbox;
    QPushButton* m_forgot_password_button;

    QWidget* m_register_tab;
    QLineEdit* m_register_username;
    QLineEdit* m_register_password;
    QLineEdit* m_register_confirm_password;
    QLineEdit* m_register_email;
    QLineEdit* m_register_first_name;
    QLineEdit* m_register_last_name;
    QLineEdit* m_register_phone;
    QPushButton* m_register_button;

    QLabel* m_status_label;
    QProgressBar* m_progress_bar;

    std::unique_ptr<User_Model> m_user_model;
    bool m_is_loading;
};