#include "simulationmanager.h"
#include <QDebug>
#include <QElapsedTimer>

SimulationManager::SimulationManager(Graph &graph, QObject *parent)
    : QObject(parent), graph(graph)
{
    // On connecte le timer à la méthode updateVehicles
    connect(&simulationTimer, &QTimer::timeout, this, &SimulationManager::updateVehicles);

    // 15 FPS => intervalle d’environ 66 ms
    simulationTimer.start(66);

    // On démarre le QElapsedTimer pour calculer le deltaTime
    elapsedTimer.start();
}

void SimulationManager::addVehicle(int id, qint64 startNodeId)
{
    if (!graph.nodes.isEmpty()) {
        Vehicle *vehicle = new Vehicle(id, graph, startNodeId);
        if (vehicle && graph.nodes.contains(startNodeId)) {
            vehicles.append(vehicle);
        } else {
            qWarning() << "Erreur lors de la création du véhicule : id=" << id;
        }
    } else {
        qWarning() << "Graph est vide. Impossible d'ajouter un véhicule.";
    }

}

void SimulationManager::updateVehicles()
{
    // Temps écoulé en millisecondes depuis le dernier appel
    qint64 elapsedMs = elapsedTimer.elapsed();
    // On relance le timer pour la prochaine fois
    elapsedTimer.restart();

    // Conversion en secondes avec facteur de vitesse
    double deltaTime = (elapsedMs / 1000.0) * speedFactor;

    // Mise à jour de chaque véhicule
    for (Vehicle *vehicle : vehicles) {
        vehicle->updatePosition(deltaTime);
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
    QList<QObject*> vehicleObjects;
    for (auto vehicle : vehicles) {
        vehicleObjects.append(vehicle);
    }
    return vehicleObjects;
}
