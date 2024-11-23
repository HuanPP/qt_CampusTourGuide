#include "Graph.h"
#include <cmath>
#include <algorithm>

Graph::Graph() : vexCounter(0) {}

int Graph::insertVex(const Vex& vex) {
    // 检查名称唯一性
    for (const auto& [id, existingVex] : vexs) {
        if (existingVex.name == vex.name) {
            return -1; // 名称重复
        }
    }

    int num = vexCounter++;
    Vex newVex = vex;
    newVex.num = num;
    vexs[num] = newVex;
    return num;
}

bool Graph::removeVex(int vexNum) {
    if (vexs.erase(vexNum)) {
        // 移除与该节点相关的边
        for (auto it = edges.begin(); it != edges.end();) {
            if (it->first == vexNum || it->second == vexNum) {
                edgeWeights.erase(*it);
                it = edges.erase(it);
            } else {
                ++it;
            }
        }
        return true;
    }
    return false;
}
void Graph::clearEdges() {
    edges.clear();
    edgeWeights.clear();
}

void Graph::addEdge(int v1, int v2, double weight) {
    if (vexs.find(v1) == vexs.end() || vexs.find(v2) == vexs.end()) {
        return; // 节点不存在，直接返回
    }

    // 始终使用较小的节点编号作为first，确保无向边的一致性
    int minV = std::min(v1, v2);
    int maxV = std::max(v1, v2);

    edges.insert({minV, maxV});
    edgeWeights[{minV, maxV}] = weight;
}

void Graph::updateEdgeWeight(int v1, int v2, double weight) {
    int minV = std::min(v1, v2);
    int maxV = std::max(v1, v2);

    if (edges.find({minV, maxV}) != edges.end()) {
        edgeWeights[{minV, maxV}] = weight;
    }
}

void Graph::removeEdge(int v1, int v2) {
    int minV = std::min(v1, v2);
    int maxV = std::max(v1, v2);

    edges.erase({minV, maxV});
    edgeWeights.erase({minV, maxV});
}

Vex Graph::getVex(int vexNum) const {
    auto it = vexs.find(vexNum);
    if (it != vexs.end()) {
        return it->second;
    }
    return Vex();
}

int Graph::getVexIndex(const std::string& name) const {
    std::string trimmedName = name;
    trimmedName.erase(trimmedName.find_last_not_of(" \n\r\t") + 1); // 去除空格

    for (const auto& pair : vexs) {
        if (pair.second.name == trimmedName) {
            return pair.first;
        }
    }
    return -1;
}

std::vector<Vex> Graph::getAllVexs() const {
    std::vector<Vex> result;
    for (const auto& pair : vexs) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<Edge> Graph::getAllEdges() const {
    std::vector<Edge> result;
    for (const auto& edge : edges) {
        result.push_back({edge.first, edge.second, edgeWeights.at(edge)});
    }
    return result;
}

std::map<int, std::vector<std::pair<int, double>>> Graph::getAdjacencyList() const {
    std::map<int, std::vector<std::pair<int, double>>> adjacencyList;
    for (const auto& vexPair : vexs) {
        adjacencyList[vexPair.first] = std::vector<std::pair<int, double>>();
    }
    for (const auto& edge : edges) {
        int v1 = edge.first;
        int v2 = edge.second;
        double weight = edgeWeights.at(edge);
        adjacencyList[v1].push_back({v2, weight});
        adjacencyList[v2].push_back({v1, weight}); // 无向图
    }
    return adjacencyList;
}
