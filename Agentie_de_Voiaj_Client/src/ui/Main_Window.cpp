#include "ui/Main_Window.h"
#include "network/Api_Client.h"

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
}

Main_Window::~Main_Window()
{}

