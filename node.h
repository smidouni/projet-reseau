#ifndef NODE_H
#define NODE_H

#include <QGeoCoordinate>

class Node {
public:
    qint64 id;
    QGeoCoordinate coordinate;

    Node(qint64 id, const QGeoCoordinate &coordinate)
        : id(id), coordinate(coordinate) {}
};

#endif // NODE_H
