#pragma once

#include <QWidget>
#include <QPixmap>

class QPushButton;
class QPaintEvent;
class QResizeEvent;

class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplashScreen(QWidget *parent = nullptr);

signals:
    void playRequested();

private slots:
    void handlePlayClicked();

private:
    QPushButton *playButton{};
    QPixmap heroPixmap{};
    QPixmap heroScaled{};

    void setupUi();
    void setupStyles();
    void loadHeroImage();
    void updateHeroPixmap();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};
