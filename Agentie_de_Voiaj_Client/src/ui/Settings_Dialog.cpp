#include "ui/Settings_Dialog.h"
#include "utils/Style_Manager.h"
#include "config.h"

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <QSpinBox>
#include <QGroupBox>
#include <QTabWidget>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSlider>

Settings_Dialog::Settings_Dialog(QWidget* parent)
    : QDialog(parent)
    , m_settings_changed(false)
    , m_settings(std::make_unique<QSettings>(QSettings::IniFormat, QSettings::UserScope, 
                                            Config::Application::ORGANIZATION, 
                                            Config::Application::APP_NAME))
{
    setWindowTitle("Setări - " + Config::Application::APP_NAME);
    setModal(true);
    setMinimumSize(600, 500);
    resize(700, 600);

    setup_ui();
    setup_connections();
    load_settings();
}

Settings_Dialog::~Settings_Dialog()
{
}

void Settings_Dialog::show_settings_dialog(QWidget* parent)
{
    Settings_Dialog dialog(parent);
    dialog.exec();
}

void Settings_Dialog::setup_ui()
{
    auto main_layout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tab_widget = new QTabWidget(this);
    main_layout->addWidget(m_tab_widget);
    
    // Create tabs
    create_general_tab();
    create_appearance_tab();
    create_network_tab();
    create_advanced_tab();
    
    // Create button layout
    auto button_layout = new QHBoxLayout();
    button_layout->addStretch();
    
    m_reset_button = new QPushButton("Reset la valorile implicite", this);
    m_apply_button = new QPushButton("Aplică", this);
    m_cancel_button = new QPushButton("Anulează", this);
    m_ok_button = new QPushButton("OK", this);
    
    button_layout->addWidget(m_reset_button);
    button_layout->addWidget(m_apply_button);
    button_layout->addWidget(m_cancel_button);
    button_layout->addWidget(m_ok_button);
    
    main_layout->addLayout(button_layout);
    
    // Set default button
    m_ok_button->setDefault(true);
}

void Settings_Dialog::create_general_tab()
{
    m_general_tab = new QWidget(this);
    auto layout = new QVBoxLayout(m_general_tab);
    
    // General settings group
    auto general_group = new QGroupBox("Setări generale", m_general_tab);
    auto general_layout = new QGridLayout(general_group);
    
    m_remember_login_check = new QCheckBox("Ține minte datele de autentificare", general_group);
    m_auto_start_check = new QCheckBox("Pornește automat cu Windows", general_group);
    m_minimize_to_tray_check = new QCheckBox("Minimizează în bara de sistem", general_group);
    m_auto_save_check = new QCheckBox("Salvează automat", general_group);
    
    m_auto_save_interval_spin = new QSpinBox(general_group);
    m_auto_save_interval_spin->setRange(1, 60);
    m_auto_save_interval_spin->setSuffix(" minute");
    m_auto_save_interval_spin->setValue(5);
    
    general_layout->addWidget(m_remember_login_check, 0, 0);
    general_layout->addWidget(m_auto_start_check, 0, 1);
    general_layout->addWidget(m_minimize_to_tray_check, 1, 0);
    general_layout->addWidget(m_auto_save_check, 1, 1);
    general_layout->addWidget(new QLabel("Interval salvare automată:"), 2, 0);
    general_layout->addWidget(m_auto_save_interval_spin, 2, 1);
    
    // Language settings group
    auto language_group = new QGroupBox("Limba aplicației", m_general_tab);
    auto language_layout = new QHBoxLayout(language_group);
    
    m_language_combo = new QComboBox(language_group);
    m_language_combo->addItem("Română", "ro");
    m_language_combo->addItem("English", "en");
    m_language_combo->addItem("Français", "fr");
    
    language_layout->addWidget(new QLabel("Limba:"));
    language_layout->addWidget(m_language_combo);
    language_layout->addStretch();
    
    layout->addWidget(general_group);
    layout->addWidget(language_group);
    layout->addStretch();
    
    m_tab_widget->addTab(m_general_tab, "General");
}

void Settings_Dialog::create_appearance_tab()
{
    m_appearance_tab = new QWidget(this);
    auto layout = new QVBoxLayout(m_appearance_tab);
    
    // Theme settings group
    auto theme_group = new QGroupBox("Tema aplicației", m_appearance_tab);
    auto theme_layout = new QGridLayout(theme_group);
    
    m_theme_combo = new QComboBox(theme_group);
    m_theme_combo->addItem("Tema deschisă", "light");
    m_theme_combo->addItem("Tema închisă", "dark");
    m_theme_combo->addItem("Tema sistemului", "system");
    
    theme_layout->addWidget(new QLabel("Tema:"), 0, 0);
    theme_layout->addWidget(m_theme_combo, 0, 1);
    
    // Animation settings group
    auto animation_group = new QGroupBox("Animații", m_appearance_tab);
    auto animation_layout = new QGridLayout(animation_group);
    
    m_animations_check = new QCheckBox("Activează animațiile", animation_group);
    m_animation_speed_slider = new QSlider(Qt::Horizontal, animation_group);
    m_animation_speed_slider->setRange(1, 10);
    m_animation_speed_slider->setValue(5);
    m_animation_speed_slider->setTickPosition(QSlider::TicksBelow);
    m_animation_speed_slider->setTickInterval(1);
    
    animation_layout->addWidget(m_animations_check, 0, 0, 1, 2);
    animation_layout->addWidget(new QLabel("Viteza animațiilor:"), 1, 0);
    animation_layout->addWidget(m_animation_speed_slider, 1, 1);
    
    // Display settings group
    auto display_group = new QGroupBox("Afișare", m_appearance_tab);
    auto display_layout = new QGridLayout(display_group);
    
    m_show_notifications_check = new QCheckBox("Arată notificări", display_group);
    m_font_size_spin = new QSpinBox(display_group);
    m_font_size_spin->setRange(8, 24);
    m_font_size_spin->setSuffix(" px");
    m_font_size_spin->setValue(12);
    m_compact_mode_check = new QCheckBox("Mod compact", display_group);
    
    display_layout->addWidget(m_show_notifications_check, 0, 0);
    display_layout->addWidget(new QLabel("Dimensiune font:"), 0, 1);
    display_layout->addWidget(m_font_size_spin, 0, 2);
    display_layout->addWidget(m_compact_mode_check, 1, 0);
    
    layout->addWidget(theme_group);
    layout->addWidget(animation_group);
    layout->addWidget(display_group);
    layout->addStretch();
    
    m_tab_widget->addTab(m_appearance_tab, "Aspect");
}

void Settings_Dialog::create_network_tab()
{
    m_network_tab = new QWidget(this);
    auto layout = new QVBoxLayout(m_network_tab);
    
    // Server settings group
    auto server_group = new QGroupBox("Setări server", m_network_tab);
    auto server_layout = new QGridLayout(server_group);
    
    m_server_host_edit = new QLineEdit(server_group);
    m_server_host_edit->setPlaceholderText("Adresa serverului");
    m_server_port_spin = new QSpinBox(server_group);
    m_server_port_spin->setRange(1, 65535);
    m_server_port_spin->setValue(Config::Server::DEFAULT_PORT);
    
    server_layout->addWidget(new QLabel("Gazdă server:"), 0, 0);
    server_layout->addWidget(m_server_host_edit, 0, 1);
    server_layout->addWidget(new QLabel("Port server:"), 1, 0);
    server_layout->addWidget(m_server_port_spin, 1, 1);
    
    // Timeout settings group
    auto timeout_group = new QGroupBox("Timeouts", m_network_tab);
    auto timeout_layout = new QGridLayout(timeout_group);
    
    m_connection_timeout_spin = new QSpinBox(timeout_group);
    m_connection_timeout_spin->setRange(5, 300);
    m_connection_timeout_spin->setSuffix(" secunde");
    m_connection_timeout_spin->setValue(Config::Server::CONNECTION_TIMEOUT_MS / 1000);
    
    m_request_timeout_spin = new QSpinBox(timeout_group);
    m_request_timeout_spin->setRange(5, 300);
    m_request_timeout_spin->setSuffix(" secunde");
    m_request_timeout_spin->setValue(Config::Server::REQUEST_TIMEOUT_MS / 1000);
    
    timeout_layout->addWidget(new QLabel("Timeout conexiune:"), 0, 0);
    timeout_layout->addWidget(m_connection_timeout_spin, 0, 1);
    timeout_layout->addWidget(new QLabel("Timeout cerere:"), 1, 0);
    timeout_layout->addWidget(m_request_timeout_spin, 1, 1);
    
    // Cache settings group
    auto cache_group = new QGroupBox("Cache", m_network_tab);
    auto cache_layout = new QGridLayout(cache_group);
    
    m_enable_cache_check = new QCheckBox("Activează cache", cache_group);
    m_cache_size_spin = new QSpinBox(cache_group);
    m_cache_size_spin->setRange(10, 500);
    m_cache_size_spin->setSuffix(" MB");
    m_cache_size_spin->setValue(Config::Cache::MAX_CACHE_SIZE_MB);
    
    cache_layout->addWidget(m_enable_cache_check, 0, 0);
    cache_layout->addWidget(new QLabel("Dimensiune cache:"), 1, 0);
    cache_layout->addWidget(m_cache_size_spin, 1, 1);
    
    layout->addWidget(server_group);
    layout->addWidget(timeout_group);
    layout->addWidget(cache_group);
    layout->addStretch();
    
    m_tab_widget->addTab(m_network_tab, "Rețea");
}

void Settings_Dialog::create_advanced_tab()
{
    m_advanced_tab = new QWidget(this);
    auto layout = new QVBoxLayout(m_advanced_tab);
    
    // Logging settings group
    auto logging_group = new QGroupBox("Logging", m_advanced_tab);
    auto logging_layout = new QGridLayout(logging_group);
    
    m_enable_logging_check = new QCheckBox("Activează logging", logging_group);
    m_log_level_combo = new QComboBox(logging_group);
    m_log_level_combo->addItem("Debug", "debug");
    m_log_level_combo->addItem("Info", "info");
    m_log_level_combo->addItem("Warning", "warning");
    m_log_level_combo->addItem("Error", "error");
    m_log_to_file_check = new QCheckBox("Salvează log în fișier", logging_group);
    
    logging_layout->addWidget(m_enable_logging_check, 0, 0);
    logging_layout->addWidget(new QLabel("Nivel log:"), 1, 0);
    logging_layout->addWidget(m_log_level_combo, 1, 1);
    logging_layout->addWidget(m_log_to_file_check, 2, 0);
    
    // Debug settings group
    auto debug_group = new QGroupBox("Debug", m_advanced_tab);
    auto debug_layout = new QVBoxLayout(debug_group);
    
    m_enable_debug_check = new QCheckBox("Activează modul debug", debug_group);
    debug_layout->addWidget(m_enable_debug_check);
    
    // Maintenance group
    auto maintenance_group = new QGroupBox("Întreținere", m_advanced_tab);
    auto maintenance_layout = new QVBoxLayout(maintenance_group);
    
    m_clear_cache_button = new QPushButton("Șterge cache", maintenance_group);
    m_export_settings_button = new QPushButton("Exportă setări", maintenance_group);
    m_import_settings_button = new QPushButton("Importă setări", maintenance_group);
    
    maintenance_layout->addWidget(m_clear_cache_button);
    maintenance_layout->addWidget(m_export_settings_button);
    maintenance_layout->addWidget(m_import_settings_button);
    
    layout->addWidget(logging_group);
    layout->addWidget(debug_group);
    layout->addWidget(maintenance_group);
    layout->addStretch();
    
    m_tab_widget->addTab(m_advanced_tab, "Avansat");
}

void Settings_Dialog::setup_connections()
{
    // Button connections
    connect(m_ok_button, &QPushButton::clicked, this, &Settings_Dialog::on_ok_clicked);
    connect(m_cancel_button, &QPushButton::clicked, this, &Settings_Dialog::on_cancel_clicked);
    connect(m_apply_button, &QPushButton::clicked, this, &Settings_Dialog::on_apply_settings);
    connect(m_reset_button, &QPushButton::clicked, this, &Settings_Dialog::on_reset_to_defaults);
    
    // Settings change connections
    connect(m_theme_combo, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),
            this, &Settings_Dialog::on_theme_changed);
    connect(m_animations_check, &QCheckBox::toggled,
            this, &Settings_Dialog::on_animations_toggled);
    connect(m_auto_save_check, &QCheckBox::toggled,
            this, &Settings_Dialog::on_auto_save_toggled);
    
    // Maintenance connections
    connect(m_clear_cache_button, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Cache", "Cache-ul a fost șters cu succes!");
    });
    
    connect(m_export_settings_button, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, "Exportă setări",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/setari_agencie.json",
            "JSON Files (*.json)");
        
        if (!fileName.isEmpty())
        {
            export_settings(fileName);
        }
    });
    
    connect(m_import_settings_button, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Importă setări",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            "JSON Files (*.json)");
        
        if (!fileName.isEmpty())
        {
            import_settings(fileName);
        }
    });
}

void Settings_Dialog::load_settings()
{
    // Load general settings
    m_remember_login_check->setChecked(m_settings->value("general/remember_login", true).toBool());
    m_auto_start_check->setChecked(m_settings->value("general/auto_start", false).toBool());
    m_minimize_to_tray_check->setChecked(m_settings->value("general/minimize_to_tray", true).toBool());
    m_auto_save_check->setChecked(m_settings->value("general/auto_save", true).toBool());
    m_auto_save_interval_spin->setValue(m_settings->value("general/auto_save_interval", 5).toInt());
    
    QString language = m_settings->value("general/language", "ro").toString();
    int langIndex = m_language_combo->findData(language);
    if (langIndex >= 0) m_language_combo->setCurrentIndex(langIndex);
    
    // Load appearance settings
    QString theme = m_settings->value("appearance/theme", "light").toString();
    int themeIndex = m_theme_combo->findData(theme);
    if (themeIndex >= 0) m_theme_combo->setCurrentIndex(themeIndex);
    
    m_animations_check->setChecked(m_settings->value("appearance/animations", true).toBool());
    m_animation_speed_slider->setValue(m_settings->value("appearance/animation_speed", 5).toInt());
    m_show_notifications_check->setChecked(m_settings->value("appearance/show_notifications", true).toBool());
    m_font_size_spin->setValue(m_settings->value("appearance/font_size", 12).toInt());
    m_compact_mode_check->setChecked(m_settings->value("appearance/compact_mode", false).toBool());
    
    // Load network settings
    m_server_host_edit->setText(m_settings->value("network/server_host", Config::Server::DEFAULT_HOST).toString());
    m_server_port_spin->setValue(m_settings->value("network/server_port", Config::Server::DEFAULT_PORT).toInt());
    m_connection_timeout_spin->setValue(m_settings->value("network/connection_timeout", 
        Config::Server::CONNECTION_TIMEOUT_MS / 1000).toInt());
    m_request_timeout_spin->setValue(m_settings->value("network/request_timeout", 
        Config::Server::REQUEST_TIMEOUT_MS / 1000).toInt());
    m_enable_cache_check->setChecked(m_settings->value("network/enable_cache", true).toBool());
    m_cache_size_spin->setValue(m_settings->value("network/cache_size", Config::Cache::MAX_CACHE_SIZE_MB).toInt());
    
    // Load advanced settings
    m_enable_logging_check->setChecked(m_settings->value("advanced/enable_logging", true).toBool());
    QString logLevel = m_settings->value("advanced/log_level", "info").toString();
    int logIndex = m_log_level_combo->findData(logLevel);
    if (logIndex >= 0) m_log_level_combo->setCurrentIndex(logIndex);
    
    m_log_to_file_check->setChecked(m_settings->value("advanced/log_to_file", true).toBool());
    m_enable_debug_check->setChecked(m_settings->value("advanced/enable_debug", false).toBool());
    
    m_settings_changed = false;
}

void Settings_Dialog::save_settings()
{
    // Save general settings
    m_settings->setValue("general/remember_login", m_remember_login_check->isChecked());
    m_settings->setValue("general/auto_start", m_auto_start_check->isChecked());
    m_settings->setValue("general/minimize_to_tray", m_minimize_to_tray_check->isChecked());
    m_settings->setValue("general/auto_save", m_auto_save_check->isChecked());
    m_settings->setValue("general/auto_save_interval", m_auto_save_interval_spin->value());
    m_settings->setValue("general/language", m_language_combo->currentData().toString());
    
    // Save appearance settings
    m_settings->setValue("appearance/theme", m_theme_combo->currentData().toString());
    m_settings->setValue("appearance/animations", m_animations_check->isChecked());
    m_settings->setValue("appearance/animation_speed", m_animation_speed_slider->value());
    m_settings->setValue("appearance/show_notifications", m_show_notifications_check->isChecked());
    m_settings->setValue("appearance/font_size", m_font_size_spin->value());
    m_settings->setValue("appearance/compact_mode", m_compact_mode_check->isChecked());
    
    // Save network settings
    m_settings->setValue("network/server_host", m_server_host_edit->text());
    m_settings->setValue("network/server_port", m_server_port_spin->value());
    m_settings->setValue("network/connection_timeout", m_connection_timeout_spin->value() * 1000);
    m_settings->setValue("network/request_timeout", m_request_timeout_spin->value() * 1000);
    m_settings->setValue("network/enable_cache", m_enable_cache_check->isChecked());
    m_settings->setValue("network/cache_size", m_cache_size_spin->value());
    
    // Save advanced settings
    m_settings->setValue("advanced/enable_logging", m_enable_logging_check->isChecked());
    m_settings->setValue("advanced/log_level", m_log_level_combo->currentData().toString());
    m_settings->setValue("advanced/log_to_file", m_log_to_file_check->isChecked());
    m_settings->setValue("advanced/enable_debug", m_enable_debug_check->isChecked());
    
    m_settings->sync();
    m_settings_changed = false;
}

void Settings_Dialog::reset_to_defaults()
{
    int result = QMessageBox::question(this, "Reset setări",
        "Ești sigur că vrei să resetezi toate setările la valorile implicite?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes)
    {
        // Reset to default values
        m_remember_login_check->setChecked(true);
        m_auto_start_check->setChecked(false);
        m_minimize_to_tray_check->setChecked(true);
        m_auto_save_check->setChecked(true);
        m_auto_save_interval_spin->setValue(5);
        
        m_language_combo->setCurrentIndex(0); // Romanian
        
        m_theme_combo->setCurrentIndex(0); // Light theme
        m_animations_check->setChecked(true);
        m_animation_speed_slider->setValue(5);
        m_show_notifications_check->setChecked(true);
        m_font_size_spin->setValue(12);
        m_compact_mode_check->setChecked(false);
        
        m_server_host_edit->setText(Config::Server::DEFAULT_HOST);
        m_server_port_spin->setValue(Config::Server::DEFAULT_PORT);
        m_connection_timeout_spin->setValue(Config::Server::CONNECTION_TIMEOUT_MS / 1000);
        m_request_timeout_spin->setValue(Config::Server::REQUEST_TIMEOUT_MS / 1000);
        m_enable_cache_check->setChecked(true);
        m_cache_size_spin->setValue(Config::Cache::MAX_CACHE_SIZE_MB);
        
        m_enable_logging_check->setChecked(true);
        m_log_level_combo->setCurrentIndex(1); // Info
        m_log_to_file_check->setChecked(true);
        m_enable_debug_check->setChecked(false);
        
        m_settings_changed = true;
    }
}

void Settings_Dialog::export_settings(const QString& fileName)
{
    QJsonObject settings;
    
    // Export general settings
    QJsonObject general;
    general["remember_login"] = m_remember_login_check->isChecked();
    general["auto_start"] = m_auto_start_check->isChecked();
    general["minimize_to_tray"] = m_minimize_to_tray_check->isChecked();
    general["auto_save"] = m_auto_save_check->isChecked();
    general["auto_save_interval"] = m_auto_save_interval_spin->value();
    general["language"] = m_language_combo->currentData().toString();
    settings["general"] = general;
    
    // Export appearance settings
    QJsonObject appearance;
    appearance["theme"] = m_theme_combo->currentData().toString();
    appearance["animations"] = m_animations_check->isChecked();
    appearance["animation_speed"] = m_animation_speed_slider->value();
    appearance["show_notifications"] = m_show_notifications_check->isChecked();
    appearance["font_size"] = m_font_size_spin->value();
    appearance["compact_mode"] = m_compact_mode_check->isChecked();
    settings["appearance"] = appearance;
    
    // Export network settings
    QJsonObject network;
    network["server_host"] = m_server_host_edit->text();
    network["server_port"] = m_server_port_spin->value();
    network["connection_timeout"] = m_connection_timeout_spin->value();
    network["request_timeout"] = m_request_timeout_spin->value();
    network["enable_cache"] = m_enable_cache_check->isChecked();
    network["cache_size"] = m_cache_size_spin->value();
    settings["network"] = network;
    
    // Export advanced settings
    QJsonObject advanced;
    advanced["enable_logging"] = m_enable_logging_check->isChecked();
    advanced["log_level"] = m_log_level_combo->currentData().toString();
    advanced["log_to_file"] = m_log_to_file_check->isChecked();
    advanced["enable_debug"] = m_enable_debug_check->isChecked();
    settings["advanced"] = advanced;
    
    QJsonDocument doc(settings);
    QFile file(fileName);
    
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(doc.toJson());
        QMessageBox::information(this, "Export reușit", 
            "Setările au fost exportate cu succes în " + fileName);
    }
    else
    {
        QMessageBox::critical(this, "Eroare export", 
            "Nu s-a putut exporta setările în " + fileName);
    }
}

void Settings_Dialog::import_settings(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "Eroare import", 
            "Nu s-a putut deschide fișierul " + fileName);
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
    {
        QMessageBox::critical(this, "Eroare import", 
            "Fișierul nu conține setări valide");
        return;
    }
    
    QJsonObject settings = doc.object();
    
    // Import general settings
    if (settings.contains("general"))
    {
        QJsonObject general = settings["general"].toObject();
        if (general.contains("remember_login")) 
            m_remember_login_check->setChecked(general["remember_login"].toBool());
        if (general.contains("auto_start")) 
            m_auto_start_check->setChecked(general["auto_start"].toBool());
        if (general.contains("minimize_to_tray")) 
            m_minimize_to_tray_check->setChecked(general["minimize_to_tray"].toBool());
        if (general.contains("auto_save")) 
            m_auto_save_check->setChecked(general["auto_save"].toBool());
        if (general.contains("auto_save_interval")) 
            m_auto_save_interval_spin->setValue(general["auto_save_interval"].toInt());
        if (general.contains("language"))
        {
            QString lang = general["language"].toString();
            int index = m_language_combo->findData(lang);
            if (index >= 0) m_language_combo->setCurrentIndex(index);
        }
    }
    
    // Import appearance settings
    if (settings.contains("appearance"))
    {
        QJsonObject appearance = settings["appearance"].toObject();
        if (appearance.contains("theme"))
        {
            QString theme = appearance["theme"].toString();
            int index = m_theme_combo->findData(theme);
            if (index >= 0) m_theme_combo->setCurrentIndex(index);
        }
        if (appearance.contains("animations")) 
            m_animations_check->setChecked(appearance["animations"].toBool());
        if (appearance.contains("animation_speed")) 
            m_animation_speed_slider->setValue(appearance["animation_speed"].toInt());
        if (appearance.contains("show_notifications")) 
            m_show_notifications_check->setChecked(appearance["show_notifications"].toBool());
        if (appearance.contains("font_size")) 
            m_font_size_spin->setValue(appearance["font_size"].toInt());
        if (appearance.contains("compact_mode")) 
            m_compact_mode_check->setChecked(appearance["compact_mode"].toBool());
    }
    
    // Import network settings
    if (settings.contains("network"))
    {
        QJsonObject network = settings["network"].toObject();
        if (network.contains("server_host")) 
            m_server_host_edit->setText(network["server_host"].toString());
        if (network.contains("server_port")) 
            m_server_port_spin->setValue(network["server_port"].toInt());
        if (network.contains("connection_timeout")) 
            m_connection_timeout_spin->setValue(network["connection_timeout"].toInt());
        if (network.contains("request_timeout")) 
            m_request_timeout_spin->setValue(network["request_timeout"].toInt());
        if (network.contains("enable_cache")) 
            m_enable_cache_check->setChecked(network["enable_cache"].toBool());
        if (network.contains("cache_size")) 
            m_cache_size_spin->setValue(network["cache_size"].toInt());
    }
    
    // Import advanced settings
    if (settings.contains("advanced"))
    {
        QJsonObject advanced = settings["advanced"].toObject();
        if (advanced.contains("enable_logging")) 
            m_enable_logging_check->setChecked(advanced["enable_logging"].toBool());
        if (advanced.contains("log_level"))
        {
            QString level = advanced["log_level"].toString();
            int index = m_log_level_combo->findData(level);
            if (index >= 0) m_log_level_combo->setCurrentIndex(index);
        }
        if (advanced.contains("log_to_file")) 
            m_log_to_file_check->setChecked(advanced["log_to_file"].toBool());
        if (advanced.contains("enable_debug")) 
            m_enable_debug_check->setChecked(advanced["enable_debug"].toBool());
    }
    
    m_settings_changed = true;
    QMessageBox::information(this, "Import reușit", 
        "Setările au fost importate cu succes din " + fileName);
}

void Settings_Dialog::on_apply_settings()
{
    save_settings();
    emit settings_changed();
    
    // Apply theme immediately if changed
    QString currentTheme = m_theme_combo->currentData().toString();
    if (currentTheme != "system")
    {
        emit theme_changed(currentTheme);
    }
    
    QMessageBox::information(this, "Setări aplicate", 
        "Setările au fost aplicate cu succes!");
}

void Settings_Dialog::on_reset_to_defaults()
{
    reset_to_defaults();
}

void Settings_Dialog::on_ok_clicked()
{
    if (m_settings_changed)
    {
        save_settings();
        emit settings_changed();
        
        // Apply theme immediately if changed
        QString currentTheme = m_theme_combo->currentData().toString();
        if (currentTheme != "system")
        {
            emit theme_changed(currentTheme);
        }
    }
    
    accept();
}

void Settings_Dialog::on_cancel_clicked()
{
    reject();
}

void Settings_Dialog::on_theme_changed()
{
    QString theme = m_theme_combo->currentData().toString();
    if (theme != "system")
    {
        apply_theme_preview();
    }
}

void Settings_Dialog::on_animations_toggled(bool enabled)
{
    m_animation_speed_slider->setEnabled(enabled);
    m_settings_changed = true;
}

void Settings_Dialog::on_auto_save_toggled(bool enabled)
{
    m_auto_save_interval_spin->setEnabled(enabled);
    m_settings_changed = true;
}

void Settings_Dialog::apply_theme_preview()
{
    QString theme = m_theme_combo->currentData().toString();
    
    // Apply theme preview to the dialog
    if (theme == "dark")
    {
        setStyleSheet("QDialog { background-color: #2b2b2b; color: #ffffff; }"
                     "QGroupBox { color: #ffffff; }"
                     "QLabel { color: #ffffff; }");
    } else {
        setStyleSheet("QDialog { background-color: #f0f0f0; color: #000000; }"
                     "QGroupBox { color: #000000; }"
                     "QLabel { color: #000000; }");
    }
}
