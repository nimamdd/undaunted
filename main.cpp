#include <QApplication>
#include "ui/SplashScreen.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SplashScreen window;
    window.setWindowTitle("Undaunted - Splash");
    window.resize(1000, 720);
    window.show();

    return app.exec();
}
