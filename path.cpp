#include "Path.h"

Path::Path() : pathLength(0) {}

Path::Path(const QList<Edge*> &edges) : edges(edges), pathLength(0) {
    double cumulative = 0.0;
    for (const Edge *edge : edges) {
        cumulativeLengths.append(cumulative);
        cumulative += edge->length;
    }
    pathLength = cumulative;
}

double Path::totalLength() const {
    return pathLength;
}

int Path::getEdgeIndexAtDistance(double distance) const {
    for (int i = 0; i < cumulativeLengths.size() - 1; ++i) {
        if (distance < cumulativeLengths[i + 1]) {
            return i;
        }
    }
    return edges.size() - 1;
}

QGeoCoordinate Path::getPositionAtDistance(double distance) const {
    if (edges.isEmpty() || distance >= pathLength) {
        return edges.isEmpty() ? QGeoCoordinate() : edges.last()->end->coordinate;
    }

    int edgeIndex = getEdgeIndexAtDistance(distance);
    Edge *currentEdge = edges[edgeIndex];
    double edgeStartDistance = cumulativeLengths[edgeIndex];
    double localDistance = distance - edgeStartDistance;

    QGeoCoordinate startCoord = currentEdge->start->coordinate;
    QGeoCoordinate endCoord = currentEdge->end->coordinate;
    double azimuth = startCoord.azimuthTo(endCoord);

    return startCoord.atDistanceAndAzimuth(localDistance, azimuth);
}

// New getter method implementation
QList<Edge*> Path::getEdges() const {
    return edges;
}
