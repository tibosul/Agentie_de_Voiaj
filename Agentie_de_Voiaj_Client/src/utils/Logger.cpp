#include "utils/Logger.h"
#include "config.h"

#include <QDir>
#include <QDebug>
#include <QMutexLocker>
#include <iostream>

Logger::Logger(const QString& logDirectory)
    : m_logDirectory(logDirectory.isEmpty() ? "logs" : logDirectory)
    , m_logLevel(Level::Debug)
    , m_logToFile(true)
    , m_logToConsole(true)
    , m_isInitialized(false)
{
}

Logger::~Logger()
{
    if (m_logStream)
    {
        m_logStream->flush();
    }
}

bool Logger::initialize()
{
    if (m_isInitialized)
    {
        return true;
    }

    // Create log directory if it doesn't exist
    QDir logDir(m_logDirectory);
    if (!logDir.exists())
    {
        if (!logDir.mkpath("."))
        {
            std::cerr << "Failed to create log directory: " 
                      << m_logDirectory.toStdString() << std::endl;
            return false;
        }
    }

    if (m_logToFile)
    {
        if (!createLogFile())
        {
            std::cerr << "Failed to create log file" << std::endl;
            return false;
        }
    }

    m_isInitialized = true;
    info("Logger initialized successfully");
    return true;
}

void Logger::setLogLevel(Level level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
}

void Logger::setLogToFile(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_logToFile = enabled;
    
    if (enabled && !m_logFile)
    {
        createLogFile();
    }
}

void Logger::setLogToConsole(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_logToConsole = enabled;
}

void Logger::debug(const QString& message)
{
    log(Level::Debug, message);
}

void Logger::info(const QString& message)
{
    log(Level::Info, message);
}

void Logger::warning(const QString& message)
{
    log(Level::Warning, message);
}

void Logger::error(const QString& message)
{
    log(Level::Error, message);
}

void Logger::critical(const QString& message)
{
    log(Level::Critical, message);
}

void Logger::log(Level level, const QString& message)
{
    if (!m_isInitialized || level < m_logLevel)
    {
        return;
    }

    QMutexLocker locker(&m_mutex);
    
    QString formattedMessage = formatMessage(level, message);
    
    if (m_logToConsole) 
    {
        writeToConsole(formattedMessage);
    }
    
    if (m_logToFile && m_logFile)
    {
        writeToFile(formattedMessage);
        
        // Check if log rotation is needed
        if (m_logFile->size() > MAX_LOG_FILE_SIZE)
        {
            rotateLogFile();
        }
    }
}

QString Logger::levelToString(Level level)
{
    switch (level)
    {
        case Level::Debug:    return "DEBUG";
        case Level::Info:     return "INFO";
        case Level::Warning:  return "WARNING";
        case Level::Error:    return "ERROR";
        case Level::Critical: return "CRITICAL";
        default:              return "UNKNOWN";
    }
}

QString Logger::getCurrentLogFileName() const
{
    return m_currentLogFile;
}

void Logger::writeToFile(const QString& formattedMessage)
{
    if (m_logStream)
    {
        *m_logStream << formattedMessage << Qt::endl;
        m_logStream->flush();
    }
}

void Logger::writeToConsole(const QString& formattedMessage)
{
    // Use QDebug for proper Qt console output
    qDebug().noquote() << formattedMessage;
}

QString Logger::formatMessage(Level level, const QString& message) const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = levelToString(level).rightJustified(8);
    
    return QString("[%1] [%2] %3")
           .arg(timestamp)
           .arg(levelStr)
           .arg(message);
}

void Logger::rotateLogFile()
{
    if (m_logStream)
    {
        m_logStream.reset();
    }
    
    if (m_logFile)
    {
        m_logFile->close();
        
        // Rename current file with timestamp
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString oldFileName = m_currentLogFile;
        QString newFileName = oldFileName;
        newFileName.replace(".log", QString("_%1.log").arg(timestamp));
        
        QFile::rename(oldFileName, newFileName);
        m_logFile.reset();
    }
    
    // Create new log file
    createLogFile();
}

bool Logger::createLogFile()
{
    QString dateStr = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString fileName = QString("client_%1.log").arg(dateStr);
    m_currentLogFile = QDir(m_logDirectory).absoluteFilePath(fileName);
    
    m_logFile = std::make_unique<QFile>(m_currentLogFile);
    
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append))
    {
        m_logFile.reset();
        return false;
    }
    
    m_logStream = std::make_unique<QTextStream>(m_logFile.get());
    m_logStream->setEncoding(QStringConverter::Utf8);
    
    return true;
}