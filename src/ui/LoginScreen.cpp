#include "LoginScreen.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

namespace
{
const QString kSpecialChars = R"(!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?)";
const char *kBackgroundImagePrimary = "assets/photos/login.jpg";
const char *kBackgroundImageAlt = "assets/photos/image.png";
}

LoginScreen::LoginScreen(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupStyles();
    loadBackground();
}

void LoginScreen::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(40, 40, 40, 48);
    root->setSpacing(12);
    root->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    auto *badge = new QLabel(tr("MISSION BRIEFING"), this);
    badge->setObjectName("Badge");
    badge->setAlignment(Qt::AlignCenter);

    auto *title = new QLabel(tr("Ready your squad"), this);
    title->setObjectName("LoginTitle");
    title->setAlignment(Qt::AlignCenter);

    auto *subtitle = new QLabel(tr("Enter two player callsigns. Each must be 8+ characters with upper, lower, digit, and a special symbol."), this);
    subtitle->setObjectName("LoginSubtitle");
    subtitle->setWordWrap(true);
    subtitle->setAlignment(Qt::AlignCenter);

    auto *metaRow = new QHBoxLayout;
    metaRow->setSpacing(8);
    metaRow->setContentsMargins(0, 0, 0, 0);
    auto makePill = [&](const QString &text) {
        auto *pill = new QLabel(text, this);
        pill->setObjectName("MetaPill");
        pill->setAlignment(Qt::AlignCenter);
        return pill;
    };
    metaRow->addStretch();
    metaRow->addWidget(makePill(tr("2 Players")));
    metaRow->addWidget(makePill(tr("Co-op / Versus")));
    metaRow->addWidget(makePill(tr("Select a map")));
    metaRow->addStretch();

    auto *card = new QWidget(this);
    card->setObjectName("FormCard");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 20, 20, 20);
    cardLayout->setSpacing(12);

    auto makeRow = [&](const QString &labelText, QLineEdit **editOut) {
        auto *row = new QWidget(card);
        auto *rowLayout = new QVBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(6);

        auto *label = new QLabel(labelText, row);
        label->setObjectName("FieldLabel");

        auto *edit = new QLineEdit(row);
        edit->setObjectName("NameInput");
        edit->setPlaceholderText(tr("e.g. Ranger84!"));
        edit->setClearButtonEnabled(true);

        connect(edit, &QLineEdit::textChanged, this, &LoginScreen::handleInputChanged);

        rowLayout->addWidget(label);
        rowLayout->addWidget(edit);
        cardLayout->addWidget(row);
        *editOut = edit;
    };

    makeRow(tr("Player One"), &playerOneEdit);
    makeRow(tr("Player Two"), &playerTwoEdit);

    errorLabel = new QLabel(this);
    errorLabel->setObjectName("ErrorLabel");
    errorLabel->setVisible(false);
    errorLabel->setWordWrap(true);

    startButton = new QPushButton(tr("Deploy"), this);
    startButton->setObjectName("StartButton");
    startButton->setEnabled(false);
    startButton->setCursor(Qt::PointingHandCursor);
    connect(startButton, &QPushButton::clicked, this, &LoginScreen::handleStartClicked);

    auto *content = new QWidget(this);
    content->setObjectName("Content");
    content->setMaximumWidth(440);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(12);

    contentLayout->addWidget(badge, 0, Qt::AlignLeft);
    contentLayout->addWidget(title, 0, Qt::AlignLeft);
    contentLayout->addWidget(subtitle, 0, Qt::AlignLeft);
    contentLayout->addLayout(metaRow);
    contentLayout->addWidget(card);
    contentLayout->addWidget(errorLabel);
    contentLayout->addWidget(startButton, 0, Qt::AlignLeft);

    root->addStretch(1);
    root->addWidget(content, 0, Qt::AlignLeft | Qt::AlignBottom);

    // Pre-fill valid default names so users can jump into a match quickly.
    playerOneEdit->setText(QStringLiteral("AlphaOne1!"));
    playerTwoEdit->setText(QStringLiteral("BravoTwo2@"));

    handleInputChanged();
}

void LoginScreen::setupStyles()
{
    setObjectName("LoginScreen");
    setStyleSheet(R"(
        #LoginScreen {
            background: transparent;
        }
        #Badge {
            padding: 6px 10px;
            border-radius: 999px;
            background: rgba(255, 255, 255, 0.08);
            color: #d5cfbf;
            font-size: 11px;
            letter-spacing: 1px;
        }
        #LoginScreen QWidget {
            color: #f6f1e7;
        }
        #LoginTitle {
            font-size: 22px;
            font-weight: 800;
            letter-spacing: 1px;
            color: #f6f1e7;
        }
        #LoginSubtitle {
            font-size: 14px;
            color: #d0c8b5;
        }
        #MetaPill {
            padding: 6px 10px;
            border-radius: 999px;
            background: rgba(255, 255, 255, 0.08);
            border: 1px solid rgba(255, 255, 255, 0.06);
            font-size: 12px;
            color: #d5cfbf;
        }
        #FormCard {
            background: rgba(8, 10, 12, 0.78);
            border-radius: 14px;
            border: 1px solid rgba(255, 255, 255, 0.08);
            box-shadow: 0 14px 38px rgba(0, 0, 0, 0.65);
        }
        #FieldLabel {
            font-size: 13px;
            font-weight: 600;
            color: #e1d5c4;
        }
        #NameInput {
            font-size: 14px;
            padding: 10px 12px;
            border-radius: 8px;
            border: 1px solid #3d4b4e;
            background: rgba(255, 255, 255, 0.06);
            color: #f4f1eb;
        }
        #NameInput:focus {
            border: 1px solid #6fa36f;
            background: rgba(255, 255, 255, 0.12);
        }
        #ErrorLabel {
            color: #ffb4a9;
            font-size: 13px;
            padding: 8px 10px;
            background: rgba(179, 38, 30, 0.12);
            border: 1px solid rgba(179, 38, 30, 0.4);
            border-radius: 8px;
        }
        #StartButton {
            min-width: 200px;
            padding: 12px 18px;
            font-size: 15px;
            font-weight: 800;
            letter-spacing: 0.5px;
            color: #0d140f;
            border: none;
            border-radius: 10px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #e7c37d, stop:1 #c89a3c);
        }
        #StartButton:disabled {
            background: #55615b;
            color: #cfd3d1;
        }
    )");
}

bool LoginScreen::isNameValid(const QString &name, QString &error) const
{
    if (name.trimmed().isEmpty()) {
        error = tr("Name cannot be empty.");
        return false;
    }

    if (name.size() < 8) {
        error = tr("Must be at least 8 characters.");
        return false;
    }

    const QChar first = name.at(0);
    if (!first.isLetter()) {
        error = tr("Must start with a letter.");
        return false;
    }

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    for (const QChar c : name) {
        hasUpper = hasUpper || c.isUpper();
        hasLower = hasLower || c.isLower();
        hasDigit = hasDigit || c.isDigit();
        if (kSpecialChars.contains(c)) {
            hasSpecial = true;
        }
    }

    if (!hasDigit) {
        error = tr("Add at least one digit.");
        return false;
    }
    if (!hasSpecial) {
        error = tr("Add at least one special symbol.");
        return false;
    }
    if (!hasUpper || !hasLower) {
        error = tr("Use both uppercase and lowercase letters.");
        return false;
    }

    return true;
}

bool LoginScreen::validateForm(QString &errorMessage) const
{
    QString error1;
    if (!isNameValid(playerOneEdit->text(), error1)) {
        errorMessage = tr("Player 1: %1").arg(error1);
        return false;
    }

    QString error2;
    if (!isNameValid(playerTwoEdit->text(), error2)) {
        errorMessage = tr("Player 2: %1").arg(error2);
        return false;
    }

    if (playerOneEdit->text().trimmed().compare(playerTwoEdit->text().trimmed(), Qt::CaseInsensitive) == 0) {
        errorMessage = tr("Player names must be different.");
        return false;
    }

    return true;
}

QString LoginScreen::openMapDialog() const
{
    QDialog dialog(const_cast<LoginScreen*>(this));
    dialog.setWindowTitle(tr("Select a map"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *label = new QLabel(tr("Choose a battlefield:"), &dialog);
    auto *list = new QListWidget(&dialog);

    const QString base = QCoreApplication::applicationDirPath();
    const QStringList dirs = {
        base + QLatin1String("/assets/maps"),
        QDir(base).filePath("../src/assets/maps"),
        QDir::currentPath() + QLatin1String("/src/assets/maps"),
        QDir::currentPath() + QLatin1String("/assets/maps")
    };

    QStringList added;
    for (const QString &dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;
        const QStringList files = dir.entryList(QStringList() << "*.txt", QDir::Files, QDir::Name);
        for (const QString &f : files) {
            const QString abs = dir.filePath(f);
            QString canon = QFileInfo(abs).canonicalFilePath();
            if (canon.isEmpty()) {
                canon = QFileInfo(abs).absoluteFilePath();
            }
            if (added.contains(canon)) continue;
            added << canon;
            auto *item = new QListWidgetItem(f, list);
            item->setData(Qt::UserRole, canon);
        }
    }

    if (list->count() == 0) {
        auto *empty = new QListWidgetItem(tr("No map files found"), list);
        empty->setFlags(Qt::NoItemFlags);
    } else {
        list->setCurrentRow(0);
    }

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    layout->addWidget(label);
    layout->addWidget(list);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && list->currentItem()) {
        return list->currentItem()->data(Qt::UserRole).toString();
    }

    return {};
}

void LoginScreen::handleStartClicked()
{
    QString error;
    if (!validateForm(error)) {
        errorLabel->setText(error);
        errorLabel->setVisible(true);
        return;
    }

    const QString map = openMapDialog();
    if (map.isEmpty()) {
        return;
    }

    errorLabel->setVisible(false);
    emit startRequested(playerOneEdit->text(), playerTwoEdit->text(), map);
}

void LoginScreen::handleInputChanged()
{
    if (startButton == nullptr) {
        return;
    }

    QString error;
    const bool ok = validateForm(error);
    startButton->setEnabled(ok);

    if (!ok && !playerOneEdit->text().isEmpty() && !playerTwoEdit->text().isEmpty()) {
        errorLabel->setText(error);
        errorLabel->setVisible(true);
    } else {
        errorLabel->setVisible(false);
    }
}

void LoginScreen::loadBackground()
{
    const QString baseDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        baseDir + QLatin1Char('/') + QLatin1String(kBackgroundImagePrimary),
        QDir(baseDir).filePath("../src/assets/photos/login.jpg"),
        QDir::currentPath() + QLatin1String("/src/assets/photos/login.jpg"),
        baseDir + QLatin1Char('/') + QLatin1String(kBackgroundImageAlt),
        QDir(baseDir).filePath("../src/assets/photos/image.png"),
        QDir::currentPath() + QLatin1String("/src/assets/photos/image.png")
    };

    for (const QString &path : candidates) {
        if (!QFileInfo::exists(path)) {
            continue;
        }
        QPixmap pix(path);
        if (!pix.isNull()) {
            bgPixmap = pix;
            updateBackground();
            return;
        }
    }

    bgPixmap = QPixmap();
}

void LoginScreen::updateBackground()
{
    if (bgPixmap.isNull()) {
        return;
    }
    const QSize targetSize = size() * devicePixelRatioF();
    bgScaled = bgPixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    bgScaled.setDevicePixelRatio(devicePixelRatioF());
}

void LoginScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateBackground();
}

void LoginScreen::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect targetRect = rect();

    if (!bgScaled.isNull()) {
        painter.drawPixmap(targetRect, bgScaled, bgScaled.rect());
    } else {
        QLinearGradient base(targetRect.topLeft(), targetRect.bottomRight());
        base.setColorAt(0.0, QColor(26, 22, 18));
        base.setColorAt(1.0, QColor(12, 10, 8));
        painter.fillRect(targetRect, base);
    }

    // warm vignette overlay to match candle lighting
    QRadialGradient vignette(QPointF(width() * 0.58, height() * 0.42), std::max(width(), height()) * 0.9);
    vignette.setColorAt(0.0, QColor(0, 0, 0, 30));
    vignette.setColorAt(1.0, QColor(0, 0, 0, 210));
    painter.fillRect(targetRect, vignette);

    // soft amber glow to hint candle on right
    painter.save();
    QRadialGradient glow(QPointF(width() * 0.82, height() * 0.25), width() * 0.4);
    glow.setColorAt(0.0, QColor(255, 210, 150, 110));
    glow.setColorAt(1.0, QColor(255, 210, 150, 0));
    painter.setCompositionMode(QPainter::CompositionMode_Screen);
    painter.fillRect(targetRect, glow);
    painter.restore();

    QWidget::paintEvent(event);
}
