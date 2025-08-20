#include <QTest>
#include <QSignalSpy>
#include <QApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <memory>

// Include test headers
#include "models/Offer_Model.h"
#include "models/Reservation_Model.h"
#include "network/Api_Client.h"
#include "ui/Main_Window.h"
#include "core/Application.h"

// Mock API Client for testing
class MockApiClient : public Api_Client {
    Q_OBJECT
public:
    MockApiClient(QObject* parent = nullptr) : Api_Client(parent) {}
    
    void search_offers(const QString& destination, double min_price, double max_price) override {
        Q_UNUSED(destination)
        Q_UNUSED(min_price)
        Q_UNUSED(max_price)
        
        // Simulate network delay
        QTimer::singleShot(100, [this]() {
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
            
            emit offers_received(offers);
        });
    }
    
    void book_offer(int offer_id, int person_count, const QJsonObject& additional_info) override {
        Q_UNUSED(offer_id)
        Q_UNUSED(person_count)
        Q_UNUSED(additional_info)
        
        QTimer::singleShot(100, [this]() {
            QJsonObject response;
            response["success"] = true;
            response["message"] = "Reservation created successfully";
            response["reservation_id"] = 123;
            
            emit booking_response_received(response);
        });
    }
    
    void get_user_reservations() override {
        QTimer::singleShot(100, [this]() {
            QJsonArray reservations;
            
            QJsonObject reservation;
            reservation["id"] = 123;
            reservation["offer_id"] = 1;
            reservation["destination"] = "Paris";
            reservation["status"] = "Confirmed";
            reservation["person_count"] = 2;
            reservation["booking_date"] = "2024-01-15";
            reservations.append(reservation);
            
            emit reservations_received(reservations);
        });
    }
    
    void cancel_reservation(int reservation_id) override {
        Q_UNUSED(reservation_id)
        
        QTimer::singleShot(100, [this]() {
            QJsonObject response;
            response["success"] = true;
            response["message"] = "Reservation cancelled successfully";
            
            emit cancellation_response_received(response);
        });
    }
    
    void update_user_info(const QJsonObject& user_data) override {
        Q_UNUSED(user_data)
        
        QTimer::singleShot(100, [this]() {
            QJsonObject response;
            response["success"] = true;
            response["message"] = "User info updated successfully";
            
            emit user_update_response_received(response);
        });
    }
};

// Test class for Offer_Model
class TestOfferModel : public QObject {
    Q_OBJECT
    
private slots:
    void initTestCase() {
        // Setup test environment
    }
    
    void test_offer_model_creation() {
        auto model = std::make_unique<Offer_Model>();
        QVERIFY(model != nullptr);
        QCOMPARE(model->rowCount(), 0);
    }
    
    void test_offer_model_roles() {
        auto model = std::make_unique<Offer_Model>();
        auto roles = model->roleNames();
        
        QVERIFY(roles.contains(Offer_Model::IdRole));
        QVERIFY(roles.contains(Offer_Model::DestinationRole));
        QVERIFY(roles.contains(Offer_Model::PriceRole));
        QVERIFY(roles.contains(Offer_Model::DurationRole));
        QVERIFY(roles.contains(Offer_Model::DescriptionRole));
        QVERIFY(roles.contains(Offer_Model::ImageUrlRole));
    }
    
    void test_offer_model_data() {
        auto model = std::make_unique<Offer_Model>();
        
        // Test empty model
        QCOMPARE(model->rowCount(), 0);
        
        // Test invalid index
        QVariant data = model->data(QModelIndex(), Offer_Model::DestinationRole);
        QVERIFY(!data.isValid());
    }
    
    void test_offer_model_search() {
        auto model = std::make_unique<Offer_Model>();
        auto api_client = std::make_unique<MockApiClient>();
        
        // Connect API client to model
        model->set_api_client(api_client.get());
        
        // Spy on signals
        QSignalSpy offersLoadedSpy(model.get(), &Offer_Model::offers_loaded);
        QSignalSpy loadingChangedSpy(model.get(), &Offer_Model::loading_changed);
        
        // Test search
        model->search_offers("Paris", 100.0, 1000.0);
        
        // Wait for signals
        QVERIFY(offersLoadedSpy.wait(500));
        QVERIFY(loadingChangedSpy.wait(500));
        
        // Verify loading state
        QCOMPARE(model->is_loading(), false);
        
        // Verify offers were loaded
        QCOMPARE(model->rowCount(), 2);
        
        // Verify first offer data
        QModelIndex index = model->index(0);
        QCOMPARE(model->data(index, Offer_Model::DestinationRole).toString(), "Paris");
        QCOMPARE(model->data(index, Offer_Model::PriceRole).toDouble(), 500.0);
        QCOMPARE(model->data(index, Offer_Model::DurationRole).toInt(), 7);
    }
    
    void test_offer_model_booking() {
        auto model = std::make_unique<Offer_Model>();
        auto api_client = std::make_unique<MockApiClient>();
        
        model->set_api_client(api_client.get());
        
        // Load some offers first
        model->search_offers("", 0.0, 1000.0);
        QSignalSpy offersLoadedSpy(model.get(), &Offer_Model::offers_loaded);
        QVERIFY(offersLoadedSpy.wait(500));
        
        // Test booking
        QSignalSpy bookingResponseSpy(model.get(), &Offer_Model::booking_response_received);
        model->book_offer(1, 2, QJsonObject{{"notes", "Window seat"}});
        
        QVERIFY(bookingResponseSpy.wait(500));
        QCOMPARE(bookingResponseSpy.count(), 1);
    }
    
    void test_offer_model_caching() {
        auto model = std::make_unique<Offer_Model>();
        auto api_client = std::make_unique<MockApiClient>();
        
        model->set_api_client(api_client.get());
        
        // Load offers
        model->search_offers("", 0.0, 1000.0);
        QSignalSpy offersLoadedSpy(model.get(), &Offer_Model::offers_loaded);
        QVERIFY(offersLoadedSpy.wait(500));
        
        // Verify offers are cached
        QCOMPARE(model->rowCount(), 2);
        
        // Create new model instance to test cache loading
        auto new_model = std::make_unique<Offer_Model>();
        QCOMPARE(new_model->rowCount(), 2); // Should load from cache
    }
    
    void cleanupTestCase() {
        // Cleanup test environment
    }
};

// Test class for Reservation_Model
class TestReservationModel : public QObject {
    Q_OBJECT
    
private slots:
    void test_reservation_model_creation() {
        auto model = std::make_unique<Reservation_Model>();
        QVERIFY(model != nullptr);
        QCOMPARE(model->rowCount(), 0);
    }
    
    void test_reservation_model_roles() {
        auto model = std::make_unique<Reservation_Model>();
        auto roles = model->roleNames();
        
        QVERIFY(roles.contains(Reservation_Model::IdRole));
        QVERIFY(roles.contains(Reservation_Model::OfferIdRole));
        QVERIFY(roles.contains(Reservation_Model::DestinationRole));
        QVERIFY(roles.contains(Reservation_Model::StatusRole));
        QVERIFY(roles.contains(Reservation_Model::PersonCountRole));
        QVERIFY(roles.contains(Reservation_Model::BookingDateRole));
    }
    
    void test_reservation_model_refresh() {
        auto model = std::make_unique<Reservation_Model>();
        auto api_client = std::make_unique<MockApiClient>();
        
        model->set_api_client(api_client.get());
        
        QSignalSpy reservationsLoadedSpy(model.get(), &Reservation_Model::reservations_loaded);
        QSignalSpy loadingChangedSpy(model.get(), &Reservation_Model::loading_changed);
        
        model->refresh_reservations();
        
        QVERIFY(reservationsLoadedSpy.wait(500));
        QVERIFY(loadingChangedSpy.wait(500));
        
        QCOMPARE(model->rowCount(), 1);
        QCOMPARE(model->is_loading(), false);
    }
    
    void test_reservation_model_cancellation() {
        auto model = std::make_unique<Reservation_Model>();
        auto api_client = std::make_unique<MockApiClient>();
        
        model->set_api_client(api_client.get());
        
        // Load reservations first
        model->refresh_reservations();
        QSignalSpy reservationsLoadedSpy(model.get(), &Reservation_Model::reservations_loaded);
        QVERIFY(reservationsLoadedSpy.wait(500));
        
        // Test cancellation
        QSignalSpy cancellationResponseSpy(model.get(), &Reservation_Model::cancellation_response_received);
        model->cancel_reservation(123);
        
        QVERIFY(cancellationResponseSpy.wait(500));
        QCOMPARE(cancellationResponseSpy.count(), 1);
    }
    
    void test_reservation_model_filtering() {
        auto model = std::make_unique<Reservation_Model>();
        auto api_client = std::make_unique<MockApiClient>();
        
        model->set_api_client(api_client.get());
        
        // Load reservations
        model->refresh_reservations();
        QSignalSpy reservationsLoadedSpy(model.get(), &Reservation_Model::reservations_loaded);
        QVERIFY(reservationsLoadedSpy.wait(500));
        
        // Test filtering by status
        model->filter_by_status("Confirmed");
        QCOMPARE(model->rowCount(), 1);
        
        model->filter_by_status("Cancelled");
        QCOMPARE(model->rowCount(), 0);
        
        // Clear filter
        model->clear_filter();
        QCOMPARE(model->rowCount(), 1);
    }
};

// Test class for Main_Window
class TestMainWindow : public QObject {
    Q_OBJECT
    
private slots:
    void test_main_window_creation() {
        auto window = std::make_unique<Main_Window>();
        QVERIFY(window != nullptr);
        QVERIFY(window->isVisible());
    }
    
    void test_main_window_tabs() {
        auto window = std::make_unique<Main_Window>();
        
        // Verify all tabs exist
        QVERIFY(window->findChild<QWidget*>("offers_tab") != nullptr);
        QVERIFY(window->findChild<QWidget*>("reservations_tab") != nullptr);
        QVERIFY(window->findChild<QWidget*>("profile_tab") != nullptr);
    }
    
    void test_main_window_authentication_state() {
        auto window = std::make_unique<Main_Window>();
        
        // Test initial state (not authenticated)
        // Reservations and Profile tabs should show login prompts
        
        // Test authenticated state
        // This would require setting up a mock user model
    }
    
    void test_main_window_offer_display() {
        auto window = std::make_unique<Main_Window>();
        
        // Test that offers tab shows loading state initially
        // Test that offers are displayed after loading
        // Test search functionality
        // Test offer booking
    }
    
    void test_main_window_reservation_display() {
        auto window = std::make_unique<Main_Window>();
        
        // Test that reservations tab shows authentication required initially
        // Test that reservations are displayed after authentication
        // Test reservation cancellation
    }
};

// Test class for Api_Client
class TestApiClient : public QObject {
    Q_OBJECT
    
private slots:
    void test_api_client_creation() {
        auto client = std::make_unique<Api_Client>();
        QVERIFY(client != nullptr);
    }
    
    void test_api_client_connection() {
        auto client = std::make_unique<Api_Client>();
        
        // Test connection to server
        // This would require a mock server or real server running
    }
    
    void test_api_client_message_formatting() {
        auto client = std::make_unique<Api_Client>();
        
        // Test JSON message formatting for different request types
        // Test response parsing
    }
};

// Main test runner
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Run all test classes
    TestOfferModel testOfferModel;
    TestReservationModel testReservationModel;
    TestMainWindow testMainWindow;
    TestApiClient testApiClient;
    
    int result = 0;
    
    result += QTest::qExec(&testOfferModel, argc, argv);
    result += QTest::qExec(&testReservationModel, argc, argv);
    result += QTest::qExec(&testMainWindow, argc, argv);
    result += QTest::qExec(&testApiClient, argc, argv);
    
    return result;
}

#include "client_tests.moc"

