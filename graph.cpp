#include "Graph.h"
#include <cmath>
#include <limits>
#include <queue>

Graph::Graph() {}

void Graph::addNode(qint64 id, const QGeoCoordinate &coordinate) {
    if (!nodes.contains(id)) {
        nodes[id] = new Node(id, coordinate);
    }
}

void Graph::addEdge(qint64 startId, qint64 endId, double length) {
    if (nodes.contains(startId) && nodes.contains(endId)) {
        Node *startNode = nodes[startId];
        Node *endNode = nodes[endId];
        Edge *edge = new Edge(startNode, endNode, length);
        edges[qMakePair(startId, endId)] = edge;
        edges[qMakePair(endId, startId)] = edge;  // Bidirectional
        adjacencyList[startId].append(edge);
        adjacencyList[endId].append(edge);
    }
}

double Graph::heuristic(const Node &a, const Node &b) const {
    return a.coordinate.distanceTo(b.coordinate);
}

QList<Edge*> Graph::findPath(qint64 startId, qint64 endId, const QSet<QPair<qint64, qint64>> &avoidEdges) {
    if (!nodes.contains(startId) || !nodes.contains(endId)) {
        return {};
    }

    auto compare = [](const QPair<qint64, double> &a, const QPair<qint64, double> &b) {
        return a.second > b.second;
    };
    std::priority_queue<QPair<qint64, double>, std::vector<QPair<qint64, double>>, decltype(compare)> openSet(compare);

    QMap<qint64, qint64> cameFrom;
    QMap<qint64, double> gScore;
    QMap<qint64, double> fScore;

    for (auto nodeId : nodes.keys()) {
        gScore[nodeId] = std::numeric_limits<double>::infinity();
        fScore[nodeId] = std::numeric_limits<double>::infinity();
    }

    gScore[startId] = 0;
    fScore[startId] = heuristic(*nodes[startId], *nodes[endId]);
    openSet.push({startId, fScore[startId]});

    while (!openSet.empty()) {
        qint64 currentId = openSet.top().first;
        openSet.pop();

        if (currentId == endId) {
            QList<Edge*> path;
            qint64 current = endId;
            while (cameFrom.contains(current)) {
                qint64 previous = cameFrom[current];
                Edge *edge = edges[qMakePair(previous, current)];
                if (edge) {
                    path.prepend(edge);
                }
                current = previous;
            }
            return path;
        }

        for (Edge *neighborEdge : adjacencyList[currentId]) {
            if (neighborEdge->blocked || avoidEdges.contains({neighborEdge->start->id, neighborEdge->end->id})) {
                continue;
            }

            qint64 neighborId = (neighborEdge->start->id == currentId) ? neighborEdge->end->id : neighborEdge->start->id;
            double tentativeGScore = gScore[currentId] + neighborEdge->length;

            if (tentativeGScore < gScore[neighborId]) {
                cameFrom[neighborId] = currentId;
                gScore[neighborId] = tentativeGScore;
                fScore[neighborId] = tentativeGScore + heuristic(*nodes[neighborId], *nodes[endId]);
                openSet.push({neighborId, fScore[neighborId]});
            }
        }
    }
    return {};  // Pas de chemin
}
