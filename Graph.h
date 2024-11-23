#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>

struct Vex {
    int num;                     // 节点编号
    std::string name;            // 节点名称
    std::string introduction;    // 节点介绍
    std::string ticketInfo;      // 门票信息
};

struct Edge {
    int vex1;                    // 边的起点
    int vex2;                    // 边的终点
    double weight;               // 边的权重
};

class Graph {
public:
    Graph();                                     // 构造函数
    int insertVex(const Vex& vex);               // 插入一个节点，返回节点编号
    bool removeVex(int vexNum);                  // 删除一个节点
    void clearEdges();                           // 清空所有边
    void clearGraph();                           // **清空整个图**
    void addEdge(int v1, int v2, double weight); // 添加一条边
    void updateEdgeWeight(int v1, int v2, double weight); // 更新边的权重
    void removeEdge(int v1, int v2);             // 删除一条边
    Vex getVex(int vexNum) const;                // 根据编号获取节点
    int getVexIndex(const std::string& name) const; // 获取节点索引
    std::vector<Vex> getAllVexs() const;         // 获取所有节点
    std::vector<Edge> getAllEdges() const;       // 获取所有边

    // 获取邻接表
    std::map<int, std::vector<std::pair<int, double>>> getAdjacencyList() const;

private:
    int vexCounter;                              // 节点计数器
    std::map<int, Vex> vexs;                     // 节点映射
    std::set<std::pair<int, int>> edges;         // 边集合
    std::map<std::pair<int, int>, double> edgeWeights; // 边的权重映射
};

#endif // GRAPH_H
