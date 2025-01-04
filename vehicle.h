#ifndef VEHICLE_H
#define VEHICLE_H

#include <QObject>
#include <QGeoCoordinate>
#include "path.h"
#include "graph.h"

class Vehicle : public QObject {
    Q_OBJECT
    Q_PROPERTY(double lat READ lat NOTIFY positionChanged)
    Q_PROPERTY(double lon READ lon NOTIFY positionChanged)

public:
    Vehicle(int id, Graph &graph, qint64 startNodeId);

    double lat() const { return currentPosition.latitude(); }
    double lon() const { return currentPosition.longitude(); }

    void updatePosition(double deltaTime);
    void handleObstacleOnEdge();

    void setDestination(qint64 destinationNodeId);
    void setRandomDestination();
signals:
    void positionChanged();

private:
    int id;
    Graph &graph;
    qint64 currentNodeId;
    qint64 destinationNodeId;
    double speed = 50.0;
    double distanceAlongPath;
    QGeoCoordinate currentPosition;
    Path currentPath;

    void recalculatePath();
    void backtrackToPreviousNode();
};

#endif // VEHICLE_H
