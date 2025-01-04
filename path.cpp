#include "path.h"
#include <QDebug>

Path::Path()
    : pathLength(0.0)
{
}

Path::Path(const QList<Edge*>& edges, qint64 startNodeId)
    : pathLength(0.0)
{
    if (edges.isEmpty()) {
        return;
    }

    double cumulative = 0.0;
    qint64 currentNode = startNodeId;

    for (Edge* e : edges) {
        if (!e) {
            continue;
        }

        bool forward = false;
        if (e->start->id == currentNode) {
            forward = true;
            currentNode = e->end->id;
        } else if (e->end->id == currentNode) {
            forward = false;
            currentNode = e->start->id;
        } else {
            // Edge doesn't match currentNode: force forward by default
            forward = true;
        }

        PathSegment seg;
        seg.edge = e;
        seg.forward = forward;
        seg.cumulativeLength = cumulative;
        segments.append(seg);

        cumulative += e->length;
    }

    pathLength = cumulative;
}

double Path::totalLength() const
{
    return pathLength;
}

QGeoCoordinate Path::getPositionAtDistance(double distance) const
{
    if (segments.isEmpty()) {
        return QGeoCoordinate();
    }

    if (distance <= 0.0) {
        const PathSegment& firstSeg = segments.first();
        return firstSeg.forward ? firstSeg.edge->start->coordinate
                                : firstSeg.edge->end->coordinate;
    }

    if (distance >= pathLength) {
        const PathSegment& lastSeg = segments.last();
        return lastSeg.forward ? lastSeg.edge->end->coordinate
                               : lastSeg.edge->start->coordinate;
    }

    int segIndex = 0;
    while (segIndex < segments.size() - 1 &&
           distance >= segments[segIndex + 1].cumulativeLength)
    {
        segIndex++;
    }

    const PathSegment& seg = segments[segIndex];
    double segmentStart = seg.cumulativeLength;
    double localDistance = distance - segmentStart;

    QGeoCoordinate startCoord = seg.forward ? seg.edge->start->coordinate
                                            : seg.edge->end->coordinate;
    QGeoCoordinate endCoord   = seg.forward ? seg.edge->end->coordinate
                                          : seg.edge->start->coordinate;

    double azimuth = startCoord.azimuthTo(endCoord);
    return startCoord.atDistanceAndAzimuth(localDistance, azimuth);
}

QList<Edge*> Path::getEdges() const
{
    QList<Edge*> list;
    for (const PathSegment& seg : segments) {
        list.append(seg.edge);
    }
    return list;
}

qint64 Path::getFinalNodeId() const
{
    if (segments.isEmpty()) {
        return -1;
    }
    const PathSegment &lastSeg = segments.last();
    return lastSeg.forward ? lastSeg.edge->end->id
                           : lastSeg.edge->start->id;
}
