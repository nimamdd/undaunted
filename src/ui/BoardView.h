#pragma once
#include <QWidget>
#include <QVector>
#include <QString>
#include <QColor>

class QLabel;
class QPushButton;

class BoardView : public QWidget
{
public:
    struct Cell {
        QString id;
        int value;
        bool offset;
        int row;
        int col;
    };

    explicit BoardView(const QString &playerOne,
                       const QString &playerTwo,
                       const QString &mapPath,
                       QWidget *parent = nullptr);

    void clear();
    bool loadFromFile(const QString &path, QString &errorMessage);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    void setupStyles();

private:
    QString playerOneName;
    QString playerTwoName;
    QString mapPath;

    QVector<QVector<Cell>> rows;
    qsizetype maxCols = 0;

    QLabel *titleLabel    = nullptr;
    QLabel *p1Label       = nullptr;
    QLabel *p2Label       = nullptr;
    QPushButton *menuButton    = nullptr;
    QPushButton *endTurnButton = nullptr;

    QColor friendlyColor;
    QColor neutralColor;
    QColor hostileColor;
};