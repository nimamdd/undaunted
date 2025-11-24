#pragma once

#include <QWidget>
#include <QString>
#include <QPixmap>

class QLabel;
class QLineEdit;
class QPushButton;
class QPaintEvent;
class QResizeEvent;

class LoginScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LoginScreen(QWidget *parent = nullptr);

signals:
    void startRequested(const QString &playerOne, const QString &playerTwo, const QString &mapName);

private slots:
    void handleStartClicked();
    void handleInputChanged();

private:
    QLineEdit *playerOneEdit{};
    QLineEdit *playerTwoEdit{};
    QLabel *errorLabel{};
    QPushButton *startButton{};
    QPixmap bgPixmap{};
    QPixmap bgScaled{};

    void setupUi();
    void setupStyles();
    bool isNameValid(const QString &name, QString &error) const;
    bool validateForm(QString &errorMessage) const;
    QString openMapDialog() const;
    void loadBackground();
    void updateBackground();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};
