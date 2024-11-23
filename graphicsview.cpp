#include "GraphicsView.h"
#include <QMimeData>
#include <QDebug>

GraphicsView::GraphicsView(QWidget* parent)
    : QGraphicsView(parent) {
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
}

void GraphicsView::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasText()) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
        qDebug() << "GraphicsView: dragEnterEvent accepted";
    } else {
        event->ignore();
        qDebug() << "GraphicsView: dragEnterEvent ignored";
    }
}

void GraphicsView::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasText()) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
        qDebug() << "GraphicsView: dragMoveEvent accepted";
    } else {
        event->ignore();
        qDebug() << "GraphicsView: dragMoveEvent ignored";
    }
}

void GraphicsView::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasText()) {
        int nodeId = event->mimeData()->text().toInt();
        QPointF position = mapToScene(event->pos());

        emit nodeDropped(nodeId, position);

        event->setDropAction(Qt::MoveAction);
        event->accept();

        qDebug() << "GraphicsView: Node" << nodeId << "dropped at position" << position;
    } else {
        event->ignore();
        qDebug() << "GraphicsView: dropEvent ignored";
    }
}
