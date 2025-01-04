#include "vehicle.h"
#include <QRandomGenerator>
#include <QDebug>

Vehicle::Vehicle(int id, Graph &graph, qint64 startNodeId)
    : id(id),
    graph(graph),
    currentNodeId(startNodeId),
    destinationNodeId(-1),
    distanceAlongPath(0.0)
{
    if (graph.nodes.isEmpty()) {
        qWarning() << "Graph is empty, cannot initialize Vehicle properly.";
        return;
    }

    // Pour une éventuelle initialisation de la position
    if (graph.nodes.contains(startNodeId)) {
        currentPosition = graph.nodes[startNodeId]->coordinate;
        emit positionChanged();  // Pour que QML sache qu'on a une position initiale
    }

    // On choisit un noeud de destination au hasard
    setRandomDestination();
}

void Vehicle::updatePosition(double deltaTime)
{
    double travelDistance = speed * deltaTime;
    distanceAlongPath += travelDistance;

    // Si on a atteint la fin du path, on choisit un nouveau
    if (distanceAlongPath >= currentPath.totalLength()) {
        // On récupère le dernier segment pour mettre à jour currentNodeId
        QList<Edge*> edges = currentPath.getEdges();
        if (!edges.isEmpty()) {
            const Edge* lastEdge = edges.last();
            // Selon le sens du dernier segment, on définit le nouveau node
            // Mais on peut aussi laisser recalculatePath gérer ça
        }

        setRandomDestination();
        distanceAlongPath = 0.0;
    }

    // Met à jour la position (même si pathLength=0, on ne veut pas de NaN)
    currentPosition = currentPath.getPositionAtDistance(distanceAlongPath);

    // Informe QML que lat/lon ont changé
    emit positionChanged();
}

void Vehicle::setDestination(qint64 destinationNodeId)
{
    if (destinationNodeId < 0) {
        qWarning() << "Invalid destinationNodeId:" << destinationNodeId;
        return;
    }
    this->destinationNodeId = destinationNodeId;
    recalculatePath();
}

void Vehicle::setRandomDestination()
{
    if (graph.nodes.isEmpty()) {
        qWarning() << "Graph is empty, cannot set random destination.";
        return;
    }
    // On prend un node au hasard différent du currentNodeId (si possible)
    qint64 newDest = 0;
    if (graph.nodes.size() > 1) {
        do {
            newDest = graph.nodes.keys().at(
                QRandomGenerator::global()->bounded(graph.nodes.size()));
        } while (newDest == currentNodeId);
    } else {
        // Si on n'a qu'un seul noeud, on ne peut pas faire mieux
        newDest = currentNodeId;
    }
    setDestination(newDest);
}

void Vehicle::recalculatePath()
{
    if (!graph.nodes.contains(currentNodeId)) {
        qWarning() << "Current node" << currentNodeId << "does not exist in graph!";
        return;
    }

    // Recherche d'un chemin entre currentNodeId et destinationNodeId
    QList<Edge*> pathEdges = graph.findPath(currentNodeId, destinationNodeId);

    // Si le chemin est vide, on loggue un avertissement
    if (pathEdges.isEmpty()) {
        qWarning() << "No path found from" << currentNodeId
                   << "to" << destinationNodeId
                   << "(Vehicle" << id << ").";
        // On peut remettre la destination à -1 pour retenter plus tard
        // ou simplement laisser distanceAlongPath à 0
        return;
    }

    // Construit le path avec la notion de direction
    currentPath = Path(pathEdges, currentNodeId);

    // On remet la distance à zéro
    distanceAlongPath = 0.0;

    // Position initiale = tout début du chemin
    currentPosition = currentPath.getPositionAtDistance(0.0);

    // Informe QML qu'on a bougé
    emit positionChanged();
}

void Vehicle::handleObstacleOnEdge()
{
    // Cas où la distanceAlongPath est déjà > 0
    if (distanceAlongPath > 0.0) {
        backtrackToPreviousNode();
    } else {
        recalculatePath();
    }
}

void Vehicle::backtrackToPreviousNode()
{
    QList<Edge*> edges = currentPath.getEdges();
    if (!edges.isEmpty()) {
        // Le premier segment = le point de départ dans le path actuel
        // S'il était "forward", currentNodeId = edge->start->id
        // s'il était "backward", currentNodeId = edge->end->id
        // On va donc "revenir" à ce node
        const Edge* firstEdge = edges.first();
        // Dans Path, on calcule la direction en fonction de startNodeId,
        // donc si le segment est forward = (firstEdge->start->id == currentNodeId)
        // ici on peut directement se caler sur firstEdge->start->id
        currentNodeId = firstEdge->start->id;

        // Now we recalc the path to the existing destination
        recalculatePath();
    }
}
