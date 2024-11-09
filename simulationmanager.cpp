#include "SimulationManager.h"

SimulationManager::SimulationManager(Graph &graph, QObject *parent)
    : QObject(parent), graph(graph) {
    connect(&simulationTimer, &QTimer::timeout, this, &SimulationManager::updateVehicles);
    simulationTimer.start(16); // ~60 updates per second
}

void SimulationManager::addVehicle(int id, qint64 startNodeId) {
    if (!graph.nodes.isEmpty()) {
        Vehicle *vehicle = new Vehicle(id, graph, startNodeId);
        vehicles.append(vehicle);
    } else {
        qWarning() << "Cannot add vehicle, graph has no nodes.";
    }
}


void SimulationManager::updateVehicles() {
    double deltaTime = 0.016 * speedFactor; // Adjust deltaTime based on speed factor
    for (Vehicle *vehicle : vehicles) {
        vehicle->updatePosition(deltaTime);
    }
    emit updated();
    emit vehiclesUpdated();
}

void SimulationManager::setSpeedFactor(double factor) {
    speedFactor = factor;
}

QList<QObject*> SimulationManager::getVehicles() const {
    QList<QObject*> vehicleObjects;
    for (auto vehicle : vehicles) {
        vehicleObjects.append(vehicle);
    }
    return vehicleObjects;
}
