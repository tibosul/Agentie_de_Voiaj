#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>

// Simple test to verify our fixes work
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Testing client fixes...";
    
    QTcpSocket socket;
    
    // Test 1: Non-blocking connection
    qDebug() << "Test 1: Testing non-blocking connection to 127.0.0.1:8080";
    
    QObject::connect(&socket, &QTcpSocket::connected, [&]() {
        qDebug() << "✅ Connected successfully!";
        
        // Test 2: Send login message
        QJsonObject loginMsg;
        loginMsg["type"] = "AUTH";
        loginMsg["username"] = "test_user";
        loginMsg["password"] = "wrong_password";
        
        QJsonDocument doc(loginMsg);
        QByteArray data = doc.toJson(QJsonDocument::Compact) + "\r\n";
        
        socket.write(data);
        socket.flush();
        qDebug() << "✅ Sent login message (non-blocking)";
    });
    
    QObject::connect(&socket, &QTcpSocket::readyRead, [&]() {
        QByteArray data = socket.readAll();
        qDebug() << "✅ Received response:" << data;
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data.trimmed(), &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonObject response = doc.object();
            qDebug() << "✅ JSON parsed successfully:" << response["message"].toString();
        } else {
            qDebug() << "❌ JSON parse error:" << error.errorString();
        }
        
        // Exit after receiving response
        QTimer::singleShot(500, &app, &QCoreApplication::quit);
    });
    
    QObject::connect(&socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
        [&](QAbstractSocket::SocketError error) {
            qDebug() << "❌ Socket error:" << socket.errorString();
            QTimer::singleShot(100, &app, &QCoreApplication::quit);
        });
    
    // Start connection (should be non-blocking now)
    socket.connectToHost("127.0.0.1", 8080);
    
    // Timeout after 10 seconds
    QTimer::singleShot(10000, [&]() {
        qDebug() << "❌ Test timeout";
        app.quit();
    });
    
    qDebug() << "Connection initiated, waiting for result...";
    return app.exec();
}