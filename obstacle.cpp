#include "obstacle.h"
#include <QTimer>

Obstacle::Obstacle(int id, double lat, double lon, QObject *parent)
    : QObject(parent), id(id), coordinate(lat, lon)
{
}

void Obstacle::flash()
{
    flashing = true;
    emit flashingChanged();

    // Stop flashing after 1 second
    QTimer::singleShot(1000, [this]() {
        flashing = false;
        emit flashingChanged();
    });
}
