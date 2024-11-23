#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QDragEnterEvent>
#include <QDropEvent>

class GraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    explicit GraphicsView(QWidget* parent = nullptr);

signals:
    void nodeDropped(int nodeId, QPointF position);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override; // 添加此行
    void dropEvent(QDropEvent* event) override;
};

#endif // GRAPHICSVIEW_H
