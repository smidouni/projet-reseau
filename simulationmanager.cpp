// simulationmanager.cpp

#include "simulationmanager.h"
#include <QQueue>
#include <QColor>
#include <QDateTime>

SimulationManager::SimulationManager(Graph &graph, QObject *parent)
    : QObject(parent), graph(graph)
{
    // Connect simulation timer to updateVehicles slot
    connect(&simulationTimer, &QTimer::timeout, this, &SimulationManager::updateVehicles);
    simulationTimer.start(16); // Approximately 60 FPS
    elapsedTimer.start();

    // Initialize BlockedEdgesModel with current blocked edges
    m_blockedEdgesModel->updateBlockedEdges(graph.getBlockedEdges(), graph.getEdges());

    // Setup edge blockage timer
    connect(&edgeBlockTimer, &QTimer::timeout, this, &SimulationManager::blockRandomEdge);
    edgeBlockTimer.start(30000); // Block an edge every 30 seconds

    // Setup unblock timer
    connect(&unblockTimer, &QTimer::timeout, this, &SimulationManager::unblockExpiredEdges);
    unblockTimer.start(5000); // Check every 5 seconds

    // Block initial 20 edges to maintain around 20 blocked edges
    placeRandomObstacles(20);
}

void SimulationManager::addVehicle(int id, qint64 startNodeId)
{
    if (!graph.nodes.isEmpty()) {
        Vehicle *vehicle = new Vehicle(id, graph, startNodeId, this); // Parent set to SimulationManager
        vehicles.append(vehicle);
        emit vehiclesUpdated(); // Notify QML about the new vehicle
    } else {
        qWarning() << "Graph is empty; cannot add vehicle" << id;
    }
}

void SimulationManager::updateVehicles()
{
    qint64 elapsedMs = elapsedTimer.elapsed();
    elapsedTimer.restart();

    double deltaTime = (elapsedMs / 1000.0) * speedFactor;

    // Update each vehicle’s position
    for (Vehicle *v : vehicles) {
        v->updatePosition(deltaTime);
    }

    emit updated();
    emit vehiclesUpdated();
}

void SimulationManager::setSpeedFactor(double factor)
{
    speedFactor = factor;
}

void SimulationManager::clearVehicles()
{
    qDebug() << "Clearing vehicles...";
    for (auto vehicle : vehicles) {
        if (vehicle) {
            disconnect(vehicle, nullptr, nullptr, nullptr);  // Disconnect all signals/slots
            delete vehicle;  // Delete the object
        }
    }
    vehicles.clear();  // Clear the list
    emit vehiclesUpdated(); // Notify QML about the change
    qDebug() << "All vehicles have been cleared.";
}

QQmlListProperty<QObject> SimulationManager::vehiclesModel()
{
    return QQmlListProperty<QObject>(this, this,
                                     &SimulationManager::appendVehicle,
                                     &SimulationManager::vehicleCount,
                                     &SimulationManager::vehicleAt,
                                     &SimulationManager::clearVehicles);
}

void SimulationManager::appendVehicle(QQmlListProperty<QObject> *list, QObject *vehicle)
{
    SimulationManager *manager = qobject_cast<SimulationManager*>(list->object);
    if (manager && vehicle) {
        manager->vehicles.append(qobject_cast<Vehicle*>(vehicle));
    }
}

qint64 SimulationManager::vehicleCount(QQmlListProperty<QObject> *list)
{
    SimulationManager *manager = qobject_cast<SimulationManager*>(list->object);
    return manager ? manager->vehicles.size() : 0;
}

QObject* SimulationManager::vehicleAt(QQmlListProperty<QObject> *list, qint64 index)
{
    SimulationManager *manager = qobject_cast<SimulationManager*>(list->object);
    return (manager && index >= 0 && index < manager->vehicles.size()) ? manager->vehicles.at(index) : nullptr;
}

void SimulationManager::clearVehicles(QQmlListProperty<QObject> *list)
{
    SimulationManager *manager = qobject_cast<SimulationManager*>(list->object);
    if (manager) {
        qDeleteAll(manager->vehicles);
        manager->vehicles.clear();
        emit manager->vehiclesUpdated(); // Notify QML about the change
    }
}

Graph& SimulationManager::getGraph()
{
    return graph;
}

QList<Vehicle*> SimulationManager::findConnectedVehicles(Vehicle* startVehicle)
{
    QList<Vehicle*> connected;
    if (!startVehicle) return connected;

    QSet<Vehicle*> visited;
    QQueue<Vehicle*> queue;

    queue.enqueue(startVehicle);
    visited.insert(startVehicle);

    while (!queue.isEmpty()) {
        Vehicle* current = queue.dequeue();
        connected.append(current);

        for (Vehicle* other : vehicles) {
            if (other == current || visited.contains(other)) continue;

            QGeoCoordinate pos1(current->lat(), current->lon());
            QGeoCoordinate pos2(other->lat(), other->lon());

            double distance = pos1.distanceTo(pos2);

            if (distance <= current->communicationRange()) {
                queue.enqueue(other);
                visited.insert(other);
            }
        }
    }

    return connected;
}

void SimulationManager::handleObstacle(Vehicle *reportingVehicle, const QPair<qint64, qint64> &blockedEdge) {
    QList<Vehicle*> connectedVehicles = findConnectedVehicles(reportingVehicle);

    QList<CommunicationLink> newLinks;
    for (Vehicle *v : connectedVehicles) {
        if (v != reportingVehicle) {
            newLinks.append({reportingVehicle->getCurrentPosition(), v->getCurrentPosition()});
        }
    }

    // Update communication links in the model
    m_communicationLinksModel->setCommunicationLinks(newLinks);
    emit communicationLinksChanged();

    // Clear communication links after 1 seconds
    QTimer::singleShot(1000, [this]() {
        m_communicationLinksModel->setCommunicationLinks({});
        emit communicationLinksChanged();
    });

    // Notify all connected vehicles
    for (Vehicle *v : connectedVehicles) {
        v->receiveObstacle(blockedEdge);

        // Turn the vehicle green for 1 seconds
        v->setMessageReceived(true);
    }
}

void SimulationManager::blockRandomEdge() {
    QList<QPair<qint64, qint64>> availableEdges;
    for (auto it = graph.getEdges().constBegin(); it != graph.getEdges().constEnd(); ++it) {
        if (!graph.getBlockedEdges().contains(it.key())) {
            if (it.key().first < it.key().second) {
                availableEdges.append(it.key());
            }
        }
    }

    if (availableEdges.isEmpty()) {
        qDebug() << "No more edges available to block.";
        edgeBlockTimer.stop();
        return;
    }

    int randomIndex = QRandomGenerator::global()->bounded(availableEdges.size());
    QPair<qint64, qint64> edgeToBlock = availableEdges.at(randomIndex);

    graph.blockEdge(edgeToBlock.first, edgeToBlock.second);
    qDebug() << "Blocked edge between nodes" << edgeToBlock.first << "and" << edgeToBlock.second;

    Edge* edge = graph.getEdges().value(edgeToBlock);
    if (!edge) {
        qWarning() << "Edge not found in graph for blocking.";
        return;
    }

    m_blockedEdgesModel->addBlockedEdgeWithTimestamp(edge->start->id, edge->end->id,
                                                     edge->start->coordinate.latitude(),
                                                     edge->start->coordinate.longitude(),
                                                     edge->end->coordinate.latitude(),
                                                     edge->end->coordinate.longitude(),
                                                     QDateTime::currentDateTime());

    emit blockedEdgesChanged();
}

QVariantList SimulationManager::getBlockedEdges() const {
    return m_blockedEdgesModel->getBlockedEdges();
}

void SimulationManager::placeRandomObstacles(int count) {
    // Block 'count' number of edges immediately
    for (int i = 0; i < count; ++i) {
        blockRandomEdge();
    }
}

void SimulationManager::unblockExpiredEdges()
{
    QDateTime now = QDateTime::currentDateTime();
    QList<QPair<qint64, qint64>> edgesToUnblock;

    QVariantList blockedEdgesVariant = m_blockedEdgesModel->getBlockedEdges();

    for (const QVariant &var : blockedEdgesVariant) {
        QVariantMap map = var.toMap();
        qint64 startId = map.value("startId").toLongLong();
        qint64 endId = map.value("endId").toLongLong();
        QString blockedAtStr = map.value("blockedAt").toString();
        QDateTime blockedAt = QDateTime::fromString(blockedAtStr, Qt::ISODate);

        if (blockedAt.isValid() && blockedAt.msecsTo(now) >= obstacleDurationMs) {
            edgesToUnblock.append(qMakePair(startId, endId));
        }
    }

    for (const QPair<qint64, qint64> &edge : edgesToUnblock) {
        graph.unblockEdge(edge.first, edge.second);
        m_blockedEdgesModel->removeBlockedEdge(edge.first, edge.second);
        qDebug() << "SimulationManager unblocked edge between nodes" << edge.first << "and" << edge.second;
    }

    if (!edgesToUnblock.isEmpty()) {
        emit blockedEdgesChanged();
    }
}

QVariantList SimulationManager::getCommunicationLinks() const {
    QVariantList list;
    for (const auto &pair : communicationLinks) {
        QVariantMap map;
        map["startLat"] = pair.first.latitude();
        map["startLon"] = pair.first.longitude();
        map["endLat"] = pair.second.latitude();
        map["endLon"] = pair.second.longitude();
        list.append(map);
    }
    return list;
}

