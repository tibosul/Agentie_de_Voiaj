#pragma once
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <memory>

// Forward declarations
class QTranslator;
class Logger;
class Style_Manager;

class Application_Manager : public QApplication
{
    Q_OBJECT

public:
    Application_Manager(int& argc, char** argv);
    ~Application_Manager();

    // Static instance getter
    static Application_Manager* instance();

    // Application lifecycle
    bool initialize();
    void setup_directories();
    void setup_logging();
    void load_settings();
    void load_style_sheet();

    // Getters
    QString get_application_data_path() const;
    QString get_log_path() const;
    QString get_cache_path() const;

private slots:
    void on_last_window_closed();

private:
    void setup_application_info();
    void create_application_directories();
    
    static Application_Manager* s_instance;
    
    // Paths
    QString m_app_data_path;
    QString m_log_path;
    QString m_cache_path;
    
    // Components
    std::unique_ptr<Logger> m_logger;
    std::unique_ptr<Style_Manager> m_style_manager;
    
    bool m_is_initialized;
};