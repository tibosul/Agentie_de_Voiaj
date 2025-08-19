#include "ui/Main_Window.h"
#include "network/Api_Client.h"
#include "models/User_Model.h"
#include "models/Destination_Model.h"

Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	Api_Client& api = Api_Client::instance();
    connect(&api, &Api_Client::connection_status_changed,
        [](bool connected) {
			qDebug() << "Connection status changed:" << (connected ? "Connected" : "Disconnected");
        });

    connect(&api, &Api_Client::network_error,
        [](const QString& error) {
            qDebug() << "Network error:" << error;
		});

    connect(&api, &Api_Client::destinations_received,
        [](const QJsonArray& destinations) {
            qDebug() << "Received destinations:" << destinations.size();
            for (const auto& dest : destinations)
            {
                qDebug() << "Destination:" << dest.toObject().value("name").toString();
            }
		});

    QTimer::singleShot(2000, [&api]() {
        // Test connection after 2 seconds
        api.test_connection();
	});

    QTimer::singleShot(3000, [&api]() {
        // Get destinations after 3 seconds
        api.get_destinations();
    });

	// Load user model
	User_Model* user_model = new User_Model(this);
    connect(user_model, &User_Model::login_succes, []() {
            qDebug() << "User logged in successfully";
		});

	// Load destination model
	Destination_Model* destination_model = new Destination_Model(this);
    connect(destination_model, &Destination_Model::destinations_loaded, [destination_model]()
        {
            qDebug() << "Destinations loaded:" << destination_model->get_destination_count();
            for (int i = 0; i < destination_model->get_destination_count(); ++i)
            {
                auto dest = destination_model->get_destination(i);
                qDebug() << "Destination" << i << ":" << dest.name << "in" << dest.country;
            }
        });

    QTimer::singleShot(2000, [destination_model]() {
        // Refresh destinations after 5 seconds
        destination_model->refresh_destinations();
		});
}

Main_Window::~Main_Window()
{}

