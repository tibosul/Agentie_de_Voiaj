#pragma once
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QSettings>
#include <QJsonObject>
#include <memory>
#include "User_Data.h"

class User_Model : public QObject
{
    Q_OBJECT

public:
    explicit User_Model(QObject* parent = nullptr);
    ~User_Model();

    void login(const QString& username, const QString& password);
    void register_user(const QString& username, const QString& password, const QString& email,
                       const QString& first_name, const QString& last_name, const QString& phone_number = "");
    void logout();

    void update_user_info(const QString& email, const QString& first_name,
                          const QString& last_name, const QString& phone_number);
    void refresh_user_data();

    const User_Data& get_user_data() const;
    bool is_logged_in() const;
    QString get_display_name() const;
    QString get_username() const;
    int get_user_id() const;
    QString get_email() const;
    QString get_first_name() const;
    QString get_last_name() const;
    QString get_phone() const;
    void refresh_user_info();

    void save_login_state();
    void load_login_state();
    void clear_saved_state();

signals:
    void login_success();
    void login_failed(const QString& error_message);
    void register_success();
    void register_failed(const QString& error_message);
    void logged_out();

    void user_data_changed();
    void user_info_updated();
    void authentication_status_changed(bool is_authenticated);

private slots:
    void on_api_login_success(const QJsonObject& user_data);
    void on_api_login_failed(const QString& error_message);
    void on_api_register_success();
    void on_api_register_failed(const QString& error_message);
    void on_api_user_info_received(const QJsonObject& user_info);

private:
    void connect_api_signals();
    void update_user_data_from_json(const QJsonObject& user_data);
    void save_user_settings();
    void load_user_settings();

    User_Data m_user_data;
    std::unique_ptr<QSettings> m_settings;
    bool m_remember_me = false;
};