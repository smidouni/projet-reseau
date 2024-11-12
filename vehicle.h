#ifndef VEHICLE_H
#define VEHICLE_H

#include <QObject>
#include <QGeoCoordinate>
#include "Path.h"
#include "Graph.h"

class Vehicle : public QObject {
    Q_OBJECT

public:
    Vehicle(int id, Graph &graph, qint64 startNodeId);
    void updatePosition(double deltaTime);
    void handleObstacleOnEdge();

    QGeoCoordinate getCurrentPosition() const { return currentPosition; }
    void setDestination(qint64 destinationNodeId);

private:
    int id;
    Graph &graph;
    qint64 currentNodeId;
    qint64 destinationNodeId;
    double speed = 10.0;
    double distanceAlongPath;
    QGeoCoordinate currentPosition;
    Path currentPath;

    void recalculatePath();
    void backtrackToPreviousNode();
};

#endif // VEHICLE_H
