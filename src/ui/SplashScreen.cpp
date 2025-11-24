#include "SplashScreen.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QResizeEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPaintEvent>
#include <QLinearGradient>
#include <QColor>

namespace
{
const char *kSplashImage = "assets/splash.png";
}

SplashScreen::SplashScreen(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupStyles();
    loadHeroImage();
}

void SplashScreen::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 24, 24, 32);
    rootLayout->setSpacing(0);
    rootLayout->setAlignment(Qt::AlignHCenter);

    playButton = new QPushButton(tr("PLAY"), this);
    playButton->setObjectName("PlayButton");
    playButton->setCursor(Qt::PointingHandCursor);
    playButton->setMinimumHeight(44);
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 140));
    playButton->setGraphicsEffect(shadow);
    connect(playButton, &QPushButton::clicked, this, &SplashScreen::handlePlayClicked);

    rootLayout->addStretch(1);
    rootLayout->addWidget(playButton, 0, Qt::AlignHCenter | Qt::AlignBottom);
}

void SplashScreen::setupStyles()
{
    setObjectName("SplashScreen");
    setStyleSheet(R"(
        #SplashScreen {
            background: #0f1116;
        }
        #PlayButton {
            min-width: 160px;
            font-size: 16px;
            font-weight: 700;
            color: white;
            border: none;
            border-radius: 22px;
            padding: 10px 20px;
            background-color: rgba(0, 0, 0, 0.72);
        }
        #PlayButton:hover {
            background-color: rgba(0, 0, 0, 0.82);
        }
        #PlayButton:pressed {
            background-color: rgba(0, 0, 0, 0.9);
        }
        #PlayButton:focus {
            outline: none;
        }
    )");
}

void SplashScreen::loadHeroImage()
{
    const QStringList candidates = {
        QCoreApplication::applicationDirPath() + QLatin1Char('/') + QLatin1String(kSplashImage),                // next to exe
        QDir(QCoreApplication::applicationDirPath()).filePath("../src/assets/splash.png"),                      // build dir
        QDir(QCoreApplication::applicationDirPath()).filePath("../../src/assets/splash.png"),                   // when running from build/subdir
        QDir::currentPath() + QLatin1String("/src/assets/splash.png"),                                          // repo root as cwd
        QDir::currentPath() + QLatin1String("/assets/splash.png")                                               // cwd/assets
    };

    for (const QString &path : candidates) {
        if (!QFileInfo::exists(path)) {
            continue;
        }
        QPixmap pix(path);
        if (!pix.isNull()) {
            heroPixmap = pix;
            updateHeroPixmap();
            return;
        }
    }

    heroPixmap = QPixmap();
}

void SplashScreen::updateHeroPixmap()
{
    if (heroPixmap.isNull()) {
        return;
    }

    const QSize targetSize = size() * devicePixelRatioF();
    heroScaled = heroPixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    heroScaled.setDevicePixelRatio(devicePixelRatioF());
}

void SplashScreen::handlePlayClicked()
{
    emit playRequested();
}

void SplashScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateHeroPixmap();
}

void SplashScreen::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QRect targetRect = rect();

    if (!heroScaled.isNull()) {
        painter.drawPixmap(targetRect, heroScaled, heroScaled.rect());
    } else {
        QLinearGradient grad(targetRect.topLeft(), targetRect.bottomLeft());
        grad.setColorAt(0.0, QColor(20, 24, 32));
        grad.setColorAt(1.0, QColor(9, 11, 18));
        painter.fillRect(targetRect, grad);
    }

    // Subtle dark overlay for legibility
    painter.fillRect(targetRect, QColor(0, 0, 0, 50));

    QWidget::paintEvent(event);
}
