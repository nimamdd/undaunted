#include <QApplication>
#include <QWidget>

#include "controllers/Navigation.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Navigation navigation;

    QWidget *window = navigation.window();
    window->setWindowTitle("Undaunted");
    window->resize(1000, 720);
    window->show();

    return app.exec();
}
