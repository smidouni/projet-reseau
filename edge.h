#ifndef EDGE_H
#define EDGE_H

#include "node.h"

class Edge {
public:
    Node *start;
    Node *end;
    double length;
    bool blocked;

    Edge(Node *start, Node *end, double length, bool blocked = false)
        : start(start), end(end), length(length), blocked(blocked) {}

    void toggleBlock(bool status) {
        blocked = status;
    }
};

#endif // EDGE_H
