#include "Navigation.h"

#include <QStackedWidget>
#include <QMessageBox>

#include "../ui/SplashScreen.h"
#include "../ui/LoginScreen.h"

Navigation::Navigation(QObject *parent)
    : QObject(parent)
{
    setupScreens();
    setupConnections();
}

void Navigation::setupScreens()
{
    stack = new QStackedWidget;

    splash = new SplashScreen;
    login = new LoginScreen;

    stack->addWidget(splash);
    stack->addWidget(login);
    stack->setCurrentWidget(splash);
}

void Navigation::setupConnections()
{
    connect(splash, &SplashScreen::playRequested, this, [this]() {
        stack->setCurrentWidget(login);
        login->setFocus();
    });

    connect(login, &LoginScreen::startRequested, this, [](const QString &p1, const QString &p2, const QString &map) {
        QMessageBox::information(nullptr, QObject::tr("Battle ready"),
                                 QObject::tr("Players: %1 vs %2\nMap: %3").arg(p1, p2, map));
    });
}

QWidget *Navigation::window() const
{
    return stack;
}
