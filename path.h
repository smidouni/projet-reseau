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
     * @brief Construit un chemin à partir d'une liste d'Edges (dans l'ordre),
     *        et d'un node de départ pour déterminer le sens des segments.
     */
    Path(const QList<Edge*>& edges, qint64 startNodeId);

    double totalLength() const;

    /**
     * @brief getPositionAtDistance
     * Calcule la position (latitude/longitude) le long du chemin
     * en fonction d'une distance parcourue depuis le début du Path.
     */
    QGeoCoordinate getPositionAtDistance(double distance) const;

    /**
     * @brief getEdges
     * Renvoie la liste des Edge* (sans précision de sens).
     */
    QList<Edge*> getEdges() const;

    /**
     * @brief getFinalNodeId
     * Renvoie l'ID du dernier nœud (celui où le Path se termine).
     */
    qint64 getFinalNodeId() const;

private:
    QList<PathSegment> segments;
    double pathLength;
};

#endif // PATH_H
