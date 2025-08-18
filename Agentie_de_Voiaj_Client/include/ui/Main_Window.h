#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Main_Window.h"

class Main_Window : public QMainWindow
{
    Q_OBJECT

public:
    Main_Window(QWidget *parent = nullptr);
    ~Main_Window();

private:
    Ui::Main_WindowClass ui;
};

