#include "main_window.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Main_Window window;
    // Don't show main window immediately - it will be shown after successful login
    // window.show();
    return app.exec();
}
