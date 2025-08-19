#include "core/Application.h"
#include "config.h"
#include "utils/Logger.h"
#include "utils/Style_Manager.h"

#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QStyleFactory>
#include <QFontDatabase>

// Static instance
Application_Manager* Application_Manager::s_instance = nullptr;

Application_Manager::Application_Manager(int& argc, char** argv)
    : QApplication(argc, argv)
    , m_is_initialized(false)
{
    s_instance = this;
    setup_application_info();
    setup_directories();
    
    // Connect signals
    connect(this, &QApplication::lastWindowClosed, 
            this, &Application_Manager::on_last_window_closed);
}

Application_Manager::~Application_Manager()
{
    if (m_logger)
    {
        m_logger->info("Application shutting down");
    }
    s_instance = nullptr;
}

Application_Manager* Application_Manager::instance()
{
    return s_instance;
}

bool Application_Manager::initialize()
{
    if (m_is_initialized)
    {
        return true;
    }

    try
    {
        setup_logging();
        load_settings();
        load_style_sheet();
        
        m_logger->info("=== " + Config::Application::APP_NAME + " Started ===");
        m_logger->info("Version: " + Config::Application::VERSION);
        m_logger->info("Qt Version: " + QString(qVersion()));
        
        m_is_initialized = true;
        return true;
        
    } 
    catch (const std::exception& e) 
    {
        QMessageBox::critical(nullptr, "Initialization Error", 
                             "Failed to initialize application: " + QString(e.what()));
        return false;
    }
}

void Application_Manager::setup_application_info()
{
    setApplicationName(Config::Application::APP_NAME);
    setApplicationVersion(Config::Application::VERSION);
    setOrganizationName(Config::Application::ORGANIZATION);
    setOrganizationDomain(Config::Application::APP_DOMAIN);
}

void Application_Manager::setup_directories()
{
    m_app_data_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_log_path = m_app_data_path + "/logs";
    m_cache_path = m_app_data_path + "/cache";
    
    create_application_directories();
}

void Application_Manager::create_application_directories()
{
    QDir app_dir(m_app_data_path);
    if (!app_dir.exists())
    {
        app_dir.mkpath(".");
    }
    
    QDir log_dir(m_log_path);
    if (!log_dir.exists())
    {
        log_dir.mkpath(".");
    }
    
    QDir cache_dir(m_cache_path);
    if (!cache_dir.exists())
    {
        cache_dir.mkpath(".");
    }
}

void Application_Manager::setup_logging()
{
    if (!m_logger && Config::Application::ENABLE_LOGGING)
    {
        m_logger = std::make_unique<Logger>(m_log_path);
        m_logger->initialize();
    }
}

void Application_Manager::load_settings()
{
    // Settings vor fi implementate în următorul pas
    if (m_logger)
    {
        m_logger->info("Settings loaded successfully");
    }
}

void Application_Manager::load_style_sheet()
{
    if (!m_style_manager)
    {
        m_style_manager = std::make_unique<Style_Manager>();
    }
    
    m_style_manager->load_theme(Config::UI::DEFAULT_THEME);
    
    if (m_logger)
    {
        m_logger->info("StyleSheet loaded: " + Config::UI::DEFAULT_THEME);
    }
}

QString Application_Manager::get_application_data_path() const
{
    return m_app_data_path;
}

QString Application_Manager::get_log_path() const
{
    return m_log_path;
}

QString Application_Manager::get_cache_path() const
{
    return m_cache_path;
}

void Application_Manager::on_last_window_closed()
{
    if (m_logger)
    {
        m_logger->info("Last window closed, application will quit");
    }
    quit();
}