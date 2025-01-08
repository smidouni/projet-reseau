// graph.cpp
#include "graph.h"
#include <cmath>
#include <limits>
#include <queue>
#include <QDebug>
#include <QSet>
#include <QQueue>
#include <QRandomGenerator>

// Define qHash for QPair<qint64, qint64> to allow using QPair in QSet
uint qHash(const QPair<qint64, qint64>& key, uint seed)
{
    return qHash(key.first, seed) ^ qHash(key.second, seed);
}

Graph::Graph() {}

void Graph::addNode(qint64 id, const QGeoCoordinate &coordinate) {
    if (!nodes.contains(id)) {
        nodes[id] = new Node(id, coordinate);
    }
}

void Graph::addEdge(qint64 startId, qint64 endId, double length) {
    if (startId == endId) {
        return; // Prevent adding self-referential edges
    }

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

void Graph::blockEdge(qint64 startId, qint64 endId) {
    if (startId == endId) {
        qWarning() << "Cannot block an edge with the same start and end node:" << startId;
        return; // Prevent blocking self-referential edges
    }

    QPair<qint64, qint64> edgeKey = qMakePair(startId, endId);
    if (edges.contains(edgeKey)) {
        blockedEdges.insert(edgeKey);
        blockedEdges.insert(qMakePair(endId, startId)); // Ensure bidirectional blocking
        edges[edgeKey]->blocked = true;
        edges[qMakePair(endId, startId)]->blocked = true;
        qDebug() << "Blocked edge between" << startId << "and" << endId;
    } else {
        qWarning() << "Attempted to block a non-existent edge between" << startId << "and" << endId;
    }
}


void Graph::unblockEdge(qint64 startId, qint64 endId) {
    QPair<qint64, qint64> edgeKey = qMakePair(startId, endId);
    if (blockedEdges.contains(edgeKey)) {
        blockedEdges.remove(edgeKey);
        blockedEdges.remove(qMakePair(endId, startId)); // Ensure bidirectional unblocking
        edges[edgeKey]->blocked = false;
        edges[qMakePair(endId, startId)]->blocked = false;
        qDebug() << "Unblocked edge between" << startId << "and" << endId;
    } else {
        qWarning() << "Attempted to unblock a non-blocked edge between" << startId << "and" << endId;
    }
}

QList<QPair<qint64, qint64>> Graph::getBlockedEdges() const {
    QList<QPair<qint64, qint64>> list;
    for (const auto &pair : blockedEdges) {
        list.append(pair);
    }
    return list;
}

void Graph::placeRandomObstacles(int count) {
    int totalUniqueEdges = edges.size() / 2; // Since edges are bidirectional
    if (totalUniqueEdges == 0) {
        qWarning() << "No edges available to place obstacles.";
        return;
    }

    int placed = 0;
    int attempt = 0;
    int maxAttempts = count * 10; // Prevent infinite loops

    QList<QPair<qint64, qint64>> uniqueEdges;
    for (auto it = edges.begin(); it != edges.end(); ++it) {
        // To avoid duplicates, only add one direction
        if (it.key().first < it.key().second) {
            uniqueEdges.append(it.key());
        }
    }

    while (placed < count && attempt < maxAttempts) {
        // Select a random edge
        int randomIndex = QRandomGenerator::global()->bounded(uniqueEdges.size());
        QPair<qint64, qint64> edgeKey = uniqueEdges.at(randomIndex);
        Edge* edge = edges.value(edgeKey);

        // Skip if already blocked
        if (blockedEdges.contains(edgeKey)) {
            attempt++;
            continue;
        }

        // Block the edge
        blockEdge(edgeKey.first, edgeKey.second);

        placed++;
        attempt++;
    }

    if (placed < count) {
        qWarning() << "Could only place" << placed << "obstacles out of requested" << count;
    } else {
        qDebug() << "Successfully placed" << placed << "obstacles on the graph.";
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
            qint64 neighborId = (neighborEdge->start->id == currentId)
            ? neighborEdge->end->id
            : neighborEdge->start->id;

            QPair<qint64, qint64> edgeKey = qMakePair(currentId, neighborId);

            // Skip blocked edges
            if (blockedEdges.contains(edgeKey)) {
                continue;
            }

            // Skip any additional avoidEdges
            if (avoidEdges.contains(edgeKey) || avoidEdges.contains(qMakePair(edgeKey.second, edgeKey.first))) {
                continue;
            }

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

        // Collect nodes that have degree=2
        QList<qint64> toRemove;
        for (auto nId : simplified.nodes.keys()) {
            auto edgesList = simplified.adjacencyList.value(nId);
            if (edgesList.size() == 2) {
                toRemove.append(nId);
            }
        }

        for (auto midId : toRemove) {
            auto edgesList = simplified.adjacencyList.value(midId);
            // Could be empty if we removed it in the same pass
            if (edgesList.size() != 2) {
                continue;
            }

            // Neighbors are A and B
            Edge *edge1 = edgesList[0];
            Edge *edge2 = edgesList[1];

            // Identify A, B
            qint64 aId = (edge1->start->id == midId) ? edge1->end->id : edge1->start->id;
            qint64 bId = (edge2->start->id == midId) ? edge2->end->id : edge2->start->id;

            // Combine distance
            double newLen = edge1->length + edge2->length;

            // Remove midId from the graph
            for (Edge *e : edgesList) {
                auto p1 = qMakePair(e->start->id, e->end->id);
                auto p2 = qMakePair(e->end->id, e->start->id);
                simplified.edges.remove(p1);
                simplified.edges.remove(p2);

                // Remove adjacency references
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
