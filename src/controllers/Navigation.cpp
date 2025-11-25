#include "Navigation.h"

#include <QStackedWidget>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>

#include "../ui/SplashScreen.h"
#include "../ui/LoginScreen.h"
#include "../ui/BoardView.h"

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

    connect(login, &LoginScreen::startRequested, this, [this](const QString &p1, const QString &p2, const QString &map) {
        auto *boardView = new BoardView(p1, p2, map);
        boardView->setWindowTitle("Undaunted - Battle");
        boardView->resize(1200, 800);
        boardView->show();
        stack->hide();
    });
}

QWidget *Navigation::window() const
{
    return stack;
}
