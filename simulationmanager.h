#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
#include "vehicle.h"
#include "graph.h"

class SimulationManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> vehiclesModel READ getVehicles NOTIFY vehiclesUpdated)

public:
    explicit SimulationManager(Graph &graph, QObject *parent = nullptr);
    void addVehicle(int id, qint64 startNodeId);
    void setSpeedFactor(double factor);
    void clearVehicles();
    Graph& getGraph();

    QList<QObject*> getVehicles() const;

signals:
    void updated();
    void vehiclesUpdated();

private slots:
    void updateVehicles();

private:
    Graph &graph;
    QList<Vehicle*> vehicles;
    QTimer simulationTimer;
    double speedFactor = 1.0;

    // Timer pour mesurer le temps écoulé entre deux mises à jour
    QElapsedTimer elapsedTimer;
};

#endif // SIMULATIONMANAGER_H
