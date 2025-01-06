#ifndef VEHICLE_H
#define VEHICLE_H

#include <QObject>
#include <QGeoCoordinate>
#include <QDebug>
#include <QRandomGenerator>
#include "path.h"
#include "graph.h"

class Vehicle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double lat READ lat NOTIFY positionChanged)
    Q_PROPERTY(double lon READ lon NOTIFY positionChanged)

public:
    Vehicle(int id, Graph &graph, qint64 startNodeId);

    double lat() const;
    double lon() const;

    void updatePosition(double deltaTime);
    void handleObstacleOnEdge();

    void setDestination(qint64 destinationNodeId);
    void setRandomDestination();

    double getCommunicationRange() const;
    void receiveMessage(const QString &message);
    int getId() const;

signals:
    void positionChanged();

private:
    int id;
    Graph &graph;
    qint64 currentNodeId;
    qint64 destinationNodeId;
    double speed;
    double distanceAlongPath;
    QGeoCoordinate currentPosition;
    Path currentPath;
    QString colorString;
    QSet<QPair<qint64, qint64>> blockedEdges;
    double communicationRange;

    void recalculatePath();
    void backtrackToPreviousNode();
    bool tryInitValidStartNode();
    void pickRandomColor();

};

#endif // VEHICLE_H
