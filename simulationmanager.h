#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include "Vehicle.h"
#include "Graph.h"

class SimulationManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> vehiclesModel READ getVehicles NOTIFY vehiclesUpdated)

public:
    explicit SimulationManager(Graph &graph, QObject *parent = nullptr);
    void addVehicle(int id, qint64 startNodeId);
    void updateVehicles();
    void setSpeedFactor(double factor);
    QList<QObject*> getVehicles() const;

signals:
    void updated();
    void vehiclesUpdated();

private:
    Graph &graph;
    QList<Vehicle*> vehicles;
    QTimer simulationTimer;
    double speedFactor = 1.0;
};

#endif // SIMULATIONMANAGER_H
