#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
#include "vehicle.h"
#include "graph.h"
#include "obstacle.h"

class SimulationManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> vehiclesModel READ getVehicles NOTIFY vehiclesUpdated)
    Q_PROPERTY(QList<QObject*> obstaclesModel READ getObstacles NOTIFY obstaclesUpdated)

public:
    explicit SimulationManager(Graph &graph, QObject *parent = nullptr);
    void addVehicle(int id, qint64 startNodeId);
    void addObstacle(int id, double lat, double lon);
    void setSpeedFactor(double factor);
    void clearVehicles();
    Graph& getGraph();

    QList<QObject*> getVehicles() const;
    QList<QObject*> getObstacles() const;

signals:
    void updated();
    void vehiclesUpdated();
    void obstaclesUpdated();

public slots:
    void updateVehicles(); // Called on timer

private:
    Graph &graph;
    QList<Vehicle*> vehicles;
    QList<Obstacle*> obstacles;
    QTimer simulationTimer;
    double speedFactor = 1.0;
    QElapsedTimer elapsedTimer;
};

#endif // SIMULATIONMANAGER_H
