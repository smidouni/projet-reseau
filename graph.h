// graph.h
#ifndef GRAPH_H
#define GRAPH_H

#include <QMap>
#include <QList>
#include <QSet>
#include <QGeoCoordinate>
#include <QPair>
#include "node.h"
#include "edge.h"

class Graph {
public:
    Graph();

    void addNode(qint64 id, const QGeoCoordinate &coordinate);
    void addEdge(qint64 startId, qint64 endId, double length);

    /**
     * @brief findPath
     * A* to find a path from startId to endId.
     */
    QList<Edge*> findPath(qint64 startId, qint64 endId,
                           const QSet<QPair<qint64, qint64>> &avoidEdges = {});

    /**
     * @brief createSimplifiedGraph
     * Creates and returns a new Graph that merges consecutive degree-2 nodes into single edges.
     */
    Graph createSimplifiedGraph() const;

    QMap<qint64, Node*> nodes;

    const QMap<QPair<qint64, qint64>, Edge*>& getEdges() const { return edges; }

    // New Methods for Obstacles
    QList<QPair<qint64, qint64>> getBlockedEdges() const; // Declaration only
    void placeRandomObstacles(int count);
    void blockEdge(qint64 startId, qint64 endId);
    void unblockEdge(qint64 startId, qint64 endId);

private:
    QMap<QPair<qint64, qint64>, Edge*> edges;
    QMap<qint64, QList<Edge*>> adjacencyList;

    // Set to store blocked edges as pairs of node IDs
    QSet<QPair<qint64, qint64>> blockedEdges;

    double heuristic(const Node &a, const Node &b) const;

    friend class OSMImporter;
};

#endif // GRAPH_H
