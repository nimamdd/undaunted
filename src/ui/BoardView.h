#pragma once

#include <QWidget>
#include <QColor>
#include <QHash>
#include <QPolygonF>

#include "game/GameModel.h"

class QLabel;
class QPushButton;
class QWidget;
class QPaintEvent;
class QMouseEvent;

class BoardView : public QWidget
{
public:
    explicit BoardView(const QString &playerOne,
                       const QString &playerTwo,
                       const QString &scenarioPath,
                       QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setupUi();
    void setupStyles();
    void initializeGame();
    QString resolveBoardPath(const QString &inputPath, bool &isScenario, QString &errorMessage) const;

    void updateHud();
    void setActionMessage(const QString &message, bool isError);
    QString playerDisplayName(model::PlayerId id) const;
    QString gameStatusText() const;
    bool requireSelectedCell(QString &errorMessage) const;

    void handleMoveAction();
    void handleAttackAction();
    void handleScoutMarkAction();
    void handleSergeantControlAction();
    void handleSergeantReleaseAction();

    QRectF boardAreaRect() const;
    QPolygonF hexPolygon(const QPointF &center, double radius) const;
    QColor shieldColor(int shield) const;
    QString pieceShortName(model::AgentType type) const;

private:
    model::GameState gameState{};
    model::GameSession session;

    QString playerOneName;
    QString playerTwoName;
    QString scenarioPath;
    QString boardPath;
    QString selectedCellId;

    bool gameLoaded{false};

    QLabel *titleLabel = nullptr;
    QLabel *turnLabel = nullptr;
    QLabel *cardLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QLabel *selectedCellLabel = nullptr;
    QLabel *actionResultLabel = nullptr;
    QLabel *p1Label = nullptr;
    QLabel *p2Label = nullptr;
    QPushButton *menuButton = nullptr;
    QPushButton *moveButton = nullptr;
    QPushButton *attackButton = nullptr;
    QPushButton *markButton = nullptr;
    QPushButton *controlButton = nullptr;
    QPushButton *releaseButton = nullptr;
    QWidget *sidePanel = nullptr;

    QHash<QString, QPolygonF> cellPolygons;

    QColor friendlyColor;
    QColor neutralColor;
    QColor hostileColor;
    QColor playerAColor;
    QColor playerBColor;
};
