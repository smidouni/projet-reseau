#include "vehicle.h"
#include <QtMath>
#include <QTimer>
#include <QColor>

// Adjust these as you see fit
static const int MAX_START_RETRIES = 50;
static const double MIN_SPEED = 50.0;
static const double MAX_SPEED = 130.0;

Vehicle::Vehicle(int id, Graph &graph, qint64 startNodeId)
    : id(id),
    graph(graph),
    currentNodeId(startNodeId),
    destinationNodeId(-1),
    speed(0.0)
{
    // 1) Random speed
    speed = QRandomGenerator::global()->bounded(MAX_SPEED - MIN_SPEED) + MIN_SPEED;

    // 2) Random bright color
    pickRandomColor();

    // 3) If you have a path-loss formula, you could do it here:
    // rangeMeters = yourFreeSpacePathLossComputation(...);

    // 4) Attempt to pick a valid path
    if (!graph.nodes.contains(currentNodeId)) {
        currentNodeId = graph.nodes.keys().at(
            QRandomGenerator::global()->bounded(graph.nodes.size())
            );
    }
    bool initOK = tryInitValidStartNode();
    if (!initOK) {
        currentPosition = graph.nodes[currentNodeId]->coordinate;
        qWarning() << "Vehicle" << id
                   << "couldn’t find valid path from start, may remain stuck.";
    } else {
        currentPosition = graph.nodes[currentNodeId]->coordinate;
    }

    emit positionChanged();
}

double Vehicle::lat() const
{
    return currentPosition.latitude();
}

double Vehicle::lon() const
{
    return currentPosition.longitude();
}

void Vehicle::updatePosition(double deltaTime)
{
    if (currentPath.totalLength() < 1e-6) {
        return;
    }

    // speed is e.g. 80 => if that's km/h, convert to m/s
    // We'll do a rough conversion: 1 km/h = 1000/3600 = ~0.2777 m/s
    double speed_m_s = speed * (1000.0/3600.0);

    double travelDistance = speed_m_s * deltaTime;
    distanceAlongPath += travelDistance;

    if (distanceAlongPath > currentPath.totalLength()) {
        distanceAlongPath = currentPath.totalLength();
    }

    if (distanceAlongPath >= currentPath.totalLength()) {
        qint64 finalNode = currentPath.getFinalNodeId();
        if (finalNode >= 0) {
            currentNodeId = finalNode;
        }
        setRandomDestination();
        distanceAlongPath = 0.0;
        currentPosition = currentPath.getPositionAtDistance(0.0);
        emit positionChanged();
        return;
    }

    currentPosition = currentPath.getPositionAtDistance(distanceAlongPath);
    emit positionChanged();
}

void Vehicle::handleObstacleOnEdge()
{
    if (distanceAlongPath > 0.0) {
        backtrackToPreviousNode();
    } else {
        recalculatePath();
    }
}

void Vehicle::setDestination(qint64 destinationNodeId)
{
    this->destinationNodeId = destinationNodeId;
    recalculatePath();
}

void Vehicle::setRandomDestination()
{
    if (graph.nodes.size() <= 1) {
        qWarning() << "Vehicle" << id << "Not enough nodes to pick a random destination.";
        return;
    }

    qint64 newDest = currentNodeId;
    while (newDest == currentNodeId) {
        newDest = graph.nodes.keys().at(
            QRandomGenerator::global()->bounded(graph.nodes.size())
            );
    }
    setDestination(newDest);
}

void Vehicle::recalculatePath()
{
    if (!graph.nodes.contains(currentNodeId)) {
        qWarning() << "Vehicle" << id << "recalculatePath: currentNodeId"
                   << currentNodeId << "not in graph!";
        return;
    }

    // We pass blockedEdges if we want to avoid obstacles
    QList<Edge*> pathEdges = graph.findPath(currentNodeId, destinationNodeId, blockedEdges);
    if (pathEdges.isEmpty()) {
        qWarning() << "Vehicle" << id << "No path found from" << currentNodeId << "to" << destinationNodeId;
        currentPath = Path();
        distanceAlongPath = 0.0;

        if (graph.nodes.contains(currentNodeId)) {
            currentPosition = graph.nodes[currentNodeId]->coordinate;
            emit positionChanged();
        }
        setRandomDestination();
        return;
    }

    currentPath = Path(pathEdges, currentNodeId);
    distanceAlongPath = 0.0;
    currentPosition = currentPath.getPositionAtDistance(0.0);
    emit positionChanged();
}

// A private helper to get a random bright color in HSV
void Vehicle::pickRandomColor()
{
    // Hue in [0..359], saturation around 0.7..1.0, value around 0.7..1.0 for brightness
    double hue = QRandomGenerator::global()->bounded(360.0);
    double saturation = 0.7 + 0.3 * QRandomGenerator::global()->generateDouble();
    double value = 0.7 + 0.3 * QRandomGenerator::global()->generateDouble();

    QColor c;
    c.setHsvF(hue/360.0, saturation, value);

    // Convert to #RRGGBB
    colorString = c.name(QColor::HexRgb); // e.g. "#FF00FF"
}

void Vehicle::backtrackToPreviousNode()
{
    QList<Edge*> edges = currentPath.getEdges();
    if (!edges.isEmpty()) {
        Edge* firstEdge = edges.first();
        currentNodeId = firstEdge->start->id;
        recalculatePath();
    }
}



bool Vehicle::tryInitValidStartNode()
{
    for (int attempt = 0; attempt < MAX_START_RETRIES; ++attempt) {
        qint64 testDest = currentNodeId;
        while (testDest == currentNodeId && graph.nodes.size() > 1) {
            testDest = graph.nodes.keys().at(
                QRandomGenerator::global()->bounded(graph.nodes.size())
                );
        }

        QList<Edge*> pathEdges = graph.findPath(currentNodeId, testDest);
        if (!pathEdges.isEmpty()) {
            currentPath = Path(pathEdges, currentNodeId);
            distanceAlongPath = 0.0;
            currentPosition = currentPath.getPositionAtDistance(0.0);
            destinationNodeId = testDest;
            return true;
        }

        qWarning() << "Vehicle" << id
                   << "No path from" << currentNodeId << "to" << testDest
                   << "(attempt" << attempt << ") picking new start node.";

        currentNodeId = graph.nodes.keys().at(
            QRandomGenerator::global()->bounded(graph.nodes.size())
            );
    }
    return false;
}
