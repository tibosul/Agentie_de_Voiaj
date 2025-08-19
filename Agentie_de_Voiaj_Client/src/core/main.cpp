#include "core/Application.h"
#include "ui/Main_Window.h"
#include "Config.h"

#include <QMessageBox>
#include <QDir>

int main(int argc, char* argv[])
{
    // Create application instance
    Application_Manager app(argc, argv);

    // Initialize application
    if (!app.initialize())
    {
        QMessageBox::critical(nullptr, "Error",
            "Failed to initialize application!");
        return -1;
    }

    try
    {
        // Create and show main window
        Main_Window window;
        window.show();

        // Start event loop
        return app.exec();

    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(nullptr, "Fatal Error",
            QString("Application error: %1").arg(e.what()));
        return -1;
    }
}