#include "login_dialog.h"
#include "network_manager.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMessageBox>

Login_Dialog::Login_Dialog(Network_Manager* network_manager, QWidget* parent)
    : QDialog(parent), network_manager(network_manager)
{
    setup_ui();
    setup_connections();
    
    setWindowTitle("Agentie de Voiaj - Autentificare");
    setModal(true);
    resize(400, 500);
    setFixedSize(400, 500);
}

void Login_Dialog::setup_ui()
{
    auto* main_layout = new QVBoxLayout(this);
    
    // Title
    auto* title_label = new QLabel("Agentie de Voiaj", this);
    title_label->setAlignment(Qt::AlignCenter);
    title_label->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
    main_layout->addWidget(title_label);
    
    // Tab widget
    tab_widget = new QTabWidget(this);
    main_layout->addWidget(tab_widget);
    
    // Setup login tab
    login_tab = new QWidget();
    auto* login_layout = new QVBoxLayout(login_tab);
    
    auto* login_form = new QFormLayout();
    
    login_username_edit = new QLineEdit();
    login_username_edit->setPlaceholderText("Introduceti numele de utilizator");
    login_form->addRow("Utilizator:", login_username_edit);
    
    login_password_edit = new QLineEdit();
    login_password_edit->setEchoMode(QLineEdit::Password);
    login_password_edit->setPlaceholderText("Introduceti parola");
    login_form->addRow("Parola:", login_password_edit);
    
    remember_me_checkbox = new QCheckBox("Retine-ma");
    login_form->addRow("", remember_me_checkbox);
    
    login_layout->addLayout(login_form);
    login_layout->addStretch();
    
    login_button = new QPushButton("Conectare");
    login_button->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #45a049; }");
    login_layout->addWidget(login_button);
    
    tab_widget->addTab(login_tab, "Conectare");
    
    // Setup register tab
    register_tab = new QWidget();
    auto* register_layout = new QVBoxLayout(register_tab);
    
    auto* register_form = new QFormLayout();
    
    register_username_edit = new QLineEdit();
    register_username_edit->setPlaceholderText("Minim 3 caractere");
    register_form->addRow("Utilizator:", register_username_edit);
    
    register_password_edit = new QLineEdit();
    register_password_edit->setEchoMode(QLineEdit::Password);
    register_password_edit->setPlaceholderText("Minim 6 caractere");
    register_form->addRow("Parola:", register_password_edit);
    
    register_confirm_password_edit = new QLineEdit();
    register_confirm_password_edit->setEchoMode(QLineEdit::Password);
    register_confirm_password_edit->setPlaceholderText("Confirmati parola");
    register_form->addRow("Confirmare:", register_confirm_password_edit);
    
    register_email_edit = new QLineEdit();
    register_email_edit->setPlaceholderText("exemplu@email.com");
    // Email validator
    QRegularExpression email_regex("^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$");
    register_email_edit->setValidator(new QRegularExpressionValidator(email_regex, this));
    register_form->addRow("Email:", register_email_edit);
    
    register_first_name_edit = new QLineEdit();
    register_first_name_edit->setPlaceholderText("Prenumele");
    register_form->addRow("Prenume:", register_first_name_edit);
    
    register_last_name_edit = new QLineEdit();
    register_last_name_edit->setPlaceholderText("Numele de familie");
    register_form->addRow("Nume:", register_last_name_edit);
    
    register_phone_edit = new QLineEdit();
    register_phone_edit->setPlaceholderText("0712345678");
    // Phone validator
    QRegularExpression phone_regex("^[0-9+\\-\\s\\(\\)]{8,15}$");
    register_phone_edit->setValidator(new QRegularExpressionValidator(phone_regex, this));
    register_form->addRow("Telefon:", register_phone_edit);
    
    register_layout->addLayout(register_form);
    register_layout->addStretch();
    
    register_button = new QPushButton("Inregistrare");
    register_button->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #1976D2; }");
    register_layout->addWidget(register_button);
    
    tab_widget->addTab(register_tab, "Inregistrare");
    
    // Status area
    status_label = new QLabel();
    status_label->setAlignment(Qt::AlignCenter);
    status_label->setWordWrap(true);
    status_label->hide();
    main_layout->addWidget(status_label);
    
    // Progress bar
    progress_bar = new QProgressBar();
    progress_bar->setRange(0, 0); // Indeterminate progress
    progress_bar->hide();
    main_layout->addWidget(progress_bar);
    
    // Cancel button
    cancel_button = new QPushButton("Anuleaza");
    cancel_button->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 8px; border: none; border-radius: 4px; } QPushButton:hover { background-color: #d32f2f; }");
    main_layout->addWidget(cancel_button);
}

void Login_Dialog::setup_connections()
{
    // Button connections
    connect(login_button, &QPushButton::clicked, this, &Login_Dialog::on_login_clicked);
    connect(register_button, &QPushButton::clicked, this, &Login_Dialog::on_register_clicked);
    connect(cancel_button, &QPushButton::clicked, this, &QDialog::reject);
    
    // Enter key support
    connect(login_password_edit, &QLineEdit::returnPressed, this, &Login_Dialog::on_login_clicked);
    connect(register_phone_edit, &QLineEdit::returnPressed, this, &Login_Dialog::on_register_clicked);
    
    // Network manager connections
    connect(network_manager, &Network_Manager::connected_to_server, 
            this, &Login_Dialog::on_connected_to_server);
    connect(network_manager, &Network_Manager::connection_error, 
            this, &Login_Dialog::on_connection_error);
    connect(network_manager, &Network_Manager::authentication_successful, 
            this, &Login_Dialog::on_authentication_successful);
    connect(network_manager, &Network_Manager::authentication_failed, 
            this, &Login_Dialog::on_authentication_failed);
    connect(network_manager, &Network_Manager::registration_successful, 
            this, &Login_Dialog::on_registration_successful);
    connect(network_manager, &Network_Manager::registration_failed, 
            this, &Login_Dialog::on_registration_failed);
}

void Login_Dialog::on_login_clicked()
{
    if (!validate_login_fields())
    {
        return;
    }
    
    clear_messages();
    set_enabled(false);
    progress_bar->show();
    
    show_message(Config::StatusMessages::CONNECTING);
    
    // Connect to server if not connected
    if (network_manager->get_connection_status() != Network_Manager::Connection_Status::Connected)
    {
        network_manager->connect_to_server();
    }
    else
    {
        // Already connected, authenticate directly
        show_message(Config::StatusMessages::AUTHENTICATING);
        network_manager->authenticate_user(login_username_edit->text(), login_password_edit->text());
    }
}

void Login_Dialog::on_register_clicked()
{
    if (!validate_register_fields())
    {
        return;
    }
    
    clear_messages();
    set_enabled(false);
    progress_bar->show();
    
    show_message(Config::StatusMessages::CONNECTING);
    
    // Connect to server if not connected
    if (network_manager->get_connection_status() != Network_Manager::Connection_Status::Connected)
    {
        network_manager->connect_to_server();
    }
    else
    {
        // Already connected, register directly
        show_message(Config::StatusMessages::REGISTERING);
        network_manager->register_user(
            register_username_edit->text(),
            register_password_edit->text(),
            register_email_edit->text(),
            register_first_name_edit->text(),
            register_last_name_edit->text(),
            register_phone_edit->text()
        );
    }
}

void Login_Dialog::on_connected_to_server()
{
    // Determine which action to take based on current tab
    if (tab_widget->currentIndex() == 0) // Login tab
    {
        show_message(Config::StatusMessages::AUTHENTICATING);
        network_manager->authenticate_user(login_username_edit->text(), login_password_edit->text());
    }
    else // Register tab
    {
        show_message(Config::StatusMessages::REGISTERING);
        network_manager->register_user(
            register_username_edit->text(),
            register_password_edit->text(),
            register_email_edit->text(),
            register_first_name_edit->text(),
            register_last_name_edit->text(),
            register_phone_edit->text()
        );
    }
}

void Login_Dialog::on_connection_error(const QString& error)
{
    progress_bar->hide();
    set_enabled(true);
    show_message(Config::ErrorMessages::CONNECTION_FAILED + ": " + error, true);
}

void Login_Dialog::on_authentication_successful()
{
    progress_bar->hide();
    show_message(Config::SuccessMessages::AUTHENTICATION_SUCCESSFUL, false);
    emit login_successful();
    accept();
}

void Login_Dialog::on_authentication_failed(const QString& error)
{
    progress_bar->hide();
    set_enabled(true);
    show_message(Config::ErrorMessages::AUTHENTICATION_FAILED + ": " + error, true);
}

void Login_Dialog::on_registration_successful()
{
    progress_bar->hide();
    set_enabled(true);
    show_message(Config::SuccessMessages::REGISTRATION_SUCCESSFUL + "! Puteti sa va conectati acum.", false);
    
    // Switch to login tab and clear register fields
    tab_widget->setCurrentIndex(0);
    register_username_edit->clear();
    register_password_edit->clear();
    register_confirm_password_edit->clear();
    register_email_edit->clear();
    register_first_name_edit->clear();
    register_last_name_edit->clear();
    register_phone_edit->clear();
}

void Login_Dialog::on_registration_failed(const QString& error)
{
    progress_bar->hide();
    set_enabled(true);
    show_message(Config::ErrorMessages::REGISTRATION_FAILED + ": " + error, true);
}

void Login_Dialog::set_enabled(bool enabled)
{
    login_button->setEnabled(enabled);
    register_button->setEnabled(enabled);
    tab_widget->setEnabled(enabled);
}

void Login_Dialog::show_message(const QString& message, bool is_error)
{
    status_label->setText(message);
    status_label->setStyleSheet(is_error ? "color: red;" : "color: green;");
    status_label->show();
}

void Login_Dialog::clear_messages()
{
    status_label->hide();
}

bool Login_Dialog::validate_login_fields()
{
    QString username = login_username_edit->text().trimmed();
    QString password = login_password_edit->text();
    
    if (!Utils::Validation::is_valid_username(username))
    {
        show_message(Utils::Validation::get_validation_error("username", username), true);
        login_username_edit->setFocus();
        return false;
    }
    
    if (!Utils::Validation::is_valid_password(password))
    {
        show_message(Utils::Validation::get_validation_error("password", password), true);
        login_password_edit->setFocus();
        return false;
    }
    
    return true;
}

bool Login_Dialog::validate_register_fields()
{
    QString username = register_username_edit->text().trimmed();
    QString password = register_password_edit->text();
    QString confirm_password = register_confirm_password_edit->text();
    QString email = register_email_edit->text().trimmed();
    QString first_name = register_first_name_edit->text().trimmed();
    QString last_name = register_last_name_edit->text().trimmed();
    QString phone = register_phone_edit->text().trimmed();
    
    // Validate username
    if (!Utils::Validation::is_valid_username(username))
    {
        show_message(Utils::Validation::get_validation_error("username", username), true);
        register_username_edit->setFocus();
        return false;
    }
    
    // Validate password
    if (!Utils::Validation::is_valid_password(password))
    {
        show_message(Utils::Validation::get_validation_error("password", password), true);
        register_password_edit->setFocus();
        return false;
    }
    
    // Check password confirmation
    if (password != confirm_password)
    {
        show_message("Parolele nu se potrivesc", true);
        register_confirm_password_edit->setFocus();
        return false;
    }
    
    // Validate email
    if (!Utils::Validation::is_valid_email(email))
    {
        show_message(Utils::Validation::get_validation_error("email", email), true);
        register_email_edit->setFocus();
        return false;
    }
    
    // Validate first name
    if (!Utils::Validation::is_valid_name(first_name))
    {
        show_message(Utils::Validation::get_validation_error("name", first_name), true);
        register_first_name_edit->setFocus();
        return false;
    }
    
    // Validate last name
    if (!Utils::Validation::is_valid_name(last_name))
    {
        show_message(Utils::Validation::get_validation_error("name", last_name), true);
        register_last_name_edit->setFocus();
        return false;
    }
    
    // Validate phone
    if (!Utils::Validation::is_valid_phone(phone))
    {
        show_message(Utils::Validation::get_validation_error("phone", phone), true);
        register_phone_edit->setFocus();
        return false;
    }
    
    return true;
}