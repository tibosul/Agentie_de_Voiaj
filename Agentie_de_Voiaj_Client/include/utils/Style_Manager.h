#pragma once
#include <QString>
#include <QApplication>

class Style_Manager
{
public:
    Style_Manager();
    ~Style_Manager();

    // Theme management
    void load_theme(const QString& themeName);
    void set_dark_theme();
    void set_light_theme();
    void toggle_theme();
    
    // Getters
    QString get_current_theme() const;
    bool is_dark_theme() const;

    // Static helpers
    static QString load_style_sheet(const QString& fileName);
    static void apply_style_sheet(const QString& styleSheet);

private:
    void apply_theme(const QString& themeName);

    QString m_currentTheme;
    bool m_isDarkTheme;
};