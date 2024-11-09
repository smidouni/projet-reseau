#ifndef GRAPH_H
#define GRAPH_H

#include <QMap>
#include <QList>
#include <QSet>
#include <QGeoCoordinate>
#include <QPair>
#include "Node.h"
#include "Edge.h"

class Graph {
public:
    Graph();

    void addNode(qint64 id, const QGeoCoordinate &coordinate);
    void addEdge(qint64 startId, qint64 endId, double length);
    QList<Edge*> findPath(qint64 startId, qint64 endId, const QSet<QPair<qint64, qint64>> &avoidEdges = {});

    QMap<qint64, Node*> nodes;

    // Public getter for edges to allow controlled access
    const QMap<QPair<qint64, qint64>, Edge*>& getEdges() const { return edges; }

private:
    QMap<QPair<qint64, qint64>, Edge*> edges;
    QMap<qint64, QList<Edge*>> adjacencyList;

    double heuristic(const Node &a, const Node &b) const;
};

#endif // GRAPH_H
