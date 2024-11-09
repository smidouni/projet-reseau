#ifndef PATH_H
#define PATH_H

#include <QList>
#include <QGeoCoordinate>
#include "Edge.h"

class Path {
public:
    Path(); // Default constructor
    Path(const QList<Edge*> &edges);

    double totalLength() const;
    QGeoCoordinate getPositionAtDistance(double distance) const;

    // New getter method to access edges
    QList<Edge*> getEdges() const;

private:
    QList<Edge*> edges;
    QList<double> cumulativeLengths;
    double pathLength;

    int getEdgeIndexAtDistance(double distance) const;
};

#endif // PATH_H