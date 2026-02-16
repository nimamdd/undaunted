#include "BoardView.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QRadialGradient>
#include <QStyle>
#include <QStringList>
#include <QTextStream>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

BoardView::BoardView(const QString &playerOne,
                     const QString &playerTwo,
                     const QString &scenario,
                     QWidget *parent)
    : QWidget(parent),
      playerOneName(playerOne.isEmpty() ? tr("PLAYER 1") : playerOne),
      playerTwoName(playerTwo.isEmpty() ? tr("PLAYER 2") : playerTwo),
      scenarioPath(scenario)
{
    friendlyColor = QColor(103, 135, 80);
    neutralColor = QColor(230, 221, 187);
    hostileColor = QColor(170, 104, 52);
    playerAColor = QColor(86, 149, 224);
    playerBColor = QColor(227, 123, 102);

    setMinimumSize(1200, 760);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);

    setupUi();
    setupStyles();
    initializeGame();
}

void BoardView::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(22, 18, 22, 18);
    root->setSpacing(10);

    titleLabel = new QLabel(tr("UNDAUNTED"), this);
    titleLabel->setObjectName("TitleLabel");
    titleLabel->setAlignment(Qt::AlignHCenter);
    root->addWidget(titleLabel);

    auto *body = new QHBoxLayout;
    body->setSpacing(18);
    body->addStretch(1);

    sidePanel = new QWidget(this);
    sidePanel->setObjectName("SidePanel");
    sidePanel->setFixedWidth(300);
    auto *panelLayout = new QVBoxLayout(sidePanel);
    panelLayout->setContentsMargins(14, 14, 14, 14);
    panelLayout->setSpacing(8);

    turnLabel = new QLabel(tr("Turn"), sidePanel);
    turnLabel->setObjectName("MetaLabel");
    cardLabel = new QLabel(tr("Card"), sidePanel);
    cardLabel->setObjectName("MetaLabel");
    statusLabel = new QLabel(tr("Status"), sidePanel);
    statusLabel->setObjectName("StatusLabel");
    selectedCellLabel = new QLabel(tr("Selected: -"), sidePanel);
    selectedCellLabel->setObjectName("MetaLabel");
    selectedCellLabel->setWordWrap(true);

    panelLayout->addWidget(turnLabel);
    panelLayout->addWidget(cardLabel);
    panelLayout->addWidget(statusLabel);
    panelLayout->addWidget(selectedCellLabel);
    panelLayout->addSpacing(6);

    auto makeButton = [&](const QString &text, const QString &obj) {
        auto *btn = new QPushButton(text, sidePanel);
        btn->setObjectName(obj);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(38);
        panelLayout->addWidget(btn);
        return btn;
    };

    moveButton = makeButton(tr("Move"), "ActionButton");
    attackButton = makeButton(tr("Attack"), "ActionButton");
    markButton = makeButton(tr("Scout Mark"), "ActionButton");
    controlButton = makeButton(tr("Sergeant Control"), "ActionButton");
    releaseButton = makeButton(tr("Sergeant Release"), "ActionButton");

    endTurnButton = makeButton(tr("End Turn"), "EndTurnButton");
    menuButton = makeButton(tr("Back To Menu"), "MenuButton");

    panelLayout->addSpacing(8);
    actionResultLabel = new QLabel(tr("Choose an action."), sidePanel);
    actionResultLabel->setObjectName("ResultOk");
    actionResultLabel->setWordWrap(true);
    panelLayout->addWidget(actionResultLabel);
    panelLayout->addStretch(1);

    body->addWidget(sidePanel, 0, Qt::AlignTop | Qt::AlignRight);
    root->addLayout(body, 1);

    auto *bottom = new QHBoxLayout;
    bottom->setSpacing(12);
    p1Label = new QLabel(playerOneName, this);
    p1Label->setObjectName("PlayerInfo");
    p2Label = new QLabel(playerTwoName, this);
    p2Label->setObjectName("PlayerInfo");
    p2Label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    bottom->addWidget(p1Label, 1);
    bottom->addWidget(p2Label, 1);
    root->addLayout(bottom);

    connect(moveButton, &QPushButton::clicked, this, &BoardView::handleMoveAction);
    connect(attackButton, &QPushButton::clicked, this, &BoardView::handleAttackAction);
    connect(markButton, &QPushButton::clicked, this, &BoardView::handleScoutMarkAction);
    connect(controlButton, &QPushButton::clicked, this, &BoardView::handleSergeantControlAction);
    connect(releaseButton, &QPushButton::clicked, this, &BoardView::handleSergeantReleaseAction);
    connect(endTurnButton, &QPushButton::clicked, this, &BoardView::handleEndTurn);
    connect(menuButton, &QPushButton::clicked, this, [this]() { close(); });
}

void BoardView::setupStyles()
{
    setStyleSheet(R"(
        QLabel#TitleLabel {
            color: #f2ead2;
            font-size: 42px;
            font-weight: 900;
            letter-spacing: 6px;
        }
        QWidget#SidePanel {
            background: rgba(11, 15, 22, 0.83);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 14px;
        }
        QLabel#MetaLabel {
            color: #d6dce8;
            font-size: 13px;
            font-weight: 600;
            padding: 2px 0;
        }
        QLabel#StatusLabel {
            color: #f4d798;
            font-size: 14px;
            font-weight: 800;
            padding: 4px 0;
        }
        QLabel#PlayerInfo {
            color: #f6f0df;
            font-size: 14px;
            font-weight: 700;
        }
        QPushButton#ActionButton,
        QPushButton#EndTurnButton,
        QPushButton#MenuButton {
            border: none;
            border-radius: 10px;
            color: #f6f1e4;
            font-size: 13px;
            font-weight: 800;
            padding: 9px 12px;
        }
        QPushButton#ActionButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #355169, stop:1 #223347);
        }
        QPushButton#ActionButton:disabled {
            background: #4d5964;
            color: #c8ced6;
        }
        QPushButton#EndTurnButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #9b5d31, stop:1 #723c19);
        }
        QPushButton#MenuButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #5b4a34, stop:1 #3d3124);
        }
        QLabel#ResultOk {
            border-radius: 10px;
            padding: 10px;
            background: rgba(40, 96, 63, 0.33);
            color: #dcf7e6;
            border: 1px solid rgba(129, 217, 169, 0.5);
            font-size: 12px;
        }
        QLabel#ResultErr {
            border-radius: 10px;
            padding: 10px;
            background: rgba(128, 43, 43, 0.33);
            color: #ffe3de;
            border: 1px solid rgba(255, 139, 139, 0.5);
            font-size: 12px;
        }
    )");
}

QString BoardView::resolveBoardPath(const QString &inputPath, bool &isScenario, QString &errorMessage) const
{
    isScenario = false;

    QFile probe(inputPath);
    if (!probe.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMessage = tr("Cannot open selected file: %1").arg(inputPath);
        return {};
    }

    QTextStream in(&probe);
    QString firstNonEmpty;
    while (!in.atEnd()) {
        firstNonEmpty = in.readLine().trimmed();
        if (!firstNonEmpty.isEmpty()) {
            break;
        }
    }

    if (firstNonEmpty.startsWith('|')) {
        return QFileInfo(inputPath).absoluteFilePath();
    }

    isScenario = true;
    const QString fileName = QFileInfo(inputPath).fileName();
    const QString baseDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        baseDir + QLatin1String("/assets/boards/") + fileName,
        QDir(baseDir).filePath(QStringLiteral("../src/assets/boards/") + fileName),
        QDir::currentPath() + QLatin1String("/src/assets/boards/") + fileName,
        QDir::currentPath() + QLatin1String("/assets/boards/") + fileName
    };

    for (const QString &candidate : candidates) {
        if (QFileInfo::exists(candidate)) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }

    errorMessage = tr("Scenario selected but matching board file was not found: %1").arg(fileName);
    return {};
}

void BoardView::initializeGame()
{
    gameState = model::buildInitialGameState(playerOneName, playerTwoName);
    gameLoaded = false;
    actionUsedThisTurn = false;
    selectedCellId.clear();
    cellPolygons.clear();

    QString error;
    bool isScenario = false;
    boardPath = resolveBoardPath(scenarioPath, isScenario, error);
    if (boardPath.isEmpty()) {
        setActionMessage(error, true);
        updateHud();
        return;
    }

    if (!model::loadBoardFromMapFile(gameState.board, boardPath, error)) {
        setActionMessage(error, true);
        updateHud();
        return;
    }

    if (isScenario) {
        if (!model::loadScenarioFromFile(gameState, scenarioPath, error)) {
            setActionMessage(error, true);
            updateHud();
            return;
        }
    } else {
        model::clearScenarioState(gameState);
        model::updateGameStatus(gameState);
    }

    if (gameState.status == model::GameStatus::InProgress) {
        model::Card drawnCard;
        if (!model::drawTurnCard(gameState, drawnCard, error)) {
            setActionMessage(error, true);
            updateHud();
            return;
        }
    }

    gameLoaded = true;
    setActionMessage(tr("Battle loaded. Select a hex and execute your action."), false);
    updateHud();
    update();
}

QString BoardView::playerDisplayName(model::PlayerId id) const
{
    if (id == model::PlayerId::A) {
        return playerOneName;
    }
    if (id == model::PlayerId::B) {
        return playerTwoName;
    }
    return tr("Unknown");
}

QString BoardView::gameStatusText() const
{
    switch (gameState.status) {
    case model::GameStatus::InProgress:
        return tr("Status: In Progress");
    case model::GameStatus::WonByA:
        return tr("Winner: %1").arg(playerOneName);
    case model::GameStatus::WonByB:
        return tr("Winner: %1").arg(playerTwoName);
    }
    return tr("Status: Unknown");
}

bool BoardView::currentTurnAgent(model::AgentType &typeOut, QString &errorMessage) const
{
    if (!gameState.turn.hasActiveCard) {
        errorMessage = tr("No active card in current turn.");
        return false;
    }
    typeOut = gameState.turn.activeCard.agent;
    return true;
}

bool BoardView::requireSelectedCell(QString &errorMessage) const
{
    if (selectedCellId.isEmpty()) {
        errorMessage = tr("Select a target hex first.");
        return false;
    }
    if (model::findCell(gameState.board, selectedCellId) == nullptr) {
        errorMessage = tr("Selected hex is not valid.");
        return false;
    }
    return true;
}

void BoardView::setActionMessage(const QString &message, bool isError)
{
    actionResultLabel->setObjectName(isError ? "ResultErr" : "ResultOk");
    actionResultLabel->setText(message);
    actionResultLabel->style()->unpolish(actionResultLabel);
    actionResultLabel->style()->polish(actionResultLabel);
    actionResultLabel->update();
}

void BoardView::updateHud()
{
    if (!gameLoaded) {
        turnLabel->setText(tr("Turn: -"));
        cardLabel->setText(tr("Active Card: -"));
        statusLabel->setText(gameStatusText());
        selectedCellLabel->setText(tr("Selected: -"));
        moveButton->setEnabled(false);
        attackButton->setEnabled(false);
        markButton->setEnabled(false);
        controlButton->setEnabled(false);
        releaseButton->setEnabled(false);
        endTurnButton->setEnabled(false);
        return;
    }

    const int controlA = model::controlledCellCount(gameState, model::PlayerId::A);
    const int controlB = model::controlledCellCount(gameState, model::PlayerId::B);
    const int aliveA = model::aliveAgentCount(gameState, model::PlayerId::A);
    const int aliveB = model::aliveAgentCount(gameState, model::PlayerId::B);

    const QString activeMark = (gameState.turn.currentPlayer == model::PlayerId::A) ? QStringLiteral(">> ") : QStringLiteral("   ");
    const QString passiveMark = (gameState.turn.currentPlayer == model::PlayerId::B) ? QStringLiteral(">> ") : QStringLiteral("   ");
    p1Label->setText(QStringLiteral("%1%2   [Control: %3 | Alive: %4]").arg(activeMark, playerOneName).arg(controlA).arg(aliveA));
    p2Label->setText(QStringLiteral("%1%2   [Control: %3 | Alive: %4]").arg(passiveMark, playerTwoName).arg(controlB).arg(aliveB));

    turnLabel->setText(tr("Turn %1 · %2")
                           .arg(gameState.turn.turnIndex)
                           .arg(playerDisplayName(gameState.turn.currentPlayer)));

    if (gameState.turn.hasActiveCard) {
        cardLabel->setText(tr("Active Card: %1")
                               .arg(model::agentTypeName(gameState.turn.activeCard.agent)));
    } else {
        cardLabel->setText(tr("Active Card: -"));
    }

    statusLabel->setText(gameStatusText());

    if (selectedCellId.isEmpty()) {
        selectedCellLabel->setText(tr("Selected: -"));
    } else {
        const model::CellNode *cell = model::findCell(gameState.board, selectedCellId);
        QString details = tr("Selected: %1").arg(selectedCellId);
        if (cell != nullptr) {
            details += tr("\nShield: %1").arg(cell->shield);
            if (cell->controlledBy == model::PlayerId::A) {
                details += tr(" | Control: %1").arg(playerOneName);
            } else if (cell->controlledBy == model::PlayerId::B) {
                details += tr(" | Control: %1").arg(playerTwoName);
            } else {
                details += tr(" | Control: none");
            }
        }
        selectedCellLabel->setText(details);
    }

    const bool inProgress = (gameState.status == model::GameStatus::InProgress);
    const bool canAct = inProgress && gameState.turn.hasActiveCard && !actionUsedThisTurn;

    moveButton->setEnabled(false);
    attackButton->setEnabled(false);
    markButton->setEnabled(false);
    controlButton->setEnabled(false);
    releaseButton->setEnabled(false);

    if (canAct) {
        moveButton->setEnabled(true);
        attackButton->setEnabled(true);
        const model::AgentType type = gameState.turn.activeCard.agent;
        if (type == model::AgentType::Scout) {
            markButton->setEnabled(true);
        } else if (type == model::AgentType::Sergeant) {
            controlButton->setEnabled(true);
            releaseButton->setEnabled(true);
        }
    }

    endTurnButton->setEnabled(inProgress && gameState.turn.hasActiveCard && actionUsedThisTurn);
}

void BoardView::handleMoveAction()
{
    QString error;
    if (!requireSelectedCell(error)) {
        setActionMessage(error, true);
        return;
    }

    model::AgentType type{};
    if (!currentTurnAgent(type, error)) {
        setActionMessage(error, true);
        return;
    }

    if (!model::moveCurrentPlayerAgent(gameState, type, selectedCellId, error)) {
        setActionMessage(error, true);
        return;
    }

    actionUsedThisTurn = true;
    setActionMessage(tr("%1 moved to %2.").arg(model::agentTypeName(type), selectedCellId), false);
    updateHud();
    update();
}

void BoardView::handleAttackAction()
{
    QString error;
    if (!requireSelectedCell(error)) {
        setActionMessage(error, true);
        return;
    }

    model::AgentType type{};
    if (!currentTurnAgent(type, error)) {
        setActionMessage(error, true);
        return;
    }

    const model::AttackResult result = model::attackCurrentPlayer(gameState, type, selectedCellId);
    if (!result.executed) {
        setActionMessage(result.errorMessage, true);
        return;
    }

    actionUsedThisTurn = true;
    QString rollText;
    for (int i = 0; i < result.rolls.size(); ++i) {
        if (i) {
            rollText += QLatin1String(", ");
        }
        rollText += QString::number(result.rolls[i]);
    }

    QString message = tr("Attack threshold %1 | Rolls: [%2]")
                          .arg(result.threshold)
                          .arg(rollText);
    if (result.success) {
        message += tr(" | Hit");
        if (result.targetEliminated) {
            message += tr(" | Enemy %1 eliminated").arg(model::agentTypeName(result.targetType));
        }
    } else {
        message += tr(" | Miss");
    }

    if (gameState.status != model::GameStatus::InProgress) {
        message += tr(" | %1").arg(gameStatusText());
    }

    setActionMessage(message, false);
    updateHud();
    update();
}

void BoardView::handleScoutMarkAction()
{
    QString error;
    if (!model::scoutMarkCurrentPlayer(gameState, error)) {
        setActionMessage(error, true);
        return;
    }

    actionUsedThisTurn = true;
    setActionMessage(tr("Scout marked current cell."), false);
    updateHud();
    update();
}

void BoardView::handleSergeantControlAction()
{
    QString error;
    if (!model::sergeantControlCurrentPlayer(gameState, error)) {
        setActionMessage(error, true);
        return;
    }

    actionUsedThisTurn = true;
    setActionMessage(tr("Sergeant controlled current cell."), false);
    updateHud();
    update();
}

void BoardView::handleSergeantReleaseAction()
{
    QString error;
    if (!model::sergeantReleaseCurrentPlayer(gameState, error)) {
        setActionMessage(error, true);
        return;
    }

    actionUsedThisTurn = true;
    setActionMessage(tr("Sergeant released enemy-controlled cell."), false);
    updateHud();
    update();
}

void BoardView::handleEndTurn()
{
    if (gameState.status == model::GameStatus::InProgress &&
        gameState.turn.hasActiveCard &&
        !actionUsedThisTurn) {
        setActionMessage(tr("You must perform an action before ending the turn."), true);
        return;
    }

    QString error;
    if (!model::endTurn(gameState, error)) {
        setActionMessage(error, true);
        return;
    }

    selectedCellId.clear();
    actionUsedThisTurn = false;

    if (gameState.status == model::GameStatus::InProgress) {
        model::Card drawn{};
        if (!model::drawTurnCard(gameState, drawn, error)) {
            setActionMessage(error, true);
            updateHud();
            return;
        }
        setActionMessage(tr("Turn passed to %1.").arg(playerDisplayName(gameState.turn.currentPlayer)), false);
    } else {
        setActionMessage(gameStatusText(), false);
    }

    updateHud();
    update();
}

QRectF BoardView::boardAreaRect() const
{
    const int rightReserve = sidePanel ? sidePanel->width() + 44 : 340;
    const qreal left = 24;
    const qreal top = 86;
    const qreal widthValue = std::max(260, width() - rightReserve - 24);
    const qreal heightValue = std::max(260, height() - 150);
    return QRectF(left, top, widthValue, heightValue);
}

QPolygonF BoardView::hexPolygon(const QPointF &center, double radius) const
{
    constexpr double kPi = 3.14159265358979323846;
    QPolygonF poly;
    poly.reserve(6);
    for (int i = 0; i < 6; ++i) {
        const double angle = (60.0 * i - 30.0) * kPi / 180.0;
        poly << QPointF(center.x() + radius * std::cos(angle),
                        center.y() + radius * std::sin(angle));
    }
    return poly;
}

QColor BoardView::shieldColor(int shield) const
{
    if (shield <= 0) {
        return neutralColor;
    }
    if (shield == 1) {
        return friendlyColor;
    }
    return hostileColor;
}

QString BoardView::pieceShortName(model::AgentType type) const
{
    switch (type) {
    case model::AgentType::Scout:
        return QStringLiteral("SC");
    case model::AgentType::Sniper:
        return QStringLiteral("SN");
    case model::AgentType::Sergeant:
        return QStringLiteral("SG");
    }
    return QStringLiteral("??");
}

void BoardView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const QRectF full = rect();
    QLinearGradient bg(full.topLeft(), full.bottomRight());
    bg.setColorAt(0.0, QColor(14, 22, 31));
    bg.setColorAt(0.6, QColor(28, 47, 34));
    bg.setColorAt(1.0, QColor(47, 34, 26));
    p.fillRect(full, bg);

    QRadialGradient vignette(QPointF(width() * 0.35, height() * 0.5), std::max(width(), height()) * 0.85);
    vignette.setColorAt(0.0, QColor(0, 0, 0, 0));
    vignette.setColorAt(1.0, QColor(0, 0, 0, 165));
    p.fillRect(full, vignette);

    QRectF boardArea = boardAreaRect();
    QLinearGradient frameGrad(boardArea.topLeft(), boardArea.bottomRight());
    frameGrad.setColorAt(0.0, QColor(47, 67, 51, 235));
    frameGrad.setColorAt(1.0, QColor(36, 49, 38, 235));
    p.setPen(QPen(QColor(240, 233, 199, 60), 2.0));
    p.setBrush(frameGrad);
    p.drawRoundedRect(boardArea, 18, 18);

    cellPolygons.clear();

    if (!gameLoaded || gameState.board.cells.empty()) {
        p.setPen(QColor(238, 238, 238));
        p.drawText(boardArea, Qt::AlignCenter, tr("Board is not loaded"));
        return;
    }

    const double sqrt3 = std::sqrt(3.0);
    QHash<QString, QPointF> unitCenters;
    unitCenters.reserve(static_cast<int>(gameState.board.cells.size()));

    double minX = 1e9;
    double maxX = -1e9;
    double minY = 1e9;
    double maxY = -1e9;

    for (const auto &cellPtr : gameState.board.cells) {
        const model::CellNode *cell = cellPtr.get();
        const double ux = cell->col * sqrt3 + (cell->offset ? sqrt3 / 2.0 : 0.0);
        const double uy = cell->row * 1.5;
        unitCenters.insert(cell->id, QPointF(ux, uy));
        minX = std::min(minX, ux);
        maxX = std::max(maxX, ux);
        minY = std::min(minY, uy);
        maxY = std::max(maxY, uy);
    }

    const double gridW = (maxX - minX) + sqrt3;
    const double gridH = (maxY - minY) + 2.0;
    if (gridW <= 0 || gridH <= 0) {
        return;
    }

    const QRectF inner = boardArea.adjusted(18, 18, -18, -18);
    const double radius = std::max(14.0, std::min(inner.width() / gridW, inner.height() / gridH));
    const QPointF boardCenter = inner.center();
    const double midX = (minX + maxX) * 0.5;
    const double midY = (minY + maxY) * 0.5;

    QFont idFont = p.font();
    idFont.setPointSize(8);
    idFont.setWeight(QFont::DemiBold);

    QFont tokenFont = p.font();
    tokenFont.setPointSize(8);
    tokenFont.setWeight(QFont::Bold);

    for (const auto &cellPtr : gameState.board.cells) {
        const model::CellNode *cell = cellPtr.get();
        const QPointF u = unitCenters.value(cell->id);
        const QPointF center(boardCenter.x() + (u.x() - midX) * radius,
                             boardCenter.y() + (u.y() - midY) * radius);

        const QPolygonF poly = hexPolygon(center, radius);
        cellPolygons.insert(cell->id, poly);

        QColor base = shieldColor(cell->shield);
        QLinearGradient fillGrad(center + QPointF(0, -radius * 0.55),
                                 center + QPointF(0, radius * 0.9));
        fillGrad.setColorAt(0.0, base.lighter(125));
        fillGrad.setColorAt(1.0, base.darker(130));

        if (cell->id == selectedCellId) {
            p.setPen(QPen(QColor(250, 235, 156), 3.2));
        } else {
            p.setPen(QPen(QColor(255, 255, 255, 95), 1.4));
        }
        p.setBrush(fillGrad);
        p.drawPolygon(poly);

        if (cell->controlledBy == model::PlayerId::A) {
            p.setPen(QPen(playerAColor, 2.8));
            p.setBrush(Qt::NoBrush);
            p.drawPolygon(poly);
        } else if (cell->controlledBy == model::PlayerId::B) {
            p.setPen(QPen(playerBColor, 2.8));
            p.setBrush(Qt::NoBrush);
            p.drawPolygon(poly);
        }

        if (cell->markedByA) {
            p.setPen(Qt::NoPen);
            p.setBrush(playerAColor);
            p.drawEllipse(center + QPointF(-radius * 0.42, -radius * 0.32), radius * 0.14, radius * 0.14);
        }
        if (cell->markedByB) {
            p.setPen(Qt::NoPen);
            p.setBrush(playerBColor);
            p.drawEllipse(center + QPointF(radius * 0.42, -radius * 0.32), radius * 0.14, radius * 0.14);
        }

        p.setFont(idFont);
        p.setPen(QColor(16, 16, 16));
        p.drawText(QRectF(center.x() - radius, center.y() - radius * 0.75, radius * 2, radius * 0.45),
                   Qt::AlignCenter,
                   cell->id);

        std::optional<model::AgentType> occ;
        model::PlayerId owner = model::PlayerId::None;
        if (cell->occupantA.has_value()) {
            occ = cell->occupantA;
            owner = model::PlayerId::A;
        } else if (cell->occupantB.has_value()) {
            occ = cell->occupantB;
            owner = model::PlayerId::B;
        }

        if (occ.has_value()) {
            const QRectF tokenRect(center.x() - radius * 0.48,
                                   center.y() - radius * 0.3,
                                   radius * 0.96,
                                   radius * 0.96);
            p.setPen(QPen(QColor(255, 255, 255, 180), 1.6));
            p.setBrush(owner == model::PlayerId::A ? playerAColor : playerBColor);
            p.drawEllipse(tokenRect);

            p.setFont(tokenFont);
            p.setPen(QColor(10, 15, 20));
            p.drawText(tokenRect, Qt::AlignCenter, pieceShortName(*occ));
        }
    }
}

void BoardView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    const QPointF click = event->position();
    QString hit;

    for (auto it = cellPolygons.constBegin(); it != cellPolygons.constEnd(); ++it) {
        if (it.value().containsPoint(click, Qt::OddEvenFill)) {
            hit = it.key();
            break;
        }
    }

    if (!hit.isEmpty()) {
        selectedCellId = hit;
        updateHud();
        update();
        return;
    }

    if (boardAreaRect().contains(click)) {
        selectedCellId.clear();
        updateHud();
        update();
        return;
    }

    QWidget::mousePressEvent(event);
}
