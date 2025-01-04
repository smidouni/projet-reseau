#ifndef PATH_H
#define PATH_H

#include <QList>
#include <QGeoCoordinate>
#include "edge.h"

/**
 * @brief PathSegment
 * Représente un segment d'un chemin, c'est-à-dire l'utilisation
 * d'un Edge particulier dans un sens précis (forward ou backward).
 */
struct PathSegment {
    Edge* edge;
    bool forward;             // true si on va de edge->start vers edge->end
    double cumulativeLength;  // distance cumulée depuis le début du path
};

class Path {
public:
    Path();
    /**
     * @brief Constructeur prenant une liste d'Edge dans l'ordre exact
     *        (ceux retournés par Graph::findPath), plus l'ID de départ.
     *
     * @param edges         Liste d'arêtes dans l'ordre du chemin
     * @param startNodeId   Le node ID d'où l'on part (pour déterminer le sens)
     */
    Path(const QList<Edge*>& edges, qint64 startNodeId);

    double totalLength() const;
    QGeoCoordinate getPositionAtDistance(double distance) const;

    /**
     * @brief getEdges
     * Renvoie la liste des Edge* (sans précision de sens).
     */
    QList<Edge*> getEdges() const;

private:
    QList<PathSegment> segments;
    double pathLength;
};

#endif // PATH_H
