#pragma once
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <memory>

class Logger
{
public:
    enum class Level 
    {
        Debug = 0,
        Info = 1,
        Warning = 2,
        Error = 3,
        Critical = 4
    };

    explicit Logger(const QString& logDirectory = "");
    ~Logger();

    // Initialization
    bool initialize();
    void setLogLevel(Level level);
    void setLogToFile(bool enabled);
    void setLogToConsole(bool enabled);

    // Logging methods
    void debug(const QString& message);
    void info(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);
    void critical(const QString& message);

    // Generic log method
    void log(Level level, const QString& message);

    // Utility methods
    static QString levelToString(Level level);
    QString getCurrentLogFileName() const;

private:
    void writeToFile(const QString& formattedMessage);
    void writeToConsole(const QString& formattedMessage);
    QString formatMessage(Level level, const QString& message) const;
    void rotateLogFile();
    bool createLogFile();

    QString m_logDirectory;
    QString m_currentLogFile;
    std::unique_ptr<QFile> m_logFile;
    std::unique_ptr<QTextStream> m_logStream;
    
    Level m_logLevel;
    bool m_logToFile;
    bool m_logToConsole;
    bool m_isInitialized;
    
    mutable QMutex m_mutex;
    
    static constexpr int MAX_LOG_FILE_SIZE = 10 * 1024 * 1024; // 10MB
};