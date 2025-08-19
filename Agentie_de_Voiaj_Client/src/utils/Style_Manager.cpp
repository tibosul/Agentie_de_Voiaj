#include "utils/Style_Manager.h"
#include "config.h"

#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QDebug>

Style_Manager::Style_Manager()
    : m_currentTheme(Config::UI::DEFAULT_THEME)
    , m_isDarkTheme(false)
{
}

Style_Manager::~Style_Manager() = default;

void Style_Manager::load_theme(const QString& theme_name)
{
    if (theme_name == m_currentTheme)
    {
        return;
    }

    apply_theme(theme_name);
}

void Style_Manager::set_dark_theme()
{
    apply_theme("dark");
}

void Style_Manager::set_light_theme()
{
    apply_theme("light");
}

void Style_Manager::toggle_theme()
{
    if (m_isDarkTheme)
    {
        set_light_theme();
    }
    else
    {
        set_dark_theme();
    }
}

QString Style_Manager::get_current_theme() const
{
    return m_currentTheme;
}

bool Style_Manager::is_dark_theme() const
{
    return m_isDarkTheme;
}

QString Style_Manager::load_style_sheet(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to load stylesheet:" << filename;
        return QString();
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    return stream.readAll();
}

void Style_Manager::apply_style_sheet(const QString& style_sheet)
{
    if (QApplication* app = qApp)
    {
        app->setStyleSheet(style_sheet);
    }
}

void Style_Manager::apply_theme(const QString& theme_name)
{
    QString style_sheet_path = QString(":/styles/%1_theme.qss").arg(theme_name);
    QString style_sheet = load_style_sheet(style_sheet_path);

    if (!style_sheet.isEmpty())
    {
        apply_style_sheet(style_sheet);
        m_currentTheme = theme_name;
        m_isDarkTheme = (theme_name == "dark");

        qDebug() << "Applied theme:" << theme_name;
    }
    else
    {
        qWarning() << "Failed to load theme:" << theme_name;
    }
}