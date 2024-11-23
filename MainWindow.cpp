#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QGraphicsTextItem>
#include <QPen>
#include <QMessageBox>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include <random>
#include <chrono>
#include <functional>
#include <queue>
#include <set>
#include <algorithm>

#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QGraphicsView>

DraggableEllipseItem::DraggableEllipseItem(int nodeId, const QString& labelText, QGraphicsItem* parent)
    : QObject(), QGraphicsEllipseItem(parent), nodeId(nodeId), moved(false) {
    setFlags(ItemIsMovable | ItemIsSelectable);
    originalPosition = pos(); // 初始化原始位置

    // 创建标签并设置位置
    label = new QGraphicsTextItem(labelText, this);
    label->setDefaultTextColor(Qt::black);
    label->setParentItem(this); // 让标签绑定在节点上
    updateLabelPosition();
}

int DraggableEllipseItem::getNodeId() const {
    return nodeId;
}

void DraggableEllipseItem::updateLabelPosition() {
    // 设置标签在圆形节点的正中央
    qreal x = rect().x() + rect().width() / 2 - label->boundingRect().width() / 2;
    qreal y = rect().y() + rect().height() / 2 - label->boundingRect().height() / 2;
    label->setPos(x, y);
}

void DraggableEllipseItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsEllipseItem::mouseMoveEvent(event);
    moved = true;
    updateLabelPosition(); // 同步更新标签位置
}

void DraggableEllipseItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsEllipseItem::mouseReleaseEvent(event);
    if (moved) {
        moved = false;
        emit positionChanged();
    }
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), scene(new QGraphicsScene(this)) {
    ui->setupUi(this);

    // 设置地图范围
    scene->setSceneRect(0, 0, 800, 600); // 根据需要调整地图大小
    ui->graphView->setScene(scene);

    // 连接信号和槽
    connect(ui->addNodeButton, &QPushButton::clicked, this, &MainWindow::on_addNodeButton_clicked);
    connect(ui->addEdgeButton, &QPushButton::clicked, this, &MainWindow::on_addEdgeButton_clicked);
    connect(ui->deleteEdgeButton, &QPushButton::clicked, this, &MainWindow::on_deleteEdgeButton_clicked);
    connect(scene, &QGraphicsScene::selectionChanged, this, &MainWindow::on_nodeSelected);

    // 连接算法按钮
    connect(ui->findShortestPathButton, &QPushButton::clicked, this, &MainWindow::on_findShortestPathButton_clicked);
    connect(ui->dfsButton, &QPushButton::clicked, this, &MainWindow::on_dfsButton_clicked);
    connect(ui->mstButton, &QPushButton::clicked, this, &MainWindow::on_mstButton_clicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_addNodeButton_clicked() {
    QString nodeName = ui->nodeNameInput->text().trimmed();
    QString nodeInfo = ui->nodeInfoInput->toPlainText().trimmed();

    if (nodeName.isEmpty() || nodeInfo.isEmpty()) {
        QMessageBox::warning(this, "警告", "节点名称和信息不能为空！");
        return;
    }

    int existingNodeId = graph.getVexIndex(nodeName.toStdString());
    if (existingNodeId != -1) {
        QMessageBox::warning(this, "警告", "节点名称已存在！");
        return;
    }

    Vex newVex;
    newVex.name = nodeName.toStdString();
    newVex.introduction = nodeInfo.toStdString();
    newVex.ticketInfo = "暂无门票信息";
    int nodeId = graph.insertVex(newVex);
    if (nodeId == -1) {
        QMessageBox::warning(this, "警告", "节点插入失败！");
        return;
    }

    // 创建 DraggableEllipseItem 并添加到场景
    DraggableEllipseItem* ellipse = new DraggableEllipseItem(nodeId, nodeName);
    ellipse->setRect(-20, -20, 40, 40);

    QRectF sceneRect = scene->sceneRect();
    static std::mt19937 generator(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_real_distribution<double> distributionX(sceneRect.left(), sceneRect.right());
    std::uniform_real_distribution<double> distributionY(sceneRect.top(), sceneRect.bottom());
    QPointF position(distributionX(generator), distributionY(generator));
    ellipse->setPos(position);
    ellipse->setBrush(Qt::green);
    scene->addItem(ellipse);

    // 插入到 nodeItems 中
    nodeItems[nodeId] = ellipse;
    qDebug() << "节点添加成功，ID：" << nodeId << "名称：" << nodeName;

    connect(ellipse, &DraggableEllipseItem::positionChanged, this, &MainWindow::on_sceneNodeMoved);

    // 清空输入框
    ui->nodeNameInput->clear();
    ui->nodeInfoInput->clear();
}

void MainWindow::on_addEdgeButton_clicked() {
    // 检查是否至少有两个节点
    if (graph.getAllVexs().size() < 6) {
        QMessageBox::warning(this, "警告", "节点少于6个时，无法添加边！");
        return;
    }

    QString startName = ui->edgeStartInput->text().trimmed();
    QString endName = ui->edgeEndInput->text().trimmed();

    if (startName.isEmpty() || endName.isEmpty()) {
        QMessageBox::warning(this, "警告", "起点和终点名称不能为空！");
        return;
    }

    int startId = graph.getVexIndex(startName.toStdString());
    int endId = graph.getVexIndex(endName.toStdString());

    if (startId == -1 || endId == -1) {
        QMessageBox::warning(this, "警告", "起点或终点不存在！");
        return;
    }

    // 使用 find 检查节点是否已存在于 nodeItems
    if (nodeItems.find(startId) == nodeItems.end() || nodeItems.find(endId) == nodeItems.end()) {
        QMessageBox::warning(this, "警告", "起点或终点未正确添加到地图中！");
        qDebug() << "nodeItems 当前状态：";
        for (const auto& pair : nodeItems) {
            qDebug() << "节点 ID：" << pair.first;
        }
        return;
    }

    int minId = std::min(startId, endId);
    int maxId = std::max(startId, endId);

    if (edgeItems.find({minId, maxId}) != edgeItems.end()) {
        QMessageBox::warning(this, "警告", "这条边已存在！");
        return;
    }

    QPointF pos1 = nodeItems[startId]->pos();
    QPointF pos2 = nodeItems[endId]->pos();
    double distance = calculateDistance(pos1, pos2);

    // 添加到图数据结构中
    graph.addEdge(minId, maxId, distance);

    // 绘制边
    QPen pen(Qt::gray);
    pen.setWidth(2);
    QGraphicsLineItem* line = scene->addLine(QLineF(pos1, pos2), pen);
    edgeItems[{minId, maxId}] = line;

    // 显示边权重
    QPointF midPoint = (pos1 + pos2) / 2;
    QGraphicsTextItem* text = scene->addText(QString::number(distance, 'f', 2));
    text->setDefaultTextColor(Qt::blue);
    text->setPos(midPoint);
    edgeWeightTexts[{minId, maxId}] = text;

    // 清空输入框
    ui->edgeStartInput->clear();
    ui->edgeEndInput->clear();
}

void MainWindow::on_deleteEdgeButton_clicked() {
    QString startName = ui->edgeStartInput->text().trimmed();
    QString endName = ui->edgeEndInput->text().trimmed();

    if (startName.isEmpty() || endName.isEmpty()) {
        QMessageBox::warning(this, "警告", "起点和终点名称不能为空！");
        return;
    }

    int startId = graph.getVexIndex(startName.toStdString());
    int endId = graph.getVexIndex(endName.toStdString());

    if (startId == -1) {
        QMessageBox::warning(this, "警告", "起点不存在：" + startName);
        return;
    }
    if (endId == -1) {
        QMessageBox::warning(this, "警告", "终点不存在：" + endName);
        return;
    }

    int minId = std::min(startId, endId);
    int maxId = std::max(startId, endId);

    // 检查边是否存在
    if (edgeItems.find({minId, maxId}) != edgeItems.end()) {
        scene->removeItem(edgeItems[{minId, maxId}]);
        delete edgeItems[{minId, maxId}];
        edgeItems.erase({minId, maxId});

        if (edgeWeightTexts.find({minId, maxId}) != edgeWeightTexts.end()) {
            scene->removeItem(edgeWeightTexts[{minId, maxId}]);
            delete edgeWeightTexts[{minId, maxId}];
            edgeWeightTexts.erase({minId, maxId});
        }

        graph.removeEdge(startId, endId);
    } else {
        QMessageBox::warning(this, "警告", "这条边不存在！");
        return;
    }

    ui->edgeStartInput->clear();
    ui->edgeEndInput->clear();
}

void MainWindow::on_sceneNodeMoved() {
    for (auto& pair : edgeItems) {
        int id1 = pair.first.first;
        int id2 = pair.first.second;

        if (nodeItems.find(id1) == nodeItems.end() || nodeItems.find(id2) == nodeItems.end()) {
            qWarning() << "节点未正确添加，无法更新边位置！";
            continue;
        }

        QPointF pos1 = nodeItems[id1]->pos();
        QPointF pos2 = nodeItems[id2]->pos();

        double distance = calculateDistance(pos1, pos2);
        graph.updateEdgeWeight(id1, id2, distance);

        if (pair.second) {
            pair.second->setLine(QLineF(pos1, pos2));
        } else {
            qWarning() << "边图形未正确初始化！";
            continue;
        }

        if (edgeWeightTexts.find(pair.first) != edgeWeightTexts.end() && edgeWeightTexts[pair.first]) {
            QPointF midPoint = (pos1 + pos2) / 2;
            edgeWeightTexts[pair.first]->setPlainText(QString::number(distance, 'f', 2));
            edgeWeightTexts[pair.first]->setPos(midPoint);
        } else {
            qWarning() << "边权重文本未正确初始化！";
        }
    }
}

double MainWindow::calculateDistance(const QPointF& p1, const QPointF& p2) {
    return std::hypot(p1.x() - p2.x(), p1.y() - p2.y());
}

void MainWindow::on_nodeSelected() {
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    if (!selectedItems.isEmpty()) {
        DraggableEllipseItem* item = dynamic_cast<DraggableEllipseItem*>(selectedItems.first());
        if (item) {
            int idx = item->getNodeId();
            Vex vex = graph.getVex(idx);

            ui->infoLabel->setText(QString("景点名称：%1\n介绍：%2\n门票信息：%3")
                                       .arg(QString::fromStdString(vex.name))
                                       .arg(QString::fromStdString(vex.introduction))
                                       .arg(QString::fromStdString(vex.ticketInfo)));
        }
    }
}

void MainWindow::on_findShortestPathButton_clicked() {
    if (graph.getAllVexs().size() < 6) {
        QMessageBox::warning(this, "警告", "节点少于6个时，无法查询最短路径！");
        return;
    }

    QString startName = ui->startPointInput->text().trimmed();
    QString endName = ui->endPointInput->text().trimmed();

    if (startName.isEmpty() || endName.isEmpty()) {
        QMessageBox::warning(this, "警告", "起点或终点名称不能为空！");
        return;
    }

    int startIdx = graph.getVexIndex(startName.toStdString());
    int endIdx = graph.getVexIndex(endName.toStdString());

    if (startIdx == -1 || endIdx == -1) {
        QMessageBox::warning(this, "警告", "起点或终点不存在！");
        return;
    }

    auto adjacencyList = graph.getAdjacencyList();
    std::map<int, double> distances;
    std::map<int, int> previous;
    std::set<int> visited;

    for (const auto& vexPair : adjacencyList) {
        distances[vexPair.first] = std::numeric_limits<double>::infinity();
    }
    distances[startIdx] = 0;

    auto cmp = [&distances](int left, int right) { return distances[left] > distances[right]; };
    std::priority_queue<int, std::vector<int>, decltype(cmp)> queue(cmp);
    queue.push(startIdx);

    while (!queue.empty()) {
        int current = queue.top();
        queue.pop();

        if (visited.find(current) != visited.end()) continue;
        visited.insert(current);

        for (const auto& neighbor : adjacencyList[current]) {
            int neighborIdx = neighbor.first;
            double weight = neighbor.second;
            double newDist = distances[current] + weight;

            if (newDist < distances[neighborIdx]) {
                distances[neighborIdx] = newDist;
                previous[neighborIdx] = current;
                queue.push(neighborIdx);
            }
        }
    }

    if (distances[endIdx] == std::numeric_limits<double>::infinity()) {
        ui->outputDisplay->setText("无法到达目标节点！");
        return;
    }

    std::vector<int> path;
    for (int at = endIdx; at != startIdx; at = previous[at]) {
        path.push_back(at);
    }
    path.push_back(startIdx);
    std::reverse(path.begin(), path.end());

    QString pathStr = "最短路径：\n";
    for (size_t i = 0; i < path.size(); ++i) {
        pathStr += QString::fromStdString(graph.getVex(path[i]).name);
        if (i != path.size() - 1) {
            pathStr += " -> ";
        }
    }
    pathStr += QString("\n总距离：%1").arg(distances[endIdx], 0, 'f', 2);
    ui->outputDisplay->setText(pathStr);

    for (auto& edgePair : edgeItems) {
        edgePair.second->setPen(QPen(Qt::gray, 2));
    }
    for (size_t i = 0; i < path.size() - 1; ++i) {
        int minId = std::min(path[i], path[i + 1]);
        int maxId = std::max(path[i], path[i + 1]);
        auto edgeKey = std::make_pair(minId, maxId);
        if (edgeItems.find(edgeKey) != edgeItems.end()) {
            edgeItems[edgeKey]->setPen(QPen(Qt::red, 4));
        }
    }
}

void MainWindow::on_dfsButton_clicked() {
    // 判断节点数是否足够
    if (graph.getAllVexs().size() < 6) {
        QMessageBox::warning(this, "警告", "节点少于6个时，无法执行DFS！");
        return;
    }

    QString startName = ui->startPointInput->text().trimmed();

    int startIdx = graph.getVexIndex(startName.toStdString());
    if (startIdx == -1) {
        QMessageBox::warning(this, "警告", "起点不存在！");
        return;
    }

    auto adjacencyList = graph.getAdjacencyList();
    std::set<int> visited;                      // 记录已访问节点
    std::vector<std::vector<int>> allPaths;     // 存储所有路径
    std::vector<int> currentPath;               // 当前路径

    // 深度优先搜索的递归函数
    std::function<void(int)> dfs = [&](int current) {
        visited.insert(current);
        currentPath.push_back(current);

        bool hasUnvisited = false;
        for (const auto& neighbor : adjacencyList[current]) {
            if (visited.find(neighbor.first) == visited.end()) {
                hasUnvisited = true;
                dfs(neighbor.first);
            }
        }

        if (!hasUnvisited) { // 如果到达叶子节点，保存路径
            allPaths.push_back(currentPath);
        }

        visited.erase(current);  // 回溯
        currentPath.pop_back();  // 回溯
    };

    // 执行DFS
    dfs(startIdx);

    // 随机生成颜色，用于路径标注
    static std::mt19937 generator(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<int> distribution(0, 255);

    for (size_t i = 0; i < allPaths.size(); ++i) {
        QColor pathColor(distribution(generator), distribution(generator), distribution(generator));

        for (size_t j = 0; j < allPaths[i].size() - 1; ++j) {
            int node1 = allPaths[i][j];
            int node2 = allPaths[i][j + 1];
            int minId = std::min(node1, node2);
            int maxId = std::max(node1, node2);
            auto edgeKey = std::make_pair(minId, maxId);

            if (edgeItems.find(edgeKey) != edgeItems.end()) {
                edgeItems[edgeKey]->setPen(QPen(pathColor, 4));
            }
        }
    }

    // 在界面上显示路径结果
    QString result = "DFS所有路径：\n";
    for (const auto& path : allPaths) {
        for (size_t i = 0; i < path.size(); ++i) {
            result += QString::fromStdString(graph.getVex(path[i]).name);
            if (i != path.size() - 1) {
                result += " -> ";
            }
        }
        result += "\n";
    }
    ui->outputDisplay->setText(result);
}

void MainWindow::on_mstButton_clicked() {
    // 实现Kruskal算法计算最小生成树
    auto allEdges = graph.getAllEdges();
    std::sort(allEdges.begin(), allEdges.end(), [](const Edge& e1, const Edge& e2) {
        return e1.weight < e2.weight;
    });

    std::map<int, int> parent;
    for (const auto& vex : graph.getAllVexs()) {
        parent[vex.num] = vex.num;
    }

    std::function<int(int)> findSet = [&](int u) {
        if (parent[u] != u)
            parent[u] = findSet(parent[u]);
        return parent[u];
    };

    std::vector<Edge> mstEdges;
    for (const auto& edge : allEdges) {
        int uSet = findSet(edge.vex1);
        int vSet = findSet(edge.vex2);
        if (uSet != vSet) {
            mstEdges.push_back(edge);
            parent[uSet] = vSet;
        }
    }

    // 在界面上显示最小生成树的边
    QString mstStr = "最小生成树的边：\n";
    double totalWeight = 0;
    for (const auto& edge : mstEdges) {
        QString startName = QString::fromStdString(graph.getVex(edge.vex1).name);
        QString endName = QString::fromStdString(graph.getVex(edge.vex2).name);
        mstStr += QString("%1 - %2，权重：%3\n").arg(startName).arg(endName).arg(edge.weight, 0, 'f', 2);
        totalWeight += edge.weight;
    }
    mstStr += QString("总权重：%1").arg(totalWeight, 0, 'f', 2);
    ui->outputDisplay->setText(mstStr);

    // 在地图上高亮最小生成树的边
    for (auto& edgePair : edgeItems) {
        edgePair.second->setPen(QPen(Qt::gray, 2));
    }
    for (const auto& edge : mstEdges) {
        int minId = std::min(edge.vex1, edge.vex2);
        int maxId = std::max(edge.vex1, edge.vex2);
        auto edgeKey = std::make_pair(minId, maxId);
        if (edgeItems.find(edgeKey) != edgeItems.end()) {
            edgeItems[edgeKey]->setPen(QPen(Qt::green, 4));
        }
    }
}
