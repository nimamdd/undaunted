#include "BoardView.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPointF>
#include <cmath>
#include <algorithm>

BoardView::BoardView(const QString &playerOne,
                     const QString &playerTwo,
                     const QString &map,
                     QWidget *parent)
    : QWidget(parent),
      mapPath(map),
      playerOneName(playerOne.isEmpty() ? tr("PLAYER 1") : playerOne),
      playerTwoName(playerTwo.isEmpty() ? tr("PLAYER 2") : playerTwo)
{
    friendlyColor = QColor(103, 135, 80);
    neutralColor  = QColor(230, 221, 187);
    hostileColor  = QColor(170, 104, 52);

    setMinimumSize(1100, 720);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setupUi();
    setupStyles();

    if (!mapPath.isEmpty()) {
        QString err;
        loadFromFile(mapPath, err);
    }
}

void BoardView::clear()
{
    rows.clear();
    maxCols = 0;
    mapPath.clear();
}

bool BoardView::loadFromFile(const QString &path, QString &errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMessage = tr("Cannot open map file: %1").arg(path);
        return false;
    }

    clear();

    QTextStream in(&file);
    int rowIndex = 0;
    QRegularExpression tokenRe("\\|\\s*([A-Z]\\d{2}):([0-2])");

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty())
            continue;

        const bool isOffset = line.startsWith(" ");
        QVector<Cell> rowCells;

        auto it = tokenRe.globalMatch(line);
        int colIndex = 0;
        while (it.hasNext()) {
            const auto m = it.next();
            Cell cell;
            cell.id = m.captured(1);
            cell.value = m.captured(2).toInt();
            cell.offset = isOffset;
            cell.col = colIndex++;
            cell.row = rowIndex;
            rowCells.append(cell);
        }

        if (!rowCells.isEmpty()) {
            maxCols = std::max(maxCols, static_cast<qsizetype>(rowCells.size()));
            rows.append(rowCells);
            ++rowIndex;
        }
    }

    if (rows.isEmpty()) {
        errorMessage = tr("Map file is empty or invalid: %1").arg(path);
        return false;
    }

    mapPath = QFileInfo(path).absoluteFilePath();
    update();
    return true;
}

void BoardView::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(0);

    titleLabel = new QLabel("UNDAUNTED", this);
    titleLabel->setObjectName("Title");
    titleLabel->setAlignment(Qt::AlignHCenter);
    root->addWidget(titleLabel);

    root->addStretch(1);

    auto *bottomRow = new QHBoxLayout;
    bottomRow->setSpacing(24);

    p1Label = new QLabel(playerOneName, this);
    p1Label->setObjectName("PlayerLabel");
    bottomRow->addWidget(p1Label, 0, Qt::AlignLeft | Qt::AlignVCenter);

    bottomRow->addStretch(1);

    auto *btns = new QHBoxLayout;
    btns->setSpacing(16);

    menuButton = new QPushButton("MENU", this);
    menuButton->setObjectName("MenuButton");
    btns->addWidget(menuButton);

    endTurnButton = new QPushButton("END TURN", this);
    endTurnButton->setObjectName("EndButton");
    btns->addWidget(endTurnButton);

    bottomRow->addLayout(btns);

    bottomRow->addStretch(1);

    p2Label = new QLabel(playerTwoName, this);
    p2Label->setObjectName("PlayerLabel");
    bottomRow->addWidget(p2Label, 0, Qt::AlignRight | Qt::AlignVCenter);

    root->addLayout(bottomRow);
}

void BoardView::setupStyles()
{
    setStyleSheet(R"(
        QLabel#Title {
            color: #f3e8ce;
            font-size: 40px;
            font-weight: 900;
            letter-spacing: 4px;
        }
        QLabel#PlayerLabel {
            color: #f3e8ce;
            font-size: 16px;
            font-weight: 700;
        }
        QPushButton#MenuButton,
        QPushButton#EndButton {
            min-width: 140px;
            padding: 10px 20px;
            font-size: 16px;
            font-weight: 800;
            border-radius: 8px;
            border: 2px solid rgba(0,0,0,60);
            color: #f3e8ce;
        }
        QPushButton#MenuButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                                        stop:0 #806437, stop:1 #614325);
        }
        QPushButton#MenuButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                                        stop:0 #8f6f3e, stop:1 #6f4d2b);
        }
        QPushButton#EndButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                                        stop:0 #b3532f, stop:1 #8c3c1b);
        }
        QPushButton#EndButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                                        stop:0 #c15c34, stop:1 #9a4520);
        }
    )");
}

void BoardView::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF area = rect();

    {
        QLinearGradient bg(area.topLeft(), area.bottomRight());
        bg.setColorAt(0.0, QColor(27, 44, 24));
        bg.setColorAt(1.0, QColor(16, 26, 16));
        p.fillRect(area, bg);
    }

    if (rows.isEmpty()) {
        p.setPen(QColor(220, 220, 220));
        p.drawText(area, Qt::AlignCenter, "No map loaded");
        return;
    }

    QRectF bar(area.left(), area.top() + 60, area.width(), 4);
    p.fillRect(bar, QColor(121, 74, 34));

    QRectF panel = area.adjusted(70, 100, -70, -130);

    QLinearGradient panelGrad(panel.topLeft(), panel.bottomRight());
    panelGrad.setColorAt(0.0, QColor(118, 136, 86));
    panelGrad.setColorAt(0.5, QColor(149, 162, 103));
    panelGrad.setColorAt(1.0, QColor(120, 136, 84));

    p.setPen(QPen(QColor(40, 55, 32), 2));
    p.setBrush(panelGrad);
    p.drawRoundedRect(panel, 22, 22);

    p.setPen(QPen(QColor(205, 193, 150, 110), 3));
    QPainterPath rd;
    rd.moveTo(panel.left() + panel.width() * 0.15,
              panel.bottom() - panel.height() * 0.15);
    rd.cubicTo(panel.center().x(), panel.center().y(),
               panel.center().x() + panel.width() * 0.10,
               panel.top() + panel.height() * 0.18,
               panel.right() - panel.width() * 0.15,
               panel.top() + panel.height() * 0.27);
    p.drawPath(rd);

    p.setPen(QPen(QColor(80, 110, 60, 90), 2));
    p.drawEllipse(panel.center(),
                  panel.width() * 0.18,
                  panel.height() * 0.18);

    QRectF boardArea = panel.adjusted(34, 26, -34, -26);

    const double sqrt3 = std::sqrt(3.0);

    const double ru = 1.0;
    const double hexW1 = sqrt3 * ru;
    const double hexH1 = 2.0 * ru;
    const double stepX1 = hexW1;
    const double stepY1 = 1.5 * ru;

    QVector<QVector<QPointF>> centersUnit(rows.size());
    double minX1 = 1e9, maxX1 = -1e9, minY1 = 1e9, maxY1 = -1e9;

    for (int rIdx = 0; rIdx < rows.size(); ++rIdx) {
        centersUnit[rIdx].resize(rows[rIdx].size());
        bool offsetRow = rows[rIdx].isEmpty() ? false : rows[rIdx][0].offset;

        for (int cIdx = 0; cIdx < rows[rIdx].size(); ++cIdx) {
            double cx1 = cIdx * stepX1 + (offsetRow ? hexW1 / 2.0 : 0.0);
            double cy1 = rIdx * stepY1;
            centersUnit[rIdx][cIdx] = QPointF(cx1, cy1);

            if (cx1 < minX1) minX1 = cx1;
            if (cx1 > maxX1) maxX1 = cx1;
            if (cy1 < minY1) minY1 = cy1;
            if (cy1 > maxY1) maxY1 = cy1;
        }
    }

    double gridWidth1  = (maxX1 - minX1) + hexW1;
    double gridHeight1 = (maxY1 - minY1) + hexH1;

    if (gridWidth1 <= 0 || gridHeight1 <= 0) {
        Q_UNUSED(event);
        return;
    }

    double innerMargin = 10.0;
    double availW = boardArea.width()  - 2.0 * innerMargin;
    double availH = boardArea.height() - 2.0 * innerMargin;

    double r = std::max(18.0, std::min(availW / gridWidth1, availH / gridHeight1));

    double hexW = hexW1 * r;
    double hexH = hexH1 * r;

    double midX1 = (minX1 + maxX1) * 0.5;
    double midY1 = (minY1 + maxY1) * 0.5;

    QPointF boardCenter = boardArea.center();

    constexpr double kPi = 3.14159265358979323846;

    auto hex = [&](QPointF c) {
        QPolygonF poly;
        for (int i = 0; i < 6; ++i) {
            double ang = (60.0 * i - 30.0) * kPi / 180.0;
            poly << QPointF(c.x() + r * std::cos(ang),
                            c.y() + r * std::sin(ang));
        }
        return poly;
    };

    QFont idF = p.font();
    idF.setPointSize(8);
    idF.setWeight(QFont::DemiBold);

    QFont valF = p.font();
    valF.setPointSize(11);
    valF.setWeight(QFont::Bold);

    const QColor outline(255, 255, 255, 90);

    for (int rIdx = 0; rIdx < rows.size(); ++rIdx) {
        for (int cIdx = 0; cIdx < rows[rIdx].size(); ++cIdx) {
            const auto &cell = rows[rIdx][cIdx];

            QPointF u = centersUnit[rIdx][cIdx];
            double cx = boardCenter.x() + (u.x() - midX1) * r;
            double cy = boardCenter.y() + (u.y() - midY1) * r;
            QPointF center(cx, cy);

            QColor base =
                (cell.value == 0) ? neutralColor :
                (cell.value == 1) ? friendlyColor :
                                    hostileColor;

            QLinearGradient fg(center + QPointF(0, -r * 0.4),
                               center + QPointF(0,  r * 0.8));
            fg.setColorAt(0.0, base.lighter(120));
            fg.setColorAt(1.0, base.darker(130));

            p.setPen(QPen(outline, 1.4));
            p.setBrush(fg);
            p.drawPolygon(hex(center));

            p.setFont(idF);
            p.setPen(QColor(15, 15, 15));
            p.drawText(QRectF(center.x() - hexW * 0.5,
                              center.y() - hexH * 0.5,
                              hexW, hexH * 0.35),
                       Qt::AlignCenter, cell.id);

            p.setFont(valF);
            p.setPen(Qt::white);
            p.drawText(QRectF(center.x() - hexW * 0.5,
                              center.y() - hexH * 0.1,
                              hexW, hexH * 0.45),
                       Qt::AlignCenter,
                       QString::number(cell.value));
        }
    }

    Q_UNUSED(event);
}