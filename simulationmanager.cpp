#include "simulationmanager.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QRandomGenerator>

SimulationManager::SimulationManager(Graph &graph, QObject *parent)
    : QObject(parent), graph(graph)
{
    connect(&simulationTimer, &QTimer::timeout, this, &SimulationManager::updateVehicles);
    simulationTimer.start(16); // ~60 FPS
    elapsedTimer.start();
}

void SimulationManager::addVehicle(int id, qint64 startNodeId)
{
    if (!graph.nodes.isEmpty()) {
        Vehicle *vehicle = new Vehicle(id, graph, startNodeId);
        vehicles.append(vehicle);
    } else {
        qWarning() << "Graph is empty; cannot add vehicle" << id;
    }
}

void SimulationManager::addObstacle(int id, double lat, double lon)
{
    Obstacle *obs = new Obstacle(id, lat, lon, this);
    obstacles.append(obs);
}

void SimulationManager::updateVehicles()
{
    qint64 elapsedMs = elapsedTimer.elapsed();
    elapsedTimer.restart();

    double deltaTime = (elapsedMs / 1000.0) * speedFactor;

    // Update each vehicle’s position
    for (Vehicle *v : vehicles) {
        v->updatePosition(deltaTime);

        // Check if the vehicle is “close enough” to any obstacle
        // e.g. < 10m => we consider it “discovered”
        for (Obstacle *obs : obstacles) {
            QGeoCoordinate c1(v->lat(), v->lon());
            QGeoCoordinate c2(obs->lat(), obs->lon());
            double dist = c1.distanceTo(c2);
            if (dist < 10.0) { // if within 10m, say it discovered or "hit" the obstacle
                obs->flash(); // obstacle flashes green
                // Possibly mark edges blocked near obstacle...
                // ...
            }
        }
    }

    emit updated();
    emit vehiclesUpdated();
}

void SimulationManager::setSpeedFactor(double factor)
{
    speedFactor = factor;
}

QList<QObject*> SimulationManager::getVehicles() const
{
    QList<QObject*> list;
    for (auto v : vehicles) {
        list.append(v);
    }
    return list;
}

QList<QObject*> SimulationManager::getObstacles() const
{
    QList<QObject*> list;
    for (auto o : obstacles) {
        list.append(o);
    }
    return list;
}

void SimulationManager::clearVehicles() {
    qDebug() << "Suppression des véhicules...";
    for (auto vehicle : vehicles) {
        if (vehicle) {
            disconnect(vehicle, nullptr, nullptr, nullptr);  // Déconnecter tous les signaux/slots
            delete vehicle;  // Supprimer l'objet
        }
    }
    vehicles.clear();  // Nettoyer la liste
    qDebug() << "Tous les véhicules ont été supprimés.";
}


Graph& SimulationManager::getGraph() {
    return graph;
}

