#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H

#include <QObject>
#include <QQmlListProperty>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
#include "vehicle.h"
#include "graph.h"
#include "blockededgesmodel.h"
#include "communicationlinksmodel.h"

class SimulationManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList blockedEdges READ getBlockedEdges NOTIFY blockedEdgesChanged)
    Q_PROPERTY(QQmlListProperty<QObject> vehiclesModel READ vehiclesModel NOTIFY vehiclesUpdated)
    Q_PROPERTY(CommunicationLinksModel* communicationLinksModel READ communicationLinksModel NOTIFY communicationLinksChanged)

public:
    explicit SimulationManager(Graph &graph, QObject *parent = nullptr);
    void addVehicle(int id, qint64 startNodeId);
    void setSpeedFactor(double factor);
    void clearVehicles();
    Graph& getGraph();

    // Expose blockedEdges as a QVariantList for QML
    QVariantList getBlockedEdges() const;

    QVariantList getCommunicationLinks() const;

    // Expose vehiclesModel as QQmlListProperty for QML
    QQmlListProperty<QObject> vehiclesModel();

    // Accessor for BlockedEdgesModel
    BlockedEdgesModel* blockedEdgesModel() const { return m_blockedEdgesModel; }
    void placeRandomObstacles(int count);

    // Method to handle obstacle reports from vehicles
    void handleObstacle(Vehicle* reportingVehicle, const QPair<qint64, qint64> &blockedEdge);
    CommunicationLinksModel* communicationLinksModel() const { return m_communicationLinksModel; }


public slots:
    void updateVehicles();       // Called on simulation timer
    void blockRandomEdge();      // Blocks a random edge periodically

private slots:
    void unblockExpiredEdges();  // Unblocks edges after their duration

signals:
    void updated();
    void vehiclesUpdated();
    void blockedEdgesChanged();  // Notify QML about blocked edges updates
    void communicationLinksChanged();

private:
    // Static functions for QQmlListProperty
    static void appendVehicle(QQmlListProperty<QObject> *list, QObject *vehicle);
    static qint64 vehicleCount(QQmlListProperty<QObject> *list);
    static QObject* vehicleAt(QQmlListProperty<QObject> *list, qint64 index);
    static void clearVehicles(QQmlListProperty<QObject> *list);

    // Helper method for vehicle communication
    QList<Vehicle*> findConnectedVehicles(Vehicle* startVehicle);

    Graph &graph;
    QList<Vehicle*> vehicles;
    QTimer simulationTimer;
    QTimer edgeBlockTimer;     // Timer for blocking edges
    double speedFactor = 1.0;
    QElapsedTimer elapsedTimer;
    QTimer unblockTimer;       // Timer to check for edges to unblock
    const int obstacleDurationMs = 100000;
    QList<QPair<QGeoCoordinate, QGeoCoordinate>> communicationLinks;

    BlockedEdgesModel *m_blockedEdgesModel = new BlockedEdgesModel(this);
    CommunicationLinksModel *m_communicationLinksModel = new CommunicationLinksModel(this);

};

#endif // SIMULATIONMANAGER_H
