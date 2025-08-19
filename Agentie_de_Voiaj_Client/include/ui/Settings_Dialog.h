#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QSettings>
#include <memory>

class Settings_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Settings_Dialog(QWidget* parent = nullptr);
    ~Settings_Dialog();

    static void show_settings_dialog(QWidget* parent = nullptr);

signals:
    void settings_changed();
    void theme_changed(const QString& theme_name);
    void language_changed(const QString& language);

private slots:
    void on_apply_settings();
    void on_reset_to_defaults();
    void on_ok_clicked();
    void on_cancel_clicked();

    void on_theme_changed();
    void on_animations_toggled(bool enabled);
    void on_auto_save_toggled(bool enabled);

private:
    void setup_ui();
    void create_general_tab();
    void create_appearance_tab();
    void create_network_tab();
    void create_advanced_tab();
    void setup_connections();

    void load_settings();
    void save_settings();
    void reset_to_defaults();

    void apply_theme_preview();

    QTabWidget* m_tab_widget;

    QWidget* m_general_tab;
    QCheckBox* m_remember_login_check;
    QCheckBox* m_auto_start_check;
    QCheckBox* m_minimize_to_tray_check;
    QComboBox* m_language_combo;
    QCheckBox* m_auto_save_check;
    QSpinBox* m_auto_save_interval_spin;

    QWidget* m_appearance_tab;
    QComboBox* m_theme_combo;
    QCheckBox* m_animations_check;
    QSlider* m_animation_speed_slider;
    QCheckBox* m_show_notifications_check;
    QSpinBox* m_font_size_spin;
    QCheckBox* m_compact_mode_check;

    QWidget* m_network_tab;
    QLineEdit* m_server_host_edit;
    QSpinBox* m_server_port_spin;
    QSpinBox* m_connection_timeout_spin;
    QSpinBox* m_request_timeout_spin;
    QCheckBox* m_enable_cache_check;
    QSpinBox* m_cache_size_spin;

    QWidget* m_advanced_tab;
    QCheckBox* m_enable_logging_check;
    QComboBox* m_log_level_combo;
    QCheckBox* m_log_to_file_check;
    QCheckBox* m_enable_debug_check;
    QPushButton* m_clear_cache_button;
    QPushButton* m_export_settings_button;
    QPushButton* m_import_settings_button;

    QPushButton* m_ok_button;
    QPushButton* m_cancel_button;
    QPushButton* m_apply_button;
    QPushButton* m_reset_button;

    std::unique_ptr<QSettings> m_settings;
    bool m_settings_changed;
};