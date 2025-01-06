#include "vehicle.h"

static const int MAX_START_RETRIES = 10;

Vehicle::Vehicle(int id, Graph &graph, qint64 startNodeId)
    : id(id),
    graph(graph),
    currentNodeId(startNodeId),
    destinationNodeId(-1),
    speed(100.0),
    distanceAlongPath(0.0)
{
    if (graph.nodes.isEmpty()) {
        qWarning() << "Graph is empty, cannot initialize Vehicle properly.";
        return;
    }

    // If startNodeId is not valid in the graph, pick a random one
    if (!graph.nodes.contains(currentNodeId)) {
        currentNodeId = graph.nodes.keys().at(
            QRandomGenerator::global()->bounded(graph.nodes.size())
            );
        qWarning() << "Vehicle" << id << "startNodeId was invalid; picked random node"
                   << currentNodeId;
    }

    // Attempt to spawn on a valid node (one that can reach at least some other node)
    bool initOK = tryInitValidStartNode();
    if (!initOK) {
        // If we fail, we keep the node, but we likely won't move
        // because no path can be found from here.
        currentPosition = graph.nodes[currentNodeId]->coordinate;
        qWarning() << "Vehicle" << id
                   << ": could not find any valid path from start node after"
                   << MAX_START_RETRIES << "tries. Vehicle may remain stuck.";
    } else {
        // If success, we already set a random destination in tryInitValidStartNode()
        currentPosition = graph.nodes[currentNodeId]->coordinate;
    }

    emit positionChanged();
}

bool Vehicle::tryInitValidStartNode()
{
    // We'll attempt up to MAX_START_RETRIES:
    for (int attempt = 0; attempt < MAX_START_RETRIES; ++attempt) {

        // 1) Try a random destination from currentNodeId
        qint64 testDest = currentNodeId;
        while (testDest == currentNodeId && graph.nodes.size() > 1) {
            testDest = graph.nodes.keys().at(
                QRandomGenerator::global()->bounded(graph.nodes.size())
                );
        }

        QList<Edge*> pathEdges = graph.findPath(currentNodeId, testDest);
        if (!pathEdges.isEmpty()) {
            // We found at least one route from currentNodeId to testDest
            // => Keep that as our initial valid route
            currentPath = Path(pathEdges, currentNodeId);
            distanceAlongPath = 0.0;
            currentPosition = currentPath.getPositionAtDistance(0.0);

            // We store testDest in destinationNodeId for normal usage
            destinationNodeId = testDest;
            return true;
        }

        // If no path found, pick a new random start node and try again
        qWarning() << "Vehicle" << id
                   << "No path found from" << currentNodeId
                   << "to" << testDest << "(init attempt" << attempt << ")."
                   << "Choosing a new start node.";

        currentNodeId = graph.nodes.keys().at(
            QRandomGenerator::global()->bounded(graph.nodes.size())
            );
    }
    return false; // all attempts failed
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
    // If no valid path, do nothing
    if (currentPath.totalLength() < 1e-6) {
        return;
    }

    double travelDistance = speed * deltaTime;
    distanceAlongPath += travelDistance;

    // Clamp to avoid overshoot
    if (distanceAlongPath > currentPath.totalLength()) {
        distanceAlongPath = currentPath.totalLength();
    }

    if (distanceAlongPath >= currentPath.totalLength()) {
        qint64 finalNode = currentPath.getFinalNodeId();
        if (finalNode >= 0) {
            currentNodeId = finalNode;
        } else {
            qWarning() << "Vehicle" << id << "finalNode is invalid (-1).";
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

    QList<Edge*> pathEdges = graph.findPath(currentNodeId, destinationNodeId);
    if (pathEdges.isEmpty()) {
        // Reset path if we can't go anywhere
        qWarning() << "Vehicle" << id << "No path found from"
                   << currentNodeId << "to" << destinationNodeId;
        currentPath = Path();
        distanceAlongPath = 0.0;

        if (graph.nodes.contains(currentNodeId)) {
            currentPosition = graph.nodes[currentNodeId]->coordinate;
            emit positionChanged();
        }

        // Optional: pick a new random destination in hopes to get a valid path
        setRandomDestination();
        return;
    }

    currentPath = Path(pathEdges, currentNodeId);
    distanceAlongPath = 0.0;

    currentPosition = currentPath.getPositionAtDistance(0.0);
    emit positionChanged();
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

double Vehicle::getCommunicationRange() const {
    return communicationRange;
}

void Vehicle::receiveMessage(const QString &message) {
    qDebug() << "Vehicle" << id << "received message:" << message;
}

int Vehicle::getId() const {
    return id;
}

