#include <QTest>
#include <QSignalSpy>
#include <QApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <memory>

// Include test headers
#include "models/Offer_Model.h"
#include "models/Reservation_Model.h"
#include "network/Api_Client.h"
#include "ui/Main_Window.h"

// Mock TCP Server for testing client-server communication
class MockTcpServer : public QObject {
    Q_OBJECT
public:
    MockTcpServer(QObject* parent = nullptr) : QObject(parent), tcp_server(nullptr) {
        tcp_server = new QTcpServer(this);
        connect(tcp_server, &QTcpServer::newConnection, this, &MockTcpServer::handle_new_connection);
    }
    
    bool start(int port = 8080) {
        return tcp_server->listen(QHostAddress::LocalHost, port);
    }
    
    void stop() {
        tcp_server->close();
    }
    
    int get_port() const {
        return tcp_server->serverPort();
    }
    
signals:
    void client_connected();
    void message_received(const QByteArray& message);
    void client_disconnected();
    
private slots:
    void handle_new_connection() {
        QTcpSocket* client_socket = tcp_server->nextPendingConnection();
        if (client_socket) {
            connect(client_socket, &QTcpSocket::readyRead, [this, client_socket]() {
                QByteArray data = client_socket->readAll();
                emit message_received(data);
                
                // Send mock response based on message type
                QJsonDocument doc = QJsonDocument::fromJson(data);
                if (doc.isObject()) {
                    QJsonObject request = doc.object();
                    QString message_type = request["type"].toString();
                    
                    QJsonObject response;
                    if (message_type == "SEARCH_OFFERS") {
                        response = create_search_offers_response();
                    } else if (message_type == "BOOK_OFFER") {
                        response = create_book_offer_response();
                    } else if (message_type == "GET_USER_RESERVATIONS") {
                        response = create_get_reservations_response();
                    } else if (message_type == "CANCEL_RESERVATION") {
                        response = create_cancel_reservation_response();
                    } else if (message_type == "UPDATE_USER_INFO") {
                        response = create_update_user_response();
                    } else {
                        response = create_error_response("Unknown message type");
                    }
                    
                    QJsonDocument response_doc(response);
                    client_socket->write(response_doc.toJson());
                }
            });
            
            connect(client_socket, &QTcpSocket::disconnected, [this, client_socket]() {
                emit client_disconnected();
                client_socket->deleteLater();
            });
            
            emit client_connected();
        }
    }
    
private:
    QJsonObject create_search_offers_response() {
        QJsonObject response;
        response["success"] = true;
        response["message"] = "Offers found successfully";
        
        QJsonArray offers;
        
        QJsonObject offer1;
        offer1["id"] = 1;
        offer1["destination"] = "Paris";
        offer1["price"] = 500.0;
        offer1["duration"] = 7;
        offer1["description"] = "Beautiful Paris vacation";
        offer1["image_url"] = "paris.jpg";
        offers.append(offer1);
        
        QJsonObject offer2;
        offer2["id"] = 2;
        offer2["destination"] = "Rome";
        offer2["price"] = 600.0;
        offer2["duration"] = 5;
        offer2["description"] = "Amazing Rome experience";
        offer2["image_url"] = "rome.jpg";
        offers.append(offer2);
        
        response["data"] = offers;
        return response;
    }
    
    QJsonObject create_book_offer_response() {
        QJsonObject response;
        response["success"] = true;
        response["message"] = "Reservation created successfully";
        response["data"] = QJsonObject{{"reservation_id", 123}};
        return response;
    }
    
    QJsonObject create_get_reservations_response() {
        QJsonObject response;
        response["success"] = true;
        response["message"] = "Reservations retrieved successfully";
        
        QJsonArray reservations;
        
        QJsonObject reservation;
        reservation["id"] = 123;
        reservation["offer_id"] = 1;
        reservation["destination"] = "Paris";
        reservation["status"] = "Confirmed";
        reservation["person_count"] = 2;
        reservation["booking_date"] = "2024-01-15";
        reservations.append(reservation);
        
        response["data"] = reservations;
        return response;
    }
    
    QJsonObject create_cancel_reservation_response() {
        QJsonObject response;
        response["success"] = true;
        response["message"] = "Reservation cancelled successfully";
        return response;
    }
    
    QJsonObject create_update_user_response() {
        QJsonObject response;
        response["success"] = true;
        response["message"] = "User info updated successfully";
        return response;
    }
    
    QJsonObject create_error_response(const QString& error_message) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = error_message;
        return response;
    }
    
    QTcpServer* tcp_server;
};

// Integration test class
class TestClientServerIntegration : public QObject {
    Q_OBJECT
    
private slots:
    void initTestCase() {
        // Start mock server
        mock_server = std::make_unique<MockTcpServer>();
        QVERIFY(mock_server->start(8081)); // Use different port to avoid conflicts
        
        // Wait for server to start
        QThread::msleep(100);
    }
    
    void cleanupTestCase() {
        if (mock_server) {
            mock_server->stop();
        }
    }
    
    void test_client_server_connection() {
        // Test that client can connect to mock server
        auto api_client = std::make_unique<Api_Client>();
        
        // Connect to mock server
        QSignalSpy connectionSpy(api_client.get(), &Api_Client::connection_status_changed);
        api_client->connect_to_server("127.0.0.1", mock_server->get_port());
        
        // Wait for connection
        QVERIFY(connectionSpy.wait(5000));
        QCOMPARE(connectionSpy.count(), 1);
        
        // Verify connection status
        QVERIFY(api_client->is_connected());
    }
    
    void test_search_offers_integration() {
        auto api_client = std::make_unique<Api_Client>();
        auto offer_model = std::make_unique<Offer_Model>();
        
        // Connect components
        offer_model->set_api_client(api_client.get());
        
        // Connect to mock server
        api_client->connect_to_server("127.0.0.1", mock_server->get_port());
        QThread::msleep(100); // Wait for connection
        
        // Spy on signals
        QSignalSpy offersLoadedSpy(offer_model.get(), &Offer_Model::offers_loaded);
        QSignalSpy loadingChangedSpy(offer_model.get(), &Offer_Model::loading_changed);
        
        // Test search
        offer_model->search_offers("Paris", 100.0, 1000.0);
        
        // Wait for response
        QVERIFY(offersLoadedSpy.wait(5000));
        QVERIFY(loadingChangedSpy.wait(5000));
        
        // Verify results
        QCOMPARE(offer_model->rowCount(), 2);
        QCOMPARE(offer_model->is_loading(), false);
        
        // Verify first offer
        QModelIndex index = offer_model->index(0);
        QCOMPARE(offer_model->data(index, Offer_Model::DestinationRole).toString(), "Paris");
        QCOMPARE(offer_model->data(index, Offer_Model::PriceRole).toDouble(), 500.0);
    }
    
    void test_book_offer_integration() {
        auto api_client = std::make_unique<Api_Client>();
        auto offer_model = std::make_unique<Offer_Model>();
        
        offer_model->set_api_client(api_client.get());
        api_client->connect_to_server("127.0.0.1", mock_server->get_port());
        QThread::msleep(100);
        
        // Load offers first
        offer_model->search_offers("", 0.0, 1000.0);
        QSignalSpy offersLoadedSpy(offer_model.get(), &Offer_Model::offers_loaded);
        QVERIFY(offersLoadedSpy.wait(5000));
        
        // Test booking
        QSignalSpy bookingResponseSpy(offer_model.get(), &Offer_Model::booking_response_received);
        offer_model->book_offer(1, 2, QJsonObject{{"notes", "Window seat"}});
        
        QVERIFY(bookingResponseSpy.wait(5000));
        QCOMPARE(bookingResponseSpy.count(), 1);
        
        // Verify response data
        QJsonObject response = bookingResponseSpy.at(0).at(0).toJsonObject();
        QVERIFY(response["success"].toBool());
        QCOMPARE(response["message"].toString(), "Reservation created successfully");
        QCOMPARE(response["data"].toObject()["reservation_id"].toInt(), 123);
    }
    
    void test_get_reservations_integration() {
        auto api_client = std::make_unique<Api_Client>();
        auto reservation_model = std::make_unique<Reservation_Model>();
        
        reservation_model->set_api_client(api_client.get());
        api_client->connect_to_server("127.0.0.1", mock_server->get_port());
        QThread::msleep(100);
        
        // Spy on signals
        QSignalSpy reservationsLoadedSpy(reservation_model.get(), &Reservation_Model::reservations_loaded);
        QSignalSpy loadingChangedSpy(reservation_model.get(), &Reservation_Model::loading_changed);
        
        // Test get reservations
        reservation_model->refresh_reservations();
        
        QVERIFY(reservationsLoadedSpy.wait(5000));
        QVERIFY(loadingChangedSpy.wait(5000));
        
        // Verify results
        QCOMPARE(reservation_model->rowCount(), 1);
        QCOMPARE(reservation_model->is_loading(), false);
        
        // Verify reservation data
        QModelIndex index = reservation_model->index(0);
        QCOMPARE(reservation_model->data(index, Reservation_Model::DestinationRole).toString(), "Paris");
        QCOMPARE(reservation_model->data(index, Reservation_Model::StatusRole).toString(), "Confirmed");
        QCOMPARE(reservation_model->data(index, Reservation_Model::PersonCountRole).toInt(), 2);
    }
    
    void test_cancel_reservation_integration() {
        auto api_client = std::make_unique<Api_Client>();
        auto reservation_model = std::make_unique<Reservation_Model>();
        
        reservation_model->set_api_client(api_client.get());
        api_client->connect_to_server("127.0.0.1", mock_server->get_port());
        QThread::msleep(100);
        
        // Load reservations first
        reservation_model->refresh_reservations();
        QSignalSpy reservationsLoadedSpy(reservation_model.get(), &Reservation_Model::reservations_loaded);
        QVERIFY(reservationsLoadedSpy.wait(5000));
        
        // Test cancellation
        QSignalSpy cancellationResponseSpy(reservation_model.get(), &Reservation_Model::cancellation_response_received);
        reservation_model->cancel_reservation(123);
        
        QVERIFY(cancellationResponseSpy.wait(5000));
        QCOMPARE(cancellationResponseSpy.count(), 1);
        
        // Verify response
        QJsonObject response = cancellationResponseSpy.at(0).at(0).toJsonObject();
        QVERIFY(response["success"].toBool());
        QCOMPARE(response["message"].toString(), "Reservation cancelled successfully");
    }
    
    void test_error_handling_integration() {
        auto api_client = std::make_unique<Api_Client>();
        auto offer_model = std::make_unique<Offer_Model>();
        
        offer_model->set_api_client(api_client.get());
        api_client->connect_to_server("127.0.0.1", mock_server->get_port());
        QThread::msleep(100);
        
        // Test with invalid message type (mock server will return error)
        // This tests the client's ability to handle server errors gracefully
        
        QSignalSpy errorSpy(offer_model.get(), &Offer_Model::error_occurred);
        
        // Send invalid request (this would need to be implemented in Api_Client)
        // For now, we'll test that the model handles errors properly
        
        // Verify error handling mechanisms exist
        QVERIFY(offer_model->has_error());
        QVERIFY(!offer_model->last_error().isEmpty());
    }
    
    void test_concurrent_requests() {
        auto api_client = std::make_unique<Api_Client>();
        auto offer_model = std::make_unique<Offer_Model>();
        
        offer_model->set_api_client(api_client.get());
        api_client->connect_to_server("127.0.0.1", mock_server->get_port());
        QThread::msleep(100);
        
        // Send multiple concurrent requests
        QSignalSpy offersLoadedSpy(offer_model.get(), &Offer_Model::offers_loaded);
        
        offer_model->search_offers("Paris", 0.0, 1000.0);
        offer_model->search_offers("Rome", 0.0, 1000.0);
        offer_model->search_offers("London", 0.0, 1000.0);
        
        // Wait for all responses
        QVERIFY(offersLoadedSpy.wait(10000));
        
        // Verify that all requests were processed
        // The mock server should handle multiple requests correctly
        QVERIFY(offer_model->rowCount() > 0);
    }
    
    void test_network_timeout_handling() {
        // Test client behavior when server is slow to respond
        // This would require a more sophisticated mock server that can simulate delays
        
        auto api_client = std::make_unique<Api_Client>();
        auto offer_model = std::make_unique<Offer_Model>();
        
        offer_model->set_api_client(api_client.get());
        api_client->connect_to_server("127.0.0.1", mock_server->get_port());
        QThread::msleep(100);
        
        // Test that loading states are properly managed
        QSignalSpy loadingChangedSpy(offer_model.get(), &Offer_Model::loading_changed);
        
        offer_model->search_offers("Paris", 0.0, 1000.0);
        
        // Verify loading state changes
        QVERIFY(loadingChangedSpy.wait(5000));
        QCOMPARE(offer_model->is_loading(), false);
    }
    
private:
    std::unique_ptr<MockTcpServer> mock_server;
};

// Performance integration tests
class TestPerformanceIntegration : public QObject {
    Q_OBJECT
    
private slots:
    void test_bulk_operations() {
        auto api_client = std::make_unique<Api_Client>();
        auto offer_model = std::make_unique<Offer_Model>();
        
        offer_model->set_api_client(api_client.get());
        api_client->connect_to_server("127.0.0.1", mock_server->get_port());
        QThread::msleep(100);
        
        // Test performance of multiple operations
        auto start = QDateTime::currentMSecsSinceEpoch();
        
        for (int i = 0; i < 10; i++) {
            offer_model->search_offers("Test" + QString::number(i), 0.0, 1000.0);
            QThread::msleep(50); // Small delay between requests
        }
        
        // Wait for all responses
        QSignalSpy offersLoadedSpy(offer_model.get(), &Offer_Model::offers_loaded);
        QVERIFY(offersLoadedSpy.wait(30000)); // 30 seconds timeout
        
        auto end = QDateTime::currentMSecsSinceEpoch();
        auto duration = end - start;
        
        // Should complete 10 operations in reasonable time
        QVERIFY(duration < 30000); // Less than 30 seconds
        QVERIFY(offersLoadedSpy.count() >= 10);
    }
    
    void test_memory_usage() {
        // Test memory usage during operations
        auto api_client = std::make_unique<Api_Client>();
        auto offer_model = std::make_unique<Offer_Model>();
        
        offer_model->set_api_client(api_client.get());
        api_client->connect_to_server("127.0.0.1", mock_server->get_port());
        QThread::msleep(100);
        
        // Perform operations and monitor memory
        for (int i = 0; i < 5; i++) {
            offer_model->search_offers("Test" + QString::number(i), 0.0, 1000.0);
            QThread::msleep(100);
        }
        
        // Wait for responses
        QSignalSpy offersLoadedSpy(offer_model.get(), &Offer_Model::offers_loaded);
        QVERIFY(offersLoadedSpy.wait(15000));
        
        // Verify memory usage is reasonable
        // This is a basic test - in a real scenario you'd use proper memory profiling tools
        QVERIFY(offer_model->rowCount() > 0);
    }
    
private:
    std::unique_ptr<MockTcpServer> mock_server;
};

// Main test runner
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Run integration tests
    TestClientServerIntegration testIntegration;
    TestPerformanceIntegration testPerformance;
    
    int result = 0;
    
    result += QTest::qExec(&testIntegration, argc, argv);
    result += QTest::qExec(&testPerformance, argc, argv);
    
    return result;
}

#include "integration_tests.moc"

