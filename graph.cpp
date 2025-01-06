#include "graph.h"
#include <cmath>
#include <limits>
#include <queue>
#include <QDebug>
#include <QSet>
#include <QQueue>

Graph::Graph() {}

void Graph::addNode(qint64 id, const QGeoCoordinate &coordinate) {
    if (!nodes.contains(id)) {
        nodes[id] = new Node(id, coordinate);
    }
}

void Graph::addEdge(qint64 startId, qint64 endId, double length) {
    if (nodes.contains(startId) && nodes.contains(endId)) {
        Node *startNode = nodes[startId];
        Node *endNode   = nodes[endId];
        Edge *edge      = new Edge(startNode, endNode, length);
        edges[qMakePair(startId, endId)] = edge;
        edges[qMakePair(endId, startId)] = edge;  // Bidirectional
        adjacencyList[startId].append(edge);
        adjacencyList[endId].append(edge);
    }
}

double Graph::heuristic(const Node &a, const Node &b) const {
    return a.coordinate.distanceTo(b.coordinate);
}

QList<Edge*> Graph::findPath(qint64 startId, qint64 endId,
                              const QSet<QPair<qint64, qint64>> &avoidEdges)
{
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

    gScore[startId] = 0.0;
    fScore[startId] = heuristic(*nodes[startId], *nodes[endId]);
    openSet.push({startId, fScore[startId]});

    while (!openSet.empty()) {
        qint64 currentId = openSet.top().first;
        openSet.pop();

        if (currentId == endId) {
            // Reconstruct the path
            QList<Edge*> path;
            qint64 curr = endId;
            while (cameFrom.contains(curr)) {
                qint64 prev = cameFrom[curr];
                Edge *edge = edges.value(qMakePair(prev, curr));
                if (edge) {
                    path.prepend(edge);
                }
                curr = prev;
            }
            return path;
        }

        // Explore neighbors
        for (Edge *neighborEdge : adjacencyList[currentId]) {
            if (neighborEdge->blocked) {
                continue;
            }
            auto eId = qMakePair(neighborEdge->start->id, neighborEdge->end->id);
            if (avoidEdges.contains(eId) || avoidEdges.contains(qMakePair(eId.second, eId.first))) {
                continue;
            }

            qint64 neighborId = (neighborEdge->start->id == currentId)
                                    ? neighborEdge->end->id
                                    : neighborEdge->start->id;

            double tentativeGScore = gScore[currentId] + neighborEdge->length;
            if (tentativeGScore < gScore[neighborId]) {
                cameFrom[neighborId] = currentId;
                gScore[neighborId]   = tentativeGScore;
                fScore[neighborId]   = tentativeGScore + heuristic(*nodes[neighborId], *nodes[endId]);
                openSet.push({neighborId, fScore[neighborId]});
            }
        }
    }
    return {};
}

/**
 * @brief Graph::createSimplifiedGraph
 * Merges consecutive degree-2 nodes in a simple while loop.
 */
Graph Graph::createSimplifiedGraph() const
{
    // Make a deep copy of this graph
    Graph simplified;

    // 1) Copy nodes
    for (auto nodeId : nodes.keys()) {
        simplified.addNode(nodeId, nodes[nodeId]->coordinate);
    }

    // 2) Copy edges
    for (auto ePair : edges.keys()) {
        auto e = edges[ePair];
        if (!e) continue;
        simplified.addEdge(e->start->id, e->end->id, e->length);
    }

    // 3) Repeatedly remove degree-2 nodes
    bool changed = true;
    while (changed) {
        changed = false;

        // We'll collect nodes that have degree=2
        QList<qint64> toRemove;
        for (auto nId : simplified.nodes.keys()) {
            auto edgesList = simplified.adjacencyList.value(nId);
            if (edgesList.size() == 2) {
                toRemove.append(nId);
            }
        }

        for (auto midId : toRemove) {
            auto edgesList = simplified.adjacencyList.value(midId);
            // Could be empty if we removed it in same pass
            if (edgesList.size() != 2) {
                continue;
            }

            // Let's say midId neighbors are A and B
            Edge *edge1 = edgesList[0];
            Edge *edge2 = edgesList[1];

            // Identify A, B
            qint64 aId = (edge1->start->id == midId) ? edge1->end->id : edge1->start->id;
            qint64 bId = (edge2->start->id == midId) ? edge2->end->id : edge2->start->id;

            // Combine distance
            double newLen = edge1->length + edge2->length;

            // Remove midId from the graph
            //  (Remove all edges referencing midId, remove midId node)
            for (Edge *e : edgesList) {
                auto p1 = qMakePair(e->start->id, e->end->id);
                auto p2 = qMakePair(e->end->id, e->start->id);
                simplified.edges.remove(p1);
                simplified.edges.remove(p2);

                // Also remove adjacency references
                simplified.adjacencyList[e->start->id].removeAll(e);
                simplified.adjacencyList[e->end->id].removeAll(e);
                delete e;
            }
            simplified.adjacencyList.remove(midId);
            delete simplified.nodes[midId];
            simplified.nodes.remove(midId);

            // Add new edge A-B if it doesn’t already exist
            auto pAB = qMakePair(aId, bId);
            auto pBA = qMakePair(bId, aId);
            if (!simplified.edges.contains(pAB) && !simplified.edges.contains(pBA)) {
                simplified.addEdge(aId, bId, newLen);
            }
            changed = true;
        }
    }

    return simplified;
}
