#include "path.h"

Path::Path()
    : pathLength(0)
{
}

Path::Path(const QList<Edge*>& edges, qint64 startNodeId)
    : pathLength(0)
{
    if (edges.isEmpty()) {
        return;
    }

    double cumulative = 0.0;
    // On sait a priori que edges représente un chemin du point startNodeId
    // vers le endNodeId final, dans l'ordre.
    // Donc pour chaque Edge, on vérifie s'il doit être parcouru
    // start->end ou end->start, selon la continuité.

    qint64 currentNode = startNodeId;

    for (Edge* e : edges) {
        // Détermine le sens "forward" si l'arête part du node courant
        bool forward = false;
        if (e->start->id == currentNode) {
            forward = true;
            currentNode = e->end->id; // Le prochain node devient end->id
        } else if (e->end->id == currentNode) {
            forward = false;
            currentNode = e->start->id; // On parcourt l'arête en sens inverse
        } else {
            // Cas inattendu: l'edge ne correspond pas à currentNode
            // On le met par défaut en "forward" ou on peut logguer une erreur
            forward = true;
        }

        PathSegment seg;
        seg.edge = e;
        seg.forward = forward;
        seg.cumulativeLength = cumulative;
        segments.append(seg);

        // On incrémente la distance cumulée
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
    // Gestion des cas limites
    if (segments.isEmpty()) {
        return QGeoCoordinate(); // Chemin vide
    }
    if (distance <= 0.0) {
        // On est au tout début du chemin
        const PathSegment& firstSeg = segments.first();
        return firstSeg.forward ? firstSeg.edge->start->coordinate
                                : firstSeg.edge->end->coordinate;
    }
    if (distance >= pathLength) {
        // On est à la fin du chemin
        const PathSegment& lastSeg = segments.last();
        return lastSeg.forward ? lastSeg.edge->end->coordinate
                               : lastSeg.edge->start->coordinate;
    }

    // Trouver le segment où se situe "distance"
    int segIndex = 0;
    while (segIndex < segments.size() - 1
           && distance >= segments[segIndex + 1].cumulativeLength) {
        segIndex++;
    }

    const PathSegment& seg = segments[segIndex];
    double segmentStart = seg.cumulativeLength;
    double localDistance = distance - segmentStart;

    // En fonction du sens "forward", on utilise start->end ou end->start
    QGeoCoordinate startCoord = seg.forward ? seg.edge->start->coordinate
                                            : seg.edge->end->coordinate;
    QGeoCoordinate endCoord   = seg.forward ? seg.edge->end->coordinate
                                          : seg.edge->start->coordinate;

    double azimuth = startCoord.azimuthTo(endCoord);

    // Interpolation "linéaire" en suivant l'azimut
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
