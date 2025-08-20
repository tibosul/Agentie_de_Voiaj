#include "models/User_Model.h"
#include "network/Api_Client.h"
#include "config.h"

#include <QJsonObject>
#include <QStandardPaths>
#include <QDebug>

User_Model::User_Model(QObject* parent)
    : QObject(parent)
    , m_remember_me(false)
{
    // Initialize settings
    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    m_settings = std::make_unique<QSettings>(settingsPath + "/user.ini", QSettings::IniFormat);
    
    connect_api_signals();
    load_login_state();
}

User_Model::~User_Model() = default;

void User_Model::login(const QString& username, const QString& password)
{
    qDebug() << "User_Model: Attempting login for user:" << username;
    Api_Client::instance().login(username, password);
}

void User_Model::register_user(const QString& username, const QString& password,
                            const QString& email, const QString& first_name,
                            const QString& last_name, const QString& phone_number)
{
    QJsonObject userData;
    userData["username"] = username;
    userData["password"] = password;
    userData["email"] = email;
    userData["first_name"] = first_name;
    userData["last_name"] = last_name;
    
    if (!phone_number.isEmpty())
    {
        userData["phone_number"] = phone_number;
    }
    
    qDebug() << "User_Model: Attempting registration for user:" << username;
    Api_Client::instance().register_user(userData);
}

void User_Model::logout()
{
    qDebug() << "User_Model: Logging out user:" << m_user_data.username;
    
    m_user_data.clear();
    clear_saved_state();
    
    Api_Client::instance().logout();
    
    emit logged_out();
    emit authentication_status_changed(false);
    emit user_data_changed();
}

void User_Model::update_user_info(const QString& email, const QString& first_name,
                              const QString& last_name, const QString& phone_number)
{
    QJsonObject userInfo;
    userInfo["email"] = email;
    userInfo["first_name"] = first_name;
    userInfo["last_name"] = last_name;
    userInfo["phone_number"] = phone_number;
    
    Api_Client::instance().update_user_info(userInfo);
}

void User_Model::refresh_user_data()
{
    if (m_user_data.is_authenticated)
    {
        Api_Client::instance().get_user_info();
    }
}

const User_Model::User_Data& User_Model::get_user_data() const
{
    return m_user_data;
}

bool User_Model::is_logged_in() const
{
    return m_user_data.is_authenticated;
}

QString User_Model::get_display_name() const
{
    if (!m_user_data.first_name.isEmpty() && !m_user_data.last_name.isEmpty())
    {
        return m_user_data.first_name + " " + m_user_data.last_name;
    }
    return m_user_data.username;
}

QString User_Model::get_username() const
{
    return m_user_data.username;
}

int User_Model::get_user_id() const
{
    return m_user_data.id;
}

QString User_Model::get_email() const
{
    return m_user_data.email;
}

QString User_Model::get_first_name() const
{
    return m_user_data.first_name;
}

QString User_Model::get_last_name() const
{
    return m_user_data.last_name;
}

QString User_Model::get_phone() const
{
    return m_user_data.phone_number;
}

void User_Model::refresh_user_info()
{
    refresh_user_data();
}

void User_Model::save_login_state()
{
    if (m_remember_me && m_user_data.is_authenticated)
    {
        save_user_settings();
    }
}

void User_Model::load_login_state()
{
    load_user_settings();
}

void User_Model::clear_saved_state()
{
    m_settings->clear();
    m_settings->sync();
}

void User_Model::connect_api_signals()
{
    Api_Client& api = Api_Client::instance();
    
    connect(&api, &Api_Client::login_success,
            this, &User_Model::on_api_login_success);
    connect(&api, &Api_Client::login_failed,
            this, &User_Model::on_api_login_failed);
    connect(&api, &Api_Client::register_success,
            this, &User_Model::on_api_register_success);
    connect(&api, &Api_Client::register_failed,
            this, &User_Model::on_api_register_failed);
    connect(&api, &Api_Client::user_info_received,
            this, &User_Model::on_api_user_info_received);
}

void User_Model::on_api_login_success(const QJsonObject& user_data)
{
    qDebug() << "User_Model: Login successful";
    
    update_user_data_from_json(user_data);
    m_user_data.is_authenticated = true;
    m_user_data.last_login = QDateTime::currentDateTime();
    
    save_login_state();
    
    emit login_success();
    emit authentication_status_changed(true);
    emit user_data_changed();
}

void User_Model::on_api_login_failed(const QString& error_message)
{
    qDebug() << "User_Model: Login failed:" << error_message;
    
    m_user_data.clear();
    
    emit login_failed(error_message);
    emit authentication_status_changed(false);
}

void User_Model::on_api_register_success()
{
    qDebug() << "User_Model: Registration successful";
    emit register_success();
}

void User_Model::on_api_register_failed(const QString& error_message)
{
    qDebug() << "User_Model: Registration failed:" << error_message;
    emit register_failed(error_message);
}

void User_Model::on_api_user_info_received(const QJsonObject& user_info)
{
    qDebug() << "User_Model: User info received";
    
    update_user_data_from_json(user_info);
    
    emit user_info_updated();
    emit user_data_changed();
}

void User_Model::update_user_data_from_json(const QJsonObject& jsonData)
{
    if (jsonData.contains("ID")) 
    {
        m_user_data.id = jsonData["ID"].toString().toInt();
    }
    if (jsonData.contains("Username"))
    {
        m_user_data.username = jsonData["Username"].toString();
    }
    if (jsonData.contains("Email"))
    {
        m_user_data.email = jsonData["Email"].toString();
    }
    if (jsonData.contains("First_Name"))
    {
        m_user_data.first_name = jsonData["First_Name"].toString();
    }
    if (jsonData.contains("Last_Name"))
    {
        m_user_data.last_name = jsonData["Last_Name"].toString();
    }
    if (jsonData.contains("Phone"))
    {
        m_user_data.phone_number = jsonData["Phone"].toString();
    }
}

void User_Model::save_user_settings()
{
    m_settings->beginGroup("User");
    m_settings->setValue("id", m_user_data.id);
    m_settings->setValue("username", m_user_data.username);
    m_settings->setValue("email", m_user_data.email);
    m_settings->setValue("first_name", m_user_data.first_name);
    m_settings->setValue("last_name", m_user_data.last_name);
    m_settings->setValue("phone_number", m_user_data.phone_number);
    m_settings->setValue("last_login", m_user_data.last_login);
    m_settings->setValue("is_authenticated", m_user_data.is_authenticated);
    m_settings->endGroup();
    m_settings->sync();
}

void User_Model::load_user_settings()
{
    m_settings->beginGroup("User");
    m_user_data.id = m_settings->value("id", 0).toInt();
    m_user_data.username = m_settings->value("username").toString();
    m_user_data.email = m_settings->value("email").toString();
    m_user_data.first_name = m_settings->value("first_name").toString();
    m_user_data.last_name = m_settings->value("last_name").toString();
    m_user_data.phone_number = m_settings->value("phone_number").toString();
    m_user_data.last_login = m_settings->value("last_login").toDateTime();
    m_user_data.is_authenticated = m_settings->value("is_authenticated", false).toBool();
    m_settings->endGroup();
    
    // Don't auto-authenticate from saved state for security
    if (m_user_data.is_authenticated)
    {
        m_user_data.is_authenticated = false;
        m_remember_me = true;
    }
}

