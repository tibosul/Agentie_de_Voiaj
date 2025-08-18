#include "ui/Main_Window.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Main_Window window;
    window.show();
    return app.exec();
}
