#include "vehicle.h"
#include <QRandomGenerator>

Vehicle::Vehicle(int id, Graph &graph, qint64 startNodeId)
    : id(id), graph(graph), currentNodeId(startNodeId), distanceAlongPath(0), currentPath() {
    if (graph.nodes.isEmpty()) {
        qWarning() << "Graph is empty, cannot initialize Vehicle properly.";
        return;
    }

    if (graph.nodes.size() > 0) {
        setDestination(graph.nodes.keys().at(QRandomGenerator::global()->bounded(graph.nodes.keys().size())));
    }
}

void Vehicle::updatePosition(double deltaTime) {
    double travelDistance = speed * deltaTime;
    distanceAlongPath += travelDistance;

    if (distanceAlongPath >= currentPath.totalLength()) {
        if (graph.nodes.size() > 0) {
            setDestination(graph.nodes.keys().at(QRandomGenerator::global()->bounded(graph.nodes.keys().size())));
        }
        distanceAlongPath = 0;
    }
    currentPosition = currentPath.getPositionAtDistance(distanceAlongPath);
}

void Vehicle::setDestination(qint64 destinationNodeId) {
    this->destinationNodeId = destinationNodeId;
    recalculatePath();
}

void Vehicle::recalculatePath() {
    QList<Edge*> pathEdges = graph.findPath(currentNodeId, destinationNodeId);
    currentPath = Path(pathEdges);
    distanceAlongPath = 0;
}

void Vehicle::handleObstacleOnEdge() {
    if (distanceAlongPath > 0) {
        backtrackToPreviousNode();
    } else {
        recalculatePath();
    }
}

void Vehicle::backtrackToPreviousNode() {
    if (!currentPath.getEdges().isEmpty()) {
        currentNodeId = currentPath.getEdges().first()->start->id;
        recalculatePath();
    }
}
