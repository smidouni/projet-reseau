// vehicle.cpp

#include "vehicle.h"
#include "simulationmanager.h"
#include <QColor>

static const int MAX_START_RETRIES = 50;
static const double MIN_SPEED = 30.0; // km/h
static const double MAX_SPEED = 50.0; // km/h
static const double PUISSANCE_MIN = 80.0; // in Watts
static const double PUISSANCE_MAX = 120.0; // in Watts
static const double FREQUENCE_MIN = 3.0 * pow(10, 9); // 3 GHz
static const double FREQUENCE_MAX = 26.0 * pow(10, 9); // 26 GHz
static const double LIGHT_SPEED = 3.0 * pow(10, 8); // m/s

Vehicle::Vehicle(int id, Graph &graph, qint64 startNodeId, QObject *parent)
    : QObject(parent),
    id(id),
    graph(graph),
    currentNodeId(startNodeId),
    destinationNodeId(-1),
    speed(0.0),
    distanceAlongPath(0.0),
    m_communicationRange(50.0) // Initialize communication range (in meters)
{
    /*
    Pt = entre 50 et 150 W
    Gt = 10
    Gr = 10
    fc = entre 3 et 26 GHz
    Pr_min = 1 microWatt
    Chaque fréquence correspond à une couleur
    */

    // Puissance transmise
    const double Pt = QRandomGenerator::global()->bounded(PUISSANCE_MAX - PUISSANCE_MIN) + PUISSANCE_MIN;
    // Gain de transmission
    const double Gt = 10.0;
    // Gain de reception
    const double Gr = 10.0;
    // Fréquence entre 3 et 26 GHz
    const double fc = QRandomGenerator::global()->bounded(FREQUENCE_MAX - FREQUENCE_MIN) + FREQUENCE_MIN;
    const double lambda = LIGHT_SPEED / fc;
    // Puissance de reception minimale
    const double Pr_min = 0.000001; // 1 microWatt

    m_communicationRange = sqrt(Pt * Gt * Gr / Pr_min) * lambda / (4 * M_PI);

    // 1) Random speed between MIN_SPEED and MAX_SPEED
    speed = QRandomGenerator::global()->bounded(MAX_SPEED - MIN_SPEED) + MIN_SPEED;

    // 2) Assign a color based on frequency
    pickRandomColor(fc);

    // 3) Attempt to pick a valid path
    if (!graph.nodes.contains(currentNodeId)) {
        if (!graph.nodes.isEmpty()) {
            currentNodeId = graph.nodes.keys().at(
                QRandomGenerator::global()->bounded(graph.nodes.size())
                );
        } else {
            qWarning() << "Graph has no nodes.";
            return;
        }
    }
    bool initOK = tryInitValidStartNode();
    if (!initOK) {
        currentPosition = graph.nodes[currentNodeId]->coordinate;
        qWarning() << "Vehicle" << id
                   << "couldn’t find valid path from start, may remain stuck.";
    } else {
        currentPosition = currentPath.getPositionAtDistance(0.0);
    }

    connect(&messageTimer, &QTimer::timeout, this, [this]() {
        // Reset messageReceived to false after 1 second
        m_messageReceived = false;
        emit messageReceivedChanged();
    });

    emit positionChanged();
    emit colorChanged(); // Notify QML of initial color
}

double Vehicle::lat() const
{
    return currentPosition.latitude();
}

double Vehicle::lon() const
{
    return currentPosition.longitude();
}

double Vehicle::communicationRange() const
{
    return m_communicationRange;
}

QString Vehicle::color() const
{
    return colorString;
}

bool Vehicle::messageReceived() const
{
    return m_messageReceived;
}

void Vehicle::setMessageReceived(bool received)
{
    if (m_messageReceived != received) {
        m_messageReceived = received;
        emit messageReceivedChanged();

        if (received) {
            // Start the timer for 10 second
            messageTimer.start(1000);
        }
    }
}


void Vehicle::setCommunicationRange(double range)
{
    if (!qFuzzyCompare(m_communicationRange, range)) {
        m_communicationRange = range;
        emit communicationRangeChanged();
    }
}

int Vehicle::getId() const
{
    return id;
}

void Vehicle::updatePosition(double deltaTime) {
    if (currentPath.totalLength() < 1e-6) {
        return; // No path to follow
    }

    double speed_m_s = speed * (1000.0 / 3600.0); // Convert speed to m/s
    double travelDistance = speed_m_s * deltaTime;
    distanceAlongPath += travelDistance;

    const QList<Edge*> &pathEdges = currentPath.getEdges();
    double cumulativeLength = 0.0;

    for (Edge *edge : pathEdges) {
        cumulativeLength += edge->length;

        // Check if the vehicle is about to traverse a blocked edge
        if (distanceAlongPath >= cumulativeLength - edge->length && edge->blocked) {
            qDebug() << "Vehicle" << id << "encountered a blocked edge. Recalculating path.";

            // Stop at the node before the blocked edge
            distanceAlongPath = cumulativeLength - edge->length;
            currentNodeId = edge->start->id;

            // Report the obstacle and immediately recalculate path
            reportObstacle(qMakePair(edge->start->id, edge->end->id), dynamic_cast<SimulationManager*>(parent()));
            recalculatePath(); // Ensure the vehicle finds a new route
            return;
        }

    }

    // If the vehicle reaches the end of its path, set a new destination
    if (distanceAlongPath >= currentPath.totalLength()) {
        distanceAlongPath = currentPath.totalLength();
        qint64 finalNode = currentPath.getFinalNodeId();
        if (finalNode >= 0) {
            currentNodeId = finalNode;
        }
        setRandomDestination();
        distanceAlongPath = 0.0;
        currentPosition = currentPath.getPositionAtDistance(0.0);
        emit positionChanged();
        return;
    }

    // Update the current position
    currentPosition = currentPath.getPositionAtDistance(distanceAlongPath);
    emit positionChanged();
}

void Vehicle::setDestination(qint64 destinationNodeId)
{
    this->destinationNodeId = destinationNodeId;
    recalculatePath();
}

void Vehicle::setRandomDestination()
{
    if (graph.nodes.size() <= 1) {
        qWarning() << "Vehicle" << id << "Not enough nodes to pick a random destination.";
        return;
    }

    qint64 newDest = currentNodeId;
    while (newDest == currentNodeId && graph.nodes.size() > 1) {
        newDest = graph.nodes.keys().at(
            QRandomGenerator::global()->bounded(graph.nodes.size())
            );
    }
    setDestination(newDest);
}

void Vehicle::recalculatePath() {
    if (!graph.nodes.contains(currentNodeId)) {
        qWarning() << "Vehicle" << id << "recalculatePath: currentNodeId"
                   << currentNodeId << "not in graph!";
        return;
    }

    QList<Edge*> pathEdges = graph.findPath(currentNodeId, destinationNodeId, knownBlockedEdges);

    if (pathEdges.isEmpty()) {
        qWarning() << "Vehicle" << id << "No path found from" << currentNodeId
                   << "to" << destinationNodeId;

        // Attempt to set a new random destination
        setRandomDestination();

        // If still no valid path, remain stationary
        pathEdges = graph.findPath(currentNodeId, destinationNodeId, knownBlockedEdges);
        if (pathEdges.isEmpty()) {
            qWarning() << "Vehicle" << id << "still has no valid path. Staying at current position.";
            currentPath = Path(); // Clear the path
            return;
        }
    }

    currentPath = Path(pathEdges, currentNodeId);
    distanceAlongPath = 0.0;
    currentPosition = currentPath.getPositionAtDistance(0.0);
    emit positionChanged();
}

void Vehicle::backtrackToPreviousNode()
{
    QList<Edge*> edges = currentPath.getEdges();
    if (!edges.isEmpty()) {
        Edge* firstEdge = edges.first();
        currentNodeId = firstEdge->start->id;
        recalculatePath();
    }
}

bool Vehicle::tryInitValidStartNode()
{
    for (int attempt = 0; attempt < MAX_START_RETRIES; ++attempt) {
        qint64 testDest = currentNodeId;
        while (testDest == currentNodeId && graph.nodes.size() > 1) {
            testDest = graph.nodes.keys().at(
                QRandomGenerator::global()->bounded(graph.nodes.size())
                );
        }

        QList<Edge*> pathEdges = graph.findPath(currentNodeId, testDest, knownBlockedEdges);
        if (!pathEdges.isEmpty()) {
            currentPath = Path(pathEdges, currentNodeId);
            distanceAlongPath = 0.0;
            destinationNodeId = testDest;
            return true;
        }

        qWarning() << "Vehicle" << id
                   << "No path from" << currentNodeId << "to" << testDest
                   << "(attempt" << attempt << ") picking new start node.";

        if (!graph.nodes.isEmpty()) {
            currentNodeId = graph.nodes.keys().at(
                QRandomGenerator::global()->bounded(graph.nodes.size())
                );
        } else {
            qWarning() << "Graph has no nodes.";
            return false;
        }
    }
    return false;
}

void Vehicle::pickRandomColor(double fc)
{
    // Generate a color based on frequency or other parameters
    // For example, map frequency to hue
    // Normalize frequency to [0, 360] for hue
    double hue = 360.0 * (fc - FREQUENCE_MIN) / (FREQUENCE_MAX - FREQUENCE_MIN);

    // Ensure hue is within [0, 360)
    hue = fmod(hue, 360.0);

    // Set saturation and value to make it bright
    double saturation = 1.0;
    double value = 1.0;

    QColor c;
    c.setHsvF(hue / 360.0, saturation, value);

    colorString = c.name(QColor::HexRgb); // e.g., "#FF00FF"

    emit colorChanged(); // Notify QML of color change
}

void Vehicle::receiveObstacle(const QPair<qint64, qint64> &blockedEdge) {
    if (knownBlockedEdges.contains(blockedEdge)) {
        return; // Already aware of this blocked edge
    }

    qDebug() << "Vehicle" << id << "received blocked edge notification for" << blockedEdge;

    // Mark the edge as blocked
    knownBlockedEdges.insert(blockedEdge);
    knownBlockedEdges.insert(qMakePair(blockedEdge.second, blockedEdge.first)); // Reverse direction

    // Turn the vehicle green for 10 seconds
    setMessageReceived(true);

    // Recalculate path at the next node
    recalculatePathAtNextNode = true;
}



void Vehicle::reportObstacle(const QPair<qint64, qint64> &blockedEdge, SimulationManager* simulationManager) {
    // Avoid reporting the same blocked edge multiple times
    if (knownBlockedEdges.contains(blockedEdge)) {
        qDebug() << "Vehicle" << id << "already reported this blocked edge. Skipping.";
        return;
    }

    qDebug() << "Vehicle" << id << "reporting blocked edge:" << blockedEdge;

    // Notify the simulation manager about the blocked edge
    simulationManager->handleObstacle(this, blockedEdge);

    // Mark the edge as blocked for this vehicle
    knownBlockedEdges.insert(blockedEdge);
    knownBlockedEdges.insert(qMakePair(blockedEdge.second, blockedEdge.first)); // Reverse direction
}


bool Vehicle::currentPathHasEdge(const QPair<qint64, qint64> &edge) const
{
    const QList<Edge*> &pathEdges = currentPath.getEdges();
    for (Edge* e : pathEdges) {
        if ((e->start->id == edge.first && e->end->id == edge.second) ||
            (e->end->id == edge.first && e->start->id == edge.second)) {
            return true;
        }
    }
    return false;
}

QGeoCoordinate Vehicle::getCurrentPosition() const {
    return currentPosition;
}

