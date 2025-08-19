#include "ui/Login_Window.h"
#include "models/User_Model.h"
#include "config.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QRegularExpressionValidator>
#include <QRegularExpression>

Login_Window::Login_Window(QWidget* parent)
    : QDialog(parent)
    , m_tab_widget(nullptr)
    , m_login_tab(nullptr)
    , m_username_input(nullptr)
    , m_password_input(nullptr)
    , m_login_button(nullptr)
    , m_remember_me_checkbox(nullptr)
    , m_forgot_password_button(nullptr)
    , m_register_tab(nullptr)
    , m_register_username(nullptr)
    , m_register_password(nullptr)
    , m_register_confirm_password(nullptr)
    , m_register_email(nullptr)
    , m_register_first_name(nullptr)
    , m_register_last_name(nullptr)
    , m_register_phone(nullptr)
    , m_register_button(nullptr)
    , m_status_label(nullptr)
    , m_progress_bar(nullptr)
    , m_user_model(std::make_unique<User_Model>(this))
    , m_is_loading(false)
{
    setup_ui();
    setup_connections();
    setup_validation();
    load_saved_credentials();
    
    setModal(true);
    setWindowTitle("Agentie de Voiaj - Autentificare");
    setFixedSize(400, 500);
}

Login_Window::~Login_Window() = default;

bool Login_Window::show_login_dialog(QWidget* parent)
{
    Login_Window dialog(parent);
    return dialog.exec() == QDialog::Accepted;
}

void Login_Window::setup_ui()
{
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Title
    QLabel* titleLabel = new QLabel("Agentie de Voiaj");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #4a90e2; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel);
    
    // Tab widget
    m_tab_widget = new QTabWidget();
    
    // === LOGIN TAB ===
    m_login_tab = new QWidget();
    QVBoxLayout* loginLayout = new QVBoxLayout(m_login_tab);
    loginLayout->setSpacing(15);
    
    // Login form
    QGroupBox* loginGroup = new QGroupBox("Conectare");
    QGridLayout* loginGridLayout = new QGridLayout(loginGroup);
    loginGridLayout->setSpacing(10);
    
    // Username
    loginGridLayout->addWidget(new QLabel("Utilizator:"), 0, 0);
    m_username_input = new QLineEdit();
    m_username_input->setPlaceholderText("Introduceți numele de utilizator");
    loginGridLayout->addWidget(m_username_input, 0, 1);
    
    // Password
    loginGridLayout->addWidget(new QLabel("Parolă:"), 1, 0);
    m_password_input = new QLineEdit();
    m_password_input->setPlaceholderText("Introduceți parola");
    m_password_input->setEchoMode(QLineEdit::Password);
    loginGridLayout->addWidget(m_password_input, 1, 1);
    
    loginLayout->addWidget(loginGroup);
    
    // Remember me
    m_remember_me_checkbox = new QCheckBox("Ține-mă minte");
    loginLayout->addWidget(m_remember_me_checkbox);
    
    // Login button
    m_login_button = new QPushButton("Conectare");
    m_login_button->setDefault(true);
    m_login_button->setMinimumHeight(40);
    loginLayout->addWidget(m_login_button);
    
    // Forgot password
    m_forgot_password_button = new QPushButton("Am uitat parola");
    m_forgot_password_button->setFlat(true);
    loginLayout->addWidget(m_forgot_password_button);
    
    loginLayout->addStretch();
    m_tab_widget->addTab(m_login_tab, "Conectare");
    
    // === REGISTER TAB ===
    m_register_tab = new QWidget();
    QVBoxLayout* registerLayout = new QVBoxLayout(m_register_tab);
    registerLayout->setSpacing(15);
    
    // Register form
    QGroupBox* registerGroup = new QGroupBox("Înregistrare Cont Nou");
    QGridLayout* registerGridLayout = new QGridLayout(registerGroup);
    registerGridLayout->setSpacing(10);
    
    // Username
    registerGridLayout->addWidget(new QLabel("Utilizator:*"), 0, 0);
    m_register_username = new QLineEdit();
    m_register_username->setPlaceholderText("Nume utilizator (minim 3 caractere)");
    registerGridLayout->addWidget(m_register_username, 0, 1);
    
    // Email
    registerGridLayout->addWidget(new QLabel("Email:*"), 1, 0);
    m_register_email = new QLineEdit();
    m_register_email->setPlaceholderText("adresa@email.com");
    registerGridLayout->addWidget(m_register_email, 1, 1);
    
    // First Name
    registerGridLayout->addWidget(new QLabel("Prenume:*"), 2, 0);
    m_register_first_name = new QLineEdit();
    m_register_first_name->setPlaceholderText("Prenumele dvs.");
    registerGridLayout->addWidget(m_register_first_name, 2, 1);
    
    // Last Name
    registerGridLayout->addWidget(new QLabel("Nume:*"), 3, 0);
    m_register_last_name = new QLineEdit();
    m_register_last_name->setPlaceholderText("Numele de familie");
    registerGridLayout->addWidget(m_register_last_name, 3, 1);
    
    // Phone (optional)
    registerGridLayout->addWidget(new QLabel("Telefon:"), 4, 0);
    m_register_phone = new QLineEdit();
    m_register_phone->setPlaceholderText("0700123456 (opțional)");
    registerGridLayout->addWidget(m_register_phone, 4, 1);
    
    // Password
    registerGridLayout->addWidget(new QLabel("Parolă:*"), 5, 0);
    m_register_password = new QLineEdit();
    m_register_password->setPlaceholderText("Minim 6 caractere");
    m_register_password->setEchoMode(QLineEdit::Password);
    registerGridLayout->addWidget(m_register_password, 5, 1);
    
    // Confirm Password
    registerGridLayout->addWidget(new QLabel("Confirmă:*"), 6, 0);
    m_register_confirm_password = new QLineEdit();
    m_register_confirm_password->setPlaceholderText("Repetați parola");
    m_register_confirm_password->setEchoMode(QLineEdit::Password);
    registerGridLayout->addWidget(m_register_confirm_password, 6, 1);
    
    registerLayout->addWidget(registerGroup);
    
    // Register button
    m_register_button = new QPushButton("Creează Cont");
    m_register_button->setMinimumHeight(40);
    registerLayout->addWidget(m_register_button);
    
    // Required fields note
    QLabel* requiredNote = new QLabel("* Câmpuri obligatorii");
    requiredNote->setStyleSheet("color: gray; font-size: 12px;");
    registerLayout->addWidget(requiredNote);
    
    registerLayout->addStretch();
    m_tab_widget->addTab(m_register_tab, "Înregistrare");
    
    mainLayout->addWidget(m_tab_widget);
    
    // Status and progress
    m_status_label = new QLabel();
    m_status_label->setAlignment(Qt::AlignCenter);
    m_status_label->setWordWrap(true);
    m_status_label->hide();
    mainLayout->addWidget(m_status_label);
    
    m_progress_bar = new QProgressBar();
    m_progress_bar->setRange(0, 0); // Indeterminate
    m_progress_bar->hide();
    mainLayout->addWidget(m_progress_bar);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* cancelButton = new QPushButton("Anulează");
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void Login_Window::setup_connections()
{
    // Login tab connections
    connect(m_login_button, &QPushButton::clicked, this, &Login_Window::on_login_clicked);
    connect(m_forgot_password_button, &QPushButton::clicked, this, &Login_Window::on_forgot_password_clicked);
    connect(m_remember_me_checkbox, &QCheckBox::toggled, this, &Login_Window::on_remember_me_toggled);
    
    // Register tab connections
    connect(m_register_button, &QPushButton::clicked, this, &Login_Window::on_register_clicked);
    
    // Enter key handling
    connect(m_password_input, &QLineEdit::returnPressed, this, &Login_Window::on_login_clicked);
    connect(m_register_confirm_password, &QLineEdit::returnPressed, this, &Login_Window::on_register_clicked);
    
    // Model connections
    connect(m_user_model.get(), &User_Model::login_succes, this, &Login_Window::on_login_success);
    connect(m_user_model.get(), &User_Model::login_failed, this, &Login_Window::on_login_failed);
    connect(m_user_model.get(), &User_Model::register_succes, this, &Login_Window::on_register_success);
    connect(m_user_model.get(), &User_Model::register_failed, this, &Login_Window::on_register_failed);
}

void Login_Window::setup_validation()
{
    // Email validator
    QRegularExpression emailRegex("\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}\\b");
    m_register_email->setValidator(new QRegularExpressionValidator(emailRegex, this));
    
    // Phone validator (Romanian format)
    QRegularExpression phoneRegex("\\+?[0-9\\s\\-()]{7,15}");
    m_register_phone->setValidator(new QRegularExpressionValidator(phoneRegex, this));
}

void Login_Window::on_login_clicked()
{
    if (!validate_login_form()) {
        return;
    }
    
    set_loading_state(true);
    
    QString username = m_username_input->text().trimmed();
    QString password = m_password_input->text();
    
    m_user_model->login(username, password);
}

void Login_Window::on_register_clicked()
{
    if (!validate_register_form()) {
        return;
    }
    
    set_loading_state(true);
    
    m_user_model->register_user(
        m_register_username->text().trimmed(),
        m_register_password->text(),
        m_register_email->text().trimmed(),
        m_register_first_name->text().trimmed(),
        m_register_last_name->text().trimmed(),
        m_register_phone->text().trimmed()
    );
}

void Login_Window::on_forgot_password_clicked()
{
    QMessageBox::information(this, "Parolă Uitată", 
                            "Funcționalitatea de resetare parolă va fi implementată în curând.\n"
                            "Contactați administratorul pentru asistență.");
}

void Login_Window::on_remember_me_toggled(bool checked)
{
    Q_UNUSED(checked)
    // Implementation for remember me functionality
}

void Login_Window::on_login_success()
{
    set_loading_state(false);
    show_success("Autentificare reușită!");
    
    if (m_remember_me_checkbox->isChecked()) {
        save_credentials();
    }
    
    accept(); // Close dialog with success
}

void Login_Window::on_login_failed(const QString& error_message)
{
    set_loading_state(false);
    show_error("Autentificare eșuată: " + error_message);
}

void Login_Window::on_register_success()
{
    set_loading_state(false);
    show_success("Cont creat cu succes! Acum vă puteți conecta.");
    
    // Switch to login tab and clear register form
    m_tab_widget->setCurrentIndex(0);
    clear_form();
}

void Login_Window::on_register_failed(const QString& error_message)
{
    set_loading_state(false);
    show_error("Înregistrare eșuată: " + error_message);
}

void Login_Window::load_saved_credentials()
{
    QSettings settings;
    settings.beginGroup("Login");
    
    QString savedUsername = settings.value("username").toString();
    bool rememberMe = settings.value("rememberMe", false).toBool();
    
    if (!savedUsername.isEmpty() && rememberMe) {
        m_username_input->setText(savedUsername);
        m_remember_me_checkbox->setChecked(true);
        m_password_input->setFocus();
    }
    
    settings.endGroup();
}

void Login_Window::save_credentials()
{
    QSettings settings;
    settings.beginGroup("Login");
    
    if (m_remember_me_checkbox->isChecked()) {
        settings.setValue("username", m_username_input->text());
        settings.setValue("rememberMe", true);
    } else {
        settings.remove("username");
        settings.setValue("rememberMe", false);
    }
    
    settings.endGroup();
}

void Login_Window::clear_form()
{
    // Clear register form
    m_register_username->clear();
    m_register_password->clear();
    m_register_confirm_password->clear();
    m_register_email->clear();
    m_register_first_name->clear();
    m_register_last_name->clear();
    m_register_phone->clear();
}

void Login_Window::set_login_mode()
{
    // Implementation for setting login mode if needed
}

void Login_Window::set_register_mode()
{
    // Implementation for setting register mode if needed
}

void Login_Window::set_loading_state(bool loading)
{
    m_is_loading = loading;
    
    // Disable/enable inputs
    m_login_button->setEnabled(!loading);
    m_register_button->setEnabled(!loading);
    m_username_input->setEnabled(!loading);
    m_password_input->setEnabled(!loading);
    m_register_username->setEnabled(!loading);
    m_register_password->setEnabled(!loading);
    m_register_confirm_password->setEnabled(!loading);
    m_register_email->setEnabled(!loading);
    m_register_first_name->setEnabled(!loading);
    m_register_last_name->setEnabled(!loading);
    m_register_phone->setEnabled(!loading);
    
    // Show/hide progress
    if (loading) {
        m_progress_bar->show();
        m_status_label->setText("Se procesează...");
        m_status_label->show();
    } else {
        m_progress_bar->hide();
    }
}

bool Login_Window::validate_login_form()
{
    if (m_username_input->text().trimmed().isEmpty()) {
        show_error("Vă rugăm să introduceți numele de utilizator.");
        m_username_input->setFocus();
        return false;
    }
    
    if (m_password_input->text().isEmpty()) {
        show_error("Vă rugăm să introduceți parola.");
        m_password_input->setFocus();
        return false;
    }
    
    return true;
}

bool Login_Window::validate_register_form()
{
    // Username validation
    QString username = m_register_username->text().trimmed();
    if (username.length() < 3) {
        show_error("Numele de utilizator trebuie să aibă minim 3 caractere.");
        m_register_username->setFocus();
        return false;
    }
    
    // Email validation
    QString email = m_register_email->text().trimmed();
    if (email.isEmpty() || !email.contains("@")) {
        show_error("Vă rugăm să introduceți o adresă de email validă.");
        m_register_email->setFocus();
        return false;
    }
    
    // First name validation
    if (m_register_first_name->text().trimmed().isEmpty()) {
        show_error("Vă rugăm să introduceți prenumele.");
        m_register_first_name->setFocus();
        return false;
    }
    
    // Last name validation
    if (m_register_last_name->text().trimmed().isEmpty()) {
        show_error("Vă rugăm să introduceți numele de familie.");
        m_register_last_name->setFocus();
        return false;
    }
    
    // Password validation
    QString password = m_register_password->text();
    if (password.length() < 6) {
        show_error("Parola trebuie să aibă minim 6 caractere.");
        m_register_password->setFocus();
        return false;
    }
    
    // Confirm password validation
    if (password != m_register_confirm_password->text()) {
        show_error("Parolele nu coincid.");
        m_register_confirm_password->setFocus();
        return false;
    }
    
    return true;
}

void Login_Window::show_error(const QString& message)
{
    m_status_label->setText(message);
    m_status_label->setStyleSheet("color: #e74c3c; font-weight: bold;");
    m_status_label->show();
}

void Login_Window::show_success(const QString& message)
{
    m_status_label->setText(message);
    m_status_label->setStyleSheet("color: #27ae60; font-weight: bold;");
    m_status_label->show();
}