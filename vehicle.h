#ifndef VEHICLE_H
#define VEHICLE_H

#include <QObject>
#include <QGeoCoordinate>
#include <QSet>
#include <QPair>
#include <QColor>
#include <QDebug>
#include <QRandomGenerator>
#include <QTimer>
#include "path.h"
#include "graph.h"

// Forward declaration to avoid circular dependency
class SimulationManager;

/**
 * @brief The Vehicle class
 * Represents a vehicle in the simulation.
 */
class Vehicle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double lat READ lat NOTIFY positionChanged)
    Q_PROPERTY(double lon READ lon NOTIFY positionChanged)
    Q_PROPERTY(double communicationRange READ communicationRange WRITE setCommunicationRange NOTIFY communicationRangeChanged)
    Q_PROPERTY(QString color READ color NOTIFY colorChanged) // Exposed color property
    Q_PROPERTY(bool messageReceived READ messageReceived WRITE setMessageReceived NOTIFY messageReceivedChanged)

public:
    Vehicle(int id, Graph &graph, qint64 startNodeId, QObject *parent = nullptr);

    double lat() const;
    double lon() const;
    double communicationRange() const;
    QString color() const; // Getter for color
    bool messageReceived() const;
    void setMessageReceived(bool received);

    void setCommunicationRange(double range);

    void updatePosition(double deltaTime);

    void setDestination(qint64 destinationNodeId);
    void setRandomDestination();

    int getId() const;

    QGeoCoordinate getCurrentPosition() const; // Getter for currentPosition


public slots:
    void receiveObstacle(const QPair<qint64, qint64> &blockedEdge);
    void reportObstacle(const QPair<qint64, qint64> &blockedEdge, SimulationManager* manager);
    bool currentPathHasEdge(const QPair<qint64, qint64> &edge) const;

signals:
    void positionChanged();
    void communicationRangeChanged();
    void colorChanged(); // Signal for color changes
    void messageReceivedChanged();

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
    double m_communicationRange;
    bool recalculatePathAtNextNode;
    QSet<QPair<qint64, qint64>> knownBlockedEdges;

    void recalculatePath();
    void backtrackToPreviousNode();
    bool tryInitValidStartNode();
    void pickRandomColor(double frequency);

    bool m_messageReceived = false;
    QTimer messageTimer; // Timer for signaling system

};

#endif // VEHICLE_H
