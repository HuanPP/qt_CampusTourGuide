#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include "Graph.h"

#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QGraphicsView>

#include <functional>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class DraggableEllipseItem : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT
public:
    explicit DraggableEllipseItem(int nodeId, const QString& labelText, QGraphicsItem* parent = nullptr);

    int getNodeId() const;
    void updateLabelPosition(); // 更新标签位置

signals:
    void positionChanged();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    int nodeId;
    bool moved;
    QPointF originalPosition; // 存储原始位置
    QGraphicsTextItem* label; // 标签
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_addNodeButton_clicked();
    void on_addEdgeButton_clicked();
    void on_deleteEdgeButton_clicked();
    void on_nodeSelected();
    void on_sceneNodeMoved();
    void on_findShortestPathButton_clicked();
    void on_dfsButton_clicked();
    void on_mstButton_clicked();

private:
    Ui::MainWindow* ui;
    Graph graph;
    QGraphicsScene* scene;

    std::map<int, DraggableEllipseItem*> nodeItems;
    std::map<std::pair<int, int>, QGraphicsLineItem*> edgeItems;
    std::map<std::pair<int, int>, QGraphicsTextItem*> edgeWeightTexts;

    void updateEdges();
    double calculateDistance(const QPointF& p1, const QPointF& p2);
};

#endif // MAINWINDOW_H
