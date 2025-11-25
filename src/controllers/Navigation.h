#pragma once

#include <QObject>
#include <QString>

class QStackedWidget;
class SplashScreen;
class LoginScreen;

class Navigation : public QObject
{
    Q_OBJECT

public:
    explicit Navigation(QObject *parent = nullptr);

    QWidget *window() const;

private:
    QStackedWidget *stack{};
    SplashScreen *splash{};
    LoginScreen *login{};

    void setupScreens();
    void setupConnections();
};
